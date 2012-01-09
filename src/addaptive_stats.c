/*
 * $Id: addaptive_stats.c,v 1.1.1.1 2000/08/28 21:11:37 slumos Exp $
 *
 * Statistical Spell Checker Prototype
 * Eric Stofsky
 * file: addaptive_stats.c (learning statistical generator)
*/

/* Ocrspell OCR-based statistical spell checker
 * Copyright (C) 1995 Regents of the University of Nevada
 * Text Retrieval Group
 * Information Science Research Institute
 * University of Nevada, Las Vegas
 * Las Vegas, NV 89154-4021
 * isri-text@isri.unlv.edu.
 *
*/


/*
 * Routines to update the statistics if in learning mode. Statistics
 * will be incremented or decremented depending on the user's selection
 * of the correct word in learning mode.  The result will be placed in
 * the learn database which is specified on the command line after the
 * -l flag.  This is a prototype for hard OCR device mappings generation;
 * typically, the dynamic confusion generator of the emacs lisp interface
 * will suffice for new statistic generation.  The learned ocr error
 * frequency database can be imported as the default ocr frequency file.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef STDC_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#endif /* STDC_HEADERS */

#ifdef HAVE_UNISTD
#include <unistd.h>
#endif

#include "statcheck.h"
#include "addaptive_stats.h"

static char rcsid[] = "$Id: addaptive_stats.c,v 1.1.1.1 2000/08/28 21:11:37 slumos Exp $";

static int substring_proc_pid;
static FILE *to_substring_st, *from_substring_st;
static RETSIGTYPE handle_substring_proc_sigchld();
static int substring_shutdown();
static int substring_startup(char *);


static RETSIGTYPE
handle_substring_proc_sigchld()
/*
 * substring generator error handling stuff
 */
{
        /*
         * if we get a SIGCHLD, the substring generator exited.  if it 
	 * exited zero, just wait on it and that's it.  if nonzero, 
	 * spit out an error message and exit.
         */
     int status;
     waitpid(substring_proc_pid, &status, 0);
     if (status & 0x0000ffff) {
	  fprintf(stderr, 
		  "substring generator exited prematurely with status %d\n",
		  (0x0000ff00 & status));
	  exit(1);
     }
}



static int
substring_shutdown()
/*
 * shutdown external substring process
 */
{
     int status;
     
     signal(SIGCHLD, SIG_DFL);
     fclose(to_substring_st);
     fclose(from_substring_st);
     waitpid(substring_proc_pid, &status, 0);
     if (status & 0x0000ffff) {
	  fprintf(stderr, "substring generator exited with status %d\n",
		  (0x0000ff00 & status));
	  return(0);
     }
     substring_proc_pid = -1;
     return(1);
}



static int
substring_startup(substring_path)
char *substring_path;
/*
 * start-up external substring process
 */
{
     int to_substring_fd[2], from_substring_fd[2];
     
     if ((pipe(to_substring_fd) == -1) ||
	 (pipe(from_substring_fd) == -1)) {
	  perror("pipe");
	  return(0);
     }
     
     signal(SIGCHLD,handle_substring_proc_sigchld);
     
     /* create substring process */
     substring_proc_pid = fork();
     if (substring_proc_pid < 0) {
	  perror("fork");
	  return(0);  
     } else if (substring_proc_pid == 0) {
	  /* close the ends of the pipes the substring proc doesn't need */
	  close(to_substring_fd[1]);
	  close(from_substring_fd[0]);
	  
	  if ((dup2(to_substring_fd[0], 0) == -1) ||
	      (dup2(from_substring_fd[1], 1) == -1)) {
	       perror("dup2");
	       exit(1);
	  }
	  close(to_substring_fd[0]);
	  close(from_substring_fd[1]);
	  execl(substring_path,"lcs2cnf",(char *)0);
	  perror(substring_path);
	  exit(1);
     }

     /* close the ends of the pipes the parent doesn't need */
     close(to_substring_fd[0]);
     close(from_substring_fd[1]);

     /* make streams out of the pipe file descriptors */
     if (!(to_substring_st = fdopen(to_substring_fd[1], "w"))) {
	  perror("fdopen");
	  close(to_substring_fd[1]);
	  close(from_substring_fd[0]);
	  return(0);
     }
     if (!(from_substring_st = fdopen(from_substring_fd[0], "r"))) {
	  perror("fdopen");
	  fclose(to_substring_st);
	  close(from_substring_fd[0]);
	  return(0);
     }
     
     return(1);
}


     
void
addaptive_stats_update(hash)
int hash;
/*
 * increment the statistics associated with the word chosen
 */
{
     near_miss_letters[hash].median_isolated = 
	  (near_miss_letters[hash].median_isolated + ADDAPT_INCREMENT);
}



void 
addaptive_stringmatch_decrement(hash,no_ocr_errors)
int hash,no_ocr_errors;
/*
 * decrement all statistics whose generated character string matches
 * that of the original misspelling, but whose substitution sequence
 * does not match that of the user chosen word
 */
{
     int index = 0;
     char *generated_string;
     if (!(generated_string = (char *) 
	   malloc (strlen (near_miss_letters[hash].generated_char) + 1))){
	  perror(NULL);
	  exit(1);
     }
     strcpy(generated_string,near_miss_letters[hash].generated_char);

     for (index=0;index<no_ocr_errors;++index){
	  if (!(strcmp(near_miss_letters[index].generated_char,
		       generated_string))){
	       if (index != hash){
		    near_miss_letters[index].median_isolated =
			 (near_miss_letters[index].median_isolated
			  - ADDAPT_DECREMENT);
	       }
	  }
     }
     free (generated_string);
}


void
addaptive_file_dump(add_file_name,no_ocr_errors)
char *add_file_name;int no_ocr_errors;
/* 
 * print out a copy of the learned database for possible future 
 * import into OCRSpell
 */
{  
     int i;
     int index;
     FILE *add_stats,*fopen(); /* learning database file pointer */

     if ((add_stats = fopen (add_file_name, "w+")) == NULL){
               printf("\n OCRSPELL can't open %s\n",add_file_name);
               exit(1);
	  }

     for (i=0;i<no_ocr_errors;++i){
	  index = near_miss_letters[i].correct_length;
	  near_miss_letters[i].correct_char[index] ='\0';
	  index = near_miss_letters[i].gener_length;
	  near_miss_letters[i].generated_char[index] ='\0';
     }

     fprintf(add_stats,"%d\n",no_ocr_errors);
     for (i=0;i<no_ocr_errors;++i){
	  fprintf(add_stats,"%-.1f\t",near_miss_letters[i].median_isolated);
	  fprintf(add_stats,"{%s}\t",near_miss_letters[i].correct_char);
	  fprintf(add_stats,"{%s}\n",near_miss_letters[i].generated_char);
     }
     fprintf(add_stats,"*****************\n");

     fclose(add_stats);
     exit(0);
}



     

  
						
