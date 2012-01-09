# 
# Copyright (c) 1995 Regents of the University of Nevada
#
# Makefile for subsequence programs
# Ocrspell OCR-based statistical spell checker
# Copyright (C) 1995 Regents of the University of Nevada
# Text Retrieval Group
# Information Science Research Institute
# University of Nevada, Las Vegas
# Las Vegas, NV 89154-4021
# isri-text@isri.unlv.edu.



CC= gcc
CFLAGS= -g -Wall 
LDFLAGS= -g 
LIBS= 
SRCS= subseq.c lcs.c lcs2cnf.c cnf.c
OBJS= subseq.o lcs.o lcs2cnf.o cnf.o

all : lcs2cnf

lcs : subseq.o lcs.o 
	$(CC) -o lcs subseq.o lcs.o $(LDFLAGS) $(LIBS)

lcs2cnf : subseq.o lcs2cnf.o cnf.o
	$(CC) -o lcs2cnf subseq.o lcs2cnf.o cnf.o $(LDFLAGS) $(LIBS)

depend : $(SRCS)
	makedepend -- $(CFLAGS) -- $(SRCS)


clean :
	-rm -f *~ core $(OBJS) lcs lcs2cnf




# DO NOT DELETE THIS LINE -- make depend depends on it.

subseq.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
subseq.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
subseq.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
subseq.o: /usr/include/memory.h /usr/include/sys/types.h
subseq.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
subseq.o: /usr/include/sys/select.h /usr/include/sys/time.h
subseq.o: /usr/include/sys/time.h /usr/include/errno.h
subseq.o: /usr/include/sys/errno.h /usr/include/string.h subseq.h
lcs.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
lcs.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
lcs.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
lcs.o: /usr/include/memory.h /usr/include/sys/types.h
lcs.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
lcs.o: /usr/include/sys/select.h /usr/include/sys/time.h
lcs.o: /usr/include/sys/time.h /usr/include/malloc.h /usr/include/string.h
lcs.o: subseq.h
lcs2cnf.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
lcs2cnf.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
lcs2cnf.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
lcs2cnf.o: /usr/include/memory.h /usr/include/sys/types.h
lcs2cnf.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
lcs2cnf.o: /usr/include/sys/select.h /usr/include/sys/time.h
lcs2cnf.o: /usr/include/sys/time.h /usr/include/malloc.h
lcs2cnf.o: /usr/include/string.h subseq.h cnf.h
cnf.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
cnf.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
cnf.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
cnf.o: /usr/include/memory.h /usr/include/sys/types.h
cnf.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
cnf.o: /usr/include/sys/select.h /usr/include/sys/time.h
cnf.o: /usr/include/sys/time.h /usr/include/malloc.h /usr/include/string.h
cnf.o: subseq.h cnf.h
