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
/*==============================================================
 * sequence.c -- Sequence data type
 * Copyright (c) 1992-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 29 Sep 93
 *   2.3.6 - 01 Jan 94    3.0.0 - 09 May 94
 *   3.0.2 - 24 Nov 94
 *============================================================*/

#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#include "sequence.h"

static BOOLEAN fndel (SEQUENCE, ELEMENT, INT *);

INT seqerr = NONE;
/*==========================
 * crtseq -- Create sequence
 *========================*/
SEQUENCE
crtseq (INT inc) /* incr length */
{
	SEQUENCE seq = (SEQUENCE) stdalloc(sizeof *seq);
	seqerr = NONE;
	if (inc < 1) {
		seqerr = INCERR;
		inc = 1;
	}
	Size(seq) = 0;
	Max(seq) = Inc(seq) = inc;
	Data(seq) = (ELEMENT *) stdalloc(inc*sizeof(ELEMENT));
	return seq;
}
/*==========================
 * rmvseq -- Remove sequence
 *========================*/
void
rmvseq (SEQUENCE seq)
{
	stdfree(Data(seq));
	stdfree(seq);
}
/*========================
 * cpyseq -- Copy sequence
 *======================*/
SEQUENCE
cpyseq (SEQUENCE seq)
{
	INT i, n;
	SEQUENCE new = (SEQUENCE) stdalloc(sizeof *new);
	Size(new) = Size(seq);
	Max(new) = Max(seq);
	Inc(new) = Inc(seq);
	Data(new) = (ELEMENT *) stdalloc(Max(seq)*sizeof(ELEMENT));
	for (i = 0, n = Size(seq); i < n; i++)
		Data(new)[i] = Data(seq)[i];
	return new;
}
/*=====================================
 * insseq -- Insert element in sequence
 *===================================*/
SEQUENCE
insseq (SEQUENCE seq,   /* sequence */
        INT idx,        /* index of el */
        ELEMENT el)     /* value of el */
{
	INT i, n;
	ELEMENT *d;
	seqerr = NONE;
	if (idx < 0 || idx > (n = Size(seq))) {
		seqerr = RANGE;
		return seq;
	}
	(void) tinsseq(seq, (ELEMENT) 0);
	d = Data(seq);
	for (i = n; i > idx; --i) 
		d[i] = d[i-1];
	d[idx] = el;
	return seq;
}
/*======================================
 * elseq -- Select element from sequence
 *====================================*/
ELEMENT
elseq (SEQUENCE seq,   /* sequence */
       INT idx)        /* index */
{
	seqerr = NONE;
	if (idx < 0 || idx >= Size(seq))  {
		seqerr = RANGE;
		return (ELEMENT) 0;
	}
	return Data(seq)[idx];
}
/*=====================================
 * altseq -- Change element in sequence
 *===================================*/
SEQUENCE
altseq (SEQUENCE seq,   /* sequence */
        INT idx,        /* index */
        ELEMENT new)    /* new value */
{
	seqerr = NONE;
	if (idx < 0 || idx >= Size(seq))  {
		seqerr = RANGE;
		return seq;
	}
	Data(seq)[idx] = new;
	return seq;
}
/*==================================================
 * delseq -- Remove and return element from sequence
 *================================================*/
ELEMENT
delseq (SEQUENCE seq,   /* sequence */
        ELEMENT val)    /* el value */
{
	INT i, n, loc;
	ELEMENT el, *d;
	seqerr = NONE;
	if (!fndel(seq, val, &loc)) return 0;
	if (loc < 0 || loc >= (n = Size(seq)))  {
		seqerr = RANGE;
		return 0;
	}
	d = Data(seq);
	el = d[loc];
	for (i = loc; i < n-1; i++)
		d[i] = d[i+1];
	--Size(seq);
	return el;
}
/*===============================================
 * hinsseq -- Insert new element at sequence head
 *=============================================*/
