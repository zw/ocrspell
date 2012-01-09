/*
 * $Id: normal.h,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $	 
 *
 * Statistical Spell Checker Prototype
 * file: normal.h (normalization)
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

struct NORMALIZATION_STATS {
     long int table_median;
     long int normal_value; /*stat normalization*/
};


typedef struct NORMALIZATION_STATS normalization_stats;

extern normalization_stats stat_norm_st;

