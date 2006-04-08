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
 * pvalmath.c -- Math functions with pvalues 
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 * Created: 2002.02.17 by Perry Rapp, out of pvalue.c
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interpi.h"
#include "liflines.h"
#include "feedback.h"
#include "zstr.h"


/*********************************************
 * local function prototypes
 *********************************************/

static void illegal_value(CNSTRING op, PVALUE val, BOOLEAN *eflg, ZSTR * zerr);
static void invalid_numeric_type(CNSTRING op, PVALUE val, BOOLEAN *eflg, ZSTR * zerr);
static void num_conform_pvalues(CNSTRING op, PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);

/*********************************************
 * local variables
 *********************************************/


/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*===============================
 * invalid_numeric_type -- Report error of using
 *  non-numeric type in numeric operation
 * Created: 2003-01-30 (Perry Rapp)
 *=============================*/
static void
invalid_numeric_type (CNSTRING op, PVALUE val, BOOLEAN *eflg, ZSTR * zerr)
{
	if (zerr) {
		ZSTR zt = describe_pvalue(val);
		ASSERT(!(*zerr));
		(*zerr) = zs_newf(_("Nonnumeric type to operation %s: %s"), op, zs_str(zt));
		zs_free(&zt);
	}
	*eflg = TRUE;
}
/*===============================
 * illegal_value -- Report error of using illegal value in numeric operation
 * Created: 2003-01-30 (Perry Rapp)
 *=============================*/
static void
illegal_value (CNSTRING op, PVALUE val, BOOLEAN *eflg, ZSTR * zerr)
{
	if (zerr) {
		ZSTR zt = describe_pvalue(val);
		ASSERT(!(*zerr));
		(*zerr) = zs_newf(_("Illegal value to operation %s: %s"), op, zs_str(zt));
		zs_free(&zt);
	}
	*eflg = TRUE;
}
/*===============================
 * add_pvalues -- Add two PVALUEs
 * modify val1 and delete val2
 *=============================*/
void
add_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg, ZSTR * zerr)
{
	if (*eflg) return;
	num_conform_pvalues("add", val1, val2, eflg, zerr);
	if (*eflg) return;
	switch (ptype(val1)) {
	case PINT:
		set_pvalue_int(val1, pvalue_to_int(val1) +  pvalue_to_int(val2));
		break;
	case PFLOAT:
		set_pvalue_float(val1, pvalue_to_float(val1) + pvalue_to_float(val2));
		break;
	default: invalid_numeric_type("add", val1, eflg, zerr); return;
	}
	delete_pvalue(val2);
}
/*====================================
 * sub_pvalues -- Subtract two PVALUEs
 *==================================*/
void
sub_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg, ZSTR * zerr)
{
	if (*eflg) return;
	num_conform_pvalues("sub", val1, val2, eflg, zerr);
	if (*eflg) return;
	switch (ptype(val1)) {
	case PINT:
		set_pvalue_int(val1, pvalue_to_int(val1) - pvalue_to_int(val2));
		break;
	case PFLOAT:
		set_pvalue_float(val1, pvalue_to_float(val1) - pvalue_to_float(val2));
		break;
	default: invalid_numeric_type("sub", val1, eflg, zerr); return;
	}
	delete_pvalue(val2);
}
/*====================================
 * mul_pvalues -- Multiply two PVALUEs
 *==================================*/
void
mul_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg, ZSTR * zerr)
{
	if (*eflg) return;
	num_conform_pvalues("mul", val1, val2, eflg, zerr);
	if (*eflg) return;
	switch (ptype(val1)) {
	case PINT:
		set_pvalue_int(val1, pvalue_to_int(val1) * pvalue_to_int(val2));
		break;
	case PFLOAT:
		set_pvalue_float(val1, pvalue_to_float(val1) * pvalue_to_float(val2));
		break;
	default: invalid_numeric_type("mul", val1, eflg, zerr); return;
	}
	delete_pvalue(val2);
}
/*==================================
 * div_pvalues -- Divide two PVALUEs
 *================================*/
