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
/* modified 2000-04-25 J.F.Chandler */
/*=============================================================
 * screen.c -- Curses user interface to LifeLines
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 29 Sep 93
 *   2.3.6 - 01 Jan 94    3.0.0 - 06 Oct 94
 *   3.0.2 - 25 Mar 95    3.0.3 - 17 Jan 96
 *===========================================================*/

#include "sys_inc.h"
#include <stdarg.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "interp.h"
#include "screen.h"
#include "liflines.h"
#include "arch.h"
#include "menuitem.h"
#include "lloptions.h"
#include "screen.h"

#include "llinesi.h"

#define LINESREQ 24
#define COLSREQ  80
#define MAXVIEWABLE 30
#define OVERHEAD_MENU 5
INT LISTWIN_WIDTH=0;
INT MAINWIN_WIDTH=0;
static INT cur_list_detail_lines = 0;

/* center windows on real physical screen (LINES x COLS) */
#define NEWWIN(r,c)   newwin(r,c,(LINES - (r))/2,(COLS - (c))/2)
#define SUBWIN(w,r,c) subwin(w,r,c,(LINES - (r))/2,(COLS - (c))/2)
#ifdef BSD
#	define BOX(w,r,c) box(w,ACS_VLINE,ACS_HLINE)
#else
#	define BOX(w,r,c) box(w,r,c)
#endif



/*********************************************
 * global/exported variables
 *********************************************/

INT ll_lines = LINESREQ; /* update to be number of lines in screen */
INT ll_cols = COLSREQ;	 /* number of columns in screen used by LifeLines */
BOOLEAN stdout_vis = FALSE;
INT cur_screen = 0;
UIWINDOW *main_win = NULL;
UIWINDOW *stdout_win=NULL, *stdout_box_win=NULL;
UIWINDOW *debug_win=NULL, *debug_box_win=NULL;
UIWINDOW *ask_win=NULL, *ask_msg_win=NULL;
UIWINDOW *choose_from_list_win=NULL;
UIWINDOW *add_menu_win=NULL, *del_menu_win=NULL;
UIWINDOW *scan_menu_win=NULL, *cset_menu_win=NULL, *rpt_cset_menu_win=NULL;
UIWINDOW *utils_menu_win=NULL, *tt_menu_win=NULL;
UIWINDOW *trans_menu_win=NULL;
UIWINDOW *extra_menu_win=NULL;

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN alldone, progrunning;
extern STRING empstr, empstr71, readpath;
extern STRING abverr, uoperr;
extern STRING mtitle,cright,plschs;
extern STRING mn_unkcmd,ronlya,ronlyr;
extern STRING askynq,askynyn,askyny;
extern STRING mn_quit,mn_ret;
extern STRING mn_mmprpt,mn_mmrpt,mn_mmcset;
extern STRING mn_csttl,mn_cstt,mn_csintcs,mn_csrptcs,mn_csndloc;
extern STRING mn_cstsort,mn_cspref,mn_cschar,mn_cslcas,mn_csucas,mn_csrpt;
extern STRING mn_csdsploc,mn_csrpttl,mn_csrptloc,mn_csnrloc;
extern STRING idsortttl,idloc;
extern STRING mn_edttttl;
extern STRING mn_utsave,mn_utread,mn_utkey,mn_utkpers,mn_utdbstat,mn_utmemsta;
extern STRING mn_utplaces,mn_utusropt;
extern STRING mn_xxbsour, mn_xxbeven, mn_xxbothr, mn_xxasour, mn_xxesour;
extern STRING mn_xxaeven, mn_xxeeven, mn_xxaothr, mn_xxeothr;
extern STRING chlist,vwlist,errlist,defttl;

extern STRING mn_uttl;
extern STRING mn_xttl;
extern STRING mn_notimpl;

extern STRING mn_add_ttl,mn_add_indi,mn_add_fam,mn_add_chil,mn_add_spou;
extern STRING mn_del_ttl,mn_del_chil,mn_del_spou,mn_del_indi,mn_del_fam;
extern STRING mn_sca_ttl,mn_sca_nmfu,mn_sca_nmfr,mn_sca_refn;
extern STRING mn_tt_ttl,mn_tt_edit,mn_tt_load,mn_tt_save,mn_tt_exp,mn_tt_imp,mn_tt_dir;
extern STRING mn_tt_edin,mn_tt_ined,mn_tt_gdin,mn_tt_ingd;
extern STRING mn_tt_dsin,mn_tt_inds,mn_tt_inrp;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void activate_uiwin(UIWINDOW * uiwin);
static INT array_interact(UIWINDOW *win, STRING ttl, INT len, STRING *strings, BOOLEAN selecting);
static void begin_action(void);
static INT calculate_screen_lines(INT screen);
static INT choose_or_view_array (STRING ttl, INT no, STRING *pstrngs, BOOLEAN selecting);
#ifdef HAVE_SETLOCALE
static void choose_sort(STRING optname);
#endif
static INT choose_tt(UIWINDOW *wparent, STRING prompt);
static UIWINDOW *choose_win(INT desiredlen, INT *actuallen);
static void clear_msgs(void);
static void clearw(void);
static void create_windows(void);
static void deactivate_uiwin(void);
static void disp_codeset(UIWINDOW * uiwin, INT row, INT col, STRING menuit, INT codeset);
static void disp_locale(UIWINDOW * uiwin, INT row, INT col, STRING menuit);
static void disp_trans_table_choice(UIWINDOW * uiwin, INT row, INT col, STRING menuit, INT indx);
static void display_status(STRING text);
static void edit_tt_menu(UIWINDOW *wparent);
static void end_action(void);
static void export_tts(void);
static void import_tts(void);
static INT indiseq_interact(UIWINDOW *uiwin, STRING ttl, INDISEQ seq);
static INDISEQ indiseq_list_interact(UIWINDOW *uiwin, STRING ttl, INDISEQ seq);
static INT interact(UIWINDOW *uiwin, STRING str, INT screen);
static NODE invoke_add_menu(void);
static void invoke_cset_menu(UIWINDOW *wparent);
static void invoke_del_menu(void);
static INT invoke_extra_menu(void);
static RECORD invoke_scan_menu(void);
static void invoke_trans_menu(UIWINDOW *wparent);
static void invoke_utils_menu(void);
static void load_tt_menu(UIWINDOW *wparent);
static void msg_impl(STRING fmt, va_list args, INT level);
static void output_menu(UIWINDOW *uiwin, INT screen);
void place_cursor(void);
static void place_std_msg(void);
static void refresh_main(void);
static void repaint_add_menu(UIWINDOW * uiwin);
static void repaint_delete_menu(UIWINDOW * uiwin);
static void repaint_scan_menu(UIWINDOW * uiwin);
static void repaint_cset_menu(UIWINDOW * uiwin);
static void repaint_rpc_menu(UIWINDOW * uiwin);
static void repaint_tt_menu(UIWINDOW * uiwin);
static void repaint_trans_menu(UIWINDOW * uiwin);
static void repaint_utils_menu(UIWINDOW * uiwin);
static void repaint_extra_menu(UIWINDOW * uiwin);
static void rpt_cset_menu(UIWINDOW *wparent);
static void run_report(BOOLEAN picklist);
static void save_tt_menu(UIWINDOW *wparent);
static void show_indi_mode(NODE indi, INT mode, INT row, INT hgt, BOOLEAN reuse);
static void show_fam_mode(NODE fam, INT mode, INT row, INT hgt, INT width, BOOLEAN reuse);
static void show_tandem_line(UIWINDOW * win, INT row);
static void shw_list(UIWINDOW *uiwin, INDISEQ seq, INT len0, INT top, INT cur, INT *scroll);
static INT update_menu(INT screen);
static void user_options(void);
static void vmprintf(STRING fmt, va_list args);
static void win_list_init(void);

/*********************************************
 * local variables
 *********************************************/

static INT menu_enabled = 1;
static INT menu_dirty = 0;

static char showing[150];
static BOOLEAN now_showing_status = FALSE;


static INT BAND;

/* total screen lines used */
INT LINESTOTAL = LINESREQ;
/* number of lines for various menus */
static INT EMPTY_MENU = -1; /* save one horizontal line */
static INT EMPTY_LINES;
/* the following values are increased if ll_lines > LINESREQ */
int TANDEM_LINES = 6;		/* number of lines of tandem info */
int LIST_LINES = 6;		/* number of lines of person info in list */
int AUX_LINES = 15;		/* number of lines in aux window */
int VIEWABLE = 10;		/* can be increased up to MAXVIEWABLE */

int winx=0, winy=0; /* user specified window size */

static LIST msg_list = 0;
static BOOLEAN msg_flag = FALSE; /* need to show msg list */
static BOOLEAN viewing_msgs = FALSE; /* user is viewing msgs */
static BOOLEAN suppress_std_msg = FALSE; /* to hold status message */
static UIWINDOW * active_uiwin = 0;

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*============================
 * init_screen -- Init screens
 * Created: c. 2000/11, Perry Rapp
 *==========================*/
int
init_screen (void)
{
	int extralines;
	if (winx) { /* user specified window size */
		ll_lines = winy;
		ll_cols = winx;
		if (ll_cols > COLS || ll_lines > LINES) {
			endwin();
			fprintf(stderr, "The requested window size (%d,%d) is too large for your terminal (%d,%d).\n",
				ll_cols, ll_lines, COLS, LINES);
			return 0; /* fail */
		}
		if (ll_cols < COLSREQ || ll_lines < LINESREQ) {
			endwin();
			fprintf(stderr, "The requested window size (%d,%d) is too small for LifeLines (%d,%d).\n",
				ll_cols, ll_lines, COLSREQ, LINESREQ);
			return 0; /* fail */
		}
	}
	else {
		ll_lines = LINES;	/* use all available lines */
		ll_cols = COLSREQ;	/* only use this many columns ??? */

		if (COLS < COLSREQ || LINES < LINESREQ) {
			endwin();
			fprintf(stderr, "Your terminal display (%d,%d) is too small for LifeLines (%d,%d).\n",
				COLS, LINES, COLSREQ, LINESREQ);
			return 0; /* fail */
		}
	}

	extralines = ll_lines - LINESREQ;
	LINESTOTAL = ll_lines;
	EMPTY_LINES = LINESTOTAL - OVERHEAD_MENU - EMPTY_MENU;
	if(extralines > 0) {
		TANDEM_LINES += (extralines / 2);
		AUX_LINES += extralines;
		LIST_LINES += extralines;
		VIEWABLE += extralines;
		if(VIEWABLE > MAXVIEWABLE) VIEWABLE = MAXVIEWABLE;
	}
	BAND = 25;	/* width of columns of menu (3) */
	create_windows();
	menuitem_initialize();
	return 1; /* succeed */
}
/*============================
 * term_screen -- Terminate screens
 * Created: 2001/02/01, Perry Rapp
 *  complement of init_screen
 *==========================*/
