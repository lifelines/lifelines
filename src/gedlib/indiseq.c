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

/* WARNING: append_indiseq allows an extra value to be stored.
 * this value must be a PVALUE if it is to be used in report programs.
 * However, internally when browsing, a family number is inserted as
 * an integer, not an integer (PINT) PVALUE. This may cause problems
 * if the value associated with an individual in a set is attempted
 * to be used in a report program - pbm 17-Feb-97
 */

extern INDISEQ find_named_seq();
extern STRING *id_by_key();
extern BOOLEAN opt_finnish;		/* Finnish language support */

#define key_to_name(k)  nval(NAME(key_to_indi(k)))

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
	INDISEQ new;
	if (!seq) return NULL;
	new = create_indiseq();
	FORINDISEQ(seq, el, num)
		append_indiseq(new, skey(el), snam(el), (WORD)sval(el),
		    TRUE, FALSE);
	ENDINDISEQ
	return new;
}
/*=============================================
 * append_indiseq -- Append element to sequence
 *===========================================*/
void
append_indiseq (INDISEQ seq,    /* sequence */
                STRING key,     /* key - not NULL */
                STRING name,    /* name - may be NULL */
                WORD val,       /* extra val - may be NULL (otherwise
                                   must be a PVALUE) */
                BOOLEAN sure,   /* no dupe check? */
                BOOLEAN alloc)  /* key alloced? */
{
	INT i, m, n;
	SORTEL el, *new, *old;
	if (!seq || !key) return;
	n = ISize(seq);
	old = IData(seq);
	if (!sure) {
		if (*key == 'I' && !name) {
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
key_compare (SORTEL el1,
             SORTEL el2)
{
	return spri(el1) - spri(el2);
}
/*===================================================
 * value_str_compare -- Compare two values as strings
 *=================================================*/
INT
value_str_compare (SORTEL el1,
                   SORTEL el2)
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
value_compare (SORTEL el1,
               SORTEL el2)
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
	IFlags(seq) &= ~KEYSORT;
	IFlags(seq) &= ~VALUESORT;
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
	IFlags(seq) &= ~NAMESORT;
	IFlags(seq) &= ~VALUESORT;
	IFlags(seq) |= KEYSORT;
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
	IFlags(seq) &= ~NAMESORT;
	IFlags(seq) &= ~KEYSORT;
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
			append_indiseq(three, key, NULL, (WORD)sval(u[i]),
			    TRUE, TRUE);
			i++;
		} else if (rel > 0) {
			key = strsave(skey(v[j]));
			append_indiseq(three, key, NULL, (WORD)sval(v[j]),
			    TRUE, TRUE);
			j++;
		} else {
			key = strsave(skey(u[i]));
			append_indiseq(three, key, NULL, (WORD)sval(u[i]),
			    TRUE, TRUE);
			i++, j++;
		}
	}
	while (i < n) {
		key = strsave(skey(u[i]));
		append_indiseq(three, key, NULL, (WORD)sval(u[i]), TRUE, TRUE);
		i++;
	}
	while (j < m) {
		key = strsave(skey(v[j]));
		append_indiseq(three, key, NULL, (WORD)sval(v[j]), TRUE, TRUE);
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
			append_indiseq(three, key, NULL, (WORD)sval(u[i]),
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
			append_indiseq(three, key, NULL, (WORD)sval(u[i]),
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
		append_indiseq(three, key, NULL, (WORD)sval(u[i]), TRUE, TRUE);
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
			append_indiseq(par, key, NULL, (WORD)sval(el), TRUE, TRUE);
			insert_table(tab, key, NULL);
		}
		if (moth && !in_table(tab, key = indi_to_key(moth))) {
			key = strsave(key);
			append_indiseq(par, key, NULL, (WORD)sval(el), TRUE, TRUE);
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
					    (WORD)sval(el), TRUE, TRUE);
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
			append_indiseq(seq, key, NULL, (WORD)NULL, TRUE, FALSE);
		ENDCHILDREN
	ENDFAMSS
	if (len) return seq;
	remove_indiseq(seq, FALSE);
	return NULL;
}
/*=======================================================
 * indi_to_spouses -- Create sequence of person's spouses
 *=====================================================*/
INDISEQ
indi_to_spouses (NODE indi)
{
	INDISEQ seq;
	INT num, num1, val, len = 0;
	STRING key;
	if (!indi) return NULL;
	seq = create_indiseq();
	FORFAMSS(indi, fam, spouse, num)
#if 0
		if (spouse) {
			len++;
			key = indi_to_key(spouse);
			val = atoi(fam_to_key(fam) + 1); /* PVALUE NEEDED */
			append_indiseq(seq, key, NULL, (WORD)val, TRUE, FALSE);
		}
#else
		FORHUSBS(fam, husb, num1)
			if(husb != indi) {
				len++;
				key = indi_to_key(husb);
				val = atoi(fam_to_key(fam) + 1); /* PVALUE NEEDED */
				append_indiseq(seq, key, NULL, (WORD)val, TRUE, FALSE);
			}
		ENDHUSBS
		FORWIFES(fam, wife, num1)
			if(wife != indi) {
				len++;
				key = indi_to_key(wife);
				val = atoi(fam_to_key(fam) + 1);
				append_indiseq(seq, key, NULL, (WORD)val, TRUE, FALSE);
			}
		ENDWIFES
#endif
	ENDFAMSS
	if (len) return seq;
	remove_indiseq(seq, FALSE);
	return NULL;
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
			append_indiseq(seq, key, NULL, (WORD)NULL, TRUE, FALSE);
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
			append_indiseq(seq, key, NULL, (WORD)NULL, TRUE, FALSE);
		ENDWIFES
	ENDFAMCS
	if (len) return seq;
	remove_indiseq(seq, FALSE);
	return NULL;
}
/*=========================================================
 * indi_to_families -- Create sequence of person's families
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
	if (fams) {
		FORFAMSS(indi, fam, spouse, num)
			key = fam_to_key(fam);
			val = spouse ? atoi(indi_to_key(spouse) + 1) : 0;
			/* PVALUE NEEDED */
			append_indiseq(seq, key, NULL, (WORD)val, TRUE, FALSE);
		ENDFAMSS
	} else {
		FORFAMCS(indi, fam, fath, moth, num)
			key = fam_to_key(fam);
			val = fath ? atoi(indi_to_key(fath) + 1) : 0;
			/* PVALUE NEEDED */
			append_indiseq(seq, key, NULL, (WORD)val, TRUE, FALSE);
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
		append_indiseq(seq, key, NULL, (WORD)NULL, TRUE, FALSE);
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
		append_indiseq(seq, key, NULL, (WORD)NULL, TRUE, FALSE);
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
 *=======================================================*/
INDISEQ
ancestor_indiseq (INDISEQ seq)
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
				create_pvalue(PINT, (WORD)gen), TRUE, TRUE);
			enqueue_list(anclist, (WORD)pkey);
			enqueue_list(genlist, (WORD)gen);
			insert_table(tab, pkey, NULL);
		}
		if (moth && !in_table(tab, pkey = indi_to_key(moth))) {
			pkey = strsave(pkey);
			append_indiseq(anc, pkey, NULL,
				create_pvalue(PINT, (WORD)gen), TRUE, TRUE);
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
 *===========================================================*/
INDISEQ
descendent_indiseq (INDISEQ seq)
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
					    create_pvalue(PINT, (WORD)gen),
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
/*===================================================================
 * gen_gedcom -- Generate GEDCOM file from sequence; only persons in
 *   sequence are in file; families that at least two persons in
 *   sequence refer to are also in file; other persons referred to by
 *   families are not included
 *=================================================================*/
void
gen_gedcom (INDISEQ seq)
{
	INT num1, num2, sex;
	NODE indi, husb, wife, chil, rest, famc, fref;
	INDISEQ fseq;
	TABLE itab, ftab;
	BOOLEAN addfam;
	STRING tag, dkey;
	char scratch[30];
	if (!seq) return;
	fseq = create_indiseq();
	itab = create_table();
	ftab = create_table();
	FORINDISEQ(seq, el, num)
		insert_table(itab, skey(el), NULL);
	ENDINDISEQ
	FORINDISEQ(seq, el, num)
		indi = key_to_indi(skey(el));
		sex = SEX(indi);
		write_nonlink_indi(indi);
		famc = indi_to_famc(indi);
		if (!famc) goto c;
		addfam = FALSE;
		split_fam(famc, &fref, &husb, &wife, &chil, &rest);
		join_fam(famc, fref, husb, wife, chil, rest);
		if (husb && in_table(itab, rmvat(nval(husb)))) addfam = TRUE;
		if (!addfam && wife && in_table(itab, rmvat(nval(wife))))
			addfam = TRUE;
		if (!addfam) {
			FORCHILDREN(famc, chl, num2)
				dkey = indi_to_key(chl);
				if (in_table(itab, dkey) &&
				    nestr(skey(el), dkey)) {
					addfam = TRUE;
					goto a;
				}
			ENDCHILDREN
	a:;
		}
		if (addfam) {
			tag = rmvat(nxref(famc));
			sprintf(scratch, "1 FAMC @%s@\n", tag);
			poutput(scratch);
			if (!in_table(ftab, tag)) {
				tag = strsave(tag);
				append_indiseq(fseq, tag, NULL, NULL,
				    TRUE, TRUE);
				insert_table(ftab, tag, NULL);
			}
		}
	c:
		FORFAMSS(indi, fam, spouse, num1)
			addfam = FALSE;
			if (spouse && in_table(itab, indi_to_key(spouse)))
				addfam = TRUE;
			if (!addfam) {
				FORCHILDREN(fam, chl, num2)
					if (in_table(itab, indi_to_key(chl))) {
						addfam = TRUE;
						goto b;
					}
				ENDCHILDREN
	b:;
			}
			if (addfam) {
				tag = rmvat(nxref(fam));
				sprintf(scratch, "1 FAMS @%s@\n", tag);
				poutput(scratch);
				if (!in_table(ftab, tag)) {
					tag = strsave(tag);
					append_indiseq(fseq, tag, NULL, NULL,
					    TRUE, TRUE);
					insert_table(ftab, tag, NULL);
				}
			}
		ENDFAMSS
	ENDINDISEQ
	FORINDISEQ(fseq, el, num)
		write_family(skey(el), itab);
	ENDINDISEQ
	remove_indiseq(fseq, FALSE);
	remove_table(itab, DONTFREE);
	remove_table(ftab, DONTFREE);
}
/*======================================================
 * write_nonlink_indi -- Write person minus linking info
 *====================================================*/
void
write_nonlink_indi (NODE indi)
{
	STRING t;
	char scratch[30];
	sprintf(scratch, "0 %s INDI\n", nxref(indi));
	poutput(scratch);
	indi = nchild(indi);
	while (indi) {
		t = ntag(indi);
		if (eqstr("FAMS", t) || eqstr("FAMC", t)) break;
		new_write_node(1, indi, FALSE);
		indi = nsibling(indi);
	}
}
/*==================================================
 * new_write_node -- Recursively write nodes to file
 * NOTE: consolidate with write_node?
 *================================================*/
void
new_write_node (INT levl,       /* level of root */
                NODE node,      /* root */
                BOOLEAN list)   /* output siblings? */
{
	char unsigned scratch[MAXLINELEN+1];
	STRING p = scratch;
	if (!node) return;
	sprintf(p, "%d", levl);
	p += strlen(p);
	if (nxref(node)) {
		sprintf(p, " %s", nxref(node));
		p += strlen(p);
	}
	sprintf(p, " %s", ntag(node));
	p += strlen(p);
	if (nval(node)) {
		sprintf(p, " %s", nval(node));
		p += strlen(p);
	}
	sprintf(p, "\n");
	poutput(scratch);
	new_write_node(levl + 1, nchild(node), TRUE);
	if (list)
		new_write_node(levl, nsibling(node), TRUE);
}
/*============================================
 * write_family -- Write family record to file
 *==========================================*/
void
write_family (STRING key,     /* family key */
              TABLE itab)     /* table of persons in file */
{
	NODE fam = key_to_fam(key);
	char scratch[30];
	STRING t;
	sprintf(scratch, "0 %s FAM\n", nxref(fam));
	poutput(scratch);
	fam = nchild(fam);
	while (fam) {
		t = ntag(fam);
		if (eqstr("HUSB", t)) {
			if (in_table(itab, rmvat(nval(fam))))
				new_write_node(1, fam, FALSE);
		} else if (eqstr("WIFE", t)) {
			if (in_table(itab, rmvat(nval(fam))))
				new_write_node(1, fam, FALSE);
		} else if (eqstr("CHIL", t)) {
			if (in_table(itab, rmvat(nval(fam))))
				new_write_node(1, fam, FALSE);
		} else
			new_write_node(1, fam, FALSE);
		fam = nsibling(fam);
	}
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
/*==================================================
 * format_indiseq -- Format print lines of sequence.
 *================================================*/
void
format_indiseq (INDISEQ seq,    /* sequence */
                BOOLEAN famp,   /* seq of fams? */
                BOOLEAN marr)   /* try to give marriage? */
{
	NODE indi, fam, spouse;
	char scratch[20];
	if (famp) {
		FORINDISEQ(seq, el, num)
			fam = key_to_fam(skey(el));
			if (sval(el)) {
				sprintf(scratch, "I%d", (INT)sval(el));
				spouse = key_to_indi(scratch);
			} else
				spouse = NULL;
			sprn(el) = indi_to_list_string(spouse, fam, 68);
		ENDINDISEQ
	} else {
		FORINDISEQ(seq, el, num)
			indi = key_to_indi(skey(el));
			if (marr) {
				sprintf(scratch, "F%d", (INT)sval(el));
				fam = key_to_fam(scratch);
			} else
				fam = NULL;
			sprn(el) = indi_to_list_string(indi, fam, 68);
		ENDINDISEQ
	}
}
/*==============================================================
 * refn_to_indiseq -- Return indiseq whose user references match
 *============================================================*/
INDISEQ
refn_to_indiseq (STRING ukey)
{
	STRING *keys;
	INT num, i;
	INDISEQ seq;

	if (!ukey || *ukey == 0) return NULL;
	get_refns(ukey, &num, &keys, 'I');
	if (num == 0) return NULL;
	seq = create_indiseq();
	for (i = 0; i < num; i++) {
		append_indiseq(seq, keys[i], NULL, NULL, FALSE, FALSE);
	}
	if (length_indiseq(seq) == 0) {
		remove_indiseq(seq, FALSE);
		return NULL;
	}
	namesort_indiseq(seq);
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
	if (!seq) seq = refn_to_indiseq(name);
	if (!seq) seq = name_to_indiseq(name);
	return seq;
}