void
div_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg, ZSTR * zerr)
{
	if (*eflg) return;
	num_conform_pvalues("div", val1, val2, eflg, zerr);
	if (*eflg) return;
	if (is_numeric_zero(val2)) { illegal_value("div", val2, eflg, zerr); return; }
	switch (ptype(val1)) {
	case PINT: 
		set_pvalue_int(val1, pvalue_to_int(val1) / pvalue_to_int(val2));
		break;
	case PFLOAT:
		set_pvalue_float(val1, pvalue_to_float(val1) / pvalue_to_float(val2));
		break;
	default: invalid_numeric_type("div", val1, eflg, zerr); return;
	}
	delete_pvalue(val2);
}
/*===================================
 * mod_pvalues -- Modulus two PVALUEs
 *=================================*/
void
mod_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg, ZSTR * zerr)
{
	if (*eflg) return;
	num_conform_pvalues("mod", val1, val2, eflg, zerr);
	if (*eflg) return;
	if (is_numeric_zero(val2)) { illegal_value("mod", val2, eflg, zerr); return; }
	switch (ptype(val1)) {
	case PINT:
		{
			INT i1 = pvalue_to_int(val1) % pvalue_to_int(val2);
			set_pvalue_int(val1, i1);
		}
		break;
	default: invalid_numeric_type("mod", val1, eflg, zerr); return;
	}
	delete_pvalue(val2);
}
/*================================================
 * le_pvalues -- Check <= relation between PVALUEs
 * Result into val1, deletes val2
 *==============================================*/
void
le_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg, ZSTR * zerr)
{
	BOOLEAN rel=FALSE;
	if (*eflg) return;
	num_conform_pvalues("le", val1, val2, eflg, zerr);
	if (*eflg) return;
	switch (ptype(val1)) {
	case PFLOAT: 
		rel = (pvalue_to_float(val1) <= pvalue_to_float(val2)); 
		break;
	case PINT:
		rel = (pvalue_to_int(val1) <= pvalue_to_int(val2));
		break;
	default: invalid_numeric_type("le", val1, eflg, zerr); return;
	}
	set_pvalue_bool(val1, rel);
	delete_pvalue(val2);
}
/*================================================
 * ge_pvalues -- Check >= relation between PVALUEs
 * Result into val1, deletes val2
 *==============================================*/
void
ge_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg, ZSTR * zerr)
{
	BOOLEAN rel=FALSE;
	if (*eflg) return;
	num_conform_pvalues("ge", val1, val2, eflg, zerr);
	if (*eflg) return;

	switch (ptype(val1)) {
	case PFLOAT: 
		rel = (pvalue_to_float(val1) >= pvalue_to_float(val2)); 
		break;
	case PINT:
		rel = (pvalue_to_int(val1) >= pvalue_to_int(val2)); 
		break;
	default: invalid_numeric_type("ge", val1, eflg, zerr); return;
	}
	set_pvalue_bool(val1, rel);
	delete_pvalue(val2);
}
/*===============================================
 * lt_pvalues -- Check < relation between PVALUEs
 * delete val2
 *=============================================*/
void
lt_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg, ZSTR * zerr)
{
	BOOLEAN rel=FALSE;
	if (*eflg) return;
	num_conform_pvalues("lt", val1, val2, eflg, zerr);
	if (*eflg) return;
	switch (ptype(val1)) {
	case PFLOAT:
		rel = (pvalue_to_float(val1) < pvalue_to_float(val2)); 
		break;
	case PINT: 
		rel = (pvalue_to_int(val1) < pvalue_to_int(val2)); 
		break;
	default: invalid_numeric_type("lt", val1, eflg, zerr); return;
	}
	set_pvalue_bool(val1, rel);
	delete_pvalue(val2);
}
/*===============================================
 * gt_pvalues -- Check > relation between PVALUEs
 * delete val2
 *=============================================*/
void
gt_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg, ZSTR * zerr)
{
	BOOLEAN rel=FALSE;
	if (*eflg) return;
	num_conform_pvalues("gt", val1, val2, eflg, zerr);
	if (*eflg) return;
	switch (ptype(val1)) {
	case PFLOAT: 
		rel = (pvalue_to_float(val1) > pvalue_to_float(val2)); 
		break;
	case PINT:
		rel = pvalue_to_int(val1) > pvalue_to_int(val2);
		break;
	default: invalid_numeric_type("gt", val1, eflg, zerr); return;
	}
	set_pvalue_bool(val1, rel);
	delete_pvalue(val2);
}
/*==============================
 * exp_pvalues -- Exponentiation
 * delete val2
 *============================*/
