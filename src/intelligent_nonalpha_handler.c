/*
 * $Id: intelligent_nonalpha_handler.c,v 1.1.1.1 2000/08/28 21:11:37 slumos Exp $	 
 *
 * Statistical Spell Checker Prototype
 * Eric Stofsky
 * file: intelligent_nonalpha_handler.c (intelligent punctuation &
 *                                       number handeling)
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
 * This module contains the routines used by ocrspell to determine what
 * part of a string is a word (word isolation).
 * A string may contain alphabet characters, numbers, and/or punctation.
 * A rule based system is used to isolate what part of the
 * string is the word to be queried as a misspelling. 
 * This module determines what is punctuation/numbers and what 
 * is the word to be corrected.  There are four posibilities:
 * (1) The whole string is a mispelled word
 * (2) Some possibly empty sequence of the left non-character string
 *     is punctuation and should not be included in the query
 * (3) Some possibly empty sequence of the right non-character string
 *     is punctuation and should not be included in the query
 * (4) A combination of (2) and (3)
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef STDC_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#endif /* STDC_HEADERS */

#include "copyright.h"
#include "statcheck.h"
#include "intelligent_nonalpha_handler.h"
#include "ocrgen.h"

static char rcsid[] = "$Id: intelligent_nonalpha_handler.c,v 1.1.1.1 2000/08/28 21:11:37 slumos Exp $";

static void process_punctuation(int);
static void initialize_word_structure(int *,char *);
int in_punction_handler(char *, int *, int);
static void test_boundary();


static void
initialize_word_structure(justification,suffix_punct)
int *justification;char *suffix_punct;
{
/*
 * initialize the word structure
*/
     a_word.NO_ISPELL = 0;
     a_word.correct_spelling = 0;
     a_word.pure_character_word = 1;
     free(a_word.original_word);
     free(a_word.left_punctuation);
     free(a_word.right_punctuation);
     if (!(a_word.left_punctuation = (char *)malloc(1))){
	  perror(NULL);
	  exit(1);
     }
     a_word.left_punctuation[0] = '\0';
     if (!(a_word.right_punctuation = (char *)malloc(1))){
	  perror(NULL);
	  exit(1);
     }
     a_word.right_punctuation[0] = '\0';
     free(a_word.word_to_check);
     if (!(suffix_punct = (char *) malloc(1))){
	  perror(NULL);
	  exit(1);}
     suffix_punct[0] = '\0';
     *justification = 0;
}


static void
process_punctuation(no_ocr_errors)
int no_ocr_errors;
{
/*
 * determine the word boundary
*/
     int j;
     char *l,*r,*m;
     
     if ((!strlen(a_word.left_punctuation))
	 && (!strlen(a_word.right_punctuation))){
	  return;
     }

     for(j=0;j<no_ocr_errors;++j){
	  
	  l = NULL;
	  r = NULL;

	  if (strlen (near_miss_letters[j].generated_char)){

	       l = strstr(a_word.left_punctuation,
			  near_miss_letters[j].generated_char);

	       r = strstr(a_word.right_punctuation,
			  near_miss_letters[j].generated_char);
	  }

	  while (l != NULL){
	       if (!strcmp(l,near_miss_letters[j].generated_char)){
		    if (!(m = (char *) 
			  malloc (strlen(a_word.word_to_check) +
				  strlen(near_miss_letters[j].generated_char)
				  + 1))){
			 perror(NULL);
			 exit(1);
		    }

		    strcpy(m,near_miss_letters[j].generated_char);
		    strcat(m,a_word.word_to_check);
		    free(a_word.word_to_check);

		    if (!(a_word.word_to_check  =
			  (char *) malloc(strlen(m) + 1))){
			 perror(NULL);
			 exit(1);
		    }

		    strcpy(a_word.word_to_check,m);
		    free(m);
		    *l ='\0';
		    a_word.pure_character_word = 0;
		    a_word.NO_ISPELL = 1;
		    l = strstr(a_word.left_punctuation,
			       near_miss_letters[j].generated_char);
	       }
	       else {
		    l = NULL;
	       }
	  }
	  
	  while (r != NULL){
	       if (r == a_word.right_punctuation){
		    if (!(m = (char *)
			  malloc (strlen(a_word.word_to_check) +
				  strlen (near_miss_letters[j].generated_char)
				  + 1))){
			 perror(NULL);
			 exit(1);
		    }

		    strcpy(m,a_word.word_to_check);
		    strcat(m,near_miss_letters[j].generated_char);
		    if (!(a_word.word_to_check = (char *) 
			  malloc (strlen (m) + 1))){
			 perror(NULL);
			 exit(1);
		    }

		    strcpy(a_word.word_to_check,m);
		    strcpy(m,a_word.right_punctuation);
		    m = m + strlen(near_miss_letters[j].generated_char);
		    free(a_word.right_punctuation);

		    if (!(a_word.right_punctuation = (char *)
			malloc (strlen(m) + 1))){
			 perror(NULL);
			 exit(1);
		    }
		    strcpy(a_word.right_punctuation,m);
		    free(m);
		    a_word.pure_character_word = 0;
		    a_word.NO_ISPELL = 1;

		    r = strstr(a_word.right_punctuation,
			       near_miss_letters[j].generated_char);
	       }
	       else{
		    r = NULL;
	       }
	       
	  }
		    
     }
     
}


int
in_punction_handler(queried_word,word_offset,no_ocr_errors)
char *queried_word;int *word_offset;int no_ocr_errors;
/*
 * Determines what is the actual word to be queried and 
 * replaced
*/
 
