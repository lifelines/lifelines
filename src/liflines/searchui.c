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
extern STRING qSmn_sea_ttl;
extern STRING qSsts_sca_ful,qSsts_sca_fra,qSsts_sca_ref,qSsts_sca_non;
extern STRING qSsts_sca_src;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static INDISEQ invoke_fullscan_menu(void);
static void repaint_fullscan_menu(UIWINDOW uiwin);
static INDISEQ invoke_search_source_menu(void);
static void repaint_search_menu(UIWINDOW uiwin);
static void repaint_search_source_menu(UIWINDOW uiwin);

/*********************************************
 * local variables
 *********************************************/

static UIWINDOW search_menu_win=NULL;
static UIWINDOW fullscan_menu_win=NULL;
static UIWINDOW search_source_menu_win=NULL;

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==============================
 * invoke_search_menu -- Handle search menu
 * If return set has one element, it has already been confirmed
 *============================*/
INDISEQ
invoke_search_menu (void)
{
	UIWINDOW uiwin=0;
	INDISEQ seq=0;
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
			seq = get_vhistory_list();
			break;
		case 'c':
			seq = get_chistory_list();
			if (seq)
				done=TRUE;
			break;
		case 'f':
			seq = invoke_fullscan_menu();
			break;
		case 'q': 
			done=TRUE;
			break;
		}
		if (seq)
			done=TRUE;
		deactivate_uiwin_and_touch_all();
	}
	return seq;
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
	mvccwaddstr(win, row++, 4, _("f  Full database scan"));
	mvccwaddstr(win, row++, 4, _(qSmn_ret));
	/* set cursor position */
	uiw_cury(uiwin) = 1;
	uiw_curx(uiwin) = 3+strlen(title);
}
/*==============================
 * invoke_fullscan_menu -- Handle fullscan menu
 * If return set has one element, it has already been confirmed
 *============================*/
static INDISEQ
invoke_fullscan_menu (void)
{
	UIWINDOW uiwin=0;
	INDISEQ seq=0;
	INT code=0;
	BOOLEAN done=FALSE;

	if (!fullscan_menu_win) {
		create_newwin2(&fullscan_menu_win, "fullscan", 8, 66);
		/* paint it for the first & only time (it's static) */
		repaint_fullscan_menu(fullscan_menu_win);
	}
	uiwin = fullscan_menu_win;

	while (!done) {
		activate_uiwin(uiwin);
		place_cursor_popup(uiwin);
		code = interact_choice_string(uiwin, "fnrsq");

		switch (code) {
		case 'f':
			seq = full_name_scan(_(qSsts_sca_ful));
			break;
		case 'n':
			seq = name_fragment_scan(_(qSsts_sca_fra));
			break;
		case 'r':
			seq = refn_scan(_(qSsts_sca_ref));
			break;
		case 's':
			seq = invoke_search_source_menu();
			break;
		case 'q': 
			done=TRUE;
			break;
		}
		if (seq) {
			if (length_indiseq(seq) > 0) {
				done=TRUE;
			} else {
				remove_indiseq(seq);
				seq = NULL;
			}
		}
		deactivate_uiwin_and_touch_all();
		if (!done)
			msg_status(_(qSsts_sca_non));
	}
	return seq;
}
/*=====================================
 * repaint_fullscan_menu -- Draw menu choices for main full scan menu
 *===================================*/
static void
repaint_fullscan_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	STRING title = _("What scan type?");
	draw_win_box(win);
	mvccwaddstr(win, row++, 2, title);
	mvccwaddstr(win, row++, 4, _("f  Full name scan"));
	mvccwaddstr(win, row++, 4, _("n  Name fragment (whitespace-delimited) scan"));
	mvccwaddstr(win, row++, 4, _("r  Refn scan"));
	mvccwaddstr(win, row++, 4, _("s  Source scan"));
	mvccwaddstr(win, row++, 4, _(qSmn_ret));
	/* set cursor position */
	uiw_cury(uiwin) = 1;
	uiw_curx(uiwin) = 3+strlen(title);
}
/*==============================
 * invoke_search_source_menu -- Handle fullscan menu
 * If return set has one element, it has already been confirmed
 *============================*/
static INDISEQ
invoke_search_source_menu (void)
{
	UIWINDOW uiwin=0;
	INDISEQ seq=0;
	INT code=0;
	BOOLEAN done=FALSE;

	if (!search_source_menu_win) {
		create_newwin2(&search_source_menu_win, "search_source", 7 ,66);
		/* paint it for the first & only time (it's static) */
		repaint_search_source_menu(search_source_menu_win);
	}
	uiwin = search_source_menu_win;

	while (!done) {
		activate_uiwin(uiwin);
		place_cursor_popup(uiwin);
		code = interact_choice_string(uiwin, "atq");

		switch (code) {
		case 'a':
			seq = scan_souce_by_author(_(qSsts_sca_src));
			break;
		case 't':
			seq = scan_souce_by_title(_(qSsts_sca_src));
			break;
		case 'q': 
			done=TRUE;
			break;
		}
		if (seq) {
			if (length_indiseq(seq) > 0) {
				done=TRUE;
			} else {
				remove_indiseq(seq);
				seq = NULL;
			}
		}

		deactivate_uiwin_and_touch_all();
		if (!done)
			msg_status(_(qSsts_sca_non));
	}
	return seq;
}
/*=====================================
 * repaint_search_source_menu -- Draw menu choices for searching all sources
 *===================================*/
static void
repaint_search_source_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	STRING title = _("Scan on what source field?");
	draw_win_box(win);
	mvccwaddstr(win, row++, 2, title);
	mvccwaddstr(win, row++, 4, _("a  Scan by author"));
	mvccwaddstr(win, row++, 4, _("t  Scan by title"));
	mvccwaddstr(win, row++, 4, _(qSmn_ret));
	/* set cursor position */
	uiw_cury(uiwin) = 1;
	uiw_curx(uiwin) = 3+strlen(title);
}
