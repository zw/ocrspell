/*
 * $Id: ispell_emulate.c,v 1.1.1.1 2000/08/28 21:11:37 slumos Exp $	
 *
 * Statistical Spell Checker Prototype
 * Eric Stofsky
 * file : ispell_emulate.c (translates stats to ispell output)
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

/* This file contains the routines to allow Ocrspell to output
 * results in ispell(1) form.  The words are sorted based on their
 * statistical probability and then displayed in the form:
 * & misspelled-word number-of-misspellings offset: correction-list.
 * If the word queried occurs in the lexicon, a '*' is returned.
 * If the length of the correction-list > 1, each word possibility
 * is delimited by a comma.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef STDC_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "statcheck.h"
#include "ispell_emulate.h"
#include "ocrgen.h"

static char rcsid[] = "$Id: ispell_emulate.c,v 1.1.1.1 2000/08/28 21:11:37 slumos Exp $";

static char ISPELL_EMULATION[] = EMULATION_VERSION_ID;

static Look_Up *replacement_info;  /*pointer to ispell table structure*/

static void table_swap(/*Look_Up[], int, int*/);
static void table_sort(/*Look_up[], int ,int*/);
static int copy_struct(stat_link);
void dump_intro(void);
void structure_setup(char *,int, stat_link);
static void mapcaseinit(void);



static void
table_swap(table,i,j)
Look_Up table[]; int i,j;
/*
 * swaps two statistical word entries
 */
{
     char *temp_name;
     float temp_stat;
     if (!(temp_name = 
	   (char *) malloc (strlen (table[i].the_word) + 1))){
	  perror(NULL);
	  exit(1);
     }
     strcpy(temp_name,table[i].the_word);
     temp_stat = table[i].the_stat;
     free(table[i].the_word);
     if (!(table[i].the_word =
	   (char *) malloc (strlen(table[j].the_word) + 1))){
	  perror(NULL);
	  exit(1);
     }
     strcpy(table[i].the_word,table[j].the_word);
     table[i].the_stat = table[j].the_stat;
     free(table[j].the_word);
     if (!(table[j].the_word = 
	   (char *) malloc (strlen (temp_name) + 1))){
	  perror(NULL);
	  exit(1);
     }
     strcpy(table[j].the_word,temp_name);
     table[j].the_stat = temp_stat;
     free(temp_name);
}


static void 
table_sort(table,left,right)
Look_Up table[]; int left,right;
/* 
 * sort the statistical word structure using qsort
 */
{
        int i, last;
	if (left >= right)
                return;

        table_swap(table, left, (left + right)/2);
        last = left;

        for(i=left+1; i<=right;i++)
                if(table[i].the_stat>table[left].the_stat)
                        table_swap(table, ++last, i);

        table_swap(table,left,last);
        table_sort(table,left,last-1);
        table_sort(table,last+1,right);
}



static int
copy_struct(head)
stat_link head;
/*
 * copies words/stats to i-emulation structure
 */
{
     int into_table = 0;
     int No_Of_Entries = 0; /*number of entries in ispell structure */
     free(replacement_info);
     if (!(replacement_info = 
	   (Look_Up *) calloc(stat_count(head), sizeof(Look_Up)))){
	  perror(NULL);
	  exit(1);
     }

     while (head != NULL){
	  replacement_info[into_table].the_stat = head ->chance;
	  if (!(replacement_info[into_table].the_word =
		(char *) malloc (strlen (head -> word) + 1))){
	       perror(NULL);
	       exit(1);
	  }
	  strcpy(replacement_info[into_table].the_word, head-> word);
	  head = (head -> next);
	  ++into_table;
	  ++No_Of_Entries;
     }
     return(No_Of_Entries);
}



void
dump_intro(void)
/*
 * emulation identification
 */
{
     printf("@(#) Ispell Output Emulator Version %s\n",ISPELL_EMULATION);
}



void 
structure_setup(query_word, curr_offset, head_stat)
char *query_word; int curr_offset;stat_link head_stat;
/*
 * performs ispell emulation
 */
{
     
     int i;
     int No_Of_Entries;
     if (cfg.interactive_prompt){
	  dump_intro();
     }
     No_Of_Entries = copy_struct(head_stat);
     table_sort(replacement_info,0,stat_count(head_stat) - 1);
     switch(No_Of_Entries){

     case 0: 
	  {
	       printf("# %s 0\n",query_word);
	       break;
	  }
     default:
	  {
	       
	       printf("& %s ",query_word);
	       printf("%d ",stat_count(head_stat));
	       printf("%d: ",curr_offset);
	       for (i=0;i<(No_Of_Entries-1);++i){
		    printf ("%s, ", replacement_info[i].the_word); 
	       }
	       printf ("%s", replacement_info[i].the_word);
	       printf("\n");
	       break;
	  }
     }
}



static void
mapcaseinit()
/*
 * maps word alphabet
 */
{
     short map_the_case[CHARACTER_SET]; /* case mapping structure */

     static one_time = 1;
     int i;
     
     if (one_time == 0)
	  return;
     
     one_time = 0;
     
     
     for (i = 'A'; i <= 'Z'; i++)
	  map_the_case[i] = i - 'A' + 'a';
     
     
     for (i = 'a'; i <= 'z'; i++)
	  map_the_case[i] = i;
     
     
     map_the_case['\''] = '\'';
     
     
     for (i = 0x80; i < 0x100; i++)
	  map_the_case[i] = i;
     
     
     map_the_case[0x80] = 0x87;		/* c cedialla */
     map_the_case[0x8e] = 0x84;		/* a umlaut */
     map_the_case[0x8f] = 0x86;		/* a with circle */
     map_the_case[0x90] = 0x82;		/* e acute */
     map_the_case[0x92] = 0x91;		/* ae ligaturrre */
     map_the_case[0x99] = 0x94;		/* o umlaut */
     map_the_case[0x9a] = 0x81;		/* u umlaut */
     map_the_case[0xa5] = 0xa4;		/* n tilde */
}







