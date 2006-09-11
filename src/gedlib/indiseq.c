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

#include "llstdlib.h" /* llstdlib.h includes standard.h, config.h, sys_inc.h */
#include "zstr.h"
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "indiseq.h"
#include "gedcomi.h"
#include "mystring.h" /* lat1_xx */

/*
	indiseqs are typed as to value
	as of 2001/01/07 (Perry)
	ival = ints
	pval = pointers (caller determined
	sval = strings
	null = not yet specified

	null indiseqs change type as soon as a fixed type value
	is assigned to them, but as long as only nulls are attached
	(append_indiseq_null) they remain uncommitted

	NB: null values can be appended to any type (append_indiseq_null)

	pointers are managed by the caller - in practice these are
	all PVALUES for report commands
*/


/*********************************************
 * local enums
 *********************************************/

/*====================
 * indiseq print types
 *==================*/
#define ISPRN_NORMALSEQ 0
#define ISPRN_FAMSEQ 1
#define ISPRN_SPOUSESEQ 2

/*********************************************
 * local types
 *********************************************/

/*==================================================================
 * SORTEL -- Data type for indiseq elements; keys are always present
 *   and belong to the structure; names are always present for
 *   persons and belong; values do not belong to the structure
 *================================================================*/
struct tag_sortel {
	STRING s_key;	/* person or family key */
	STRING s_nam;	/* name of person */
	UNION s_val;	/* any value */
	STRING s_prn;	/* menu print string */
	INT s_pri;	/* key as integer (exc valuesort_indiseq puts values here) */
};
/* typedef struct tag_sortel *SORTEL; */ /* in indiseq.h */
#define skey(s) ((s)->s_key)
#define snam(s) ((s)->s_nam)
#define sval(s) ((s)->s_val)
#define sprn(s) ((s)->s_prn)
#define spri(s) ((s)->s_pri)

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void append_all_tags(INDISEQ, NODE, STRING tagname, BOOLEAN recurse, BOOLEAN nonptrs);
static void append_indiseq_impl(INDISEQ seq, STRING key, 
	CNSTRING name, UNION val, BOOLEAN sure, BOOLEAN alloc);
static void calc_indiseq_name_el(INDISEQ seq, INT index);
static INT canonkey_compare(SORTEL el1, SORTEL el2, VPTR param);
static INT canonkey_order(char c);
static void check_indiseq_valtype(INDISEQ seq, INT valtype);
static UNION copyval(INDISEQ seq, UNION uval);
static INDISEQ create_indiseq_impl(INT valtype, INDISEQ_VALUE_FNCTABLE fnctable);
static void delete_el(INDISEQ seq, SORTEL el);
static void deleteval(INDISEQ seq, UNION uval);
static INDISEQ dupseq(INDISEQ seq);
static STRING get_print_el(INDISEQ, INT i, INT len, RFMT rfmt);
static BOOLEAN is_locale_current(INDISEQ seq);
static INT key_compare(SORTEL el1, SORTEL el2, VPTR param);
static INT name_compare(SORTEL el1, SORTEL el2, VPTR param);
static void llqsort2(SORTEL *data, ELCMPFNC cmp, VPTR param, INT a, INT b);
static void partition2(SORTEL *arr, ELCMPFNC cmp, VPTR param, INT a, INT b, INT *pi, INT *pj);
static STRING qkey_to_name(STRING key);
static void update_locale(INDISEQ seq);
static INT value_compare(SORTEL el1, SORTEL el2, VPTR param);

/*********************************************
 * local variables
 *********************************************/

static struct tag_indiseq_value_fnctable def_valfnctbl =
{
	&default_copy_value
	, &default_delete_value
	, &default_create_gen_value
	, &default_compare_values
};

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*===============================================
 * create_indiseq_ival -- Create sequence of INTs
 * Created: 2001/01/07, Perry Rapp
 *=============================================*/
INDISEQ
create_indiseq_ival (void)
{
	return create_indiseq_impl(ISVAL_INT, NULL);
}
/*===================================================
 * create_indiseq_null -- Create sequence of not yet
 *  determined type of objects
 * Created: 2001/01/07, Perry Rapp
 *=================================================*/
INDISEQ
create_indiseq_null (void)
{
	return create_indiseq_impl(ISVAL_NUL, NULL);
}
/*===================================================
 * create_indiseq_pval -- Create sequence of pointers
 * Created: 2001/01/07, Perry Rapp
 *=================================================*/
INDISEQ
create_indiseq_pval (void)
{
	return create_indiseq_impl(ISVAL_PTR, NULL);
}
/*==================================================
 * create_indiseq_sval -- Create sequence of STRINGs
 * Created: 2001/01/07, Perry Rapp
 *================================================*/
INDISEQ
create_indiseq_sval (void)
{
	return create_indiseq_impl(ISVAL_STR, NULL);
}
/*=======================================
 * create_indiseq_impl -- Create sequence
 * fnctable specifies the value function table for the 
 * new seq, and is optional - if NULL, the default
 * one will be used
 *=====================================*/
static INDISEQ
create_indiseq_impl (INT valtype, INDISEQ_VALUE_FNCTABLE fnctable)
{
	INDISEQ seq = (INDISEQ) stdalloc(sizeof *seq);
	memset(seq, 0, sizeof(*seq));
	IMax(seq) = 20;
	IData(seq) = (SORTEL *) stdalloc(20*sizeof(SORTEL));
	IPrntype(seq) = ISPRN_NORMALSEQ;
	IValtype(seq) = valtype;
	IValfnctbl(seq) = fnctable ? fnctable : &def_valfnctbl;
	return seq;
}
/*==================================
 * remove_indiseq -- Remove sequence
 *================================*/
void
remove_indiseq (INDISEQ seq)
{
	SORTEL *d = IData(seq);
	INT i, n = ISize(seq);
	/* remove each element's heap memory */
	for (i = 0; i < n; i++, d++) {
		stdfree(skey(*d));
		if (snam(*d)) stdfree(snam(*d));
		deleteval(seq, sval(*d));
		if (sprn(*d)) stdfree(sprn(*d));
		stdfree(*d);
	}
	stdfree(IData(seq));
	if (ILocale(seq))
		stdfree(ILocale(seq));
	stdfree(seq);
}
/*==============================
 * copyval -- Copy a value using the value function table
 * Created: 2001/03/25, Perry Rapp
 * This takes care of the problem that report indiseqs
 * must use report-allocated values (pvalues) - and these
 * cannot be copied directly, a copy must be allocated.
 * This is a complication that is solved thru a function table,
 * because indiseq.c is in a layer lower than the interpreter,
 * and does not know about pvalues. The interpreter registers
 * a copy function in the seq's function table to solve this.
 *============================*/
static UNION
copyval (INDISEQ seq, UNION uval)
{
	return (*IValfnctbl(seq)->copy_fnc)(uval, IValtype(seq));
}
/*==============================
 * deleteval -- Delete a value using the value function table
 * Created: 2001/03/25, Perry Rapp
 *============================*/
static void
deleteval (INDISEQ seq, UNION uval)
{
	(*IValfnctbl(seq)->delete_fnc)(uval, IValtype(seq));
}
/*==============================
 * creategenval -- Create a value for a new element
 *  of a particular generation
 * (This is for ancestorset & descendantset)
 * Handle case that seq is null value type, and callee
 *  assigns a value type.
 * Created: 2001/03/25, Perry Rapp
 *============================*/
static UNION
creategenval (INDISEQ seq, INT gen)
{
	UNION u;
	INT valtype = IValtype(seq);
	u = (*IValfnctbl(seq)->create_gen_fnc)(gen, &valtype);
	if (valtype != IValtype(seq)) {
		ASSERT(IValtype(seq) == ISVAL_NUL);
		IValtype(seq) = valtype;
	}
	return u;
}
/*==============================
 * copy_indiseq -- Copy sequence
 *============================*/
