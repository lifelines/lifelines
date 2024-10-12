/* 
   Copyright (c) 1991-2007 Thomas T. Wetmore IV

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
 * builtin_list.c -- Report language (interpreter) list functions
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"

#include "interpi.h"

/*********************************************
 * local function prototypes
 *********************************************/

static VPTR create_list_value_pvalue(LIST list);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/


/*============================+
 * llrpt_clear -- Clear a list, set, indiseq
 * usage: clear(LIST) -> VOID
 *===========================*/
PVALUE
llrpt_clear (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
      LIST list;
      PNODE argvar = builtin_args(node);
      PVALUE val = eval_and_coerce(PLIST, argvar, stab, eflg);
      if (*eflg) {
              prog_var_error(node, stab, argvar, val, nonlst1, "1");
              delete_pvalue(val);
              return NULL;
      }
      list = pvalue_to_list(val);
      make_list_empty(list); /* leaking elements? 2005-02-05 Perry */
      return NULL;
}
/*============================+
 * llrpt_list -- Create list
 * usage: list(IDENT) -> VOID
 *===========================*/
PVALUE
llrpt_list (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{

	PVALUE newval=0;
	PNODE argvar = builtin_args(node);
	if (!iistype(argvar, IIDENT)) {
		prog_var_error(node, stab, argvar, NULL, nonvar1, "list");
		*eflg = TRUE;
		return NULL;
	}
	*eflg = FALSE;

	newval = create_new_pvalue_list();

	assign_iden(stab, iident_name(argvar), newval);
	return NULL;
}
/*=======================================+
 * llrpt_push -- Push element on front of list
 * usage: push(LIST, ANY) -> VOID
 *======================================*/
PVALUE
llrpt_push (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE argvar = builtin_args(node);
	LIST list=0;
	PVALUE el=0;
	PVALUE val = eval_and_coerce(PLIST, argvar, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, argvar, val, nonlst1, "1");
		delete_pvalue(val);
		return NULL;
	}
	el = evaluate(argvar=inext(argvar), stab, eflg);
	if (*eflg || !el) {
		/* TODO - use std errors */
		*eflg = TRUE;
		prog_error(node, "2nd arg to push is in error");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue_ptr(&val);
	push_list(list, el);
	return NULL;
}
/*======================================+
 * llrpt_inlist -- see if element is in list
 * usage: inlist(LIST, STRING) -> BOOL
 *=====================================*/
PVALUE
llrpt_inlist (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE argvar = builtin_args(node);
	LIST list=0;
	PVALUE el=0;
	BOOLEAN bFound;
	PVALUE val = eval_and_coerce(PLIST, argvar, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, argvar, val, nonlstx, "inlist", "1");
		delete_pvalue(val);
		return NULL;
	}
	el = evaluate(argvar=inext(argvar), stab, eflg);
	if (*eflg || !el) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, el, badargx, "inlist", "2");
		delete_pvalue(el);
		return NULL;
	}
	list = pvalue_to_list(val);
	bFound = in_list(list, el, eqv_pvalues) >= 0;
	set_pvalue_bool(val, bFound);
	delete_pvalue_ptr(&el);
	return val;
}
/*====================================+
 * llrpt_enqueue -- Enqueue element on list
 * usage: enqueue(LIST, ANY) -> VOID
 *===================================*/
PVALUE
llrpt_enqueue (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE argvar = builtin_args(node);
	LIST list=NULL;
	PVALUE el=NULL;
	PVALUE val = eval_and_coerce(PLIST, argvar, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, argvar, val, nonlstx, "enqueue", "1");
		delete_pvalue(val);
		return NULL;
	}
	el = evaluate(argvar=inext(argvar), stab, eflg);
	if (*eflg || !el) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, el, badargx, "enqueue", "2");
		delete_pvalue(el);
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue_ptr(&val);
	enqueue_list(list, el);
	return NULL;
}
/*========================================+
 * llrpt_requeue -- Add element to back of list
 * usage: requeue(LIST, ANY) -> VOID
 *=======================================*/