void
term_screen (void)
{
	menuitem_terminate();
}
/*=======================================
 * repaint_main_menu --
 *=====================================*/
void
repaint_main_menu (UIWINDOW * uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row;

	werase(win);
	BOX(win, 0, 0);
	show_horz_line(uiwin, 4, 0, ll_cols);
	show_horz_line(uiwin, ll_lines-3, 0, ll_cols);
	wmove(win, 1, 2);
	wprintw(win, mtitle, get_lifelines_version(ll_cols-4));
	mvwaddstr(win, 2, 4, cright);
	mvwprintw(win, 3, 4, "Current Database - %s", readpath);
	if (immutable)
		wprintw(win, " (immutable)");
	else if (readonly)
		wprintw(win, " (read only)");
	row = 5;
	mvwaddstr(win, row++, 2, plschs);
	mvwaddstr(win, row++, 4, "b  Browse the persons in the database");
	mvwaddstr(win, row++, 4, "s  Search database");
	mvwaddstr(win, row++, 4, "a  Add information to the database");
	mvwaddstr(win, row++, 4, "d  Delete information from the database");
	mvwaddstr(win, row++, 4, mn_mmprpt);
	mvwaddstr(win, row++, 4, mn_mmrpt);
	mvwaddstr(win, row++, 4, mn_mmcset);
	mvwaddstr(win, row++, 4, "t  Modify character translation tables");
	mvwaddstr(win, row++, 4, "u  Miscellaneous utilities");
	mvwaddstr(win, row++, 4, "x  Handle source, event and other records");
	mvwaddstr(win, row++, 4, "q  Quit");
}
/*================================================
 * paint_screen -- Paint a screen using new menu code
 * Created: 2001/02/01, Perry Rapp
 *==============================================*/
void
paint_screen (INT screen)
{
	UIWINDOW *uiwin = main_win;
	WINDOW *win = uiw_win(uiwin);
	werase(win);
	BOX(win, 0, 0);
	show_horz_line(uiwin, ll_lines-3,  0, ll_cols);
	if (!menu_enabled)
		return;
	output_menu(uiwin, screen);
}
/*==============================================
 * paint_list_screen -- Paint list browse screen
 *============================================*/
void
paint_list_screen (void)
{
	UIWINDOW *uiwin = main_win;
	WINDOW *win = uiw_win(uiwin);
	INT row, col;
	werase(win);
	BOX(win, 0, 0);
	show_horz_line(uiwin, LIST_LINES+1, 0, ll_cols);
	show_horz_line(uiwin, ll_lines-3, 0, ll_cols);
	show_vert_line(uiwin, LIST_LINES+1, 52, 15);
#ifndef BSD
	mvwaddch(win, LIST_LINES+1, 52, ACS_TTEE);
#endif
	mvwaddstr(win, LIST_LINES+2, 54, "Choose an operation:");
	row = LIST_LINES+3; col = 55;
	mvwaddstr(win, row++, col, "j  Move down list");
	mvwaddstr(win, row++, col, "k  Move up list");
	mvwaddstr(win, row++, col, "e  Edit this person");
	mvwaddstr(win, row++, col, "i  Browse this person");
	mvwaddstr(win, row++, col, "m  Mark this person");
	mvwaddstr(win, row++, col, "d  Delete from list");
	mvwaddstr(win, row++, col, "t  Enter tandem mode");
	mvwaddstr(win, row++, col, "n  Name this list");
	mvwaddstr(win, row++, col, "b  Browse new persons");
	mvwaddstr(win, row++, col, "a  Add to this list");
	mvwaddstr(win, row++, col, "x  Swap mark/current");
	mvwaddstr(win, row++, col, mn_quit);
}
/*==========================================
 * create_uiwindow_impl -- Create our WINDOW wrapper
 * Created: 2001/11/24, Perry Rapp
 *========================================*/
static UIWINDOW *
create_uiwindow_impl (WINDOW * win)
{
	UIWINDOW * uiwin = (UIWINDOW *)stdalloc(sizeof(*uiwin));
	memset(uiwin, 0, sizeof(*uiwin));
	uiw_win(uiwin) = win;
	return uiwin;
}
/*==========================================
 * create_newwin -- Create our WINDOW wrapper
 * Created: 2001/11/24, Perry Rapp
 *========================================*/
static UIWINDOW *
create_newwin (INT rows, INT cols, INT begy, INT begx)
{
	WINDOW * win = newwin(rows, cols, begy, begx);
	return create_uiwindow_impl(win);
}
/*==========================================
 * create_newwin2 -- Create our WINDOW wrapper
 * Created: 2001/11/24, Perry Rapp
 *========================================*/
static UIWINDOW *
create_newwin2 (INT rows, INT cols)
{
	WINDOW * win = NEWWIN(rows, cols);
	return create_uiwindow_impl(win);
}
/*==========================================
 * create_uisubwindow -- Create our WINDOW wrapper
 *  for a true (& permanent) subwindow
 * Created: 2001/11/24, Perry Rapp
 *========================================*/
UIWINDOW *
create_uisubwindow (UIWINDOW * parent, INT rows, INT cols, INT begy, INT begx)
{
	WINDOW * win = subwin(uiw_win(parent), rows, cols, begy, begx);
	UIWINDOW * uiwin = create_uiwindow_impl(win);
	uiw_parent(uiwin) = parent;
	uiw_permsub(uiwin) = TRUE;
	return uiwin;
}
/*==========================================
 * create_uisubwindow2 -- Create our WINDOW wrapper
 *  for a true (& permanent) subwindow
 * Created: 2001/11/24, Perry Rapp
 *========================================*/
UIWINDOW *
create_uisubwindow2 (UIWINDOW * uiparent, INT rows, INT cols)
{
	INT begy = (LINES - rows)/2;
	INT begx = (COLS - cols)/2;
	return create_uisubwindow(uiparent, rows, cols, begy, begx);
}
/*==========================================
 * create_windows -- Create and init windows
 *========================================*/
void
create_windows (void)
{
	INT col;
	stdout_box_win = create_newwin2(ll_lines-4, ll_cols-4);
	stdout_win = create_uisubwindow2(stdout_box_win, ll_lines-6, ll_cols-6);
	scrollok(uiw_win(stdout_win), TRUE);
	col = COLS/4;
	debug_box_win = create_newwin(8, ll_cols-col-2, 1, col);
	debug_win = create_uisubwindow(debug_box_win, 6, ll_cols-col-4, 2, col+1);
	scrollok(uiw_win(debug_win), TRUE);

	MAINWIN_WIDTH = ll_cols;
	LISTWIN_WIDTH = 73;
 	main_win = create_newwin2(ll_lines, MAINWIN_WIDTH);
	tt_menu_win = create_newwin2(11,66);
	ask_win = create_newwin2(4, 73);
	ask_msg_win = create_newwin2(5, 73);
	choose_from_list_win = create_newwin2(15, 73);
	win_list_init();

	/* cset_menu_win is drawn dynamically */
	/* rpt_cset_menu_win is drawn dynamically */
	/* trans_menu_win is drawn dynamically */
	/* tt_menu_win is drawn dynamically */
	BOX(uiw_win(ask_win), 0, 0);
	BOX(uiw_win(ask_msg_win), 0, 0);
	BOX(uiw_win(debug_box_win), 0, 0);
}
/*=================================
 * display_screen -- Display screen
 *===============================*/
void
display_screen (INT new_screen)
{
	WINDOW * win = uiw_win(main_win);
	cur_screen = new_screen;
	if (stdout_vis) {
		llwprintf("\nStrike any key to continue.\n");
		crmode();
		(void) wgetch(uiw_win(stdout_win));
		nocrmode();
	}
	stdout_vis = FALSE;
	if (!now_showing_status)
		place_std_msg();
	else
		mvwaddstr(win, ll_lines-2, 2, showing);
	place_cursor();
	touchwin(win);
	wrefresh(win);
}
/*=====================================
 * main_menu -- Handle main_menu screen
 *===================================*/
void
main_menu (void)
{
	INT c;
	UIWINDOW * uiwin = main_win;
	WINDOW * win = uiw_win(uiwin);
	repaint_main_menu(uiwin);
	display_screen(MAIN_SCREEN);
	/* place_std_msg(); */ /*POSS*/
	c = interact(uiwin, "bsadprctuxq", -1);
	place_std_msg();
	wrefresh(win);
	switch (c) {
	case 'b': browse(NULL, BROWSE_INDI); break;
	case 's':
		{
			RECORD rec = invoke_scan_menu();
			if (rec)
				browse(nztop(rec), BROWSE_UNK);
		}
		break;
	case 'a': 
		{
			NODE node;
			if (readonly) {
				message(ronlya);
				break;
			}
			node = invoke_add_menu();
			if (node)
				browse(node, BROWSE_UNK);
		}
		break;
	case 'd':
		{
			if (readonly) {
				message(ronlyr);
				break;
			}
			invoke_del_menu();
		}
		break;
	case 'p': run_report(TRUE); break;
	case 'r': run_report(FALSE); break;
	case 'c': invoke_cset_menu(main_win); break;
	case 't': edit_tt_menu(main_win); break;
	case 'u': invoke_utils_menu(); break;
	case 'x': 
		c = invoke_extra_menu();
		if (c != BROWSE_QUIT)
			browse(NULL, c);
		break;
	case 'q': alldone = TRUE; break;
	}
}
/*=========================================
 * run_report -- run a report program
 *  picklist:  [IN]  display list of reports to user ?
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
void
run_report (BOOLEAN picklist)
{
	/*
	Begin/End action doesn't work because the llwprintf statements
	have a lot of embedded carriage returns
	*/
/*	begin_action();*/
	interp_main(picklist);
	if (length_list(msg_list) < 8)
		clear_msgs();
/*	end_action();*/
}
/*=========================================
 * update_menu -- redraw menu if needed
 *  uses new menus
 * Created: 2001/02/01, Perry Rapp
 *=======================================*/
