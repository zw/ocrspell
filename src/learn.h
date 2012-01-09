/*
 * $Id: learn.h,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $	 
 *
 * Statistical Spell Checker Prototype
 * Eric Stofsky
 * file: learn.h (statistical learning  header)
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

#define INPUT_WORD_LENGTH 50


void add_another_confusion(char *, char *, int *);
void interactive_training(char *,int *);


