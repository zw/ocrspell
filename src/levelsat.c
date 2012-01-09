/*
 * $Id: levelsat.c,v 1.2 2001/06/15 18:40:05 slumos Exp $	 
 *
 * Statistical Spell Checker Prototype
 * Eric Stofsky
 * File: levelsat.c (multi-level statistics/word generator)
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
 * This module uses level saturation to generate words from misspellings 
 * with multiple OCR induced substring errors (set for two levels). It 
 * uses a statistically based string matching algorithm the uses device
 * mapping frequencies along with n-gram statistics pertaining to the 
 * current document set to establish a Bayesian ranking of the 
 * possibilities, or suggestions, for each misspelled word.  Compound
 * and complex device mappings are applied to all misspellings such that
 * any word that occurs in the lexicon that is two distinct device mappings
 * away will be delivered as a choice for the corresponding misspelling.
 * For example rnouiitain
 * will generate : mountain 
 * This is because {rn} -> {m} and {ii} ->{n} is the default static 
 * confusion file.
 * 
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include "copyright.h"
#include "statcheck.h"
#include "ocrgen.h"
#include "levelsat.h"
#include "spell.h"

static char rcsid[] = "$Id: levelsat.c,v 1.2 2001/06/15 18:40:05 slumos Exp $";

#define LEVEL 2 /* Number of recursive levels ocrspell will search */


static int speed_in (stat_link, char *);
static void quick_insert(stat_link *, char *, float);


static int
speed_in(head,entrie)
/*
 * This function is used exclusively by levelsat because each levels
 * probability is known to be lower than the previous substitution.
*/
stat_link head;
char *entrie;
{
     char query_word[MAX_WORD_CHAR];
     strcpy(query_word,entrie);

     while (head != NULL){
	  if (!(strcmp(head->word,query_word))){
	       return(1);
	  }
	  else 
	  {
	       head = (head -> next);
	  }
     }
     return(0);
}
     

static void
quick_insert(head,entrie,the_chance)
/*
 * This function speeds up the insertion process during the level
 * saturation process because of the properties of multiplication
 * on reals less than one
*/
stat_link *head;
char *entrie;
float the_chance;
{
     stat_link temp;
     if (speed_in(*head,entrie)){
	  return;
     }
     if (!(temp = 
	 (stat_link) malloc (sizeof(statistics)))){
	  perror(NULL);
	  exit(1);
     }
     temp -> chance = the_chance;
     temp -> hash = -1; /* Blanket hash*/
     if (!(temp -> word = 
	   (char *) malloc (strlen(entrie) + 1))){
	  perror(NULL);
	  exit(1);
     }
     strcpy(temp->word,entrie);
     temp -> next = *head;
     *head = temp;
}