INDISEQ
copy_indiseq (INDISEQ seq)
{
	INDISEQ newseq;
	UNION uval;
	if (!seq) return NULL;
	newseq = create_indiseq_impl(IValtype(seq), IValfnctbl(seq));
	FORINDISEQ(seq, el, num)
		uval = copyval(seq, sval(el));
		append_indiseq_impl(newseq, skey(el), snam(el), uval,
		    TRUE, FALSE);
	ENDINDISEQ
	/* set flags after, so append doesn't do unique checks or generate names */
	IFlags(newseq) = IFlags(seq);
	IFlags(newseq) &= (~WITHNAMES); /* didn't generate names */
	return newseq;
}
/*==================================================
 * check_indiseq_valtype -- Check that value type
 *  is as expected or null (in which case, convert)
 * Created: 2001/01/07, Perry Rapp
 *================================================*/
static void
check_indiseq_valtype (INDISEQ seq, INT valtype)
{
	if (IValtype(seq) == ISVAL_NUL) {
		IValtype(seq) = valtype;
	} else {
		ASSERT(IValtype(seq) == valtype);
	}
}
/*==================================================
 * append_indiseq_ival -- Append element to sequence
 *  with INT value
 * INDISEQ seq:    sequence
 * STRING key:     key - not NULL
 * STRING name:    name - may be NULL
 * INT val:        extra val
 * BOOLEAN sure:   no dupe check?
 * BOOLEAN alloc:  key alloced?
 * Created: 2001/01/05, Perry Rapp
 *================================================*/
void
append_indiseq_ival (INDISEQ seq, STRING key, STRING name, INT val
	, BOOLEAN sure, BOOLEAN alloc)
{
	UNION u;
	u.i = val;
	check_indiseq_valtype(seq, ISVAL_INT);
	append_indiseq_impl(seq, key, name, u, sure, alloc);
}
/*==================================================
 * append_indiseq_pval -- Append element to sequence
 *  with pointer value
 * Created: 2001/01/07, Perry Rapp
 * INDISEQ seq:    sequence
 * STRING key:     key - not NULL
 * STRING name:    name - may be NULL
 * VPTR pval:      extra val
 * BOOLEAN sure:   no dupe check?
 * BOOLEAN alloc:  key alloced?
 *================================================*/
void
append_indiseq_pval (INDISEQ seq,    /* sequence */
                     STRING key,     /* key - not NULL */
                     STRING name,    /* name - may be NULL */
                     VPTR pval,       /* extra val */
                     BOOLEAN sure)   /* no dupe check? */

{
	BOOLEAN alloc=FALSE; /* let append_indiseq_impl copy key */
	UNION u;
	u.w = pval;
	check_indiseq_valtype(seq, ISVAL_PTR);
	append_indiseq_impl(seq, key, name, u, sure, alloc);
}
/*==================================================
 * append_indiseq_sval -- Append element to sequence
 *  with STRING value
 * (Should be alloc'd values, unless caller is using
 *  a custom value function table)
 * Created: 2001/01/05, Perry Rapp
 *================================================*/
void
append_indiseq_sval (INDISEQ seq,    /* sequence */
                     STRING key,     /* key - not NULL */
                     CNSTRING name,    /* name - may be NULL */
                     STRING sval,       /* extra val */
                     BOOLEAN sure,   /* no dupe check? */
                     BOOLEAN alloc)  /* key alloced? */
{
	UNION u;
	u.w = sval;
	check_indiseq_valtype(seq, ISVAL_STR);
	append_indiseq_impl(seq, key, name, u, sure, alloc);
}
/*==================================================
 * append_indiseq_null -- Append element to sequence
 *  without value
 * Created: 2001/01/07, Perry Rapp
 *================================================*/
void
append_indiseq_null (INDISEQ seq,    /* sequence */
                     STRING key,     /* key - not NULL */
                     CNSTRING name,    /* name - may be NULL */
                     BOOLEAN sure,   /* no dupe check? */
                     BOOLEAN alloc)  /* key alloced? */
{
	UNION u;
	u.i=0;
	/* no type check - valid for any seq */
	append_indiseq_impl(seq, key, name, u, sure, alloc);
}
/*==================================================
 * append_indiseq_impl -- Append element to sequence
 *  all type appends use this implementation
 * INDISEQ seq:      sequence
 * STRING key:       key - not NULL
 * CNSTRING name:    name - may be NULL
 * UNION val:        extra val is pointer
 * BOOLEAN sure:     no dupe check?
 * BOOLEAN alloc:    key alloced?
 *================================================*/
static void
append_indiseq_impl (INDISEQ seq, STRING key, CNSTRING name, UNION val
	, BOOLEAN sure, BOOLEAN alloc)
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
				if (eqstr(key, skey(old[i]))) {
						/* failed dupe check - bail */
					if (alloc)
						stdfree(key);
					deleteval(seq, val);
					return;
				}
			}
		}
	}
	el = (SORTEL) stdalloc(sizeof(*el));
	skey(el) = alloc ? key : strsave(key);
	snam(el) = NULL;
	if (*key == 'I' && (IFlags(seq) & WITHNAMES)) {
		if (!name)
			name = qkey_to_name(key);
		if (name)
			snam(el) = strsave(name);
		else
			snam(el) = NULL;
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
 *  (actually will update all instances of specified key)
 *  seq:  [IN]  sequence to modify
 *  key:  [IN]  particular element(s) to update
 *=======================================================*/
void
rename_indiseq (INDISEQ seq, STRING key)
{
	INT i, n;
	SORTEL *data;
	if (!seq || !key || *key != 'I') return;
	n = ISize(seq);
	data = IData(seq);
	for (i = 0; i < n; i++) {
		if (eqstr(key, skey(data[i]))) {
			STRING name = qkey_to_name(key);
			if (snam(data[i])) stdfree(snam(data[i]));
			if (name)
				snam(data[i]) = strsave(name);
			else
				snam(data[i]) = NULL;
		}
	}
}
/*==============================================
 * in_indiseq -- See if element is in an INDISEQ
 *=============================================*/
BOOLEAN
in_indiseq (INDISEQ seq, STRING key)
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
 * delete_indiseq -- Remove el from sequence
 *  if key & name given, look for element matching both
 *  if key given, look for element matching key
 *   if neither, use index passed
 * seq:   [I/O] sequence
 * key:   [IN]  key - may be NULL
 * name:  [IN]  name - may be NULL
 * index: [IN]  index of el to remove - may be computed
 *==============================================================*/
BOOLEAN
delete_indiseq (INDISEQ seq, STRING key, STRING name, INT index)
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
	delete_el(seq, el);
	stdfree(el);
	return TRUE;
}
/*===============================================================
 * delete_el -- Free contents of element of INDISEQ
 *==============================================================*/
static void
delete_el (INDISEQ seq, SORTEL el)
{
	stdfree(skey(el));
	if (snam(el)) {
		stdfree(snam(el));
		snam(el)=NULL;
	}
	if (sprn(el)) {
		stdfree(sprn(el));
		sprn(el)=NULL;
	}
	deleteval(seq, sval(el));
	if (IValtype(seq) == ISVAL_INT)
		sval(el).i = 0;
	else
		sval(el).w = 0;
}
/*================================================
 * element_indiseq -- Return element from sequence
 *  seq:   [IN]  sequence
 *  index: [IN]  element desired
 *  pkey:  [OUT] returned key
 *  pname: [OUT] returned name
 *==============================================*/
BOOLEAN
element_indiseq (INDISEQ seq, INT index, STRING *pkey, STRING *pname)
{
	*pkey = *pname = NULL;
	if (!seq || index < 0 || index > ISize(seq) - 1) return FALSE;
	calc_indiseq_name_el(seq, index);
	*pkey =  skey(IData(seq)[index]);
	*pname = snam(IData(seq)[index]);
	return TRUE;
}
/*================================================
 * elementval_indiseq -- Return element & value from sequence
 * Created: 2000/11/29, Perry Rapp
 *  seq;   [IN]  sequence
 *  index: [IN]  index of desired element
 *  pkey:  [OUT] returned key
 *  pval:  [OUT] returned val
 *  pname: [OUT] returned name
 *==============================================*/
