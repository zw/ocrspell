/*
 * Statistical Spell Checker Prototype
 * file: cnf.c (confusion generator)
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



struct c_v {
	 int *vector;
	 struct confusion *cnf;
};


static struct c_v *compare_confusion_vectors(struct c_v *, struct c_v *, 
											 int, int);
static void vector_confusion_values(struct c_v *, int *, int *, 
									int *, int, int);
static struct confusion *confusion_generation(char *, char *, int *);
static struct confusion *new_confusion(char *, char *, int *, 
									   int, int, int, int);



extern char *prog;
extern int CNFtrace;
extern int LCStrace;




struct confusion 
*select_confusions (string1, string2, Vectors)
  char *string1, *string2;
  int **Vectors;
{
	int i, m, n;
	int **vp, *vpp, v;
	struct c_v *confusion_vector_list;
	struct c_v *best, *left;


	m = strlen(string1);
	n = strlen(string2);


	/*  if there are no vectors, the strings are completely different;
	 *  therefore, return the confusion with the complete strings.
	 */
	if (Vectors == NULL) {

		 return (confusion_generation (string1, string2, NULL));
	}


	for (vp=Vectors, v=0; vp && *vp; vp++, v++) ;

	/*
	 *  if more than one vector exists, find the one that generates
	 *  the best confusions. 
	 */
	if (v > 1) {

		 /*
		  *  build the c_v structure, which maintains the association
		  *  between the vectors and their confusions.  
		  */
		 if (!(confusion_vector_list = (struct c_v *) malloc 
			   (sizeof(struct c_v)*v))) {
			  perror(prog);
			  return(NULL);
		 }

		 /*
		  *  generate the confusions.
		  */
		 for (i=0; i<v; i++) {
			  confusion_vector_list[i].vector = Vectors[i];
			  confusion_vector_list[i].cnf = confusion_generation
				   (string1, string2, Vectors[i]);
		 }

		 /*
		  *  compare confusion/vectors to pick the best one.
		  */
		 best = NULL;
		 for (left=confusion_vector_list, i=1; i<v; 
			  left=best, i++) {
			  best = compare_confusion_vectors(left, confusion_vector_list+i, 
											   m, n);
		 }

		 if (LCStrace) {
			  printf("\nbest match:\t[ ");
			  for (vpp = best->vector; *vpp>=0; vpp++) {
				   printf ("%d ", *vpp);
			  }
			  printf ("]\n");
		 }


		 return (best->cnf);

	} else {
		 /*
		  *  only one vector; calculate its confusions.
		  */
		 return (confusion_generation (string1, string2, *Vectors));
	}
}




static struct c_v *compare_confusion_vectors(A, B, m, n)
    struct c_v *A, *B;
    int m, n;
{
	 int *vpp;
	 int A_groups, A_cnfs, A_diff, B_groups, B_cnfs, B_diff;

	 vector_confusion_values(A, &A_groups, &A_cnfs, &A_diff, m, n);
	 vector_confusion_values(B, &B_groups, &B_cnfs, &B_diff, m, n);

	 if (CNFtrace) {
		  printf ("vector A  [ ");
		  for (vpp = A->vector; *vpp>=0; vpp++) {
			   printf ("%d ", *vpp);
		  }
		  printf ("] has %d groups, %d confusions, and a difference of %d\n",
				  A_groups, A_cnfs, A_diff);
		  
		  printf ("vector B  [ ");
		  for (vpp = B->vector; *vpp>=0; vpp++) {
			   printf ("%d ", *vpp);
		  }
		  printf ("] has %d groups, %d confusions, and a difference of %d\n",
				  B_groups, B_cnfs, B_diff);
	 }

	 if (A_groups > B_groups) 
		  return (B);
	 else if (B_groups > A_groups)
		  return (A);
	 if (A_cnfs > B_cnfs) 
		  return (B);
	 else if (B_cnfs > A_cnfs)
		  return (A);
	 else if (A_diff > B_diff)
		  return (B);
	 else if (B_diff > A_diff)
		  return (A);
	 else 
		  return(A);
}






