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
/*=============================================================
 * indiseq.c -- Person sequence operations
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 25 Aug 93
 *   3.0.0 - 09 May 94    3.0.2 - 23 Dec 94
 *===========================================================*/
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/* modified 2000-01-26 J.F.Chandler */
/* modified 2000-08-21 J.F.Chandler */

#include "sys_inc.h"
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "indiseq.h"
#include "interp.h"
#include "gedcomi.h"

/* WARNING: append_indiseq allows an extra value to be stored.
 * this value must be a PVALUE if it is to be used in report programs.
 * However, internally when browsing, a family number is inserted as
 * an integer, not an integer (PINT) PVALUE. This may cause problems
 * if the value associated with an individual in a set is attempted
 * to be used in a report program - pbm 17-Feb-97
 */
/*
Perry is beginning a revision of indiseq
to differentiate INT, WORD, and STRING value style
indiseqs
Perry, 2001/01/05
*/

extern BOOLEAN opt_finnish;		/* Finnish language support */

#define key_to_name(k)  nval(NAME(key_to_indi(k)))

/*====================
 * indiseq print types
 *==================*/
#define ISPRN_NORMALSEQ 0
#define ISPRN_FAMSEQ 1
#define ISPRN_SPOUSESEQ 2

static STRING get_print_el (INDISEQ, INT);
static void append_all_tags(INDISEQ, NODE, STRING, BOOLEAN);

/* Matt 1/1/1 - should these be static, or will they be used elsewhere? */
static INT name_compare (SORTEL, SORTEL);
static INT key_compare (SORTEL, SORTEL);
static INT canonkey_order(char c);
static INT canonkey_compare(SORTEL el1, SORTEL el2);
INT value_str_compare (SORTEL, SORTEL);
INT value_compare (SORTEL, SORTEL);

/*==================================
 * create_indiseq -- Create sequence
 *================================*/
INDISEQ
create_indiseq (void)
{
	INDISEQ seq = (INDISEQ) stdalloc(sizeof *seq);
	ISize(seq) = 0;
	IMax(seq) = 20;
	IData(seq) = (SORTEL *) stdalloc(20*sizeof(SORTEL));
	IFlags(seq) = 0;
	IPrntype(seq) = ISPRN_NORMALSEQ;
	return seq;
}
/*==================================
 * remove_indiseq -- Remove sequence
 *================================*/
void
remove_indiseq (INDISEQ seq,
                BOOLEAN fval)   /* free values? */
{
	SORTEL *d = IData(seq);
	INT i, n = ISize(seq);
	for (i = 0; i < n; i++, d++) {
		stdfree(skey(*d));
		if (snam(*d)) stdfree(snam(*d));
		if (fval && sval(*d)) stdfree(sval(*d));
		if (sprn(*d)) stdfree(sprn(*d));
	}
	stdfree(IData(seq));
	stdfree(seq);
}
/*==============================
 * copy_indiseq -- Copy sequence
 *============================*/
INDISEQ
copy_indiseq (INDISEQ seq)
{
	INDISEQ newseq;
	if (!seq) return NULL;
	newseq = create_indiseq();
	FORINDISEQ(seq, el, num)
		append_indiseq(newseq, skey(el), snam(el), sval(el),
		    TRUE, FALSE);
	ENDINDISEQ
	return newseq;
}
/*==================================================
 * append_indiseq_ival -- Append element to sequence
 *  with INT value
 *================================================*/
void
append_indiseq_ival (INDISEQ seq,    /* sequence */
                     STRING key,     /* key - not NULL */
                     STRING name,    /* name - may be NULL */
                     INT val,       /* extra val */
                     BOOLEAN sure,   /* no dupe check? */
                     BOOLEAN alloc)  /* key alloced? */
{
	WORD wval=(WORD)val;
	/*
	to do - verify that this is an ival indiseq
	*/
	append_indiseq(seq, key, name, wval, sure, alloc);
}
/*==================================================
 * append_indiseq_sval -- Append element to sequence
 *  with STRING value
 *================================================*/
void
append_indiseq_sval (INDISEQ seq,    /* sequence */
                     STRING key,     /* key - not NULL */
                     STRING name,    /* name - may be NULL */
                     STRING sval,       /* extra val */
                     BOOLEAN sure,   /* no dupe check? */
                     BOOLEAN alloc)  /* key alloced? */
{
	WORD wval=(WORD)sval;
	/*
	to do - verify that this is a sval indiseq
	*/
	append_indiseq(seq, key, name, wval, sure, alloc);
}
/*=============================================
 * append_indiseq -- Append element to sequence
 *  with pvalue
 *===========================================*/
void
append_indiseq (INDISEQ seq,    /* sequence */
                STRING key,     /* key - not NULL */
                STRING name,    /* name - may be NULL */
                WORD val,       /* extra val is pointer */
                BOOLEAN sure,   /* no dupe check? */
                BOOLEAN alloc)  /* key alloced? */
{
	INT i, m, n;
	SORTEL el, *new, *old;
	if (!seq || !key) return;
	n = ISize(seq);
	old = IData(seq);
	if (!sure) {
		/* Perry, 2000/11/28 - I'm skipping dupcheck
		for FAMs for compatibility, but I don't know
		why FAM seqs didn't do dupcheck */
		BOOLEAN dupcheck = (*key != 'F' && *key != 'I')
			|| (*key == 'I' && !name);
		if (dupcheck)
		{
			for (i = 0; i < n; i++) {
				if (eqstr(key, skey(old[i]))) return;
			}
		}
	}
	el = (SORTEL) stdalloc(sizeof(*el));
	skey(el) = alloc ? key : strsave(key);
	snam(el) = NULL;
	if (*key == 'I') {
		if (name)
			snam(el) = strsave(name);
		else
			snam(el) = strsave(key_to_name(key));
	}
	sval(el) = val;
	spri(el) = 0;
	sprn(el) = NULL;
	if ((n = ISize(seq)) >= IMax(seq))  {
		m = 3*n;
		new = (SORTEL *) stdalloc(m*sizeof(SORTEL));
		for (i = 0; i < n; i++)
			new[i] = old[i];
		stdfree(old);
		IData(seq) = old = new;
		IMax(seq) = m;
	}
	old[ISize(seq)++] = el;
	IFlags(seq) = 0;
}
/*=========================================================
 * rename_indiseq -- Update element name with standard name
 *=======================================================*/
