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
 * newrecs.c -- Handle source, event and other record types
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 11 Sep 94    3.0.2 - 14 Apr 95
 *   3.0.3 - 17 Feb 96
 *===========================================================*/

#include "llstdlib.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "feedback.h"
#include "lloptions.h"

#include "llinesi.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern BTREE BTR;
extern STRING qScfradd, qScfeadd, qScfxadd, qSrredit, qSeredit, qSxredit;
extern STRING qScfrupt, qScfeupt, qScfxupt, qSgdrmod, qSgdemod, qSgdxmod;
extern STRING qSidredt, qSideedt, qSidxedt, qSduprfn, qSronlya, qSronlye;
extern STRING qSrreditopt, qSereditopt, qSxreditopt;
extern STRING qSbadreflink, qSbadreflinks;
extern STRING qSnofopn, qSidkyrfn;
extern STRING qSdefsour,qSdefeven,qSdefothr,qSnosuchrec;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static NODE edit_add_record(STRING recstr, STRING redt, STRING redtopt
	, char ntype, STRING cfrm);
static void edit_record(NODE node1, STRING idedt, INT letr, STRING redt
	, STRING redtopt , BOOLEAN (*val)(NODE, STRING *, NODE), STRING cfrm
	, void (*todbase)(NODE), STRING gdmsg);
static BOOLEAN ntagdiff(NODE node1, NODE node2);
static BOOLEAN nvaldiff(NODE node1, NODE node2);


/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*================================================
 * edit_add_source -- Add source to database by editing
 *==============================================*/
NODE
edit_add_source (void)
{
	STRING str;
	if (readonly) {
		message(_(qSronlya));
		return NULL;
	}
	str = getoptstr("SOURREC", _(qSdefsour));
	return edit_add_record(str, _(qSrredit), _(qSrreditopt), 'S', _(qScfradd));
}
/*==============================================
 * edit_add_event -- Add event to database by editing
 *============================================*/
NODE
edit_add_event (void)
{
	STRING str;
	if (readonly) {
		message(_(qSronlya));
		return NULL;
	}
	str = getoptstr("EVENREC", _(qSdefeven));
	return edit_add_record(str, _(qSeredit), _(qSereditopt), 'E', _(qScfeadd));
}
/*====================================================
 * edit_add_other -- Add user record to database by editing
 *==================================================*/
NODE
edit_add_other (void)
{
	STRING str;
	if (readonly) {
		message(_(qSronlya));
		return NULL;
	}
	str = getoptstr("OTHR", _(qSdefothr));
	return edit_add_record(str, _(qSxredit), _(qSxreditopt), 'X', _(qScfxadd));
}
/*================================================
 * edit_add_record -- Add record to database by editing
 *  recstr:  [IN] default record
 *  redt:    [IN] re-edit message
 *  ntype,   [IN] S, E, or X
 *  cfrm:    [IN] confirm message
 *==============================================*/
NODE
edit_add_record (STRING recstr, STRING redt, STRING redtopt, char ntype, STRING cfrm)
{
	FILE *fp;
	NODE node=0, refn;
	STRING msg, key;
	BOOLEAN emp;
	TRANTABLE tti = tran_tables[MEDIN];
	STRING (*getreffnc)(void) = NULL; /* get next internal key */
	void (*todbasefnc)(NODE) = NULL;  /* write record to dbase */
	void (*tocachefnc)(NODE) = NULL;  /* write record to cache */
	
	/* set up functions according to type */
	if (ntype == 'S') {
		getreffnc = getsxref;
		todbasefnc = sour_to_dbase;
		tocachefnc = sour_to_cache;
	} else if (ntype == 'E') {
		getreffnc = getexref;
		todbasefnc = even_to_dbase;
		tocachefnc = even_to_cache;
	} else { /* X */
		getreffnc = getxxref;
		todbasefnc = othr_to_dbase;
		tocachefnc = othr_to_cache;
	}

/* Create template for user to edit */
	if (!(fp = fopen(editfile, LLWRITETEXT))) {
		msg_error(_(qSnofopn), editfile);
		return FALSE;
	}
	fprintf(fp, "%s\n", recstr);

/* Have user edit new record */
	fclose(fp);
	do_edit();
	while (TRUE) {
		INT cnt;
		node = file_to_node(editfile, tti, &msg, &emp);
		if (!node) {
			if (ask_yes_or_no_msg(msg, redt)) { /* yes, edit again */
				do_edit();
				continue;
			} 
			break;
		}
		cnt = resolve_refn_links(node);
		/* check validation & allow user to reedit if invalid */
		/* this is a showstopper, so alternative is to abort */
		if (!valid_node_type(node, ntype, &msg, NULL)) {
			if (ask_yes_or_no_msg(msg, redt)) {
				do_edit();
				continue;
			}
			free_nodes(node);
			node = NULL; /* fail out */
			break;
		}
		/* Allow user to reedit if desired if any refn links unresolved */
		/* this is not a showstopper, so alternative is to continue */
		if (cnt > 0) {
			char msgb[120];
			snprintf(msgb, sizeof(msgb), _pl(qSbadreflink, qSbadreflinks, cnt), cnt);
			if (ask_yes_or_no_msg(msgb, redtopt)) {
				write_node_to_editfile(node);
				do_edit();
				continue;
			}
		}
		break;
	}
	if (!node || !ask_yes_or_no(cfrm)) {
		if (node) free_nodes(node);
		return NULL;
	}
	nxref(node) = strsave((STRING)(*getreffnc)());
	key = rmvat(nxref(node));
	for (refn = nchild(node); refn; refn = nsibling(refn)) {
		if (eqstr("REFN", ntag(refn)) && nval(refn))
			add_refn(nval(refn), key);
	}
	(*todbasefnc)(node);
	(*tocachefnc)(node);
	return node;
}
/*=======================================
 * edit_source -- Edit source in database
 *=====================================*/