static INT
update_menu (INT screen)
{
	INT lines = calculate_screen_lines(screen);
	if (menu_dirty || (cur_screen != screen))
		paint_screen(screen);
	menu_dirty = FALSE;
	return lines;
}
/*=========================================
 * show_indi_mode -- Show indi according to mode
 * [in] indi:  whom to display
 * [in] mode:  how to display
 * [in] row:   starting row to use
 * [in] hgt:   how many rows allowed
 * [in] reuse: flag to save recalculating display strings
 *=======================================*/
static void
show_indi_mode (NODE indi, INT mode, INT row, INT hgt, BOOLEAN reuse)
{
	CACHEEL icel;
	icel = indi_to_cacheel_old(indi);
	lock_cache(icel);
	if (mode=='g')
		show_gedcom_main(indi, GDVW_NORMAL, row, hgt, reuse);
	else if (mode=='x')
		show_gedcom_main(indi, GDVW_EXPANDED, row, hgt, reuse);
	else if (mode=='t')
		show_gedcom_main(indi, GDVW_TEXT, row, hgt, reuse);
	else if (mode=='a')
		show_ancestors(indi, row, hgt, reuse);
	else if (mode=='d')
		show_descendants(indi, row, hgt, reuse);
	else
		show_person_main(indi, row, hgt, reuse);
	unlock_cache(icel);
}
/*=========================================
 * show_fam_mode -- Show indi according to mode
 * [in] fam:  whom to display
 * [in] mode:  how to display
 * [in] row:   starting row to use
 * [in] hgt:   how many rows allowed
 * [in] width: how many columns allowed
 * [in] reuse: flag to save recalculating display strings
 *=======================================*/
static void
show_fam_mode (NODE fam, INT mode, INT row, INT hgt, INT width, BOOLEAN reuse)
{
	CACHEEL fcel;
	fcel = fam_to_cacheel(fam);
	lock_cache(fcel);
	if (mode=='g')
		show_gedcom_main(fam, GDVW_NORMAL, row, hgt, reuse);
	else if (mode=='x')
		show_gedcom_main(fam, GDVW_EXPANDED, row, hgt, reuse);
	else
		show_long_family(fam, row, hgt, width, reuse);
	unlock_cache(fcel);
}
/*=========================================
 * display_indi -- Paint indi on-screen
 *=======================================*/
void
display_indi (NODE indi, INT mode, BOOLEAN reuse)
{
	INT screen = ONE_PER_SCREEN;
	INT lines = update_menu(screen);
	show_indi_mode(indi, mode, 1, lines, reuse);
	display_screen(screen);
}
/*=========================================
 * interact_indi -- Get menu choice for indi browse
 *=======================================*/
INT
interact_indi (void)
{
	INT screen = ONE_PER_SCREEN;
	return interact(main_win, NULL, screen);
}
/*=======================================
 * display_fam -- Paint fam on-screen
 *=====================================*/
void
display_fam (NODE fam, INT mode, BOOLEAN reuse)
{
	INT width = MAINWIN_WIDTH;
	INT screen = ONE_FAM_SCREEN;
	INT lines = update_menu(screen);
	show_fam_mode(fam, mode, 1, lines, width, reuse);
	display_screen(screen);
}
/*=========================================
 * interact_fam -- Get menu choice for indi browse
 *=======================================*/
INT
interact_fam (void)
{
	INT screen = ONE_FAM_SCREEN;
	return interact(main_win, NULL, screen);
}
/*=============================================
 * display_2indi -- Paint tandem indi screen
 *===========================================*/
void
display_2indi (NODE indi1, NODE indi2, INT mode)
{
	INT screen = TWO_PER_SCREEN;
	INT lines = update_menu(screen);
	INT lines1,lines2;
	BOOLEAN reuse = FALSE; /* can't reuse display strings in tandem */
	lines--; /* for tandem line */
	lines2 = lines/2;
	lines1 = lines - lines2;

	show_indi_mode(indi1, mode, 1, lines1, reuse);
	show_tandem_line(main_win, lines1+1);
	switch_scrolls();
	show_indi_mode(indi2, mode, lines1+2, lines2, reuse);
	switch_scrolls();

	display_screen(screen);
}
/*=========================================
 * interact_2indi -- Get menu choice for tandem indi
 *=======================================*/
INT
interact_2indi (void)
{
	INT screen = TWO_PER_SCREEN;
	return interact(main_win, NULL, screen);
}
/*====================================
 * show_tandem_line -- Display horizontal line between top & bottom
 * PR 1999/03
 *==================================*/
static void
show_tandem_line (UIWINDOW * win, INT row)
{
	show_horz_line(win, row, 0, ll_cols);
}
/*=============================================
 * display_2fam -- Paint tandem families
 *===========================================*/
void
display_2fam (NODE fam1, NODE fam2, INT mode)
{
	INT width=MAINWIN_WIDTH;
	INT screen = TWO_FAM_SCREEN;
	INT lines = update_menu(screen);
	INT lines1,lines2;
	BOOLEAN reuse = FALSE; /* can't reuse display strings in tandem */
	lines--; /* for tandem line */
	lines2 = lines/2;
	lines1 = lines - lines2;

	show_fam_mode(fam1, mode, 1, lines1, width, reuse);
	show_tandem_line(main_win, lines1+1);
	switch_scrolls();
	show_fam_mode(fam2, mode, lines1+2, lines2, width, reuse);
	switch_scrolls();

	display_screen(screen);
}
/*=========================================
 * interact_2fam -- Get menu choice for tandem fam
 *=======================================*/
INT
interact_2fam (void)
{
	INT screen = TWO_FAM_SCREEN;
	return interact(main_win, NULL, screen);
}
/*=======================================
 * aux_browse -- Handle aux_browse screen
 * Implemented: 2001/01/27, Perry Rapp
 *=====================================*/
INT
aux_browse (NODE node, INT mode, BOOLEAN reuse)
{
	INT screen = AUX_SCREEN;
	INT lines = update_menu(screen);
	show_aux_display(node, mode, lines, reuse);
	display_screen(screen);
	return interact(main_win, NULL, screen);
}
/*=========================================
 * list_browse -- Handle list_browse screen
 *  cur & pindi are passed for GUI doing
 *  direct navigation in list
 *  this curses implementation does not use them
 *=======================================*/
INT
list_browse (INDISEQ seq,
             INT top,
             INT * cur,
             INT mark,
             NODE * pindi)
{
	if (cur_screen != LIST_SCREEN) paint_list_screen();
	show_list(seq, top, *cur, mark);
	display_screen(LIST_SCREEN);
	return interact(main_win, "jkeimdtbanxq", -1);
}
/*======================================
 * ask_for_db_filename -- Ask user for lifelines database directory
 *  returns static buffer
 *====================================*/
STRING
ask_for_db_filename (STRING ttl, STRING prmpt, STRING basedir)
{
	/* This could have a list of existing ones like askprogram.c */
	return ask_for_string(ttl, prmpt);
}
/*======================================
 * ask_for_output_filename -- Ask user for filename to which to write
 *  returns static buffer
 *====================================*/
STRING
ask_for_output_filename (STRING ttl, STRING path, STRING prmpt)
{
	path; /* unused by curses version */
	return ask_for_string(ttl, prmpt);
}
/*======================================
 * ask_for_input_filename -- Ask user for filename from which to read
 *  returns static buffer
 *====================================*/
STRING
ask_for_input_filename (STRING ttl, STRING path, STRING prmpt)
{
	path; /* unused by curses version */
	return ask_for_string(ttl, prmpt);
}
/*======================================
 * refresh_main -- touch & refresh main or stdout
 *  as appropriate
 *====================================*/
static void
refresh_main (void)
{
	WINDOW *win = stdout_vis ? uiw_win(stdout_win) : uiw_win(main_win);
	touchwin(win);
	wrefresh(win);
}
/*======================================
 * ask_for_string -- Ask user for string
 *  returns static buffer
 *  ttl:   [in] title of question (1rst line)
 *  prmpt: [in] prompt of question (2nd line)
 * returns static buffer (less than 100 chars)
 *====================================*/
STRING
ask_for_string (STRING ttl, STRING prmpt)
{
	UIWINDOW *uiwin = ask_win;
	WINDOW *win = uiw_win(uiwin);
	STRING rv, p;
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, 1, 1, ttl);
	mvwaddstr(win, 2, 1, prmpt);
	wrefresh(win);
	rv = get_answer(uiwin, prmpt); /* less then 100 chars */
	if (!rv) return (STRING) "";
	p = rv;
	while (chartype((unsigned char)*p) == WHITE)
		p++;
	striptrail(p);
	refresh_main();
	return p;
}
/*========================================
 * ask_yes_or_no -- Ask yes or no question
 *======================================*/
BOOLEAN
ask_yes_or_no (STRING ttl)
{
	STRING ptr;
	INT c = ask_for_char(ttl, askynq, askynyn);
	for (ptr = askyny; *ptr; ptr++) {
		if (c == *ptr) return TRUE;
	}
	return FALSE;
}
/*=========================================================
 * ask_yes_or_no_msg -- Ask yes or no question with message
 *=======================================================*/
BOOLEAN
ask_yes_or_no_msg (STRING msg, STRING ttl)
{
	STRING ptr;
	INT c = ask_for_char_msg(msg, ttl, askynq, askynyn);
	for (ptr = askyny; *ptr; ptr++) {
		if (c == *ptr) return TRUE;
	}
	return FALSE;
}
/*=======================================
 * ask_for_char -- Ask user for character
 *=====================================*/
INT
ask_for_char (STRING ttl,
              STRING prmpt,
              STRING ptrn)
{
	UIWINDOW *uiwin = ask_win;
	WINDOW *win = uiw_win(uiwin);
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, 1, 2, ttl);
	mvwaddstr(win, 2, 2, prmpt);
	wrefresh(win);
	return interact(uiwin, ptrn, -1);
}
/*===========================================
 * ask_for_char_msg -- Ask user for character
 *=========================================*/
INT
ask_for_char_msg (STRING msg,
                  STRING ttl,
                  STRING prmpt,
                  STRING ptrn)
{
	UIWINDOW *uiwin = ask_msg_win;
	WINDOW *win = uiw_win(uiwin);
	INT rv;
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, 1, 2, msg);
	mvwaddstr(win, 2, 2, ttl);
	mvwaddstr(win, 3, 2, prmpt);
	wrefresh(win);
	rv = interact(uiwin, ptrn, -1);
	return rv;
}
/*============================================
 * choose_from_array -- Choose from string list
 *  ttl:      [IN] title for choice display
 *  no:       [IN] number of choices
 *  pstrngs:  [IN] array of choices
 *==========================================*/
