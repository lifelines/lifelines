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
#include "interp.h"
#include "screen.h"
#include "liflines.h"
#include "fpattern.h"

#include "llinesi.h"

/*********************************************
 * external variables (no header)
 *********************************************/

extern STRING scanrs, scnnmf, scnfnm, scnrfn, scantt;

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
static NOD0 name_scan(INT scantype);

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
	if (patt->scantype == NAMESCAN_FULL) {
		if (pattern_match(patt, name)) {
			/* if we pass in name, append_indiseq won't check for dups */
			append_indiseq_null(results_seq, key, NULL, FALSE, FALSE);
		}
	} else {
		/* NAMESCAN_FRAG */
		list = create_list();
		name_to_list(name, list, &len, &ind);
		FORLIST(list, el)
			piece = (STRING)el;
			if (pattern_match(patt, piece)) {
				/* if we pass in name, append_indiseq won't check for dups */
				append_indiseq_null(results_seq, key, NULL, FALSE, FALSE);
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

	if (pattern_match(patt, refn)) {
		/* if we pass in name, append_indiseq won't check for dups */
		append_indiseq_null(results_seq, key, NULL, FALSE, FALSE);
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
 * name_scan -- traverse names
 *  looking for pattern matching
 *============================*/
static NOD0
name_scan (INT scantype)
{
	SCAN_PATTERN patt;
	NOD0 indi = NULL;
	STRING str;

	patt.scantype = scantype;
	while (1) {
		if (scantype == NAMESCAN_FRAG)
			str = ask_for_string(scnnmf, scantt);
		else
			str = ask_for_string(scnfnm, scantt);
		if (!str || !str[0])
			return NULL;
		if (set_pattern(&patt, str, scantype))
			break;
	}


	results_seq = create_indiseq_null();
	traverse_names(ns_callback, &patt);

	if (length_indiseq(results_seq)) {
		indi = choose_from_indiseq(results_seq, DOASK1, scanrs, scanrs);
	}
	remove_indiseq(results_seq, FALSE);
	results_seq=NULL;
	return indi;
}
/*==============================================
 * name_fragment_scan -- traverse name fragments
 *  looking for pattern matching
 *============================================*/
NOD0
name_fragment_scan (void)
{
	return name_scan(NAMESCAN_FRAG);
}
/*======================================
 * full_name_scan -- traverse full names
 *  looking for pattern matching
 *====================================*/
NOD0
full_name_scan (void)
{
	return name_scan(NAMESCAN_FULL);
}
/*==============================
 * refn_scan -- traverse refns
 *  looking for pattern matching
 *============================*/
NOD0
refn_scan (void)
{
	SCAN_PATTERN patt;
	NOD0 nod0 = NULL;
	STRING str;
	INT scantype = REFNSCAN;

	patt.scantype = scantype;
	while (1) {
		str = ask_for_string(scnrfn, scantt);
		if (!str || !str[0])
			return NULL;
		if (set_pattern(&patt, str, scantype))
			break;
	}

	results_seq = create_indiseq_null();
	traverse_refns(rs_callback, &patt);

	if (length_indiseq(results_seq)) {
		nod0 = choose_from_indiseq(results_seq, DOASK1, scanrs, scanrs);
	}
	remove_indiseq(results_seq, FALSE);
	results_seq=NULL;
	return nod0;
}
