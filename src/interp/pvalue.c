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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * pvalue.c -- Handle prgram typed values
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *   3.0.3 - 03 Jul 96
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interp.h"

static char *ptypes[] = {
	"PNONE", "PANY", "PINT", "PLONG", "PFLOAT", "PBOOL", "PSTRING",
	"PGNODE", "PINDI", "PFAM", "PSOUR", "PEVEN", "POTHR", "PLIST",
	"PTABLE", "PSET"};

#ifndef _PROTO_H
INT bool_to_int();
FLOAT bool_to_float();
#endif

/*========================================
 * create_pvalue -- Create a program value
 *======================================*/
PVALUE create_pvalue (type, value)
INT type;
WORD value;
{
        PVALUE val;

        if (type == PNONE) return NULL;
        val = (PVALUE) stdalloc(sizeof(*val));
        switch (type) {
        case PSTRING:
		if (value) value = (WORD) strsave((STRING) value);
                break;
        case PANY: case PINT: case PFLOAT: case PLONG: case PGNODE:
        case PINDI: case PFAM: case PSOUR: case PEVEN: case POTHR:
        case PLIST: case PTABLE: case PSET:
		break;
        }
        ptype(val) = type;
        pvalue(val) = value;
        return val;
}
/*========================================
 * delete_pvalue -- Delete a program value
 *======================================*/
void delete_pvalue (val)
PVALUE val;
{
#ifdef DEBUG
	llwprintf("__delete_pvalue: val == ");
	show_pvalue(val);
	llwprintf("\n");
#endif	
        if (!val) return;
        switch (ptype(val)) {
        case PSTRING:
		if (pvalue(val)) stdfree((STRING) pvalue(val));
                break;
        case PANY: case PINT: case PFLOAT: case PLONG: case PGNODE:
        case PINDI: case PFAM: case PSOUR: case PEVEN: case POTHR:
        case PLIST: case PTABLE: case PSET:
                break;
        }
        stdfree(val);
}
/*====================================
 * copy_pvalue -- Copy a program value
 *==================================*/
PVALUE copy_pvalue (val)
PVALUE val;
{
	if (!val) {
llwprintf("copy_pvalue: copying null pvalue\n");
		return NULL;
	}
        return create_pvalue(ptype(val), pvalue(val));
}
/*==================================
 * set_pvalue -- Set a program value
 *================================*/
PVALUE set_pvalue (val, type, value)
PVALUE val;
INT type;
WORD value;
{
#ifdef DEBUG
	llwprintf("\nset_pvalue called: val=");
	show_pvalue(val);
	llwprintf(" new type=%d new value = %d\n", type, value);
#endif
	if (ptype(val) == PSTRING && pvalue(val)) stdfree(pvalue(val));
	ptype(val) = type;
	if (type == PSTRING && value) value = (WORD) strsave((STRING) value);
	pvalue(val) = value;
}
/*==================================================
 * numeric_pvalue -- See if program value is numeric
 *================================================*/
BOOLEAN numeric_pvalue (val)
PVALUE val;
{
	INT type = ptype(val);
	return type == PINT || type == PLONG || type == PFLOAT;
}
/*============================================================
 * num_conform_pvalues -- Make the types of two values conform
 *==========================================================*/
num_conform_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	INT hitype;

	ASSERT(val1 && val2);
	if (ptype(val1) == PANY && pvalue(val1) == NULL)
		ptype(val1) = ptype(val2);
	if (ptype(val2) == PANY && pvalue(val2) == NULL)
		ptype(val2) = ptype(val1);
	if (numeric_pvalue(val1) && numeric_pvalue(val2)) {
		hitype = max(ptype(val1), ptype(val2));
		if (ptype(val1) != hitype) coerce_pvalue(hitype, val1, eflg);
		if (ptype(val2) != hitype) coerce_pvalue(hitype, val2, eflg);
		return;
	}
	*eflg = TRUE;
}
/*===========================================================
 * eq_conform_pvalues -- Make the types of two values conform
 *=========================================================*/
