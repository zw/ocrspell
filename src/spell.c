/*
 * $Id: spell.c,v 1.2 2002/08/22 22:39:36 condit Exp $
 *
 * Copyright (C) 1995 Regents of the University of Nevada
 *
 * See copyright.h for details.
 *
 * This code is written to work with ispell version 3.1
 * (3.1.08 is the one we are running currently).  This is
 * not the GNU version.  You can easily modify this code to
 * work with a different version of ispell, or an entirely
 * different spell checker.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef STDC_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#endif /* STDC_HEADERS */

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif /* HAVE_SYS_WAIT_H */
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

#include "copyright.h"

#define DEFAULT_HASH_DICT_PATH "/local/manicure/lib/manicure2.hash"
#define MAX_NEAR_MISSES 8
#define ISPELL_RESPONSE_BUFSIZ  1000
#define ISPELL_MAX_WORD_LEN     100
#define ISPELL_MAX_GREETING_LEN 100


static int spell_checker_pid = -1;
static FILE *to_spell_checker_st, *from_spell_checker_st;


void
handle_spell_checker_sigchld() {
	/*
	 * if we get a SIGCHLD, the spell checker exited.  if it exited zero,
	 * just wait on it and that's it.  if nonzero, spit out
	 * an error message and exit.
	 */
	int status;
	waitpid(spell_checker_pid, &status, 0);
	if (status & 0x0000ffff) {
		fprintf(stderr, "spell checker exited prematurely with status %d\n",
				(0x0000ff00 & status));
		exit(1);
	}
}


int
spell_checker_startup(char *ispell_path, char *hash_dict_path) {
	/*
	 * ispell_path:     the path to the ispell(1) executable
	 * hash_dict_path:  if non-NULL, ispell is invoked with -d using
	 *   hash_dict_path as the dictionary.  if NULL, it uses
	 *   DEFAULT_HASH_DICT_PATH instead.  note if hash_dict_path
	 *   does not contain a '/', ispell prepends the installation
	 *   library directory.
	 */
	int to_spell_checker_fd[2], from_spell_checker_fd[2];
	char buf[ISPELL_MAX_GREETING_LEN];

	if ((pipe(to_spell_checker_fd) == -1) ||
		(pipe(from_spell_checker_fd) == -1)) {
		perror("pipe");
		return(0);
	}

	signal(SIGCHLD, handle_spell_checker_sigchld);

	/* create spell checker process */
	spell_checker_pid = fork();
	if (spell_checker_pid < 0) {
		perror("fork");
		return(0);
	} else if (spell_checker_pid == 0) {
		/* close the ends of the pipes the spell checker doesn't need */
		close(to_spell_checker_fd[1]);
		close(from_spell_checker_fd[0]);

		if ((dup2(to_spell_checker_fd[0], 0) == -1) ||
			(dup2(from_spell_checker_fd[1], 1) == -1)) {
			perror("dup2");
			exit(1);
		}
		close(to_spell_checker_fd[0]);
		close(from_spell_checker_fd[1]);

		if (hash_dict_path) {
			execlp((ispell_path == NULL) ? "ispell" : ispell_path,
				   "ispell", "-a", "-d", hash_dict_path, (char *)0);
		} else {
			execlp((ispell_path == NULL) ? "ispell" : ispell_path,
				   "ispell", "-a", "-d", DEFAULT_HASH_DICT_PATH, (char *)0);
		}
		perror(ispell_path);
		exit(1);
	}

	/* close the ends of the pipes the parent doesn't need */
	close(to_spell_checker_fd[0]);
	close(from_spell_checker_fd[1]);

	/* make streams out of the pipe file descriptors */
	if (!(to_spell_checker_st = fdopen(to_spell_checker_fd[1], "w"))) {
		perror("fdopen");
		close(to_spell_checker_fd[1]);
		close(from_spell_checker_fd[0]);
		return(0);
	}
	if (!(from_spell_checker_st = fdopen(from_spell_checker_fd[0], "r"))) {
		perror("fdopen");
		fclose(to_spell_checker_st);
		close(from_spell_checker_fd[0]);
		return(0);
	}

	/* trash spell_checker greeting line */
	fgets(buf, ISPELL_MAX_GREETING_LEN, from_spell_checker_st);
	
	return(1);
}