INT
choose_from_array (STRING ttl, INT no, STRING *pstrngs)
{
	BOOLEAN selecting = TRUE;
	if (!ttl) ttl=defttl;
	return choose_or_view_array(ttl, no, pstrngs, selecting);
}
/*============================================
 * view_array -- Choose from string list
 *  ttl:      [IN] title for choice display
 *  no:       [IN] number of choices
 *  pstrngs:  [IN] array of choices
 *==========================================*/
void
view_array (STRING ttl, INT no, STRING *pstrngs)
{
	BOOLEAN selecting = FALSE;
	choose_or_view_array(ttl, no, pstrngs, selecting);
}
/*============================================
 * choose_or_view_array -- Implement choose/view from array
 *  ttl:       [IN] title for choice display
 *  no:        [IN] number of choices
 *  pstrngs:   [IN] array of choices
 *  selecting: [IN] if FALSE then view-only
 *==========================================*/
static INT
choose_or_view_array (STRING ttl, INT no, STRING *pstrngs, BOOLEAN selecting)
{
	UIWINDOW *uiwin = choose_win(no, NULL);
	WINDOW *win = uiw_win(uiwin);
	INT rv;
	werase(win);
	BOX(win, 0, 0);
	wrefresh(win);
	rv = array_interact(uiwin, ttl, no, pstrngs, selecting);
	refresh_main();
	return rv;
}

/*=============================================================
 * choose_one_from_indiseq -- User chooses person from sequence
 * Resize rewrite: c. 2000/12, Perry Rapp
 *===========================================================*/
INT
choose_one_from_indiseq (STRING ttl, INDISEQ seq)
{
	UIWINDOW *uiwin=0;
	WINDOW *win=0;
	INT rv, len, actlen, minlen, asklen;
	INT top, cur, row, done;
	INT scroll;
	char fulltitle[128];
	char buffer[31];
	char * ptr;
	INT titlen;
	INT elemwidth=68; /* TO DO - how wide can this be ? */
	ASSERT(seq);
	len = length_indiseq(seq);
	if (len<50)
		preprint_indiseq(seq, elemwidth, &disp_shrt_rfmt);
		
	scroll=0;
	minlen = 4; /* TO DO: what should this be ? */
	top = cur = 0;
resize_win:
	asklen = len+cur_list_detail_lines;
	if (asklen < minlen)
		asklen = minlen;
	uiwin = choose_win(asklen, &actlen);
	win = uiw_win(uiwin);
	/* check in case we pushed current offscreen */
	touchwin(uiw_win(main_win));
	wrefresh(uiw_win(main_win));
	
	if (cur-scroll>actlen-1-cur_list_detail_lines)
		cur=actlen-1-cur_list_detail_lines+scroll;
	werase(win);
	BOX(win, 0, 0);
	wrefresh(win);
	len = length_indiseq(seq);
	werase(win);
	BOX(win, 0, 0);
	row = len + cur_list_detail_lines + 2;
	if (row > VIEWABLE + 2)
		row = VIEWABLE + 2;
	show_horz_line(uiwin, row++, 0, 73);
	mvwaddstr(win, row, 2, "Commands:   j Move down     k Move up    i Select     q Quit");
	done = FALSE;
	while (!done) {
		fulltitle[0] = 0;
		titlen = LISTWIN_WIDTH-1;
		if (titlen > sizeof(fulltitle))
			titlen = sizeof(fulltitle);
		ptr = fulltitle;
		llstrcatn(&ptr, ttl, &titlen);
		sprintf(buffer, " (%d/%d)", cur+1, len);
		llstrcatn(&ptr, buffer, &titlen);
		mvwaddstr(win, 1, 1, fulltitle);
		shw_list(uiwin, seq, len, top, cur, &scroll);
		wmove(win, row, 11);
		wrefresh(win);
		switch (interact(uiwin, "jkiq()[]", -1)) {
		case 'j':
			if (cur >= len - 1) break;
			cur++;
			if (cur >= top + VIEWABLE - cur_list_detail_lines)
				top++;
			break;
		case 'k':
			if (cur <= 0) break;
			cur--;
			if (cur + 1 == top) top--;
			break;
		case 'i':
			done=TRUE;
			break;
		case '(':
			if (scroll)
				scroll--;
			break;
		case ')':
			if (scroll<2)
				scroll++;
			break;
		case '[':
			if (cur_list_detail_lines) {
				cur_list_detail_lines--;
				goto resize_win;
			}
			break;
		case ']':
			if (cur_list_detail_lines < 8) {
				cur_list_detail_lines++;
				goto resize_win;
			}
			break;
		case 'q':
		default:
			done=TRUE;
			cur = -1;
			break;
		}
	}
	rv = cur;
	
	refresh_main();

	return rv;
}
/*==========================================================
 * choose_list_from_indiseq -- User chooses subsequence from
 *   person sequence
 * returns input sequence, but may have deleted elements
 * called by both reports & interactive use
 *  ttl:  [in] title/caption for choice list
 *  seq:  [in] list from which to choose
 *========================================================*/
INDISEQ
choose_list_from_indiseq (STRING ttl, INDISEQ seq)
{
	UIWINDOW *uiwin=0;
	WINDOW *win=0;
	INT len;
	INT elemwidth=68; /* TO DO - how wide can this be ? */
	ASSERT(seq);
	len = length_indiseq(seq);
	if (len<50)
		preprint_indiseq(seq, elemwidth, &disp_shrt_rfmt);
	uiwin = choose_win(len+cur_list_detail_lines, NULL);
	win = uiw_win(uiwin);
	werase(win);
	BOX(win, 0, 0);
	wrefresh(win);
	seq = indiseq_list_interact(uiwin, ttl, seq);
	refresh_main();
	return seq;
}
#ifdef HAVE_SETLOCALE
/*======================================
 * choose_sort -- Enter new sort locale
 * Created: 2001/07/21 (Perry Rapp)
 *====================================*/
static void
choose_sort (STRING optname)
{
	STRING str;
	STRING result = 0;
	while (!result) {
		str = ask_for_string(idsortttl, idloc);
		if (!str || !str[0]) return;
		result = setlocale(LC_COLLATE, str);
		if (result) {
			changeoptstr(optname, strsave(result));
		}
	}
}
#endif

/*==============================
 * draw_tt_win -- Draw menu for edit translations
 * Created: 2001/07/20 (Perry Rapp)
 *============================*/
static void
draw_tt_win (STRING prompt)
{
	UIWINDOW *uiwin = tt_menu_win;
	WINDOW *win = uiw_win(uiwin);
	INT row = 0;
	werase(win);
	BOX(win, 0, 0);
	row = 1;
	mvwaddstr(win, row++, 2, prompt);
	disp_trans_table_choice(uiwin, row++, 4, mn_tt_edin, MEDIN);
	disp_trans_table_choice(uiwin, row++, 4, mn_tt_ined, MINED);
	disp_trans_table_choice(uiwin, row++, 4, mn_tt_gdin, MGDIN);
	disp_trans_table_choice(uiwin, row++, 4, mn_tt_ingd, MINGD);
	disp_trans_table_choice(uiwin, row++, 4, mn_tt_dsin, MDSIN);
	disp_trans_table_choice(uiwin, row++, 4, mn_tt_inds, MINDS);
	disp_trans_table_choice(uiwin, row++, 4, mn_tt_inrp, MINRP);
	mvwaddstr(win, row++, 4, mn_ret);
}
/*==============================
 * disp_codeset -- Display code set line
 * including description
 * Created: 2001/08/02 (Perry Rapp)
 *============================*/
static void
disp_codeset (UIWINDOW * uiwin, INT row, INT col, STRING menuit, INT codeset)
{
	char buff[60];
	WINDOW * win = uiw_win(uiwin);
	int menulen = strlen(menuit);
	int buflen = sizeof(buff)-menulen;
	mvwaddstr(win, row, 4, menuit);
	mvwaddstr(win, row, 4+menulen, get_codeset_desc(codeset, buff, buflen));
}
/*==============================
 * disp_locale -- Display locale description
 * Created: 2001/08/02 (Perry Rapp)
 *============================*/
static void
disp_locale (UIWINDOW * uiwin, INT row, INT col, STRING menuit)
{
	char buff[60];
	WINDOW * win = uiw_win(uiwin);
	int menulen = strlen(menuit);
	int buflen = sizeof(buff)-menulen;
	mvwaddstr(win, row, 4, menuit);
	mvwaddstr(win, row, 4+menulen, get_sort_desc(buff, buflen));
}
/*==============================
 * disp_trans_table_choice -- Display line in
 * translation table menu, & show current info
 * Created: 2001/07/20 (Perry Rapp)
 *============================*/
static void
disp_trans_table_choice (UIWINDOW * uiwin, INT row, INT col, STRING menuit, INT indx)
{
	TRANTABLE tt = tran_tables[indx];
	char line[120];
	WINDOW * win = uiw_win(uiwin);
	INT mylen = sizeof(line);
	STRING ptr = line;

	ptr[0] = 0;
	llstrcatn(&ptr, menuit, &mylen);

	if (tt) {
		if (tt->name[0]) {
			llstrcatn(&ptr, "  :  ", &mylen);
			llstrcatn(&ptr, tt->name, &mylen);
		}
		else {
			llstrcatn(&ptr, "     (Unnamed table)", &mylen);
		}
	}
	else {
		llstrcatn(&ptr, "     (None)", &mylen);
	}
	mvwaddstr(win, row, col, line);
}
/*==============================
 * invoke_scan_menu -- Handle scan menu
 * Created: c. 2000/12, Perry Rapp
 *============================*/
RECORD
invoke_scan_menu (void)
{
	UIWINDOW *uiwin=0;
	WINDOW *win=0;
	RECORD rec=0;
	INT code=0;
	BOOLEAN done=FALSE;

	if (!scan_menu_win) {
		scan_menu_win = create_newwin2(7,66);
		/* paint it for the first & only time (it's static) */
		repaint_scan_menu(scan_menu_win);
	}
	uiwin = scan_menu_win;
	win = uiw_win(uiwin);
	activate_uiwin(uiwin);

	while (!done) {
		wmove(win, 1, 27);
		code = interact(uiwin, "fnrq", -1);

		switch (code) {
		case 'f':
			rec = full_name_scan();
			if (rec)
				done=TRUE;
			break;
		case 'n':
			rec = name_fragment_scan();
			if (rec)
				done=TRUE;
			break;
		case 'r':
			rec = refn_scan();
			if (rec)
				done=TRUE;
			break;
		case 'q': 
			done=TRUE;
			break;
		}
	}
	deactivate_uiwin();
	return rec;
}
/*============================
 * invoke_add_menu -- Handle add menu
 *==========================*/