eq_conform_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	INT hitype;

	ASSERT(val1 && val2);
	if (ptype(val1) == ptype(val2)) return;
	if (ptype(val1) == PANY && pvalue(val1) == NULL)
		ptype(val1) = ptype(val2);
	if (ptype(val2) == PANY && pvalue(val2) == NULL)
		ptype(val2) = ptype(val1);
	if (ptype(val1) == ptype(val2)) return;
	if (ptype(val1) == PINT && pvalue(val1) == 0 && !numeric_pvalue(val2))
		ptype(val1) = ptype(val2);
	if (ptype(val2) == PINT && pvalue(val2) == 0 && !numeric_pvalue(val1))
		ptype(val2) = ptype(val1);
	if (ptype(val1) == ptype(val2)) return;
	if (numeric_pvalue(val1) && numeric_pvalue(val2)) {
		hitype = max(ptype(val1), ptype(val2));
		if (ptype(val1) != hitype) coerce_pvalue(hitype, val1, eflg);
		if (ptype(val2) != hitype) coerce_pvalue(hitype, val2, eflg);
		return;
	}
	*eflg = TRUE;
}
/*=========================================================
 * coerce_pvalue -- Convert PVALUE from one type to another
 *=======================================================*/
coerce_pvalue (type, val, eflg)
INT type;	/* type to convert to */
PVALUE val;	/* old and new value */
BOOLEAN *eflg;
{
	PVALUE new;
	INT vint;
	/*LONG vlong;*/
	FLOAT vfloat;
	BOOLEAN vbool;
	UNION u;

#ifdef DEBUG
	llwprintf("coerce_pvalue: coerce ");
	show_pvalue(val);
	llwprintf(" to %s\n", ptypes[type]);
#endif
	if (*eflg) return;
	ASSERT(is_pvalue(val));
	if (type == ptype(val)) return;
	u.w = pvalue(val);
	if (type == PBOOL) {	 /* Handle PBOOL as special case */
		set_pvalue(val, PBOOL, (WORD)(u.w != NULL));
		return;
	}
	if (type == PANY) {	/* Handle PANY as a special case */
		ptype(val) = PANY;
		return;
	}
/* NEW - 7/31/95 */
	if ((ptype(val) == PANY || ptype(val) == PINT) && pvalue(val) == NULL) {
		ptype(val) = type;
		return;
	}
/* END */

	switch (ptype(val)) {

	case PINT:
		switch (type) {
		case PINT: return;
		case PLONG: /*u.l = int_to_long(u.w);*/ break;
		case PFLOAT: u.f = u.i; break;
		default: goto bad;
		}
		break;
#if 0
	case PLONG:
		switch (type) {
		case PINT: vint = long_to_int(u.w); break;
		case PLONG: return;
		case PFLOAT: vfloat = long_to_float(u.w); break;
		default: goto bad;
		}
		break;
#endif
	case PFLOAT:
		switch (type) {
		case PINT: u.i = u.f; break;
		case PLONG: /*u.l = float_to_long(u.w);*/ break;
		case PFLOAT: return;
		default: goto bad;
		}
		break;
	case PBOOL:
		switch (type) {
		case PINT: u.i = bool_to_int((BOOLEAN)u.w); break;
		case PLONG: /*u.l = bool_to_long((BOOLEAN)u.w);*/ break;
		case PFLOAT: u.f = bool_to_float((BOOLEAN)u.w); break;
		default: goto bad;
		}
		break;
	case PINDI:
		goto bad;
	case PANY:
		goto bad;
	case PGNODE:
		goto bad;
	default:
		goto bad;
	}
	ptype(val) = type;
	pvalue(val) = u.w;
	return;
bad:
	*eflg = TRUE;
	return;
}

/*===========================================================
 * is_pvalue -- Checks PVALUE for validity -- doesn't do much
 *=========================================================*/
BOOLEAN is_pvalue (pval)
PVALUE pval;
{
	if (!pval) return FALSE;
	return ptype(pval) >= PNONE  && ptype(pval) <= PSET;
}
/*=================================================
 * is_record_pvalue -- Checks PVALUE for recordness
 *===============================================*/
BOOLEAN is_record_pvalue (val)
PVALUE val;
{
	INT type = ptype(val);
	return type >= PINDI && type <= POTHR;
}

INT bool_to_int (b)
BOOLEAN b;
{
	return b ? 1 : 0;
}
FLOAT bool_to_float (b)
BOOLEAN b;
{
	return b ? 1. : 0.;
}
/*===============================
 * add_pvalues -- Add two PVALUEs
 *=============================*/