void
rename_indiseq (INDISEQ seq,
                STRING key)     /* key of el to modify */
{
	INT i, n;
	SORTEL *data;
	if (!seq || !key || *key != 'I') return;
	n = ISize(seq);
	data = IData(seq);
	for (i = 0; i < n; i++) {
		if (eqstr(key, skey(data[i]))) {
			if (snam(data[i])) stdfree(snam(data[i]));
			snam(data[i]) = strsave(key_to_name(key));
		}
	}
}
/*==============================================
 * in_indiseq -- See if element is in an INDISEQ
 *=============================================*/
BOOLEAN
in_indiseq (INDISEQ seq,    /* sequence */
            STRING key)     /* key */
{
	INT i, len;
	SORTEL *data;

	if (!seq || !key) return FALSE;
	len = ISize(seq);
	data = IData(seq);
	for (i = 0; i < len; i++) {
		if (eqstr(key, skey(data[i]))) return TRUE;
	}
	return FALSE;
}
/*===============================================================
 * delete_indiseq -- Remove el from sequence; if key not NULL use
 *   it; else use index, rel 0; el must be person
 *==============================================================*/
BOOLEAN
delete_indiseq (INDISEQ seq,    /* sequence */
                STRING key,     /* key - may be NULL */
                STRING name,    /* name - may be NULL */
                INT index)      /* index of el to remove - may be computed */
{
	INT i, len;
	SORTEL *data, el;
	if (!seq) return FALSE;
	len = ISize(seq);
	data = IData(seq);
	if (key) {
		if (*key != 'I') return FALSE;
		for (i = 0; i < len; i++) {
			if (eqstr(key, skey(data[i])) && (!name ||
			    eqstr(name, snam(data[i])))) break;
		}
		if (i >= len) return FALSE;
		index = i;
	}
	if (index < 0 || index >= len) return FALSE;
	len--;
	el = data[index];
	for (i = index; i < len; i++)
		data[i] = data[i+1];
	ISize(seq)--;
	stdfree(skey(el));
	if (snam(el)) stdfree(snam(el));
	if (sprn(el)) stdfree(sprn(el));
	stdfree(el);
	return TRUE;
}
/*================================================
 * element_indiseq -- Return element from sequence
 *==============================================*/
BOOLEAN
element_indiseq (INDISEQ seq,    /* sequence */
                 INT index,      /* index */
                 STRING *pkey,   /* returned key */
                 STRING *pname)  /* returned name */
{
	*pkey = *pname = NULL;
	if (!seq || index < 0 || index > ISize(seq) - 1) return FALSE;
	*pkey =  skey(IData(seq)[index]);
	*pname = snam(IData(seq)[index]);
	return TRUE;
}
/*================================================
 * elementval_indiseq -- Return element & value from sequence
 *==============================================*/
BOOLEAN
elementval_indiseq (INDISEQ seq,    /* sequence */
                    INT index,      /* index */
                    STRING *pkey,   /* returned key */
                    INT *pval,      /* returned val */
                    STRING *pname)  /* returned name */
{
	*pkey = *pname = NULL;
	if (!seq || index < 0 || index > ISize(seq) - 1) return FALSE;
	*pkey =  skey(IData(seq)[index]);
	*pval = sval(IData(seq)[index]);
	*pname = snam(IData(seq)[index]);
	return TRUE;
}
/*==================================
 * name_compare -- Compare two names
 *================================*/
INT
name_compare (SORTEL el1,
              SORTEL el2)
{
	INT rel = namecmp(snam(el1), snam(el2));
	if (rel) return rel;
	return spri(el1) - spri(el2);
}
/*================================
 * key_compare -- Compare two keys
 * also used for integer value sort
 *==============================*/
INT
key_compare (SORTEL el1, SORTEL el2)
{
	return spri(el1) - spri(el2);
}
/*===========================================
 * canonkey_order -- Canonical order of a type
 *  letter (I,F,S,E,X)
 *=========================================*/
static INT
canonkey_order (char c)
{
	switch(c) {
	case 'I': return 0;
	case 'F': return 1;
	case 'S': return 2;
	case 'E': return 3;
	default: return 4;
	}
}
/*================================
 * canonkey_compare -- Compare two keys
 * in canonical key order (I,F,S,E,X)
 *==============================*/
static INT
canonkey_compare (SORTEL el1, SORTEL el2)
{
	char c1=skey(el1)[0], c2=skey(el2)[0];
	if (c1 == c2)
		return spri(el1) - spri(el2);
	return canonkey_order(c1) - canonkey_order(c2);
}
/*===================================================
 * value_str_compare -- Compare two values as strings
 *=================================================*/
INT
value_str_compare (SORTEL el1, SORTEL el2)
{
	PVALUE val1, val2;
	val1 = sval(el1);
	val2 = sval(el2);
	return ll_strcmp((STRING) pvalue(val1), (STRING) pvalue(val2));
}
/*====================================
 * value_compare -- Compare two values
 *==================================*/
INT
value_compare (SORTEL el1, SORTEL el2)
{
	/* WARNING: this is not correct as sval() is a PVALUE structure */
	return (INT) sval(el1) - (INT) sval(el2);
}
/*==========================================
 * namesort_indiseq -- Sort sequence by name
 *========================================*/
