/* 
   Copyright (c) 2000-2001 Perry Rapp

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
/*================================================================
 * scan.c -- Search database with full scan (several types)
 * Copyright(c) 2000-2001 by Perry Rapp; all rights reserved
 *   Created: 2000/12
 *==============================================================*/

#include "sys_inc.h"
#include <stdarg.h>
#include "llstdlib.h"
#include "table.h"
#include "indiseq.h"
#include "feedback.h"
#include "liflines.h"
#include "fpattern.h"

#include "llinesi.h"

/*********************************************
 * external variables (no header)
 *********************************************/

extern STRING qSscanrs, qSscnnmf, qSscnfnm, qSscnrfn, qSscantt;

/*********************************************
 * local types
 *********************************************/

typedef struct
{
	INT scantype;
	char string[64];
} SCAN_PATTERN;

/*********************************************
 * local enums
 *********************************************/

static INT NAMESCAN_FULL=0;
static INT NAMESCAN_FRAG=1;
static INT REFNSCAN=2;

/*********************************************
 * local function prototypes
 *********************************************/

static BOOLEAN pattern_match(SCAN_PATTERN *patt, STRING name);
static BOOLEAN ns_callback(STRING key, STRING name, BOOLEAN newset, void *param);
static BOOLEAN rs_callback(STRING key, STRING refn, BOOLEAN newset, void *param);
static BOOLEAN set_pattern(SCAN_PATTERN * patt, STRING str, INT scantype);
static RECORD name_scan(INT scantype, STRING sts);

/*********************************************
 * local variables
 *********************************************/

static INDISEQ results_seq;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=============================================
 * pattern_match -- Compare a name to a pattern
 *===========================================*/
static BOOLEAN
pattern_match (SCAN_PATTERN *patt, STRING name)
{
	return (fpattern_matchn(patt->string, name));
}
/*===========================================
 * ns_callback -- callback for name traversal
 *=========================================*/
static BOOLEAN
ns_callback (STRING key, STRING name, BOOLEAN newset, void *param)
{
	LIST list;
	INT len, ind;
	STRING piece;
	SCAN_PATTERN * patt = (SCAN_PATTERN *)param;
	newset=newset; /* unused */
	if (patt->scantype == NAMESCAN_FULL) {
		if (pattern_match(patt, name)) {
			/* if we pass in name, append_indiseq won't check for dups */
			append_indiseq_null(results_seq, strsave(key), NULL, FALSE, TRUE);
		}
	} else {
		/* NAMESCAN_FRAG */
		list = create_list();
		name_to_list(name, list, &len, &ind);
		FORLIST(list, el)
			piece = (STRING)el;
			if (pattern_match(patt, piece)) {
				/* if we pass in name, append_indiseq won't check for dups */
				append_indiseq_null(results_seq, strsave(key), NULL, FALSE, TRUE);
				STOPLIST
				break;
			}
		ENDLIST
		free_name_list(list);
	}
	return TRUE;
}
/*===========================================
 * rs_callback -- callback for refn traversal
 *=========================================*/
static BOOLEAN
rs_callback (STRING key, STRING refn, BOOLEAN newset, void *param)
{
	SCAN_PATTERN * patt = (SCAN_PATTERN *)param;
	ASSERT(patt->scantype == REFNSCAN);
	newset=newset; /* unused */

	if (pattern_match(patt, refn)) {
		/* if we pass in name, append_indiseq won't check for dups */
		append_indiseq_null(results_seq, strsave(key), NULL, FALSE, TRUE);
	}
	return TRUE;
}
/*=================================================
 * set_pattern -- check pattern for validity
 *  for first cut, just refuse anything with spaces
 *===============================================*/
static BOOLEAN
set_pattern (SCAN_PATTERN * patt, STRING str, INT scantype)
{
	INT i;
	/* spaces don't make sense in a name fragment */
	if (scantype == NAMESCAN_FRAG) {
		for (i=0; str[i]; i++)
			if (str[i] == ' ')
				return FALSE;
	}

	if (!fpattern_isvalid(str))
		return FALSE;

	if (strlen(str) > sizeof(patt->string)-1)
		return FALSE;

	strcpy(patt->string, str);

	return TRUE;
}
/*==============================
 * name_scan -- traverse names looking for pattern matching
 *  scantype:  [IN]  which type of scan (full or partial)
 *  sts:       [IN]  status msg to display during scan
 *============================*/
static RECORD
name_scan (INT scantype, STRING sts)
{
	SCAN_PATTERN patt;
	RECORD indi = NULL;

	patt.scantype = scantype;
	while (1) {
		char request[MAXPATHLEN];
		BOOLEAN rtn;
		if (scantype == NAMESCAN_FRAG)
			rtn = ask_for_string(_(qSscnnmf), _(qSscantt), request, sizeof(request));
		else
			rtn = ask_for_string(_(qSscnfnm), _(qSscantt), request, sizeof(request));
		if (!rtn || !request[0])
			return NULL;
		if (set_pattern(&patt, request, scantype))
			break;
	}

	msg_status(sts);

	results_seq = create_indiseq_null();
	traverse_names(ns_callback, &patt);

	if (length_indiseq(results_seq)) {
		namesort_indiseq(results_seq);
		indi = choose_from_indiseq(results_seq, DOASK1, _(qSscanrs), _(qSscanrs));
	}
	remove_indiseq(results_seq);
	results_seq=NULL;
	return indi;
}
/*==============================================
 * name_fragment_scan -- traverse name fragments
 *  sts: [IN]  status to show during scan
 *  looking for pattern matching
 *============================================*/
RECORD
name_fragment_scan (STRING sts)
{
	return name_scan(NAMESCAN_FRAG, sts);
}
/*======================================
 * full_name_scan -- traverse full names
 *  sts: [IN]  status to show during scan
 *  looking for pattern matching
 *====================================*/
RECORD
full_name_scan (STRING sts)
{
	return name_scan(NAMESCAN_FULL, sts);
}
/*==============================
 * refn_scan -- traverse refns
 *  sts: [IN]  status to show during scan
 *  looking for pattern matching
 *============================*/
RECORD
refn_scan (STRING sts)
{
	SCAN_PATTERN patt;
	RECORD rec = NULL;
	INT scantype = REFNSCAN;

	patt.scantype = scantype;
	while (1) {
		char request[MAXPATHLEN];
		BOOLEAN rtn;
		rtn = ask_for_string(_(qSscnrfn), _(qSscantt), request, sizeof(request));
		if (!rtn || !request[0])
			return NULL;
		if (set_pattern(&patt, request, scantype))
			break;
	}

	msg_status(sts);

	results_seq = create_indiseq_null();
	traverse_refns(rs_callback, &patt);

	if (length_indiseq(results_seq)) {
		/* namesort uses canonkeysort for non-persons */
		namesort_indiseq(results_seq);
		rec = choose_from_indiseq(results_seq, DOASK1, _(qSscanrs), _(qSscanrs));
	}
	remove_indiseq(results_seq);
	results_seq=NULL;
	return rec;
}