void
exp_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg, ZSTR * zerr)
{
	INT i, n;

	if (*eflg) return;
	coerce_pvalue(PINT, val2, eflg);
	if (*eflg) return;
	n = pvalue_to_int(val2);
	switch (ptype(val1)) {
	case PINT:
		{
			INT xi = 1, xe = pvalue_to_int(val1);
			for (i = 1; i <= n; i++)
				xi *= xe;
			set_pvalue_int(val1, xi);
		}
		break;
	case PFLOAT:
		{
			float xf=1, xe = pvalue_to_float(val1);
			for (i = 1; i <= n; i++)
				xf *= xe;
			set_pvalue_float(val1, xf);
		}
		break;
	default: invalid_numeric_type("exp", val1, eflg, zerr); return;
	}
	delete_pvalue(val2);
}
/*==================================
 * incr_pvalue -- Increment a PVALUE
 *================================*/
void
incr_pvalue (PVALUE val, BOOLEAN *eflg, ZSTR * zerr)
{
	if (*eflg) return;
	switch (ptype(val)) {
	case PINT:
		set_pvalue_int(val, pvalue_to_int(val) + 1);
		break;
	case PFLOAT:
		set_pvalue_float(val, pvalue_to_float(val) + 1.0);
		break;
	default: invalid_numeric_type("incr", val, eflg, zerr); return;
	}
	return;
}
/*==================================
 * decr_pvalue -- Decrement a PVALUE
 *================================*/
void
decr_pvalue (PVALUE val, BOOLEAN *eflg, ZSTR * zerr)
{
	if (*eflg) return;
	switch (ptype(val)) {
	case PINT:
		set_pvalue_int(val, pvalue_to_int(val) - 1);
		break;
	case PFLOAT:
		set_pvalue_float(val, pvalue_to_float(val) - 1.0);
		break;
	default: invalid_numeric_type("decr", val, eflg, zerr); return;
	}
}
/*============================
 * neg_pvalue -- Negate PVALUE
 *==========================*/
void
neg_pvalue (PVALUE val, BOOLEAN *eflg, ZSTR * zerr)
{
	if (*eflg) return;
	switch (ptype(val)) {
	case PINT:
		set_pvalue_int(val, -pvalue_to_int(val));
		break;
	case PFLOAT:
		set_pvalue_float(val, -pvalue_to_float(val));
		break;
	default: invalid_numeric_type("neg", val, eflg, zerr); return;
	}
	return;
}
/*=================================
 * is_numeric_zero -- See if numeric PVALUE is zero
 *===============================*/
BOOLEAN
is_numeric_zero (PVALUE val)
{
	switch (ptype(val)) {
	case PINT: return pvalue_to_int(val) == 0;
	case PFLOAT: return pvalue_to_float(val) == 0.;
	case PNULL: return TRUE;
	}
	ASSERT(0); /* No other numeric types */
	return FALSE;
}
/*============================================================
 * num_conform_pvalues -- Make the types of two values conform
 *==========================================================*/
static void
num_conform_pvalues (CNSTRING op, PVALUE val1, PVALUE val2, BOOLEAN *eflg, ZSTR * zerr)
{
	ASSERT(val1);
	ASSERT(val2);

	if (ptype(val1) == PNULL)
		ptype(val1) = ptype(val2);
	if (ptype(val2) == PNULL)
		ptype(val2) = ptype(val1);
	if (ptype(val1) == PNULL && ptype(val2) == PNULL)
		ptype(val1) = ptype(val2) = PINT;
	if (is_numeric_pvalue(val1) && is_numeric_pvalue(val2)) {
		INT hitype = max(ptype(val1), ptype(val2));
		if (ptype(val1) != hitype) coerce_pvalue(hitype, val1, eflg);
		if (ptype(val2) != hitype) coerce_pvalue(hitype, val2, eflg);
		if (!(*eflg)) return;
	}
	if (zerr) {
		ZSTR zt1 = describe_pvalue(val1), zt2 = describe_pvalue(val2);
		ASSERT(!(*zerr));
		(*zerr) = zs_newf(_("%s: not compatible numeric types: %s and %s")
			, op, zs_str(zt1), zs_str(zt2));
		zs_free(&zt1);
		zs_free(&zt2);
	}
	*eflg = TRUE;
}
