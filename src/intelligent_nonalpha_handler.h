/*
 * $Id: intelligent_nonalpha_handler.h,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $	
 *
 * Statistical Spell Checker Prototype
 * Eric Stofsky
 * file: intelligent_nonalpha_handler.h (intelligent punctuation &
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


struct WORD_TYPE {
     int pure_character_word;   
     int dictionary_insertion;
     char *original_word;
     char *left_punctuation;
     char *right_punctuation;
     int apost;
     int correct_spelling; 
     char *word_to_check;
     int NO_ISPELL;  
};

typedef struct WORD_TYPE word_props;
extern word_props a_word;

int in_punction_handler(char *, int *, int);


     
