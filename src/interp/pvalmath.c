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
#include "interp.h"
#include "liflines.h"
#include "feedback.h"


/*********************************************
 * local function prototypes
 *********************************************/


/*********************************************
 * local variables
 *********************************************/


/*********************************************
 * local function definitions
 * body of module
 *********************************************/


/*===============================
 * add_pvalues -- Add two PVALUEs
 * modify val1 and delete val2
 *=============================*/
void
add_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg)
{
	UNION u1, u2;

	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	u1.w = pvalue(val1);
	u2.w = pvalue(val2);
	switch (ptype(val1)) {
	case PINT:   *pvalue_to_pint(val1) += pvalue_to_int(val2); break;
	case PFLOAT: *pvalue_to_pfloat(val1) += pvalue_to_float(val2); break;
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*====================================
 * sub_pvalues -- Subtract two PVALUEs
 *==================================*/
void
sub_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg)
{
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	switch (ptype(val1)) {
	case PINT:   *pvalue_to_pint(val1) -= pvalue_to_int(val2); break;
	case PFLOAT: *pvalue_to_pfloat(val1) -= pvalue_to_float(val2); break;
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*====================================
 * mul_pvalues -- Multiply two PVALUEs
 *==================================*/
void
mul_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg)
{
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	switch (ptype(val1)) {
	case PINT:   *pvalue_to_pint(val1) *= pvalue_to_int(val2); break;
	case PFLOAT: *pvalue_to_pfloat(val1) *= pvalue_to_float(val2); break;
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*==================================
 * div_pvalues -- Divide two PVALUEs
 *================================*/
void
div_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg)
{
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	if (is_zero(val2)) {
		*eflg = TRUE;
		return;
	}
	switch (ptype(val1)) {
	case PINT:   *pvalue_to_pint(val1) /= pvalue_to_int(val2); break;
	case PFLOAT: *pvalue_to_pfloat(val1) /= pvalue_to_float(val2); break;
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*===================================
 * mod_pvalues -- Modulus two PVALUEs
 *=================================*/
void
mod_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg)
{
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	if (is_zero(val2)) {
		*eflg = TRUE;
		return;
	}
	switch (ptype(val1)) {
	case PINT:
		{
			INT i1 = pvalue_to_int(val1) % pvalue_to_int(val2);
			*pvalue_to_pint(val1) = i1;
		}
		break;
	default:
		*eflg = TRUE;
		return;
	}
	delete_pvalue(val2);
}
/*================================================
 * le_pvalues -- Check <= relation between PVALUEs
 * delete val2
 *==============================================*/
void
le_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg)
{
	BOOLEAN rel=FALSE;
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	switch (ptype(val1)) {
	case PFLOAT: 
		rel = (pvalue_to_float(val1) <= pvalue_to_float(val2)); 
		break;
	case PINT:
		rel = (pvalue_to_int(val1) <= pvalue_to_int(val2));
		break;
	default:
		*eflg = TRUE;
		break;
	}
	set_pvalue(val1, PBOOL, (VPTR)rel);
	delete_pvalue(val2);
}
/*================================================
 * ge_pvalues -- Check >= relation between PVALUEs
 * delete val2
 *==============================================*/
void
ge_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg)
{
	BOOLEAN rel=FALSE;
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;

	switch (ptype(val1)) {
	case PFLOAT: 
		rel = (pvalue_to_float(val1) >= pvalue_to_float(val2)); 
		break;
	case PINT:
		rel = (pvalue_to_int(val1) >= pvalue_to_int(val2)); 
		break;
	default:
		*eflg = TRUE;
		break;
	}
	set_pvalue(val1, PBOOL, (VPTR)rel);
	delete_pvalue(val2);
}
/*===============================================
 * lt_pvalues -- Check < relation between PVALUEs
 * delete val2
 *=============================================*/
void
lt_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg)
{
	BOOLEAN rel=FALSE;
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	switch (ptype(val1)) {
	case PFLOAT:
		rel = (pvalue_to_float(val1) < pvalue_to_float(val2)); 
		break;
	case PINT: 
		rel = (pvalue_to_int(val1) < pvalue_to_int(val2)); 
		break;
	default:
		*eflg = TRUE;
		break;
	}
	set_pvalue(val1, PBOOL, (VPTR)rel);
	delete_pvalue(val2);
}
/*===============================================
 * gt_pvalues -- Check > relation between PVALUEs
 * delete val2
 *=============================================*/
void
gt_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg)
{
	BOOLEAN rel=FALSE;
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	switch (ptype(val1)) {
	case PFLOAT: 
		rel = (pvalue_to_float(val1) > pvalue_to_float(val2)); 
		break;
	case PINT:
		rel = pvalue_to_int(val1) > pvalue_to_int(val2);
		break;
	default: 
		*eflg = TRUE;
		break;
	}
	set_pvalue(val1, PBOOL, (VPTR)rel);
	delete_pvalue(val2);
}
/*==============================
 * exp_pvalues -- Exponentiation
 * delete val2
 *============================*/
void
exp_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg)
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
			*pvalue_to_pint(val1) = xi;
		}
		break;
	case PFLOAT:
		{
			float xf=1, xe = pvalue_to_float(val1);
			for (i = 1; i <= n; i++)
				xf *= xe;
			*pvalue_to_pfloat(val1) = xf;
		}
		break;
	default: 
		*eflg=TRUE;
		break;
	}
	delete_pvalue(val2);
}
/*==================================
 * incr_pvalue -- Increment a PVALUE
 *================================*/
void
incr_pvalue (PVALUE val,
             BOOLEAN *eflg)
{
	if (*eflg) return;
	switch (ptype(val)) {
	case PINT:   *pvalue_to_pint(val) += 1; break;
	case PFLOAT: *pvalue_to_pfloat(val) += 1.; break;
	default: *eflg = TRUE; break;
	}
	return;
}
/*==================================
 * decr_pvalue -- Decrement a PVALUE
 *================================*/
void
decr_pvalue (PVALUE val,
             BOOLEAN *eflg)
{
	if (*eflg) return;
	switch (ptype(val)) {
	case PINT:   *pvalue_to_pint(val) -= 1; break;
	case PFLOAT: *pvalue_to_pfloat(val) -= 1.; break;
	default: *eflg = TRUE; break;
	}
}
/*============================
 * neg_pvalue -- Negate PVALUE
 *==========================*/
void
neg_pvalue (PVALUE val,
            BOOLEAN *eflg)
{
	if (*eflg) return;
	switch (ptype(val)) {
	case PINT:   *pvalue_to_pint(val) = -pvalue_to_int(val); break;
	case PFLOAT: *pvalue_to_pfloat(val) = -pvalue_to_float(val); break;
	default: *eflg = TRUE; return;
	}
	return;
}
/*=================================
 * is_zero -- See if PVALUE is zero
 *===============================*/
BOOLEAN
is_zero (PVALUE val)
{
	switch (ptype(val)) {
	case PINT: return pvalue_to_int(val) == 0;
	case PFLOAT: return pvalue_to_float(val) == 0.;
	default: return TRUE;
	}
}
