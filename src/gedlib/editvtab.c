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
#include <curses.h>
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "feedback.h"
#include "bfs.h"

extern STRING aredit,dataerr,sepch;

static STRING trans_edin(STRING input, INT len);
static STRING trans_ined(STRING input, INT len);

/*==============================================
 * edit_valtab -- Edit value table from database
 *  key:   [IN]  db key where record to edit is stored
 *  ptab:  [I/O] hash table for key/value strings
 *  sep:   [IN]  separator char between key & value on each line
 *  ermsg: [IN]  error message to print in retry prompt if parse fails
 * Looks up key in db, gets record, puts it in temp file
 * and allows user to interactively edit it.
 * key/value pairs are separated by \n, and their is a
 * single character (the sep arg) between each key & value.
 *============================================*/
BOOLEAN
edit_valtab (STRING key, TABLE *ptab, INT sep, STRING ermsg)
{
	TABLE tmptab = NULL;
	STRING msg;
	char fullerr[78];
	STRING ptr;
	char temp[64], chardesc[8];
	INT mylen;
	TRANTABLE tti = tran_tables[MEDIN];
	endwin();

	unlink(editfile);

	if (retrieve_to_textfile(key, editfile, trans_ined) == RECORD_ERROR) {
		msg_error(dataerr);
		return FALSE;
	}
	do_edit();
	while (TRUE) {
		tmptab = create_table();
		if (init_valtab_from_file(editfile, tmptab, tti, sep, &msg)) {
			if (*ptab) remove_table(*ptab, DONTFREE);
			*ptab = tmptab;
			store_text_file_to_db(key, editfile, trans_edin);
			return TRUE;
		}
		ptr=fullerr;
		ptr[0]=0;
		mylen=sizeof(fullerr)/sizeof(fullerr[0]);
		llstrcatn(&ptr, ermsg, &mylen);
		llstrcatn(&ptr, " ", &mylen);
		llstrcatn(&ptr, msg, &mylen);
		llstrcatn(&ptr, " ", &mylen);
		snprintf(chardesc, sizeof(chardesc), "%c", (uchar)sep);
		snprintf(temp, sizeof(temp), sepch, chardesc); /* (separator is %s) */
		llstrcatn(&ptr, temp, &mylen);
		if (ask_yes_or_no_msg(fullerr, aredit))
			do_edit();
		else {
			remove_table(tmptab, DONTFREE);
			return FALSE;
		}
		remove_table(tmptab, DONTFREE);
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
	TRANTABLE tti = tran_tables[MEDIN];
	bfptr bfs = bfNew((int)(len*1.3));
	translate_string_to_buf(tti, input, bfs);
	return bfDetachAndKill(&bfs);
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
	TRANTABLE tto = tran_tables[MINED];
	bfptr bfs = bfNew((int)(len*1.2));
	translate_string_to_buf(tto, input, bfs);
	return bfDetachAndKill(&bfs);
}