SEQUENCE
hinsseq (SEQUENCE seq,   /* sequence */
         ELEMENT el)     /* element */
{
	INT i, n;
	ELEMENT *old = Data(seq);
	if ((n = Size(seq)) == Max(seq)) {
		ELEMENT *new = (ELEMENT *) stdalloc(sizeof(ELEMENT)*
		    (n + Inc(seq)));
		for (i = n; i > 0; --i)
 			new[i] = old[i-1];
		stdfree(old);
		Data(seq) = old = new;
		Max(seq) += Inc(seq);
	}
	else
		for (i = n; i > 0; --i)
			old[i] = old[i-1];
	old[0] = el;
	Size(seq)++;
	return seq;
}
/*==================================================
 * tinsseq -- Insert new element at tail of sequence
 *================================================*/
SEQUENCE
tinsseq (SEQUENCE seq,
         ELEMENT el)
{
	INT i, n;
	ELEMENT *old = Data(seq);
	if ((n = Size(seq)) == Max(seq))  {
		ELEMENT *new = (ELEMENT *) stdalloc(sizeof(ELEMENT)*
		    (n + Inc(seq)));
		for (i = 0; i < n; i++)
			new[i] = old[i];
		stdfree(old);
		Data(seq) = old = new;
		Max(seq) += Inc(seq);
	}
	old[Size(seq)++] = el;
	return seq;
}
/*=======================================================
 *  hdelseq -- Remove and return element at sequence head
 *=====================================================*/
ELEMENT
hdelseq (SEQUENCE seq)
{
	INT i, n;
	ELEMENT e, *d;
	seqerr = NONE;
	if ((n = Size(seq)) == 0)  {
		seqerr = EMPTY;
		return (ELEMENT) 0;
	}
	d = Data(seq);
	e = d[0];
	for (i = 1; i < n; i++)
		d[i-1] = d[i];
	--Size(seq);
	return e;
}
/*======================================================
 * tdelseq -- Remove and return element at sequence tail
 *====================================================*/
ELEMENT
tdelseq (SEQUENCE seq)
{
	seqerr = NONE;
	if (Size(seq) == 0)  {
		seqerr = EMPTY;
		return (ELEMENT) 0;
	}
	return Data(seq)[--Size(seq) - 1];
}
/*======================================================
 * sinsseq -- Insert item to sorted location in sequence
 *====================================================*/
SEQUENCE
sinsseq (SEQUENCE seq,   /* sequence */
         ELEMENT el)     /* element */
{
	INT i;
	ELEMENT *d=0;
	(void) tinsseq(seq, (ELEMENT) 0);
	d = Data(seq);
	i = Size(seq) - 1;
	while (i > 0 && el < d[i-1])  {
		d[i] = d[i-1];
		--i;
	}
	d[i] = el;
	return seq;
}
/*=====================================================
 * fndel -- Search sequence for value - return location
 *===================================================*/
static BOOLEAN
fndel (SEQUENCE seq,   /* sequence */
       ELEMENT val,    /* element value */
       INT *loc)       /* index if found */
{
	INT i, n;
	ELEMENT *d = Data(seq);
	n = Size(seq);
	*loc = -1;
	for (i = 0; i < n; i++) {
		if (val == d[i]) {
			*loc = i;
			return TRUE;
		}
	}
	return FALSE;
}
/*===========================================================
 * sfndel -- Search sorted sequence for key - return location
 *=========================================================*/
#ifdef UNUSED_CODE
static BOOLEAN
sfndel (SEQUENCE seq,   /* sequence */
        ELEMENT key,    /* search key */
        INT *loc)       /* index if found */
{
	INT lo, hi, md, v;
	ELEMENT *d = Data(seq);
	lo = 0;
	hi = Size(seq) - 1;
	while (lo <= hi)  {
		md = (lo + hi) / 2;
		if ((v = (INT) (key - d[md])) == 0)  {
			*loc = md;
			return TRUE;
		}
		if (v < 0)
			hi = md - 1;
		else
			lo = md + 1;
	}
	return FALSE;
}
#endif
/*===============================================
 * joinseq -- Join two sequences - first modified
 *=============================================*/
SEQUENCE
joinseq (SEQUENCE seq1,
         SEQUENCE seq2)
{
	INT i, n = Size(seq2);
	for (i = 0;  i < n;  i++)
		(void) tinsseq(seq1, (Data(seq2))[i]);
	return seq1;
}
