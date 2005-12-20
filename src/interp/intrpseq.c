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
#include "interpi.h"
#include "indiseq.h"
#include "gengedc.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING nonindx,nonvar1,nonset1,nonsetx,nonboox;

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
 * llrpt_indiset -- Declare an INDISEQ variable
 * usage: indiset(VARB) -> VOID
 *=====================================*/
PVALUE
llrpt_indiset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ newseq=0;
	PVALUE newval=0;
	PNODE arg1 = (PNODE) iargs(node);
	if (!iistype(arg1, IIDENT)) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg1, NULL, nonvar1, "indiset");
		return NULL;
	}
	*eflg = FALSE;
	newseq = create_indiseq_pval();
	set_indiseq_value_funcs(newseq, &pvseq_fnctbl);
	newval = create_pvalue_from_seq(newseq);
	assign_iden(stab, iident(arg1), newval);
	/* gave val1 to stab, so don't clear it */
	return NULL;
}
/*==================================+
 * llrpt_addtoset -- Add person to INDISEQ
 * usage: addtoset(SET, INDI, ANY) -> VOID
 *=================================*/
PVALUE
llrpt_addtoset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi=0;
	STRING key=0;
	INDISEQ seq=0;
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1),
	    arg3 = inext(arg2);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	PVALUE val2=0;
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonsetx, "addtoset", "1");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	indi = eval_indi(arg2, stab, eflg, NULL);
	if (*eflg) {
		prog_var_error(node, stab, arg2, NULL, nonindx, "addtoset","2");
		goto ats_exit;
	}
	if (!indi) goto ats_exit;
	*eflg = TRUE;
	if (!(key = strsave(rmvat(nxref(indi))))) {
		prog_error(node, "major error in addtoset.");
		goto ats_exit;
	}
	*eflg = FALSE;
	val2 = evaluate(arg3, stab, eflg);
	if (*eflg) {
		prog_error(node, "3rd arg to addtoset is in error.");
		goto ats_exit;
	}
	append_indiseq_pval(seq, key, NULL, val2, FALSE);
ats_exit:
	if (key) strfree(&key); /* append made its own copy */
	/* delay to last minute val1 cleanup lest it is a temp owning seq,
	    eg, addtoset(ancestorset(i),j) */
	if (val1) delete_pvalue(val1);
	return NULL;
}
/*======================================+
 * llrpt_lengthset -- Find length of an INDISEQ
 * usage: lengthset(SET) -> INT
 * Implementation Detail:
 * - implemented using llrpt_length(), which
 *   was changed to accept LISTs, SETs
 *   and TABLEs
 * - this function is DEPRECATED
 *=====================================*/
PVALUE
llrpt_lengthset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	return llrpt_length(node, stab, eflg);
}
/*====================================+
 * llrpt_inset -- See if person is in INDISEQ
 * usage: inset(SET, INDI) -> BOOL
 *==========================================*/
PVALUE
llrpt_inset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi;
	STRING key=0;
	INDISEQ seq;
	BOOLEAN rel;
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	PVALUE valr=0;
	if (*eflg ||!val1 || !(seq = pvalue_to_seq(val1))) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg1, val1, nonsetx, "inset", "1");
		goto inset_exit;
	}
	indi = eval_indi(arg2, stab, eflg, NULL);
	if (*eflg) {
		prog_var_error(node, stab, arg2, NULL, nonindx, "inset", "2");
		goto inset_exit;
	}
	if (!indi) {
		rel = FALSE;
        } else { 
		if (!(key = strsave(rmvat(nxref(indi))))) {
			*eflg = TRUE;
			prog_error(node, "major error in inset.");
			goto inset_exit;
		}
		rel = in_indiseq(seq, key);
	}
	valr = create_pvalue_from_bool(rel);
inset_exit:
	/* delay delete of val1 to last minute lest it is a temp owning seq,
	    eg, inset(ancestorset(i),j) */
	if (val1) delete_pvalue(val1);
	if (key) strfree(&key);
	return valr;
}
/*===========================================+
 * llrpt_deletefromset -- Remove person from INDISEQ
 * usage: deletefromset(SET, INDI, BOOL) -> VOID
 *==========================================*/
PVALUE
llrpt_deletefromset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi;
	STRING key=0;
	BOOLEAN all, rc;
	INDISEQ seq;
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1),
	    arg3 = inext(arg2);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	PVALUE val3=0;
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonsetx, "deletefromset", "1");
		goto dfs_exit;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	indi = eval_indi(arg2, stab, eflg, NULL);
	if (*eflg) {
		prog_var_error(node, stab, arg2, NULL, nonindx, "deletefromset", "2");
		goto dfs_exit;
	}
	if (!indi) goto dfs_exit;
	*eflg = TRUE;
	if (!(key = strsave(rmvat(nxref(indi))))) {
		prog_error(node, "major error in deletefromset.");
		goto dfs_exit;
	}
	*eflg = FALSE;
	val3 = eval_and_coerce(PBOOL, arg3, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg2, NULL, nonboox, "deletefromset", "3");
		goto dfs_exit;
	}
	all = pvalue_to_bool(val3);
	delete_pvalue(val3);
	do {
		rc = delete_indiseq(seq, key, NULL, 0);
	} while (rc && all);
