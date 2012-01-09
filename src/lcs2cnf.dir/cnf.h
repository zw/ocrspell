 /*
  * Statistical Spell Checker Prototype
  * file: cnf.h (confusion generator)
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
 

#define CONFUSION_LEN 16

struct confusion {
	 char a[CONFUSION_LEN];
	 char b[CONFUSION_LEN];
	 struct confusion *next;
};

struct confusion *select_confusions(char *, char *, int **);
