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

extern STRING qSscanrs, qSscnrfn;

/*********************************************
 * local types
 *********************************************/

typedef struct
{
	INT scantype;
	CNSTRING statusmsg;
	char pattern[64];
	INDISEQ seq;
	STRING field; /* field to scan, eg "AUTH" for sources by author */
} SCANNER;

/*********************************************
 * local enums
 *********************************************/

static INT SCAN_NAME_FULL=0;
static INT SCAN_NAME_FRAG=1;
static INT SCAN_REFN=2;
static INT SCAN_SRC_AUTH=3;
static INT SCAN_SRC_TITL=4;

/*********************************************
 * local function prototypes
 *********************************************/

static void do_fields_scan(SCANNER * scanner, RECORD rec);
static void do_name_scan(SCANNER * scanner, STRING prompt);
static void do_sources_scan(SCANNER * scanner, CNSTRING prompt);
static BOOLEAN ns_callback(CNSTRING key, CNSTRING name, BOOLEAN newset, void *param);
static BOOLEAN rs_callback(CNSTRING key, CNSTRING refn, BOOLEAN newset, void *param);
static void scanner_add_result(SCANNER * scanner, CNSTRING key);
static BOOLEAN scanner_does_pattern_match(SCANNER *scanner, CNSTRING text);
static INDISEQ scanner_free_and_return_seq(SCANNER * scanner);
static void scanner_init(SCANNER * scanner, INT scantype, CNSTRING statusmsg);
static void scanner_set_field(SCANNER * scanner, STRING field);
static BOOLEAN scanner_set_pattern(SCANNER * scanner, STRING pattern);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==============================================
 * name_fragment_scan -- Ask for pattern and search all persons by name
 *  sts: [IN]  status to show during scan
 *============================================*/
INDISEQ
name_fragment_scan (STRING sts)
{
	SCANNER scanner;
	scanner_init(&scanner, SCAN_NAME_FRAG, sts);
	do_name_scan(&scanner, _("Enter pattern to match against single surname or given name."));
	return scanner_free_and_return_seq(&scanner);
}
/*======================================
 * full_name_scan -- Ask for pattern and search all persons by full name
 *  sts: [IN]  status to show during scan
 *====================================*/
INDISEQ
full_name_scan (STRING sts)
{
	SCANNER scanner;
	scanner_init(&scanner, SCAN_NAME_FULL, sts);
	do_name_scan(&scanner, _("Enter pattern to match against full name."));
	return scanner_free_and_return_seq(&scanner);
}
/*==============================
 * refn_scan -- Ask for pattern and search all refns
 *  sts: [IN]  status to show during scan
 *============================*/
INDISEQ
refn_scan (STRING sts)
{
	SCANNER scanner;
	scanner_init(&scanner, SCAN_REFN, sts);

	while (1) {
		char request[MAXPATHLEN];
		STRING prompt = _("Enter pattern to match against refn.");
		BOOLEAN rtn = ask_for_string(prompt, _("pattern: "),
			request, sizeof(request));
		if (!rtn || !request[0])
			return scanner_free_and_return_seq(&scanner);
		if (scanner_set_pattern(&scanner, request))
			break;
	}
	msg_status(sts);
	traverse_refns(rs_callback, &scanner);
	msg_status("");

	return scanner_free_and_return_seq(&scanner);
}
/*==============================
 * scan_souce_by_author -- Ask for pattern and search all sources by author
 *  sts: [IN]  status to show during scan
 *============================*/
INDISEQ
scan_souce_by_author (STRING sts)
{
	SCANNER scanner;
	scanner_init(&scanner, SCAN_SRC_AUTH, sts);
	scanner_set_field(&scanner, "AUTH");
	do_sources_scan(&scanner, _("Enter pattern to match against author."));
	return scanner_free_and_return_seq(&scanner);

}
/*==============================
 * scan_souce_by_title -- Ask for pattern and search all sources by title
 *  sts: [IN]  status to show during scan
 *============================*/
INDISEQ
scan_souce_by_title (STRING sts)
{
	SCANNER scanner;
	scanner_init(&scanner, SCAN_SRC_TITL, sts);
	scanner_set_field(&scanner, "TITL");
	do_sources_scan(&scanner, _("Enter pattern to match against author."));
	return scanner_free_and_return_seq(&scanner);
}
/*==============================
 * do_name_scan -- traverse names looking for pattern matching
 *  scanner:   [I/O] all necessary scan info, including sequence of results
 *  prompt:    [IN]  appropriate prompt to ask for pattern
 *============================*/
static void
do_name_scan (SCANNER * scanner, STRING prompt)
{
	while (1) {
		char request[MAXPATHLEN];
		BOOLEAN rtn = ask_for_string(prompt, _("pattern: "),
			request, sizeof(request));
		if (!rtn || !request[0])
			return;
		if (scanner_set_pattern(scanner, request))
			break;
	}
	msg_status((STRING)scanner->statusmsg);
	traverse_names(ns_callback, scanner);
	msg_status("");
}
/*==============================
 * do_sources_scan -- traverse sources looking for pattern matching
 *  scanner:   [I/O] all necessary scan info, including sequence of results
 *  prompt:    [IN]  appropriate prompt to ask for pattern
 *============================*/
