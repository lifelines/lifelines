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
/*=============================================================
 * write.c -- Handle changes to the database
 * Copyright(c) 1994-95 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.6 - 01 Jan 94    3.0.0 - 01 Jan 94
 *   3.0.2 - 11 Dec 94    3.0.3 - 07 Aug 95
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interp.h"
#include "interpi.h"
#include "liflines.h"
#include "feedback.h"

/*
 ***** WARNING !!!!!! **********
 Only createnode, addnode, __detachnode have been fixed, because evaluate() returns PVALUES, not NODES 
 2003-02-02 (Perry)
 */

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING nonstrx,nonnod1,nonnodx;

/*=====================================
 * createnode -- Create GEDCOM node
 *   createnode(STRING, STRING) -> NODE
 *   args: (tag, value)
 *===================================*/
PVALUE
__createnode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	NODE newnode=0, prnt=NULL;
	PVALUE val1, val2;
	STRING str1, str2;
	val1 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, nonstrx, "createnode", "1");
		delete_pvalue(val1);
		return NULL;
	}
	str1 = pvalue_to_string(val1);
	val2 = eval_and_coerce(PSTRING, arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val2, nonstrx, "createnode", "2");
		delete_pvalue(val2);
		return NULL;
	}
	str2 = pvalue_to_string(val2);
	newnode = create_temp_node(NULL, str1, str2, prnt);
	return create_pvalue_from_node(newnode);
}
/*=======================================
 * addnode -- Add a node to a GEDCOM tree
 *   addnode(NODE, NODE, NODE) -> VOID
 *  args: (node being added, parent, previous child)
 *=====================================*/
PVALUE
__addnode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	NODE newchild, next, prnt, prev;
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnodx, "addnode", "1");
		delete_pvalue(val);
		return NULL;
	}
	newchild = pvalue_to_node(val);
	delete_pvalue_wrapper(val);
	val = eval_and_coerce(PGNODE, arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnodx, "addnode", "2");
		delete_pvalue(val);
		return NULL;
	}
	prnt = pvalue_to_node(val);
	delete_pvalue_wrapper(val);
	val = eval_and_coerce(PGNODE, arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnodx, "addnode", "3");
		delete_pvalue(val);
		return NULL;
	}
	prev = pvalue_to_node(val);
	delete_pvalue_wrapper(val);
	/* reparent node, but ensure its locking is only releative to new parent */
	dolock_node_in_cache(newchild, FALSE);
	nparent(newchild) = prnt;
	set_temp_node(newchild, is_temp_node(prnt));
	dolock_node_in_cache(newchild, TRUE);
	if (prev == NULL) {
		next = nchild(prnt);
		nchild(prnt) = newchild;
	} else {
		next = nsibling(prev);
		nsibling(prev) = newchild;
	}
	nsibling(newchild) = next;
	return NULL;
}
/*============================================
 * detachnode -- Remove node from GEDCOM tree
 *   detachnode(NODE) -> VOID
 * (This is the historic deletenode)
 *==========================================*/
PVALUE
__detachnode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	NODE dead, prnt;
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "detachnode");
		delete_pvalue(val);
		return NULL;
	}
	dead = pvalue_to_node(val);
	if (prnt = nparent(dead)) {
		NODE prev = NULL, next;
		NODE curs = nchild(prnt);
		while (curs && curs != dead) {
			prev = curs;
			curs = nsibling(curs);
		}
		ASSERT(curs); /* else broken tree: dead was not child of its parent */
		next = nsibling(dead);
		if (prev == NULL) 
			nchild(prnt) = next;
		else
			nsibling(prev) = next;
	}
	/* unparent node, but ensure its locking is only releative to new parent */
	dolock_node_in_cache(dead, FALSE);
	nparent(dead) = NULL;
	dolock_node_in_cache(dead, TRUE);
	nsibling(dead) = NULL;
	/* we don't actually delete the node, garbage collection must get it */
	return NULL;
}
/*======================================
 * writeindi -- Write person to database
 *   writeindi(INDI) -> VOID
 *====================================*/
PVALUE
__writeindi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi1;
	NODE indi2 = eval_indi(iargs(node), stab, eflg, NULL);
	STRING rawrec, msg;
	INT len;
	if (*eflg) return NULL;
	rawrec = retrieve_raw_record(rmvat(nxref(indi2)), &len);
	if (!rawrec) {
		/*
		TODO: What do we do here ? Are they adding a new indi ?
		or did they get the xref wrong ?
		*/
		return NULL;
	}
	ASSERT(indi1 = string_to_node(rawrec));
	/*
	TODO: 2003-02-04
	indi2 is the cache'd indi, and the xref is getting deleted here
	I don't know what is wrong -- Perry
	*/
	if (!replace_indi(indi1, indi2, &msg)) {
		*eflg = TRUE;
		prog_error(node, _("writeindi failed: %s"), msg ? msg : _("Unknown error"));
	}
	strfree(&rawrec);
	return NULL;
}
/*=====================================
 * writefam -- Write family to database
 *   writefam(FAM) -> VOID
 *===================================*/
PVALUE
__writefam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE fam1;
	NODE fam2 = eval_fam(iargs(node), stab, eflg, NULL);
	STRING rawrec, msg;
	INT len;
	if (*eflg) return NULL;
	rawrec = retrieve_raw_record(rmvat(nxref(fam2)), &len);
	if (!rawrec) {
		/*
		TODO: What do we do here ? Are they adding a new fam ?
		or did they get the xref wrong ?
		*/
		return NULL;
	}
	ASSERT(fam1 = string_to_node(rawrec));
	if (!replace_fam(fam1, fam2, &msg)) {
		*eflg = TRUE;
		prog_error(node, _("writefam failed: %s"), msg ? msg : _("Unknown error"));
	}
	strfree(&rawrec);
	return NULL;
}
