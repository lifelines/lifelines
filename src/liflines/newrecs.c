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
extern STRING cfradd, cfeadd, cfxadd, rredit, eredit, xredit;
extern STRING cfrupt, cfeupt, cfxupt, gdrmod, gdemod, gdxmod;
extern STRING idredt, ideedt, idxedt, duprfn, ronlya, ronlye;
extern STRING nofopn, idkyrfn;
extern STRING defsour,defeven,defothr;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static NODE add_record(STRING recstr, STRING redt, char ntype, STRING cfrm);
static void edit_record(NODE node1, STRING idedt, INT letr, STRING redt,
                         BOOLEAN (*val)(NODE, STRING *, NODE), STRING cfrm,
                         STRING tag, void (*todbase)(NODE), STRING gdmsg);
static BOOLEAN ntagdiff(NODE node1, NODE node2);
static BOOLEAN nvaldiff(NODE node1, NODE node2);


/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*================================================
 * add_source -- Add source to database by editing
 *==============================================*/
NODE
add_source (void)
{
	STRING str;
	if (readonly) {
		message(_(ronlya));
		return NULL;
	}
	str = getoptstr("SOURREC", _(defsour));
	return add_record(str, rredit, 'S', _(cfradd));
}
/*==============================================
 * add_event -- Add event to database by editing
 *============================================*/
NODE
add_event (void)
{
	STRING str;
	if (readonly) {
		message(_(ronlya));
		return NULL;
	}
	str = getoptstr("EVENREC", _(defeven));
	return add_record(str, eredit, 'E', _(cfeadd));
}
/*====================================================
 * add_other -- Add user record to database by editing
 *==================================================*/
NODE
add_other (void)
{
	STRING str;
	if (readonly) {
		message(_(ronlya));
		return NULL;
	}
	str = getoptstr("OTHR", _(defothr));
	return add_record(str, xredit, 'X', _(cfxadd));
}
/*================================================
 * add_record -- Add record to database by editing
 *  recstr:  [IN] default record
 *  redt:    [IN] re-edit message
 *  ntype,   [IN] S, E, or X
 *  cfrm:    [IN] confirm message (localized)
 *==============================================*/
NODE
add_record (STRING recstr, STRING redt, char ntype, STRING cfrm)
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
		msg_error(nofopn, editfile);
		return FALSE;
	}
	fprintf(fp, "%s\n", recstr);

/* Have user edit new record */
	fclose(fp);
	do_edit();
	while (TRUE) {
		node = file_to_node(editfile, tti, &msg, &emp);
		if (!node) {
			if (ask_yes_or_no_msg(msg, redt)) { /* yes, edit again */
				do_edit();
				continue;
			} 
			break;
		}
		if (!valid_node_type(node, ntype, &msg, NULL)) { /* invalid */
			if (ask_yes_or_no_msg(msg, redt)) { /* yes, edit again */
				do_edit();
				continue;
			} /* otherwise leave rtn==-1 for cancel */
			free_nodes(node);
			node = NULL;
			break;
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
	edit_record(node, idredt, 'S', rredit, valid_sour_tree,
	    _(cfrupt), "SOUR", sour_to_dbase, gdrmod);
}
/*=====================================
 * edit_event -- Edit event in database
 *===================================*/
void
edit_event (NODE node)
{
	edit_record(node, ideedt, 'E', eredit, valid_even_tree,
	     _(cfeupt), "EVEN", even_to_dbase, gdemod);
}
/*===========================================
 * edit_other -- Edit user record in database
 *=========================================*/
void
edit_other (NODE node)
{
	edit_record(node, idxedt, 'X', xredit, valid_othr_tree,
	     _(cfxupt), NULL, othr_to_dbase, gdxmod);
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
 *  node1:   [IN]  record to edit (may be NULL)
 *  idedt:   [IN]  user id prompt
 *  letr:    [IN]  record type (E, S, or X)
 *  val:     [IN]  callback to validate routine
 *  cfrm:    [IN]  confirmation msg string (localized)
 *  tag:     [IN]  tag (SOUR, EVEN, or NULL)
 *  todbase: [IN]  callback to write record to dbase
 *  gdmsg:   [IN]  success message
 *=====================================*/
void
edit_record (NODE node1, STRING idedt, INT letr, STRING redt
	, BOOLEAN (*val)(NODE, STRING *, NODE)
	, STRING cfrm, STRING tag, void (*todbase)(NODE), STRING gdmsg)
{
	TRANTABLE tti = tran_tables[MEDIN];
	STRING msg, newr, oldr, key;
	BOOLEAN emp;
	NODE refn, node2=0, temp;
	STRING str;
	tag=tag; /* unused */ /* TODO: what is this ? 2002/01/04 Perry */

/* Identify record if need be */
	if (!node1) {
		node1 = nztop(ask_for_record(idedt, letr));
	}
	if (!node1) {
		message("There is no record with that key or reference.");
		return;
	}
	refn = REFN(node1);
	oldr = refn ? nval(refn) : NULL;

/* Have user edit record */
	write_node_to_editfile(node1);
	do_edit();
	if (readonly) {
		node2 = file_to_node(editfile, tti, &msg, &emp);
		if (!equal_tree(node1, node2))
			message(_(ronlye));
		free_nodes(node2);
		return;
	}

	while (TRUE) {
		node2 = file_to_node(editfile, tti, &msg, &emp);
		if (!node2) {
			if (ask_yes_or_no_msg(msg, redt)) {
				do_edit();
				continue;
			}
			break;
		}
		if (!(*val)(node2, &msg, node1)) {
			if (ask_yes_or_no_msg(msg, redt)) {
				do_edit();
				continue;
			}
			free_nodes(node2);
			node2 = NULL;
			break;
		}
		break;
	}

/* If error or no change or user backs out return */
	if (!node2) return;
	if (equal_tree(node1, node2) || !ask_yes_or_no(cfrm)) {
		free_nodes(node2);
		return;
	}

/* Change database */

	refn = REFN(node2);
	newr = refn ? nval(refn) : NULL;
	if (newr && oldr && eqstr(newr, oldr)) {
		newr = oldr = NULL;
	}
	key = rmvat(nxref(node1));
	if (oldr) remove_refn(oldr, key);
	if (newr) add_refn(newr, key);
	/* did value of top node change ? */
	if (nvaldiff(node1, node2)) {
		/* swap value of node2 into node1, which is the one we keep */
		str = nval(node1);
		nval(node1) = nval(node2);
		nval(node2) = str;
	}
	if (ntagdiff(node1, node2)) {
		/* swap tag of node2 into node1, which is the one we keep */
		str = ntag(node1);
		ntag(node1) = ntag(node2);
		ntag(node2) = str;
	}
	temp = nchild(node1);
	nchild(node1) = nchild(node2);
	nchild(node2) = temp;
	(*todbase)(node1);
	free_nodes(node2);
	msg_info(gdmsg);
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
 *  idstr: [IN]  question prompt (will translate)
 *  letr:  [IN]  letter to possibly prepend to key (ie, I/F/S/E/X)
 *=============================================*/
RECORD
ask_for_record (STRING idstr, INT letr)
{
	RECORD rec;
	STRING str = ask_for_string(idstr, idkyrfn);
	if (!str || *str == 0) return NULL;
	rec = key_possible_to_record(str, letr);
	if (!rec) {
		INDISEQ seq;
		seq = refn_to_indiseq(str, letr, KEYSORT);
		if (!seq) return NULL;
		rec = choose_from_indiseq(seq, NOASK1, duprfn, duprfn);
		remove_indiseq(seq);
	}
	return rec;
}
