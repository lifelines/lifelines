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

#include "llinesi.h"

#define LINESREQ 24
#define COLSREQ  80
#define MAXVIEWABLE 30
#define NEWWIN(r,c)   newwin(r,c,(LINES - (r))/2,(COLS - (c))/2)
#define SUBWIN(w,r,c) subwin(w,r,c,(LINES - (r))/2,(COLS - (c))/2)
#ifdef BSD
#	define BOX(w,r,c) box(w,ACS_VLINE,ACS_HLINE)
#else
#	define BOX(w,r,c) box(w,r,c)
#endif

WINDOW *choose_win();
extern BOOLEAN alldone, progrunning;
extern STRING version, betaversion, empstr, empstr71, readpath;
extern STRING abverr, uoperr;
STRING mtitle = (STRING) "LifeLines %s - Genealogical Database and Programming System";
STRING cright = (STRING) "Copyright(c) 1991 to 1996, by T. T. Wetmore IV";
STRING plschs = (STRING) "Please choose an operation:";
BOOLEAN stdout_vis = FALSE;

static INDISEQ indiseq_list_interact(WINDOW *win, STRING ttl, INDISEQ seq);
static void add_menu (void);
static void create_windows (void);
static void del_menu (void);
static void extra_menu (void);
static void init_all_windows (void);
static void trans_menu (void);
static void utils_menu (void);
static void win_list_init (void);
static void shw_list (WINDOW *win, INDISEQ seq, INT len0, INT top, INT cur);
static void place_std_msg (void);
static void clearw (void);
static void place_cursor (void);
static INT interact (WINDOW *win, STRING str);
static INT list_interact(WINDOW *win, STRING ttl, INT len, STRING *strings);
static INT indiseq_interact (WINDOW *win, STRING ttl, INDISEQ seq);

INT ll_lines = LINESREQ; /* update to be number of lines in screen */
INT ll_cols = COLSREQ;	 /* number of columns in screen used by LifeLines */

INT cur_screen = 0;

WINDOW *main_win = NULL;
WINDOW *stdout_win, *stdout_box_win;
WINDOW *debug_win, *debug_box_win;
WINDOW *ask_win, *ask_msg_win;
WINDOW *choose_from_list_win;
WINDOW *add_menu_win, *del_menu_win;
WINDOW *utils_menu_win, *trans_menu_win;
WINDOW *extra_menu_win;

INT BAND;

/* the following values are increased if ll_lines > LINESREQ */
int TANDEM_LINES = 6;		/* number of lines of tandem info */
int PER_LINES = 11;		/* number of lines of person info */
int FAM_LINES = 13;		/* number of lines of family info */
int PED_LINES = 15;		/* number of lines of pedigree */
int LIST_LINES = 6;		/* number of lines of person info in list */
int AUX_LINES = 15;		/* number of lines in aux window */
int VIEWABLE = 10;		/* can be increased up to MAXVIEWABLE */

int winx=0, winy=0; /* user specified window size */

static char showing[150];
static BOOLEAN now_showing = FALSE;

/* forward refs */
void win_list_init (void);

/* in show.c */
void show_person (NODE pers,	/* person */
		  INT row,	/* start row */
		  INT hgt);	/* avail rows */
void show_long_family (NODE fam,
		       INT row,
		       INT hgt);
void show_short_family (NODE fam,
			INT row,
			INT hgt);
void show_pedigree (NODE indi);
void show_aux_display (NODE node,
		       INT row,
		       INT hgt);
void show_list (INDISEQ seq,
		INT top,
		INT cur,
		INT mark);
/* export.c */
BOOLEAN archive_in_file ();
/* import.c */
BOOLEAN import_from_file ();
/* in miscutls.c */
void key_util (void);
void who_is_he_she (void);
void show_database_stats (void);
/* in newrecs.c */
BOOLEAN add_source (void);
void edit_source (NODE node);
BOOLEAN add_event (void);
void edit_event (NODE node);
BOOLEAN add_other (void);
void edit_other (NODE node);

/*============================
 * init_screen -- Init screens
 *==========================*/