dfs_exit:
	/* delay delete of val1 to last minute lest it is a temp owning seq,
	    eg, deletefromset(ancestorset(i),j) */
	if (val1) delete_pvalue(val1);
	if (key) strfree(&key);
	return NULL;
}
/*================================+
 * llrpt_namesort -- Sort INDISEQ by name
 * usage: namesort(SET) -> VOID
 *===============================*/
PVALUE
llrpt_namesort (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PNODE arg1 = (PNODE) iargs(node);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonset1, "namesort");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	namesort_indiseq(seq);
	/* delay to last minute lest it is a temp owning seq,
	eg, namesort(ancestorset(i) */
	delete_pvalue(val1);
	return NULL;
}
/*==============================+
 * llrpt_keysort -- Sort INDISEQ by key
 * usage: keysort(SET) -> VOID
 *=============================*/
PVALUE
llrpt_keysort (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PNODE arg1 = (PNODE) iargs(node);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonset1, "namesort");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	keysort_indiseq(seq);
	/* delay to last minute lest it is a temp owning seq,
	eg, keysort(ancestorset(i) */
	delete_pvalue(val1);
	return NULL;
}
/*===================================
 * llrpt_valuesort -- Sort INDISEQ by value
 * usage: valuesort(SET) -> VOID
 *=================================*/
PVALUE
llrpt_valuesort (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq;
	PNODE arg1 = (PNODE) iargs(node);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonset1, "valuesort");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	valuesort_indiseq(seq,eflg);
	if (*eflg) {
		prog_error(node, _("missing or incorrect value for sort"));
		return NULL;
	}
	/* delay to last minute lest it is a temp owning seq,
	eg, valuesort(ancestorset(i) */
	delete_pvalue(val1);
	return NULL;
}
/*=========================================+
 * llrpt_uniqueset -- Eliminate dupes from INDISEQ
 * usage: uniqueset(SET) -> VOID
 *========================================*/
PVALUE
llrpt_uniqueset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq=0;
	PNODE arg1 = (PNODE) iargs(node);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonset1, "uniqueset");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	unique_indiseq(seq);
	/* delay to last minute lest it is a temp owning seq,
	eg, uniqueset(ancestorset(i) */
	delete_pvalue(val1);
	return NULL;
}
/*=====================================+
 * llrpt_union -- Create union of two INDISEQs
 * usage: union(SET, SET) -> SET
 *====================================*/
PVALUE
llrpt_union (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1);
	INDISEQ op2=0, op1=0;
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	PVALUE val2=0;
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonsetx, "union", "1");
		return NULL;
	}
	/* NULL indiseqs are possible, because of getindiset */
	op1 = pvalue_to_seq(val1);
	val2 = eval_and_coerce(PSET, arg2, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg2, val2, nonsetx, "union", "2");
		return NULL;
	}
	op2 = pvalue_to_seq(val2);
	op2 = union_indiseq(op1, op2);
	set_pvalue_seq(val1, op2);
	/* delay to last minute lest it is a temp owning seq,
	eg, intersect(ancestorset(i),ancestorset(j)) */
	delete_pvalue(val2);
	return val1;
}
/*================================================+
 * llrpt_intersect -- Create intersection of two INDISEQs
 * usage: intersect(SET, SET) -> SET
 *===============================================*/
PVALUE
llrpt_intersect (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1);
	INDISEQ op2=0, op1=0;
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	PVALUE val2=0;
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonsetx, "intersect", "1");
		return NULL;
	}
	/* NULL indiseqs are possible, because of getindiset */
	op1 = pvalue_to_seq(val1);
	val2 = eval_and_coerce(PSET, arg2, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg2, val2, nonsetx, "intersect", "2");
		return NULL;
	}
	op2 = pvalue_to_seq(val2);
	/* do actual interset */
	op2 = intersect_indiseq(op1, op2);
	set_pvalue_seq(val1, op2);
	/* delay to last minute lest it is a temp owning seq,
	eg, intersect(ancestorset(i),ancestorset(j)) */
	delete_pvalue(val2);
	return val1;
}
/*===============================================+
 * llrpt_difference -- Create difference of two INDISEQs
 * usage: difference(SET, SET) -> SET
 *==============================================*/
