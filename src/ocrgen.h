/*
 * $Id: ocrgen.h,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $	 
 *
 * Statistical Spell Checker Prototype
 * Eric Stofsky
 * file : ocrgen.h (header for driver module)
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



struct ocrspell_configuration{
     char *ispell_path;           /*path to ispell executable*/
     char *learn_database;        /*path to learn database*/
     char *ocr_static_freqfile;   /*path to ocr statistics(static)*/
     char *letter_ngram_file;     /*path to n-gram statistics file*/
     int use_alternate_dictionary;/*boolean flag for alternate dictionary*/
     char *alt_dictionary_path;   /*path to an alternate hashed dictionary*/
     int spell_by_line;           /*boolean flag for spell-by-line option*/
     char *line_break;            /*line break sequence*/
     int log_file_p;              /*boolean flag for log file detection*/
     int intelligent_word;
     int dictionary_insertion;    /*indicates whether local insertions are
				    allowed*/
     int ispell_evaluation;       /*boolean flag indication whether to
				    evaluate ispell's output*/
     int multilevel_saturation;   /*allow for use of multilevel_saturation
				    technique*/
     int in_word;                 /*keeps track of current word in sentence*/
     int interactive_mode;        /*flag indicating whether ocrspell is in 
				    interactive mode*/
     int interactive_prompt;      /*boolean flag indicating whether 
				    interactive ocrspell will prompt at
                                    each iteration*/
     int interactive_learning;    /*flag indicticating whether ocrspell should
				    learn from its mistakes*/
     int fast_stats_mode;         /*flag indictating whether ocrspell is in
				    fast stats mode*/
     int evaluate_ispell_choices; /*flag indicating  whether ocrspell is to
				    evaluate the initial near misses generated
				    by the spell checking program*/
     FILE *log_file;              /*log file information*/
};

extern struct ocrspell_configuration cfg;
 
     
     