void
init_screen (void)
{
	int extralines;
	if (winx) {
		ll_lines = winy;
		ll_cols = winx;
		if (ll_cols > COLS || ll_lines > LINES) {
			endwin();
			fprintf(stderr, "The requested window size (%d,%d) is too large for your terminal (%d,%d).",
				ll_cols, ll_lines, COLS, LINES);
			exit(1);
		}
		if (ll_cols < COLSREQ || ll_lines < LINESREQ) {
			endwin();
			fprintf(stderr, "The requested window size (%d,%d) is too small for LifeLines (%d,%d).",
				ll_cols, ll_lines, COLSREQ, LINESREQ);
			exit(1);
		}
	}
	else {
		ll_lines = LINES;	/* use all available lines */
		ll_cols = COLSREQ;	/* only use this many columns ??? */

		if (COLS < COLSREQ || LINES < LINESREQ) {
			endwin();
			fprintf(stderr, "Your terminal display (%d,%d) is too small for LifeLines (%d,%d).",
				COLS, LINES, COLSREQ, LINESREQ);
			exit(1);
		}
	}

	extralines = ll_lines - LINESREQ;
	if(extralines > 0) {
	    TANDEM_LINES += (extralines / 2);
	    PER_LINES += extralines;
	    FAM_LINES += extralines;
	    LIST_LINES += extralines;
	    if(extralines >= 16) PED_LINES += 16; /* one more generation */
	    VIEWABLE += extralines;
	    if(VIEWABLE > MAXVIEWABLE) VIEWABLE = MAXVIEWABLE;
	}
	BAND = 25;	/* width of columns of menu (3) */
	create_windows();
	init_all_windows();
}
/*=======================================
 * paint_main_screen -- Paint main screen
 *=====================================*/
void
paint_main_screen(void)
{
	WINDOW *win = main_win;
	INT row;
#ifdef BETA
	char buffer[100];
#endif
	werase(win);
	BOX(win, 0, 0);
	show_horz_line(win, 4, 0, COLSREQ);
	show_horz_line(win, ll_lines-3, 0, COLSREQ);
	wmove(win, 1, 2);
#ifdef BETA
	sprintf(buffer, "%s%s", version, betaversion);
	wprintw(win, mtitle, buffer);
#else
	wprintw(win, mtitle, version);
#endif
	mvwaddstr(win, 2, 4, cright);
	mvwprintw(win, 3, 4, "Current Database - %s", readpath);
	if (readonly)
		wprintw(win, " (read only)");
	row = 5;
	mvwaddstr(win, row++, 2, plschs);
	mvwaddstr(win, row++, 4, "b  Browse the persons in the database");
	mvwaddstr(win, row++, 4, "a  Add information to the database");
	mvwaddstr(win, row++, 4, "d  Delete information from the database");
	mvwaddstr(win, row++, 4, "r  Generate reports from the database");
	mvwaddstr(win, row++, 4, "t  Modify character translation tables");
	mvwaddstr(win, row++, 4, "u  Miscellaneous utilities");
	mvwaddstr(win, row++, 4, "x  Handle source, event and other records");
	mvwaddstr(win, row++, 4, "q  Quit");
}
/*================================================
 * paint_one_per_screen -- Paint one person screen
 *==============================================*/
void
paint_one_per_screen (void)
{
	WINDOW *win = main_win;
	INT row, col;
	werase(win);
	BOX(win, 0, 0);
	show_horz_line(win, ll_lines-12, 0, COLSREQ);
	show_horz_line(win, ll_lines-3,  0, COLSREQ);
	row = ll_lines - 11;
	mvwaddstr(win, row, 2, plschs);
	mvwaddstr(win, row++, 2+strlen(plschs)+3,
		 "[Advanced View: A, Tandem Browse: C,F,G,M,S,U]");
	col = 3;
	mvwaddstr(win, row++, col, "e  Edit the person");
	mvwaddstr(win, row++, col, "f  Browse to father(s)");
	mvwaddstr(win, row++, col, "m  Browse to mother(s)");
	mvwaddstr(win, row++, col, "s  Browse to spouse/s");
	mvwaddstr(win, row++, col, "c  Browse to children");
	mvwaddstr(win, row++, col, "o  Browse to older sib");
	mvwaddstr(win, row++, col, "y  Browse to younger sib");
	row = ll_lines - 10; col = 3 + BAND;
	mvwaddstr(win, row++, col, "g  Browse to family");
	mvwaddstr(win, row++, col, "u  Browse to parents");
	mvwaddstr(win, row++, col, "b  Browse to persons");
	mvwaddstr(win, row++, col, "h  Add as spouse");
	mvwaddstr(win, row++, col, "i  Add as child");
	mvwaddstr(win, row++, col, "r  Remove as spouse");
	mvwaddstr(win, row++, col, "d  Remove as child");
	row = ll_lines - 10; col = 3 + 2*BAND;
	mvwaddstr(win, row++, col, "p  Show pedigree");
	mvwaddstr(win, row++, col, "n  Create new person");
	mvwaddstr(win, row++, col, "a  Create new family");
	mvwaddstr(win, row++, col, "x  Swap two families");
	mvwaddstr(win, row++, col, "t  Enter tandem mode");
	mvwaddstr(win, row++, col, "z  Browse to person");
	mvwaddstr(win, row++, col, "q  Return to main menu");
}
/*================================================
 * paint_one_fam_screen -- Paint one family screen
 *==============================================*/
