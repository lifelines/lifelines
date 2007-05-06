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
/*==============================================================
 * eval.c -- Evaulate report program expressions
 * Copyright(c) 1991-95 by T. T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 17 Aug 93
 *   3.0.0 - 29 Jun 94    3.0.2 - 22 Dec 94
 *   3.0.3 - 23 Nov 95
 *============================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interpi.h"
#include "feedback.h"
#include "lloptions.h"

static void trace_outv(STRING fmt, va_list args);


extern BOOLEAN explicitvars;

/*=============================+
 * evaluate -- Generic evaluator
 *============================*/
PVALUE
evaluate (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	if (prog_trace) {
		trace_out("%d: ", iline(node)+1);
		trace_pnode(node);
		trace_endl();
	}
	if (iistype(node, IIDENT))
		return evaluate_iden(node, stab, eflg);
	if (iistype(node, IBCALL))
		return evaluate_func(node, stab, eflg);
	if (iistype(node, IFCALL))
		return evaluate_ufunc(node, stab, eflg);
	*eflg = FALSE;
	if (iistype(node, IICONS))
		return copy_pvalue(ivalue(node));
	if (iistype(node, ISCONS))
		return copy_pvalue(ivalue(node));
	if (iistype(node, IFCONS))
		return copy_pvalue(ivalue(node));
	*eflg = TRUE;
	return NULL;
}
/*====================================+
 * trace_out -- output report trace info
 *===================================*/
void
trace_out (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	trace_outv(fmt, args);
	va_end(args);
}
/*====================================+
 * trace_outv -- output report trace info
 *===================================*/
static void
trace_outv (STRING fmt, va_list args)
{
	llvwprintf(fmt, args);
}
/*====================================+
 * trace_outl -- output report trace info & line end
 *===================================*/
void
trace_outl (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	trace_outv(fmt, args);
	va_end(args);
	trace_endl();
}
/*====================================+
 * trace_pvalue -- Send pvalue to trace output
 *===================================*/
void
trace_pvalue (PVALUE val)
{
	show_pvalue(val);
}
/*====================================+
 * trace_pnode -- Send pnode to trace output
 *===================================*/
void
trace_pnode (PNODE node)
{
	debug_show_one_pnode(node);
}
/*====================================+
 * trace_endl -- Finish trace line
 *===================================*/
void
trace_endl (void)
{
	llwprintf("\n");
}
/*====================================+
 * evaluate_iden -- Evaluate identifier
 * makes & returns copy
 *===================================*/
PVALUE
evaluate_iden (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING iden = (STRING) iident(node);
	if (prog_trace)
		trace_outl("evaluate_iden called: iden = %s", iden);
	*eflg = FALSE;
	return valueof_iden(node, stab, iden, eflg);
}
/*=======================================+
 * valueof_iden - Find value of identifier
 * makes & returns copy
 *======================================*/
PVALUE
valueof_iden (PNODE node, SYMTAB stab, STRING iden, BOOLEAN *eflg)
{
	BOOLEAN there;
	PVALUE val;

#ifdef DEBUG
	llwprintf("valueof_iden: iden, stab, globtab: %s, %d, %d\n",
	  	  iden, stab, globtab);
#endif

	*eflg = FALSE;
	val = symtab_valueofbool(stab, iden, &there);
	if (there) return copy_pvalue(val);
	val = symtab_valueofbool(globtab, iden, &there);
	if (there) return copy_pvalue(val);
	/* undeclared identifier */
	if (explicitvars) {
		*eflg = TRUE;
		prog_error(node, "Undeclared identifier: %s", iden);
	}
	return create_pvalue_any();
}
/*================================================+
 * evaluate_cond -- Evaluate conditional expression
 *===============================================*/
BOOLEAN
evaluate_cond (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE val;
	BOOLEAN rc;
	PNODE var = node, expr = inext(node);
	if (!expr) {
		expr = var;
		var = NULL;
	}
	if (var && !iistype(var, IIDENT)) {
		*eflg = TRUE;
		prog_error(node, "1st arg in conditional must be variable");
		return FALSE;
	}
	val = evaluate(expr, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_error(node, "error in conditional expression");
		return FALSE;
	}
#ifdef DEBUG
	llwprintf("interp_if: cond = ");
	show_pvalue(val);
	wprintf("\n");
#endif
	if (var) assign_iden(stab, iident(node), copy_pvalue(val));
	coerce_pvalue(PBOOL, val, eflg);
	rc = pvalue_to_bool(val);
	delete_pvalue(val);
	return rc;
}
/*==========================================+
 * evaluate_func -- Evaluate builtin function
 *=========================================*/