int
in_dictionary(char *word, char ***near_misses) {
	/*
	 * returns true if word is in dictionary, false if not.
	 * also, if 'near_misses' is non-NULL, and if ispell returns
	 * a '&' indicating the word was misspelled and it was able
	 * to give alternatives, then 'near_misses' will be assigned
	 * a pointer to a NULL terminated array of char *'s (a la argv),
	 * where each string in the array is a near miss.  the caller
	 * is responsible for freeing up that space when finished with it.
	 */
	char wbuf[ISPELL_MAX_WORD_LEN], rbuf[ISPELL_RESPONSE_BUFSIZ],
		*p, *q, *r, *near_miss_start;
	char terminator;
	int bad, num_near_misses, i, nm, len;

	if (spell_checker_pid < 0) {
		fprintf(stderr, "spell checker not running\n");
		exit(1);
	}

	/*
	 * make sure it's not null, doesn't start with a dash, and
	 * isn't ridiculously long.
	 */
	if (!word) {
		return(0);
	}
	len = strlen(word);
	if ((word[0] == '-') || (len > (ISPELL_MAX_WORD_LEN-1))) {
		return(0);
	}

	strcpy(wbuf, word);

	/*
	 * if the word contains anything but [-a-zA-Z0-9'/.]
	 * then it is not in the dictionary.
	 */
	for (p = wbuf; *p; p++) {
		if (!isalnum((int)*p) && (*p != '-') && (*p != '\'') &&
			(*p != '/') && (*p != '.')) {
			return(0);
		}
	}

	/*
	 * chop off trailing 's if present.  ispell sometimes thinks
	 * blah is correct but blah's is not.
	 */
	if ((len > 2) && !strcasecmp(&wbuf[len-2], "'s")) {
		wbuf[len-2] = '\0';
	}

	fprintf(to_spell_checker_st, "%s\n", wbuf);
	fflush(to_spell_checker_st);
	rbuf[0] = '\0';
	bad = 0;
	if (near_misses) {
		*near_misses = NULL;
	}

	/*
	 * it's possible that ispell will return more than one
	 * response for a single word due to hyphens, etc.
	 * we say if any part of the word is misspelled,
	 * then the whole word is misspelled.
	 */
	for (;;) {
		if (!fgets(rbuf, ISPELL_RESPONSE_BUFSIZ, from_spell_checker_st)) {
			fprintf(stderr, "error reading spell checker output\n");
			return(0);
		} else if (rbuf[0] == '\n') {
			break;
		} else if ((rbuf[0] == '#') || (rbuf[0] == '?')) {
			bad++;
		} else if (rbuf[0] == '&') {
			bad++;
			if (bad == 1) {
				/*
				 * word was not in the dictionary and there were
				 * near misses given.
				 *
				 * ispell response looks like this:
				 *   & ountain 2 0: fountain, mountain
				 *
				 * first scan through the near misses quickly
				 * and see if one of them matches the original
				 * word, disregarding case.  if so, then we say
				 * it's a correctly spelled word.  if not, and if
				 * near_misses is non NULL, parse the near misses
				 * out into an argv-like structure and return them
				 * in *near_misses.
				 */

				/*
				 * grab the number of near misses.
				 */
				p = strchr(rbuf+2, ' ');
				p++;
				q = strchr(p, ' ');
				*q = '\0';
				num_near_misses = atoi(p);
				*q = ' ';
				p = strchr(q, ':');
				p += 2;
				near_miss_start = p;

				/*
				 * scan for a case insensitive match.
				 */
				for (i=0; bad && (i<num_near_misses); i++) {
					q = strpbrk(p, ",\n");
					terminator = *q;
					*q = '\0';
					if (!strcasecmp(p, wbuf)) {
						bad = 0;
					}
					*q = terminator;
					p = q+2;
				}
				if (!bad || !near_misses) {
					continue;
				}

				/*
				 * didn't find a match.  the word is misspelled.
				 * caller wants near misses (near_misses is non NULL)
				 * so go suck them up.
				 */
				if (!(*near_misses = (char **)malloc((num_near_misses + 1)
													 * sizeof(char *)))) {
					perror(NULL);
					exit(1);
				}
				p = near_miss_start;
				if (num_near_misses > MAX_NEAR_MISSES) {
					num_near_misses = MAX_NEAR_MISSES;
				}
				for (i=0,nm=0; (i < num_near_misses) &&
								   (nm < MAX_NEAR_MISSES); i++) {
					q = strpbrk(p, ",\n");
					*q = '\0';
					/* exclude near misses that contain a space */
					if (!(r = strchr(p, ' '))) {
						if (!((*near_misses)[nm] =
							  (char *)malloc(strlen(p)+1))) {
							perror(NULL);
							exit(1);
						}
						strcpy((*near_misses)[nm], p);
						nm++;
					}
					p = q+2;
				}
				if (nm != num_near_misses) {
					/*
					 * either we hit the MAX_NEAR_MISSES limit,
					 * or there were some near misses that contained
					 * a space, causing them to get ignored.
					 * fix the size of the *near_misses array.
					 */
					if (!(*near_misses = (char **)
						  realloc(*near_misses, ((nm+1) * sizeof(char *))))) {
						perror(NULL);
						exit(1);
					}
				}
				(*near_misses)[nm] = NULL;
			}
		}
	}
	return(bad ? 0 : 1);
}


int
spell_checker_shutdown() {
	int status;

	signal(SIGCHLD, SIG_DFL);
	fclose(to_spell_checker_st);
	fclose(from_spell_checker_st);
	waitpid(spell_checker_pid, &status, 0);
	if (status & 0x0000ffff) {
		fprintf(stderr, "spell checker exited with status %d\n",
				(0x0000ff00 & status));
		return(0);
	}
	spell_checker_pid = -1;
	return(1);
}