PVALUE
llrpt_difference (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg1 = (PNODE) iargs(node), arg2 = inext(arg1);
	INDISEQ op2=0, op1=0;
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	PVALUE val2=0;
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonsetx, "difference", "1");
		return NULL;
	}
	/* NULL indiseqs are possible, because of getindiset */
	op1 = pvalue_to_seq(val1);
	val2 = eval_and_coerce(PSET, arg2, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg2, val2, nonsetx, "difference", "2");
		return NULL;
	}
	op2 = pvalue_to_seq(val2);
	/* do actual difference */
	op2 = difference_indiseq(op1, op2);
	set_pvalue_seq(val1, op2);
	/* delay to last minute lest it is a temp owning seq,
	eg, difference(ancestorset(i),ancestorset(j)) */
	delete_pvalue(val2);
	return val1;
}
/*=========================================+
 * llrpt_parentset -- Create parent set of INDISEQ
 * usage: parentset(SET) -> SET
 *========================================*/
PVALUE
llrpt_parentset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq=0;
	PNODE arg1 = (PNODE) iargs(node);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonset1, "parentset");
		return NULL;
	}
	/* NULL indiseqs are possible, because of getindiset */
	seq = pvalue_to_seq(val1);
	/* do actual construction of parent set */
	seq = parent_indiseq(seq);
	set_pvalue_seq(val1, seq);
	return val1;
}
/*==========================================+
 * llrpt_childset -- Create child set of an INDISEQ
 * usage: childset(SET) -> SET
 *=========================================*/
PVALUE
llrpt_childset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq=0;
	PNODE arg1 = (PNODE) iargs(node);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonset1, "childset");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	/* do actual construction of child set */
	seq = child_indiseq(seq);
	set_pvalue_seq(val1, seq);
	return val1;
}
/*==============================================+
 * llrpt_siblingset -- Create sibling set of an INDISEQ
 * usage: siblingset(SET) -> SET
 *=============================================*/
PVALUE
llrpt_siblingset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq=0;
	PNODE arg1 = (PNODE) iargs(node);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonset1, "siblingset");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	seq = sibling_indiseq(seq, FALSE);
	set_pvalue_seq(val1, seq);
	return val1;
}
/*============================================+
 * llrpt_spouseset -- Create spouse set of an INDISEQ
 * usage: spouseset(SET) -> SET
 *===========================================*/
PVALUE
llrpt_spouseset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq=0;
	PNODE arg1 = (PNODE) iargs(node);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonset1, "spouseset");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	seq = spouse_indiseq(seq);
	set_pvalue_seq(val1, seq);
	return val1;
}
/*================================================+
 * llrpt_ancestorset -- Create ancestor set of an INDISEQ
 * usage: ancestorset(SET) -> SET
 *===============================================*/
PVALUE
llrpt_ancestorset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq=0;
	PNODE arg1 = (PNODE) iargs(node);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonset1, "ancestorset");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	seq = ancestor_indiseq(seq);
	set_pvalue_seq(val1, seq);
	return val1;
}
/*====================================================+
 * llrpt_descendentset -- Create descendent set of an INDISEQ
 * usage: descendantset(SET) -> SET
 *===================================================*/
PVALUE
llrpt_descendentset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq=0;
	PNODE arg1 = (PNODE) iargs(node);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonset1, "descendentset");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	seq = descendent_indiseq(seq);
	set_pvalue_seq(val1, seq);
	return val1;
}
/*===================================================+
 * llrpt_gengedcom -- Generate GEDCOM output from an INDISEQ
 * usage: gengedcom(SET) -> VOID
 *==================================================*/
PVALUE
llrpt_gengedcom (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq=0;
	PNODE arg1 = (PNODE) iargs(node);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonset1, "gengedcom");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	gen_gedcom(seq, GENGEDCOM_ORIGINAL, eflg);
	/* delay to last minute lest it is a temp owning seq,
	eg, gengedcom(ancestorset(i)) */
	delete_pvalue(val1);
	return NULL;
}

/*===================================================+
 * llrpt_gengedcomweak -- Generate GEDCOM output from an INDISEQ
 * usage: gengedcom(SET) -> VOID
 * Perry 2000/11/03
 *==================================================*/
PVALUE
llrpt_gengedcomweak (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq=0;
	PNODE arg1 = (PNODE) iargs(node);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonset1, "gengedcomweak");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	gen_gedcom(seq, GENGEDCOM_WEAK_DUMP, eflg);
	/* delay to last minute lest it is a temp owning seq,
	eg, gengedcom(ancestorset(i)) */
	delete_pvalue(val1);
	return NULL;
}

/*===================================================+
 * llrpt_gengedcomstrong -- Generate GEDCOM output from an INDISEQ
 * usage: gengedcom(SET) -> VOID
 * Perry 2000/11/03
 *==================================================*/
PVALUE
llrpt_gengedcomstrong (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INDISEQ seq=0;
	PNODE arg1 = (PNODE) iargs(node);
	PVALUE val1 = eval_and_coerce(PSET, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, val1, nonset1, "gengedcomstrong");
		return NULL;
	}
	ASSERT(seq = pvalue_to_seq(val1));
	gen_gedcom(seq, GENGEDCOM_STRONG_DUMP, eflg);
	/* delay to last minute lest it is a temp owning seq,
	eg, gengedcom(ancestorset(i)) */
	delete_pvalue(val1);
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
	return pvalues_collate(val1, val2);
}
