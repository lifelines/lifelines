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
 * write.c -- Report language commands that change the database
 * Copyright(c) 1994-95 by T.T. Wetmore IV; all rights reserved
 *===========================================================*/

#include "llstdlib.h"
#include "gedcom.h"
#include "interpi.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING nonind1,nonstrx,nonnod1,nonnodx;

/*=====================================
 * llrpt_createnode -- Create GEDCOM node
 * usage: createnode(STRING, STRING) -> NODE
 * args: (tag, value)
 *===================================*/
PVALUE
llrpt_createnode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	NODE newnode=0;
	NODE prnt=NULL; /* parent node for new node */
	STRING xref=NULL; /* xref for new node */
	PVALUE val1=NULL, val2=NULL;
	STRING str1=NULL; /* 1st arg, which is tag for new node */
	STRING str2=NULL; /* 2nd arg, which is value for new node */
	val1 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, nonstrx, "createnode", "1");
		delete_pvalue(val1);
		return NULL;
	}
	/* 1st arg is tag for new node */
	str1 = pvalue_to_string(val1);
	val2 = eval_and_coerce(PSTRING, arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val2, nonstrx, "createnode", "2");
		delete_pvalue(val2);
		return NULL;
	}
	/* 2nd arg is value for new node */
	str2 = pvalue_to_string(val2);
	newnode = create_temp_node(xref, str1, str2, prnt);
	return create_pvalue_from_node(newnode);
}
/*=======================================
 * llrpt_addnode -- Add a node to a GEDCOM tree
 * usage: addnode(NODE, NODE, NODE) -> VOID
 * args: (node being added, parent, previous child)
 *=====================================*/
PVALUE
llrpt_addnode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	NODE newchild, next, prnt, prev;

	/* first argument, node (must be nonnull) */
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnodx, "addnode", "1");
		delete_pvalue(val);
		return NULL;
	}
	newchild = remove_node_and_delete_pvalue(&val);
	if (!newchild) {
		prog_var_error(node, stab, arg, val, nonnodx, "addnode", "1");
		return NULL;
	}

	/* second argument, parent (must be nonnull) */
	val = eval_and_coerce(PGNODE, arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnodx, "addnode", "2");
		return NULL;
	}
	prnt = remove_node_and_delete_pvalue(&val);
	if (!prnt) {
		prog_var_error(node, stab, arg, val, nonnodx, "addnode", "2");
		return NULL;
	}

	/* third argument, prior sibling (may be null) */
	val = eval_and_coerce(PGNODE, arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnodx, "addnode", "3");
		delete_pvalue(val);
		return NULL;
	}
	prev = remove_node_and_delete_pvalue(&val);

	if (prev) {
		/* Check that previous sibling actually is child of new parent */
		if (prnt != nparent(prev)) {
			prog_error(node, "2nd arg to addnode must be parent of 3rd arg");
			*eflg = 1;
			return NULL;
		}
	}

	/* reparent node, but ensure its locking is only relative to new parent */
	dolock_node_in_cache(newchild, FALSE);
	nparent(newchild) = prnt;
	newchild->n_cel = prnt->n_cel;
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
 * llrpt_detachnode -- Remove node from GEDCOM tree
 * usage: detachnode(NODE) -> VOID
 * (This is the historic deletenode)
 *==========================================*/
PVALUE
llrpt_detachnode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	if ((prnt = nparent(dead))) {
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
 * llrpt_writeindi -- Write person to database
 * usage: writeindi(INDI) -> BOOLEAN
 *====================================*/
PVALUE
llrpt_writeindi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi1;
	PNODE arg = iargs(node);
	NODE indi2 = eval_indi(arg, stab, eflg, NULL);
	STRING rawrec=0, msg;
	INT len, cnt;
	BOOLEAN rtn=FALSE;

	if (*eflg || !indi2) {
		prog_var_error(node, stab, arg, 0, nonind1, "writeindi");
		return NULL;
	}

	/* make a copy, so we can delete it */
	indi2 = copy_node_subtree(indi2);

	/* get existing record */
	rawrec = retrieve_raw_record(rmvat(nxref(indi2)), &len);
	if (!rawrec) {
		/*
		TODO: What do we do here ? Are they adding a new indi ?
		or did they get the xref wrong ?
		*/
		goto end_writeindi;
	}
	ASSERT(indi1 = string_to_node(rawrec));
 
	cnt = resolve_refn_links(indi2);
	/* validate for showstopper errors */
	if (!valid_indi_tree(indi2, &msg, indi1)) {
		/* TODO: What to do with msg ? */
		goto end_writeindi;
	}
	if (cnt > 0) {
		/* unresolvable refn links */
		/* TODO: optional argument to make this fatal ? */
	}

	if (equal_tree(indi1, indi2)) {
		/* optimization :) */
		rtn = TRUE;
		goto end_writeindi;
	}
	if (readonly) {
		/* TODO: database is read only error message */
		goto end_writeindi;
	}
	
	replace_indi(indi1, indi2);
	strfree(&rawrec);
	rtn = TRUE;

end_writeindi:
	return create_pvalue_from_bool(rtn);
}
/*=====================================
 * llrpt_writefam -- Write family to database
 * usage: writefam(FAM) -> BOOLEAN
 *===================================*/
PVALUE
llrpt_writefam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE fam1;
	NODE fam2 = eval_fam(iargs(node), stab, eflg, NULL);
	STRING rawrec=0, msg;
	INT len, cnt;
	BOOLEAN rtn=FALSE;
	if (*eflg) return NULL;

	/* make a copy, so we can delete it */
	fam2 = copy_node_subtree(fam2);

	/* get existing record */
	rawrec = retrieve_raw_record(rmvat(nxref(fam2)), &len);
	if (!rawrec) {
		/*
		TODO: What do we do here ? Are they adding a new fam ?
		or did they get the xref wrong ?
		*/
		goto end_writefam;
	}
	ASSERT(fam1 = string_to_node(rawrec));

	cnt = resolve_refn_links(fam2);
	/* validate for showstopper errors */
	if (!valid_fam_tree(fam2, &msg, fam1)) {
		/* TODO: What to do with msg ? */
		goto end_writefam;
	}
	if (cnt > 0) {
		/* unresolvable refn links */
		/* TODO: optional argument to make this fatal ? */
	}

	if (equal_tree(fam1, fam2)) {
		/* optimization :) */
		rtn = TRUE;
		goto end_writefam;
	}
	if (readonly) {
		/* TODO: database is read only error message */
		goto end_writefam;
	}
	
	replace_fam(fam1, fam2);
	strfree(&rawrec);
	rtn = TRUE;

end_writefam:
	return create_pvalue_from_bool(rtn);
}
