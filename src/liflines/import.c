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
 * import.c -- Import GEDCOM file to LifeLines database
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 29 Aug 94    3.0.2 - 10 Dec 94
 *========================================================*/

#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "sequence.h"
#include "gedcheck.h"
#include "translat.h"

extern STRING idgedf, gdcker, gdnadd;
extern TRANTABLE tran_tables[];
static BOOLEAN translate_values();
static restore_record();
STRING translate_key();

/*=================================================
 * import_from_file -- Read GEDCOM file to database
 *===============================================*/
BOOLEAN import_from_file ()
{
	FILE *fp, *ask_for_file();
	NODE node, conv;
	TRANTABLE tt = tran_tables[MGDIN];
	STRING msg, fname;
	BOOLEAN emp;
	INT nindi = 0, nfam = 0, neven = 0;
	INT nsour = 0, nothr = 0, type, num;

/* Open and validate GEDCOM file */
	fp = ask_for_file("r", idgedf, &fname, NULL);
	if (!fp) return FALSE;
	wprintf(gdcker, fname);
	if (!validate_gedcom(fp)) {
		wfield(9, 0, gdnadd);
		wpos(10, 0);
		return FALSE;
	}
	rewind(fp);

	wfield(9,  0, "No errors; adding records to database.");
	wfield(10, 1, "     0 Persons");
	wfield(11, 1, "     0 Families");
	wfield(12, 1, "     0 Events");
	wfield(13, 1, "     0 Sources");
	wfield(14, 1, "     0 Others");

/* Add records to database */
	node = first_fp_to_node(fp, FALSE, tt, &msg, &emp);
	while (node) {
		if (!(conv = node_to_node(node, &type))) {
			free_nodes(node);
			node = next_fp_to_node(fp, FALSE, tt, &msg, &emp);
			continue;
		}
		switch (type) {
		case INDI_REC: num = ++nindi; break;
		case FAM_REC:  num = ++nfam;  break;
		case EVEN_REC: num = ++neven; break;
		case SOUR_REC: num = ++nsour; break;
		case OTHR_REC: num = ++nothr; break;
		default: FATAL();
		}
		restore_record(conv, type, num);
		free_nodes(node);
		node = next_fp_to_node(fp, FALSE, tt, &msg, &emp);
	}
	wpos(15, 0);
	mprintf("Added (%dP, %dF, %dS, %dE, %dX) records from file `%s'.",
	    nindi, nfam, nsour, neven, nothr, fname);
	return TRUE;
}
/*=============================================
 * restore_record -- Restore record to database
 *===========================================*/
static restore_record (node, type, num)
INT num;
NODE node;
INT type;
{
	STRING old, new, str, key;
	NODE refn;
	char scratch[10];
	if (!node) return;
	ASSERT(old = nxref(node));
	new = translate_key(rmvat(old));
	sprintf(scratch, "%6d", num);
	switch (type) {
	case INDI_REC: wfield(10, 1, scratch); break;
	case FAM_REC:  wfield(11, 1, scratch); break;
	case EVEN_REC: wfield(12, 1, scratch); break;
	case SOUR_REC: wfield(13, 1, scratch); break;
	case OTHR_REC: wfield(14, 1, scratch); break;
	default: FATAL();
	}
	if (nestr(old, new)) {
		stdfree(old);
		nxref(node) = strsave(new);
	}
	traverse_nodes(node, translate_values);
	if (type == INDI_REC) {
		add_linked_indi(node);
		return;
	}
	ASSERT(str = node_to_string(node));
	key = rmvat(nxref(node));
	ASSERT(store_record(key, str, strlen(str)));
	refn = REFN(node);
	if (refn && nval(refn)) add_refn(nval(refn), key);
	stdfree(str);
}
/*==============================================================
 * translate_key -- Translate key from external to internal form
 *============================================================*/
STRING translate_key (key)
STRING key;
{
	ELMNT elm;
	INT dex = xref_to_index(key);
	ASSERT(dex > -1);
	elm = index_data[dex];
	ASSERT(elm);
	switch (Type(elm)) {
	case INDI_REC:
		if (!New(elm)) New(elm) = strsave(getixref());
		return New(elm);
	case FAM_REC:
		if (!New(elm)) New(elm) = strsave(getfxref());
		return New(elm);
	case SOUR_REC:
		if (!New(elm)) New(elm) = strsave(getsxref());
		return New(elm);
	case EVEN_REC:
		if (!New(elm)) New(elm) = strsave(getexref());
		return New(elm);
	case OTHR_REC:
		if (!New(elm)) New(elm) = strsave(getxxref());
		return New(elm);
	default:
		FATAL();
	}
}
/*============================================================
 * translate_values -- Traverse function to translate pointers
 *==========================================================*/
static BOOLEAN translate_values (node)
NODE node;
{
	STRING new;
	if (!pointer_value(nval(node))) return TRUE;
	new = translate_key(rmvat(nval(node)));
	stdfree(nval(node));
	nval(node) = strsave(new);
	return TRUE;
}
