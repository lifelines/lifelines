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

#include "standard.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interp.h"
#include "liflines.h"
#include "screen.h"

extern BOOLEAN traceprogram;

/*=============================+
 * evaluate -- Generic evaluator
 *============================*/
PVALUE evaluate (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	if (prog_debug) {
		llwprintf("%d: ", iline(node));
		show_one_pnode(node);
		llwprintf("\n");
	}
	if (iistype(node, IIDENT)) return evaluate_iden(node, stab, eflg);
	if (iistype(node, IBCALL)) return evaluate_func(node, stab, eflg);
	if (iistype(node, IFCALL)) return evaluate_ufunc(node, stab, eflg);
	*eflg = FALSE;
	if (iistype(node, IICONS)) return copy_pvalue(ivalue(node));
	if (iistype(node, ISCONS)) return copy_pvalue(ivalue(node));
	if (iistype(node, IFCONS)) return copy_pvalue(ivalue(node));
	*eflg = TRUE;
	return NULL;
}
/*====================================+
 * evaluate_iden -- Evaluate identifier
 *===================================*/
PVALUE evaluate_iden (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	STRING iden = (STRING) iident(node);
#if 0
	if (prog_debug)
		llwprintf("evaluate_iden called: iden = %s\n", iden);
#endif
	*eflg = FALSE;
	return valueof_iden(stab, iden);
}
/*=======================================+
 * valueof_iden - Find value of identifier
 *======================================*/
PVALUE valueof_iden (stab, iden)
TABLE stab;  STRING iden;
{
	BOOLEAN there;
	PVALUE val;

#ifdef DEBUG
	llwprintf("valueof_iden: iden, stab, globtab: %s, %d, %d\n",
	  	  iden, stab, globtab);
#endif

	val = (PVALUE) valueofbool(stab, iden, &there);
	if (there) return copy_pvalue(val);
	val = (PVALUE) valueofbool(globtab, iden, &there);
	if (there) return copy_pvalue(val);
	return create_pvalue(PANY, NULL);
}
/*================================================+
 * evaluate_cond -- Evaluate conditional expression
 *===============================================*/
BOOLEAN evaluate_cond (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
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
	rc = (BOOLEAN) pvalue(val);
	delete_pvalue(val);
	return rc;
}
/*==========================================+
 * evaluate_func -- Evaluate builtin function
 *=========================================*/
PVALUE evaluate_func (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	PVALUE val;
	char trace[20];

	*eflg = FALSE;
#if 0
	if (prog_debug)
		llwprintf("evaluate_func called: %d: %s\n",
		    iline(node), iname(node));
#endif
	if (traceprogram) {
		sprintf(trace, "%d: %s\n", iline(node), (char*)iname(node));
		poutput(trace);
	}
	val = (*(PFUNC)ifunc(node))(node, stab, eflg);
	return val;
}
/*================================================+
 * evaluate_ufunc -- Evaluate user defined function
 *===============================================*/
PVALUE evaluate_ufunc (node, stab, eflg)
PNODE node; TABLE stab; BOOLEAN *eflg;
{
	STRING nam = (STRING) iname(node);
	PNODE func, arg, parm;
	TABLE newtab;
	PVALUE val;
	INTERPTYPE irc;

	*eflg = TRUE;
	if ((func = (PNODE) valueof(functab, nam)) == NULL) {
		unsigned char s[1024];
#ifdef HAVE_SNPRINTF
		snprintf(s, sizeof(s), "undefined function %s()", nam);
#else
		sprintf(s, "undefined function %s()", nam);
#endif
		prog_error(node, s);
		return NULL;
	}
	newtab = create_table();
	arg = (PNODE) iargs(node);
	parm = (PNODE) iargs(func);
	while (arg && parm) {
		BOOLEAN eflg;
		PVALUE value = evaluate(arg, stab, &eflg);
		if (eflg) return INTERROR;
		insert_table(newtab, iident(parm), (WORD) value);
		arg = inext(arg);
		parm = inext(parm);
	}
	if (arg || parm) {
		prog_error(node, "mismatched args and params");
		remove_table(newtab, DONTFREE);
		return INTERROR;
	}
	irc = interpret((PNODE) ibody(func), newtab, &val);
	remove_table(newtab, DONTFREE);
	switch (irc) {
	case INTRETURN:
	case INTOKAY:
#ifdef DEBUG
	llwprintf("Successful ufunc call -- val returned was ");
	show_pvalue(val);
	llwprintf("\n");
#endif
		*eflg = FALSE;
		return val;
	case INTBREAK:
	case INTCONTINUE:
	case INTERROR:
		break;
	}
	*eflg = TRUE;
	return NULL;
}
/*=====================================
 * iistype -- Check type of interp node
 *===================================*/
BOOLEAN iistype (node, type)
PNODE node;
INT type;
{
	return itype(node) == type;
}
/*==============================================
 * num_params -- Return number of params in list
 *============================================*/
INT num_params (node)
PNODE node;
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
void assign_iden (stab, id, value)
TABLE stab; STRING id; WORD value;
{
	TABLE tab = stab;
#ifdef HOGMEMORY
	if (!in_table(stab, id) && in_table(globtab, id)) tab = globtab;
#else
	BOOLEAN there;
	PVALUE val;
	val = (PVALUE) valueofbool(tab, id, &there);
	if (!there) {
	    val = (PVALUE) valueofbool(globtab, id, &there);
	    if(there) tab = globtab;
	}
	if (there && val) {
	    delete_pvalue(val);
	}
#endif
	insert_table(tab, id, value);
	return;
}
/*=================================================
 * eval_and_coerce -- Generic evaluator and coercer
 *===============================================*/
PVALUE eval_and_coerce (type, node, stab, eflg)
INT type; PNODE node; TABLE stab; BOOLEAN *eflg;
{
	PVALUE val;
	if (*eflg) return NULL;
	val = evaluate(node, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		return NULL;
	}
	coerce_pvalue(type, val, eflg);
	return val;
}
