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
 * pre-SourceForge version information:
 *   3.0.2 - 11 Dec 94
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "feedback.h"
#include "liflines.h"
#include "llinesi.h"


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static NODE expand_tree(NODE);
static BOOLEAN advedit_expand_traverse(NODE, VPTR param);

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=================================================================
 * expand_tree -- Create copy of node tree with additional link info
 *===============================================================*/
static NODE
expand_tree (NODE root0)
{
	NODE copy, node, sub;
	STRING key;
	static NODE root;	/* root of record being edited */
	LIST subs;	/* list of contained records */
	NODE expd;	/* expanded main record - copy - our retval */

	root = root0;
	expd = copy_nodes(root, TRUE, TRUE);
	subs = create_list();
	traverse_nodes(expd, advedit_expand_traverse, subs);

   /* expand the list of records into the copied record */
	FORLIST(subs, el)
		node = (NODE) el;
#ifdef DEBUG
		llwprintf("in list: %s %s\n", ntag(node), nval(node));
#endif
		key = rmvat(nval(node));
		if ((sub = nztop(key_possible_to_record(key, *key)))) {
			copy = copy_node_subtree(sub);
			nxref(node)    = nxref(copy);
			ntag(node)     = ntag(copy);
			nchild(node)   = nchild(copy);
			nparent(node)  = nparent(copy);
/*MEMORY LEAK; MEMORY LEAK; MEMORY LEAK: node not removed (because its
  value and possibly xref [probably not] are still being referred to */
		}
	ENDLIST
	/* Shouldn't we free subs now ? Perry 2001/06/22 */
#ifdef DEBUG
	show_node(expd);
#endif
	return expd;
}

/*=================================================================
 * advanced_person_edit --
 *===============================================================*/
void
advanced_person_edit (NODE root0)
{
	FILE *fp;
	NODE expd;

#ifdef DEBUG
	llwprintf("advanced_person_edit: %s %s %s\n", nxref(root0), 
		  ntag(root0),nval(root0));
#endif
	expd = expand_tree(root0);
	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	write_nodes(0, fp, NULL, expd, TRUE, TRUE, TRUE);
	fclose(fp);
	do_edit();
}

/*=================================================================
 * advanced_family_edit --
 *===============================================================*/
void
advanced_family_edit (NODE root0)
{
	FILE *fp;
	NODE expd;

#ifdef DEBUG
	llwprintf("advanced_family_edit: %s %s %s\n", nxref(root0),
		  ntag(root0),nval(root0));
#endif
	expd = expand_tree(root0);
	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	write_nodes(0, fp, NULL, expd, TRUE, TRUE, TRUE);
	fclose(fp);
	do_edit();
}
/*=================================================================
 * advedit_expand_traverse -- Traverse routine called when expanding record
 *===============================================================*/
static BOOLEAN
advedit_expand_traverse (NODE node, VPTR param)
{
	LIST subs = (LIST)param;
	STRING key = value_to_xref(nval(node));
	if (!key) return TRUE;
	key = strsave(key);
#ifdef DEBUG
	llwprintf("expand_traverse: %s %s\n", ntag(node), nval(node));
#endif /* DEBUG */
	FORLIST(subs, el)
#ifdef DEBUG
	llwprintf("expand_traverse: %s %s\n", key, rmvat(nval((NODE) el)));
#endif /* DEBUG */
		if (eqstr(key, rmvat(nval((NODE) el)))) {
			STOPLIST
			stdfree(key);
			return TRUE;
		}
	ENDLIST
	enqueue_list(subs, node);
	stdfree(key);
	return TRUE;
}
