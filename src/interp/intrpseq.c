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

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interp.h"
#include "interpi.h"
#include "indiseq.h"
#include "gengedc.h"

/*********************************************
 * local function prototypes
 *********************************************/

static UNION pvseq_copy_value(UNION uval, INT valtype);
static void pvseq_delete_value(UNION uval, INT valtype);
static INT pvseq_compare_values(VPTR ptr1, VPTR ptr2, INT valtype);
static UNION pvseq_create_gen_value(INT gen, INT * valtype);

/*********************************************
 * local variables
 *********************************************/

static struct tag_indiseq_value_fnctable pvseq_fnctbl =
{
	&pvseq_copy_value
	, &pvseq_delete_value
	, &pvseq_create_gen_value
	, &pvseq_compare_values
};

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*======================================+
 * indiset -- Declare an INDISEQ variable
 *   indiset(VARB) -> VOID
 *=====================================*/
PVALUE
__indiset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PNODE var = (PNODE) iargs(node);
	if (!iistype(var, IIDENT)) {
		*eflg = TRUE;
		/* TODO: change to prog_var_error */
		prog_error(node, "the arg to indiset is not a variable.");
		return NULL;
	}
	*eflg = FALSE;
	seq = create_indiseq_pval();
	set_indiseq_value_funcs(seq, &pvseq_fnctbl);
	assign_iden(stab, iident(var), create_pvalue(PSET, (VPTR) seq));
	return NULL;
}
/*==================================+
 * addtoset -- Add person to INDISEQ
 *   addtoset(SET, INDI, ANY) -> VOID
 *=================================*/
PVALUE
__addtoset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	ASSERT(seq = pvalue_to_seq(val));
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
	append_indiseq_pval(seq, key, NULL, val, FALSE, TRUE);
	return NULL;
}
/*======================================+
 * lengthset -- Find length of an INDISEQ
 *   lengthset(SET) -> INT
 *=====================================*/
PVALUE
__lengthset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	INT len;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to lengthset must be a set.");
		return NULL;
	}
	seq = pvalue_to_seq(val);
	len = seq ? length_indiseq(seq) : 0;
	delete_pvalue(val);
	return create_pvalue_from_int(len);
}
/*====================================+
 * inset -- See if person is in INDISEQ
 *   inset(SET, INDI) -> BOOL
 *==========================================*/
PVALUE
__inset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi;
	STRING key;
	INDISEQ seq;
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1);
	PVALUE val = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg || !val || !pvalue(val)) {
		*eflg = TRUE;
		prog_error(node, "1st arg to inset must be a set.");
		return NULL;
	}
	seq = pvalue_to_seq(val);
	delete_pvalue(val);
	indi = eval_indi(arg2, stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, "2nd arg to inset must be a person.");
		return NULL;
	}
	if (!indi) return create_pvalue(PBOOL, (VPTR) FALSE);
	if (!(key = strsave(rmvat(nxref(indi))))) {
		*eflg = TRUE;
		prog_error(node, "major error in inset.");
		return NULL;
	}
	return create_pvalue(PBOOL, (VPTR) in_indiseq(seq, key));
}
/*===========================================+
 * deletefromset -- Remove person from INDISEQ
 *   deletefromset(SET, INDI, BOOL) -> VOID
 *==========================================*/
PVALUE
__deletefromset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	ASSERT(seq = pvalue_to_seq(val));
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
	all = pvalue_to_bool(val);
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
PVALUE
__namesort (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to namesort must be a set.");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val));
	delete_pvalue(val);
	namesort_indiseq(seq);
	return NULL;
}
/*==============================+
 * keysort -- Sort INDISEQ by key
 *   keysort(SET) -> VOID
 *=============================*/
PVALUE
__keysort (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to namesort must be a set.");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val));
	delete_pvalue(val);
	keysort_indiseq(seq);
	return NULL;
}
/*===================================
 * valuesort -- Sort INDISEQ by value
 *   valuesort(SET) -> VOID
 *=================================*/