void
namesort_indiseq (INDISEQ seq)
{
	if (IFlags(seq) & NAMESORT) return;
	FORINDISEQ(seq, el, num)
		spri(el) = atoi(skey(el) + 1);
	ENDINDISEQ
	partition_sort(IData(seq), ISize(seq), name_compare);
	IFlags(seq) &= ~ALLSORTS;
	IFlags(seq) |= NAMESORT;
}
/*========================================
 * keysort_indiseq -- Sort sequence by key
 *======================================*/
void
keysort_indiseq (INDISEQ seq)
{
	if (IFlags(seq) & KEYSORT) return;
	FORINDISEQ(seq, el, num)
		spri(el) = atoi(skey(el) + 1);
	ENDINDISEQ
	partition_sort(IData(seq), ISize(seq), key_compare);
	IFlags(seq) &= ~ALLSORTS;
	IFlags(seq) |= KEYSORT;
}
/*=============================================
 * canonkeysort_indiseq -- Sort sequence by key
 *  in key canonical order (I,F,S,E,X)
 *===========================================*/
void
canonkeysort_indiseq (INDISEQ seq)
{
	if (IFlags(seq) & CANONKEYSORT) return;
	FORINDISEQ(seq, el, num)
		spri(el) = atoi(skey(el) + 1);
	ENDINDISEQ
	partition_sort(IData(seq), ISize(seq), canonkey_compare);
	IFlags(seq) &= ~ALLSORTS;
	IFlags(seq) |= CANONKEYSORT;
}
/*============================================
 * valuesort_indiseq -- Sort sequence by value
 *==========================================*/
void
valuesort_indiseq (INDISEQ seq,
                   BOOLEAN *eflg)
{
	PVALUE val;
	SORTEL *data;
	int settype;
	if (IFlags(seq) & VALUESORT) return;
	data = IData(seq);
	val = sval(*data);
	if ((settype = ptype(val)) != PINT && settype != PSTRING ) {
		*eflg = TRUE;
		return;
	}
	FORINDISEQ(seq, el, num)
		if (!(val = sval(el)) || ptype(val) != settype) {
			*eflg = TRUE;
			return;
		}
		if (settype == PINT) spri(el) = (INT)pvalue(val);
	ENDINDISEQ
  /* OLD: partition_sort(IData(seq), ISize(seq), value_compare); */
	if (settype == PINT)
		partition_sort(IData(seq), ISize(seq), key_compare);
	else
		partition_sort(IData(seq), ISize(seq), value_str_compare);
	IFlags(seq) &= ~ALLSORTS;
	IFlags(seq) |= VALUESORT;
}
/*=========================================
 * partition_sort -- Partition (quick) sort
 *=======================================*/
#define LNULL -1
static SORTEL *ldata;
static INT (*lcmp)(SORTEL, SORTEL);

void
partition_sort (SORTEL *data,   /* array of els to sort */
                INT len,        /* len of array */
                INT (*cmp)(SORTEL, SORTEL))   /* compare function */
{
	ldata = data;
	lcmp = cmp;
	llqsort(0, len-1);
}
/*======================================
 * llqsort -- Recursive core of quick sort
 *====================================*/
void
llqsort (INT left,        /* range to sort */
         INT right)
{
	INT pcur = getpivot(left, right);
	if (pcur != LNULL) {
		SORTEL pivot = ldata[pcur];
		INT mid = partition(left, right, pivot);
		llqsort(left, mid-1);
		llqsort(mid, right);
	}
}
/*====================================
 * partition -- Partition around pivot
 *==================================*/
INT
partition (INT left,
           INT right,
           SORTEL pivot)
{
	INT i = left, j = right;
	do {
		SORTEL tmp = ldata[i];
		ldata[i] = ldata[j];
		ldata[j] = tmp;
		while ((*lcmp)(ldata[i], pivot) < 0)
			i++;
		while ((*lcmp)(ldata[j], pivot) >= 0)
			j--;
	} while (i <= j);
	return i;
}
/*=============================
 * getpivot -- Choose key pivot
 *===========================*/
INT
getpivot (INT left,
          INT right)
{
	SORTEL pivot = ldata[left];
	INT left0 = left, rel;
	for (++left; left <= right; left++) {
		SORTEL next = ldata[left];
		if ((rel = (*lcmp)(next, pivot)) > 0) return left;
		if (rel < 0) return left0;
	}
	return LNULL;
}
/*==================================================================
 * unique_indiseq -- Remove identical (key, name) els from sequence
 * NOTE: this routine has MEMORY LEAK -- it doesn't free storage for
 *   els removed from sequence
 *================================================================*/
void
unique_indiseq (INDISEQ seq)
{
	INT i, j, n;
	SORTEL *d;
	if (!seq) return;
	n = ISize(seq);
	d = IData(seq);
	if (n == 0 || (IFlags(seq) & UNIQUED)) return;
	if (!(IFlags(seq) & KEYSORT)) keysort_indiseq(seq);
	for (j = 0, i = 1; i < n; i++)
		if (spri(d[i]) != spri(d[j])) d[++j] = d[i];
	ISize(seq) = j + 1;
	IFlags(seq) |= UNIQUED;
}
/*===============================================
 * union_indiseq -- Create union of two sequences
 *=============================================*/