static struct confusion 
*confusion_generation (string1, string2, V)
  char *string1, *string2;
  int *V;
{
	 int i, in_group, groups, cnfs, expected;
	 int difference, total_difference=0;
	 int start_position=-1, end_position=-1, start_index=1, end_index=1;

	 int m = strlen(string1);
	 int n = strlen(string2);

	 struct confusion *confusions = NULL, *new_cnf, *last_cnf;


	 /*
	  *  bullshit
	  */
	 if (CNFtrace) {
		  printf("\nvector\t[ ");
		  for (i=0; i<n; i++) printf("%d ", V[i]);
		  printf("]\n");
	 }

	/*  if there are no vectors, the strings are completely different;
	 *  therefore, return the confusion with the complete strings.
	 */
	 if (V == NULL) {

		  return (new_confusion(string1, string2, &difference,
								n, 0, m, 1));
	 }

	 /*
	  *  process vector
	  */
	 for (i=0, groups=0, cnfs=0, in_group=0, expected=1; i<n; ) {

		  /* 
		   *  iterate over all sequential vector entries.
		   */
		  while (V[i] == expected && i<n) {
			   expected++;
			   i++;
			   in_group = 1;
		  }

		  /*
		   *  keep track of the ending.
		   */
		  end_position = i;
		  end_index = expected;

		  if (in_group) groups++;
		  in_group = 0;

		  /*
		   *  iterate over all zero-entries.
		   */
		  while (V[i] == 0 && i<n) {
			   i++;
		  }

		  /*
		   *  calculate the substitution/insertion beginning.
		   */
		  start_position = i - 1;
		  start_index = (i == n) ? m: V[i] - 1;

		  /*
		   *  if this is the last iteration/grouping and the
		   *  last index has been accounted for, no confusion
		   *  should be generated.
		   */
		  if (i < n || (expected <= m || V[n-1] != m)) {

			   /*
				*  build a new confusion structure
				*/
			   if (CNFtrace)  printf("[%d] ", ++cnfs);
			   if (!(new_cnf = new_confusion(string1, string2, &difference,
											  start_position, end_position, 
											  start_index, end_index))) {
					return (NULL);
			   }
			   
			   /*
				*  add the confusion to the list
				*/
			   if (!confusions) 
					confusions = last_cnf = new_cnf;
			   else {
					last_cnf->next = new_cnf;
					last_cnf = new_cnf;
			   }
			   
			   total_difference += difference;
			   
			   expected = V[i];
		  }
	 }


	 if (CNFtrace) 
		  printf(
		  "* for %d groups, %d confusions, and a difference of %d.\n\n", 
			groups, cnfs, total_difference);


	 return (confusions);
}







static struct confusion 
*new_confusion(string1, string2, diff, start_pos, end_pos, start_idx, end_idx)
  char *string1, *string2;
  int *diff;
  int start_pos, end_pos, start_idx, end_idx;
{
	 struct confusion *new_cnf;

	 if (!(new_cnf = (struct confusion *) malloc 
		   (sizeof(struct confusion)))) {
		  perror(prog);
		  return(NULL);
	 }
	 memset(new_cnf->a, '\0', CONFUSION_LEN);
	 memset(new_cnf->b, '\0', CONFUSION_LEN);
	 new_cnf->next = NULL;

	 end_idx--;
	 start_idx--;

	 /*
	  * copy portions of the strings into the confusion buffers.
	  */
	 if (start_pos >= end_pos && end_pos >= 0) 
		  strncpy(new_cnf->a, string2+end_pos, 
				  start_pos-end_pos+1);
	 if (start_idx >= end_idx && end_idx >= 0) 
		  strncpy(new_cnf->b, string1+end_idx, 
				  start_idx-end_idx+1);

	 /*
	  *  calculate the size difference
	  */
	 *diff = abs((start_idx - end_idx + 1) - (start_pos - end_pos + 1));

	 /*
	  *  bullshit
	  */
	 if (CNFtrace) {
		  printf("replacing positions %d-%d with indices %d-%d\n",
				 end_pos, start_pos, 
				 end_idx+1, start_idx+1);
		  printf("\t`%s' -> `%s'\tfor a difference of %d\n", 
				 new_cnf->a, new_cnf->b, 
				 *diff);
	 }

	 return (new_cnf);
}






static void vector_confusion_values(V, V_groups, V_cnfs, V_diff, m, n)
  struct c_v *V;
  int *V_groups, *V_cnfs, *V_diff, m, n;
{
	 int i, len, in_group, groups, expected;
	 int *Vec = V->vector;
	 struct confusion *cnf;

	 for (i=0; Vec[i]>=0; i++) ;
	 len = i;

	 for (i=0, groups=0, in_group=0, expected=1; i<len; ) {

		  /* 
		   *  iterate over all sequential vector entries.
		   */
		  while (Vec[i] == expected && i<len) {
			   expected++;
			   i++;
			   in_group = 1;
		  }

		  if (in_group) groups++;
		  in_group = 0;

		  /*
		   *  iterate over all zero-entries.
		   */
		  while (Vec[i] == 0 && i<len) {
			   i++;
		  }

		  /*
		   *  if this is the last iteration/grouping and the
		   *  last index has been accounted for, no confusion
		   *  should be generated.
		   */
		  if (i < len || expected <= m) {
			   expected = Vec[i];
		  }
	 }

	 for (cnf=V->cnf, *V_cnfs=0, *V_diff=0; cnf; (*V_cnfs)++, cnf=cnf->next) 
	 {
		  *V_diff += abs( strlen(cnf->a) - strlen(cnf->b) );
	 }

	 *V_groups = groups;
}
