*
 * Statistical Spell Checker Prototype
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

#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <string.h>
#include "subseq.h"


char *acronym, *leaders, *types;

static int  *compare_vectors (int *, int *, char *);
static void vector_values(int *, int *, int *, int *, int *, char *);




int
main(argc, argv)
    int argc;
    char *argv[];
{
	int LCS_length, i, j, n, m;
	int **LCSVectors = NULL;
	int **vp, *vpp, v;
	int *left, *best;
	char b[MAX_LEN][MAX_LEN];

    if (argc != 4) {
        fprintf(stderr, "Usage: lcs <acronym> <leaders> <types>\n");
        exit(1);
    }

	acronym = argv[1];
	leaders = argv[2];
	types = argv[3];
	m = strlen(acronym);
	n = strlen(leaders);

	printf ("processing \"%s\":\n", argv[1]);
	printf ("[ ");
	for (i=0; i<n; i++) {
		 printf("%c ", types[i]);
	}
	printf ("] types\n");

	printf ("[ ");
	for (i=0; i<n; i++) {
		 printf("%c ", leaders[i]);
	}
	printf("] leaders\n\n");

    LCS_length = build_LCS_matrix(b, acronym, leaders);

	printf("\nthe `b' array:\n");
	for (i=1; i<=m; i++) {
		 for (j=1; j<=n; j++)
			  printf("%c", b[i][j]);
		 printf("\n");
	}
	printf("\n");

	parse_LCS_matrix(b, 1, 1, m, n, LCS_length, &LCSVectors);

	printf ("LCS list:\n");
	for (vp=LCSVectors, v=0; vp && *vp; vp++, v++) {
		 printf ("\t[ ");
		 for (vpp = *vp; *vpp>=0; vpp++) {
			  printf ("%d ", *vpp);
		 }
		 printf ("]\n");
	}
	if (v > 1) {
		 best = NULL;
		 for (vp=LCSVectors, left=*vp++; vp && *vp; left=best, vp++) {
			  best = compare_vectors(left, *vp, types);
		 }
		 printf("best match:\t[ ");
		 for (vpp = best; *vpp>=0; vpp++) {
			  printf ("%d ", *vpp);
		 }
		 printf ("]\n");
	}

    exit(0);
}







void vector_values(V, misses, stopcount, distance, length, types)
    int *V, *misses, *stopcount, *distance, *length;
    char *types;
{
    int i = 1, len, first, last;

	for (len=0; V[len] != -1; len++) ;
	i = 0;
	while (i < len && V[i] == 0)
		i++;
	first = i;
	i = len-1;
	while (i > 0 && V[i] == 0)
		i--;
	last = i;
	(*length) = last - first;
	(*distance) = len - last;
	for (i=first; i<last; i++) {
		if (V[i] > 0 &&types[i] == 's') 
			(*stopcount)++;
		else if (V[i] == 0 && types[i] != 's' && types[i] != 'h') 
			(*misses)++;
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