BOOLEAN
element_indiseq_ival (INDISEQ seq, INT index, STRING *pkey, INT *pval
	, STRING *pname)
{
	*pkey = *pname = NULL;
	if (!seq || index < 0 || index > ISize(seq) - 1) return FALSE;
	*pkey =  skey(IData(seq)[index]);
	/* do we need to allow for NUL type here ? */
	ASSERT(IValtype(seq) == ISVAL_INT || IValtype(seq) == ISVAL_NUL);
	*pval = sval(IData(seq)[index]).i;
	*pname = snam(IData(seq)[index]);
	return TRUE;
}
/*================================================
 * element_key_indiseq -- Return element key from sequence
 *  or NULL if bad inputs
 *==============================================*/
CNSTRING
element_key_indiseq (INDISEQ seq, INT index)
{
	if (!seq) return NULL;
	if (index<0 || index>=ISize(seq)) return NULL;
	return skey(IData(seq)[index]);
}
/*================================================
 * element_skey -- Return element key from indiseq element
 *  Requires valid input
 *==============================================*/
CNSTRING
element_skey (SORTEL el)
{
	ASSERT(el);
	return el->s_key;
}
/*================================================
 * element_name -- Return element name from indiseq element
 *  Requires valid input
 *==============================================*/
CNSTRING
element_name (SORTEL el)
{
	ASSERT(el);
	return el->s_nam;
}
/*================================================
 * element_sval -- Return string value from indiseq element
 *  Requires valid input
 *==============================================*/
CNSTRING
element_sval (SORTEL el)
{
	ASSERT(el);
	return el->s_val.w;
}
/*================================================
 * element_ikey -- Return integer key value from indiseq element
 *  Requires valid input
 *==============================================*/
INT
element_ikey (SORTEL el)
{
	ASSERT(el);
	return el->s_pri;
}
/*================================================
 * element_pval -- Return pointer value from indiseq element
 *  Requires valid input
 *==============================================*/
VPTR
element_pval (SORTEL el)
{
	ASSERT(el);
	return el->s_val.w;
}
/*================================================
 * set_element_pval -- Change pointer value of element
 *  Requires valid input
 *==============================================*/
void
set_element_pval (SORTEL el, VPTR ptr)
{
	ASSERT(el);
	el->s_val.w = ptr;
}
/*==================================
 * name_compare -- Compare two names
 *================================*/
INT
name_compare (SORTEL el1, SORTEL el2, VPTR param)
{
	if (!snam(el2)) {
		if (snam(el1))
			return -1;
	} else if (!snam(el1)) {
		if (snam(el2))
			return 1;
	} else {
		INT rel = namecmp(snam(el1), snam(el2));
		if (rel) return rel;
	}
	return canonkey_compare(el1, el2, param);
}
/*================================
 * key_compare -- Compare two keys
 * also used for integer value sort
 *==============================*/
INT
key_compare (SORTEL el1, SORTEL el2, VPTR param)
{
	param = param; /* unused */
	return spri(el1) - spri(el2);
}
/*===========================================
 * canonkey_order -- Canonical order of a type
 *  letter (I,F,S,E,X)
 * Created: 2001/01/06, Perry Rapp
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
 * Created: 2001/01/06, Perry Rapp
 *==============================*/
static INT
canonkey_compare (SORTEL el1, SORTEL el2, VPTR param)
{
	char c1=skey(el1)[0], c2=skey(el2)[0];
	param = param; /* unused */
	if (c1 == c2)
		return spri(el1) - spri(el2);
	return canonkey_order(c1) - canonkey_order(c2);
}
/*===================================================
 * value_compare -- Compare two values as strings
 *=================================================*/
static INT
value_compare (SORTEL el1, SORTEL el2, VPTR param)
{
	INDISEQ seq = (INDISEQ)param;
	INT valtype = IValtype(seq);
	INT rel = 0;
	if (valtype == ISVAL_INT) {
		INT i1=sval(el1).i, i2=sval(el2).i;
		rel = i1 - i2;
	} else if (valtype == ISVAL_STR) {
		STRING str1=sval(el1).w, str2=sval(el2).w;
		if (!str2) {
			if (str1)
				rel = -1;
		} else if (!str1) {
			if (str2)
				rel = 1;
		} else {
			rel = strcoll(str1, str2);
		}
	} else if (valtype == ISVAL_PTR) {
		VPTR ptr1=sval(el1).w, ptr2=sval(el2).w;
		rel = (*IValfnctbl(seq)->compare_val_fnc)(ptr1, ptr2, valtype);
	} else {
		/* nothing -- fall through to default to canonkey_compare */
	}
	if (!rel)
		rel = canonkey_compare(el1, el2, param);
	return rel;
}
/*==========================================
 * namesort_indiseq -- Sort sequence by name
 *========================================*/
void
namesort_indiseq (INDISEQ seq)
{
	calc_indiseq_names(seq);
	if ((IFlags(seq) & NAMESORT) && is_locale_current(seq)) return;
	FORINDISEQ(seq, el, num)
		spri(el) = atoi(skey(el) + 1);
	ENDINDISEQ
	partition_sort(IData(seq), ISize(seq), name_compare, seq);
	IFlags(seq) &= ~ALLSORTS;
	IFlags(seq) |= NAMESORT;
	update_locale(seq);
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
	partition_sort(IData(seq), ISize(seq), key_compare, seq);
	IFlags(seq) &= ~ALLSORTS;
	IFlags(seq) |= KEYSORT;
}
/*=============================================
 * canonkeysort_indiseq -- Sort sequence by key
 *  in key canonical order (I,F,S,E,X)
 * Created: 2001/01/06, Perry Rapp
 *===========================================*/
void
canonkeysort_indiseq (INDISEQ seq)
{
	if (IFlags(seq) & CANONKEYSORT) return;
	FORINDISEQ(seq, el, num)
		spri(el) = atoi(skey(el) + 1);
	ENDINDISEQ
	partition_sort(IData(seq), ISize(seq), canonkey_compare, seq);
	IFlags(seq) &= ~ALLSORTS;
	IFlags(seq) |= CANONKEYSORT;
}
/*============================================
 * is_locale_current -- 
 *  returns FALSE if locale has changed since
 *  this was sorted
 *==========================================*/
static BOOLEAN
is_locale_current (INDISEQ seq)
{
	if (are_locales_supported())
		return eqstr(ILocale(seq), llsetlocale(LC_COLLATE, NULL));
	else
		return TRUE;
}
/*============================================
 * update_locale -- 
 *  Annotate seq with current locale
 *==========================================*/
static void
update_locale (INDISEQ seq)
{
	const char *locstr = llsetlocale(LC_COLLATE, NULL);
	if (!ILocale(seq) || !eqstr(ILocale(seq), locstr)) {
		if (ILocale(seq))
			stdfree(ILocale(seq));
		ILocale(seq) = strsave(locstr);
	}
}
/*============================================
 * valuesort_indiseq -- Sort sequence by value
 *==========================================*/
void
valuesort_indiseq (INDISEQ seq, BOOLEAN *eflg)
{
	eflg = eflg; /* unused */
	if ((IFlags(seq) & VALUESORT) && is_locale_current(seq)) return;
	partition_sort(IData(seq), ISize(seq), value_compare, seq);
	IFlags(seq) &= ~ALLSORTS;
	IFlags(seq) |= VALUESORT;
	update_locale(seq);
}
/*=========================================
 * partition_sort -- Partition (quick) sort
 *=======================================*/
/*
 *  data:  [I/O] array of els to sort
 *  len:   [IN]  size of data
 *  cmp:   [IN]  callback to compare two elements
 *  param: [IN]  opaque parameter for callback
 */
void
partition_sort (SORTEL *data, INT len, ELCMPFNC cmp, VPTR param)
{
	llqsort2(data, cmp, param, 0, len-1);
}
/*======================================
 * llqsort2 -- Nonrecursive quicksort
 *  median of three pivot
 *====================================*/