PVALUE
__valuesort (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to valuesort must be a set.");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val));
	delete_pvalue(val);
	valuesort_indiseq(seq,eflg);
	if (*eflg) {
		prog_error(node, "missing or incorrect value for sort");
		return NULL;
	}
	return NULL;
}
/*=========================================+
 * uniqueset -- Eliminate dupes from INDISEQ
 *   uniqueset(SET) -> VOID
 *========================================*/
PVALUE
__uniqueset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to uniqueset must be a set.");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val));
	delete_pvalue(val);
	unique_indiseq(seq);
	return NULL;
}
/*=====================================+
 * union -- Create union of two INDISEQs
 *   union(SET, SET) -> SET
 *====================================*/
PVALUE
__union (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1);
	INDISEQ op2, op1;
	PVALUE val = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to union must be a set.");
		return NULL;
	}
	/* NULL indiseqs are possible, because of getindiset */
	op1 = pvalue_to_seq(val);
	delete_pvalue(val);
	val = eval_and_coerce(PSET, arg2, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to union must be a set.");
		return NULL;
	}
	op2 = pvalue_to_seq(val);
	op2 = union_indiseq(op1, op2);
	set_pvalue(val, PSET, op2);
	return val;
}
/*================================================+
 * intersect -- Create intersection of two INDISEQs
 *   intersect(SET, SET) -> SET
 *===============================================*/
PVALUE
__intersect (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1);
	INDISEQ op2, op1;
	PVALUE val = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to intersect must be a set.");
		return NULL;
	}
	/* NULL indiseqs are possible, because of getindiset */
	op1 = pvalue_to_seq(val);
	delete_pvalue(val);
	val = eval_and_coerce(PSET, arg2, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to intersect must be a set.");
		return NULL;
	}
	op2 = pvalue_to_seq(val);
	set_pvalue(val, PSET, op2 = intersect_indiseq(op1, op2));
	return val;
}
/*===============================================+
 * difference -- Create difference of two INDISEQs
 *   difference(SET, SET) -> SET
 *==============================================*/
PVALUE
__difference (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1);
	INDISEQ op2, op1;
	PVALUE val = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to difference must be a set.");
		return NULL;
	}
	/* NULL indiseqs are possible, because of getindiset */
	op1 = pvalue_to_seq(val);
	delete_pvalue(val);
	val = eval_and_coerce(PSET, arg2, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to difference must be a set.");
		return NULL;
	}
	op2 = pvalue_to_seq(val);
	set_pvalue(val, PSET, op2 = difference_indiseq(op1, op2));
	return val;
}
/*=========================================+
 * parentset -- Create parent set of INDISEQ
 *   parentset(SET) -> SET
 *========================================*/
PVALUE
__parentset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to parentset must be a set.");
		return NULL;
	}
	/* NULL indiseqs are possible, because of getindiset */
	seq = pvalue_to_seq(val);
	seq = parent_indiseq(seq);
	set_pvalue(val, PSET, seq);
	return val;
}
/*==========================================+
 * childset -- Create child set of an INDISEQ
 *   childset(SET) -> SET
 *=========================================*/
PVALUE
__childset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to childset must be a set.");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val));
	set_pvalue(val, PSET, seq = child_indiseq(seq));
	return val;
}
/*==============================================+
 * siblingset -- Create sibling set of an INDISEQ
 *   siblingset(SET) -> SET
 *=============================================*/
PVALUE
__siblingset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to siblingset must be a set.");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val));
	set_pvalue(val, PSET, seq = sibling_indiseq(seq, FALSE));
	return val;
}
/*============================================+
 * spouseset -- Create spouse set of an INDISEQ
 *   spouseset(SET) -> SET
 *===========================================*/
PVALUE
__spouseset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to spouseset must be a set.");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val));
	set_pvalue(val, PSET, seq = spouse_indiseq(seq));
	return val;
}
/*================================================+
 * ancestorset -- Create ancestor set of an INDISEQ
 *   ancestorset(SET) -> SET
 *===============================================*/
PVALUE
__ancestorset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to ancestorset must be a set.");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val));
	seq = ancestor_indiseq(seq);
	set_pvalue(val, PSET, seq);
	return val;
}
/*====================================================+
 * descendentset -- Create descendent set of an INDISEQ
 *   descendantset(SET) -> SET
 *===================================================*/
