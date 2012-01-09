/*
 * $Id: stats.c,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $	 
 *
 * Statistical Spell Checker Prototype
 * Eric Stofsky
 * file: stats.c (heuristic generator)
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
 * Routines to create the ngram lookup table and perform statistical
 * normalization of the device mapping statistics.  Ngrams are hashed
 * by length and sorted.  A binary search is used to retrieve the 
 * statistical frequencies for its respective n-gram length. Medians 
 * are corrected via Bayes' rule.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef STDC_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif /* STDC_HEADERS */

#include "statcheck.h"
#include "stats.h"
#include "ocrgen.h"
#include "normal.h"

static char rcsid[] = "$Id: stats.c,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $";

static letter_table *letter_lookup; 
static letter_freq_boundary *letter_boundary_lookup;
static void letter_lookup_fast_hash(int);
static int binary_lookup(char *,int,int);
static void letter_lookup_swap(letter_table *,int,int);
static void letter_lookup_sort(letter_table *,int,int);
static int apply_normalization_value(char *);
static void initialize_characterset(void);
static FILE *letter_freqs;       /* generated letter frequency file */


static void
letter_lookup_fast_hash(number_of_letter_freqs)
int number_of_letter_freqs;
/*
 * fast hash the n-grams by length;
 */
{
     int current_length,i,j;

     if (!(letter_boundary_lookup = (letter_freq_boundary *)
	   calloc(MAX_NGRAM, sizeof(letter_freq_boundary)))){
	  perror(NULL);
	  exit(1);
     }
     
     letter_boundary_lookup[0].start_position = 1;
     i = 0; j = 0;
     current_length = 1;

     while (i <= (MAX_NGRAM - 1)){
	  while ((strlen(letter_lookup[j].characters) == current_length)
		 && (j < number_of_letter_freqs-1)){
	       ++j;
	  }
	  letter_boundary_lookup[i].end_position = j-1;
	  if (i < (MAX_NGRAM - 1)){
	       letter_boundary_lookup[i+1].start_position = j;
	  }
	  ++current_length;
	  ++i;
     }
     letter_boundary_lookup[(MAX_NGRAM - 1)].end_position =
	  number_of_letter_freqs - 1;
     
}



static int
binary_lookup(n_gram,low_boundary,high_boundary)
char *n_gram;int low_boundary,high_boundary;
/* 
 * Perform binary search over the hashed boundary. Low_boundary
 * and high_boundary are the upper and lower bounds for the current
 * n-gram size.
 */
{
     int current_length,mid;
     while (low_boundary <=high_boundary){
	  mid = (low_boundary + high_boundary)/2;
	  if ((current_length = 
	       strcmp(n_gram,letter_lookup[mid].characters)) < 0){
		    high_boundary = mid - 1;
	       }

	  else if (current_length > 0){
	       low_boundary = mid + 1;
	  }
	  else {
	       return (mid);
	  }
     }
     return (-1);
}
  


static void
letter_lookup_swap(table,i,j)
letter_table table[];int i,j;
/*
 * Swaps two n-gram entries.  Used by the n-gram structure sort 
 * routine. See letter_lookup_sort()
 */
{
     char *temp_name;
     int temp_frequency;

     if (!(temp_name = 
	   (char *) malloc (strlen (table[i].characters) + 1))){
	  perror(NULL);
	  exit(1);
     }
     strcpy(temp_name,table[i].characters);
     temp_frequency = table[i].frequency;
     free(table[i].characters);

     if (!(table[i].characters =
	   (char *) malloc (strlen(table[j].characters) + 1))){
	  perror(NULL);
	  exit(1);
     }
     strcpy(table[i].characters,table[j].characters);
     table[i].frequency = table[j].frequency;
     free(table[j].characters);
     if (!(table[j].characters = 
	   (char *) malloc (strlen (temp_name) + 1))){
	  perror(NULL);
	  exit(1);
     }
     strcpy(table[j].characters,temp_name);
     table[j].frequency = temp_frequency;
     free(temp_name);
}



