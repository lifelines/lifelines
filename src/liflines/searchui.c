/* 
   Copyright (c) 2006 Perry Rapp

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
/*=============================================================
 * searchui.c -- menus for full database search
 *  (see scan.c for code that actually scans entire database)
 * Copyright(c) 2006
 *===========================================================*/

#include "llstdlib.h"
#include "liflines.h"
#include "lloptions.h"
#include "llinesi.h"
#include "screen.h"
#include "screeni.h"
#include "cscurses.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSmn_ret;
extern STRING qSmn_sca_ttl,qSmn_sca_nmfu,qSmn_sca_nmfr,qSmn_sca_refn;
extern STRING qSmn_sea_ttl;
extern STRING qSsts_sca_ful,qSsts_sca_fra,qSsts_sca_ref,qSsts_sca_non;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static RECORD invoke_fullscan_menu(void);
static void repaint_fullscan_menu(UIWINDOW uiwin);
static void repaint_search_menu(UIWINDOW uiwin);

/*********************************************
 * local variables
 *********************************************/

static UIWINDOW search_menu_win=NULL, fullscan_menu_win=NULL;

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==============================
 * invoke_search_menu -- Handle search menu
 *============================*/
RECORD
invoke_search_menu (void)
{
	UIWINDOW uiwin=0;
	RECORD rec=0;
	INT code=0;
	BOOLEAN done=FALSE;

	if (!search_menu_win) {
		create_newwin2(&search_menu_win, "search_menu", 7,66);
	}
	/* repaint it every time, as history counts change */
	repaint_search_menu(search_menu_win);
	uiwin = search_menu_win;

	while (!done) {
		activate_uiwin(uiwin);
		place_cursor_popup(uiwin);
		code = interact_choice_string(uiwin, "vcfq");

		switch (code) {
		case 'v':
			rec = disp_vhistory_list();
			if (rec)
				done=TRUE;
			break;
		case 'c':
			rec = disp_chistory_list();
			if (rec)
				done=TRUE;
			break;
		case 'f':
			rec = invoke_fullscan_menu();
			if (rec)
				done=TRUE;
			break;
		case 'q': 
			done=TRUE;
			break;
		}
		deactivate_uiwin_and_touch_all();
	}
	return rec;
}
/*==============================
 * invoke_fullscan_menu -- Handle fullscan menu
 *============================*/
static RECORD
invoke_fullscan_menu (void)
{
	UIWINDOW uiwin=0;
	RECORD rec=0;
	INT code=0;
	BOOLEAN done=FALSE;

	if (!fullscan_menu_win) {
		create_newwin2(&fullscan_menu_win, "fullscan", 7,66);
		/* paint it for the first & only time (it's static) */
		repaint_fullscan_menu(fullscan_menu_win);
	}
	uiwin = fullscan_menu_win;

	while (!done) {
		activate_uiwin(uiwin);
		place_cursor_popup(uiwin);
		code = interact_choice_string(uiwin, "fnrq");

		switch (code) {
		case 'f':
			rec = full_name_scan(_(qSsts_sca_ful));
			if (rec)
				done=TRUE;
			break;
		case 'n':
			rec = name_fragment_scan(_(qSsts_sca_fra));
			if (rec)
				done=TRUE;
			break;
		case 'r':
			rec = refn_scan(_(qSsts_sca_ref));
			if (rec)
				done=TRUE;
			break;
		case 'q': 
			done=TRUE;
			break;
		}
		deactivate_uiwin_and_touch_all();
		if (!done)
			msg_status(_(qSsts_sca_non));
	}
	return rec;
}
/*=====================================
 * repaint_fullscan_menu -- Draw menu choices for main full scan menu
 *===================================*/
static void
repaint_fullscan_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	STRING title = _(qSmn_sca_ttl);
	draw_win_box(win);
	mvccwaddstr(win, row++, 2, title);
	mvccwaddstr(win, row++, 4, _(qSmn_sca_nmfu));
	mvccwaddstr(win, row++, 4, _(qSmn_sca_nmfr));
	mvccwaddstr(win, row++, 4, _(qSmn_sca_refn));
	mvccwaddstr(win, row++, 4, _(qSmn_ret));
	/* set cursor position */
	uiw_cury(uiwin) = 1;
	uiw_curx(uiwin) = 3+strlen(title);
}
/*=====================================
 * repaint_search_menu -- Draw menu for main history/scan menu
 *===================================*/
static void
repaint_search_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	STRING title = _(qSmn_sea_ttl);
	INT n = 0;
	char buffer[80];
	draw_win_box(win);
	mvccwaddstr(win, row++, 2, title);
	n = get_vhist_len();
	if (n>0) {
		llstrncpyf(buffer, sizeof(buffer), uu8
			, _pl("v  Review visit history (%d record)"
			, "v  Review visit history (%d records)"
			, n), n);
	} else {
		llstrncpy(buffer, _("(visit history is empty)"), sizeof(buffer), uu8);
	}
	mvccwaddstr(win, row++, 4, buffer);
	n = get_chist_len();
	if (n>0) {
		llstrncpyf(buffer, sizeof(buffer), uu8
			, _pl("c  Review change history (%d record)"
			, "c  Review change history (%d records)"
			, n), n);
	} else {
		llstrncpy(buffer, _("(change history is empty)")
			, sizeof(buffer), uu8);
	}
	mvccwaddstr(win, row++, 4, buffer);
	mvccwaddstr(win, row++, 4, ("f  Full database scan"));
	mvccwaddstr(win, row++, 4, _(qSmn_ret));
	/* set cursor position */
	uiw_cury(uiwin) = 1;
	uiw_curx(uiwin) = 3+strlen(title);
}
