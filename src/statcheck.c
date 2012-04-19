/*
 * $Id: statcheck.c,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $	 
 *
 * Statistical Spell Checker Prototype
 * Eric Stofsky et al.
 * File: statcheck.c (statistics/word generator for simple 
 *                    device mappings)
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

/* This file contains the routines to create and maintain all of the 
 * statistical word structures, start the ispell(1) process, set up
 * the static confusion matrix, and perform the optional analysis of
 * ispell(1) generated words. Many of the routines dump to the log
 * file if the [-L] log file option is chosen.  An external ispell(1)
 * process is started to verify that only words that are included in
 * the user specified lexicon are offered as near miss selections.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#ifdef STDC_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#endif /* STDC_HEADERS */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include "copyright.h"
#include "statcheck.h"
#include "intelligent_nonalpha_handler.h"
#include "ocrgen.h"
#include "ispell_emulate.h"
#include "stats.h"
#include "normal.h"

static char rcsid[] = "$Id: statcheck.c,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $";

NEAR_MISS_INFO *near_miss_letters;
int spell_checker_pid;
FILE *to_spell_checker_st, *from_spell_checker_st;
normalization_stats stat_norm_st;

static void insert(stat_link *, char *, float, int);
static stat_link search_replace(char *, float, stat_link, int);
static RETSIGTYPE handle_spell_checker_sigchld();
static int enter_choices_mode();
static int enter_binary_mode();

int analyze_word_list(char *, char ***, stat_link *);
long int rate_the_choices_wrongletter(char *, char ***, stat_link *, int);
long int rate_the_choices_extra_insertion(char *,char ***, stat_link *, int);
long int rate_the_choices_missingletter (char *, char ***, stat_link *, int);
long int ocrstringreplacement_generator(char *, char ***, stat_link *, int);



void
print_the_stat_list(head)
stat_link head;
/*
 * dump out words with their respective stats
 */
{
     if (head == NULL){
	  fprintf(stderr,"\n***********************************\n");
     }
     else
     {
	  fprintf(stderr,"\n%s\t %f", head ->word, head->chance);
	  print_the_stat_list(head->next);
     }
}



static void
insert(head,entrie,the_chance,the_hash)
stat_link *head;
char *entrie;
float the_chance;
int the_hash;
/*
 * insert words/stats into list, set hashing to -1
 */
{
     stat_link temp;
     if (!(temp = 
	   (stat_link) malloc (sizeof(statistics)))){
	  perror(NULL);
	  exit(1);
     }
     temp -> chance = the_chance;
     temp -> hash = -1; 
     if (!(temp -> word = 
	   (char *) malloc (strlen(entrie) + 1))){
	  perror(NULL);
	  exit(1);
     }
     strcpy(temp->word,entrie);
     temp -> next = *head;
     *head = temp;
}



static stat_link
search_replace(key,stat,head,the_hash)
char *key;
float stat;
stat_link head;
int the_hash;
/*
 * check if current word is in the list, if not add it
 * if current word is on the list but has a better statistical
 * chance of being the correct word, change the statistic
 */
{
     while (head != NULL){
	  if (!(strcmp(head->word,key))){
	       if (head ->chance < stat){
		    head -> chance = stat;
		    head -> hash = the_hash;
	       }
	       return(head);
	  }
	  else
	  {
	       head = (head -> next);
	  }
     }
     return(0);
}



int
stat_count(head)
stat_link head;
/*
 * recursively counts the number of entries in the list
 */
{
     if (head == NULL){
	  return(0);
     }
     else
     {
	  return(1 + stat_count(head -> next));
     }
}



