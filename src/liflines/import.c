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
/* modified 2000-04-25 J.F.Chandler */
/*==========================================================
 * import.c -- Import GEDCOM file to LifeLines database
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 29 Aug 94    3.0.2 - 10 Dec 94
 *   3.0.3 - 14 Jan 96
 *========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "sequence.h"
#include "gedcheck.h"
#include "screen.h"
#include "liflines.h"
#include "lloptions.h"

#include "llinesi.h"

/* external data set by check_stdkeys() */

static INT gd_reuse = 1;/* reuse original keys in GEDCOM file if possible */

extern INT gd_itot;	/* total number of individuals */
extern INT gd_ftot;	/* total number of families */
extern INT gd_stot;	/* total number of sources */
extern INT gd_etot;	/* total number of events */
extern INT gd_xtot;	/* total number of others */
extern INT gd_imax;	/* maximum individual key number */
extern INT gd_fmax;	/* maximum family key number */
extern INT gd_smax;	/* maximum source key number */
extern INT gd_emax;	/* maximum event key number */
extern INT gd_xmax;	/* maximum other key number */

extern STRING idgedf, gdcker, gdnadd, dboldk, dbnewk, dbodel,
  cfoldk, dbdelk, dbrdon;
extern TRANTABLE tran_tables[];

static BOOLEAN translate_values(NODE);
static void restore_record(NODE node, INT type, INT num);
static STRING translate_key(STRING);

/*=================================================
 * import_from_file -- Read GEDCOM file to database
 *===============================================*/
BOOLEAN
import_from_file (void)
{
	FILE *fp;
	NODE node, conv;
	TRANTABLE tt = tran_tables[MGDIN];
	STRING msg, fname;
	BOOLEAN emp;
	INT nindi = 0, nfam = 0, neven = 0;
	INT nsour = 0, nothr = 0, type, num = 0;
	INT totkeys = 0, totused = 0;
	char msgbuf[80];
	STRING srcdir=NULL;

/* Open and validate GEDCOM file */
	srcdir = lloptions.inputpath;
	fp = ask_for_input_file(LLREADTEXT, idgedf, &fname, srcdir, ".ged");
	if (!fp) return FALSE;
	llwprintf(gdcker, fname);
	if (!validate_gedcom(fp)) {
		fclose(fp);
		fp=NULL;
		wfield(9, 0, gdnadd);
		wpos(10, 0);
		return FALSE;
	}

	if((num_indis() > 0)
		|| (num_fams() > 0)
		|| (num_sours() > 0)
		|| (num_evens() > 0)
		|| (num_othrs() > 0)) gd_reuse = FALSE;
	else if((gd_reuse = check_stdkeys())) {
		totused = gd_itot + gd_ftot + gd_stot + gd_etot + gd_xtot;
		totkeys = gd_imax + gd_fmax + gd_smax + gd_emax + gd_xmax;
		if((totkeys-totused) > 0) {
		    sprintf(msgbuf, dbodel, totkeys-totused);
		}
		else strcpy(msgbuf, " ");
		gd_reuse = ask_yes_or_no_msg(msgbuf, cfoldk);
		touchwin(stdout_win);
		wrefresh(stdout_win);
	}

	/* start loading the file */
	rewind(fp);

	if(gd_reuse)
	  wfield(9,  0, dboldk);
	else
	  wfield(9,  0, dbnewk);

	/* test for read-only database here */

	if(readonly) {
		wfield(10, 0, dbrdon);
		wpos(11, 0);
		fclose(fp);
		fp=NULL;
		return FALSE;
	}

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
	if(gd_reuse && ((totkeys - totused) > 0)) {
	    wfield(15, 0, dbdelk);
	    addmissingkeys(INDI_REC);
	    addmissingkeys(FAM_REC);
	    addmissingkeys(EVEN_REC);
	    addmissingkeys(SOUR_REC);
	    addmissingkeys(OTHR_REC);
	}
	wpos(15, 0);
	mprintf_info("Added (%dP, %dF, %dS, %dE, %dX) records from file `%s'.",
	    nindi, nfam, nsour, neven, nothr, fname);

	fclose(fp);
	fp=NULL;

	return TRUE;
}
/*=============================================
 * restore_record -- Restore record to database
 *===========================================*/
static void
restore_record (NODE node,
                INT type,
                INT num)
{
	STRING old, new, str, key;
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
	resolve_links(node);
	ASSERT(str = node_to_string(node));
	key = rmvat(nxref(node));
	ASSERT(store_record(key, str, strlen(str)));
	index_by_refn(node, key);
	stdfree(str);
}
/*==============================================================
 * translate_key -- Translate key from external to internal form
 *============================================================*/
STRING
translate_key (STRING key)    /* key does not have surrounding @ chars */
{
	ELMNT elm;
	INT dex = xref_to_index(key);
	ASSERT(dex > -1);
	elm = index_data[dex];
	ASSERT(elm);
	switch (Type(elm)) {
	case INDI_REC:
		if (!New(elm)) New(elm) = strsave(newixref(key, gd_reuse));
		return New(elm);
	case FAM_REC:
		if (!New(elm)) New(elm) = strsave(newfxref(key, gd_reuse));
		return New(elm);
	case SOUR_REC:
		if (!New(elm)) New(elm) = strsave(newsxref(key, gd_reuse));
		return New(elm);
	case EVEN_REC:
		if (!New(elm)) New(elm) = strsave(newexref(key, gd_reuse));
		return New(elm);
	case OTHR_REC:
		if (!New(elm)) New(elm) = strsave(newxxref(key, gd_reuse));
		return New(elm);
	default:
		FATAL();
	}
	/* NOTREACHED */
	return NULL;
}
/*============================================================
 * translate_values -- Traverse function to translate pointers
 *==========================================================*/
static BOOLEAN
translate_values (NODE node)
{
	STRING new;
	if (!pointer_value(nval(node))) return TRUE;
	new = translate_key(rmvat(nval(node)));
	stdfree(nval(node));
	nval(node) = strsave(new);
	return TRUE;
}