INDISEQ
union_indiseq (INDISEQ one,
               INDISEQ two)
{
	INT n, m, i, j, rel;
	STRING key;
	INDISEQ three;
	SORTEL *u, *v;
	if (!one || !two) return NULL;
	if (!(IFlags(one) & KEYSORT)) keysort_indiseq(one);
	if (!(IFlags(one) & UNIQUED)) unique_indiseq(one);
	if (!(IFlags(two) & KEYSORT)) keysort_indiseq(two);
	if (!(IFlags(two) & UNIQUED)) unique_indiseq(two);
	n = length_indiseq(one);
	m = length_indiseq(two);
	three = create_indiseq();
	i = j = 0;
	u = IData(one);
	v = IData(two);
	while (i < n && j < m) {
		if ((rel = spri(u[i]) - spri(v[j])) < 0) {
			key = strsave(skey(u[i]));
			append_indiseq(three, key, NULL, sval(u[i]),
			    TRUE, TRUE);
			i++;
		} else if (rel > 0) {
			key = strsave(skey(v[j]));
			append_indiseq(three, key, NULL, sval(v[j]),
			    TRUE, TRUE);
			j++;
		} else {
			key = strsave(skey(u[i]));
			append_indiseq(three, key, NULL, sval(u[i]),
			    TRUE, TRUE);
			i++, j++;
		}
	}
	while (i < n) {
		key = strsave(skey(u[i]));
		append_indiseq(three, key, NULL, sval(u[i]), TRUE, TRUE);
		i++;
	}
	while (j < m) {
		key = strsave(skey(v[j]));
		append_indiseq(three, key, NULL, sval(v[j]), TRUE, TRUE);
		j++;
	}
	FORINDISEQ(three, el, num)
		spri(el) = atoi(skey(el) + 1);
	ENDINDISEQ
	IFlags(three) = KEYSORT|UNIQUED;
	return three;
}
/*==========================================================
 * intersect_indiseq -- Create intersection of two sequences
 *========================================================*/
INDISEQ
intersect_indiseq (INDISEQ one,
                   INDISEQ two)
{
	INT n, m, i, j, rel;
	STRING key;
	INDISEQ three;
	SORTEL *u, *v;
	if (!one || !two) return NULL;
	if (!(IFlags(one) & KEYSORT)) keysort_indiseq(one);
	if (!(IFlags(one) & UNIQUED)) unique_indiseq(one);
	if (!(IFlags(two) & KEYSORT)) keysort_indiseq(two);
	if (!(IFlags(two) & UNIQUED)) unique_indiseq(two);
	n = length_indiseq(one);
	m = length_indiseq(two);
	three = create_indiseq();
	i = j = 0;
	u = IData(one);
	v = IData(two);
	while (i < n && j < m) {
		if ((rel = spri(u[i]) - spri(v[j])) < 0) {
			i++;
		} else if (rel > 0) {
			j++;
		} else {
			key = strsave(skey(u[i]));
			append_indiseq(three, key, NULL, sval(u[i]),
			    TRUE, TRUE);
			i++, j++;
		}
	}
	FORINDISEQ(three, el, num)
		spri(el) = atoi(skey(el) + 1);
	ENDINDISEQ
	IFlags(three) = KEYSORT|UNIQUED;
	return three;
}
/*=========================================================
 * difference_indiseq -- Create difference of two sequences
 *=======================================================*/
INDISEQ
difference_indiseq (INDISEQ one,
                    INDISEQ two)
{
	INT n, m, i, j, rel;
	STRING key;
	INDISEQ three;
	SORTEL *u, *v;
	if (!one || !two) return NULL;
	if (!(IFlags(one) & KEYSORT)) keysort_indiseq(one);
	if (!(IFlags(one) & UNIQUED)) unique_indiseq(one);
	if (!(IFlags(two) & KEYSORT)) keysort_indiseq(two);
	if (!(IFlags(two) & UNIQUED)) unique_indiseq(two);
	n = length_indiseq(one);
	m = length_indiseq(two);
	three = create_indiseq();
	i = j = 0;
	u = IData(one);
	v = IData(two);
	while (i < n && j < m) {
		if ((rel = spri(u[i]) - spri(v[j])) < 0) {
			key = strsave(skey(u[i]));
			append_indiseq(three, key, NULL, sval(u[i]),
			    TRUE, TRUE);
			i++;
		} else if (rel > 0) {
			j++;
		} else {
			i++, j++;
		}
	}
	while (i < n) {
		key = strsave(skey(u[i]));
		append_indiseq(three, key, NULL, sval(u[i]), TRUE, TRUE);
		i++;
	}
	FORINDISEQ(three, el, num)
		spri(el) = atoi(skey(el) + 1);
	ENDINDISEQ
	IFlags(three) = KEYSORT|UNIQUED;
	return three;
}
/*=====================================================
 * parent_indiseq -- Create parent sequence of sequence
 *===================================================*/
INDISEQ
parent_indiseq (INDISEQ seq)
{
	TABLE tab;
	INDISEQ par;
	NODE indi, fath, moth;
	STRING key;
	if (!seq) return NULL;
	tab = create_table();
	par = create_indiseq();
	FORINDISEQ(seq, el, num)
		indi = key_to_indi(skey(el));
		fath = indi_to_fath(indi);
		moth = indi_to_moth(indi);
		if (fath && !in_table(tab, key = indi_to_key(fath))) {
			key = strsave(key);
			append_indiseq(par, key, NULL, sval(el), TRUE, TRUE);
			insert_table(tab, key, NULL);
		}
		if (moth && !in_table(tab, key = indi_to_key(moth))) {
			key = strsave(key);
			append_indiseq(par, key, NULL, sval(el), TRUE, TRUE);
			insert_table(tab, key, NULL);
		}
	ENDINDISEQ
	remove_table(tab, DONTFREE);
	return par;
}
/*======================================================
 * child_indiseq -- Create children sequence of sequence
 *====================================================*/