static void
do_sources_scan (SCANNER * scanner, CNSTRING prompt)
{
	INT keynum = 0;

	while (1) {
		char request[MAXPATHLEN];
		BOOLEAN rtn = ask_for_string(prompt, _("pattern: "),
			request, sizeof(request));
		if (!rtn || !request[0])
			return;
		if (scanner_set_pattern(scanner, request))
			break;
	}
	/* msg_status takes STRING arg, should take CNSTRING - const declaration error */
	msg_status((STRING)scanner->statusmsg);

	while (1) {
		RECORD rec = 0;
		keynum = xref_nexts(keynum);
		if (!keynum)
			break;
		rec = keynum_to_srecord(keynum);
		do_fields_scan(scanner, rec);
	}
	msg_status("");
}
/*==============================
 * do_fields_scan -- traverse top nodes looking for desired field value
 *  scanner:   [I/O] all necessary scan info, including sequence of results
 *  rec:       [IN]  record to search
 *============================*/
static void
do_fields_scan (SCANNER * scanner, RECORD rec)
{
	/* NB: Only scanning top-level nodes right now */
	NODE node = nztop(rec);
	for (node = nchild(node); node; node = nsibling(node)) {
		STRING tag = ntag(node);
		if (tag && eqstr(tag, scanner->field)) {
			STRING val = nval(node);
			if (val && scanner_does_pattern_match(scanner, val)) {
				CNSTRING key = nzkey(rec);
				scanner_add_result(scanner, key);
				return;
			}
		}
	}
}
/*==============================
 * init_scan_pattern -- Initialize scan pattern fields
 *============================*/
static void
scanner_init (SCANNER * scanner, INT scantype, CNSTRING statusmsg)
{
	scanner->scantype = scantype;
	scanner->seq = create_indiseq_null();
	strcpy(scanner->pattern, "");
	scanner->statusmsg = statusmsg;
	scanner->field = NULL;
}
/*==============================
 * free_scanner_and_return_seq -- Free scanner data, except return result sequence
 *============================*/
static INDISEQ
scanner_free_and_return_seq (SCANNER * scanner)
{
	strfree(&scanner->field);
	return scanner->seq;
}
/*=================================================
 * set_pattern -- Store scanner pattern (or return FALSE if invalid)
 *===============================================*/
static BOOLEAN
scanner_set_pattern (SCANNER * scanner, STRING str)
{
	INT i;
	/* spaces don't make sense in a name fragment */
	if (scanner->scantype == SCAN_NAME_FRAG) {
		for (i=0; str[i]; i++)
			if (str[i] == ' ')
				return FALSE;
	}

	if (!fpattern_isvalid(str))
		return FALSE;

	if (strlen(str) > sizeof(scanner->pattern)-1)
		return FALSE;

	strcpy(scanner->pattern, str);

	return TRUE;
}
/*=================================================
 * scanner_set_field -- Store name of field to match
 *===============================================*/
static void
scanner_set_field (SCANNER * scanner, STRING field)
{
	strupdate(&scanner->field, field);
}
/*=============================================
 * scanner_does_pattern_match -- Compare a name to a pattern
 *===========================================*/
static BOOLEAN
scanner_does_pattern_match (SCANNER *scanner, CNSTRING text)
{
	return (fpattern_matchn(scanner->pattern, text));
}
/*=============================================
 * scanner_add_result -- Add a hit to the result list
 *===========================================*/
static void
scanner_add_result (SCANNER * scanner, CNSTRING key)
{
	/* if we pass in name, append_indiseq won't check for dups */
	append_indiseq_null(scanner->seq, strsave(key), NULL, FALSE, TRUE);
}
/*===========================================
 * ns_callback -- callback for name traversal
 *=========================================*/
static BOOLEAN
ns_callback (CNSTRING key, CNSTRING name, BOOLEAN newset, void *param)
{
	INT len, ind;
	STRING piece;
	SCANNER * scanner = (SCANNER *)param;
	newset=newset; /* unused */
	if (scanner->scantype == SCAN_NAME_FULL) {
		if (scanner_does_pattern_match(scanner, name)) {
			scanner_add_result(scanner, key);
		}
	} else {
		/* SCAN_NAME_FRAG */
		LIST list = name_to_list(name, &len, &ind);
		FORLIST(list, el)
			piece = (STRING)el;
			if (scanner_does_pattern_match(scanner, piece)) {
				scanner_add_result(scanner, key);
				STOPLIST
				break;
			}
		ENDLIST
		destroy_list(list);
	}
	return TRUE;
}
/*===========================================
 * rs_callback -- callback for refn traversal
 *=========================================*/
static BOOLEAN
rs_callback (CNSTRING key, CNSTRING refn, BOOLEAN newset, void *param)
{
	SCANNER * scanner = (SCANNER *)param;
	ASSERT(scanner->scantype == SCAN_REFN);
	newset=newset; /* unused */

	if (scanner_does_pattern_match(scanner, refn)) {
		scanner_add_result(scanner, key);
	}
	return TRUE;
}
