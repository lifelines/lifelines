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
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 21 Aug 93
 *   3.0.2 - 11 Dec 94
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

/*=======================================================
 * initset -- Initialize list that holds created INDISEQs
 *=====================================================*/
initset ()
{
	keysets = create_list();
}
/*=======================================
 * indiset -- Declare an INDISEQ variable
 *   indiset(VARB) -> VOID
 *=====================================*/
WORD __indiset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq;
	INTERP var = (INTERP) ielist(node);
	*eflg = TRUE;
	if (!iistype(var, IIDENT)) return NULL;
	*eflg = FALSE;
	seq = create_indiseq();
	assign_iden(stab, iident(var), seq);
	push_list(keysets, seq);
	return NULL;
}
/*===================================
 * addtoset -- Add person to INDISEQ
 *   addtoset(SET, INDI, ANY) -> VOID
 *=================================*/
WORD __addtoset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg1 = (INTERP) ielist(node), arg2 = inext(arg1),
	    arg3 = inext(arg2);
	INDISEQ seq = (INDISEQ) evaluate(arg1, stab, eflg);
	NODE indi;
	STRING key;
	WORD any;
	if (*eflg || !seq) return NULL;
	indi = (NODE) eval_indi(arg2, stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	*eflg = TRUE;
	key = strsave(rmvat(nxref(indi)));
	if (!seq || !key) return NULL;
	any = (WORD) evaluate(arg3, stab, eflg);
	if (*eflg) return NULL;
	append_indiseq(seq, key, NULL, any, FALSE, TRUE);
	return NULL;
}
/*=======================================
 * lengthset -- Find length of an INDISEQ
 *   lengthset(SET) -> INT
 *=====================================*/
WORD __lengthset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	if (*eflg || !seq) return NULL;
	return (WORD) length_indiseq(seq);
}
/*============================================
 * deletefromset -- Remove person from INDISEQ
 *   deletefromset(SET, INDI, BOOL) -> VOID
 *==========================================*/
WORD __deletefromset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg1 = (INTERP) ielist(node), arg2 = inext(arg1),
	    arg3 = inext(arg2);
	INDISEQ seq = (INDISEQ) evaluate(arg1, stab, eflg);
	NODE indi;
	STRING key;
	BOOLEAN all, rc;
	if (*eflg) return NULL;
	indi = (NODE) eval_indi(arg2, stab, eflg, NULL);
	if (*eflg) return NULL;
	*eflg = TRUE;
	key = rmvat(nxref(indi));
	if (!seq || !key) return NULL;
	all = (BOOLEAN) evaluate(arg3, stab, eflg);
	if (*eflg) return NULL;
	do {
		rc = delete_indiseq(seq, key, NULL, 0);
	} while (rc && all);
	return NULL;
}
/*=================================
 * namesort -- Sort INDISEQ by name
 *   namesort(SET) -> VOID
 *===============================*/
WORD __namesort (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	if (*eflg || !seq) return NULL;
	namesort_indiseq(seq);
	return NULL;
}
/*===============================
 * keysort -- Sort INDISEQ by key
 *   keysort(SET) -> VOID
 *=============================*/
WORD __keysort (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	if (*eflg || !seq) return NULL;
	keysort_indiseq(seq);
	return NULL;
}
/*===================================
 * valuesort -- Sort INDISEQ by value
 *   valuesort(SET) -> VOID
 *=================================*/
WORD __valuesort (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	if (*eflg || !seq) return NULL;
	valuesort_indiseq(seq);
	return NULL;
}
/*==========================================
 * uniqueset -- Eliminate dupes from INDISEQ
 *   uniqueset(SET) -> VOID
 *========================================*/
WORD __uniqueset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	if (*eflg || !seq) return NULL;
	return (WORD) unique_indiseq(seq);
}
/*======================================
 * union -- Create union of two INDISEQs
 *   union(SET, SET) -> SET
 *====================================*/