void
edit_source (NODE node)
{
	edit_record(node, _(qSidredt), 'S', _(qSrredit), _(qSrreditopt)
		, valid_sour_tree, _(qScfrupt), sour_to_dbase, _(qSgdrmod));
}
/*=====================================
 * edit_event -- Edit event in database
 *===================================*/
void
edit_event (NODE node)
{
	edit_record(node, _(qSideedt), 'E', _(qSeredit), _(qSereditopt)
		, valid_even_tree, _(qScfeupt), even_to_dbase, _(qSgdemod));
}
/*===========================================
 * edit_other -- Edit other record in database (eg, NOTE)
 *=========================================*/
void
edit_other (NODE node)
{
	edit_record(node, _(qSidxedt), 'X', _(qSxredit), _(qSxreditopt)
		, valid_othr_tree, _(qScfxupt), othr_to_dbase, _(qSgdxmod));
}
/*========================================================
 * write_node_to_editfile - write all parts of gedcom node
 *  to a file for editing
 *======================================================*/
void
write_node_to_editfile (NODE node)
{
	FILE *fp;
	TRANTABLE tto = tran_tables[MINED];

	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	write_nodes(0, fp, tto, node,  TRUE, TRUE, TRUE);
	fclose(fp);
}
/*=======================================
 * edit_record -- Edit record in database
 *  root1:   [IN]  record to edit (may be NULL)
 *  idedt:   [IN]  user id prompt
 *  letr:    [IN]  record type (E, S, or X)
 *  redt:    [IN]  reedit prompt displayed if hard error after editing
 *  redtopt: [IN]  reedit prompt displayed if soft error (unresolved links)
 *  val:     [IN]  callback to validate routine
 *  cfrm:    [IN]  confirmation msg string
 *  tag:     [IN]  tag (SOUR, EVEN, or NULL)
 *  todbase: [IN]  callback to write record to dbase
 *  gdmsg:   [IN]  success message
 *=====================================*/
