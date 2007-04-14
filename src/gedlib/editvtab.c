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
 * editvtab.c -- Handle value tables in LifeLines
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   3.0.0 - 12 Sep 94    3.0.2 - 22 Dec 94
 *===========================================================*/

#include "sys_inc.h"
#include "mycurses.h"
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "feedback.h"
#include "zstr.h"

extern STRING qSaredit,qSdataerr,qSsepch,qSronly;

static STRING trans_edin(STRING input, INT len);
static STRING trans_ined(STRING input, INT len);
static BOOLEAN edit_valtab_impl(TABLE *ptab, INT sep, STRING ermsg, STRING (*validator)(TABLE tab, void * param), void * param);

/*
TODO
 2002-10-05, Perry: This file does I/O, so may need to be moved out of gedlib
*/

/*==============================================
 * edit_valtab_from_db -- Edit value table from database
 *  key:       [IN]  db key where record to edit is stored
 *  ptab:      [I/O] hash table for key/value strings
 *  sep:       [IN]  separator char between key & value on each line
 *  ermsg:     [IN]  error message to print in retry prompt if parse fails
 *  validator: [IN]  caller specified validation (optional)
 * Looks up key in db, gets record, puts it in temp file
 * and allows user to interactively edit it.
 * key/value pairs are separated by \n, and their is a
 * single character (the sep arg) between each key & value.
 *============================================*/
BOOLEAN
edit_valtab_from_db (STRING key, TABLE *ptab, INT sep, STRING ermsg, STRING (*validator)(TABLE tab, void * param), void * param)
{
	unlink(editfile);

	if (retrieve_to_textfile(key, editfile, trans_ined) == RECORD_ERROR) {
		msg_error(_(qSdataerr));
		return FALSE;
	}
	if (!edit_valtab_impl(ptab, sep, ermsg, validator, param))
		return FALSE;

	if (readonly) {
		msg_error(_(qSronly));
		return FALSE;
	}
	store_text_file_to_db(key, editfile, trans_edin);
	return TRUE;
}
/*==============================================
 * edit_valtab_impl -- Edit value table from database
 *  key:       [IN]  db key where record to edit is stored (optional)
 *  ptab:      [I/O] hash table for key/value strings
 *  sep:       [IN]  separator char between key & value on each line
 *  ermsg:     [IN]  error message to print in retry prompt if parse fails
 *  validator: [IN]  caller specified validation (optional)
 * Looks up key in db, gets record, puts it in temp file
 * and allows user to interactively edit it.
 * key/value pairs are separated by \n, and their is a
 * single character (the sep arg) between each key & value.
 *============================================*/
static BOOLEAN
edit_valtab_impl (TABLE *ptab, INT sep, STRING ermsg, STRING (*validator)(TABLE tab, void * param), void * param)
{
	TABLE tmptab = NULL;
	STRING msg;
	static char fullerr[78]; /* TODO -- fix static usage */
	STRING ptr;
	XLAT ttmi = transl_get_predefined_xlat(MEDIN); /* editor to internal */

	do_edit();
	while (TRUE) {
		tmptab = create_table_str();
		if (init_valtab_from_file(editfile, tmptab, ttmi, sep, &msg)) {
			if (!validator || !(ptr = (*validator)(tmptab, param))) {
				if (*ptab) destroy_table(*ptab);
				*ptab = tmptab;
				return TRUE;
			}
		} else {
			INT max = sizeof(fullerr);
			char temp[64], chardesc[8], temp2[2];

			llstrncpyf(fullerr, max, uu8, "%s %s ", ermsg, msg);
			llstrncpyf(chardesc, sizeof(chardesc), uu8, "%c", (uchar)sep);
			temp2[0] = sep;
			temp2[1] = 0;
			llstrncpyf(temp, sizeof(temp), uu8, _(qSsepch), temp2);  /* (separator is %s) */
			llstrapps(fullerr, max, uu8, temp);
			ptr = fullerr; /* static buffer */
		}
		if (ask_yes_or_no_msg(ptr, _(qSaredit)))
			do_edit();
		else {
			destroy_table(tmptab);
			return FALSE;
		}
		destroy_table(tmptab);
	}
}
/*==============================================
 * trans_edin -- Translate editor text to internal
 * returns stdalloc'd buffer
 * this is used for a TRANSLFNC
 * Assumes non-empty input
 * Created: 2001/07/22 (Perry Rapp)
 *============================================*/
static STRING
trans_edin (STRING input, INT len)
{
	XLAT ttmi = transl_get_predefined_xlat(MEDIN); /* editor to internal */
	ZSTR zstr = translate_string_to_zstring(ttmi, input);
	STRING str = strdup(zs_str(zstr));
	len=len; /* unused */
	zs_free(&zstr);
	return str;
}
/*==============================================
 * trans_ined -- Translate internal text to editor
 * returns stdalloc'd buffer
 * this is used for a TRANSLFNC
 * Assumes non-empty input
 * Created: 2001/07/22 (Perry Rapp)
 *============================================*/
static STRING
trans_ined (STRING input, INT len)
{
	XLAT ttmo = transl_get_predefined_xlat(MINED);
	ZSTR zstr = translate_string_to_zstring(ttmo, input);
	STRING str = strdup(zs_str(zstr));
	len=len; /* unused */
	zs_free(&zstr);
	return str;
}