PVALUE
evaluate_func (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE val;

	*eflg = FALSE;
	if (prog_trace)
		trace_outl("evaluate_func called: %d: %s",
		    iline(node)+1, iname(node));
	val = (*(PFUNC)ifunc(node))(node, stab, eflg);
	return val;
}
/*================================================+
 * evaluate_ufunc -- Evaluate user defined function
 *  node:   [in] parsed node of function definition
 *  stab:   [in] function's symbol table
 *  eflg:   [out] error flag
 *===============================================*/
PVALUE
evaluate_ufunc (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING procname = (STRING) iname(node);
	PNODE func, arg, parm;
	SYMTAB newstab = NULL;
	PVALUE val=NULL;
	INTERPTYPE irc;
	INT count=0;

	*eflg = TRUE;
	/* find func in local or global table */
	func = get_proc_node(procname, irptinfo(node)->functab, gfunctab, &count);
	if (!func) {
		if (!count)
			prog_error(node, _("Undefined func: %s"), procname);
		else
			prog_error(node, _("Ambiguous call to func: %s"), procname);
		goto ufunc_leave;
	}

	newstab = create_symtab_proc(procname, stab);
	arg = (PNODE) iargs(node);
	parm = (PNODE) iargs(func);
	while (arg && parm) {
		BOOLEAN eflg=TRUE;
		PVALUE value = evaluate(arg, stab, &eflg);
		if (eflg) {
			if (getlloptint("FullReportCallStack", 0) > 0)
				prog_error(node, "In user function %s()", procname);
			return INTERROR;
		}
		insert_symtab(newstab, iident(parm), value);
		arg = inext(arg);
		parm = inext(parm);
	}
	if (arg || parm) {
		prog_error(node, "``%s'': mismatched args and params\n", procname);
		goto ufunc_leave;
	}
	irc = interpret((PNODE) ibody(func), newstab, &val);
	switch (irc) {
	case INTRETURN:
	case INTOKAY:
#ifdef DEBUG
	llwprintf("Successful ufunc call -- val returned was ");
	show_pvalue(val);
	llwprintf("\n");
#endif
		*eflg = FALSE;
		goto ufunc_leave;
	case INTBREAK:
	case INTCONTINUE:
	case INTERROR:
		break;
	}
	if (getlloptint("FullReportCallStack", 0) > 0)
		prog_error(node, "In user function %s()", procname);
	*eflg = TRUE;
	delete_pvalue(val);
	val=NULL;

ufunc_leave:
	if (newstab) {
		remove_symtab(newstab);
		newstab = NULL;
	}
	return val;
}
/*=====================================
 * iistype -- Check type of interp node
 *===================================*/
BOOLEAN
iistype (PNODE node,
         INT type)
{
	return itype(node) == type;
}
/*==============================================
 * num_params -- Return number of params in list
 *============================================*/
INT
num_params (PNODE node)
{
	INT np = 0;
	while (node) {
		np++;
		node = inext(node);
	}
	return np;
}
/*============================================
 * assign_iden -- Assign ident value in symtab
 *==========================================*/
void
assign_iden (SYMTAB stab, STRING id, PVALUE value)
{
	SYMTAB tab = stab;
	if (!in_symtab(stab, id) && in_symtab(globtab, id))
		tab = globtab;
	insert_symtab(tab, id, value);
	return;
}
/*=================================================
 * eval_and_coerce -- Generic evaluator and coercer
 *  type:  [IN]  desired pvalue type
 *  node:  [IN]  node to coerce
 *  stab:  [IN]  symbol table
 *  eflg:  [OUT] error flag
 *===============================================*/
PVALUE
eval_and_coerce (INT type, PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE val = eval_without_coerce(node, stab, eflg);
	if (*eflg) return NULL;
	coerce_pvalue(type, val, eflg);
	return val;
}
/*=================================================
 * eval_without_coerce -- Generic evaluator
 *  node:  [IN]  node to coerce
 *  stab:  [IN]  symbol table
 *  eflg:  [OUT] error flag
 * Created: 2001/12/24, Perry Rapp
 *===============================================*/
PVALUE
eval_without_coerce (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE val;
	if (*eflg) return NULL;
	val = evaluate(node, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		if (val) {
			delete_pvalue(val);
			val=NULL;
		}
		return NULL;
	}
	return val;
}