static void
edit_record (NODE root1, STRING idedt, INT letr, STRING redt, STRING redtopt
	, BOOLEAN (*val)(NODE, STRING *, NODE)
	, STRING cfrm, void (*todbase)(NODE), STRING gdmsg)
{
	TRANTABLE tti = tran_tables[MEDIN];
	STRING msg, key;
	BOOLEAN emp;
	NODE root0=0, root2=0;
	NODE refn1=0, refn2=0, refnn=0, refn1n=0;
	NODE body=0, node=0;

/* Identify record if need be */
	if (!root1) {
		root1 = nztop(ask_for_record(idedt, letr));
	}
	if (!root1) {
		message(_(qSnosuchrec));
		return;
	}

/* Have user edit record */
	if (getoptint("ExpandRefnsDuringEdit", 0) > 0)
		expand_refn_links(root1);
	write_node_to_editfile(root1);
	resolve_refn_links(root1);

	do_edit();
	if (readonly) {
		root2 = file_to_node(editfile, tti, &msg, &emp);
		if (!equal_tree(root1, root2))
			message(_(qSronlye));
		free_nodes(root2);
		return;
	}

	while (TRUE) {
		INT cnt;
		root2 = file_to_node(editfile, tti, &msg, &emp);
		if (!root2) {
			if (ask_yes_or_no_msg(msg, redt)) {
				do_edit();
				continue;
			}
			break;
		}
		cnt = resolve_refn_links(root2);
		/* check validation & allow user to reedit if invalid */
		/* this is a showstopper, so alternative is to abort */
		if (!(*val)(root2, &msg, root1)) {
			if (ask_yes_or_no_msg(msg, redt)) {
				do_edit();
				continue;
			}
			free_nodes(root2);
			root2 = NULL;
			break;
		}
		/* Allow user to reedit if desired if any refn links unresolved */
		/* this is not a showstopper, so alternative is to continue */
		if (cnt > 0) {
			char msgb[120];
			snprintf(msgb, sizeof(msgb), _pl(qSbadreflink, qSbadreflinks, cnt), cnt);
			if (ask_yes_or_no_msg(msgb, redtopt)) {
				write_node_to_editfile(root2);
				do_edit();
				continue;
			}
		}
		break;
	}

/* If error or no change or user backs out return */
	if (!root2) return;
	if (equal_tree(root1, root2) || !ask_yes_or_no(cfrm)) {
		free_nodes(root2);
		return;
	}

/* Prepare to change database */

	/* Move root1 data into root0 & delete it (saving refns) */
	split_othr(root1, &refn1, &body);
	root0 = copy_node(root1);
	join_othr(root0, NULL, body);
	free_nodes(root0);
	/* Move root2 data into root1, also copy out list of refns */
	split_othr(root2, &refn2, &body);
	refnn = copy_nodes(refn2, TRUE, TRUE);
	join_othr(root1, refn2, body);
	free_node(root2);

/* Change the database */

	(*todbase)(root1);
	key = rmvat(nxref(root1));
	/* remove deleted refns & add new ones */
	classify_nodes(&refn1, &refnn, &refn1n);
	for (node = refn1; node; node = nsibling(node))
		if (nval(node)) remove_refn(nval(node), key);
	for (node = refnn; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);
	free_nodes(refn1);
	free_nodes(refnn);
	free_nodes(refn1n);
	msg_info(gdmsg);

/* Change database */

#if OLDCODE
	/* BUG 2002.06.02 -- this assumes only one REFN */
	refn = REFN(root2);
	newr = refn ? nval(refn) : NULL;
	if (newr && oldr && eqstr(newr, oldr)) {
		newr = oldr = NULL;
	}
	key = rmvat(nxref(root1));
	if (oldr) remove_refn(oldr, key);
	if (newr) add_refn(newr, key);
	/* did value of top node change ? */
	if (nvaldiff(root1, root2)) {
		/* swap value of root2 into root1, which is the one we keep */
		str = nval(root1);
		nval(root1) = nval(root2);
		nval(root2) = str;
	}
	if (ntagdiff(root1, root2)) {
		/* swap tag of root2 into root1, which is the one we keep */
		str = ntag(root1);
		ntag(root1) = ntag(root2);
		ntag(root2) = str;
	}
	temp = nchild(root1);
	nchild(root1) = nchild(root2);
	nchild(root2) = temp;
	(*todbase)(root1);
	free_nodes(root2);
#endif
}
/*===============================================
 * nvaldiff -- Do nodes have different values ?
 *  handles NULLs in either
 * Created: 2001/04/08, Perry Rapp
 *=============================================*/
static BOOLEAN
nvaldiff (NODE node1, NODE node2)
{
	if (!nval(node1) && !nval(node2)) return FALSE;
	if (!nval(node1) || !nval(node2)) return TRUE;
	return strcmp(nval(node1), nval(node2));
}
/*===============================================
 * ntagdiff -- Do nodes have different tags ?
 *  handles NULLs in either
 * Created: 2001/12/30, Perry Rapp
 *=============================================*/
static BOOLEAN
ntagdiff (NODE node1, NODE node2)
{
	if (!ntag(node1) && !ntag(node2)) return FALSE;
	if (!ntag(node1) || !ntag(node2)) return TRUE;
	return !eqstr(ntag(node1), ntag(node2));
}
/*===============================================
 * ask_for_record -- Ask user to identify record
 *  lookup by key or by refn (& handle dup refns)
 *  idstr: [IN]  question prompt
 *  letr:  [IN]  letter to possibly prepend to key (ie, I/F/S/E/X)
 *=============================================*/
RECORD
ask_for_record (STRING idstr, INT letr)
{
	RECORD rec;
	STRING str = ask_for_string(idstr, _(qSidkyrfn));
	if (!str || *str == 0) return NULL;
	rec = key_possible_to_record(str, letr);
	if (!rec) {
		INDISEQ seq;
		seq = refn_to_indiseq(str, letr, KEYSORT);
		if (!seq) return NULL;
		rec = choose_from_indiseq(seq, NOASK1, _(qSduprfn), _(qSduprfn));
		remove_indiseq(seq);
	}
	return rec;
}