void
paint_one_fam_screen (void)
{
	WINDOW *win = main_win;
	INT row, col;
	werase(win);
	BOX(win, 0, 0);
	show_horz_line(win, ll_lines-10, 0, COLSREQ);
	show_horz_line(win, ll_lines-3, 0, COLSREQ);
	row = ll_lines - 9;
	mvwaddstr(win, row, 2, plschs);
	mvwaddstr(win, row++, 2+strlen(plschs)+6,
		 "[Advanced View: A, Tandem Browse: C,F,M]");
	col = 3;
	mvwaddstr(win, row++, col, "e  Edit the family");
	mvwaddstr(win, row++, col, "f  Browse to father");
	mvwaddstr(win, row++, col, "m  Browse to mother");
	mvwaddstr(win, row++, col, "c  Browse to children");
	mvwaddstr(win, row++, col, "n  Create new person");
	row = ll_lines - 8; col = 3 + BAND;
	mvwaddstr(win, row++, col, "s  Add spouse to family");
	mvwaddstr(win, row++, col, "a  Add child to family");
	mvwaddstr(win, row++, col, "r  Remove spouse from");
	mvwaddstr(win, row++, col, "d  Remove child from");
	mvwaddstr(win, row++, col, "x  Swap two children");
	row = ll_lines - 8; col = 3 + 2*BAND;
	mvwaddstr(win, row++, col, "t  Enter family tandem");
	mvwaddstr(win, row++, col, "b  Browse to persons");
	mvwaddstr(win, row++, col, "z  Browse to person");
	mvwaddstr(win, row++, col, "q  Return to main menu");
}
/*================================================
 * paint_two_per_screen -- Paint two person screen
 *==============================================*/
void
paint_two_per_screen (void)
{
	WINDOW *win = main_win;
	INT row, col;
	werase(win);
	BOX(win, 0, 0);
	show_horz_line(win, TANDEM_LINES+1, 0, COLSREQ);
	show_horz_line(win, 2*TANDEM_LINES+2, 0, COLSREQ);
	show_horz_line(win, ll_lines-3, 0, COLSREQ);
	mvwaddstr(win, 2*TANDEM_LINES+3, 2, plschs);
	row = 2*TANDEM_LINES+4; col = 3;
	mvwaddstr(win, row++, col, "e  Edit top person");
	mvwaddstr(win, row++, col, "t  Browse to top");
	mvwaddstr(win, row++, col, "f  Browse top father");
	mvwaddstr(win, row++, col, "m  Browse top mother");
	row = 2*TANDEM_LINES+4; col = 3 + BAND;
	mvwaddstr(win, row++, col, "s  Browse top spouse/s");
	mvwaddstr(win, row++, col, "c  Browse top children");
	mvwaddstr(win, row++, col, "b  Browse to persons");
	mvwaddstr(win, row++, col, "d  Copy top to bottom");
	row = 2*TANDEM_LINES+4; col = 3 + 2*BAND;
	mvwaddstr(win, row++, col, "a  Add family");
	mvwaddstr(win, row++, col, "j  Merge bottom to top");
	mvwaddstr(win, row++, col, "x  Switch top/bottom");
	mvwaddstr(win, row++, col, "q  Return to main menu");
}
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
	show_horz_line(win, TANDEM_LINES+1, 0, COLSREQ);
	show_horz_line(win, 2*TANDEM_LINES+2, 0, COLSREQ);
	show_horz_line(win, ll_lines-3, 0, COLSREQ);
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
/*==========================================
 * paint_ped_screen -- Paint pedigree screen
 *========================================*/