add_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	UNION u1, u2;

	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	u1.w = pvalue(val1);
	u2.w = pvalue(val2);
	switch (ptype(val1)) {
	case PINT:   u1.i += u2.i; pvalue(val1) = u1.w; break;
	case PFLOAT: u1.f += u2.f; pvalue(val1) = u1.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*====================================
 * sub_pvalues -- Subtract two PVALUEs
 *==================================*/
sub_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	UNION u1, u2;
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	u1.w = pvalue(val1);
	u2.w = pvalue(val2);
	switch (ptype(val1)) {
	case PINT:   u1.i -= u2.i; pvalue(val1) = u1.w; break;
	case PFLOAT: u1.f -= u2.f; pvalue(val1) = u1.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*====================================
 * mul_pvalues -- Multiply two PVALUEs
 *==================================*/
mul_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	UNION u1, u2;

	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	u1.w = pvalue(val1);
	u2.w = pvalue(val2);
	switch (ptype(val1)) {
	case PINT:   u1.i *= u2.i; pvalue(val1) = u1.w; break;
	case PFLOAT: u1.f *= u2.f; pvalue(val1) = u1.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*==================================
 * div_pvalues -- Divide two PVALUEs
 *================================*/
div_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	UNION u1, u2;

	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	u1.w = pvalue(val1);
	u2.w = pvalue(val2);
	if (is_zero(val2)) {
		*eflg = TRUE;
		return;
	}
	switch (ptype(val1)) {
	case PINT:   u1.i /= u2.i; pvalue(val1) = u1.w; break;
	case PFLOAT: u1.f /= u2.f; pvalue(val1) = u1.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*===================================
 * mod_pvalues -- Modulus two PVALUEs
 *=================================*/
mod_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	UNION u1, u2;
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	u1.w = pvalue(val1);
	u2.w = pvalue(val2);
	if (is_zero(val2)) {
		*eflg = TRUE;
		return;
	}
	switch (ptype(val1)) {
	case PINT:   u1.i %= u2.i; pvalue(val1) = u1.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*===================================================================+
 * eqv_pvalues -- See if two PVALUEs are equal (no change to PVALUEs)
 *==================================================================*/
BOOLEAN eqv_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
    STRING v1, v2;
    BOOLEAN rel = FALSE;
    if(val1 && val2 && (ptype(val1) == ptype(val2))) {
	switch (ptype(val1)) {
	case PSTRING:
		v1 = pvalue(val1);
		v2 = pvalue(val2);
		if(v1 && v2) rel = eqstr(v1, v2);
		else rel = (v1 == v2);
		break;
	default:
		rel = (pvalue(val1) == pvalue(val2));
		break;
	}
    }
    return rel;
}
/*===========================================
 * eq_pvalues -- See if two PVALUEs are equal
 *=========================================*/
eq_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	BOOLEAN rel;
	STRING v1, v2;

	if (*eflg) return;
	eq_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	switch (ptype(val1)) {
	case PSTRING:
		v1 = pvalue(val1);
		v2 = pvalue(val2);
		if(v1 && v2) rel = eqstr(v1, v2);
		else rel = (v1 == v2);
		break;
	default:
		rel = (pvalue(val1) == pvalue(val2));
		break;
	}
	set_pvalue(val1, PBOOL, (WORD)rel);
	delete_pvalue(val2);
}
/*===============================================
 * ne_pvalues -- See if two PVALUEs are not equal
 *=============================================*/
ne_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	BOOLEAN rel;
	STRING v1, v2;

	if (*eflg) return;
	eq_conform_pvalues(val1, val2, eflg);

#ifdef DEBUG
	llwprintf("ne_pvalues: val1, val2, rel = ");
	show_pvalue(val1);
	llwprintf(", ");
	show_pvalue(val2);
	llwprintf(", ");
#endif

	if (*eflg) return;
	switch (ptype(val1)) {
	case PSTRING:
		v1 = pvalue(val1);
		v2 = pvalue(val2);
		if(v1 && v2) rel = nestr(v1, v2);
		else rel = (v1 != v2);
		break;
	default:
		rel = (pvalue(val1) != pvalue(val2));
		break;
	}

#ifdef DEBUG
	llwprintf("%d\n", rel);
#endif

	set_pvalue(val1, PBOOL, (WORD)rel);
	delete_pvalue(val2);
}
/*================================================
 * le_pvalues -- Check <= relation between PVALUEs
 *==============================================*/
le_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	BOOLEAN rel;
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	switch (ptype(val1)) {
	/*case LONG:*/
	default: rel = ((INT) pvalue(val1) <= (INT) pvalue(val2));
	}
	set_pvalue(val1, PBOOL, (WORD)rel);
	delete_pvalue(val2);
}
/*================================================
 * ge_pvalues -- Check >= relation between PVALUEs
 *==============================================*/
ge_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	BOOLEAN rel;
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;

#ifdef DEBUG
	llwprintf("ge_pvalues: val1, val2 = ");
	show_pvalue(val1);
	llwprintf(", ");
	show_pvalue(val2);
	llwprintf("\n");
#endif

	switch (ptype(val1)) {
	/*case LONG:*/
	default: rel = ((INT) pvalue(val1) >= (INT) pvalue(val2));
	}
	set_pvalue(val1, PBOOL, (WORD)rel);
	delete_pvalue(val2);
}
/*===============================================
 * lt_pvalues -- Check < relation between PVALUEs
 *=============================================*/