PVALUE
__descendentset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to descendentset must be a set.");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val));
	seq = descendent_indiseq(seq);
	set_pvalue(val, PSET, seq);
	return val;
}
/*===================================================+
 * gengedcom -- Generate GEDCOM output from an INDISEQ
 *   gengedcom(SET) -> VOID
 *==================================================*/
PVALUE
__gengedcom (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to gengedcom must be a set.");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val));
	delete_pvalue(val);
	gen_gedcom(seq, GENGEDCOM_ORIGINAL, eflg);
	return NULL;
}

/*===================================================+
 * gengedcomweak -- Generate GEDCOM output from an INDISEQ
 *   gengedcom(SET) -> VOID
 * Perry 2000/11/03
 *==================================================*/
PVALUE __gengedcomweak (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to gengedcomweak must be a set.");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val));
	delete_pvalue(val);
	gen_gedcom(seq, GENGEDCOM_WEAK_DUMP, eflg);
	return NULL;
}

/*===================================================+
 * gengedcomstrong -- Generate GEDCOM output from an INDISEQ
 *   gengedcom(SET) -> VOID
 * Perry 2000/11/03
 *==================================================*/
PVALUE __gengedcomstrong (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to gengedcomstrong must be a set.");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val));
	delete_pvalue(val);
	gen_gedcom(seq, GENGEDCOM_STRONG_DUMP, eflg);
	return NULL;
}
/*=====================================+
 * pvseq_copy_value -- Copy PVALUE in an INDISEQ
 * Created: 2001/03/25, Perry Rapp
 *====================================*/
static UNION
pvseq_copy_value (UNION uval, INT valtype)
{
	UNION retval;
	PVALUE val = (PVALUE)uval.w;
	ASSERT(valtype == ISVAL_PTR || valtype == ISVAL_NUL);
	ASSERT(is_pvalue(val) || !val);
	retval.w = copy_pvalue(val);
	return retval;
}
/*=====================================+
 * pvseq_delete_value -- Delete a PVALUE in an INDISEQ
 * Created: 2001/03/25, Perry Rapp
 *====================================*/
static void
pvseq_delete_value (UNION uval, INT valtype)
{
	PVALUE val = (PVALUE)uval.w;
	ASSERT(valtype == ISVAL_PTR || valtype == ISVAL_NUL);
	ASSERT(is_pvalue(val) || !val);
	delete_pvalue(val);
}
/*=====================================+
 * pvseq_create_gen_value -- Create a PVALUE 
 *  for a specific generation in an ancestor
 *  or descendant set in an INDISEQ
 * Assumes seq is NUL or PTR type
 * Created: 2001/03/25, Perry Rapp
 *====================================*/
static UNION
pvseq_create_gen_value (INT gen, INT * valtype)
{
	UNION uval;
	ASSERT(*valtype == ISVAL_PTR || *valtype == ISVAL_NUL);
	*valtype = ISVAL_PTR;
	uval.w = create_pvalue_from_int(gen);
	return uval;
}
/*=============================================
 * pvseq_compare_values -- Compare two pvalues
 * for sorting (collation) of an indiset
 *============================================*/
static INT
pvseq_compare_values (VPTR ptr1, VPTR ptr2, INT valtype)
{
	PVALUE val1=ptr1, val2=ptr2;
	ASSERT(valtype == ISVAL_PTR || valtype == ISVAL_NUL);
	if (valtype == ISVAL_NUL)
		return 0;
	/* if dissimilar types, we'll use the numerical order of the types */
	if (ptype(val1) != ptype(val2))
		return ptype(val1) - ptype(val2);

	/* ok, they are the same types, how do we compare them ? */
	switch(ptype(val1)) {
	case PSTRING:
		return cmpstrloc(pvalue_to_string(val1), pvalue_to_string(val2));
	case PINT:
		return pvalue_to_int(val1) - pvalue_to_int(val2);
	case PFLOAT:
		return pvalue_to_float(val1) - pvalue_to_float(val2);
	}
	return 0; /* TODO: what about other types ? */
}
