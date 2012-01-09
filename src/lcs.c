/*
 * $Id: lcs.c,v 1.1.1.1 2000/08/28 21:11:37 slumos Exp $	 
 *
 * Statistical Spell Checker Prototype
 * Jeff Gilbreth, Eric Stofsky
 * file: lcs.c (confusion generator)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef STDC_HEADERS
#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <string.h>
#endif /* STDC_HEADERS */

#include "subseq.h"

static char rcsid[] = "$Id: lcs.c,v 1.1.1.1 2000/08/28 21:11:37 slumos Exp $";

char *acronym, *leaders, *types;

static int  *compare_vectors (int *, int *, char *);
static void vector_values(int *, int *, int *, int *, int *, char *);


void vector_values(V, misses, stopcount, distance, length, types)
int *V, *misses, *stopcount, *distance, *length;
char *types;
{
     int i = 1, len, first, last;

     for (len=0; V[len] != -1; len++) ;

     i = 0;

     while (i < len && V[i] == 0){
	  i++;
     }
     first = i;
     i = len-1;
     while (i > 0 && V[i] == 0){
	  i--;
     }
     last = i;

     (*length) = last - first;
     (*distance) = len - last;

     for (i=first; i<last; i++) {
	  if (V[i] > 0 &&types[i] == 's'){ 
	       (*stopcount)++;
	  }
	  else if (V[i] == 0 && types[i] != 's' && types[i] != 'h'){ 
	       (*misses)++;
	  }
     }
}



int * 
compare_vectors(A, B, types)
int *A, *B;
char *types;
{
     int A_misses=0, A_stopcount=0, A_distance=0, A_length=0;
     int B_misses=0, B_stopcount=0, B_distance=0, B_length=0;
     
     vector_values(A, &A_misses, &A_stopcount, &A_distance, &A_length, types);
     vector_values(B, &B_misses, &B_stopcount, &B_distance, &B_length, types);

     if (A_misses > B_misses)
	  return (B);
     else if (A_misses < B_misses)
	  return (A);
     if (A_stopcount > B_stopcount)
	  return (B);
     else if (A_stopcount < B_stopcount)
	  return (A);
     if (A_distance > B_distance)
	  return (B);
     else if (A_distance < B_distance)
	  return (A);
     if (A_length > B_length)
	  return (B);
     else if (A_length < B_length)
	  return (A);
     return (A);
}