long int
multilevelsaturation_generator (char *word, char ***choice, stat_link head_stat,
				int fast_analysis, int no_ocr_errors)
{
  /* performs level saturation analysis of the word, generating
   * statistics for all non-null levels */ 
          
  char *p, *q, *r, *y;
  char temp;
  int i,j;
  char new_word[MAX_WORD_CHAR];
  char suffix[MAX_WORD_CHAR];
  char prefix[MAX_WORD_CHAR];
  char new_suffix[MAX_WORD_CHAR];
  char ng_word[MAX_WORD_CHAR];
  int parse;
  char buf[BUFSIZE];
  char correct_let;
  int level_one,level_two;
  float fsub = 0.0;
  int near_miss_hash;
     
  near_miss_hash = no_ocr_errors;
      
  *new_word = '\0';
  strncat(new_word, word, sizeof(new_word));     
               
  for(j = 0; j<near_miss_hash;++j){
    *new_word = '\0';
    strncat(new_word, word, sizeof(new_word));
    r = strstr(new_word, near_miss_letters[j].generated_char);
	  
    if (r != NULL){
      q = new_word + strlen(new_word);
      *q = '\0';

      p = r + near_miss_letters[j].gener_length;
      *suffix = '\0';
      strncat(suffix, p, sizeof(suffix));

      temp = *r;
      *r = '\0';

      *prefix = '\0';
      strncat(prefix, new_word, sizeof(prefix));
      *r = temp;

      for (parse=0; parse<near_miss_letters[j].correct_length; ++parse)
      {
	*r = near_miss_letters[j].correct_char[parse];
	++r;
      }

      *r = '\0';
      strncat(new_word, suffix, sizeof(new_word)-strlen(new_word));

      *ng_word = '\0';
      strncat(ng_word, new_word, sizeof(ng_word));
      level_one = 1;

      for (y = new_word; *y; y++) {
	if (!isalpha(*y)){
	  correct_let = '\0';
	  level_one = 0;			 
	}
      }

      correct_let = '\0';

      if (level_one) {
	fseek(to_spell_checker_st,0,SEEK_END);
	fseek(from_spell_checker_st,0,SEEK_END);
	fprintf(to_spell_checker_st, "%s\n", new_word);
	fflush(to_spell_checker_st);
	       
	buf[0] = '\0';
	if (!fgets(buf, BUFSIZE, from_spell_checker_st)) {
	  fprintf(stderr, "error reading word list\n");
	  return(-1);}
	correct_let = buf[0];
		    
	fseek(from_spell_checker_st,0,SEEK_END);
      }
	       
      if ((correct_let == '*')||(correct_let == '+')){
	/*
	 * just generated a word that occurs in the current
	 * lexicon
	 */
	buf[0] = '\0';
	if (cfg.log_file_p){
	  fprintf(cfg.log_file,
		  "\n-*NON ISPELL WORD GENERATED**");
	  fprintf(cfg.log_file,
		  " %s to %s has a rating of ...",word,
		  new_word);
	  fprintf(cfg.log_file,"%f\n",freq(fast_analysis,j));
	}
		    
	quick_insert(head_stat,new_word,freq(fast_analysis,j));

		    
	strcpy(new_word,word);
      }
      else {

	fsub = freq(fast_analysis,j);/* First level substitution */

	for (i=0;i<near_miss_hash;++i){
			 
	  if ((strstr(prefix,
		      near_miss_letters[i].generated_char)
	       != NULL)
	      || (strstr(suffix,
			 near_miss_letters[i].generated_char)
		  != NULL)){
			      
			     			      
	    r = strstr(new_word, 
		       near_miss_letters[i].generated_char);
			      
			      
	    if (r != NULL){
				   
	      {
		q = new_word + strlen(new_word);
		*q = '\0';
		p = r + 
		  near_miss_letters[i].gener_length;
		strcpy(new_suffix,p);
		for (parse=0;
		     parse<near_miss_letters[i].correct_length;
		     ++parse)
		{
		  *r = near_miss_letters[i].correct_char[parse];
		  ++r;
		}
		*r = '\0';
		strcat(new_word,new_suffix);
	      }
	      level_two = 1;
	      for (y = new_word; *y; y++) {
		if (!isalpha(*y)){
		  correct_let = '\0';
		  level_two = 0;
		}
	      }
	      correct_let = '\0';
	      if (level_two){
					
		fprintf(to_spell_checker_st, 
			"%s\n", new_word);
		fflush(to_spell_checker_st);
		correct_let = '\0';
		buf[0] = '\0';
		if (!fgets(buf, BUFSIZE,
			   from_spell_checker_st)) {
		  fprintf(stderr, "error reading word list\n");
		  return(-1);}
		correct_let = buf[0];
					
					
		fseek(from_spell_checker_st,0,SEEK_END);
	      }
	      if ((correct_let == '*')||
		  (correct_let == '+')){
		/*
		 * Just generated a word that occurs
		 * in the current lexicon at level
		 * two
		 */
		if (cfg.log_file_p){
		  fprintf(cfg.log_file,"\n+*NON ISPELL WORD GENERATED**");
		  fprintf(cfg.log_file," %s to %s has a rating of ...",word,new_word);
		  fprintf(cfg.log_file,"%f\n",(freq(fast_analysis,i)*fsub));
		}
					     
		quick_insert(head_stat,new_word,
			     (freq(fast_analysis,i)*
			      fsub));
	      }
	      strcpy(new_word,ng_word);
				   
	    }
	  }
	}
		    
      }
	       	       
    }
	 
  }
  return(0);
}

#if 0
long int
multilevelsaturation_generator
(word,choice,head_stat,fast_analysis,no_ocr_errors)
/*
 * performs level saturation analysis of the word, generating
 * statistics for all non-null levels
 */ 