{
     char *p;
     char unprepunct_word[MAX_WORD_CHAR];
     char *punctuation;
     int number_of_apost;
     int Q_NO;          /*right justify the word*/
     char *after_punct; /*string of punctuation which occurs at the
			  end of the word*/

     initialize_word_structure(&Q_NO,after_punct);
     
     if (!(a_word.original_word = 
	   (char *) malloc (strlen(queried_word)+ 1))){
	  perror(NULL);
	  exit(1);
     }

     strcpy(a_word.original_word,queried_word);
     
     /* cut ending punctuation */
     p = (queried_word + strlen(queried_word) - 1);
     
     while (((!isalpha(*p)) && (p > queried_word)) && (*p != '~')){
	  --p;
     }

     if ((p ==queried_word) && (!isalpha(*p))){
	  a_word.NO_ISPELL = 1;
	  a_word.pure_character_word = 0;
	  if (!(a_word.word_to_check =
		(char *)malloc(strlen(queried_word) + 1))){
	       perror(NULL);
	       exit(1);
	  }
	  strcpy(a_word.word_to_check,queried_word);
     }

     if ((p!=queried_word) || ((p == queried_word)
			       && ((isalpha(*p)) || (*p == '~')))){
	  ++p;
	  punctuation = p;
	  if (!(after_punct = 
		(char *) malloc(strlen(punctuation) + 1))){
	       perror(NULL);
	       exit(1);
	  }
	  strcpy(after_punct,punctuation);
	  if (!(a_word.right_punctuation = 
		(char *) malloc(strlen(punctuation) + 1))){
	       perror(NULL);
	       exit(1);
	  }
	  strcpy(a_word.right_punctuation,punctuation);
	  *p = '\0';
     }

     else
     {
	*a_word.right_punctuation = '\0' ; 
   }
     a_word.dictionary_insertion = 0;
     /* cut beggining punctuation*/
     p = queried_word;
     if (*p == '@'){
	  add_new_word_for_session(queried_word);
	  a_word.correct_spelling = 1;
	  a_word.dictionary_insertion = 1;
     }

     Q_NO = 0;

     while (!((isalpha(*p) || (*p == '~'))) && 
	    (p <= (strlen(queried_word) + queried_word - 1))){
	  ++Q_NO;
	  ++p;
     }

     if (p != queried_word){
	  if (!(a_word.left_punctuation = 
		(char *) malloc (p - queried_word + 1))){
	       perror(NULL);
	       exit(1);}
	  *a_word.left_punctuation = '\0';
	  strncpy(a_word.left_punctuation,queried_word,(p - queried_word));
	  a_word.left_punctuation[(p - queried_word)] = '\0';
     }
     else
     {
	  *a_word.left_punctuation = '\0';
    } 
     if (Q_NO == strlen(queried_word)){
	  Q_NO = 0;
     }
     else{
	  strcpy(unprepunct_word,p);
	  strcpy(queried_word,unprepunct_word);
	  if (!(a_word.word_to_check = 
		(char *) malloc (strlen (queried_word) + 1))){
	       perror(NULL);
	       exit(1);
	  }
	  strcpy(a_word.word_to_check,queried_word);
     }
     a_word.pure_character_word = 1;
          
     number_of_apost = 0;
     a_word.apost = 0;

     for (p = queried_word; *p; p++) {
	  if (!isalnum(*p) && (*p != '-') && (*p != '\'') && (*p != '/')) {
	       if (strlen(queried_word) == 1){
		    a_word.correct_spelling = 1;
	       }
	       
	  }
	  
	  if (!isalpha(*p)){
	       if (*p == '\''){
		    ++number_of_apost;
		    ++ a_word.apost;
		    if ((number_of_apost > 1) || (strlen(queried_word) == 1)){
			 a_word.pure_character_word = 0;
		    }
	       }
	       else {
		    a_word.pure_character_word = 0;
	       }
	       
	  }
     }
     
     if (!strcmp(queried_word, LINE_BREAK)){
	  a_word.correct_spelling = 1;
	  printf("\n");
     }

      if (!a_word.pure_character_word){
	   a_word.NO_ISPELL = 1;
      }
     
     if ((strcmp(a_word.original_word,a_word.word_to_check) &&
	  (!a_word.dictionary_insertion)) && (cfg.intelligent_word)){
	  process_punctuation(no_ocr_errors);
     }

     *word_offset = Q_NO;
     return(1);
}


static void
test_boundary(void){
/*
 * Use for debugging word boundary stuff
 */
     fprintf(stderr,"\n*******************\n");
     fprintf(stderr,"\nPure word = %d\n", a_word.pure_character_word);
     fprintf(stderr,"\nDictionary_insertion = %d\n", 
	     a_word.dictionary_insertion);
     fprintf(stderr,"\nOriginal Words = %s\n", a_word.original_word);
     fprintf(stderr,"\nleft punctuation = %s\n", a_word.left_punctuation);
     fprintf(stderr,"\nright punctuation = %s\n", a_word.right_punctuation);
     fprintf(stderr,"\napostrophe = %d\n", a_word.apost);
     fprintf(stderr,"\nCorrect Spelling = %d\n", a_word.correct_spelling);
     fprintf(stderr,"\nWord to check = %s \n", a_word.word_to_check);
     fprintf(stderr,"\nNO Activate Ispell = %d\n", a_word.NO_ISPELL);
     
}
	    




