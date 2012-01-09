/*
 * Statistical Spell Checker Prototype
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

#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <string.h>
#include "subseq.h"
#include "cnf.h"


static void usage();



char *prog;
int LCStrace=0;
int CNFtrace=0;



int
main(argc, argv)
    int argc;
    char *argv[];
{
	int LCS_length, i, m, n;
	int **LCSVectors = NULL;
	int **vp, *vpp, v;
	char b[MAX_LEN][MAX_LEN];
	char *string1, *string2;
	struct confusion *confusions, *cnf;

	
	prog = strrchr (*argv, '/');
	if (prog) prog++;
	else prog = *argv;


    if (argc != 3) usage();


	string2 = argv[1];
	string1 = argv[2];
	m = strlen(string1);
	n = strlen(string2);

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

	printf("\n%s -> %s\n", string2, string1);
	for (cnf=confusions; cnf; cnf=cnf->next)
		 printf("\t%s -> %s\n", cnf->a, cnf->b);
	
    return(0);
}




static void
usage(){

	 fprintf(stderr, "Usage: %s <string1> <string2>\n\n", prog);
	 exit(1);
}
