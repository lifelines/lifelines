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
/*==========================================================
 * write.c -- Handle changes to the database
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   2.3.6 - 01 Jan 94    3.0.0 - 01 Jan 94
 *   3.0.2 - 11 Dec 94
 *========================================================*/

#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "interp.h"

/*=====================================
 * createnode -- Create GEDCOM node
 *   createnode(STRING, STRING) -> NODE
 *===================================*/
WORD __createnode (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	STRING val, tag = (STRING) evaluate(ielist(node), stab, eflg);
	if (*eflg) return NULL;
	val = (STRING) evaluate(inext((INTERP)ielist(node)), stab, eflg);
	if (*eflg) return NULL;
	return (WORD) create_node(NULL, tag, val, NULL);
}
/*=======================================
 * addnode -- Add a node to a GEDCOM tree
 *   addnode(NODE, NODE, NODE) -> VOID
 *=====================================*/
WORD __addnode (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	NODE next, prnt, prev;
	NODE this = (NODE) evaluate(arg, stab, eflg);
	if (*eflg || this == NULL) return NULL;
	arg = inext(arg);
	prnt = (NODE) evaluate(arg, stab, eflg);
	if (*eflg || prnt == NULL) return NULL;
	prev = (NODE) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	nparent(this) = prnt;
	if (prev == NULL) {
		next = nchild(prnt);
		nchild(prnt) = this;
	} else {
		next = nsibling(prev);
		nsibling(prev) = this;
	}
	nsibling(this) = next;
	return NULL;
}
/*============================================
 * deletenode -- Remove node from GEDCOM tree
 *   deletenode(NODE) -> VOID
 *   NOTE: MEMORY LEAK MEMORY LEAK MEMORY LEAK
 *==========================================*/
WORD __deletenode (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE prnt, prev, curs, next;
	NODE this = (NODE) evaluate(ielist(node), stab, eflg);
	if (*eflg || this == NULL) return NULL;
	if ((prnt = nparent(this)) == NULL) return NULL;
	prev = NULL;
	curs = nchild(prnt);
	while (curs && curs != this) {
		prev = curs;
		curs = nsibling(curs);
	}
	if (curs == NULL) return NULL;
	next = nsibling(this);
	if (prev == NULL) 
		nchild(prnt) = next;
	else
		nsibling(prev) = next;
	return NULL;
}