static NODE
invoke_add_menu (void)
{
	UIWINDOW *uiwin=0;
	WINDOW * win=0;
	NODE node=NULL;
	INT code;

	if (!add_menu_win) {
		add_menu_win = create_newwin2(8, 66);
		/* paint it for the first & only time (it's static) */
		repaint_add_menu(add_menu_win);
	}
	uiwin = add_menu_win;
	win = uiw_win(uiwin);

	activate_uiwin(uiwin);
	wmove(win, 1, 27);
	code = interact(uiwin, "pfcsq", -1);
	deactivate_uiwin();

	switch (code) {
	case 'p':
		node = nztop(add_indi_by_edit());
		break;
	case 'f': add_family(NULL, NULL, NULL); break;
	case 'c': add_child(NULL, NULL); break;
	case 's': add_spouse(NULL, NULL, TRUE); break;
	case 'q': break;
	}
	return node;
}
/*===============================
 * invoke_del_menu -- Handle delete menu
 *=============================*/
void
invoke_del_menu (void)
{
	INT code;
	UIWINDOW *uiwin=0;
	WINDOW * win=0;
	if (!del_menu_win) {
		del_menu_win = create_newwin2(8, 66);
		/* paint it for the first & only time (it's static) */
		repaint_delete_menu(del_menu_win);
	}
	uiwin = del_menu_win;
	win = uiw_win(uiwin);

	activate_uiwin(uiwin);
	wmove(win, 1, 30);
	code = interact(uiwin, "csifq", -1);
	deactivate_uiwin();

	switch (code) {
	case 'c': choose_and_remove_child(NULL, NULL, FALSE); break;
	case 's': choose_and_remove_spouse(NULL, NULL, FALSE); break;
	case 'i': delete_indi(NULL, TRUE); break;
	case 'f': choose_and_remove_family(); break;
	case 'q': break;
	}
}
/*======================================
 * invoke_cset_menu -- Handle character set menu
 *====================================*/
static void
invoke_cset_menu (UIWINDOW * wparent)
{
	INT code=0;
	UIWINDOW *uiwin=0;
	WINDOW *win=0;
	BOOLEAN done=FALSE;

	if (!cset_menu_win) {
		cset_menu_win = create_newwin2(12,66);
	}
	uiwin = cset_menu_win;
	win = uiw_win(uiwin);
	activate_uiwin(uiwin);

	while (!done) {
		stdout_vis=FALSE;
		touchwin(win);
		repaint_cset_menu(uiwin);
		wrefresh(win);

		wmove(win, 1, strlen(mn_csttl)+3);
#ifdef NOTYET
		code = interact(uiwin, "Lscluprtq", -1);
#endif
		code = interact(uiwin, "Lsrtq", -1);

		switch (code) {
#ifdef HAVE_SETLOCALE
		case 'L': choose_sort("UiLocale"); uilocale(); break;
#endif
		case 's': edit_mapping(MSORT); break;
		case 'c': edit_mapping(MCHAR); break;
		case 'l': edit_mapping(MLCAS); break;
		case 'u': edit_mapping(MUCAS); break;
		case 'p': edit_mapping(MPREF); break;
		case 'r': rpt_cset_menu(uiwin); break;
		case 't': invoke_trans_menu(uiwin); break;
		case 'q': done=TRUE; break;
		}
	}
	deactivate_uiwin();
}
/*======================================
 * rpt_cset_menu -- Handle report character set menu
 *====================================*/
static void
rpt_cset_menu (UIWINDOW * wparent)
{
	INT code;
	UIWINDOW *uiwin=0;
	WINDOW *win=0;
	BOOLEAN done=FALSE;

	if (!rpt_cset_menu_win) {
		rpt_cset_menu_win = create_newwin2(7,66);
	}
	uiwin = rpt_cset_menu_win;
	win = uiw_win(uiwin);
	activate_uiwin(uiwin);

	while (!done) {
		stdout_vis=FALSE;
		touchwin(win);
		repaint_rpc_menu(uiwin);
		wrefresh(win);
		wmove(win, 1, strlen(mn_csttl)+3);
		code = interact(uiwin, "Lrq", -1);

		switch (code) {
#ifdef HAVE_SETLOCALE
		case 'L': choose_sort("RptLocale"); break;
#endif
		case 'r': edit_mapping(MINRP); break;
		case 'q': done=TRUE; break;
		}
	}
	deactivate_uiwin();
}
/*======================================
 * invoke_trans_menu -- menu for translation tables
 *====================================*/
static void
invoke_trans_menu (UIWINDOW *wparent)
{
	INT code;
	UIWINDOW *uiwin=0;
	WINDOW *win=0;
	BOOLEAN done=FALSE;

	if (!trans_menu_win) {
		trans_menu_win = create_newwin2(10,66);
	}
	uiwin = trans_menu_win;
	win = uiw_win(uiwin);
	activate_uiwin(uiwin);

	while (!done) {
		stdout_vis=FALSE;
		repaint_trans_menu(uiwin);
		activate_uiwin(uiwin);
		wmove(win, 1, strlen(mn_tt_ttl)+3);
		code = interact(uiwin, "elsxiq", -1);

		switch (code) {
		case 'e': edit_tt_menu(uiwin); break;
		case 'l': load_tt_menu(uiwin); break;
		case 's': save_tt_menu(uiwin); break;
		case 'x': export_tts(); break;
		case 'i': import_tts(); break;
		case 'q': done=TRUE; break;
		}
	}
	deactivate_uiwin();
}
/*======================================
 * edit_tt_menu -- menu for "Edit translation table"
 *====================================*/
static void
edit_tt_menu (UIWINDOW *wparent)
{
	INT tt;
	while ((tt = choose_tt(wparent, mn_edttttl)) != -1) {
		edit_mapping(tt);
	}
}
/*======================================
 * load_tt_menu -- menu for "Load translation table"
 *====================================*/
static void
load_tt_menu (UIWINDOW * wparent)
{
	message(mn_notimpl);
}
/*======================================
 * save_tt_menu -- menu for "Save translation table"
 *====================================*/
static void
save_tt_menu (UIWINDOW *wparent)
{
	message(mn_notimpl);
}
/*======================================
 * import_tts -- import translation tables
 *====================================*/
static void
import_tts (void)
{
	message(mn_notimpl);
}
/*======================================
 * export_tts -- export translation tables
 *====================================*/
static void
export_tts (void)
{
	message(mn_notimpl);
}
/*======================================
 * choose_tt -- select a translation table (-1 for none)
 *====================================*/
static INT
choose_tt (UIWINDOW *wparent, STRING prompt)
{
	INT code;
	UIWINDOW *uiwin = tt_menu_win;
	WINDOW *win = uiw_win(uiwin);
	while (1) {
		stdout_vis=FALSE;
		wrefresh(uiw_win(wparent));
		touchwin(win);
		draw_tt_win(prompt);
		wrefresh(win);
		wmove(win, 1, strlen(prompt)+3);
		code = interact(uiwin, "emixgdrq", -1);
		touchwin(uiw_win(wparent));
		wrefresh(uiw_win(wparent));
		switch (code) {
		case 'e': return MEDIN;
		case 'm': return MINED;
		case 'i': return MGDIN;
		case 'x': return MINGD;
		case 'g': return MDSIN;
		case 'd': return MINDS;
		case 'r': return MINRP;
		case 'q': return -1;
		}
	}
}
/*====================================
 * invoke_utils_menu -- Handle utilities menu
 *==================================*/
static void
invoke_utils_menu (void)
{
	INT code;
	UIWINDOW *uiwin=0;
	WINDOW *win=0;

	if (!utils_menu_win) {
		utils_menu_win = create_newwin2(12, 66);
		/* paint it for the first & only time (it's static) */
		repaint_utils_menu(utils_menu_win);
	}
	uiwin = utils_menu_win;
	win = uiw_win(uiwin);
	activate_uiwin(uiwin);

	wmove(win, 1, strlen(mn_uttl)+3);
	code = interact(uiwin, "srkidmeoq", -1);
	deactivate_uiwin();

	begin_action();
	switch (code) {
	case 's': archive_in_file(); break;
	case 'r': import_from_file(); break;
	case 'k': key_util(); break;
	case 'i': who_is_he_she(); break;
	case 'd': show_database_stats(); break;
	case 'm': display_cache_stats(); break;
	case 'e': edit_valtab("VPLAC", &placabbvs, ':', abverr); break;
	case 'o': user_options(); break;
	case 'q': break;
	}
	end_action();
}
/*================================
 * invoke_extra_menu -- Handle extra menu
 *==============================*/
static INT
invoke_extra_menu (void)
{
	INT code;
	UIWINDOW *uiwin=0;
	WINDOW *win=0;

	if (!extra_menu_win) {
		extra_menu_win = create_newwin2(13,66);
		/* paint it for the first & only time (it's static) */
		repaint_extra_menu(extra_menu_win);
	}
	uiwin = extra_menu_win;
	win = uiw_win(uiwin);
	activate_uiwin(uiwin);

	while (1) {
		wmove(win, 1, strlen(mn_xttl)+3);
		code = interact(uiwin, "sex123456q", -1);
		deactivate_uiwin();

		switch (code) {
		case 's': return BROWSE_SOUR;
		case 'e': return BROWSE_EVEN;
		case 'x': return BROWSE_AUX;
		case '1': add_source(); return BROWSE_QUIT;
		case '2': edit_source(NULL); return BROWSE_QUIT;;
		case '3': add_event(); return BROWSE_QUIT;;
		case '4': edit_event(NULL); return BROWSE_QUIT;;
		case '5': add_other(); return BROWSE_QUIT;;
		case '6': edit_other(NULL); return BROWSE_QUIT;;
		case 'q': return BROWSE_QUIT;;
		}
	}
}
/*===============================
 * user_options -- Edit user options
 * Created: 2001/08/02 (Perry Rapp)
 *=============================*/
static void
user_options (void)
{
	edit_valtab("VUOPT", &useropts, '=', uoperr);
	update_useropts();
}
/*===============================
 * interact -- Interact with user
 *=============================*/