void
paint_ped_screen (void)
{
	WINDOW *win = main_win;
	INT row, col;
	werase(win);
	BOX(win, 0, 0);
	show_horz_line(win, PED_LINES+1, 0, COLSREQ);
	show_horz_line(win, ll_lines-3, 0, COLSREQ);
	mvwaddstr(win, PED_LINES+2, 2, plschs);
	row = PED_LINES+3; col = 3;
	mvwaddstr(win, row++, col, "e  Edit the person");
	mvwaddstr(win, row++, col, "i  Browse to person");
	mvwaddstr(win, row++, col, "f  Browse to father");
	row = PED_LINES+3; col = 3 + BAND;
	mvwaddstr(win, row++, col, "m  Browse to mother");
	mvwaddstr(win, row++, col, "s  Browse to spouse/s");
	mvwaddstr(win, row++, col, "c  Browse to children");
	row = PED_LINES+3; col = 3 + 2*BAND;
	mvwaddstr(win, row++, col, "g  Browse to family");
	mvwaddstr(win, row++, col, "b  Browse to persons");
	mvwaddstr(win, row++, col, "q  Return to main menu");
}
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
	show_horz_line(win, LIST_LINES+1, 0, COLSREQ);
	show_horz_line(win, ll_lines-3, 0, COLSREQ);
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
/*===========================================
 * paint_aux_screen -- Paint auxillary screen
 *=========================================*/
void
paint_aux_screen (void)
{
	WINDOW *win = main_win;
	INT row, col;
	werase(win);
	BOX(win, 0, 0);
	show_horz_line(win, AUX_LINES+1, 0, COLSREQ);
	show_horz_line(win, ll_lines-3, 0, COLSREQ);
	mvwaddstr(win, AUX_LINES+2, 2, plschs);
	row = AUX_LINES+3; col = 3;
	mvwaddstr(win, row++, col, "e  Edit the record");
	mvwaddstr(win, row++, col, "i  Browse to record");
	mvwaddstr(win, row++, col, "x  Not implemented");
	row = AUX_LINES+3; col = 3 + BAND;
	mvwaddstr(win, row++, col, "x  Not implemented");
	mvwaddstr(win, row++, col, "x  Not implemented");
	mvwaddstr(win, row++, col, "x  Not implemented");
	row = AUX_LINES+3; col = 3 + 2*BAND;
	mvwaddstr(win, row++, col, "x  Not implemented");
	mvwaddstr(win, row++, col, "x  Not implemented");
	mvwaddstr(win, row++, col, "q  Return to main menu");
}
/*==========================================
 * create_windows -- Create and init windows
 *========================================*/
void
create_windows (void)
{
	INT col;
	stdout_box_win = NEWWIN(ll_lines-4, COLSREQ-4);
	stdout_win = SUBWIN(stdout_box_win, ll_lines-6, COLSREQ-6);
	scrollok(stdout_win, TRUE);
	col = COLS/4;
	debug_box_win = newwin(8, COLSREQ-col-2, 1, col);
	debug_win = subwin(debug_box_win, 6, COLSREQ-col-4, 2, col+1);
	scrollok(debug_win, TRUE);

 	main_win = NEWWIN(ll_lines, COLSREQ);
	add_menu_win = NEWWIN(8, 66);
	del_menu_win = NEWWIN(7, 66);
	trans_menu_win = NEWWIN(10,66);
	utils_menu_win = NEWWIN(12, 66);
	extra_menu_win = NEWWIN(13,66);
	ask_win = NEWWIN(4, 73);
	ask_msg_win = NEWWIN(5, 73);
	choose_from_list_win = NEWWIN(15, 73);
	win_list_init();

	BOX(add_menu_win, 0, 0);
	BOX(del_menu_win, 0, 0);
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
void
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
	c = interact(main_win, "badrtuxq");
	place_std_msg();
	wrefresh(main_win);
	switch (c) {
	case 'b': browse(NULL); break;
	case 'a': add_menu(); break;
	case 'd': del_menu(); break;
	case 'r': interp_main(); break;
	case 't': trans_menu(); break;
	case 'u': utils_menu(); break;
	case 'x': extra_menu(); break;
	case 'q': alldone = TRUE; break;
	}
}
/*=========================================
 * indi_browse -- Handle indi_browse screen
 *=======================================*/
