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
 *   3.0.0 - 12 Sep 94    3.0.2 - 22 Dec 94
 *===========================================================*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <curses.h>
#include "standard.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "liflines.h"

extern STRING aredit;

/*==============================================
 * edit_valtab -- Edit value table from database
 *============================================*/
BOOLEAN edit_valtab (key, ptab, sep, ermsg)
STRING key;	/* value table key */
TABLE *ptab;	/* hash table for values */
INT sep;	/* separator char */
STRING ermsg;	/* error message */
{
	TABLE tmptab = NULL;
	STRING msg;
	endwin();

	unlink(editfile);

	retrieve_file(key, editfile);
	do_edit();
	while (TRUE) {
		tmptab = create_table();
		if (init_valtab_from_file(editfile, tmptab, sep, &msg)) {
			if (*ptab) remove_table(*ptab, DONTFREE);
			*ptab = tmptab;
			store_file(key, editfile);
			return TRUE;
		}
		if (ask_yes_or_no_msg(ermsg, aredit))
			do_edit();
		else {
			remove_table(tmptab, DONTFREE);
			return FALSE;
		}
		remove_table(tmptab, DONTFREE);
	}
}
