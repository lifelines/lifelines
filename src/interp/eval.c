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
/*==============================================================
 * eval.c -- Evaulate report program expressions
 * Copyright(c) 1991-94 by T. T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 17 Aug 93
 *   3.0.0 - 29 Jun 94    3.0.2 - 22 Dec 94
 *============================================================*/

#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "interp.h"

/*==============================
 * evaluate -- Generic evaluator
 *============================*/
WORD evaluate (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	if (iistype(node, IIDENT)) return evaluate_iden(node, stab, eflg);
	if (iistype(node, IBCALL)) return evaluate_func(node, stab, eflg);
	if (iistype(node, IFCALL)) return evaluate_ufunc(node, stab, eflg);
	*eflg = FALSE;
	if (iistype(node, IICONS)) return iicons(node);
	if (iistype(node, ILITERAL)) return iliteral(node);
	*eflg = TRUE;
	return NULL;
}
/*=====================================
 * evaluate_iden -- Evaluate identifier
 *===================================*/
WORD evaluate_iden (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	STRING iden = (STRING) iident(node);
	*eflg = FALSE;
	return valueof_iden(stab, iden);
}
/*========================================
 * valueof_iden - Find value of identifier
 *======================================*/
WORD valueof_iden (stab, iden)
TABLE stab;  STRING iden;
{
	if (in_table(stab, iden)) return (WORD) valueof(stab, iden);
	return (WORD) valueof(globtab, iden);
}
/*================================================
 * evaluate_cond - Evaluate conditional expression
 *==============================================*/
WORD evaluate_cond (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	WORD value;
	if (!inext(node)) return evaluate(node, stab, eflg);
	*eflg = TRUE;
	if (!iistype(node, IIDENT)) return NULL;
	value = evaluate(inext(node), stab, eflg);
	if (*eflg) return NULL;
	insert_table(stab, iident(node), value);
	return value;
}
/*==========================================
 * evaluate_func - Evaluate builtin function
 *========================================*/
WORD evaluate_func (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	WORD value;
	*eflg = TRUE;
	value = (*(FUNC)ifunc(node))(node, stab, eflg);
/*
	if (*eflg) wprintf("Error: line %d: function %s\n",
	    iline(node), iident(node));
*/
	return value;
}
/*================================================
 * evaluate_ufunc - Evaluate user defined function
 *==============================================*/
WORD evaluate_ufunc (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	STRING nam = (STRING) iname(node);
	INTERP func, arg, parm;
	TABLE newtab;
	WORD val;
	INTERPTYPE irc;

	*eflg = TRUE;
	if ((func = (INTERP) valueof(functab, nam)) == NULL) {
		wprintf("Error: line %d: `%s': undefined function\n",
		    iline(node), nam);
		return NULL;
	}
	newtab = create_table();
	arg = (INTERP) iargs(node);
	parm = (INTERP) iparams(func);
	while (arg && parm) {
		BOOLEAN eflg;
		WORD value = evaluate(arg, stab, &eflg);
		if (eflg) return INTERROR;
		insert_table(newtab, iident(parm), (WORD) value);
		arg = inext(arg);
		parm = inext(parm);
	}
	if (arg || parm) {
		wprintf("``%s'': mismatched args and params\n", iname(node));
		remove_table(newtab, DONTFREE);
		return INTERROR;
	}
	irc = interpret((INTERP) ibody(func), newtab, &val);
	remove_table(newtab, DONTFREE);
	switch (irc) {
	case INTRETURN:
	case INTOKAY:
		*eflg = FALSE;
		return val;
	case INTBREAK:
	case INTCONTINUE:
	case INTERROR:
		*eflg = TRUE;
		return NULL;
	}
}
/*=====================================
 * iistype -- Check type of interp node
 *===================================*/
BOOLEAN iistype (node, type)
INTERP node;
INT type;
{
	return itype(node) == type;
}
/*==============================================
 * num_params -- Return number of params in list
 *============================================*/
INT num_params (node)
INTERP node;
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
assign_iden (stab, id, value)
TABLE stab; STRING id; WORD value;
{
	TABLE tab = stab;
	if (!in_table(stab, id) && in_table(globtab, id)) tab = globtab;
	insert_table(tab, id, value);
	return;
}