INDISEQ
child_indiseq (INDISEQ seq)
{
	INT num1, num2;
	TABLE tab;
	INDISEQ cseq;
	NODE indi;
	STRING key;
	if (!seq) return NULL;
	tab = create_table();
	cseq = create_indiseq();
	FORINDISEQ(seq, el, num)
		indi = key_to_indi(skey(el));
		FORFAMSS(indi, fam, spouse, num1)
			FORCHILDREN(fam, chil, num2)
				key = indi_to_key(chil);
				if (!in_table(tab, key)) {
					key = strsave(key);
					append_indiseq(cseq, key, NULL,
					    sval(el), TRUE, TRUE);
					insert_table(tab, key, NULL);
				}
			ENDCHILDREN
		ENDFAMSS
	ENDINDISEQ
	remove_table(tab, DONTFREE);
	return cseq;
}
/*=========================================================
 * indi_to_children -- Create sequence of person's children
 *=======================================================*/
INDISEQ
indi_to_children (NODE indi)
{
	INDISEQ seq;
	INT num1, num2, len = 0;
	STRING key;
	if (!indi) return NULL;
	seq = create_indiseq();
	FORFAMSS(indi, fam, spouse, num1)
		FORCHILDREN(fam, chil, num2)
			len++;
			key = indi_to_key(chil);
			append_indiseq(seq, key, NULL, NULL, TRUE, FALSE);
		ENDCHILDREN
	ENDFAMSS
	if (len) return seq;
	remove_indiseq(seq, FALSE);
	return NULL;
}
/*=======================================================
 * indi_to_spouses -- Create sequence of person's spouses
 *  values will be family keynums
 *  (this is the ISPRN_SPOUSESEQ style)
 *=====================================================*/
INDISEQ
indi_to_spouses (NODE indi)
{
	INDISEQ seq;
	INT num, num1, val, len = 0;
	STRING key;
	if (!indi) return NULL;
	seq = create_indiseq();
	IPrntype(seq) = ISPRN_SPOUSESEQ;
	FORFAMSS(indi, fam, spouse, num)
#if 0
		if (spouse) {
			len++;
			key = indi_to_key(spouse);
			val = atoi(fam_to_key(fam) + 1); /* PVALUE NEEDED */
			append_indiseq(seq, key, NULL, val, TRUE, FALSE);
		}
#else
		FORHUSBS(fam, husb, num1)
		    if(husb != indi) {
			len++;
			key = indi_to_key(husb);
			val = atoi(fam_to_key(fam) + 1); /* PVALUE NEEDED */
			append_indiseq_ival(seq, key, NULL, val, TRUE, FALSE);
		    }
		ENDHUSBS
		FORWIFES(fam, wife, num1)
		    if(wife != indi) {
			len++;
			key = indi_to_key(wife);
			val = atoi(fam_to_key(fam) + 1);
			append_indiseq_ival(seq, key, NULL, val, TRUE, FALSE);
		    }
		ENDWIFES
#endif
	ENDFAMSS
	if (!len) {
		remove_indiseq(seq, FALSE);
		seq=NULL;
	}
	return seq;
}
/*=======================================================
 * indi_to_fathers -- Create sequence of person's fathers
 *=====================================================*/
INDISEQ
indi_to_fathers (NODE indi)
{
	INDISEQ seq;
	INT num1, num2, len = 0;
	STRING key;
	if (!indi) return NULL;
	seq = create_indiseq();
	FORFAMCS(indi, fam, fath, moth, num1)
		FORHUSBS(fam, husb, num2)
			len++;
			key = indi_to_key(husb);
			append_indiseq(seq, key, NULL, NULL, TRUE, FALSE);
		ENDHUSBS
	ENDFAMCS
	if (len) return seq;
	remove_indiseq(seq, FALSE);
	return NULL;
}
/*=======================================================
 * indi_to_mothers -- Create sequence of person's mothers
 *=====================================================*/
INDISEQ
indi_to_mothers (NODE indi)
{
	INDISEQ seq;
	INT num1, num2, len = 0;
	STRING key;
	if (!indi) return NULL;
	seq = create_indiseq();
	FORFAMCS(indi, fam, fath, moth, num1)
		FORWIFES(fam, wife, num2)
			len++;
			key = indi_to_key(wife);
			append_indiseq(seq, key, NULL, NULL, TRUE, FALSE);
		ENDWIFES
	ENDFAMCS
	if (len) return seq;
	remove_indiseq(seq, FALSE);
	return NULL;
}
/*=========================================================
 * indi_to_families -- Create sequence of person's families
 *  values will be spouse keynums
 *  (this is the ISPRN_FAMSEQ style)
 *=======================================================*/
INDISEQ
indi_to_families (NODE indi,      /* person */
                  BOOLEAN fams)   /* families as spouse? */
{
	INDISEQ seq;
	INT num, val;
	STRING key;
	if (!indi) return NULL;
	seq = create_indiseq();
	IPrntype(seq) = ISPRN_FAMSEQ;
	if (fams) {
		FORFAMSS(indi, fam, spouse, num)
			key = fam_to_key(fam);
			val = spouse ? atoi(indi_to_key(spouse) + 1) : 0;
			append_indiseq_ival(seq, key, NULL, val, TRUE, FALSE);
		ENDFAMSS
	} else {
		FORFAMCS(indi, fam, fath, moth, num)
			key = fam_to_key(fam);
			val = fath ? atoi(indi_to_key(fath) + 1) : 0;
			append_indiseq_ival(seq, key, NULL, val, TRUE, FALSE);
		ENDFAMCS
	}
	if (num) return seq;
	remove_indiseq(seq, FALSE);
	return NULL;
}
/*========================================================
 * fam_to_children -- Create sequence of family's children
 *======================================================*/
INDISEQ
fam_to_children (NODE fam)
{
	INDISEQ seq;
	INT num;
	STRING key;
	if (!fam) return NULL;
	seq = create_indiseq();
	FORCHILDREN(fam, chil, num)
		key = indi_to_key(chil);
		append_indiseq(seq, key, NULL, NULL, TRUE, FALSE);
	ENDCHILDREN
	if (num) return seq;
	remove_indiseq(seq, FALSE);
	return NULL;
}
/*======================================================
 * fam_to_fathers -- Create sequence of family's fathers
 *====================================================*/