/*======================================
 * partition2 -- Median of three pivot
 *  *pi is set to position of pivot
 *  *pj is set to index of highest item before *pi not equal to pivot
 *  (that is, anything between *pj and *pi has value of pivot)
 *====================================*/
#define CMP(qa, qb) ((*cmp)(qa, qb, param))
static void
partition2 (SORTEL *arr, ELCMPFNC cmp, VPTR param, INT a, INT b, INT *pi, INT *pj)
{
	INT i, j, c;
	SORTEL pivot=0, t;

	/* sort left, middle, and right items */
	c = ((unsigned)a + (unsigned)b)/2;

	if (CMP(arr[a],arr[c]) > 0) {
		t = arr[a]; arr[a] = arr[c]; arr[c] = t;
	}
	if (CMP(arr[a],arr[b]) > 0) {
		t = arr[a]; arr[a] = arr[b]; arr[b] = t;
	}
	if (CMP(arr[c],arr[b]) > 0) {
		t = arr[c]; arr[c] = arr[b]; arr[b] = t;
	}

	/* tuck the pivot, at arr[c], away */
	pivot = arr[c]; arr[c] = arr[b-1]; arr[b-1] = pivot;
	i = a; j = b-1;

	/* swap up & down interval to actually partition it */
	while (1) {
		/* shrink interval from bottom while elements below pivot */
		while (CMP(arr[++i], pivot) < 0)
			;
		/* shrink interval from top while elements above pivot */
		while (CMP(arr[--j], pivot) > 0)
			;
		if (j < i) break;
		/* have out-of-place elements that stopped climb & descent */
		t = arr[i]; arr[i] = arr[j]; arr[j] = t;
	}

	/* put pivot into final resting place */
	pivot = arr[i]; arr[i] = arr[b-1]; arr[b-1] = pivot;

	/* give caller the partition info */
	*pi = i; *pj = j;
}
static void
llqsort2 (SORTEL *data, ELCMPFNC cmp, VPTR param, INT a, INT b)
{
	INT stack[64];
	INT i, j, top;

	top = 0;

	while (1) {
		while (b > a+1) {
			partition2(data, cmp, param, a, b, &i, &j);
			/* loop to simulate tail recursion on smaller interval */
			if (j-a > b-i) {
				/* (i+1,b) is shorter interval than (a,j) */
				/* do (i+1, b) now, and push (a,j) */
				stack[top++] = a; stack[top++] = j;
				a=i+1;
			} else {
				/* (a,j) is shorter interval than (i+1,b) */
				/* do (a,j) now, and push (i+1,b) */
				stack[top++] = i+1; stack[top++] = b;
				b = j;
			}
		}
		if (b > a) {
			/* swap data[a] and data[b] if needed */
			if (CMP(data[a], data[b])>0) {
				SORTEL t = data[a]; data[a] = data[b]; data[b] = t;
			}
		}
		if (top == 0) break;
		/* pop & do whatever is on top of our stack */
		b = stack[--top]; a = stack[--top];
	}
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
		if (spri(d[i]) != spri(d[j])) {
			d[++j] = d[i];
		} else {
			/* TO DO - this is untested - Perry 2001/03/25 */
			delete_el(seq, d[i]);
		}
	ISize(seq) = j + 1;
	IFlags(seq) |= UNIQUED;
}
/*================================================
 * get_combined_valtype -- What valtype should the
 *  new, combined seq be ?
 * Created: 2001/01/07, Perry Rapp
 *==============================================*/
static INT
get_combined_valtype (INDISEQ one, INDISEQ two)
{
	if (length_indiseq(one) && IValtype(one) != ISVAL_NUL) {
		if (length_indiseq(two) && IValtype(two) != ISVAL_NUL) {
			ASSERT(IValtype(one) == IValtype(two));
			return IValtype(one);
		} else {
			return IValtype(one);
		}
	} else {
		return IValtype(two);
	}
}
/*===============================================
 * dupseq -- Return a duplicate of input sequence
 * Created:  2001/04/09, Perry Rapp
 * copies values from input seq using copyval
 *=============================================*/
static INDISEQ
dupseq (INDISEQ seq)
{
	INDISEQ newseq;
	STRING key;
	UNION uval;
	SORTEL *u;
	INT i,n;

	if (!seq)
		return NULL;

	/* Create New Sequence */
	newseq = create_indiseq_impl(IValtype(seq), IValfnctbl(seq));
	u = IData(seq);
	n = length_indiseq(seq);

	/* Copy Every Element */
	for (i=0; i<n; i++) {
			key = strsave(skey(u[i]));
			/* indiseq values must be copied with copyval */
			uval = copyval(seq, sval(u[i]));
			append_indiseq_impl(newseq, key, NULL, uval,
			    TRUE, TRUE);
	}
	return newseq;
}
/*===============================================
 * union_indiseq -- Create union of two sequences
 * copies values from appropriate input (taking
 *  from "one" if in both) using copyval
 *=============================================*/
