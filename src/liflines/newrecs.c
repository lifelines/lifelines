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
#include "screen.h"

#include "llinesi.h"

extern BTREE BTR;

#define SS static STRING

extern STRING cfradd, cfeadd, cfxadd, rredit, eredit, xredit;
extern STRING cfrupt, cfeupt, cfxupt, gdrmod, gdemod, gdxmod;
extern STRING idredt, ideedt, idxedt;

SS rstr = (STRING) "0 SOUR\n1 REFN\n1 TITL Title\n1 AUTH Author";
SS estr = (STRING) "0 EVEN\n1 REFN\n1 DATE\n1 PLAC\n1 INDI\n  2 NAME\n  2 ROLE\n1 SOUR";
SS xstr = (STRING) "0 XXXX\n1 REFN";

static NODE ask_for_record(STRING, INT);
static void edit_record(NODE, STRING, INT, STRING, BOOLEAN (*func1)(), STRING, STRING, void (*func2)(), STRING);
static BOOLEAN add_record (STRING, STRING, BOOLEAN (*val)(), STRING, STRING (*getref)(), void (*todbase)(), void (*tocache)());

void edit_event(NODE);
void edit_other(NODE);
void edit_source(NODE);

BOOLEAN add_event(void);
BOOLEAN add_other(void);
BOOLEAN add_source(void);

/*================================================
 * add_source -- Add source to database by editing
 *==============================================*/
BOOLEAN
add_source (void)
{
	STRING str = (STRING) valueof(useropts, "SOURREC");
	if (!str) str = rstr;
	return add_record(str, rredit, valid_sour_tree, cfradd,
	    getsxref, sour_to_dbase, sour_to_cache);
}
/*==============================================
 * add_event -- Add event to database by editing
 *============================================*/
BOOLEAN
add_event (void)
{
	STRING str = (STRING) valueof(useropts, "EVENREC");
	if (!str) str = estr;
	return add_record(str, eredit, valid_even_tree, cfeadd,
	    getexref, even_to_dbase, even_to_cache);
}
/*====================================================
 * add_other -- Add user record to database by editing
 *==================================================*/
BOOLEAN
add_other (void)
{
	STRING str = (STRING) valueof(useropts, "OTHRREC");
	if (!str) str = xstr;
	return add_record(str, xredit, valid_othr_tree, cfxadd,
	    getxxref, othr_to_dbase, othr_to_cache);
}
/*================================================
 * add_record -- Add record to database by editing
 *==============================================*/
BOOLEAN
add_record (STRING recstr,                        /* default record */
            STRING redt,                          /* re-edit message */
            BOOLEAN (*val)(NODE, STRING *, NODE), /* tree validate predicate */
            STRING cfrm,                          /* confirm message */
            STRING (*getref)(void),               /* get next internal key */
            void (*todbase)(NODE),                /* write record to dbase */
            void (*tocache)(NODE))                /* write record to cache */
{
	FILE *fp;
	NODE node, refn;
	STRING msg, key;
	BOOLEAN emp;
	TRANTABLE tti = tran_tables[MEDIN];

/* Create template for user to edit */
	if (!(fp = fopen(editfile, LLWRITETEXT))) return FALSE;
	fprintf(fp, "%s\n", recstr);

/* Have user edit new record */
	fclose(fp);
	do_edit();
	while (TRUE) {
		node = file_to_node(editfile, tti, &msg, &emp);
		if (!node) {
			if (ask_yes_or_no_msg(msg, redt)) {
				do_edit();
				continue;
			} 
			break;
		}
		if (!(*val)(node, &msg, NULL)) {
			if (ask_yes_or_no_msg(msg, redt)) {
				do_edit();
				continue;
			}
			free_nodes(node);
			node = NULL;
			break;
		}
		break;
	}
	if (!node || !ask_yes_or_no(cfrm)) {
		if (node) free_nodes(node);
		return FALSE;
	}
	nxref(node) = strsave((STRING)(*getref)());
	key = rmvat(nxref(node));
	for (refn = nchild(node); refn; refn = nsibling(refn)) {
		if (eqstr("REFN", ntag(refn)) && nval(refn))
			add_refn(nval(refn), key);
	}
	(*todbase)(node);
	(*tocache)(node);
	return TRUE;
}
/*=======================================
 * edit_source -- Edit source in database
 *=====================================*/
void
edit_source (NODE node)
{
	edit_record(node, idredt, 'S', rredit, valid_sour_tree,
	    cfrupt, "SOUR", sour_to_dbase, gdrmod);
}
/*=====================================
 * edit_event -- Edit event in database
 *===================================*/
void
edit_event (NODE node)
{
	edit_record(node, ideedt, 'E', eredit, valid_even_tree,
	     cfeupt, "EVEN", even_to_dbase, gdemod);
}
/*===========================================
 * edit_other -- Edit user record in database
 *=========================================*/
void
edit_other (NODE node)
{
	edit_record(node, idxedt, 'X', xredit, valid_othr_tree,
	     cfxupt, NULL, othr_to_dbase, gdxmod);
}
/*=======================================
 * edit_record -- Edit record in database
 *=====================================*/
void
edit_record (NODE node1,           /* record to edit, poss NULL */
             STRING idedt,         /* user id prompt */
             INT letr,             /* record type: E, S or X */
             STRING redt,          /* re-edit message */
             BOOLEAN (*val)(NODE, STRING *, NODE), /* validate routine */
             STRING cfrm,          /* confirm message */
             STRING tag,           /* tag */
             void (*todbase)(NODE),/* write record to dbase */
             STRING gdmsg)         /* success message */
{
	TRANTABLE tti = tran_tables[MEDIN], tto = tran_tables[MINED];
	STRING msg, newr, oldr, key;
	FILE *fp;
	BOOLEAN emp;
	NODE refn, node2, temp;

/* Identify record if need be */
	if (!node1) node1 = ask_for_record(idedt, letr);
	if (!node1) {
		message("There is no record with that key or reference.");
		return;
	}
	refn = REFN(node1);
	oldr = refn ? nval(refn) : NULL;

/* Have user edit record */
	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	write_nodes(0, fp, tto, node1,  TRUE, TRUE, TRUE);
	fclose(fp);
	do_edit();
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
	if (newr && oldr && eqstr(newr, oldr))
                newr = oldr = NULL;
	key = rmvat(nxref(node1));
        if (oldr) remove_refn(oldr, key);
        if (newr) add_refn(newr, key);
	temp = nchild(node1);
	nchild(node1) = nchild(node2);
	nchild(node2) = temp;
	(*todbase)(node1);
	free_nodes(node2);
	mprintf(gdmsg);
}
/*==============================================
 * ask_for_record -- Ask user to identify record
 *============================================*/
static NODE
ask_for_record (STRING idstr,   /* question prompt */
                INT letr)       /* letter to possibly prepend to key */
{
	NODE node;
	STRING str = ask_for_string(idstr, "enter key or refn: ");
	if (!str || *str == 0) return NULL;
	node = key_to_record(str, letr);
	if (!node) node = refn_to_record(str, letr);
	return node;
}