static void
letter_lookup_sort(table,left,right)
letter_table table[];int left,right;
/*
 * Sort the n-gram table using qsort by length
 * lexicographically to group all n-grams of the same
 * length together alphabetically to allow for a bounded
 * binary search
 */     
{
     int i, last;
     if (left >= right)
	  return;
     letter_lookup_swap(table, left, (left + right)/2);
     last = left;
     
     for(i=left+1; i<=right;i++){
	  if (strlen(table[i].characters)<strlen(table[left].characters)){
	       letter_lookup_swap(table, ++last, i);
	  }
	  else if ((strcmp(table[i].characters,table[left].characters) < 0)
		   && (strlen(table[i].characters) ==
		    strlen(table[left].characters))){
	       letter_lookup_swap(table, ++last, i);
	  }
	  
     }

     letter_lookup_swap(table,left,last);
     letter_lookup_sort(table,left,last-1);
     letter_lookup_sort(table,last+1,right);
}



void
create_letter_look_up(void)
/*
 * dynamically allocate statistical lookup normalization structure
 */
{
     int size_of_table = 0;
     char garbage[BUFSIZE];
     
     while (fgets(garbage,BUFSIZE,letter_freqs) != NULL){
	  ++size_of_table;      /* count the number of n-grams */
     }
     rewind(letter_freqs);            /* rewind stats file */
     
     if (!(letter_lookup = (letter_table *) 
	   calloc(size_of_table, sizeof(letter_table)))){
	  perror(NULL);
	  exit(1);
     }     
}



void
open_letter_stats_file(void)
/*
 * open stats normalization file
 */
{
     if ((letter_freqs = fopen (cfg.letter_ngram_file, "r")) == NULL){
	  printf("\n Error: Can't read %s file", cfg.letter_ngram_file);
	  exit(1);
     }
}



void  
read_letter_file(void)
/*
 * copy stat normalization statistics to lookup structure
 */
{
     char temp_string[MAX_NGRAM + 1];
     int num_chars_read = 0;
     int number_of_letter_freqs = 0;
     
     stat_norm_st.normal_value = 0;   

     while (fscanf(letter_freqs,"%d ",
		   &(letter_lookup[num_chars_read].frequency)) != EOF){
	  if (fscanf(letter_freqs,"%s\n",temp_string) == EOF){
	       printf("\n Error reading letter.freq file\n");
	       exit(1);
	  }
	  stat_norm_st.normal_value = 
	       stat_norm_st.normal_value + letter_lookup[num_chars_read].frequency;
	  if (!(letter_lookup[num_chars_read].characters = 
		(char *) malloc (sizeof (strlen(temp_string)) +1))){
	       perror(NULL);
	       exit(1);
	  }
	  strcpy(letter_lookup[num_chars_read].characters, temp_string);
	  ++num_chars_read;
	  ++number_of_letter_freqs;
     }
     stat_norm_st.normal_value = stat_norm_st.normal_value/100;
     /*
      * sort the n-grams by length lexicographically
      */
     letter_lookup_sort(letter_lookup,0,num_chars_read - 1);
     /*
      * index the the n-gram length boundaries
      */
     letter_lookup_fast_hash(number_of_letter_freqs);
     fclose(letter_freqs);
}



static int
apply_normalization_value(substitution)
/*
 * Perform normalization lookup on the substitution string by
 * determining the length boundaries and performing a binary
 * search
 */
char *substitution;
{
     int high,low,location;
     low =
	  letter_boundary_lookup[(strlen(substitution) - 1)].start_position -1;
     high =
	  letter_boundary_lookup[(strlen(substitution) - 1)].end_position -1;

     location = binary_lookup(substitution,low,high);
     if (location == -1){
	  return(DEFAULT_NORMALIZATION);
     }
     return(letter_lookup[location].frequency);
}


int 
normalized_sub_value(sub)
char *sub;
/* 
 * compute normalization stats for sub string substitution
 */
{
     return (apply_normalization_value(sub));
     
}


static void
initialize_characterset(void)
/*
 * Initialize common english character substitution placements.
 * This has not been implemented yet...
 */
{
     struct characterset charset[] =
     {
	  
     };

}
     