static INT
interact (UIWINDOW *uiwin, STRING str, INT screen)
{
	char buffer[4]; /* 3 char cmds max */
	INT offset=0;
	INT cmdnum;
	INT c, i, n = str ? strlen(str) : 0;
	while (TRUE) {
		crmode();
		c = wgetch(uiw_win(uiwin));
		if (c == EOF) c = 'q';
		nocrmode();
		now_showing_status = FALSE;
		if (!progrunning && !suppress_std_msg)
			place_std_msg();
		if (str) { /* traditional */
			for (i = 0; i < n; i++) {
				if (c == str[i]) return c;
			}
		} else { /* new menus */
			if (offset < sizeof(buffer)-1) {
				buffer[offset] = c;
				buffer[offset+1] = 0;
				offset++;
			} else {
				buffer[0] = c;
				buffer[1] = 0;
				offset = 1;
			}
			cmdnum = menuitem_check_cmd(screen, buffer);
			if (cmdnum != CMD_NONE && cmdnum != CMD_PARTIAL)
				return cmdnum;
			if (cmdnum != CMD_PARTIAL) {
				message(mn_unkcmd);
				offset = 0;
			}
		}
	}
}
/*============================================
 * get_answer -- Have user respond with string
 *  win:   [in] which window to use
 *  prmpt: [in] prompt string to show
 *  returns static buffer 100 length
 *==========================================*/
STRING
get_answer (UIWINDOW *uiwin, STRING prmpt)
{
	static uchar lcl[100];
	WINDOW *win = uiw_win(uiwin);

#ifndef USEBSDMVGETSTR
	echo();
	/* would be nice to use mvwgetnstr here */
	mvwgetstr(win, 2, strlen(prmpt) + 2, lcl);
	noecho();
#else
	bsd_mvwgetstr(win, 2, strlen(prmpt) + 2, lcl, sizeof(lcl));
#endif
	return lcl;
}
/*===========================================================
 * win_list_init -- Create list of windows of increasing size
 *=========================================================*/
static UIWINDOW *list_wins[MAXVIEWABLE];
void
win_list_init (void)
{
	INT i;
	for (i = 0; i < VIEWABLE; i++) {
		list_wins[i] = create_newwin2(i+6, LISTWIN_WIDTH);
	}
}
/*=========================================
 * choose_win -- Choose window to hold list
 *=======================================*/
static UIWINDOW *
choose_win (INT desiredlen, INT *actuallen)
{
	UIWINDOW *win = list_wins[VIEWABLE-1];
	INT retlen;
	retlen = VIEWABLE;
	if (desiredlen <= VIEWABLE) {
		win = list_wins[desiredlen-1];
		retlen = desiredlen;
	}
	if (actuallen)
		*actuallen = retlen;
	return win;
}
/*=====================================================
 * indiseq_interact -- Interact with user over sequence
 *===================================================*/
static INT
indiseq_interact (UIWINDOW *uiwin, STRING ttl, INDISEQ seq)
{
	WINDOW *win = uiw_win(uiwin);
	INT top, cur, len, row;
	INT scroll=0;
	top = cur = 0;
	len = length_indiseq(seq);
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, 1, 1, ttl);
	row = len > VIEWABLE ? VIEWABLE + 2 : len + 2;
	show_horz_line(uiwin, row++, 0, 73);
	mvwaddstr(win, row, 2, "Commands:   j Move down     k Move up    i Select     q Quit");
	while (TRUE) {
		shw_list(uiwin, seq, len, top, cur, &scroll);
		wmove(win, row, 11);
		wrefresh(win);
		switch (interact(uiwin, "jkiq", -1)) {
		case 'j':
			if (cur >= len - 1) break;
			cur++;
			if (cur >= top + VIEWABLE) top++;
			break;
		case 'k':
			if (cur <= 0) break;
			cur--;
			if (cur + 1 == top) top--;
			break;
		case 'i':
			return cur;
		case 'q':
		default:
			return -1;
		}
	}
}

/*=====================================================
 * indiseq_list_interact --
 *===================================================*/
INDISEQ
indiseq_list_interact (UIWINDOW *uiwin, STRING ttl, INDISEQ seq)
{
	WINDOW *win = uiw_win(uiwin);
	INT top, cur, len, len0, row;
	INT scroll;

	top = cur = 0;
	len = len0 = length_indiseq(seq);
	scroll = 0;
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, 1, 1, ttl);
	row = len0 + cur_list_detail_lines + 2;
	if (row > VIEWABLE + 2)
		row = VIEWABLE + 2;
	show_horz_line(uiwin, row++, 0, 73);
	mvwaddstr(win, row, 2, "Commands:  j Move down   k Move up  d Delete   i Select   q Quit");
	while (TRUE) {
		shw_list(uiwin, seq, len0, top, cur, &scroll);
		wmove(win, row, 11);
		wrefresh(win);
		switch (interact(uiwin, "jkdiq", -1)) {
		case 'j':
			if (cur >= len - 1) break;
			cur++;
			if (cur >= top + VIEWABLE - cur_list_detail_lines)
				top++;
			break;
		case 'k':
			if (cur <= 0) break;
			cur--;
			if (cur + 1 == top) top--;
			break;
		case 'd':
			delete_indiseq(seq, NULL, NULL, cur);
			if (--len == 0) {
				remove_indiseq(seq);
				return NULL;
			}
			if (cur == len) cur--;
			if (cur < top) top = cur;
			break;
		case 'i':
			return seq;
		case 'q':
		default:
			return NULL;
		}
	}
}
/*=====================================================
 * shw_list -- Show string list in list interact window
 *  len0 is original length of list (items may have
 *  been deleted)
 * Detail lines rewrite: c. 2000/12, Perry Rapp
 *===================================================*/
void
shw_list (UIWINDOW *uiwin, INDISEQ seq, INT len0, INT top, INT cur, INT *scroll)
{
	WINDOW *win = uiw_win(uiwin);
	INT i, j, row, nrows, len, numdet;
	BOOLEAN reuse=FALSE; /* don't reuse display strings in list */
	/* TO DO - how big can we make buffer ?
	ie, how wide can print element be ? */
	char buffer[60];
	len = length_indiseq(seq);
	numdet = cur_list_detail_lines;
	nrows = numdet + len0;
	if (nrows>VIEWABLE)
		nrows=VIEWABLE;
	for (j=0; j<nrows; j++) {
		row=2+j;
		mvwaddstr(win, row, 1, empstr71);
		if (j>=numdet) {
			i=j-numdet+top;
			if (i<len) {
				if (i == cur) mvwaddch(win, row, 3, '>');
				print_indiseq_element(seq, i, buffer, sizeof(buffer), &disp_shrt_rfmt);
				mvwaddstr(win, row, 4, buffer);
			}
		}
	}
	if (numdet) {
		STRING key, name;
		element_indiseq(seq, cur, &key, &name);
		if (key[0]=='I') {
			NODE indi = key_to_indi(key);
			mvwaddstr(win, numdet+1, 2, "---");
			show_person((void *)uiwin, indi, 2, numdet-1, LISTWIN_WIDTH, scroll, reuse);
		}
	}
}
/*================================================================
 * shw_array_of_strings -- Show string list in list interact window
 *  uiwin:   [IN]  curses window to use
 *  strings: [IN]  array (of choices) to be listed
 *  len,:    [IN]  size of array
 *  top:     [IN]  show items starting with this one
 *  cur:     [IN]  currently selected item
 *==============================================================*/
void
shw_array_of_strings (UIWINDOW *uiwin, STRING *strings, INT len, INT top, INT cur)
{
	WINDOW *win = uiw_win(uiwin);
	INT i, j, row = len > VIEWABLE ? VIEWABLE + 1 : len + 1;
	for (i = 2; i <= row; i++)
		mvwaddstr(win, i, 1, empstr71);
	row = 2;
	for (i = top, j = 0; j < VIEWABLE && i < len; i++, j++) {
		/* for short lists, we show leading numbers */
		if (len<10) {
			char numstr[12]="";
			INT offset=0;
			snprintf(numstr, sizeof(numstr), "%d: ", i+1);
			if (i == cur) mvwaddch(win, row, 3, '>');
			mvwaddstr(win, row, 4, numstr);
			mvwaddstr(win, row, 4+strlen(numstr), strings[i]);
		} else {
			if (i == cur) mvwaddch(win, row, 3, '>');
			mvwaddstr(win, row, 4, strings[i]);
		}
		row++;
	}
}
/*==============================================
 * array_interact -- Interact with user over list
 *  uiwin:      [IN]  interaction window
 *  ttl:        [IN]  title
 *  len:        [IN]  number of choices
 *  strings:    [IN]  array of choices
 *  selectable: [IN]  FALSE for view-only
 *============================================*/
INT
array_interact (UIWINDOW *uiwin, STRING ttl, INT len, STRING *strings, BOOLEAN selectable)
{
	WINDOW *win = uiw_win(uiwin);
	INT top = 0, cur = 0, row;
	STRING responses = len<10 ? "jkiq123456789" : "jkiq";
	STRING promptline = selectable ? chlist : vwlist;
	char c;
	while (TRUE) {
		werase(win);
		BOX(win, 0, 0);
		mvwaddstr(win, 1, 1, ttl);
		row = len > VIEWABLE ? VIEWABLE + 2 : len + 2;
		show_horz_line(uiwin, row++, 0, 73);
		mvwaddstr(win, row, 2, promptline);
		shw_array_of_strings(uiwin, strings, len, top, cur);
		wrefresh(win);
		switch (c=interact(uiwin, responses, -1)) {
		case 'j':
			if (cur >= len - 1) break;
			cur++;
			if (cur >= top + VIEWABLE) top++;
			break;
		case 'k':
			if (cur <= 0) break;
			cur--;
			if (cur + 1 == top) top--;
			break;
		case 'i':
			if (selectable)
				return cur;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (c - '1' < len)
				return c - '1';
			break;
		case 'q':
		default:
			return -1;
		}
	}
}
/*===================================================
 * message_string -- Return background message string
 *=================================================*/
STRING
message_string (void)
{
	if (!cur_screen) return "";
	return g_ScreenInfo[cur_screen].Title;
}
/*=================================================
 * place_std_msg - Place standard message on screen
 *===============================================*/