INT
indi_browse (NODE indi)
{
	if (cur_screen != ONE_PER_SCREEN) paint_one_per_screen();
	show_person(indi, 1, PER_LINES);
	display_screen(ONE_PER_SCREEN);
	return interact(main_win,
		"efmscoygubhirdpnaxtzqACFGMSU$#123456789+-");
}
/*=======================================
 * fam_browse -- Handle fam_browse screen
 *=====================================*/
INT
fam_browse (NODE fam)
{
	if (cur_screen != ONE_FAM_SCREEN) paint_one_fam_screen();
	show_long_family(fam, 1, FAM_LINES);
	display_screen(ONE_FAM_SCREEN);
	return interact(main_win, 
		"efmcnsardxtbzqABCFM$#123456789+-");
}
/*=============================================
 * tandem_browse -- Handle tandem_browse screen
 *===========================================*/
INT
tandem_browse (NODE indi1,
                   NODE indi2)
{
	if (cur_screen != TWO_PER_SCREEN) paint_two_per_screen();
	show_person(indi1, 1, TANDEM_LINES);
	show_person(indi2, TANDEM_LINES+2, TANDEM_LINES);
	display_screen(TWO_PER_SCREEN);
	return interact(main_win, "etfmscbdajxq");
}
/*=============================================
 * twofam_browse -- Handle twofam_browse screen
 *===========================================*/
INT
twofam_browse (NODE fam1, NODE fam2)
{
	if (cur_screen != TWO_FAM_SCREEN) paint_two_fam_screen();
	show_short_family(fam1, 1, TANDEM_LINES);
	show_short_family(fam2, TANDEM_LINES+2, TANDEM_LINES);
	display_screen(TWO_FAM_SCREEN);
	return interact(main_win, "etbfmxjq");
}
/*=======================================
 * ped_browse -- Handle ped_browse screen
 *=====================================*/
INT
ped_browse (NODE indi)
{
	if (cur_screen != PED_SCREEN) paint_ped_screen();
	show_pedigree(indi);
	display_screen(PED_SCREEN);
	return interact(main_win,
		"eifmscgb$+-q");
}
#ifdef UNUSED
/*=======================================
 * aux_browse -- Handle aux_browse screen
 *=====================================*/
INT
aux_browse (NODE node)
{
	if (cur_screen != AUX_SCREEN) paint_aux_screen();
	show_aux_display(node, 1, 11);
	display_screen(AUX_SCREEN);
	return interact(main_win, "eq");
}
#endif
/*=========================================
 * list_browse -- Handle list_browse screen
 *=======================================*/
INT
list_browse (INDISEQ seq,
             INT top,
             INT cur,
             INT mark)
{
	if (cur_screen != LIST_SCREEN) paint_list_screen();
	show_list(seq, top, cur, mark);
	display_screen(LIST_SCREEN);
	return interact(main_win, "jkeimdtbanxq");
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
	return interact(win, ptrn);
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
	rv = interact(win, ptrn);
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
	WINDOW *win = choose_win(no);
	INT rv;
	werase(win);
	BOX(win, 0, 0);
	wrefresh(win);
	rv = list_interact(win, ttl, no, pstrngs);
	touchwin(main_win);
	wrefresh(main_win);
	return rv;
}
/*=============================================================
 * choose_one_from_indiseq -- User chooses person from sequence
 *===========================================================*/
