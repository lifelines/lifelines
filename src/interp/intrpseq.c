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
 * intrpseq.c -- Programming interface to the INDISEQ data type
 * Copyright(c) 1992-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 21 Aug 93
 *   3.0.2 - 11 Dec 94    3.0.3 - 08 Aug 95
 *===========================================================*/

#include <stdio.h>
#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "interp.h"
#include "indiseq.h"

LIST keysets;
INDISEQ union_indiseq(), intersect_indiseq(), child_indiseq();
INDISEQ parent_indiseq(), spouse_indiseq(), ancestor_indiseq();
INDISEQ descendent_indiseq(), difference_indiseq();
INDISEQ sibling_indiseq();

/*======================================================+
 * initset -- Initialize list that holds created INDISEQs
 *=====================================================*/
initset ()
{
	keysets = create_list();
}
/*======================================+
 * indiset -- Declare an INDISEQ variable
 *   indiset(VARB) -> VOID
 *=====================================*/
PVALUE __indiset (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq;
	PNODE var = (PNODE) iargs(node);
	if (!iistype(var, IIDENT)) {
		*eflg = TRUE;
		prog_error(node, "the arg to indiset is not a variable.");
		return NULL;
	}
	*eflg = FALSE;
	seq = create_indiseq();
	assign_iden(stab, iident(var), create_pvalue(PSET, seq));
	push_list(keysets, seq);
	return NULL;
}
/*==================================+
 * addtoset -- Add person to INDISEQ
 *   addtoset(SET, INDI, ANY) -> VOID
 *=================================*/
WORD __addtoset (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi;
	STRING key;
	INDISEQ seq;
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1),
	    arg3 = inext(arg2);
	PVALUE val= eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to addtoset is not a set.");
		return NULL;
	}
	ASSERT(seq = (INDISEQ) pvalue(val));
	delete_pvalue(val);
	indi = eval_indi(arg2, stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, "2nd arg to addtoset must be a person.");
		return NULL;
	}
	if (!indi) return NULL;
	*eflg = TRUE;
	if (!(key = strsave(rmvat(nxref(indi))))) {
		prog_error(node, "major error in addtoset.");
		return NULL;
	}
	*eflg = FALSE;
	val = evaluate(arg3, stab, eflg);
	if (*eflg) {
		prog_error(node, "3rd arg to addtoset is in error.");
		return NULL;
	}
	append_indiseq(seq, key, NULL, val, FALSE, TRUE);
	return NULL;
}
/*======================================+
 * lengthset -- Find length of an INDISEQ
 *   lengthset(SET) -> INT
 *=====================================*/
PVALUE __lengthset (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to lengthset must be a set.");
		return NULL;
	}
	seq = (INDISEQ) pvalue(val);
	delete_pvalue(val);
	if (!seq) return create_pvalue(PINT, 0);
	return create_pvalue(PINT, length_indiseq(seq));
}
/*====================================+
 * inset -- See if person is in INDISEQ
 *   inset(SET, INDI) -> BOOL
 *==========================================*/
PVALUE __inset (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi;
	STRING key;
	BOOLEAN all, rc;
	INDISEQ seq;
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1);
	PVALUE val = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg || !val || !pvalue(val)) {
		*eflg = TRUE;
		prog_error(node, "1st arg to inset must be a set.");
		return NULL;
	}
	seq = (INDISEQ) pvalue(val);
	delete_pvalue(val);
	indi = eval_indi(arg2, stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, "2nd arg to inset must be a person.");
		return NULL;
	}
	if (!indi) return create_pvalue(PBOOL, FALSE);
	if (!(key = strsave(rmvat(nxref(indi))))) {
		*eflg = TRUE;
		prog_error(node, "major error in inset.");
		return NULL;
	}
	return create_pvalue(PBOOL, in_indiseq(seq, key));
}
/*===========================================+
 * deletefromset -- Remove person from INDISEQ
 *   deletefromset(SET, INDI, BOOL) -> VOID
 *==========================================*/
PVALUE __deletefromset (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi;
	STRING key;
	BOOLEAN all, rc;
	INDISEQ seq;
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1),
	    arg3 = inext(arg2);
	PVALUE val = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to deletefromset must be a set.");
		return NULL;
	}
	ASSERT(seq = (INDISEQ) pvalue(val));
	delete_pvalue(val);
	indi = eval_indi(arg2, stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, "2nd arg to deletefromset must be a person.");
		return NULL;
	}
	if (!indi) return NULL;
	*eflg = TRUE;
	if (!(key = strsave(rmvat(nxref(indi))))) {
		prog_error(node, "major error in deletefromset.");
		return NULL;
	}
	*eflg = FALSE;
	val = eval_and_coerce(PBOOL, arg3, stab, eflg);
	if (*eflg) {
		prog_error(node, "3rd arg to deletefromset must be boolean.");
		return NULL;
	}
	all = (BOOLEAN) pvalue(val);
	delete_pvalue(val);
	do {
		rc = delete_indiseq(seq, key, NULL, 0);
	} while (rc && all);
	return NULL;
}
/*================================+
 * namesort -- Sort INDISEQ by name
 *   namesort(SET) -> VOID
 *===============================*/
PVALUE __namesort (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to namesort must be a set.");
		return NULL;
	}
	ASSERT(seq = (INDISEQ) pvalue(val));
	delete_pvalue(val);
	namesort_indiseq(seq);
	return NULL;
}
/*==============================+
 * keysort -- Sort INDISEQ by key
 *   keysort(SET) -> VOID
 *=============================*/
