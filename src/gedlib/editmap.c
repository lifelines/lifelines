/*==========================================================
 * editmap.c -- LifeLines character mapping feature
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 09 Nov 1994
 *========================================================*/

#include "sys_inc.h"
#include <curses.h>
#include "standard.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "liflines.h"

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

	unlink(editfile);

	if (tran_tables[code])
		retrieve_file(map_keys[code], editfile);
	do_edit();
	while (TRUE) {
		tt = init_map_from_file(editfile, code, &err);
		if (!err) {
			if (tran_tables[code])
				remove_trantable(tran_tables[code]);
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
