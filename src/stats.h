/*
 * $Id: stats.h,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $	 
 *
 * Statistical Spell Checker Prototype
 * Eric Stofsky
 * file: stats.h (heuristic generator header)
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



#define NUM_LETTERS_MAX 16000
#define MAX_NGRAM 3
#define DEFAULT_NORMALIZATION 10

struct letter_freq{
     char  *characters;
     int    frequency;
};

typedef struct letter_freq letter_table;

struct letter_freq_boundary{
     int start_position;
     int end_position;
};

typedef struct letter_freq_boundary letter_freq_boundary;

struct characterset
{
     char lower;
     char word_cmp;
};
extern struct characterset charset[];

void create_letter_look_up();
void open_letter_stats_file();
void read_letter_file();
int normalized_sub_value (char *);