void
place_std_msg (void)
{
	UIWINDOW *uiwin = main_win;
	WINDOW *win = uiw_win(uiwin);
	STRING str = message_string();
	INT row;
	wmove(win, row = ll_lines-2, 2);
	if (cur_screen != LIST_SCREEN) {
		wclrtoeol(win);
		mvwaddch(win, row, ll_cols-1, ACS_VLINE);
	} else
		mvwaddstr(win, row, 2, empstr);
	mvwaddstr(win, row, 2, str);
	place_cursor();
}
/*=================================================
 * llvwprintf -- Called as wprintf(fmt, argp)
 *===============================================*/
void
llvwprintf (STRING fmt, va_list args)
{
	UIWINDOW *uiwin = stdout_win;
	WINDOW *win = uiw_win(uiwin);
	if (!stdout_vis)
		clearw();
	vwprintw(win, fmt, args);
	wrefresh(win);
	/* following doesn't work because of embedded carriage returns */
/*	msg_impl(fmt, args, -1);*/ /* also send to msg list */
}
/*=================================================
 * llwprintf -- Called as wprintf(fmt, arg, arg, ...)
 *===============================================*/
void
llwprintf (STRING fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	llvwprintf(fmt, args);
	va_end(args);
}
/*==============================
 * clearw -- Clear stdout window
 *============================*/
void
clearw (void)
{
	UIWINDOW *uiwin = stdout_win;
	WINDOW *win = uiw_win(uiwin);
	werase(win);
	BOX(uiw_win(stdout_box_win), 0, 0);
	wmove(win, 0, 0);
	stdout_vis = TRUE;
	wrefresh(uiw_win(stdout_box_win));
}
/*=======================================
 * wfield -- Write field in stdout window
 *=====================================*/
void
wfield (INT row, INT col, STRING str)
{
	UIWINDOW *uiwin = stdout_win;
	WINDOW *win = uiw_win(uiwin);
	if (!stdout_vis) clearw();
	mvwaddstr(win, row, col, str);
	wrefresh(win);
}
/*===========================================
 * wpos -- Position to place in stdout window
 *=========================================*/
void
wpos (INT row, INT col)
{
	UIWINDOW *uiwin = stdout_win;
	WINDOW *win = uiw_win(uiwin);
	wmove(win, row, col);
}
/*=======================================
 * show_horz_line -- Draw horizontal line
 *=====================================*/
void
show_horz_line (UIWINDOW *uiwin, INT row, INT col, INT len)
{
	WINDOW *win = uiw_win(uiwin);
	INT i;
	mvwaddch(win, row, col, ACS_LTEE);
	for (i = 0; i < len-2; i++)
		waddch(win, ACS_HLINE);
	waddch(win, ACS_RTEE);
}
/*=====================================
 * show_vert_line -- Draw vertical line
 *===================================*/
void
show_vert_line (UIWINDOW *uiwin, INT row, INT col, INT len)
{
#ifndef BSD
	WINDOW *win = uiw_win(uiwin);
	INT i;
	mvwaddch(win, row++, col, ACS_TTEE);
	for (i = 0; i < len-2; i++)
		mvwaddch(win, row++, col, ACS_VLINE);
	mvwaddch(win, row, col, ACS_BTEE);
#endif
}
/*=============================================
 * place_cursor -- Move to idle cursor location
 *===========================================*/
void
place_cursor (void)
{
	/* TO DO - integrate menuitem version! */
	INT row, col = 30;
	switch (cur_screen) {
	case MAIN_SCREEN:    row = 5;        break;
	case ONE_PER_SCREEN: row = ll_lines-11; break;
	case ONE_FAM_SCREEN: row = ll_lines-9;  break;
	case AUX_SCREEN:     row = AUX_LINES+2;       break;
	case TWO_PER_SCREEN: row = 2*TANDEM_LINES+3;       break;
	case TWO_FAM_SCREEN: row = 2*TANDEM_LINES+3;       break;
	case LIST_SCREEN:    row = LIST_LINES+2; col = 75; break;
	default:             row = 1; col = 1; break;
	}
	wmove(uiw_win(main_win), row, col);
}
/*=============================================
 * dbprintf -- Debug printf(fmt, arg, arg, ...)
 *===========================================*/
void
dbprintf (STRING fmt, ...)
{
	va_list args;
	touchwin(uiw_win(debug_box_win));
	va_start(args, fmt);
	vwprintw(uiw_win(debug_win), fmt, args);
	va_end(args);
	wrefresh(uiw_win(debug_box_win));
	sleep(2);
	touchwin(uiw_win(main_win));
	wrefresh(uiw_win(main_win));
}
/*==================================================
 * do_edit -- Shift to user's screen editor and back
 *================================================*/
void
do_edit (void)
{
	endwin();
#ifdef WIN32
	/* use w32system, because it will wait for the editor to finish */
	w32system(editstr);
#else
	system(editstr);
#endif
	touchwin(uiw_win(main_win));
	clearok(curscr, 1);
	wrefresh(curscr);
	noecho();
}
/*==================================================================
 * bsd_mvwgetstr -- Special BSD version of mvwgetstr that does erase
 *   character handling
 *  win:  [in] window to use
 *  row:  [in] y location to start
 *  col:  [in] x location to start
 *  str:  [out] user's string
 *  len:  [in] max length of string
 *================================================================*/
#ifdef BSD
static void
bsd_mvwgetstr (UIWINDOW *win, INT row, INT col, STRING str, INT len)
{
	STRING p = str;
	INT c, ers = erasechar();
	wmove(win, row, col);
	crmode();
	while ((c = getch()) != '\n') {
		if (c == ers && p > str) {
			wmove(win, row, --col);
			waddch(win, ' ');
			wmove(win, row, col);
			--p;
		} else if (c != ers && p < str+len-1) {
			waddch(win, c);
			col++;
			*p++ = c;
		}
		wrefresh(win);
	}
	*p = 0;
	nocrmode();
}
#endif
/*================================================
 * output_menu -- print menu array to screen
 * for 3 column full width menus
 *==============================================*/
static void
output_menu (UIWINDOW *uiwin, INT screen)
{
	WINDOW *win = uiw_win(uiwin);
	INT row;
	INT icol=0;
	INT col=3;
	INT MenuRows = g_ScreenInfo[screen].MenuRows;
	INT MenuSize = g_ScreenInfo[screen].MenuSize;
	INT Item = 0;
	INT page, pageitems, pages;
	char prompt[128];
	MenuItem ** Menu = g_ScreenInfo[screen].Menu;
	INT OnePageFlag = 0;
	page = g_ScreenInfo[screen].MenuPage;
	pageitems = (MenuRows-1)*3-2;
	pages = (MenuSize-1)/pageitems+1;
	if (MenuSize <= pageitems+1) /* don't need '?' if they fit */
	{
		OnePageFlag = 1;
		page = 0;
		pages = 1;
	}
	Item = page * pageitems;
	if (Item >= MenuSize)
		Item = ((MenuSize-1)/pageitems)*pageitems;
	icol = 0;
	col = 3;
	row = LINESTOTAL-MenuRows-OVERHEAD_MENU+1;
	show_horz_line(uiwin, row++, 0, ll_cols);
	sprintf(prompt, "%s            (pg %d/%d)", 
		plschs, page+1, pages);
	mvwaddstr(win, row++, 2, prompt);
	while (1)
	{
		mvwaddstr(win, row, col, Menu[Item++]->Display);
		if (Item == MenuSize)
			break;
		row++;
		if (icol<2 && row==LINESTOTAL-3)
		{
			icol++;
			col += BAND;
			row = LINESTOTAL-MenuRows-2;
			continue;
		}
		if (OnePageFlag) {
			if (icol==2 && row==LINESTOTAL-4)
				break;
		} else {
			if (icol==2 && row==LINESTOTAL-5)
				break;
		}
	}
	row = LINESTOTAL-5; col = 3+BAND*2;
	if (!OnePageFlag)
		mvwaddstr(win, row, col, g_MenuItemOther.Display);
	mvwaddstr(win, ++row, col, g_MenuItemQuit.Display);
}
/*==================================================================
 * toggle_menu() - toggle display of menu
 * Created: 1999/02, Perry Rapp
 * Joined repository: 2001/01/28, Perry Rapp
 *================================================================*/
void
toggle_menu (void)
{
	menu_enabled = !menu_enabled;
	menu_dirty = 1;
}
/*==================================================================
 * cycle_menu() - show other menu choices
 * Created: 1999/03, Perry Rapp
 * Joined repository: 2001/01/28, Perry Rapp
 *================================================================*/
void
cycle_menu (void)
{
	INT MenuSize = g_ScreenInfo[cur_screen].MenuSize;
	INT MenuRows = g_ScreenInfo[cur_screen].MenuRows;
	INT cols = g_ScreenInfo[cur_screen].MenuCols;
	INT pageitems = (MenuRows-1)*cols-2;
	if (pageitems+1 == MenuSize)
		return; /* only one page */
	g_ScreenInfo[cur_screen].MenuPage++;
	if (g_ScreenInfo[cur_screen].MenuPage > (MenuSize-1)/pageitems)
		g_ScreenInfo[cur_screen].MenuPage = 0;
        menu_dirty = 1;
}
/*==================================================================
 * adjust_menu_height() - Change height of menu on person screen
 * Created: 1999/03, Perry Rapp
 * Joined repository: 2001/01/28, Perry Rapp
 *================================================================*/
void
adjust_menu_height (INT delta)
{
	INT min=4, max=10;
	if (g_ScreenInfo[cur_screen].MenuCols == 1)
	{
		min = 5;
		max = 14;
	}
	g_ScreenInfo[cur_screen].MenuRows += delta;
	if (g_ScreenInfo[cur_screen].MenuRows<min)
		g_ScreenInfo[cur_screen].MenuRows=min;
	else if (g_ScreenInfo[cur_screen].MenuRows>max)
		g_ScreenInfo[cur_screen].MenuRows=max;
	menu_dirty = 1;
}
/*=========================================
 * calculate_screen_lines -- How many lines above menu?
 * Created: 1999/03, Perry Rapp
 * Joined repository: 2001/01/28, Perry Rapp
 *=======================================*/
static INT
calculate_screen_lines (INT screen)
{
	INT menu = g_ScreenInfo[screen].MenuRows;
	INT lines;
	if (!menu_enabled) menu = EMPTY_MENU;
	lines = LINESTOTAL-OVERHEAD_MENU-menu;
	return lines;
}
/*===============================================
 * vmprintf -- send error, info, or status message out
 * legacy
 *=============================================*/
