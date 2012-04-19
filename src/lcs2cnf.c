/*
 * $Id: lcs2cnf.c,v 1.1.1.1 2000/08/28 21:11:37 slumos Exp $
 *
 * Statistical Spell Checker Prototype
 * Jeff Gilbreth, Eric Stofsky
 * file: lcs2cnf.c  (confusion generator)
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

/* 
 * Routines to generate confusions for strings when OCRSpell is running
 * in learning mode. Confusions are generated using the well known dynamic
 * programming longest common substring algorithm.  Each of the computed
 * confusions are treated as distinct device mappings by OCRSpell.  Thus,
 * the the device mapping statistics are adjusted accordingly.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef STDC_HEADERS
#include <stdio.h>
#include <memory.h>
#include <string.h>
#endif /* STDC_HEADERS */

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif /* HAVE_MALLOC_H */

#include "subseq.h"
#include "cnf.h"
#include "learn.h"
#include "lcs2cnf.h"

static char rcsid[] = "$Id: lcs2cnf.c,v 1.1.1.1 2000/08/28 21:11:37 slumos Exp $";

static void destroy_LCSVectors(int ***);
static void destroy_confusions(struct confusion **);
int LCStrace=0;
int CNFtrace=0;


int
find_confusions(string1,string2,current_no_mappings)
char *string1,*string2;int *current_no_mappings;
/*
 * Determines the confusions between two strings by using the longest
 * common substring algorithm
 */
{
     int LCS_length, m, n;
     int **LCSVectors = NULL;
     int **vp, *vpp, v;
     char b[MAX_LEN][MAX_LEN];
     struct confusion *confusions, *cnf;
     
     m = strlen(string1);
     n = strlen(string2);

     /* 
      * check to see if the strings are the same or the words
      * are so different that a longest common substring would
      * be meaningless, i.e. there are no character synchs in
      * the strings
      */
     
     if (!strcmp(string1,string2)){
	  return(0);
     }

     if ((strpbrk(string1,string2) == NULL) ||
	 (strpbrk(string2,string1) == NULL)){
	  return(0);
     }

#ifdef DEBUG
     printf ("\nprocessing \"%s\" -> \"%s\"\n", string2, string1);
#endif
     
     LCS_length = build_LCS_matrix(b, string1, string2);
     
     parse_LCS_matrix(b, 1, 1, m, n, LCS_length, &LCSVectors);
     
     if (LCStrace) {
	  printf ("\nLCS list:\n");
	  for (vp=LCSVectors, v=0; vp && *vp; vp++, v++) {
	       printf ("\t[ ");
	       for (vpp = *vp; *vpp>=0; vpp++) {
		    printf ("%d ", *vpp);
	       }
	       printf ("]\n");
	  }
     }	
     
     
     if (!(confusions = select_confusions (string1, string2, LCSVectors) )) {
	  exit (1);
     }

     printf("\nOCRSpell Confusion Generator V%s", VERSION_ID);
     printf("\n%s -> %s\n", string2, string1);

     for (cnf=confusions; cnf; cnf=cnf->next){
	  printf("\t%s -> %s\n", cnf->a, cnf->b);
	  add_another_confusion(cnf->a,cnf->b,current_no_mappings);
     }
     
     destroy_LCSVectors (&LCSVectors);
     destroy_confusions (&confusions);
     

     return(0);
}


static void destroy_LCSVectors (V) 
int ***V;
{
     /*
      * free up memory used the the LCS vector structure
      */

     int **vp;

     for (vp=*V; vp && *vp; vp++) {
	  free (*vp);
     }

     free (*V);
     *V = NULL;
}


static void destroy_confusions (confusions) 
struct confusion **confusions;
{
     /*
      * free up memory used to store confusions and device
      * mappings
      */

     struct confusion *cnf, *prev;

     for (cnf=*confusions; cnf; ) {
	  prev=cnf;
	  cnf=cnf->next;
	  prev->next = NULL;
	  free (prev);
     }
     *confusions = NULL;
}