char *word, ***choice;stat_link *head_stat;int fast_analysis,no_ocr_errors;
{
          
     char *p, *q, *r, *y;
     char temp;
     int i,j;
     char new_word[MAX_WORD_CHAR];
     char suffix[MAX_WORD_CHAR];
     char prefix[MAX_WORD_CHAR];
     char new_suffix[MAX_WORD_CHAR];
     char ng_word[MAX_WORD_CHAR];
     int parse;
     char buf[BUFSIZE];
     char correct_let;
     int level_one,level_two;
     float fsub = 0.0;
     int near_miss_hash;
     
     near_miss_hash = no_ocr_errors;
      
     (void)strcpy(new_word,word);
     
               
     for(j = 0; j<near_miss_hash;++j){
	  
	  strcpy(new_word,word);
	  
	  r = strstr(new_word, near_miss_letters[j].generated_char);
	  
	  
	  if (r != NULL){
	       
	       
	       q = new_word + strlen(new_word);
	       *q = '\0';
	       p = r + near_miss_letters[j].gener_length;
	       strcpy(suffix,p);
	       temp = *r;
	       *r = '\0';
	       strcpy(prefix,new_word);
	       *r = temp;

	       for (parse=0;parse<near_miss_letters[j].correct_length;++parse)
	       {
		    *r = near_miss_letters[j].correct_char[parse];
		    ++r;
	       }

	       *r = '\0';
	       strcat(new_word,suffix);
	       strcpy(ng_word,new_word);
	       level_one = 1;

	       for (y = new_word; *y; y++) {
		    if (!isalpha(*y)){
			 correct_let = '\0';
			 level_one = 0;
			 
		    }
	       }

	       correct_let = '\0';
	       if (level_one){
	       	       
		    fseek(to_spell_checker_st,0,SEEK_END);
		    fseek(from_spell_checker_st,0,SEEK_END);
		    fprintf(to_spell_checker_st, "%s\n", new_word);
		    fflush(to_spell_checker_st);
	       
		    buf[0] = '\0';
		    if (!fgets(buf, BUFSIZE, from_spell_checker_st)) {
			 fprintf(stderr, "error reading word list\n");
			 return(-1);}
		    correct_let = buf[0];
		    
		    fseek(from_spell_checker_st,0,SEEK_END);
	       }
	       
	       if ((correct_let == '*')||(correct_let == '+')){
		    /*
		     * just generated a word that occurs in the current
		     * lexicon
		     */
		    buf[0] = '\0';
		    if (cfg.log_file_p){
			 fprintf(cfg.log_file,
				 "\n-*NON ISPELL WORD GENERATED**");
			 fprintf(cfg.log_file,
				 " %s to %s has a rating of ...",word,
				 new_word);
			 fprintf(cfg.log_file,"%f\n",freq(fast_analysis,j));
		    }
		    
		    quick_insert(head_stat,new_word,freq(fast_analysis,j));

		    
		    strcpy(new_word,word);
	       }
	       else {

		    fsub = freq(fast_analysis,j);/* First level substitution */

		    for (i=0;i<near_miss_hash;++i){
			 
			 if ((strstr(prefix,
				     near_miss_letters[i].generated_char)
			      != NULL)
			     || (strstr(suffix,
					near_miss_letters[i].generated_char)
				 != NULL)){
			      
			     			      
			      r = strstr(new_word, 
					 near_miss_letters[i].generated_char);
			      
			      
			      if (r != NULL){
				   
				   {
					q = new_word + strlen(new_word);
					*q = '\0';
					p = r + 
					     near_miss_letters[i].gener_length;
					strcpy(new_suffix,p);
					for (parse=0;
					     parse<near_miss_letters[i].correct_length;
					     ++parse)
					{
					     *r = near_miss_letters[i].correct_char[parse];
					     ++r;
					}
					*r = '\0';
					strcat(new_word,new_suffix);
				   }
				   level_two = 1;
				   for (y = new_word; *y; y++) {
					if (!isalpha(*y)){
					     correct_let = '\0';
					     level_two = 0;
					}
				   }
				   correct_let = '\0';
				   if (level_two){
					
					fprintf(to_spell_checker_st, 
						"%s\n", new_word);
					fflush(to_spell_checker_st);
					correct_let = '\0';
					buf[0] = '\0';
					if (!fgets(buf, BUFSIZE,
						   from_spell_checker_st)) {
					     fprintf(stderr, "error reading word list\n");
					     return(-1);}
					correct_let = buf[0];
					
					
					fseek(from_spell_checker_st,0,SEEK_END);
				   }
				   if ((correct_let == '*')||
				       (correct_let == '+')){
					/*
					 * Just generated a word that occurs
					 * in the current lexicon at level
					 * two
					 */
					if (cfg.log_file_p){
					     fprintf(cfg.log_file,"\n+*NON ISPELL WORD GENERATED**");
					     fprintf(cfg.log_file," %s to %s has a rating of ...",word,new_word);
					     fprintf(cfg.log_file,"%f\n",(freq(fast_analysis,i)*fsub));
					}
					     
					quick_insert(head_stat,new_word,
						     (freq(fast_analysis,i)*
						      fsub));
				   }
				   strcpy(new_word,ng_word);
				   
			      }
			 }
		    }
		    
	       }
	       	       
	  }
	 
     }
     return(0);
}
#endif