WORD __union (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg1 = (INTERP) ielist(node), arg2 = inext(arg1);
	INDISEQ op2, op1 = (INDISEQ) evaluate(arg1, stab, eflg);
	if (*eflg) return NULL;
	op2 = (INDISEQ) evaluate(arg2, stab, eflg);
	if (*eflg || !op1 || !op2) return NULL;
	op2 = union_indiseq(op1, op2);
	push_list(keysets, op2);
	return (WORD) op2;
}
/*=================================================
 * intersect -- Create intersection of two INDISEQs
 *   intersect(SET, SET) -> SET
 *===============================================*/
WORD __intersect (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg1 = (INTERP) ielist(node), arg2 = inext(arg1);
	INDISEQ op2, op1 = (INDISEQ) evaluate(arg1, stab, eflg);
	if (*eflg) return NULL;
	op2 = (INDISEQ) evaluate(arg2, stab, eflg);
	if (*eflg || !op1 || !op2) return NULL;
	op2 = intersect_indiseq(op1, op2);
	push_list(keysets, op2);
	return (WORD) op2;
}
/*================================================
 * difference -- Create difference of two INDISEQs
 *   difference(SET, SET) -> SET
 *==============================================*/
WORD __difference (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg1 = (INTERP) ielist(node), arg2 = inext(arg1);
	INDISEQ op2, op1 = (INDISEQ) evaluate(arg1, stab, eflg);
	if (*eflg) return NULL;
	op2 = (INDISEQ) evaluate(arg2, stab, eflg);
	if (*eflg || !op1 || !op2) return NULL;
	op2 = difference_indiseq(op1, op2);
	push_list(keysets, op2);
	return (WORD) op2;
}
/*==========================================
 * parentset -- Create parent set of INDISEQ
 *   parentset(SET) -> SET
 *========================================*/
WORD __parentset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	if (*eflg || !seq) return NULL;
	seq = parent_indiseq(seq);
	push_list(keysets, seq);
	return (WORD) seq;
}
/*===========================================
 * childset -- Create child set of an INDISEQ
 *   childset(SET) -> SET
 *=========================================*/
WORD __childset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	if (*eflg || !seq) return NULL;
	seq = child_indiseq(seq);
	push_list(keysets, seq);
	return (WORD) seq;
}
/*===============================================
 * siblingset -- Create sibling set of an INDISEQ
 *   siblingset(SET) -> SET
 *=============================================*/
WORD __siblingset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	if (*eflg || !seq) return NULL;
	seq = sibling_indiseq(seq, TRUE);
	push_list(keysets, seq);
	return (WORD) seq;
}
/*=============================================
 * spouseset -- Create spouse set of an INDISEQ
 *   spouseset(SET) -> SET
 *===========================================*/
WORD __spouseset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	if (*eflg || !seq) return NULL;
	seq = spouse_indiseq(seq);
	push_list(keysets, seq);
	return (WORD) seq;
}
/*=================================================
 * ancestorset -- Create ancestor set of an INDISEQ
 *   ancestorset(SET) -> SET
 *===============================================*/
WORD __ancestorset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	if (*eflg || !seq) return NULL;
	seq = ancestor_indiseq(seq);
	push_list(keysets, seq);
	return (WORD) seq;
}
/*=====================================================
 * descendentset -- Create descendent set of an INDISEQ
 *   descendentset(SET) -> SET
 *   descendantset(SET) -> SET
 *===================================================*/
WORD __descendentset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	if (*eflg || !seq) return NULL;
	seq = descendent_indiseq(seq);
	push_list(keysets, seq);
	return (WORD) seq;
}
/*====================================================
 * gengedcom -- Generate GEDCOM output from an INDISEQ
 *   gengedcom(SET) -> VOID
 *==================================================*/
WORD __gengedcom (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	if (*eflg || !seq) return NULL;
	gen_gedcom(seq);
	return NULL;
}
