/*
 * $Id: ispell_emulate.h,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $	
 *
 * Statistical Spell Checker Prototype
 * Eric Stofsky
 * file: ispell_emulate.h (header for ispell emulation)
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

#define EMULATION_VERSION_ID "1.0.00 08/17/95"

#define CHARACTER_SET 256

struct Look_Up {
     char *the_word;
     float the_stat;
};

typedef struct Look_Up Look_Up;


void dump_intro(void);
void structure_setup(char *,int,stat_link);