static void
vmprintf (STRING fmt, va_list args)
{
	vsnprintf(showing, sizeof(showing), fmt, args);
	if (strlen(showing)>60) {
		showing[60] = 0;
		strcat(showing, "...");
	}
	display_status(showing);
}
/*===============================================
 * display_status -- put string in status line
 * Created: 2001/11/11, Perry Rapp
 * TO DO - how do we limit its length ? Yet another copy ?
 *=============================================*/
static void
display_status (STRING text)
{
	UIWINDOW *uiwin = main_win;
	WINDOW *win = uiw_win(uiwin);
	INT row;
	wmove(win, row = ll_lines-2, 2);
	if (cur_screen != LIST_SCREEN) {
		wclrtoeol(win);
		mvwaddch(win, row, ll_cols-1, ACS_VLINE);
	} else
		mvwaddstr(win, row, 2, empstr);
	wmove(win, row, 2);
	mvwaddstr(win, row, 2, text);
	now_showing_status = TRUE;
	place_cursor();
	wrefresh(win);
}
/*=========================================
 * msg_error -- handle error message
 * delegates to msg_impl
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
void
msg_error (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	msg_impl(fmt, args, -1);
	va_end(args);
}
/*=========================================
 * msg_info -- handle regular messages
 * delegates to msg_impl
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
void
msg_info (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	msg_impl(fmt, args, 0);
	va_end(args);
}
/*=========================================
 * msg_status -- handle transitory/status messages
 * delegates to msg_impl
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
void
msg_status (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	msg_impl(fmt, args, 1);
	va_end(args);
}
/*=========================================
 * msg_impl -- handle all messages
 * fmt,args:  printf style varargs from client
 * level:     -1=error,0=info,1=status
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
void
msg_impl (STRING fmt, va_list args, INT level)
{
	char buffer[250];
	STRING ptr;
	unsigned int width = MAINWIN_WIDTH-5;
	/* prefix errors & infos with * and space respectively */
	switch(level) {
		case -1:
			buffer[0] = '*';
			ptr = &buffer[1];
			break;
		case 0:
			buffer[0] = ' ';
			ptr = &buffer[1];
			break;
		default:
			ptr = buffer;
			break;
	}
	vsnprintf(ptr, sizeof(buffer), fmt, args);
	/* first add it to msg list */
	if (level<1) {
		if (!msg_list)
			msg_list = create_list();
		enqueue_list(msg_list, strsave(buffer));
		if (!viewing_msgs && !msg_flag &&
			(length_list(msg_list)>1 || strlen(buffer)>width)) {
			msg_flag = TRUE;
		}
		
	}
	/* now put it to status area if appropriate */
	if (strlen(buffer)>width) {
		buffer[width-3]=0;
		strcat(buffer, "...");
	}
	display_status(buffer);
}
/*=========================================
 * begin_action -- prepare to process users choice
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
static
void begin_action (void)
{
	clear_msgs();
}
/*=========================================
 * end_action -- finished processing users choice
 *  show error list if appropriate
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
static
void end_action (void)
{
	if (msg_flag) {
		STRING * strngs = (STRING *)stdalloc(length_list(msg_list)*sizeof(STRING));
		INT i=0;
		FORLIST(msg_list, el)
			strngs[i++] = el;
		ENDLIST
		viewing_msgs = TRUE; /* suppress msg generation */
		view_array(errlist, length_list(msg_list), strngs);
		viewing_msgs = FALSE;
		stdfree(strngs);
		clear_msgs();
	}
}
/*=========================================
 * clear_msgs -- delete msg list
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
static void
clear_msgs (void)
{
	if (msg_list) {
		free_string_list(msg_list);
		msg_list = 0;
	}
	msg_flag = FALSE;
}
/*=========================================
 * lock_status_msg -- temporarily hold status message
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
void
lock_status_msg (BOOLEAN lock)
{
	suppress_std_msg = lock;
}
/*=====================================
 * repaint_add_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_add_menu (UIWINDOW * uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	BOX(win, 0, 0);
	mvwaddstr(win, row++, 2, mn_add_ttl);
	mvwaddstr(win, row++, 4, mn_add_indi);
	mvwaddstr(win, row++, 4, mn_add_fam);
	mvwaddstr(win, row++, 4, mn_add_chil);
	mvwaddstr(win, row++, 4, mn_add_spou);
	mvwaddstr(win, row++, 4, mn_ret);
}
/*=====================================
 * repaint_delete_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_delete_menu (UIWINDOW * uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	BOX(win, 0, 0);
	mvwaddstr(win, row++, 2, mn_del_ttl);
	mvwaddstr(win, row++, 4, mn_del_chil);
	mvwaddstr(win, row++, 4, mn_del_spou);
	mvwaddstr(win, row++, 4, mn_del_indi);
	mvwaddstr(win, row++, 4, mn_del_fam);
	mvwaddstr(win, row++, 4, mn_ret);
}
/*=====================================
 * repaint_scan_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_scan_menu (UIWINDOW * uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	BOX(win, 0, 0);
	mvwaddstr(win, row++, 2, mn_sca_ttl);
	mvwaddstr(win, row++, 4, mn_sca_nmfu);
	mvwaddstr(win, row++, 4, mn_sca_nmfr);
	mvwaddstr(win, row++, 4, mn_sca_refn);
	mvwaddstr(win, row++, 4, mn_ret);
}
/*=====================================
 * repaint_cset_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_cset_menu (UIWINDOW * uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, row++, 2, mn_csttl);
	disp_codeset(uiwin, row++, 4, mn_csintcs, int_codeset);
	disp_locale(uiwin, row++, 4, mn_csdsploc);
	disp_trans_table_choice(uiwin, row++, 4, mn_cstsort, MSORT);
#ifdef NOTYET
	disp_trans_table_choice(uiwin, row++, 4, mn_cspref, MPREF);
	disp_trans_table_choice(uiwin, row++, 4, mn_cschar, MCHAR);
	disp_trans_table_choice(uiwin, row++, 4, mn_cslcas, MLCAS);
	disp_trans_table_choice(uiwin, row++, 4, mn_csucas, MUCAS);
#endif
#ifdef HAVE_SETLOCALE
	mvwaddstr(win, row++, 4, mn_csndloc);
#endif
	mvwaddstr(win, row++, 4, mn_csrpt);
	mvwaddstr(win, row++, 4, mn_cstt);
	mvwaddstr(win, row++, 4, mn_ret);
}
/*=====================================
 * repaint_rpc_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_rpc_menu (UIWINDOW * uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, row++, 2, mn_csrpttl);
	disp_locale(uiwin, row++, 4, mn_csrptloc);
#ifdef HAVE_SETLOCALE
	mvwaddstr(win, row++, 4, mn_csnrloc);
#endif
	disp_trans_table_choice(uiwin, row++, 4, mn_tt_inrp, MINRP);
	mvwaddstr(win, row++, 4, mn_ret);
}
/*=====================================
 * repaint_tt_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_tt_menu (UIWINDOW * uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	mvwaddstr(win, row++, 2, mn_tt_ttl);
/*TO DO*/
}
/*=====================================
 * repaint_trans_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_trans_menu (UIWINDOW * uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	char line[120];
	INT mylen = sizeof(line);
	STRING ptr = line;
	werase(win);
	BOX(win, 0, 0);
	row = 1;
	mvwaddstr(win, row++, 2, mn_tt_ttl);
	mvwaddstr(win, row++, 4, mn_tt_edit);
	mvwaddstr(win, row++, 4, mn_tt_load);
	mvwaddstr(win, row++, 4, mn_tt_save);
	mvwaddstr(win, row++, 4, mn_tt_exp);
	mvwaddstr(win, row++, 4, mn_tt_imp);
	ptr[0] = 0;
/*	llstrcatn(&ptr, mn_tt_dir, &mylen); */
/*	llstrcatn(&ptr, lloptions.llttexport, &mylen); */
	mvwaddstr(win, row++, 4, line);
	mvwaddstr(win, row++, 4, mn_ret);
}
/*=====================================
 * repaint_utils_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_utils_menu (UIWINDOW * uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	BOX(win, 0, 0);
	mvwaddstr(win, row++, 2, mn_uttl);
	mvwaddstr(win, row++, 4, mn_utsave);
	mvwaddstr(win, row++, 4, mn_utread);
	mvwaddstr(win, row++, 4, mn_utkey);
	mvwaddstr(win, row++, 4, mn_utkpers);
	mvwaddstr(win, row++, 4, mn_utdbstat);
	mvwaddstr(win, row++, 4, mn_utmemsta);
	mvwaddstr(win, row++, 4, mn_utplaces);
	mvwaddstr(win, row++, 4, mn_utusropt);
	mvwaddstr(win, row++, 4, mn_quit);
}
/*=====================================
 * repaint_extra_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_extra_menu (UIWINDOW * uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	BOX(win, 0, 0);
	mvwaddstr(win, row++, 2, mn_xttl);
	mvwaddstr(win, row++, 4, mn_xxbsour);
	mvwaddstr(win, row++, 4, mn_xxbeven);
	mvwaddstr(win, row++, 4, mn_xxbothr);
	mvwaddstr(win, row++, 4, mn_xxasour);
	mvwaddstr(win, row++, 4, mn_xxesour);
	mvwaddstr(win, row++, 4, mn_xxaeven);
	mvwaddstr(win, row++, 4, mn_xxeeven);
	mvwaddstr(win, row++, 4, mn_xxaothr);
	mvwaddstr(win, row++, 4, mn_xxeothr);
	mvwaddstr(win, row++, 4, mn_quit);
}
/*============================
 * activate_uiwin -- 
 * Created: 2001/11/24, Perry Rapp
 *==========================*/
static void
activate_uiwin (UIWINDOW * uiwin)
{
	WINDOW * win = uiw_win(uiwin);
	/* hook current as parent */
	uiw_parent(uiwin) = active_uiwin;
	/* refresh current (in case it was obscured by stdout */
	if (active_uiwin)
		wrefresh(uiw_win(active_uiwin));
	/* switch to new & refresh */
	active_uiwin = uiwin;
	touchwin(win);
	wrefresh(win);
}
/*============================
 * deactivate_uiwin -- Remove currently active
 *  and pop to its parent (if it has one)
 * Created: 2001/11/24, Perry Rapp
 *==========================*/
static void
deactivate_uiwin (void)
{
	active_uiwin = uiw_parent(active_uiwin);
	if (active_uiwin) {
		WINDOW * win = uiw_win(active_uiwin);
		touchwin(win);
		wrefresh(win);
	}
}