INDISEQ
fam_to_fathers (NODE fam)
{
	INDISEQ seq;
	INT num;
	STRING key;
	if (!fam) return NULL;
	seq = create_indiseq();
	FORHUSBS(fam, husb, num)
		key = indi_to_key(husb);
		append_indiseq(seq, key, NULL, NULL, TRUE, FALSE);
	ENDHUSBS
	if (num) return seq;
	remove_indiseq(seq, FALSE);
	return NULL;
}
/*======================================================
 * fam_to_mothers -- Create sequence of family's mothers
 *====================================================*/
INDISEQ
fam_to_mothers (NODE fam)
{
	INDISEQ seq;
	INT num;
	STRING key;
	if (!fam) return NULL;
	seq = create_indiseq();
	FORWIFES(fam, wife, num)
		key = indi_to_key(wife);
		append_indiseq(seq, key, NULL, NULL, TRUE, FALSE);
	ENDWIFES
	if (num) return seq;
	remove_indiseq(seq, FALSE);
	return NULL;
}
/*=========================================================
 * sibling_indiseq -- Create sibling sequence of a sequence
 *=======================================================*/
INDISEQ
sibling_indiseq (INDISEQ seq,
                 BOOLEAN close)
{
	INDISEQ fseq, sseq;
	NODE indi, fam;
	STRING key, fkey;
	INT num2;
	TABLE tab = create_table();
	fseq = create_indiseq();
	sseq = create_indiseq();
	FORINDISEQ(seq, el, num)
		indi = key_to_indi(skey(el));
		if ((fam = indi_to_famc(indi))) {
			fkey = fam_to_key(fam);
			append_indiseq(fseq, fkey, NULL, NULL, FALSE, FALSE);
		}
		if (!close) insert_table(tab, skey(el), NULL);
	ENDINDISEQ
	FORINDISEQ(fseq, el, num)
		fam = key_to_fam(skey(el));
		FORCHILDREN(fam, chil, num2)
			key = indi_to_key(chil);
			if (!in_table(tab, key)) {
				key = strsave(key);
				append_indiseq(sseq, key, NULL, NULL,
				    TRUE, TRUE);
				insert_table(tab, key, NULL);
			}
		ENDCHILDREN
	ENDINDISEQ
	remove_table(tab, DONTFREE);
	remove_indiseq(fseq, FALSE);
	return sseq;
}
/*=========================================================
 * ancestor_indiseq -- Create ancestor sequence of sequence
 *  values are created with the generation number
 *  (passed to create_value callback)
 *=======================================================*/
INDISEQ
ancestor_indiseq (INDISEQ seq, WORD (*create_value_fnc)(INT gen))
{
	TABLE tab;
	LIST anclist, genlist;
	INDISEQ anc;
	NODE indi, fath, moth;
	STRING key, pkey;
	INT gen;
	if (!seq) return NULL;
	tab = create_table();
	anclist = create_list();
	genlist = create_list();
	anc = create_indiseq();
	FORINDISEQ(seq, el, num)
		enqueue_list(anclist, (WORD)skey(el));
		enqueue_list(genlist, (WORD)0);
	ENDINDISEQ
	while (!empty_list(anclist)) {
		key = (STRING) dequeue_list(anclist);
		gen = (INT) dequeue_list(genlist) + 1;
		indi = key_to_indi(key);
		fath = indi_to_fath(indi);
		moth = indi_to_moth(indi);
		if (fath && !in_table(tab, pkey = indi_to_key(fath))) {
			pkey = strsave(pkey);
			append_indiseq(anc, pkey, NULL,
				(*create_value_fnc)(gen), TRUE, TRUE);
			enqueue_list(anclist, (WORD)pkey);
			enqueue_list(genlist, (WORD)gen);
			insert_table(tab, pkey, NULL);
		}
		if (moth && !in_table(tab, pkey = indi_to_key(moth))) {
			pkey = strsave(pkey);
			append_indiseq(anc, pkey, NULL,
				(*create_value_fnc)(gen), TRUE, TRUE);
			enqueue_list(anclist, (WORD)pkey);
			enqueue_list(genlist, (WORD)gen);
			insert_table(tab, pkey, NULL);
		}
	}
	remove_table(tab, DONTFREE);
	remove_list(anclist, NULL);
	remove_list(genlist, NULL);
	return anc;
}
/*=============================================================
 * descendant_indiseq -- Create descendant sequence of sequence
 *  values are created with the generation number
 *  (passed to create_value callback)
 *===========================================================*/
INDISEQ
descendent_indiseq (INDISEQ seq, WORD (*create_value_fnc)(INT gen))
{
	INT gen;
	TABLE itab, ftab;
	LIST deslist, genlist;
	INDISEQ des;
	NODE indi;
	STRING key, dkey, fkey;
	if (!seq) return NULL;
	itab = create_table();
	ftab = create_table();
	deslist = create_list();
	genlist = create_list();
	des = create_indiseq();
	FORINDISEQ(seq, el, num)
		enqueue_list(deslist, (WORD)skey(el));
		enqueue_list(genlist, (WORD)0);
	ENDINDISEQ
	while (!empty_list(deslist)) {
		INT num1, num2;
		key = (STRING) dequeue_list(deslist);
		gen = (INT) dequeue_list(genlist) + 1;
		indi = key_to_indi(key);
		FORFAMSS(indi, fam, spouse, num1)
			if (in_table(ftab, fkey = indi_to_key(fam)))
				goto a;
			insert_table(ftab, strsave(fkey), NULL);
			FORCHILDREN(fam, child, num2)
				if (!in_table(itab,
				    dkey = indi_to_key(child))) {
					dkey = strsave(dkey);
					append_indiseq(des, dkey, NULL, 
					    (*create_value_fnc)(gen),
					    TRUE, TRUE);
					enqueue_list(deslist, (WORD)dkey);
					enqueue_list(genlist, (WORD)gen);
					insert_table(itab, dkey, NULL);
				}
			ENDCHILDREN
		a:;
		ENDFAMSS
	}
	remove_table(itab, DONTFREE);
	remove_table(ftab, FREEKEY);
	return des;
}
/*========================================================
 * spouse_indiseq -- Create spouses sequence of a sequence
 *======================================================*/