static RETSIGTYPE
handle_spell_checker_sigchld()
/*
 * spell checker error handling stuff
 */
{
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
spell_checker_shutdown()
/*
 * shutdown external spell checking process
 */
{
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




int
spell_checker_startup(ispell_path, hash_dict_path)
char *ispell_path;
char *hash_dict_path;
/*
 * start-up external spelling process
 */
{
     /*
      * ispell_path:     the path to the ispell(1) executable
      * hash_dict_path:  if non-NULL, ispell is passed the
      *   -d option followed by this path.  it specifies
      *   an alternate hashed dictionary.  note if hash_dict_path
      *   does not contain a '/', the library directory for the
      *   default dictionary file is prefixed.
      */
     int to_spell_checker_fd[2], from_spell_checker_fd[2];
     char buf[BUFSIZ];
     
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
	  /*
	   * If there is an alternate hashed dictionary then
	   * include it in the ispell command line
	   */
	  if (hash_dict_path) {
	       execl(ispell_path, "ispell", "-a",
		     "-d", hash_dict_path, (char *)0);
	  } else {
	       execl(ispell_path, "ispell", "-a", (char *)0);
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
     fgets(buf, BUFSIZ, from_spell_checker_st);
     
     return(1);
}



int 
read_stat_list(error_count)
int *error_count;
{
/* 
 * Opens and reads the static frequency file.  The default frequency
 * file can be changed by changing OCR_STATIC_FREQFILE in config.h
 * if the -f flag command line option is selected that becomes the
 * static frequency file for the current session.
 * Read the ocr.freq stat file into the global data structure
*/
     static FILE *stats;            /*file containing error statistics*/
     float median;
     int x;
     char correct[MAX_CHAR_REPLACEMENT];
     char generated[MAX_CHAR_REPLACEMENT];
     char buf[BUFSIZE];
     char real_holder[MAX_WORD_CHAR];
     char *p,*q;
     int cor_length,gen_length;
     int near_miss_hash = 0;
     int no_errors;

     stat_norm_st.table_median = 0;
     if ((stats = fopen (cfg.ocr_static_freqfile, "r")) == NULL){
	  printf("Can not open ocr.freq file:%s \n",cfg.ocr_static_freqfile);
	  exit(1);
     }
     
     fscanf(stats,"%d\n",&no_errors);
     *error_count = no_errors;

     if (!(near_miss_letters = (NEAR_MISS_INFO *) 
	   calloc(no_errors, sizeof(NEAR_MISS_INFO)))){
	  perror(NULL);
	  exit(1);
     }


     /* parse numeric (real) stat */
     while ((fgets(buf,BUFSIZE,stats) != NULL) && (*buf != '*')){
	  x = 0;
	  while (isdigit(buf[x]) || buf[x] == '.'){
	       real_holder[x] = buf[x];
	       ++x;
	  }
	  
	  real_holder[x] = '\0';
	  median = atof(real_holder); /* convert to float */
	  stat_norm_st.table_median = stat_norm_st.table_median + median;
	  
	  /* parse the correct string */
	  p = strchr(buf,'{');
	  p++;
	  cor_length = 0;
	  while (*p != '}'){
	       correct[cor_length] = *p;
	       ++cor_length;
	       ++p;
	  }
	  correct[(cor_length)] = '\0';

	  /* parse the generated string */
	  q = strchr(p,'{');
	  gen_length = 0;
	  ++q;
	  while (*q != '}'){
	       generated[gen_length] = *q;
	       ++gen_length;
	       ++q;
	  }
	  generated[(gen_length)] = '\0';

	  if (!(near_miss_letters[near_miss_hash].correct_char =
		(char *)malloc(MAX_CHAR_REPLACEMENT+1))){
	       perror(NULL);
	       exit(1);
	  }

	  if (!(near_miss_letters[near_miss_hash].generated_char =
		(char *)malloc(MAX_CHAR_REPLACEMENT +1))){
	       perror(NULL);
	       exit(1);
	  }

	  /*
	   * copy information into the near_miss_letters structure
	   */
	  near_miss_letters[near_miss_hash].median_isolated = median;
	  strcpy(near_miss_letters[near_miss_hash].correct_char,correct);
	  strcpy(near_miss_letters[near_miss_hash].generated_char,generated);
	  near_miss_letters[near_miss_hash].correct_length = cor_length;
	  near_miss_letters[near_miss_hash].gener_length = gen_length;
	  
	  ++near_miss_hash;
	  buf[0]= '\0';
     }
     return(0);
}



int
add_new_word_for_session(add_word)
char *add_word;
/* adds add_word to the current dictionary for the rest
 * of the session
*/
{
     fprintf(to_spell_checker_st, "%s\n", add_word);
     fflush(to_spell_checker_st);
     return(1);
}


     
static int
enter_choices_mode(void)
/*
 * have ispell generate choices
 */
{
#ifdef ISPELL_MODE_CHANGE
     fprintf(to_spell_checker_st, "<\n"); 
#endif /* !SLUMOS */

     fflush(to_spell_checker_st);
     return(1);
}



static int
enter_binary_mode(void)
/*
 * just want to know if the word is in the dictionary
 * evoke a yes or no response from ispell
 */
{
#ifdef ISPELL_MODE_CHANGE
     fprintf(to_spell_checker_st, ">\n");
#endif /* !SLUMOS */

     fflush(to_spell_checker_st);
     return(1);
}

void discard_end_of_matches_newline() {
     char discardbuf[2]; // expecting "\n\0"
     if (!fgets(discardbuf, 2, from_spell_checker_st)) {
          perror("error reading ispell end-of-matches newline");
          exit(-1);
     } else if (discardbuf[0] != '\n') {
          fprintf(stderr, "error reading ispell end-of-matches newline: expected \\n, got %x\n", discardbuf[0]);
          exit(-1);
     }
}


int
analyze_word_list(word, near_misses,head_stat)
char *word;
char ***near_misses;
stat_link *head_stat;
/*
 * send ispell the word and perform a cursory statistical
 * analysis on the output, try to weed out proper nouns
 * and acronyms
*/
{
     char buf[BUFSIZE], *p, *q, *r, *near_miss_start;
     char t;
     int misspelled = 0;
     int num_near_misses, i, num_miss;
     extern void dump_intro(void);

     /* send word to spell correction program */

     /* want nearmisses for original word */
     enter_choices_mode();

     fprintf(to_spell_checker_st, "%s\n", word);
     fflush(to_spell_checker_st);

     /* 
      * Just want binary responses for the rest of the
      * time
      */
     enter_binary_mode();

     buf[0] = '\0';

     if (near_misses){
	  *near_misses = NULL;
     }
     
     if (cfg.log_file_p){
	  fprintf(cfg.log_file,"\n The word is %s\n", word);
     }
     /*
      * Parse ispell's(1) output to see if the current word is
      * correct.  Also evaluate ispell's selection of words if the
      * ISPELL_EVALUATION is on.
      * +,-,* : indicates the word is correct
      * #, & : indictes the word is incorrect
      * In the case of &:
      * parse the word choice list and statistically evaluate
      * them
      */

     for (;;){

	  if (!fgets(buf, BUFSIZE, from_spell_checker_st)) {
	       fprintf(stderr, "error reading word list\n");
	       return(-1);
	  }

	  discard_end_of_matches_newline();

	  if (buf[0] == '#'){

	       /*
		* The word is a misspelling and no ispell near misses
		* could be generated
		*/
	       ++a_word.NO_ISPELL;
	       return(0);

	  } else if (((buf[0] == '*') || (buf[0] =='-')) || (buf[0] == '+')){

	       /*
		* The word or its derivative occur in the user selected
		* lexicon
		*/
	       if (cfg.interactive_prompt){
		    dump_intro();
	       }
	       printf("*\n");
	       ++a_word.correct_spelling;
	       return(0);

	  } else if (buf[0] == '\n') {

	       /*
		* The buffer contents is not a word
		*/
	       break;

	  } else if (buf[0] == '?') {

	       /*
		* The word is a misspelling with guess(es) generated.
		* Skip the guesses since they tend not to be device
		* mapping induced errors
		*/
	       misspelled++;
	       ++a_word.NO_ISPELL;
	       return(0);

	  } else if (buf[0] == '&') {

	       /*
		* parse the misspellings and evaluate
		*/
	       misspelled++;
	       if (cfg.log_file_p){
		    fprintf(cfg.log_file,"\n ---> %c\n",buf[0]);
	       }
	       if (misspelled ==1 ) {
		    p = strchr(buf+2, ' ');
		    p++;
		    q = strchr(p, ' ');
		    *q = '\0';
		    num_near_misses = atoi(p);
		    *q = ' ';
		    p = strchr(q, ':');
		    p += 2;
		    near_miss_start = p;
		    
		    if (cfg.log_file_p){
			 fprintf(cfg.log_file,"\n %d near misses",
				 num_near_misses);
			 fprintf(cfg.log_file,"\n %s \n",p);
		    }
				
		    for (i=0; misspelled && (i<num_near_misses); i++) {
			 q = strpbrk(p, ",\n");
			 t = *q;
			 *q = '\0';
			 if (!strcasecmp(p, word)) {
			      misspelled = 0;
			      ++a_word.correct_spelling;
			      if (cfg.interactive_prompt){
				   dump_intro();
			      }
			      /*
			       * uncomment for acronym detection
			       * kind of primitive, but it's a start:
			       * printf("* [acronym]\n");
			       */
			      printf("*\n");
			      return(0);
			 }
			 *q = t;
			 p = q+2;
		    }
		    
		    if (!misspelled || !near_misses) {
			 continue;
		    }
		    
		    if (!(*near_misses = (char **)malloc((num_near_misses + 1)
							 * sizeof(char *)))) {
			 perror(NULL);
			 exit(1);
		    }

		    p = near_miss_start;
		    for (i=0,num_miss=0; i < num_near_misses; i++) {
			 
			 q = strpbrk(p, ",\n");
			 *q = '\0';
			 
			 if (!(r = strchr(p, ' '))) {
			      
			      if (!((*near_misses)[num_miss] =
				    (char *)malloc(strlen(p)+1))) {
				   perror(NULL);
				   exit(1);
			      }

			      strcpy((*near_misses)[num_miss], p);
			      if (cfg.log_file_p){
				   fprintf(cfg.log_file,"\n%s",p);
			      }

			      /*
			       * insert all of ispell's near misses
			       */
			      insert(head_stat,p,0,-1);
			      num_miss++;
			 }
			 p = q+2;
		    }
		    (*near_misses)[num_miss] = NULL;
			break;
	       }
	  }
     }
     return(misspelled ? 0:1);
}



void 
dump_the_table(word,strg)
char *word;
char ***strg;
/*
 * dump statistical word structure (for -p mode)
 */
{
     int i= 0;
     fprintf(cfg.log_file,"\n*************************\n");
     while ((*strg)[i] != NULL){
	  fprintf(cfg.log_file,"\n%s\n", (*strg)[i]);
	  ++i;
     }
     fprintf(cfg.log_file,"\n*************************\n");
     
}


float 
freq(fast,index)
int fast,index;
{
/* 
 * perform statistical analysis on the string substitution
 */
     float median = 0;
     char *sub_sequence;

     if (fast && cfg.fast_stats_mode){
	  return(near_miss_letters[index].median_isolated
		 /stat_norm_st.normal_value);
     }

     if (!(sub_sequence = 
	   (char *) malloc (strlen 
			    (near_miss_letters[index].correct_char) + 1))){
	  perror(NULL);
	  exit(1);
     }

     strcpy(sub_sequence,near_miss_letters[index].correct_char);
     /*
      * find the ranking of the confusion 
      */
     median = near_miss_letters[index].median_isolated;
     /* 
      * return the normalized statistic
      */
     median = (near_miss_letters[index].median_isolated
	      /stat_norm_st.normal_value
	    * normalized_sub_value(sub_sequence)/stat_norm_st.table_median);
     
     free(sub_sequence);
     return(median);
}



long int
ocrstringreplacement_generator(word,choice,head_stat,no_ocr_errors)
char *word, ***choice;stat_link *head_stat;int no_ocr_errors;
/*
 * statistically checks for common ocr-string replacement
 * errors
 */
{
     int i,length;
     int j;
     int parse;
     char *p, *q, *r;
     int diff;
     char new_word[MAX_WORD_CHAR];
     char suffix[MAX_WORD_CHAR];
     char buf[BUFSIZE];
     int near_miss_hash = no_ocr_errors;
     length = 1; 
          
     (void)strcpy(new_word,word);
     
     for(i=0;i<length;i++){
	  
	  for(j = 0; j<near_miss_hash;++j){

	       if ((near_miss_letters[j].gener_length > 1)||
		   (near_miss_letters[j].correct_length > 1)){
		    
		    r = strstr(new_word, near_miss_letters[j].generated_char);
		    
		
		    if (r != NULL){
			 if (near_miss_letters[j].gener_length
			     < near_miss_letters[j].correct_length)
			 {
			      diff = (near_miss_letters[j].correct_length - 
				      near_miss_letters[j].gener_length);
			      q = new_word + strlen(new_word);
			      *q = '\0';
			      p = r + near_miss_letters[j].gener_length;
			      strcpy(suffix,p);
			      for (parse=0;
				   parse<near_miss_letters[j].correct_length;
				   ++parse)
			      {
				   *r = near_miss_letters[j].correct_char[parse];
				   ++r;
			      }
			      *r = '\0';
			      strcat(new_word,suffix);
			 }

			 if (near_miss_letters[j].gener_length >=
			     near_miss_letters[j].correct_length)
			 {
			      diff = (near_miss_letters[j].gener_length -
				      near_miss_letters[j].correct_length);
			      q = new_word + strlen(new_word);
			      *q = '\0';
			      p = r + near_miss_letters[j].gener_length;
			      strcpy(suffix,p);
			      for (parse=0;
				   parse<near_miss_letters[j].correct_length;
				   ++parse)
			      {
				   *r = near_miss_letters[j].correct_char[parse];
				   ++r;
			      }
			      *r = '\0';
			      strcat(new_word,suffix);
			 }

			 fprintf(to_spell_checker_st, "%s\n", new_word);
			 fflush(to_spell_checker_st);
			 
			 if (!fgets(buf, BUFSIZE, from_spell_checker_st)) {
			      fprintf(stderr, "error reading word list\n");
			      return(-1);}


			 if (buf[0] == '*'){
			      /*
			       * the word is correct
			       */
			      if (cfg.log_file_p){
				   fprintf(cfg.log_file,
					   "\n**NON ISPELL WORD GENERATED**");
				   fprintf(cfg.log_file,
					   " %s to %s has a rating of ...",word,new_word);
				   fprintf(cfg.log_file,"%f\n",freq(0,j));
			      }

			      /*
			       * Check to see if the word is already in the
			       * near misses list. If it is and has a lower
			       * statistic associated with it, change it to
			       * the new value
			       */
			      if (search_replace(new_word,freq(0,j),
						 *head_stat,j)==NULL){
				   insert(head_stat,new_word,0,j);
				   search_replace(new_word,freq(0,j),
						  *head_stat,j);
			      }
			 }
			 discard_end_of_matches_newline();
			      strcpy(new_word,word);
				
		    }
	       }
	  }
     }
     return(0);
}



long int
rate_the_choices_wrongletter(word,choice,head_stat,no_ocr_errors)
char *word, ***choice;stat_link *head_stat;int no_ocr_errors;
/*
 * statistically checks for common ocr-characters replacement
 * errors
 */
{
     int i, length;
     int j;
     int jump = 0;
     int x = 0;
     char newword[MAX_WORD_CHAR];
     int near_miss_hash = no_ocr_errors;
     while ((*choice)[x] != NULL){
	  length = strlen (word);
	  
	  (void) strcpy (newword, word);
	  
	  for (i = 0; i < length; i++)
	  {
	       for (j = 0; j < near_miss_hash; j++)
	       {
		    if ((newword[i]==near_miss_letters[j].generated_char[0])
			&& ((near_miss_letters[j].gener_length == 1) &&
			    (near_miss_letters[j].correct_length == 1))){     
			 newword[i] = near_miss_letters[j].correct_char[0];
			 
			 if (!(strcmp(newword,(*choice)[x]))){
			      if (cfg.log_file_p){
				   fprintf(cfg.log_file,
					   "\n %s to %s has a rating of...",
					   word,(*choice)[x]);
				   fprintf(cfg.log_file,"%f",freq(i,j));
			      }
			      /*
			       * Check to see if the word is already in the
			       * near misses list. If it is and has a lower
			       * statistic associated with it, change it to
			       * the new value
			       */
			      search_replace(newword,freq(i,j),*head_stat,j);
			      ++jump;
			      break;
			 }
			 newword[i] = word[i];
		    }
	       }
	  }
	  
	  ++x;
     }
     return(0);
}



long int
rate_the_choices_extra_insertion(word,choice,head_stat,no_ocr_errors)
char *word,***choice;stat_link *head_stat;int no_ocr_errors;
/*
 * statistically checks for common ocr-characters insertion
 * errors
 */
{
     char *p, *q, *r;
     int i;
     char new_word[MAX_WORD_CHAR];
     char prev_char,skip_char;/* For future use */
     int length;
     int x = 0;
     int near_miss_hash = no_ocr_errors;
     while ((*choice)[x] != NULL){
	  length = strlen (word);
	  
	  if (length > 2){
	       
	       for (p = word; *p; p++)
	       {
		    for (q = word, r = new_word; *q; q++)
		    {
			 if (q != p)
			      *r++ = *q;
		    }
		    prev_char = *p;
		    skip_char = *r;
		    *r = '\0';
		    if (!(strcmp(new_word,(*choice)[x])))
		    {
			 for (i=0;i<near_miss_hash;++i){
			      if ((near_miss_letters[i].correct_length == 0) &&
				  ((near_miss_letters[i].gener_length == 1) &&
				   (near_miss_letters[i].generated_char[0]
				    == skip_char))){
				   if (cfg.log_file_p){
					fprintf(cfg.log_file,
						"\n %s to %s has a rating of...",word,(*choice)[x]);
					fprintf(cfg.log_file,"%f\n",freq(0,i));
				   }
				   /*
				    * Check to see if the word 
				    * is already in the near misses list.
				    * If it is and has a lower statistic 
				    * associated with it, change it to
				    * the new value
				    */
				   search_replace(new_word,freq(0,i),
						  *head_stat,i);
			      }
			 }
		    }
	       }
	  }
	  ++x;
     }
     return(0);
}



long int
rate_the_choices_missingletter (word,choice,head_stat,no_ocr_errors)
char *word, ***choice;stat_link *head_stat;int no_ocr_errors;
/*
 * statistically checks for common ocr-character ommision
 * errors
 */
{
     char *p, *r, *s, *t;
     int length;
     int i;
     int x = 0;
     char newword[MAX_WORD_CHAR];
     int near_miss_hash = no_ocr_errors;
     while ((*choice)[x] != NULL){
	  length = strlen (word);
	  
	  for (p = word; p == word || p[-1]; p++)
	  {
	       
	       
	       for (s = newword, t = word; t != p; s++, t++)
		    *s = *t;
	       r = s++;
	       while (*t)
		    *s++ = *t++;
	       *s = 0;
	       
	       for (i = 0; i < near_miss_hash; i++)
	       {
		    if ((near_miss_letters[i].correct_length == 1) &&
			(near_miss_letters[i].gener_length == 0))
		    {
			 *r = near_miss_letters[i].correct_char[0];
			 if (!(strcmp(newword,(*choice)[x]))){
			      if (cfg.log_file_p){
				   fprintf(cfg.log_file,"\n !%s to %s has a rating of...",word,(*choice)[x]);
				   fprintf(cfg.log_file,"%f",freq(0,i));
			      }
			      /*
			       * Check to see if the word is already in the
			       * near misses list. If it is and has a lower
			       * statistic associated with it, change it to
			       * the new value
			       */
			      search_replace(newword,freq(0,i),*head_stat,i);
			 }
		    }
	       }
	  }
	  ++x;
     }
     return(0);
}




