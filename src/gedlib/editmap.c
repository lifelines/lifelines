/*==========================================================
 * editmap.c -- LifeLines character mapping feature
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 09 Nov 1994
 *========================================================*/

#include "sys_inc.h"
#include <curses.h>
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "liflines.h"
#include "screen.h"
#include "gedcomi.h"

#define NOMAPS 6

extern STRING map_keys[];
extern STRING cmperr, aredit, ronlye, dataerr;

/*==============================================
 * edit_mapping -- Edit character mapping record
 *  code: [in] which translation table (see defn of map_keys)
 *============================================*/
BOOLEAN
edit_mapping (INT code)
{
	TRANTABLE tt;
	BOOLEAN err;

	if (code < 0 || code >= NOMAPS) {
		mprintf_error("System error: illegal map code");
		return FALSE;
	}
	if (readonly) {
		message(ronlye);
		return FALSE;
	}
	endwin();

	unlink(editfile);

	if (tran_tables[code]) {
		INT rtn;
		rtn = retrieve_to_textfile(map_keys[code], editfile);
		if (rtn == RECORD_ERROR) {
			mprintf_error(dataerr);
			return FALSE;
		}
	}
	do_edit();
	while (TRUE) {
		tt = init_map_from_file(editfile, code, &err);
		if (!err) {
			if (tran_tables[code])
				remove_trantable(tran_tables[code]);
			tran_tables[code] = tt;
			store_text_file(map_keys[code], editfile);
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