INDISEQ
spouse_indiseq (INDISEQ seq)
{
	TABLE tab;
	INDISEQ sps;
	STRING spkey;
	NODE indi;
	INT num1;
	if (!seq) return NULL;
	tab = create_table();
	sps = create_indiseq();
	FORINDISEQ(seq, el, num)
		indi = key_to_indi(skey(el));
		FORSPOUSES(indi, spouse, fam, num1)
			spkey = indi_to_key(spouse);
			if (!in_table(tab, spkey)) {
				spkey = strsave(spkey);
				append_indiseq(sps, spkey, NULL,
				    (WORD)sval(el), TRUE, TRUE);
				insert_table(tab, spkey, NULL);
			}
		ENDSPOUSES
	ENDINDISEQ
	remove_table(tab, DONTFREE);
	return sps;
}
/*============================================================
 * name_to_indiseq -- Return person sequence whose names match
 *==========================================================*/
INDISEQ
name_to_indiseq (STRING name)
{
	STRING *keys, *names;
	INT i, num, c, lastchar;
	INDISEQ seq = NULL;
	BOOLEAN made = FALSE;
	char scratch[MAXLINELEN+1];
	if (!name || *name == 0) return NULL;
	if (*name != '*') {
		names = get_names(name, &num, &keys, TRUE);
		if (num == 0) return NULL;
		seq = create_indiseq();
		for (i = 0; i < num; i++)
			append_indiseq(seq, keys[i], NULL, NULL, FALSE,
			    FALSE);
		namesort_indiseq(seq);
		return seq;
	}
	sprintf(scratch, "a/%s/", getsurname(name));
	lastchar = 'z';
	if(opt_finnish) lastchar = 255;
	for (c = 'a'; c <= lastchar; c++) {
		if(opt_finnish && !my_islower(c)) continue;
		scratch[0] = c;
		names = get_names(scratch, &num, &keys, TRUE);
		if (num == 0) continue;
		if (!made) {
			seq = create_indiseq();
			made = TRUE;
		}
		for (i = 0; i < num; i++) {
			append_indiseq(seq, keys[i], NULL, NULL, TRUE,
			    FALSE);
		}
	}
	scratch[0] = '$';
	names = get_names(scratch, &num, &keys, TRUE);
	if (num) {
		if (!made) {
			seq = create_indiseq();
			made = TRUE;
		}
		for (i = 0; i < num; i++) {
			append_indiseq(seq, keys[i], NULL, NULL, TRUE,
			    FALSE);
		}
	}
	if (seq) {
		 /* The following call to unique_indiseq was added as
		    part of the Finnish language patches. Is it
		    applicable to all?
		  */
		 unique_indiseq(seq);
		 namesort_indiseq(seq);
	}
	return seq;
}
/*===========================================
 * generic_print_el -- Format a print line of
 *  sequence of indis
 *=========================================*/
static STRING
generic_print_el (INDISEQ seq, INT i)
{
	STRING key, name, str;
	element_indiseq(seq, i, &key, &name);

	str=NULL; /* set to appropriate format */

	switch (key[0])
	{
	case 'I':
		{
			NODE indi = key_to_indi(key);
			str = indi_to_list_string(indi, NULL, 68);
		}
		break;
	case 'S':
		{
			NODE sour = qkey_to_sour(key);
			if (sour)
			{
				str = sour_to_list_string(sour, 68, ", ");
			}
		}
		break;
	case 'E':
		/* TO DO - any expected structure for events ? */
		break;
	case 'X':
		{
			NODE node = key_to_type(key, TRUE);
			if (node)
			{
				str = generic_to_list_string(node, 68, ", ");
			}
		}
		break;
	}
	if (!str)
		str = strsave(key);
	return str;
}
/*=============================================
 * spouseseq_print_el -- Format a print line of
 *  sequence of spouses
 * assume values are family keys
 *===========================================*/
static STRING
spouseseq_print_el (INDISEQ seq, INT i)
{
	NODE indi, fam;
	STRING key, name, str;
	INT val;
	elementval_indiseq(seq, i, &key, &val, &name);
	indi = key_to_indi(key);
	fam = keynum_to_fam(val);
	str = indi_to_list_string(indi, fam, 68);
	return str;
}
/*==========================================
 * famseq_print_el -- Format a print line of
 *  sequence of families
 * assume values are spouse keys
 *========================================*/
static STRING
famseq_print_el (INDISEQ seq, INT i)
{
	NODE fam, spouse;
	STRING key, name, str;
	INT val;
	elementval_indiseq(seq, i, &key, &val, &name);
	fam = key_to_fam(key);
	spouse = ( val ? keynum_to_indi(val) : NULL);
	str = indi_to_list_string(spouse, fam, 68);
	return str;
}
/*================================================
 * get_print_el -- Get appropriate format line for
 *  one element of an indiseq
 *==============================================*/
static STRING
get_print_el (INDISEQ seq, INT i)
{
	STRING str;
	switch(IPrntype(seq)) {
	case ISPRN_FAMSEQ: str = famseq_print_el(seq, i); break;
	case ISPRN_SPOUSESEQ: str = spouseseq_print_el(seq, i); break;
	default: str = generic_print_el(seq, i); break;
	}
	return str;
}
/*================================================
 * print_indiseq_element -- Format a print line of
 *  an indiseq (any type)
 *==============================================*/