PVALUE __keysort (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to namesort must be a set.");
		return NULL;
	}
	ASSERT(seq = (INDISEQ) pvalue(val));
	delete_pvalue(val);
	keysort_indiseq(seq);
	return NULL;
}
#if 0
/*===================================
 * valuesort -- Sort INDISEQ by value
 *   valuesort(SET) -> VOID
 *=================================*/
WORD __valuesort (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(iargs(node), stab, eflg);
	if (*eflg || !seq) return NULL;
	valuesort_indiseq(seq);
	return NULL;
}
#endif
/*=========================================+
 * uniqueset -- Eliminate dupes from INDISEQ
 *   uniqueset(SET) -> VOID
 *========================================*/
PVALUE __uniqueset (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to uniqueset must be a set.");
		return NULL;
	}
	ASSERT(seq = (INDISEQ) pvalue(val));
	delete_pvalue(val);
	unique_indiseq(seq);
	return NULL;
}
/*=====================================+
 * union -- Create union of two INDISEQs
 *   union(SET, SET) -> SET
 *====================================*/
PVALUE __union (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1);
	INDISEQ op2, op1;
	PVALUE val = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to union must be a set.");
		return NULL;
	}
	ASSERT(op1 = (INDISEQ) pvalue(val));
	delete_pvalue(val);
	val = eval_and_coerce(PSET, arg2, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to union must be a set.");
		return NULL;
	}
	ASSERT(op2 = (INDISEQ) pvalue(val));
	set_pvalue(val, PSET, op2 = union_indiseq(op1, op2));
	push_list(keysets, op2);
	return val;
}
/*================================================+
 * intersect -- Create intersection of two INDISEQs
 *   intersect(SET, SET) -> SET
 *===============================================*/
PVALUE __intersect (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1);
	INDISEQ op2, op1;
	PVALUE val = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to intersect must be a set.");
		return NULL;
	}
	ASSERT(op1 = (INDISEQ) pvalue(val));
	delete_pvalue(val);
	val = eval_and_coerce(PSET, arg2, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to intersect must be a set.");
		return NULL;
	}
	ASSERT(op2 = (INDISEQ) pvalue(val));
	set_pvalue(val, PSET, op2 = intersect_indiseq(op1, op2));
	push_list(keysets, op2);
	return val;
}
/*===============================================+
 * difference -- Create difference of two INDISEQs
 *   difference(SET, SET) -> SET
 *==============================================*/
PVALUE __difference (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1);
	INDISEQ op2, op1;
	PVALUE val = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to difference must be a set.");
		return NULL;
	}
	ASSERT(op1 = (INDISEQ) pvalue(val));
	delete_pvalue(val);
	val = eval_and_coerce(PSET, arg2, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to difference must be a set.");
		return NULL;
	}
	ASSERT(op2 = (INDISEQ) pvalue(val));
	set_pvalue(val, PSET, op2 = difference_indiseq(op1, op2));
	push_list(keysets, op2);
	return val;
}
/*=========================================+
 * parentset -- Create parent set of INDISEQ
 *   parentset(SET) -> SET
 *========================================*/
PVALUE __parentset (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to parentset must be a set.");
		return NULL;
	}
	ASSERT(seq = (INDISEQ) pvalue(val));
	set_pvalue(val, PSET, seq = parent_indiseq(seq));
	push_list(keysets, seq);
	return val;
}
/*==========================================+
 * childset -- Create child set of an INDISEQ
 *   childset(SET) -> SET
 *=========================================*/
PVALUE __childset (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to childset must be a set.");
		return NULL;
	}
	ASSERT(seq = (INDISEQ) pvalue(val));
	set_pvalue(val, PSET, seq = child_indiseq(seq));
	push_list(keysets, seq);
	return val;
}
/*==============================================+
 * siblingset -- Create sibling set of an INDISEQ
 *   siblingset(SET) -> SET
 *=============================================*/
PVALUE __siblingset (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to siblingset must be a set.");
		return NULL;
	}
	ASSERT(seq = (INDISEQ) pvalue(val));
	set_pvalue(val, PSET, seq = sibling_indiseq(seq));
	push_list(keysets, seq);
	return val;
}
/*============================================+
 * spouseset -- Create spouse set of an INDISEQ
 *   spouseset(SET) -> SET
 *===========================================*/
PVALUE __spouseset (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to spouseset must be a set.");
		return NULL;
	}
	ASSERT(seq = (INDISEQ) pvalue(val));
	set_pvalue(val, PSET, seq = spouse_indiseq(seq));
	push_list(keysets, seq);
	return val;
}
/*================================================+
 * ancestorset -- Create ancestor set of an INDISEQ
 *   ancestorset(SET) -> SET
 *===============================================*/
PVALUE __ancestorset (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to ancestorset must be a set.");
		return NULL;
	}
	ASSERT(seq = (INDISEQ) pvalue(val));
	set_pvalue(val, PSET, seq = ancestor_indiseq(seq));
	push_list(keysets, seq);
	return val;
}
/*====================================================+
 * descendentset -- Create descendent set of an INDISEQ
 *   descendentset(SET) -> SET
 *   descendantset(SET) -> SET
 *===================================================*/
PVALUE __descendentset (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to descendentset must be a set.");
		return NULL;
	}
	ASSERT(seq = (INDISEQ) pvalue(val));
	set_pvalue(val, PSET, seq = descendent_indiseq(seq));
	push_list(keysets, seq);
	return val;
}
/*===================================================+
 * gengedcom -- Generate GEDCOM output from an INDISEQ
 *   gengedcom(SET) -> VOID
 *==================================================*/
PVALUE __gengedcom (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to gengedcom must be a set.");
		return NULL;
	}
	ASSERT(seq = (INDISEQ) pvalue(val));
	delete_pvalue(val);
	gen_gedcom(seq);
	return NULL;
}
