/* 
 * $Id: statcheck.h,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $	 
 *
 * Statistical Spell Checker Protoype
 * Eric Stofsky 
 * file : statcheck.h (stat structures)
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

#define BUFSIZE 1024           /*maximum buffer size*/

#define MAX_WORD_CHAR 100      /*maximum word length*/

#define MAX_CHAR_REPLACEMENT 3 /*maximum character substitution or
				 replacement length*/

struct NEAR_MISS_INFO {
     float median_isolated;    /*mean of error*/
     char *correct_char;       /*correct character string*/
     int correct_length;       /*length of correct character string*/
     char *generated_char;     /*generated character string*/
     int gener_length;         /*length of generated character string*/
};


typedef struct NEAR_MISS_INFO NEAR_MISS_INFO;


struct STATISTICS{
     char *word;                /*word possibility*/
     float chance;              /*statistical probability of selection*/
     int hash;                  /*hash to OCR error table*/
     struct STATISTICS *next;   
};



typedef struct STATISTICS statistics;

typedef statistics *stat_link;

extern NEAR_MISS_INFO *near_miss_letters;

extern int spell_checker_pid;
extern FILE *to_spell_checker_st, *from_spell_checker_st;

void print_the_stat_list(stat_link);
int stat_count(stat_link);
int spell_checker_shutdown();
int spell_checker_startup(char *, char *);
int read_stat_list(int *);
int add_new_word_for_session(char *);
void discard_end_of_matches_newline();
int analyze_word_list(/* char *, char *** , stat_link * */);
void dump_the_table();
float freq(int,int);

long int 
ocrstringreplacement_generator(/* char *, char ***, stat_link * int */);

long int 
rate_the_choices_wrongletter(/* char *, char ***, stat_link * int */);

long int 
rate_the_choices_extra_insertion(/* char *, char ***, stat_link * int */);

long int 
rate_the_choices_missingletter (/* char *, char ***, stat_link * int */);


