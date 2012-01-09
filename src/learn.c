/*
/* $Id: learn.c,v 1.1.1.1 2000/08/28 21:11:37 slumos Exp $
 *
 * Statistical Spell Checker Prototype
 * Eric Stofsky
 * file: learn.c (statistic learning prototype)
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
 * This module creates new statistics by analyzing the user selection
 * of replacement words.  Adjusted statistics are then stored in the
 * user defined learned database
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef STDC_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif /* STDC_HEADERS */

#include "learn.h"
#include "statcheck.h"
#include "lcs2cnf.h"
#include "addaptive_stats.h"
#include "ocrgen.h"
#include "lcs2cnf.h"

static char rcsid[] = "$Id: learn.c,v 1.1.1.1 2000/08/28 21:11:37 slumos Exp $";

static void prompt_for_correct_choice();



void
add_another_confusion(generated,correct,current_no_mappings)
char *generated,*correct;int *current_no_mappings;
{
   /*
    * Update the device mapping statistics table with the new
    * results derived from the -l option.
    * There are two possibilities:
    * (1) increment a statistic which already exists
    * (2) add a new mapping to the table
    * default incrementing constants can be modified in
    * addaptive_stats.h
    */

   int i;
   for(i=0;i<*current_no_mappings;++i){
	if (!strcmp(near_miss_letters[i].correct_char,correct) &&
	    !strcmp(near_miss_letters[i].generated_char,generated)){
	     addaptive_stats_update(i);
	     return;
	}
   }
   
   if (!(near_miss_letters = (NEAR_MISS_INFO *)
	 realloc(near_miss_letters,
		 (*current_no_mappings + 1) * sizeof(NEAR_MISS_INFO)))){
	perror(NULL);
	exit(1);
   }
         
   if (!(near_miss_letters[*current_no_mappings].correct_char =
	(char *) malloc (strlen(correct) + 1))){
	perror(NULL);
	exit(1);
   }
   if (!(near_miss_letters[*current_no_mappings].generated_char = 
	(char *) malloc (strlen(generated) + 1))){
	perror(NULL);
   }

   near_miss_letters[*current_no_mappings].median_isolated = 1;
   strcpy(near_miss_letters[*current_no_mappings].correct_char,correct);
   strcpy(near_miss_letters[*current_no_mappings].generated_char,generated);
   near_miss_letters[*current_no_mappings].correct_length = strlen(correct);
   near_miss_letters[*current_no_mappings].gener_length = strlen(generated); 
   ++(*current_no_mappings);
}
   

void
prompt_for_correct_choice(original_word)
char *original_word;
{
     fprintf(stderr,"The original word is : %s\n",original_word);
     fprintf(stderr,"\nEnter the correct word [s to save database] >");
}

          
void
interactive_training(original_word,no_ocr_errors)
char *original_word;int *no_ocr_errors;
/*
 * Interactive statistical trainer
 * (1) Prompts user for correct word
 * (2) Determines the longest common substring from the
 *     correct word to the original word
 * (3) Updates the statistics accordingly
 */
{
     char correct_word[INPUT_WORD_LENGTH];
     prompt_for_correct_choice(original_word);
     scanf("%s",correct_word);
     if (!strcmp(correct_word,"save") || !strcmp(correct_word,"s")){
	  addaptive_file_dump(cfg.learn_database,*no_ocr_errors);
     }
     
     find_confusions(correct_word,original_word,no_ocr_errors);
}







