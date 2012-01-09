/*
 * $Id: subseq.h,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $	 
 *
 * Statistical Spell Checker Prototype
 * file: subseq.h (confusion generator)
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


#define MAX_LEN 255

int  build_LCS_matrix (char [MAX_LEN][MAX_LEN], char *, char *);
void parse_LCS_matrix (char [MAX_LEN][MAX_LEN], int, int, int, int, int, 
					   int ***);