lt_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	BOOLEAN rel;
	if (prog_debug) {
		llwprintf("lt_pvalues: val1 = ");
		show_pvalue(val1);
		llwprintf(" val2 = ");
		show_pvalue(val2);
		llwprintf(" eflg = %d\n", *eflg);
	}
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (prog_debug) {
		llwprintf("lt_pvalues: after conforming: val1 = ");
		show_pvalue(val1);
		llwprintf(" val2 = ");
		show_pvalue(val2);
		llwprintf(" eflg = %d\n", *eflg);
	}
	if (*eflg) return;
	switch (ptype(val1)) {
	/*case LONG:*/
	default: rel = ((INT) pvalue(val1) < (INT) pvalue(val2));
	}
	set_pvalue(val1, PBOOL, (WORD)rel);
	delete_pvalue(val2);
	if (prog_debug) {
		llwprintf("lt_pvalues: at end: val1 = ");
		show_pvalue(val1);
		llwprintf("\n");
	}
}
/*===============================================
 * gt_pvalues -- Check > relation between PVALUEs
 *=============================================*/
gt_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	BOOLEAN rel;
if (prog_debug) {
	llwprintf("gt_pvalues: at start: val1 = ");
	show_pvalue(val1);
	llwprintf(" val2 = ");
	show_pvalue(val2);
	llwprintf(" eflg = %d\n", *eflg);
}
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	switch (ptype(val1)) {
	/*case LONG:*/
	default: rel = ((INT) pvalue(val1) > (INT) pvalue(val2));
if (prog_debug) llwprintf("rel is %d\n", rel);
	}
	set_pvalue(val1, PBOOL, (WORD)rel);
	delete_pvalue(val2);
if (prog_debug) {
	llwprintf("gt_pvalues: at end: val1 = ");
	show_pvalue(val1);
	llwprintf("\n");
}
}
/*==============================
 * exp_pvalues -- Exponentiation
 *============================*/
exp_pvalues (val1, val2, eflg)
PVALUE val1, val2;
BOOLEAN *eflg;
{
	UNION u;
	INT xi, i, n;
	FLOAT xf;
	BOOLEAN rel;
	if (*eflg) return;
	coerce_pvalue(PINT, val2, eflg);
	if (*eflg) return;
	u.w = pvalue(val1);
	n = (INT) pvalue(val2);
	switch (ptype(val1)) {
	case PINT:
		xi = 1;
		for (i = 1; i <= n; i++)
			xi *= u.i;
		u.i = xi;
		break;
	case PFLOAT:
		xf = 1.;
		for (i = 1; i <= n; i++)
			xf *= u.f;
		u.f = xf;
		break;
	/*case PLONG:*/
	default: u.i = 0;
	}
	set_pvalue(val1, ptype(val1), (WORD)u.w);
	delete_pvalue(val2);
}
/*==================================
 * incr_pvalue -- Increment a PVALUE
 *================================*/
incr_pvalue (val, eflg)
PVALUE val;
BOOLEAN *eflg;
{
	UNION u;
	if (*eflg) return;
	u.w = pvalue(val);
	switch (ptype(val)) {
	case PINT:   u.i += 1;  pvalue(val) = u.w; break;
	case PFLOAT: u.f += 1.; pvalue(val) = u.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; break;
	}
	return;
}
/*==================================
 * decr_pvalue -- Decrement a PVALUE
 *================================*/
decr_pvalue (val, eflg)
PVALUE val;
BOOLEAN *eflg;
{
	UNION u;
	if (*eflg) return;
	u.w = pvalue(val);
	switch (ptype(val)) {
	case PINT:   u.i -= 1;  pvalue(val) = u.w; break;
	case PFLOAT: u.f -= 1.; pvalue(val) = u.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; break;
	}
}
/*============================
 * neg_pvalue -- Negate PVALUE
 *==========================*/
neg_pvalue (val, eflg)
PVALUE val;
BOOLEAN *eflg;
{
	UNION u;
	if (*eflg) return;
	u.w = pvalue(val);
	switch (ptype(val)) {
	case PINT:   u.i = -u.i; pvalue(val) = u.w; break;
	case PFLOAT: u.f = -u.f; pvalue(val) = u.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; return;
	}
	return;
}
/*=================================
 * is_zero -- See if PVALUE is zero
 *===============================*/
