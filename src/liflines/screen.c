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
 *   2.3.4 - 24 Jun 93    2.3.5 - 29 Sep 93
 *   2.3.6 - 01 Jan 94    3.0.0 - 06 Oct 94
 *   3.0.2 - 25 Mar 95    3.0.3 - 17 Jan 96
 *===========================================================*/

#include "sys_inc.h"
#include <stdarg.h>
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

#include "llinesi.h"

#define LINESREQ 24
#define COLSREQ  80
#define MAXVIEWABLE 30
#define OVERHEAD_MENU 5
INT LISTWIN_WIDTH=0;
INT MAINWIN_WIDTH=0;
static INT list_detail_lines = 0;

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
WINDOW *main_win = NULL;
WINDOW *stdout_win, *stdout_box_win;
WINDOW *debug_win, *debug_box_win;
WINDOW *ask_win, *ask_msg_win;
WINDOW *choose_from_list_win;
WINDOW *add_menu_win, *del_menu_win;
WINDOW *scan_menu_win;
WINDOW *utils_menu_win, *trans_menu_win;
WINDOW *extra_menu_win;

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN alldone, progrunning;
extern STRING empstr, empstr71, readpath;
extern STRING abverr, uoperr;
extern STRING mtitle, cright, plschs;
extern STRING mn_unkcmd;
extern STRING askynq, askynyn, askyny;

/*********************************************
 * local function prototypes
 *********************************************/

static INDISEQ indiseq_list_interact(WINDOW *win, STRING ttl, INDISEQ seq);
static void add_menu (void);
static void create_windows (void);
static void del_menu (void);
static void extra_menu (void);
static void init_all_windows (void);
static INT update_menu(INT screen);
static void show_indi_mode(NODE indi, INT mode, INT row, INT hgt);
static void show_fam_mode(NODE fam, INT mode, INT row, INT hgt, INT width);
static void scan_menu (void);
static void trans_menu (void);
static void utils_menu (void);
static void win_list_init (void);
static void shw_list(WINDOW *win, INDISEQ seq, INT len0, INT top, INT cur, INT *scroll);
static void place_std_msg (void);
static void clearw (void);
static void place_cursor (void);
static INT interact(WINDOW *win, STRING str, INT screen);
static INT list_interact(WINDOW *win, STRING ttl, INT len, STRING *strings);
static void vmprintf (STRING fmt, va_list args);
static INT indiseq_interact (WINDOW *win, STRING ttl, INDISEQ seq);
static WINDOW *choose_win(INT desiredlen, INT *actuallen);
static void output_menu(WINDOW *win, INT screen);
static INT calculate_screen_lines(INT screen);


static INT menu_enabled = 1;
static INT menu_dirty = 0;


INT BAND;

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

static char showing[150];
static BOOLEAN now_showing = FALSE;


/*============================
 * init_screen -- Init screens
 * Created: c. 2000/11, Perry Rapp
 *==========================*/