void
print_indiseq_element (INDISEQ seq, INT i, STRING buf, INT len)
{
	STRING str, ptr=buf;
	BOOLEAN alloc=FALSE;
	buf[0]='\0';
	str = sprn(IData(seq)[i]);
	if (!str) {
		/*
		 If not precomputed, make print string on-the-fly .
		 This is used for long seqs, when we don't want to keep
		 all these strings in memory all the time.
		 *
		 Note: The print_el functions return a strsave'd string.
		 It would be more efficient not to strsave. This requires
		 changing indi_to_list_string, etc.
		*/
		str = get_print_el(seq, i);
		alloc=TRUE;
	}
	llstrcatn(&ptr, str, &len);
	if (alloc)
		free(str);
}
/*=====================================================
 * preprint_indiseq -- Preformat print lines of indiseq
 *===================================================*/
void
preprint_indiseq (INDISEQ seq)
{
	FORINDISEQ(seq, el, num)
		sprn(el) = get_print_el(seq, num);
	ENDINDISEQ
}
/*==============================================================
 * refn_to_indiseq -- Return indiseq whose user references match
 *============================================================*/
INDISEQ
refn_to_indiseq (STRING ukey, INT letr, INT sort)
{
	STRING *keys;
	INT num, i;
	INDISEQ seq;

	if (!ukey || *ukey == 0) return NULL;
	get_refns(ukey, &num, &keys, letr);
	if (num == 0) return NULL;
	seq = create_indiseq();
	for (i = 0; i < num; i++) {
		append_indiseq(seq, keys[i], NULL, NULL, FALSE, FALSE);
	}
	if (sort == NAMESORT)
		namesort_indiseq(seq);
	else
		keysort_indiseq(seq);
	return seq;
}
/*=============================================================
 * key_to_indiseq -- Return person sequence of the matching key
 *=============================================================*/
INDISEQ
key_to_indiseq (STRING name)
{
	STRING *keys;
	INDISEQ seq = NULL;
	if (!name) return NULL;
	if (!(id_by_key(name, &keys))) return NULL;
	seq = create_indiseq();
	append_indiseq(seq, keys[0], NULL, NULL, FALSE, FALSE);
	return seq;
}
/*===========================================================
 * str_to_indiseq -- Return person sequence matching a string
 * The rules of search precedence are implemented here:
 *  1. named indiset
 *  2. key, with or without the leading "I"
 *  3. REFN
 *  4. name
 *===========================================================*/
INDISEQ
str_to_indiseq (STRING name)
{
	INDISEQ seq;
	seq = find_named_seq(name);
	if (!seq) seq = key_to_indiseq(name);
	if (!seq) seq = refn_to_indiseq(name, 'I', NAMESORT);
	if (!seq) seq = name_to_indiseq(name);
	return seq;
}
/*=======================================================
 * append_all_tags -- append all tags of specified type
 *  to indiseq (optionally recursive
 *=====================================================*/
static void
append_all_tags(INDISEQ seq, NODE node, STRING tagname, BOOLEAN recurse)
{
	if (eqstr(ntag(node), tagname))
	{
		STRING key;
		INT val;
		key = nval(node);
		if (key)
		{
			STRING skey = rmvat(key);
			if (skey)
				val = atoi(skey+1); /* PVALUE NEEDED */
			else
			{
				skey = key; /* leave invalid sources alone, but mark invalid with val==-1 */
				val = -1;
			}
			append_indiseq_ival(seq, skey, NULL, val, FALSE, FALSE);
		}
	}
	if (nchild(node) && recurse )
		append_all_tags(seq, nchild(node), tagname, recurse);
	if (nsibling(node))
		append_all_tags(seq, nsibling(node), tagname, recurse);

}
/*=======================================================
 * node_to_sources -- Create sequence of all sources
 *  inside a node record (at any level)
 *=====================================================*/
INDISEQ node_to_sources (NODE indi)
{
	INDISEQ seq;
	if (!indi) return NULL;
	seq = create_indiseq();
	append_all_tags(seq, indi, "SOUR", TRUE);
	if (!length_indiseq(seq))
	{
		remove_indiseq(seq, FALSE);
		seq = NULL;
	}
	return seq;
}
/*=======================================================
 * get_all_sour -- Create sequence of all sources
 *=====================================================*/
INDISEQ get_all_sour (void)
{
	INDISEQ seq=NULL;
	int i=0;
	while ((i=xref_nexts(i)))
	{
		static char skey[10];
		if (!seq)
			seq = create_indiseq();
		sprintf(skey, "S%d", i);
		append_indiseq_ival(seq, skey, NULL, i, TRUE, FALSE);
	}
	return seq;
}		
/*=======================================================
 * get_all_even -- Create sequence of all event records
 *=====================================================*/
INDISEQ get_all_even (void)
{
	INDISEQ seq=NULL;
	INT i=0;
	while ((i=xref_nexte(i)))
	{
		static char skey[10];
		if (!seq)
			seq = create_indiseq();
		sprintf(skey, "E%d", i);
		append_indiseq_ival(seq, skey, NULL, i, TRUE, FALSE);
	}
	return seq;
}		
/*=======================================================
 * get_all_othe -- Create sequence of all other records
 *=====================================================*/
INDISEQ get_all_othe (void)
{
	INDISEQ seq=NULL;
	INT i=0;
	while ((i=xref_nextx(i)))
	{
		static char skey[10];
		if (!seq)
			seq = create_indiseq();
		sprintf(skey, "X%d", i);
		append_indiseq_ival(seq, skey, NULL, i, TRUE, FALSE);
	}
	return seq;
}		