BOOLEAN is_zero (val)
PVALUE val;
{
	UNION u;
	u.w = pvalue(val);
	switch (ptype(val)) {
	case PINT: return u.i == 0;
	case PFLOAT: return u.f == 0.;
	default: return TRUE;
	}
}
/*======================================================
 * insert_pvtable -- Update symbol table with new PVALUE
 *====================================================*/
insert_pvtable (stab, iden, type, value)
TABLE stab;	/* symbol table */
STRING iden;	/* variable in symbol table */
INT type;	/* type of new value to assign to identifier */
WORD value;	/* new value of identifier */
{
	PVALUE val = (PVALUE) valueof(stab, iden);
	if (val) delete_pvalue(val);
	insert_table(stab, iden, create_pvalue(type, value));
}
#ifndef HOGMEMORY
/*=================================================
 * zero_pventry -- zero the value of a symbol table 
 *================================================*/
zero_pventry (ent)
ENTRY ent;	/* symbol table entry */
{
    PVALUE val;
    if(ent && (val = ent->evalue)) {
	ASSERT(is_pvalue(val));
	delete_pvalue(val);
	ent->evalue = 0;
    }
}
/*========================================
 * remove_pvtable -- Remove symbol table 
 *======================================*/
FILE *errfp = NULL;
remove_pvtable (stab)
TABLE stab;	/* symbol table */
{
#ifdef HOGMEMORYERROR
    if(errfp == NULL) errfp = fopen("pbm.err", "w");
    if(errfp) { fprintf(errfp, "traverse_table()...\n"); fflush(stderr); }
    	traverse_table(stab, zero_pventry);
    if(errfp) { fprintf(errfp, "remove_table(, DONTFREE)...\n"); fflush(stderr); }
#endif
	remove_table(stab, DONTFREE);
#ifdef HOGMEMORYERROR
    if(errfp) { fprintf(errfp, "remove_pvtable() done.\n"); fflush(stderr); }
#endif
}
#endif
/*=================================================
 * show_pvalue -- DEBUG routine that shows a PVALUE
 *===============================================*/
show_pvalue (val)
PVALUE val;
{
	NODE node;
	CACHEEL cel;
	INT type;
	UNION u;

	if (!is_pvalue(val)) {
		if(val) llwprintf("*NOT PVALUE (%d,?)*", ptype(val));
		else llwprintf("*NOT PVALUE (NULL)*");
		return;
	}
	type = ptype(val);
	llwprintf("<%s,", ptypes[type]);
	if (pvalue(val) == NULL) {
		llwprintf("0>");
		return;
	}
	u.w = pvalue(val);
	switch (type) {
	case PINT:
		llwprintf("%d>", u.i);
		return;
	case PFLOAT:
		llwprintf("%f>", u.f);
		break;
	case PSTRING: llwprintf("%s>", (STRING) pvalue(val)); return;
	case PINDI:
		cel = (CACHEEL) pvalue(val);
		if (!cnode(cel))
			cel = key_to_indi_cacheel(ckey(cel));
        	node = cnode(cel);
		llwprintf("%s>", nval(NAME(node)));
		return;
	default:
		llwprintf("%d>", pvalue(val)); return;
	}
}
/*======================================================
 * pvalue_to_string -- DEBUG routine that shows a PVALUE
 *====================================================*/
STRING pvalue_to_string (val)
PVALUE val;
{
	NODE node;
	CACHEEL cel;
	INT type;
	UNION u;
	static char scratch[40];
	char *p;

	if (!is_pvalue(val)) return (STRING) "*NOT PVALUE*";
	type = ptype(val);
	sprintf(scratch, "<%s,", ptypes[type]);
	p = scratch + strlen(scratch);
	if (pvalue(val) == NULL) {
		sprintf(p, "0>");
		return (STRING) scratch;
	}
	u.w = pvalue(val);
	switch (type) {
	case PINT:
		sprintf(p, "%d>", u.i);
		break;
	case PFLOAT:
		sprintf(p, "%f>", u.f);
		break;
	case PSTRING: sprintf(p, "%s>", (STRING) pvalue(val)); break;
	case PINDI:
		cel = (CACHEEL) pvalue(val);
		if (!cnode(cel))
			cel = key_to_indi_cacheel(ckey(cel));
        	node = cnode(cel);
		sprintf(p, "%s>", nval(NAME(node)));
		break;
	default:
		sprintf(p, "%d>", pvalue(val));
		break;
	}
	return (STRING) scratch;
}
