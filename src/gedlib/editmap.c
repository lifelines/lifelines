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
 * editmap.c -- LifeLines character mapping feature
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 09 Nov 1994
 *========================================================*/

#include <sys/types.h>
#include <sys/stat.h>
#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "translat.h"

#define NOMAPS 6

extern STRING map_keys[];
extern STRING cmperr, aredit, ronlye;
extern TRANTABLE init_map_from_file();

/*==============================================
 * edit_mapping -- Edit character mapping record
 *============================================*/
BOOLEAN edit_mapping (code)
INT code;	/* map code */
{
	char scratch[50];
	TRANTABLE tt;
	BOOLEAN err;

	if (code < 0 || code >= NOMAPS) {
		mprintf("System error: illegal map code");
		return FALSE;
	}
	if (readonly) {
		message(ronlye);
		return FALSE;
	}
	endwin();
	sprintf(scratch, "rm -f %s", editfile);
	system(scratch);
	if (tran_tables[code])
		retrieve_file(map_keys[code], editfile);
	do_edit();
	while (TRUE) {
		tt = init_map_from_file(editfile, code, &err);
		if (!err) {
			if (tran_tables[code])
				remove_trantable(*(tran_tables[code]));
			tran_tables[code] = tt;
			store_file(map_keys[code], editfile);
			return TRUE;
		}
		if (ask_yes_or_no_msg(cmperr, aredit))
			do_edit();
		else {
			remove_trantable(tt);
			return FALSE;
		}
		remove_trantable(tt);
	}
}
