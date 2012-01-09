/*
 * $Id: subseq.c,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $	 
 *
 * Statistical Spell Checker Prototype
 * Jeff Gilbreth, Eric Stofsky
 * file: subseq.c (confusion generator)
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
 * Routines to generate the longest common subsequence between 
 * two strings.  The common dynamic programming method is used.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef STDC_HEADERS
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <string.h>
#endif /* STDC_HEADERS */

#include "subseq.h"
#include "lcs2cnf.h"

static char rcsid[] = "$Id: subseq.c,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $";

struct link_st {
    int i;
    int j;
    struct link_st *next;
};


struct link_st *stack = NULL;
static int  save_vector(int, int ***);


static int save_vector(n, vectorlist)
int n;
int ***vectorlist;
/*
 * Save subsequence as a vector
 */
{
     int i, vlen=0;
     int **v, *vector;
     struct link_st *s;
     
     if (!(vector = (int *) malloc (sizeof(int)*n+1))) {
	  perror("malloc");
	  return(0);
     }

     for (i=0; i<n; i++) {
	  vector[i] = 0;
     }

     vector[n] = -1;
     
     for (s=stack; s; s=s->next) {
	  vector[s->j-1] = s->i;
     }

     if (!(*vectorlist)) {

	  if (!(*vectorlist = (int **) malloc (sizeof(int *)*2))) {
	       perror(NULL);
	       return(0);
	  }

     } else {

	  for (vlen=0, v=(*vectorlist); v && *v; v++, vlen++) ;
	  
	  if (!(*vectorlist = (int **) realloc (*vectorlist, 
						sizeof(int *)*(vlen+2)))) {
	       perror(NULL);
	       return(0);
	  }
     }

     (*vectorlist)[vlen] = vector;
     (*vectorlist)[vlen+1] = NULL;

     return(1);
}




void parse_LCS_matrix(b, si, sj, n, m, LCS_length, LCSvectors)
char b[MAX_LEN][MAX_LEN];
int si, sj;
int n, m;
int LCS_length;
int ***LCSvectors;
/*
 * Parse the dynamic programming table
 */
{
     int i, j;
     struct link_st *node, *curr;

     if (b == NULL) return;

     for (i=si; i<=n; i++) {
	  for (j=sj; j<=m; j++) {

	       if (b[i][j] == '\\') {

		    /* 
		     *  build this node and pop onto stack
		     */
		    if (!(node = (struct link_st *) 
			  malloc (sizeof(struct link_st)))) {
			 perror("malloc");
			 return;
		    }
		    node->i = i;
		    node->j = j;
		    node->next = stack;
		    stack = node;

		    if (LCS_length == 1) {
						
			 /* 
			  *  save this lcs in the vectorlist.
			  */
			 if (!(save_vector(m, LCSvectors))) {
			      perror("save_vector failed");
			      exit(1);
			 }
			 
		    } else {

			 /* 
			  *  call ourself 
			  */
			 parse_LCS_matrix(b, i+1, j+1, n, m, 
					  LCS_length-1, LCSvectors);
		    }

		    /* 
		     *  pop off the top of the stack
		     */
		    curr = stack;
		    if (curr) {
			 stack = curr->next;
			 free (curr);
		    }
		    /*
		       } else if ((i + LCS_length > n) 
		       || (j + LCS_length > m)) {
		       printf ("dead row\n");
		       break;
		       */
	       }
	       
	  }
     }
     return;
}



int build_LCS_matrix(b, A, B)
char b[MAX_LEN][MAX_LEN], *A, *B;
/*
 * Build the dynamic programming table
 */
{
     int i, j;
     int n = strlen(A);
     int m = strlen(B);
     int c[MAX_LEN][MAX_LEN];

     for (i=0; i<m; i++)
	  c[i][0] = 0;

     for (j=0; j<n; j++)
	  c[0][j] = 0;

     for (i=1; i<=n; i++) {
	  for (j=1; j<=m; j++) {
	       if (A[i-1] == B[j-1]) {
		    c[i][j] = c[i-1][j-1] + 1;
		    b[i][j] = '\\';
	       } else if (c[i-1][j] >= c[i][j-1]) {
		    c[i][j] = c[i-1][j];
		    b[i][j] = '|';
	       } else {
		    c[i][j] = c[i][j-1];
		    b[i][j] = '-';
	       }
	  }
     }


     if (LCStrace){ 
	  for (i=0; i<=n; i++) {
	       for (j=0; j<=m; j++) {
		    
		    if (b[i][j] == '\\') printf("\\");
		    else printf(" ");

		    if (b[i][j] == '|') printf("|");
		    else printf(" ");
	       }
	       printf("\n");
	       for (j=0; j<=m; j++) {
		    
		    if (b[i][j] == '-') printf("-");
		    else printf(" ");
		    
		    printf("%d", c[i][j]);
	       }
	       printf("\n");
	  }
     }

     return (c[n][m]);
}