INT
choose_one_from_indiseq (STRING ttl,
                         INDISEQ seq)
{
	WINDOW *win;
	INT rv, len;
	ASSERT(seq);
	len = length_indiseq(seq);
	win = choose_win(len);
	werase(win);
	BOX(win, 0, 0);
	wrefresh(win);
	rv = indiseq_interact(win, ttl, seq);
	if (stdout_vis) {
		touchwin(stdout_win);
		wrefresh(stdout_win);
	} else {
		touchwin(main_win);
		wrefresh(main_win);
	}
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
	win = choose_win(len);
	werase(win);
	BOX(win, 0, 0);
	wrefresh(win);
	seq = indiseq_list_interact(win, ttl, seq);
	touchwin(main_win);
	wrefresh(main_win);
	return seq;
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
	code = interact(add_menu_win, "pfcsq");
	touchwin(main_win);
	wrefresh(main_win);
	switch (code) {
	case 'p':
		node = add_indi_by_edit();
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
	code = interact(del_menu_win, "csiq");
	touchwin(main_win);
	wrefresh(main_win);
	switch (code) {
	case 'c': remove_child(NULL, NULL, FALSE); break;
	case 's': remove_spouse(NULL, NULL, FALSE); break;
	case 'i': delete_indi(NULL, TRUE); break;
	case 'q': break;
	}
}
/*======================================
 * trans_menu -- Handle translation menu
 *====================================*/
void
trans_menu (void)
{
	INT code;
	touchwin(trans_menu_win);
	wmove(trans_menu_win, 1, 47);
	wrefresh(trans_menu_win);
	code = interact(trans_menu_win, "emixdrq");
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
void
utils_menu (void)
{
	INT code;
	touchwin(utils_menu_win);
	wmove(utils_menu_win, 1, 39);
	wrefresh(utils_menu_win);
	code = interact(utils_menu_win, "srkidmeoq");
	touchwin(main_win);
	wrefresh(main_win);
	switch (code) {
	case 's': archive_in_file(); break;
	case 'r': import_from_file(); break;
	case 'k': key_util(); break;
	case 'i': who_is_he_she(); break;
	case 'd': show_database_stats(); break;
	case 'm': cache_stats(); break;
	case 'e': edit_valtab("VPLAC", &placabbvs, ':', abverr); break;
	case 'o': edit_valtab("VUOPT", &useropts, '=', uoperr); break;
	case 'q': break;
	}
}
/*================================
 * extra_menu -- Handle extra menu
 *==============================*/
void
extra_menu (void)
{
	INT code;
	touchwin(extra_menu_win);
	wmove(extra_menu_win, 1, 39);
	wrefresh(extra_menu_win);
	code = interact(extra_menu_win, "sex123456q");
	touchwin(main_win);
	wrefresh(main_win);
	switch (code) {
	case 's': browse_sources(); break;
	case 'e': browse_events(); break;
	case 'x': browse_others(); break;
	case '1': add_source(); break;
	case '2': edit_source(NULL); break;
	case '3': add_event(); break;
	case '4': edit_event(NULL); break;
	case '5': add_other(); break;
	case '6': edit_other(NULL); break;
	case 'q': break;
	}
}
/*===============================
 * interact -- Interact with user
 *=============================*/
INT
interact (WINDOW *win,
          STRING str)
{
	INT c, i, n = strlen(str);
	while (TRUE) {
		crmode();
		c = wgetch(win);
		if (c == EOF) c = 'q';
		nocrmode();
		now_showing = FALSE;
		if (!progrunning)
			place_std_msg();
		for (i = 0; i < n; i++) {
			if (c == str[i]) return c;
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
		list_wins[i] = NEWWIN(i+6, 73);
	}
}
/*=========================================
 * choose_win -- Choose window to hold list
 *=======================================*/
WINDOW *
choose_win (INT len)        /* length */
{
	WINDOW *win = list_wins[VIEWABLE-1];
	if (len <= VIEWABLE) win = list_wins[len-1];
	return win;
}
/*=====================================================
 * indiseq_interact -- Interact with user over sequence
 *===================================================*/
INT
indiseq_interact (WINDOW *win,
                  STRING ttl,
                  INDISEQ seq)
{
	INT top, cur, len, row;
	top = cur = 0;
	len = length_indiseq(seq);
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, 1, 1, ttl);
	row = len > VIEWABLE ? VIEWABLE + 2 : len + 2;
	show_horz_line(win, row++, 0, 73);
	mvwaddstr(win, row, 2, "Commands:   j Move down     k Move up    i Select     q Quit");
	while (TRUE) {
		shw_list(win, seq, len, top, cur);
		wmove(win, row, 11);
		wrefresh(win);
		switch (interact(win, "jkiq")) {
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

	top = cur = 0;
	len = len0 = length_indiseq(seq);
	werase(win);
	BOX(win, 0, 0);
	mvwaddstr(win, 1, 1, ttl);
	row = len0 > VIEWABLE ? VIEWABLE + 2 : len0 + 2;
	show_horz_line(win, row++, 0, 73);
	mvwaddstr(win, row, 2, "Commands:  j Move down   k Move up  d Delete   i Select   q Quit");
	while (TRUE) {
		shw_list(win, seq, len0, top, cur);
		wmove(win, row, 11);
		wrefresh(win);
		switch (interact(win, "jkdiq")) {
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
 *===================================================*/
void
shw_list (WINDOW *win,
          INDISEQ seq,
          INT len0,
          INT top,
          INT cur)
{
	INT i, j, row = 2, len = length_indiseq(seq);
	j = len0 > VIEWABLE ? VIEWABLE : len0;
	for (i = 0; i < j; i++)
		mvwaddstr(win, row++, 1, empstr71);
	row = 2;
	for (i = top, j = 0; j < VIEWABLE && i < len; i++, j++) {
		if (i == cur) mvwaddch(win, row, 3, '>');
		mvwaddstr(win, row, 4, sprn(IData(seq)[i]));
		row++;
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
		switch (interact(win, "jkiq")) {
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
void
vmprintf (STRING fmt, va_list args)
{
	INT row;
	wmove(main_win, row = ll_lines-2, 2);
	if (cur_screen != LIST_SCREEN) {
		wclrtoeol(main_win);
		mvwaddch(main_win, row, COLSREQ-1, ACS_VLINE);
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
 *=============================================*/
void
mprintf_status (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vmprintf(fmt, args);
	va_end(args);
}
#ifdef OBSOLETE
/*===============================================
 * Other: mprintf -- Call as mprintf(fmt, arg, arg, ...)
 *=============================================*/
mprintf (fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
STRING fmt;
INT arg1, arg2, arg3, arg4, arg5, arg6, arg7;
{
	INT row;
	wmove(main_win, row = ll_lines-2, 2);
	if (cur_screen != LIST_SCREEN) {
		wclrtoeol(main_win);
		mvwaddch(main_win, row, COLSREQ-1, ACS_VLINE);
	} else
		mvwaddstr(main_win, row, 2, empstr);
	wmove(main_win, row, 2);
	sprintf(showing, fmt, arg1, arg2, arg3, arg4, arg5,
	    arg6, arg7);
	mvwaddstr(main_win, row, 2, showing);
	now_showing = TRUE;
	place_cursor();
	wrefresh(main_win);
}
#endif
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
	switch (cur_screen) {
	case MAIN_SCREEN:
		return (STRING) "LifeLines -- Main Menu";
	case ONE_PER_SCREEN:
		return (STRING) "LifeLines -- Person Browse Screen";
	case ONE_FAM_SCREEN:
		return (STRING) "LifeLines -- Family Browse Screen";
	case PED_SCREEN:
		return (STRING) "LifeLines -- Pedigree Browse Screen";
	case AUX_SCREEN:
		return (STRING) "LifeLines -- Auxilliary Browse Screen";
	case TWO_PER_SCREEN:
		return (STRING) "LifeLines -- Tandem Browse Screen";
	case TWO_FAM_SCREEN:
		return (STRING) "LifeLines -- Two Family Browse Screen";
	case LIST_SCREEN:
		return (STRING) "LifeLines -- List Browse Screen";
	default:
		return (STRING) "";
	}
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
		mvwaddch(main_win, row, COLSREQ-1, ACS_VLINE);
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
	INT row, col = 30;
	switch (cur_screen) {
	case MAIN_SCREEN:    row = 5;        break;
	case ONE_PER_SCREEN: row = ll_lines-11; break;
	case ONE_FAM_SCREEN: row = ll_lines-9;  break;
	case PED_SCREEN:     row = PED_LINES+2;       break;
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
#ifdef WIN32
	w32system(editstr);
#else
	system(editstr);
#endif
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
               INt row,
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