INDISEQ
union_indiseq (INDISEQ one, INDISEQ two)
{
	INT n, m, i, j, rel;
	STRING key;
	INDISEQ three;
	SORTEL *u, *v;
	INT valtype;
	UNION uval;
	if (!one && !two) return NULL;
	if (!one)
		return dupseq(two);
	if (!two)
		return dupseq(one);
	if (!(IFlags(one) & KEYSORT)) keysort_indiseq(one);
	if (!(IFlags(one) & UNIQUED)) unique_indiseq(one);
	if (!(IFlags(two) & KEYSORT)) keysort_indiseq(two);
	if (!(IFlags(two) & UNIQUED)) unique_indiseq(two);
	n = length_indiseq(one);
	m = length_indiseq(two);
	valtype = get_combined_valtype(one, two);
	three = create_indiseq_impl(valtype, IValfnctbl(one));
	i = j = 0;
	u = IData(one);
	v = IData(two);
	while (i < n && j < m) {
		if ((rel = spri(u[i]) - spri(v[j])) < 0) {
			key = strsave(skey(u[i]));
			/* indiseq values must be copied with copyval */
			uval = copyval(one, sval(u[i]));
			append_indiseq_impl(three, key, NULL, uval,
			    TRUE, TRUE);
			i++;
		} else if (rel > 0) {
			key = strsave(skey(v[j]));
			/* indiseq values must be copied with copyval */
			uval = copyval(two, sval(v[j]));
			append_indiseq_impl(three, key, NULL, uval,
			    TRUE, TRUE);
			j++;
		} else { /* in both, copy value from one */
			key = strsave(skey(u[i]));
			/* indiseq values must be copied with copyval */
			uval = copyval(one, sval(u[i]));
			append_indiseq_impl(three, key, NULL, uval,
			    TRUE, TRUE);
			i++, j++;
		}
	}
	while (i < n) {
		key = strsave(skey(u[i]));
		/* indiseq values must be copied with copyval */
		uval = copyval(one, sval(u[i]));
		append_indiseq_impl(three, key, NULL, uval, TRUE, TRUE);
		i++;
	}
	while (j < m) {
		key = strsave(skey(v[j]));
		/* indiseq values must be copied with copyval */
		uval = copyval(two, sval(v[j]));
		append_indiseq_impl(three, key, NULL, uval, TRUE, TRUE);
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
 * copies values from input "one" using copyval
 *========================================================*/
INDISEQ
intersect_indiseq (INDISEQ one, INDISEQ two)
{
	INT n, m, i, j, rel;
	STRING key;
	INDISEQ three;
	SORTEL *u, *v;
	INT valtype;
	UNION uval;
	if (!one || !two) return NULL;
	if (!(IFlags(one) & KEYSORT)) keysort_indiseq(one);
	if (!(IFlags(one) & UNIQUED)) unique_indiseq(one);
	if (!(IFlags(two) & KEYSORT)) keysort_indiseq(two);
	if (!(IFlags(two) & UNIQUED)) unique_indiseq(two);
	n = length_indiseq(one);
	m = length_indiseq(two);
	valtype = get_combined_valtype(one, two);
	three = create_indiseq_impl(valtype, IValfnctbl(one));
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
			/* indiseq values must be copied with copyval */
			uval = copyval(one, sval(u[i]));
			append_indiseq_impl(three, key, NULL, uval,TRUE, TRUE);
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
 * copies values from original seq using copyval
 *=======================================================*/
INDISEQ
difference_indiseq (INDISEQ one, INDISEQ two)
{
	INT n, m, i, j, rel;
	STRING key;
	INDISEQ three;
	SORTEL *u, *v;
	INT valtype;
	UNION uval;
	if (!one)
		return NULL;
	if (!two)
		return dupseq(one);
	if (!(IFlags(one) & KEYSORT)) keysort_indiseq(one);
	if (!(IFlags(one) & UNIQUED)) unique_indiseq(one);
	if (!(IFlags(two) & KEYSORT)) keysort_indiseq(two);
	if (!(IFlags(two) & UNIQUED)) unique_indiseq(two);
	n = length_indiseq(one);
	m = length_indiseq(two);
	valtype = get_combined_valtype(one, two);
	three = create_indiseq_impl(valtype, IValfnctbl(one));
	i = j = 0;
	u = IData(one);
	v = IData(two);
	while (i < n && j < m) {
		if ((rel = spri(u[i]) - spri(v[j])) < 0) {
			key = strsave(skey(u[i]));
			/* indiseq values must be copied with copyval */
			uval = copyval(one, sval(u[i]));
			append_indiseq_impl(three, key, NULL, uval, TRUE, TRUE);
			i++;
		} else if (rel > 0) {
			j++;
		} else {
			i++, j++;
		}
	}
	while (i < n) {
		key = strsave(skey(u[i]));
		/* indiseq values must be copied with copyval */
		uval = copyval(one, sval(u[i]));
		append_indiseq_impl(three, key, NULL, uval, TRUE, TRUE);
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
 * copies values from original seq using copyval
 *===================================================*/
INDISEQ
parent_indiseq (INDISEQ seq)
{
	TABLE tab=0; /* table of people inserted (values not used) */
	INDISEQ par=0;
	NODE indi=0;
	INT fnum=0, snum=0;
	STRING key=0;
	UNION uval;
	if (!seq) return NULL;
	tab = create_table_vptr();
	par = create_indiseq_impl(IValtype(seq), IValfnctbl(seq));
	FORINDISEQ(seq, el, num)
		indi = key_to_indi(skey(el));
		FORFAMCS(indi, fam, fath, moth, fnum)
			FORFAMSPOUSES(fam, spouse, snum)
				if (spouse && !in_table(tab, key = indi_to_key(spouse))) {
					/* indiseq values must be copied with copyval */
					uval = copyval(seq, sval(el));
					key = strsave(key);
					append_indiseq_impl(par, key, NULL, uval, TRUE, TRUE);
					insert_table_ptr(tab, key, 0);
				}
			ENDFAMSPOUSES
		ENDFAMCS
	ENDINDISEQ
	destroy_table(tab);
	return par;
}
/*======================================================
 * child_indiseq -- Create children sequence of sequence
 * copies values from original seq using copyval
 *====================================================*/
INDISEQ
child_indiseq (INDISEQ seq)
{
	INT num1, num2;
	TABLE tab=0; /* table of people already inserted (values not used) */
	INDISEQ cseq=0;
	NODE indi=0;
	STRING key=0;
	UNION uval;
	if (!seq) return NULL;
	tab = create_table_vptr();
	cseq = create_indiseq_impl(IValtype(seq), IValfnctbl(seq));
	FORINDISEQ(seq, el, num)
		indi = key_to_indi(skey(el));
		FORFAMS(indi, fam, num1)
			FORCHILDRENx(fam, chil, num2)
				key = indi_to_key(chil);
				if (!in_table(tab, key)) {
					key = strsave(key);
					/* indiseq values must be copied with copyval */
					uval = copyval(seq, sval(el));
					append_indiseq_impl(cseq, key, NULL, uval, TRUE, TRUE);
					insert_table_ptr(tab, key, 0);
				}
			ENDCHILDRENx
		ENDFAMS
	ENDINDISEQ
	destroy_table(tab);
	return cseq;
}
/*=========================================================
 * indi_to_children -- Create sequence of person's children
 *  (filters out duplicates)
 *=======================================================*/
INDISEQ
indi_to_children (NODE indi)
{
	INDISEQ seq;
	INT num1, num2, len = 0;
	STRING key;
	if (!indi) return NULL;
	seq = create_indiseq_null();
	FORFAMS(indi, fam, num1)
		FORCHILDRENx(fam, chil, num2)
			len++;
			key = indi_to_key(chil);
			append_indiseq_null(seq, key, NULL, FALSE, FALSE);
		ENDCHILDRENx
	ENDFAMS
	if (len) return seq;
	remove_indiseq(seq);
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
	seq = create_indiseq_ival();
	IPrntype(seq) = ISPRN_SPOUSESEQ;
	FORFAMS(indi, fam, num)
		FORFAMSPOUSES(fam, spouse, num1)
			if (spouse != indi) {
				len++;
				key = indi_to_key(spouse);
				val = atoi(fam_to_key(fam) + 1);
				append_indiseq_ival(seq, key, NULL, val, TRUE, FALSE);
			}
		ENDFAMSPOUSES
	ENDFAMS
	if (!len) {
		remove_indiseq(seq);
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
	seq = create_indiseq_null();
	FORFAMCS(indi, fam, fath, moth, num1)
		FORHUSBS(fam, husb, num2)
			len++;
			key = indi_to_key(husb);
			append_indiseq_null(seq, key, NULL, TRUE, FALSE);
		ENDHUSBS
	ENDFAMCS
	if (len) return seq;
	remove_indiseq(seq);
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
	seq = create_indiseq_null();
	FORFAMCS(indi, fam, fath, moth, num1)
		FORWIFES(fam, wife, num2)
			len++;
			key = indi_to_key(wife);
			append_indiseq_null(seq, key, NULL, TRUE, FALSE);
		ENDWIFES
	ENDFAMCS
	if (len) return seq;
	remove_indiseq(seq);
	return NULL;
}
/*=========================================================
 * indi_to_families -- Create sequence of person's families
 *  values will be spouse keynums
 *  (this is the ISPRN_FAMSEQ style)
 * indi: [IN]  find families of this person
 * fams: [IN]  if TRUE, find spousal families, else parental
 *=======================================================*/
INDISEQ
indi_to_families (NODE indi, BOOLEAN fams)
{
	INDISEQ seq;
	INT num, num2, val;
	STRING key;
	INT mykeynum=0;
	if (!indi) return NULL;
	mykeynum = atoi(indi_to_key(indi) + 1);
	seq = create_indiseq_ival();
	IPrntype(seq) = ISPRN_FAMSEQ;
	if (fams) {
		FORFAMS(indi, fam, num)
		{
			INT spkeynum=0;
			STRING fkey = strsave(fam_to_key(fam));
			/* look for a spouse besides indi */
			FORFAMSPOUSES(fam, spouse, num2)
			{
				INT temp = atoi(indi_to_key(spouse) + 1);
				if (temp && temp != mykeynum)
					spkeynum = temp;
			}
			ENDFAMSPOUSES
			append_indiseq_ival(seq, fkey, NULL, spkeynum, TRUE, FALSE);
			strfree(&fkey);
		}
		ENDFAMS
	} else {
		FORFAMCS(indi, fam, fath, moth, num)
			key = fam_to_key(fam);
			val = fath ? atoi(indi_to_key(fath) + 1) : 0;
			append_indiseq_ival(seq, key, NULL, val, TRUE, FALSE);
		ENDFAMCS
	}
	if (num && ISize(seq)) return seq;
	remove_indiseq(seq);
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
	seq = create_indiseq_null();
	FORCHILDRENx(fam, chil, num)
		key = indi_to_key(chil);
		append_indiseq_null(seq, key, NULL, TRUE, FALSE);
	ENDCHILDRENx
	if (num && ISize(seq)) return seq;
	remove_indiseq(seq);
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
	seq = create_indiseq_null();
	FORHUSBS(fam, husb, num)
		key = indi_to_key(husb);
		append_indiseq_null(seq, key, NULL, TRUE, FALSE);
	ENDHUSBS
	if (num && ISize(seq)) return seq;
	remove_indiseq(seq);
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
	seq = create_indiseq_null();
	FORWIFES(fam, wife, num)
		key = indi_to_key(wife);
		append_indiseq_null(seq, key, NULL, TRUE, FALSE);
	ENDWIFES
	if (num && ISize(seq)) return seq;
	remove_indiseq(seq);
	return NULL;
}
/*=========================================================
 * sibling_indiseq -- Create sibling sequence of a sequence
 * close: if TRUE, do not include duplicates
 *=======================================================*/
INDISEQ
sibling_indiseq (INDISEQ seq, BOOLEAN close)
{
	INDISEQ fseq=0, sseq=0;
	NODE indi=0, fam=0;
	STRING key=0, fkey=0;
	INT num2=0;
	/* table lists people already listed (values unused) */
	TABLE tab = create_table_vptr();
	fseq = create_indiseq_null(); /* temporary */
	sseq = create_indiseq_null();
	FORINDISEQ(seq, el, num)
		indi = key_to_indi(skey(el));
		if ((fam = indi_to_famc(indi))) {
			fkey = fam_to_key(fam);
			append_indiseq_null(fseq, fkey, NULL, FALSE, FALSE);
		}
		if (!close) insert_table_ptr(tab, skey(el), 0);
	ENDINDISEQ
	FORINDISEQ(fseq, el, num)
		fam = key_to_fam(skey(el));
		FORCHILDRENx(fam, chil, num2)
			key = indi_to_key(chil);
			if (!in_table(tab, key)) {
				key = strsave(key);
				append_indiseq_null(sseq, key, NULL, TRUE, TRUE);
				insert_table_ptr(tab, key, 0);
			}
		ENDCHILDRENx
	ENDINDISEQ
	destroy_table(tab);
	remove_indiseq(fseq);
	return sseq;
}
/*=========================================================
 * ancestor_indiseq -- Create ancestor sequence of sequence
 *  values are created with the generation number
 *  (via value function table)
 *=======================================================*/
INDISEQ
ancestor_indiseq (INDISEQ seq)
{
	/* table lists people already listed (values unused) */
	TABLE tab;
	LIST anclist, genlist;
	INDISEQ anc=0;
	NODE indi=0;
	STRING key, pkey;
	INT gen=0;
	INT fnum=0, snum=0;
	UNION uval;
	if (!seq) return NULL;
		/* table of people already added */
	tab = create_table_vptr();
		/* paired processing list - see comments in descendant_indiseq code */
	anclist = create_list();
	genlist = create_list();
	anc = create_indiseq_impl(IValtype(seq), IValfnctbl(seq));
	FORINDISEQ(seq, el, num)
		enqueue_list(anclist, (VPTR)skey(el));
		enqueue_list(genlist, (VPTR)0);
	ENDINDISEQ
	while (!is_empty_list(anclist)) {
		key = (STRING) dequeue_list(anclist);
		gen = (INT) dequeue_list(genlist) + 1;
		indi = key_to_indi(key);

		FORFAMCS(indi, fam, fath, moth, fnum)
			FORFAMSPOUSES(fam, spouse, snum)
				if (spouse && !in_table(tab, pkey = indi_to_key(spouse))) {
						/* copy key for list, others make their own copies */
					uval = creategenval(seq, gen);
					append_indiseq_pval(anc, pkey, NULL, uval.w, TRUE);
					enqueue_list(anclist, (VPTR)strsave(pkey));
					enqueue_list(genlist, (VPTR)gen);
					insert_table_ptr(tab, pkey, 0);
				}
			ENDFAMSPOUSES
		ENDFAMCS
	}
	destroy_table(tab);
	destroy_empty_list(anclist);
	destroy_empty_list(genlist);
	return anc;
}
/*=============================================================
 * descendant_indiseq -- Create descendant sequence of sequence
 *  values are created with the generation number
 *  (passed to create_value callback)
 *===========================================================*/
INDISEQ
descendent_indiseq (INDISEQ seq)
{
	INT gen;
	/* itab lists people already entered, ftab families
	(values in both are unused) */
	TABLE itab, ftab;
	LIST deslist, genlist;
	INDISEQ des;
	NODE indi;
	STRING key, dkey, fkey;
	UNION uval;
	if (!seq) return NULL;
		/* itab = people already added, value irrelevant */
	itab = create_table_vptr();
		/* ftab = families already added (processed), value irrelevant */
	ftab = create_table_vptr();
		/*
		deslist & genlist are paired - 
		dequeue the person from deslist & the generation
		from genlist, for one person to be processed
		(added to result & all children added to processing list)
		(deslist does not own its strings)
		*/
	deslist = create_list();
	genlist = create_list();
		/* result indiseq */
	des = create_indiseq_impl(IValtype(seq), IValfnctbl(seq));
		/* add everyone from original seq to processing list */
	FORINDISEQ(seq, el, num)
		enqueue_list(deslist, (VPTR)skey(el));
		enqueue_list(genlist, (VPTR)0);
	ENDINDISEQ
		/* loop until processing list is empty */
	while (!is_empty_list(deslist)) {
		INT num1, num2;
		key = (STRING) dequeue_list(deslist);
		gen = (INT) dequeue_list(genlist) + 1;
		indi = key_to_indi(key);
		FORFAMS(indi, fam, num1)
				/* skip families already processed */
			if (in_table(ftab, fkey = fam_to_key(fam)))
				goto a;
			insert_table_ptr(ftab, fkey, 0);
			FORCHILDRENx(fam, child, num2)
					/* only do people not processed */
				if (!in_table(itab,
				    dkey = indi_to_key(child))) {
						/* copy key for list, others make their own copies */
					uval = creategenval(seq, gen);
						/* add person to output */
					append_indiseq_pval(des, strsave(dkey), NULL, uval.w, TRUE);
						/* also want descendants, so add person to processing list */
					enqueue_list(deslist, (VPTR)strsave(dkey));
					enqueue_list(genlist, (VPTR)gen);
					insert_table_ptr(itab, dkey, 0);
				}
			ENDCHILDRENx
		a:;
		ENDFAMS
	}
	destroy_table(itab);
	destroy_table(ftab);
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
	tab = create_table_vptr();
	sps = create_indiseq_impl(IValtype(seq), IValfnctbl(seq));
	FORINDISEQ(seq, el, num)
		indi = key_to_indi(skey(el));
		FORSPOUSES(indi, spouse, fam, num1)
			spkey = indi_to_key(spouse);
			if (!in_table(tab, spkey)) {
				UNION u;
					/* key copy for seq & list - owned by seq */
				spkey = strsave(spkey);
				u = copyval(seq, sval(el));
				append_indiseq_impl(sps, spkey, NULL, u, TRUE, TRUE);
				insert_table_ptr(tab, spkey, 0);
			}
		ENDSPOUSES
	ENDINDISEQ
	destroy_table(tab);
	return sps;
}
/*============================================================
 * name_to_indiseq -- Return person sequence whose names match
 *==========================================================*/
INDISEQ
name_to_indiseq (STRING name)
{
	INDISEQ seq = NULL;
	LIST list=0;
	if (!name || *name == 0) return NULL;

	list = find_indis_by_name(name);
	ASSERT(list);
	if (length_list(list)) {
		LIST_ITER listit=0;
		VPTR ptr=0;
		/* add all entries from list to sequence */
		seq = create_indiseq_null();
		listit = begin_list(list);
		while (next_list_ptr(listit, &ptr)) {
			CNSTRING key = (CNSTRING)ptr;
			/* skip dupe check, because we do unique_indiseq below */
			append_indiseq_null(seq, strsave(key), NULL, TRUE, TRUE);
		}
		end_list_iter(&listit);
		/* may have duplicates from multiple soundexes or wildcard searches */
		unique_indiseq(seq);
		/* people like to see lists alphabetically by name */
		namesort_indiseq(seq);
	}
	destroy_list(list);
	return seq;
}
/*===========================================
 * generic_print_el -- Format a print line of
 *  sequence of indis
 *  returns heap-alloc'd string
 *
 *  seq:  [in] sequence containing desired element
 *  i:    [in] index of desired element
 *  len:  [in] max width description desired
 *  rfmt: [in] reformatting information
 *=========================================*/
static STRING
generic_print_el (INDISEQ seq, INT i, INT len, RFMT rfmt)
{
	STRING key, name;
	element_indiseq(seq, i, &key, &name);
	return generic_to_list_string(NULL, key, len, ", ", rfmt, TRUE);
}
/*=============================================
 * spouseseq_print_el -- Format a print line of
 *  sequence of spouses
 * assume values are family keys
 *===========================================*/
static STRING
spouseseq_print_el (INDISEQ seq, INT i, INT len, RFMT rfmt)
{
	NODE indi, fam;
	STRING key, name, str;
	INT val;
	element_indiseq_ival(seq, i, &key, &val, &name);
	indi = key_to_indi(key);
	fam = keynum_to_fam(val);
	str = indi_to_list_string(indi, fam, len, rfmt, TRUE);
	return str;
}
/*==========================================
 * famseq_print_el -- Format a print line of
 *  sequence of families
 * assume values are spouse keys
 *========================================*/
static STRING
famseq_print_el (INDISEQ seq, INT i, INT len, RFMT rfmt)
{
	NODE fam, spouse;
	STRING key, name, str;
	INT val;
	element_indiseq_ival(seq, i, &key, &val, &name);
	fam = key_to_fam(key);
	spouse = ( val ? keynum_to_indi(val) : NULL);
	str = indi_to_list_string(spouse, fam, len, rfmt, TRUE);
	return str;
}
/*================================================
 * get_print_el -- Get appropriate format line for
 *  one element of an indiseq
 *==============================================*/
static STRING
get_print_el (INDISEQ seq, INT i, INT len, RFMT rfmt)
{
	STRING str;
	switch(IPrntype(seq)) {
	case ISPRN_FAMSEQ: str = famseq_print_el(seq, i, len, rfmt); break;
	case ISPRN_SPOUSESEQ: str = spouseseq_print_el(seq, i, len, rfmt); break;
	default: str = generic_print_el(seq, i, len, rfmt); break;
	}
	return str;
}
/*================================================
 * print_indiseq_element -- Format a print line of
 *  an indiseq (any type)
 *
 *  seq:  [in] indiseq of interest
 *  i:    [in] index of desired element
 *  buf:  [out] buffer to which to print description
 *  len:  [in] max length of buffer
 *  rfmt: [in] reformatting info
 *==============================================*/
void
print_indiseq_element (INDISEQ seq, INT i, STRING buf, INT len, RFMT rfmt)
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
		str = get_print_el(seq, i, len-1, rfmt);
		alloc=TRUE;
	}
	llstrcatn(&ptr, str, &len);
	if (alloc)
		stdfree(str);
}
/*=====================================================
 * preprint_indiseq -- Preformat print lines of indiseq
 *  seq:  [in] sequence to prepare (for display)
 *  len:  [in] max line width desired
 *  rfmt: [in] reformatting info
 *===================================================*/
void
preprint_indiseq (INDISEQ seq, INT len, RFMT rfmt)
{
	FORINDISEQ(seq, el, num)
		sprn(el) = get_print_el(seq, num, len, rfmt);
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
	seq = create_indiseq_null();
	for (i = 0; i < num; i++) {
		append_indiseq_null(seq, keys[i], NULL, FALSE, FALSE);
	}
	if (sort == NAMESORT)
		namesort_indiseq(seq);
	else
		keysort_indiseq(seq);
	return seq;
}
/*=============================================================
 * key_to_indiseq -- Return person sequence of the matching key
 *  name:  [IN]  name to search for
 *  ctype: [IN]  type of record (eg, 'I') (0 for any)
 * returns NULL if name is empty or not found
 *=============================================================*/
INDISEQ
key_to_indiseq (STRING name, char ctype)
{
	STRING key=0;
	INDISEQ seq = NULL;
	RECORD rec=0;
	if (!name) return NULL;
	rec = id_by_key(name, ctype);
	if (!rec) return NULL;
	key = rmvat(nxref(nztop(rec)));
	release_record(rec); /* done with record */
	seq = create_indiseq_null();
	append_indiseq_null(seq, key, NULL, FALSE, FALSE);
	return seq;
}
/*=============================================================
 * rec_to_indiseq -- Return new sequence containing specified record
 *  rec:  [IN]  record to put in list
 * returns NULL if record is null
 *=============================================================*/
#ifdef UNUSED_CODE
INDISEQ
rec_to_indiseq (RECORD rec)
{
	STRING key=0;
	INDISEQ seq = NULL;
	if (!rec) return NULL;
	key = rmvat(nxref(nztop(rec)));
	seq = create_indiseq_null();
	append_indiseq_null(seq, key, NULL, FALSE, FALSE);
	return seq;
}
#endif /* UNUSED CODE */
/*===========================================================
 * str_to_indiseq -- Return person sequence matching a string
 *  name:  [IN]  name to search for
 *  ctype: [IN]  type of record (eg, 'I') (0 for any)
 * The rules of search precedence are implemented here:
 *  1. named indiset
 *  2. key, with or without the leading "I"
 *  3. REFN
 *  4. name
 * Returned indiseq is null type
 *===========================================================*/
INDISEQ
str_to_indiseq (STRING name, char ctype)
{
	INDISEQ seq;
	/* 'B' is a code meaning try persons first, but allow any type */
	char cmatch = (ctype == 'B' ? 0 : ctype);
	seq = find_named_seq(name);
	if (!seq) seq = key_to_indiseq(name, cmatch);
	if (!seq) seq = refn_to_indiseq(name, cmatch, NAMESORT);
	if (!seq) seq = name_to_indiseq(name);
	return seq;
}
/*=======================================================
 * append_all_tags -- append all tags of specified type
 *  to indiseq (optionally recursive
 * Created: 2000/11/29, Perry Rapp
 *=====================================================*/
static void
append_all_tags(INDISEQ seq, NODE node, STRING tagname
	, BOOLEAN recurse, BOOLEAN nonptrs)
{
	if (!tagname || (ntag(node) && eqstr(ntag(node), tagname))) {
		STRING key;
		INT val=0;
		key = nval(node);
		if (key && key[0]) {
			INT keylen = strlen(key);
			STRING skey = 0;
			BOOLEAN include=TRUE;
			strupdate(&skey, rmvat(key));
			if (skey) {
				val = atoi(skey+1);
			} else {
				if (nonptrs) {
					ZSTR zstr = zs_newn(keylen+100);
					NODE chil;
					/* include non-pointers, but mark invalid with val==-1 */
					val = -1;
					zs_sets(zstr, key);
					/* collect any CONC or CONT children */
					for (chil = nchild(node); chil; chil=nsibling(chil)) {
						STRING text = nval(chil) ? nval(chil) : "";
						BOOLEAN cr=FALSE;
						if (eqstr_ex(ntag(chil), "CONC")) {
						} else if (eqstr_ex(ntag(chil), "CONT")) {
							cr=TRUE;
						} else {
							break;
						}
						if (cr)
							zs_apps(zstr, "||");
						zs_apps(zstr, text);
					}
					strupdate(&skey, zs_str(zstr));
					zs_free(&zstr);
				} else {
					include=FALSE;
				}
			}
			if (include) {
				append_indiseq_ival(seq, skey, NULL, val, FALSE, TRUE);
			}
		}
	}
	if (nchild(node) && recurse )
		append_all_tags(seq, nchild(node), tagname, recurse, nonptrs);
	if (nsibling(node))
		append_all_tags(seq, nsibling(node), tagname, recurse, nonptrs);

}
/*=======================================================
 * node_to_sources -- Create sequence of all sources
 *  inside a node record (at any level)
 * Created: 2000/11/29, Perry Rapp
 *=====================================================*/
INDISEQ
node_to_sources (NODE node)
{
	INDISEQ seq;
	if (!node) return NULL;
	seq = create_indiseq_ival();
	append_all_tags(seq, node, "SOUR", TRUE, TRUE);
	if (!length_indiseq(seq))
	{
		remove_indiseq(seq);
		seq = NULL;
	}
	return seq;
}
/*=======================================================
 * node_to_notes -- Create sequence of all notes
 *  inside a node record (at any level)
 * 2001/02/11, Perry Rapp
 *=====================================================*/
INDISEQ
node_to_notes (NODE node)
{
	INDISEQ seq;
	if (!node) return NULL;
	seq = create_indiseq_ival();
	append_all_tags(seq, node, "NOTE", TRUE, TRUE);
	if (!length_indiseq(seq))
	{
		remove_indiseq(seq);
		seq = NULL;
	}
	return seq;
}
/*=======================================================
 * node_to_pointers -- Create sequence of all pointers
 *  inside a node record (at any level)
 * 2001/02/24, Perry Rapp
 *=====================================================*/
INDISEQ
node_to_pointers (NODE node)
{
	INDISEQ seq;
	if (!node) return NULL;
	seq = create_indiseq_ival();
	append_all_tags(seq, node, NULL, TRUE, FALSE);
	if (!length_indiseq(seq))
	{
		remove_indiseq(seq);
		seq = NULL;
	}
	return seq;
}
/*=======================================================
 * get_all_sour -- Create sequence of all sources
 * Created: 2000/11/29, Perry Rapp
 *=====================================================*/
INDISEQ
get_all_sour (void)
{
	INDISEQ seq=NULL;
	int i=0;
	while ((i=xref_nexts(i)))
	{
		static char skey[10];
		if (!seq)
			seq = create_indiseq_ival();
		sprintf(skey, "S%d", i);
		append_indiseq_ival(seq, skey, NULL, i, TRUE, FALSE);
	}
	return seq;
}		
/*=======================================================
 * get_all_even -- Create sequence of all event records
 * Created: 2000/11/29, Perry Rapp
 *=====================================================*/
INDISEQ
get_all_even (void)
{
	INDISEQ seq=NULL;
	INT i=0;
	while ((i=xref_nexte(i)))
	{
		static char skey[10];
		if (!seq)
			seq = create_indiseq_ival();
		sprintf(skey, "E%ld", i);
		append_indiseq_ival(seq, skey, NULL, i, TRUE, FALSE);
	}
	return seq;
}		
/*=======================================================
 * get_all_othe -- Create sequence of all other records
 * Created: 2000/11/29, Perry Rapp
 *=====================================================*/
INDISEQ
get_all_othe (void)
{
	INDISEQ seq=NULL;
	INT i=0;
	while ((i=xref_nextx(i)))
	{
		static char skey[10];
		if (!seq)
			seq = create_indiseq_ival();
		sprintf(skey, "X%ld", i);
		append_indiseq_ival(seq, skey, NULL, i, TRUE, FALSE);
	}
	return seq;
}		
/*=======================================================
 * indiseq_is_valtype_ival -- Is this full of ivals ?
 * Created: 2001/01/21, Perry Rapp
 *=====================================================*/
BOOLEAN
indiseq_is_valtype_ival (INDISEQ seq)
{
	return IValtype(seq) == ISVAL_INT;
}
/*=======================================================
 * indiseq_is_valtype_null -- Is this full of null values ?
 * Created: 2001/01/21, Perry Rapp
 *=====================================================*/
BOOLEAN
indiseq_is_valtype_null (INDISEQ seq)
{
	return IValtype(seq) == ISVAL_NUL;
}
/*=======================================================
 * get_indiseq_ival -- Get ival from an indiseq element
 * Created: 2001/01/21, Perry Rapp
 *=====================================================*/
INT
get_indiseq_ival (INDISEQ seq, INT i)
{
	ASSERT(i >= 0);
	ASSERT(i < ISize(seq));
	ASSERT(IValtype(seq) == ISVAL_INT || IValtype(seq) == ISVAL_NUL);
	return sval(IData(seq)[i]).i;

}
/*=======================================================
 * set_indiseq_value_funcs -- Set the value function table for an INDISEQ
 * This is to allow the report layer to register its functions to 
 * create, compare, or delete values (so it can work with pvalues)
 * Created: 2001/03/25, Perry Rapp
 *=====================================================*/
void
set_indiseq_value_funcs (INDISEQ seq, INDISEQ_VALUE_FNCTABLE valfnctbl)
{
	IValfnctbl(seq) = valfnctbl;
}
/*=======================================================
 * default_copy_value -- copy a value
 * (reports supply their own callback to replace this)
 * Created: 2001/03/25, Perry Rapp
 *=====================================================*/
UNION
default_copy_value (UNION uval, INT valtype)
{
	UNION retval;
	/* only copy ints - all other values turn to NULL */
	if (valtype == ISVAL_INT)
		retval.i = uval.i;
	else
		retval.w = 0;
	return retval;
}
/*=======================================================
 * default_delete_value -- delete a value
 * (reports supply their own callback to replace this)
 * Created: 2001/03/25, Perry Rapp
 *=====================================================*/
void
default_delete_value (UNION uval, INT valtype)
{
	if (valtype == ISVAL_STR) {
		if (uval.w) {
			STRING str = (STRING)uval.w;
			stdfree(str);
			uval.w = NULL;
		}
	}
}
/*=======================================================
 * default_create_gen_value -- create a value for a specific
 *  generation (for ancestorset or descendantset)
 *  default implementation
 * (reports supply their own callback to replace this)
 * Created: 2001/03/25, Perry Rapp
 *=====================================================*/
UNION
default_create_gen_value (INT gen, INT * valtype)
{
	UNION uval;
	if (*valtype == ISVAL_INT)
		uval.i = gen;
	else
		uval.w = NULL;
	return uval;
}
/*===========================================================
 * default_compare_values -- compare values of two elements
 * Created: 2002/02/19, Perry Rapp
 *=========================================================*/
INT
default_compare_values (VPTR ptr1, VPTR ptr2, INT valtype)
{
	valtype = valtype; /* unused */
	/* We don't know how to deal with ptrs here */
	/* Let's just sort them in memory order */
	return (INT)ptr1 - (INT)ptr2;
}
/*=======================================================
 * calc_indiseq_names -- fill in element names
 *  for any persons on list with names
 * Created: 2002/02/14
 *=====================================================*/
void
calc_indiseq_names (INDISEQ seq)
{
	if (IFlags(seq) & WITHNAMES)
		return;

	FORINDISEQ(seq, el, num)
		if (*skey(el)=='I' && !snam(el)) {
			STRING name = qkey_to_name(skey(el));
			if (name)
				snam(el) = strsave(name);
		}
	ENDINDISEQ
	IFlags(seq) |= WITHNAMES;
}
/*=======================================================
 * calc_indiseq_name_el -- Try to find name of requested element
 *  if needed & appropriate
 *=====================================================*/
static void
calc_indiseq_name_el (INDISEQ seq, INT index)
{
	STRING key, name;

	ASSERT(seq);
	ASSERT(index>=0);
	ASSERT(index<ISize(seq));

	if (IFlags(seq) & WITHNAMES)
		return;

	if (snam(IData(seq)[index]))
		return;

	key = skey(IData(seq)[index]);
	if (key[0] != 'I')
		return;

	name = qkey_to_name(key);
	if (name)
		snam(IData(seq)[index]) = strsave(name);
}
/*=======================================================
 * qkey_to_name -- find the name for person with given key
 *  key: [IN]  key of person (eg, "46")
 * Note: key must be non-NULL, but may be invalid
 * Returns pointer into node cache memory
 * Created: 2002/02/14
 *=====================================================*/
static STRING
qkey_to_name (STRING key)
{
	NODE indi,name;

	indi = qkey_to_indi(key);
	if (!indi) return NULL;
	name = NAME(indi);
	if (!name) return NULL;
	return nval(name);
}