int
init_screen (void)
{
	int extralines;
	if (winx) {
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
	init_all_windows();
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
 * paint_main_screen -- Paint main screen
 *=====================================*/
void
paint_main_screen(void)
{
	WINDOW *win = main_win;
	INT row;

	werase(win);
	BOX(win, 0, 0);
	show_horz_line(win, 4, 0, ll_cols);
	show_horz_line(win, ll_lines-3, 0, ll_cols);
	wmove(win, 1, 2);
	wprintw(win, mtitle, get_lifelines_version(ll_cols-4));
	mvwaddstr(win, 2, 4, cright);
	mvwprintw(win, 3, 4, "Current Database - %s", readpath);
	if (readonly)
		wprintw(win, " (read only)");
	row = 5;
	mvwaddstr(win, row++, 2, plschs);
	mvwaddstr(win, row++, 4, "b  Browse the persons in the database");
	mvwaddstr(win, row++, 4, "s  Search database");
	mvwaddstr(win, row++, 4, "a  Add information to the database");
	mvwaddstr(win, row++, 4, "d  Delete information from the database");
	mvwaddstr(win, row++, 4, "p  Pick a report from list and run");
	mvwaddstr(win, row++, 4, "r  Generate report by entering report name");
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
	WINDOW *win = main_win;
	werase(win);
	BOX(win, 0, 0);
	show_horz_line(win, ll_lines-3,  0, ll_cols);
	if (!menu_enabled)
		return;
	output_menu(win, screen);
}
#ifdef UNUSED
/*================================================
 * paint_two_fam_screen -- Paint two family screen
 *==============================================*/
void
paint_two_fam_screen (void)
{
	WINDOW *win = main_win;
	INT row, col;
	werase(win);
	BOX(win, 0, 0);
	show_horz_line(win, TANDEM_LINES+1, 0, ll_cols);
	show_horz_line(win, 2*TANDEM_LINES+2, 0, ll_cols);
	show_horz_line(win, ll_lines-3, 0, ll_cols);
	mvwaddstr(win, 2*TANDEM_LINES+3, 2, plschs);
	row = 2*TANDEM_LINES+4; col = 3;
	mvwaddstr(win, row++, col, "e  Edit top family");
	mvwaddstr(win, row++, col, "t  Browse to top");
	mvwaddstr(win, row++, col, "b  Browse to bottom");
	row = 2*TANDEM_LINES+4; col = 3 + BAND;
	mvwaddstr(win, row++, col, "f  Browse to fathers");
	mvwaddstr(win, row++, col, "m  Browse to mothers");
	mvwaddstr(win, row++, col, "x  Switch top/bottom");
	row = 2*TANDEM_LINES+4; col = 3 + 2*BAND;
	mvwaddstr(win, row++, col, "j  Merge bottom to top");
	mvwaddstr(win, row++, col, "q  Return to main menu");
}
#endif
/*==============================================
 * paint_list_screen -- Paint list browse screen
 *============================================*/
void
paint_list_screen (void)
{
	WINDOW *win = main_win;
	INT row, col;
	werase(win);
	BOX(win, 0, 0);
	show_horz_line(win, LIST_LINES+1, 0, ll_cols);
	show_horz_line(win, ll_lines-3, 0, ll_cols);
	show_vert_line(win, LIST_LINES+1, 52, 15);
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
	mvwaddstr(win, row++, col, "q  Return to main menu");
}
/*==========================================
 * create_windows -- Create and init windows
 *========================================*/
void
create_windows (void)
{
	INT col;
	stdout_box_win = NEWWIN(ll_lines-4, ll_cols-4);
	stdout_win = SUBWIN(stdout_box_win, ll_lines-6, ll_cols-6);
	scrollok(stdout_win, TRUE);
	col = COLS/4;
	debug_box_win = newwin(8, ll_cols-col-2, 1, col);
	debug_win = subwin(debug_box_win, 6, ll_cols-col-4, 2, col+1);
	scrollok(debug_win, TRUE);

	MAINWIN_WIDTH = ll_cols;
	LISTWIN_WIDTH = 73;
 	main_win = NEWWIN(ll_lines, MAINWIN_WIDTH);
	add_menu_win = NEWWIN(8, 66);
	del_menu_win = NEWWIN(8, 66);
	scan_menu_win = NEWWIN(7,66);
	trans_menu_win = NEWWIN(10,66);
	utils_menu_win = NEWWIN(12, 66);
	extra_menu_win = NEWWIN(13,66);
	ask_win = NEWWIN(4, 73);
	ask_msg_win = NEWWIN(5, 73);
	choose_from_list_win = NEWWIN(15, 73);
	win_list_init();

	BOX(add_menu_win, 0, 0);
	BOX(del_menu_win, 0, 0);
	BOX(scan_menu_win, 0, 0);
	BOX(trans_menu_win, 0, 0);
	BOX(utils_menu_win, 0, 0);
	BOX(extra_menu_win, 0, 0);
	BOX(ask_win, 0, 0);
	BOX(ask_msg_win, 0, 0);
	BOX(debug_box_win, 0, 0);
}
/*=====================================
 * init_all_windows -- Init all windows
 *===================================*/
static void
init_all_windows (void)
{
	WINDOW *win = add_menu_win;
	INT row = 1;
	mvwaddstr(win, row++, 2, "What do you want to add?");
	mvwaddstr(win, row++, 4, "p  Person - add new person to the database");
	mvwaddstr(win, row++, 4,
	    "f  Family - create family record from one or two spouses");
	mvwaddstr(win, row++, 4,
	    "c  Child - add a child to an existing family");
	mvwaddstr(win, row++, 4,
	    "s  Spouse - add a spouse to an existing family");
	mvwaddstr(win, row++, 4, "q  Quit - return to the previous menu");

	win = del_menu_win;
	row = 1;
	mvwaddstr(win, row++, 2, "What do you want to remove?");
	mvwaddstr(win, row++, 4,
	    "c  Child - remove a child from his/her family");
	mvwaddstr(win, row++, 4, "s  Spouse - remove a spouse from a family");
	mvwaddstr(win, row++, 4, "i  Individual - remove a person completely");
	mvwaddstr(win, row++, 4, "f  Family - remove a family completely");
	mvwaddstr(win, row++, 4, "q  Quit - return to the previous menu");

	win = scan_menu_win;
	row = 1;
	mvwaddstr(win, row++, 2, "What scan type?");
	mvwaddstr(win, row++, 4, "f  Full name scan");
	mvwaddstr(win, row++, 4, "n  Name fragment (whitespace-delimited) scan");
	mvwaddstr(win, row++, 4, "r  Refn scan");
	mvwaddstr(win, row++, 4, "q  Quit - return to the previous menu");

	win = trans_menu_win;
	row = 1;
	mvwaddstr(win, row++, 2,
	    "Which character mapping do you want to edit?");
	mvwaddstr(win, row++, 4, "e  Editor to Internal mapping");
	mvwaddstr(win, row++, 4, "m  Internal to Editor mapping");
	mvwaddstr(win, row++, 4, "i  GEDCOM to Internal mapping");
	mvwaddstr(win, row++, 4, "x  Internal to GEDCOM mapping");
	mvwaddstr(win, row++, 4, "d  Internal to Display mapping");
	mvwaddstr(win, row++, 4, "r  Internal to Report mapping");
	mvwaddstr(win, row++, 4, "q  Return to the main menu");

	win = utils_menu_win;
	row = 1;
	mvwaddstr(win, row++, 2, "What utility do you want to perform?");
	mvwaddstr(win, row++, 4, "s  Save the database in a GEDCOM file");
	mvwaddstr(win, row++, 4, "r  Read in data from a GEDCOM file");
	mvwaddstr(win, row++, 4, "k  Find a person's key value");
	mvwaddstr(win, row++, 4, "i  Identify a person from key value");
	mvwaddstr(win, row++, 4, "d  Show database statistics");
	mvwaddstr(win, row++, 4, "m  Show memory statistics");
	mvwaddstr(win, row++, 4, "e  Edit the place abbreviation file");
	mvwaddstr(win, row++, 4, "o  Edit the user options file");
	mvwaddstr(win, row++, 4, "q  Return to the main menu");

	win = extra_menu_win;
	row = 1;
	mvwaddstr(win, row++, 2, "What activity do you want to perform?");
	mvwaddstr(win, row++, 4, "s  Browse source records");
	mvwaddstr(win, row++, 4, "e  Browse event records");
	mvwaddstr(win, row++, 4, "x  Browse other records");
	mvwaddstr(win, row++, 4, "1  Add a source record to the database");
	mvwaddstr(win, row++, 4, "2  Edit source record from the database");
	mvwaddstr(win, row++, 4, "3  Add an event record to the database");
	mvwaddstr(win, row++, 4, "4  Edit event record from the database");
	mvwaddstr(win, row++, 4, "5  Add an other record to the database");
	mvwaddstr(win, row++, 4, "6  Edit other record from the database");
	mvwaddstr(win, row++, 4, "q  Return to the main menu");
}
/*=================================
 * display_screen -- Display screen
 *===============================*/
void
display_screen (INT new_screen)
{
	cur_screen = new_screen;
	if (stdout_vis) {
		llwprintf("\nStrike any key to continue.\n");
		crmode();
		(void) wgetch(stdout_win);
		nocrmode();
	}
	stdout_vis = FALSE;
	if (!now_showing)
		place_std_msg();
	else
		mvwaddstr(main_win, ll_lines-2, 2, showing);
	place_cursor();
	touchwin(main_win);
	wrefresh(main_win);
}
/*=====================================
 * main_menu -- Handle main_menu screen
 *===================================*/
void
main_menu (void)
{
	INT c;
	if (cur_screen != MAIN_SCREEN) paint_main_screen();
	display_screen(MAIN_SCREEN);
	/* place_std_msg(); */ /*POSS*/
	c = interact(main_win, "bsadprtuxq", -1);
	place_std_msg();
	wrefresh(main_win);
	switch (c) {
	case 'b': browse(NULL); break;
	case 's': scan_menu(); break;
	case 'a': add_menu(); break;
	case 'd': del_menu(); break;
	case 'p': interp_main(TRUE); break;
	case 'r': interp_main(FALSE); break;
	case 't': trans_menu(); break;
	case 'u': utils_menu(); break;
	case 'x': extra_menu(); break;
	case 'q': alldone = TRUE; break;
	}
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
 *=======================================*/
static void
show_indi_mode (NODE indi, INT mode, INT row, INT hgt)
{
	if (mode=='g')
		show_gedcom_main(indi, row, hgt);
	else if (mode=='a')
		show_ancestors(indi, row, hgt);
	else if (mode=='d')
		show_descendants(indi, row, hgt);
	else
		show_person_main(indi, row, hgt);
}
/*=========================================
 * show_fam_mode -- Show indi according to mode
 *=======================================*/
static void
show_fam_mode (NODE fam, INT mode, INT row, INT hgt, INT width)
{
	if (mode=='g')
		show_gedcom_main(fam, row, hgt);
	else
		show_long_family(fam, row, hgt, width);
}
/*=========================================
 * indi_browse -- Handle indi_browse screen
 *=======================================*/
INT
indi_browse (NODE indi, INT mode)
{
	INT screen = ONE_PER_SCREEN;
	INT lines = update_menu(screen);
	show_indi_mode(indi, mode, 1, lines);
	display_screen(screen);
	return interact(main_win, NULL, screen);
}
/*=======================================
 * fam_browse -- Handle fam_browse screen
 *=====================================*/
INT
fam_browse (NODE fam, INT mode)
{
	INT width = MAINWIN_WIDTH;
	INT screen = ONE_FAM_SCREEN;
	INT lines = update_menu(screen);
	show_fam_mode(fam, mode, 1, lines, width);
	display_screen(screen);
	return interact(main_win, NULL, screen);
}
/*====================================
 * show_tandem_line -- Display horizontal line between top & bottom
 * PR 1999/03
 *==================================*/
static void
show_tandem_line (WINDOW * win, INT row)
{
	show_horz_line(win, row, 0, ll_cols);
}
/*=============================================
 * twoindi_browse -- Handle tandem_browse screen
 *===========================================*/
INT
twoindi_browse (NODE indi1, NODE indi2, INT mode)
{
	INT screen = TWO_PER_SCREEN;
	INT lines = update_menu(screen);
	INT lines1,lines2;
	lines--; /* for tandem line */
	lines2 = lines/2;
	lines1 = lines - lines2;

	show_indi_mode(indi1, mode, 1, lines1);
	show_tandem_line(main_win, lines1+1);
	switch_scrolls();
	show_indi_mode(indi2, mode, lines1+2, lines2);
	switch_scrolls();

	display_screen(screen);
	return interact(main_win, NULL, screen);
/*	return interact(main_win, "etfmscbdajxq", -1);*/
}
/*=============================================
 * twofam_browse -- Handle twofam_browse screen
 *===========================================*/
INT
twofam_browse (NODE fam1, NODE fam2, INT mode)
{
	INT width=MAINWIN_WIDTH;
	INT screen = TWO_FAM_SCREEN;
	INT lines = update_menu(screen);
	INT lines1,lines2;
	lines--; /* for tandem line */
	lines2 = lines/2;
	lines1 = lines - lines2;

	show_fam_mode(fam1, mode, 1, lines1, width);
	show_tandem_line(main_win, lines1+1);
	switch_scrolls();
	show_fam_mode(fam2, mode, lines1+2, lines2, width);
	switch_scrolls();

	display_screen(screen);
	return interact(main_win, NULL, screen);
/*	return interact(main_win, "etbfmxjq", -1);*/
}
/*=======================================
 * aux_browse -- Handle aux_browse screen
 * Implemented: 2001/01/27, Perry Rapp
 *=====================================*/
INT
aux_browse (NODE node)
{
	INT screen = AUX_SCREEN;
	INT lines = update_menu(screen);
	show_aux_display(node, lines);
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
 * ask_for_lldb -- Ask user for lifelines database directory
 *====================================*/
STRING ask_for_lldb (STRING ttl, STRING prmpt, STRING basedir)
{
	/* This could have a list of existing ones like askprogram.c */
	return ask_for_string(ttl, prmpt);
}
/*======================================
 * ask_for_output_filename -- Ask user for filename to which to write
 *====================================*/
STRING
ask_for_output_filename (STRING ttl, STRING path, STRING prmpt)
{
	return ask_for_string(ttl, prmpt);
}
/*======================================
 * ask_for_input_filename -- Ask user for filename from which to read
 *====================================*/
STRING
ask_for_input_filename (STRING ttl, STRING path, STRING prmpt)
{
	return ask_for_string(ttl, prmpt);
}
/*======================================
 * ask_for_string -- Ask user for string
 *====================================*/
STRING
ask_for_string (STRING ttl,
                STRING prmpt)
{
	WINDOW *win = ask_win;
	STRING rv, p;
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, 1, 1, ttl);
	mvwaddstr(win, 2, 1, prmpt);
	wrefresh(win);
	rv = get_answer(win, prmpt);
	if (!rv) return (STRING) "";
	p = rv;
	while (chartype(*p) == WHITE)
		p++;
	striptrail(p);
	win = stdout_vis ? stdout_win : main_win;
	touchwin(win);
	wrefresh(win);
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
	WINDOW *win = ask_win;
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, 1, 2, ttl);
	mvwaddstr(win, 2, 2, prmpt);
	wrefresh(win);
	return interact(win, ptrn, -1);
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
	WINDOW *win = ask_msg_win;
	INT rv;
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, 1, 2, msg);
	mvwaddstr(win, 2, 2, ttl);
	mvwaddstr(win, 3, 2, prmpt);
	wrefresh(win);
	rv = interact(win, ptrn, -1);
	return rv;
}
/*============================================
 * choose_from_list -- Choose from string list
 *==========================================*/
INT
choose_from_list (STRING ttl,
                  INT no,
                  STRING *pstrngs)
{
	WINDOW *win = choose_win(no, NULL);
	INT rv;
	werase(win);
	BOX(win, 0, 0);
	wrefresh(win);
	rv = list_interact(win, ttl, no, pstrngs);
	win = stdout_vis ? stdout_win : main_win;
	touchwin(win);
	wrefresh(win);
	return rv;
}
/*=============================================================
 * choose_one_from_indiseq -- User chooses person from sequence
 * Resize rewrite: c. 2000/12, Perry Rapp
 *===========================================================*/
INT
choose_one_from_indiseq (STRING ttl, INDISEQ seq)
{
	WINDOW *win;
	INT rv, len, actlen, minlen, asklen;
	INT top, cur, row, done;
	INT scroll;
	char fulltitle[128];
	char buffer[31];
	char * ptr;
	INT titlen;
	ASSERT(seq);
	len = length_indiseq(seq);
	if (len<50)
		preprint_indiseq(seq);
		
	scroll=0;
	/*
	prevent shrinking because we're not redrawing
	window beneath us
	*/
	minlen = len;
	top = cur = 0;
resize_win:
	asklen = len+list_detail_lines;
	if (asklen < minlen)
		asklen = minlen;
	win = choose_win(asklen, &actlen);
	if (actlen > minlen)
		minlen = actlen;
	/* check in case we pushed current offscreen */
	if (cur-scroll>actlen-1-list_detail_lines)
		cur=actlen-1-list_detail_lines+scroll;
	werase(win);
	BOX(win, 0, 0);
	wrefresh(win);
	len = length_indiseq(seq);
	werase(win);
	BOX(win, 0, 0);
	row = len + list_detail_lines + 2;
	if (row > VIEWABLE + 2)
		row = VIEWABLE + 2;
	show_horz_line(win, row++, 0, 73);
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
		shw_list(win, seq, len, top, cur, &scroll);
		wmove(win, row, 11);
		wrefresh(win);
		switch (interact(win, "jkiq()[]", -1)) {
		case 'j':
			if (cur >= len - 1) break;
			cur++;
			if (cur >= top + VIEWABLE - list_detail_lines)
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
			if (list_detail_lines) {
				list_detail_lines--;
				goto resize_win;
			}
			break;
		case ']':
			if (list_detail_lines < 8) {
				list_detail_lines++;
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
	
	win = stdout_vis ? stdout_win : main_win;
	touchwin(win);
	wrefresh(win);

	return rv;
}
/*==========================================================
 * choose_list_from_indiseq -- User chooses subsequence from
 *   person sequence
 *========================================================*/
INDISEQ
choose_list_from_indiseq (STRING ttl,
                          INDISEQ seq)
{
	WINDOW *win;
	INT len;
	ASSERT(seq);
	len = length_indiseq(seq);
	if (len<50)
		preprint_indiseq(seq);
	win = choose_win(len+list_detail_lines, NULL);
	werase(win);
	BOX(win, 0, 0);
	wrefresh(win);
	seq = indiseq_list_interact(win, ttl, seq);
	win = stdout_vis ? stdout_win : main_win;
	touchwin(win);
	wrefresh(win);
	return seq;
}
/*==============================
 * scan_menu -- Handle scan menu
 * Created: c. 2000/12, Perry Rapp
 *============================*/
void
scan_menu (void)
{
	NOD0 nod0;
	INT code;
	while (1) {
		touchwin(scan_menu_win);
		wmove(scan_menu_win, 1, 27);
		wrefresh(scan_menu_win);
		code = interact(scan_menu_win, "fnrq", -1);
		touchwin(main_win);
		wrefresh(main_win);
		switch (code) {
		case 'f':
			nod0 = full_name_scan();
			if (nod0) {
				browse(nztop(nod0));
				return;
			}
			break;
		case 'n':
			nod0 = name_fragment_scan();
			if (nod0) {
				browse(nztop(nod0));
				return;
			}
			break;
		case 'r':
			nod0 = refn_scan();
			if (nod0) {
				switch(nztype(nod0)) {
				case 'I': browse(nztop(nod0)); return;
				/* TO DO - families */
				case 'S':  browse_source(nod0); return;
				case 'E':  browse_event(nod0); return;
				case 'X':  browse_other(nod0); return;
				}
			}
			break;
		case 'q': return;
		}
	}
}
/*============================
 * add_menu -- Handle add menu
 *==========================*/
void
add_menu (void)
{
	NODE node;
	INT code;
	touchwin(add_menu_win);
	wmove(add_menu_win, 1, 27);
	wrefresh(add_menu_win);
	code = interact(add_menu_win, "pfcsq", -1);
	touchwin(main_win);
	wrefresh(main_win);
	switch (code) {
	case 'p':
		node = nztop(add_indi_by_edit());
		if (node) browse(node);
		break;
	case 'f': add_family(NULL, NULL, NULL); break;
	case 'c': add_child(NULL, NULL); break;
	case 's': add_spouse(NULL, NULL, TRUE); break;
	case 'q': break;
	}
}
/*===============================
 * del_menu -- Handle delete menu
 *=============================*/
void
del_menu (void)
{
	INT code;
	touchwin(del_menu_win);
	wmove(del_menu_win, 1, 30);
	wrefresh(del_menu_win);
	code = interact(del_menu_win, "csifq", -1);
	touchwin(main_win);
	wrefresh(main_win);
	switch (code) {
	case 'c': choose_and_remove_child(NULL, NULL, FALSE); break;
	case 's': choose_and_remove_spouse(NULL, NULL, FALSE); break;
	case 'i': delete_indi(NULL, TRUE); break;
	case 'f': choose_and_delete_family(); break;
	case 'q': break;
	}
}
/*======================================
 * trans_menu -- Handle translation menu
 *====================================*/
static void
trans_menu (void)
{
	INT code;
	touchwin(trans_menu_win);
	wmove(trans_menu_win, 1, 47);
	wrefresh(trans_menu_win);
	code = interact(trans_menu_win, "emixdrq", -1);
	touchwin(main_win);
	wrefresh(main_win);
	switch (code) {
	case 'e': edit_mapping(MEDIN); break;
	case 'm': edit_mapping(MINED); break;
	case 'i': edit_mapping(MGDIN); break;
	case 'x': edit_mapping(MINGD); break;
	case 'd': edit_mapping(MINDS); break;
	case 'r': edit_mapping(MINRP); break;
	case 'q': break;
	}
}
/*====================================
 * utils_menu -- Handle utilities menu
 *==================================*/
static void
utils_menu (void)
{
	INT code;
	touchwin(utils_menu_win);
	wmove(utils_menu_win, 1, 39);
	wrefresh(utils_menu_win);
	code = interact(utils_menu_win, "srkidmeoq", -1);
	touchwin(main_win);
	wrefresh(main_win);
	switch (code) {
	case 's': archive_in_file(); break;
	case 'r': import_from_file(); break;
	case 'k': key_util(); break;
	case 'i': who_is_he_she(); break;
	case 'd': show_database_stats(); break;
	case 'm': display_cache_stats(); break;
	case 'e': edit_valtab("VPLAC", &placabbvs, ':', abverr); break;
	case 'o': edit_valtab("VUOPT", &useropts, '=', uoperr); break;
	case 'q': break;
	}
}
/*================================
 * extra_menu -- Handle extra menu
 *==============================*/
static void
extra_menu (void)
{
	INT code;
	while (1) {
		touchwin(extra_menu_win);
		wmove(extra_menu_win, 1, 39);
		wrefresh(extra_menu_win);
		code = interact(extra_menu_win, "sex123456q", -1);
		touchwin(main_win);
		wrefresh(main_win);
		switch (code) {
		case 's': browse_sources(); break;
		case 'e': browse_events(); break;
		case 'x': browse_others(); break;
		case '1': add_source(); return;
		case '2': edit_source(NULL); return;
		case '3': add_event(); return;
		case '4': edit_event(NULL); return;
		case '5': add_other(); return;
		case '6': edit_other(NULL); return;
		case 'q': return;
		}
	}
}
/*===============================
 * interact -- Interact with user
 *=============================*/
static INT
interact (WINDOW *win, STRING str, INT screen)
{
	char buffer[4]; /* 3 char cmds max */
	INT offset=0;
	INT cmdnum;
	INT c, i, n = str ? strlen(str) : 0;
	while (TRUE) {
		crmode();
		c = wgetch(win);
		if (c == EOF) c = 'q';
		nocrmode();
		now_showing = FALSE;
		if (!progrunning)
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
 *==========================================*/
STRING
get_answer (WINDOW *win,
            STRING prmpt)
{
	static unsigned char lcl[100];

#ifndef USEBSDMVGETSTR
	echo();
	mvwgetstr(win, 2, strlen(prmpt) + 2, lcl);
	noecho();
#else
	bsd_mvwgetstr(win, 2, strlen(prmpt) + 2, lcl);
#endif
	return lcl;
}
/*===========================================================
 * win_list_init -- Create list of windows of increasing size
 *=========================================================*/
static WINDOW *list_wins[MAXVIEWABLE];
void
win_list_init (void)
{
	INT i;
	for (i = 0; i < VIEWABLE; i++) {
		list_wins[i] = NEWWIN(i+6, LISTWIN_WIDTH);
	}
}
/*=========================================
 * choose_win -- Choose window to hold list
 *=======================================*/
static WINDOW *
choose_win (INT desiredlen, INT *actuallen)
{
	WINDOW *win = list_wins[VIEWABLE-1];
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
indiseq_interact (WINDOW *win,
                  STRING ttl,
                  INDISEQ seq)
{
	INT top, cur, len, row;
	INT scroll=0;
	top = cur = 0;
	len = length_indiseq(seq);
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, 1, 1, ttl);
	row = len > VIEWABLE ? VIEWABLE + 2 : len + 2;
	show_horz_line(win, row++, 0, 73);
	mvwaddstr(win, row, 2, "Commands:   j Move down     k Move up    i Select     q Quit");
	while (TRUE) {
		shw_list(win, seq, len, top, cur, &scroll);
		wmove(win, row, 11);
		wrefresh(win);
		switch (interact(win, "jkiq", -1)) {
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
indiseq_list_interact (WINDOW *win,
                       STRING ttl,
                       INDISEQ seq)
{
	INT top, cur, len, len0, row;
	INT scroll;

	top = cur = 0;
	len = len0 = length_indiseq(seq);
	scroll = 0;
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, 1, 1, ttl);
	row = len0 + list_detail_lines + 2;
	if (row > VIEWABLE + 2)
		row = VIEWABLE + 2;
	show_horz_line(win, row++, 0, 73);
	mvwaddstr(win, row, 2, "Commands:  j Move down   k Move up  d Delete   i Select   q Quit");
	while (TRUE) {
		shw_list(win, seq, len0, top, cur, &scroll);
		wmove(win, row, 11);
		wrefresh(win);
		switch (interact(win, "jkdiq", -1)) {
		case 'j':
			if (cur >= len - 1) break;
			cur++;
			if (cur >= top + VIEWABLE - list_detail_lines)
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
				remove_indiseq(seq, FALSE);
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
shw_list (WINDOW *win,
          INDISEQ seq,
          INT len0,
          INT top,
          INT cur,
          INT *scroll)
{
	INT i, j, row, nrows, len, numdet;
	char buffer[60];
	len = length_indiseq(seq);
	numdet = list_detail_lines;
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
				print_indiseq_element(seq, i, buffer, sizeof(buffer));
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
			show_person(win, indi, 2, numdet-1, LISTWIN_WIDTH, scroll);
		}
	}
}
/*================================================================
 * shw_list_of_strings -- Show string list in list interact window
 *==============================================================*/
void
shw_list_of_strings (WINDOW *win,
                     STRING *strings,
                     INT len,
                     INT top,
                     INT cur)
{
	INT i, j, row = len > VIEWABLE ? VIEWABLE + 1 : len + 1;
	for (i = 2; i <= row; i++)
		mvwaddstr(win, i, 1, empstr71);
	row = 2;
	for (i = top, j = 0; j < VIEWABLE && i < len; i++, j++) {
		if (i == cur) mvwaddch(win, row, 3, '>');
		mvwaddstr(win, row, 4, strings[i]);
		row++;
	}
}
/*==============================================
 * list_interact -- Interact with user over list
 *============================================*/
INT
list_interact(WINDOW *win,    /* interaction window */
              STRING ttl,     /* title */
              INT len,        /* list length */
              STRING *strings)/* string list */
{
	INT top = 0, cur = 0, row;
	while (TRUE) {
		werase(win);
		BOX(win, 0, 0);
		mvwaddstr(win, 1, 1, ttl);
		row = len > VIEWABLE ? VIEWABLE + 2 : len + 2;
		show_horz_line(win, row++, 0, 73);
		mvwaddstr(win, row, 2, "Commands:   j Move down     k Move up    i Select     q Quit");
		shw_list_of_strings(win, strings, len, top, cur);
		wrefresh(win);
		switch (interact(win, "jkiq", -1)) {
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
/*===============================================
 * vmprintf -- send error, info, or status message out
 *=============================================*/
static void
vmprintf (STRING fmt, va_list args)
{
	INT row;
	wmove(main_win, row = ll_lines-2, 2);
	if (cur_screen != LIST_SCREEN) {
		wclrtoeol(main_win);
		mvwaddch(main_win, row, ll_cols-1, ACS_VLINE);
	} else
		mvwaddstr(main_win, row, 2, empstr);
	wmove(main_win, row, 2);
	vsprintf(showing, fmt, args);
	mvwaddstr(main_win, row, 2, showing);
	now_showing = TRUE;
	place_cursor();
	wrefresh(main_win);
}
/*===============================================
 * mprintf_error -- Call as mprintf_error(fmt, ...)
 * tell the user something went wrong
 * Created: c. 2000/11, Perry Rapp
 *=============================================*/
void
mprintf_error (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vmprintf(fmt, args);
	va_end(args);
}
/*===============================================
 * mprintf_info -- Call as mprintf_info(fmt, ...)
 * usually displaying results of user's action
 * Created: c. 2000/11, Perry Rapp
 *=============================================*/
void
mprintf_info (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vmprintf(fmt, args);
	va_end(args);
}
/*===============================================
 * mprintf_status -- Call as mprintf_status(fmt, ...)
 * transient status during import/export, eg, counting nodes
 * Created: c. 2000/11, Perry Rapp
 *=============================================*/
void
mprintf_status (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vmprintf(fmt, args);
	va_end(args);
}
/*=======================================
 * message -- Simple interface to mprintf
 *=====================================*/
void
message (STRING s)
{
	mprintf_info("%s", s);
}
/*===================================================
 * message_string -- Return background message string
 *=================================================*/
STRING
message_string (void)
{
	if (!cur_screen) return "";
	return f_ScreenInfo[cur_screen].Title;
}
/*=================================================
 * place_std_msg - Place standard message on screen
 *===============================================*/
void
place_std_msg (void)
{
	STRING str = message_string();
	INT row;
	wmove(main_win, row = ll_lines-2, 2);
	if (cur_screen != LIST_SCREEN) {
		wclrtoeol(main_win);
		mvwaddch(main_win, row, ll_cols-1, ACS_VLINE);
	} else
		mvwaddstr(main_win, row, 2, empstr);
	mvwaddstr(main_win, row, 2, str);
	place_cursor();
}
/*=================================================
 * llvwprintf -- Called as wprintf(fmt, argp)
 *===============================================*/
void
llvwprintf (STRING fmt,
            va_list args)
{
	if (!stdout_vis)
		clearw();
	vwprintw(stdout_win, fmt, args);
	wrefresh(stdout_win);
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
	werase(stdout_win);
	BOX(stdout_box_win, 0, 0);
	wmove(stdout_win, 0, 0);
	stdout_vis = TRUE;
	wrefresh(stdout_box_win);
}
/*=======================================
 * wfield -- Write field in stdout window
 *=====================================*/
void
wfield (INT row,
        INT col,
        STRING str)
{
	if (!stdout_vis) clearw();
	mvwaddstr(stdout_win, row, col, str);
	wrefresh(stdout_win);
}
/*===========================================
 * wpos -- Position to place in stdout window
 *=========================================*/
void
wpos (INT row,
      INT col)
{
	wmove(stdout_win, row, col);
}
/*=======================================
 * show_horz_line -- Draw horizontal line
 *=====================================*/
void
show_horz_line (WINDOW *win,
                INT row,
                INT col,
                INT len)
{
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
show_vert_line (WINDOW *win,
                INT row,
                INT col,
                INT len)
{
#ifndef BSD
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
	wmove(main_win, row, col);
}
/*=============================================
 * dbprintf -- Debug printf(fmt, arg, arg, ...)
 *===========================================*/
void
dbprintf (STRING fmt, ...)
{
	va_list args;
	touchwin(debug_box_win);
	va_start(args, fmt);
	vwprintw(debug_win, fmt, args);
	va_end(args);
	wrefresh(debug_box_win);
	sleep(2);
	touchwin(main_win);
	wrefresh(main_win);
}
/*==================================================
 * do_edit -- Shift to user's screen editor and back
 *================================================*/
void
do_edit (void)
{
	endwin();
	system(editstr);
	touchwin(main_win);
	clearok(curscr, 1);
	wrefresh(curscr);
	noecho();
}
/*==================================================================
 * bsd_mvwgetstr -- Special BSD version of mvwgetstr that does erase
 *   character handling
 *================================================================*/
#ifdef BSD
void
bsd_mvwgetstr (WINDOW *win,
               INT row,
               INT col,
               STRING str)
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
		} else if (c != ers) {
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
output_menu (WINDOW *win, INT screen)
{
	INT row;
	INT icol=0;
	INT col=3;
	INT MenuRows = f_ScreenInfo[screen].MenuRows;
	INT MenuSize = f_ScreenInfo[screen].MenuSize;
	INT Item = 0;
	INT page, pageitems, pages;
	char prompt[128];
	MenuItem ** Menu = f_ScreenInfo[screen].Menu;
	INT OnePageFlag = 0;
	page = f_ScreenInfo[screen].MenuPage;
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
	show_horz_line(win, row++, 0, ll_cols);
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
	INT MenuSize = f_ScreenInfo[cur_screen].MenuSize;
	INT MenuRows = f_ScreenInfo[cur_screen].MenuRows;
	INT cols = f_ScreenInfo[cur_screen].MenuCols;
	INT pageitems = (MenuRows-1)*cols-2;
	if (pageitems+1 == MenuSize)
		return; /* only one page */
	f_ScreenInfo[cur_screen].MenuPage++;
	if (f_ScreenInfo[cur_screen].MenuPage > (MenuSize-1)/pageitems)
		f_ScreenInfo[cur_screen].MenuPage = 0;
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
	if (f_ScreenInfo[cur_screen].MenuCols == 1)
	{
		min = 5;
		max = 14;
	}
	f_ScreenInfo[cur_screen].MenuRows += delta;
	if (f_ScreenInfo[cur_screen].MenuRows<min)
		f_ScreenInfo[cur_screen].MenuRows=min;
	else if (f_ScreenInfo[cur_screen].MenuRows>max)
		f_ScreenInfo[cur_screen].MenuRows=max;
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
	INT menu = f_ScreenInfo[screen].MenuRows;
	INT lines;
	if (!menu_enabled) menu = EMPTY_MENU;
	lines = LINESTOTAL-OVERHEAD_MENU-menu;
	return lines;
}