PVALUE
llrpt_requeue (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE argvar = builtin_args(node);
	LIST list=NULL;
	PVALUE el=NULL;
	PVALUE val = eval_and_coerce(PLIST, argvar, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_error(node, "1st arg to requeue is not a list");
		return NULL;
	}
	el = evaluate(argvar=inext(argvar), stab, eflg);
	if (*eflg || !el) {
		*eflg = TRUE;
		prog_error(node, "2nd arg to requeue is in error");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue_ptr(&val);
	back_list(list, el);
	return NULL;
}
/*=======================================+
 * llrpt_pop -- Pop element from front of list
 * usage: pop(LIST) -> ANY
 *======================================*/
PVALUE
llrpt_pop (PNODE node, SYMTAB stab, BOOLEAN  *eflg)
{
	LIST list=0;
	PNODE argvar = builtin_args(node);
	PVALUE val = eval_and_coerce(PLIST, argvar, stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to pop is not a list");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue_ptr(&val);
	if (is_empty_list(list)) return create_pvalue_any();
	return (PVALUE) pop_list(list);
}
/*=============================================+
 * llrpt_dequeue -- Remove element from back of list
 * usage: dequeue(LIST) -> ANY
 *============================================*/
PVALUE
llrpt_dequeue (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list=0;
	PNODE argvar = builtin_args(node);
	PVALUE val = eval_and_coerce(PLIST, argvar, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_error(node, nonlst1, "dequeue");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	val = (PVALUE) dequeue_list(list);
	if (!val) return create_pvalue_any();
	return val;
}
/*===================================
 * llrpt_empty is in builtin.c, as it is shared by table, list, and seq
 *=================================*/
/*===================================
 * create_list_value_pvalue -- 
 *  Create filler element
 *  Used when accessing list as an array
 *  Created: 2002/12/29 (Perry Rapp)
 *=================================*/
static VPTR
create_list_value_pvalue (HINT_PARAM_UNUSED LIST list)
{
	return create_pvalue_any();
}
/*==================================+
 * llrpt_getel -- Get nth value from list
 * usage: getel(LIST, INT) -> ANY
 *=================================*/
PVALUE
llrpt_getel (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list;
	INT ind;
	PNODE argvar = builtin_args(node);
	PVALUE val = eval_and_coerce(PLIST, argvar, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, val, nonlstx, "getel", "1");
		delete_pvalue(val);
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	val = eval_and_coerce(PINT, argvar=inext(argvar), stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, val, nonlstx, "getel", "2");
		delete_pvalue(val);
		return NULL;
	}
	ind = pvalue_to_int(val);
	delete_pvalue(val);
	if (!(val = (PVALUE) get_list_element(list, ind, &create_list_value_pvalue)))
		return create_pvalue_any();
	return copy_pvalue(val);
}
/*=======================================+
 * llrpt_setel -- Set nth value in list
 * usage: setel(LIST, INT, ANY) -> VOID
 *======================================*/
PVALUE
llrpt_setel (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list=0;
	INT ind=0;
	PNODE argvar = builtin_args(node);
	PVALUE old=0;
	PVALUE val = eval_and_coerce(PLIST, argvar, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, val, nonlstx, "setel", "1");
		delete_pvalue(val);
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	argvar = inext(argvar);
	val = eval_and_coerce(PINT, argvar, stab, eflg);
	if (*eflg) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, val, nonintx, "setel", "2");
		delete_pvalue(val);
		return NULL;
	}
	ind = pvalue_to_int(val);
	delete_pvalue(val);
	val = evaluate(argvar=inext(argvar), stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, val, badargx, "setel", "3");
		delete_pvalue(val);
		return NULL;
	}
	old = (PVALUE) get_list_element(list, ind, &create_list_value_pvalue);
	if(old) delete_pvalue(old);
	set_list_element(list, ind, val, &create_list_value_pvalue);
	return NULL;
}
/*===================================
 * llrpt_length is in builtin.c, as it is shared by table, list, and seq
 *=================================*/
/*===============================+
 * llrpt_dup -- Dup operation
 * usage: dup(LIST) -> LIST
 *==============================*/
PVALUE
llrpt_dup (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE argvar = builtin_args(node);
	LIST list=0;
	LIST newlist=0;
	PVALUE val=0, newval=0;
	INT i=0;

	val = evaluate(argvar, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, val, badargx, "dup", "1");
		return NULL;
	}
	/* traverse and copy */
	list = pvalue_to_list(val);
	delete_pvalue_ptr(&val);
	newlist = create_list3(delete_vptr_pvalue);
	for (i=0; i<length_list(list); i++) {
		newval = (PVALUE) get_list_element(list, i+1, NULL);
		enqueue_list(newlist, copy_pvalue(newval));
	}
	/* assign new list */
	newval = create_pvalue_from_list(newlist);
	release_list(newlist); /* release our ref to newlist */
	return newval;
}
