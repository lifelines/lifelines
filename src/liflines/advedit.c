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
 * advedit.c -- Advanced edit features
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 11 Dec 94
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "translat.h"

static NODE root;	/* root of record being edited */
static LIST subs;	/* list of contained records */
static NODE expd;	/* expanded main record - copy */

expand_tree (root0)
NODE root0;
{
	NODE copy, node, sub;
	STRING key;
	INT expand_traverse();

	root = root0;
	expd = copy_nodes(root, TRUE, TRUE);
	subs = create_list();
	traverse_nodes(expd, expand_traverse);
/*FORLIST(subs, el) llwprintf("in list: %s %s\n", ntag(el), nval(el));
ENDLIST/*DEBUG*/

   /* expand the list of records into the copied record */
	FORLIST(subs, el)
		node = (NODE) el;
		key = rmvat(nval(node));
		if (sub = key_to_record(key, *key)) {
			copy = copy_nodes(sub, TRUE, FALSE);
			nxref(node)    = nxref(copy);
			ntag(node)     = ntag(copy);
			nchild(node)   = nchild(copy);
			nparent(node)  = nparent(copy);
/*MEMORY LEAK; MEMORY LEAK; MEMORY LEAK: node not removed (because its
  value and possibly xref [probably not] are still being referred to */
		}
	ENDLIST
/*show_node(expd);/*DEBUG*/
}

advanced_person_edit (root0)
NODE root0;
{
	FILE *fp;

/*llwprintf("advanced_person_edit: %s %s %s\n", nxref(root0), ntag(root0),
nval(root0));/*DEBUG*/
	expand_tree(root0);
	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	write_nodes(0, fp, NULL, expd, TRUE, TRUE, TRUE);
	fclose(fp);
	do_edit();
}

advanced_family_edit (root0)
NODE root0;
{
	FILE *fp;

/*llwprintf("advanced_family_edit: %s %s %s\n", nxref(root0), ntag(root0),
nval(root0));/*DEBUG*/
	expand_tree(root0);
	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	write_nodes(0, fp, NULL, expd, TRUE, TRUE, TRUE);
	fclose(fp);
	do_edit();
}
/*=================================================================
 * expand_traverse -- Traverse routine called when expanding record
 *===============================================================*/
BOOLEAN expand_traverse (node)
NODE node;
{
	NODE refd;
	STRING key = value_to_xref(nval(node));
	if (!key) return TRUE;
	key = strsave(key);
/*llwprintf("expand_traverse: %s %s\n", ntag(node), nval(node));/*DEBUG*/
	FORLIST(subs, el)
/*llwprintf("expand_traverse: %s %s\n", key, rmvat(nval((NODE) el)));/*DEBUG*/
		if (eqstr(key, rmvat(nval((NODE) el)))) {
			stdfree(key);
			return TRUE;
		}
	ENDLIST
	enqueue_list(subs, node);
	stdfree(key);
	return TRUE;
}
