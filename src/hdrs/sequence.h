/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
/*=================================================================
 * sequence.h -- Defines type and macros for sequence data type.
 * Copyright (c) 1991 by Thomas T. Wetmore IV; all rights reserved.
 *   Version 2.3.4 - 24 Jun 93 - controlled
 *   Version 2.3.5 - 20 Aug 93 - modified
 *==================================================================
 */

#ifndef _SEQUENCE_H
#define _SEQUENCE_H

#include "standard.h"

#define ELEMENT int

typedef struct  {
	int size;	/*  current length of list  */
	int smax;	/*  max length before increment  */
	int inc;	/*  increment size  */
	ELEMENT *data;	/*  actual list of items */
} *SEQUENCE;
#define Size(s)  ((s)->size)
#define Max(s)   ((s)->smax)
#define Inc(s)   ((s)->inc)
#define Data(s)  ((s)->data)

#define NONE    0
#define INCERR  1
#define RANGE   2
#define EMPTY   3

extern INT seqerr;

#define lenseq(seq)  (Size(seq))

SEQUENCE crtseq(INT);
void rmvseq(SEQUENCE);
SEQUENCE cpyseq(SEQUENCE);
SEQUENCE insseq(SEQUENCE, INT, ELEMENT);
ELEMENT elseq(SEQUENCE, INT);
SEQUENCE altseq(SEQUENCE, INT, ELEMENT);
ELEMENT delseq(SEQUENCE, ELEMENT);
SEQUENCE hinsseq(SEQUENCE, ELEMENT);
SEQUENCE tinsseq(SEQUENCE, ELEMENT);
ELEMENT hdelseq(SEQUENCE);
ELEMENT tdelseq(SEQUENCE);
SEQUENCE sinsseq(SEQUENCE, ELEMENT);
SEQUENCE joinseq(SEQUENCE, SEQUENCE);

#define FORALL(i,e,s)\
	{	int i, _n;\
		ELEMENT e;\
		for (i = 0, _n = Size(s); i < _n; i++) {\
			e = Data(s)[i];
#define ENDLOOP }}

#endif /* _SEQUENCE_H */
