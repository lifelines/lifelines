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
/*==============================================================
 * screen.h -- Header file for curses-based screen I/O
 * Copyright (c) 1991-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 20 Aug 93
 *   3.0.0 - 26 Jul 94    3.0.2 - 16 Dec 94
 *============================================================*/

#ifndef _SCREEN_H
#define _SCREEN_H

#include "gedcom.h"

#ifndef _INDISEQ_H
#include "indiseq.h"
#endif

#include "mycurses.h"

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif

#ifndef TRUE
#       define TRUE ((BOOLEAN)1)
#endif
#ifndef FALSE
#       define FALSE ((BOOLEAN)0)
#endif


/* Support for 'broken' curses implementations */
/* That don't define ACS_xxx constants */
#ifndef ACS_TTEE
#   undef ACS_TTEE
#	define ACS_TTEE '+'
#   undef ACS_RTEE
#	define ACS_RTEE '+'
#   undef ACS_LTEE
#	define ACS_LTEE '+'
#   undef ACS_BTEE
#	define ACS_BTEE '+'
#   undef ACS_VLINE 
#	define ACS_VLINE '|'
#   undef ACS_HLINE
#	define ACS_HLINE '-'
#   undef ACS_LLCORNER
#   define ACS_LLCORNER '*'
#   undef ACS_LRCORNER
#   define ACS_LRCORNER '*'
#   undef ACS_ULCORNER
#   define ACS_ULCORNER '*'
#   undef ACS_URCORNER
#   define ACS_URCORNER '*'
#endif

/* box drawing character type */
typedef unsigned long llchtype;

/*=========================================
 * UIWINDOWs -- Main screen, menus and popups
 *=======================================*/
/* wrapper for WINDOW */
typedef struct tag_uiwindow {
	CNSTRING name;       /* non-heap name (for debugging) */
	WINDOW * win;      /* curses window */
	WINDOW * boxwin;   /* surrounding window just for boxing */
	struct tag_uiwindow * parent; /* fixed or dynamic parent */
	struct tag_uiwindow * child;
	BOOLEAN permsub;   /* TRUE if a fixed subwindow */
	BOOLEAN dynamic;   /* TRUE means delete when finished */
	BOOLEAN outdated;  /* for language changes */
	INT rows;
	INT cols;
	INT cur_y;         /* row for input cursor */
	INT cur_x;         /* col for input cursor */
} * UIWINDOW;
#define uiw_win(x)      ((x)->win)
#define uiw_boxwin(x)   ((x)->boxwin)
#define uiw_parent(x)   ((x)->parent)
#define uiw_child(x)    ((x)->child)
#define uiw_permsub(x)  ((x)->permsub)
#define uiw_dynamic(x)  ((x)->dynamic)
#define uiw_rows(x)     ((x)->rows)
#define uiw_cols(x)     ((x)->cols)
#define uiw_cury(x)     ((x)->cur_y)
#define uiw_curx(x)     ((x)->cur_x)

extern INT ll_lines; /* number of lines used by LifeLines (usually LINES) */
extern INT ll_cols;  /* number of columns used by LifeLines (usually COLSREQ) */
extern INT cur_screen;
extern UIWINDOW stdout_win;
extern UIWINDOW main_win;

enum {
        BROWSE_INDI
        , BROWSE_FAM
        , BROWSE_PED
        , BROWSE_TAND
        , BROWSE_QUIT
        , BROWSE_2FAM
        , BROWSE_LIST
        , BROWSE_AUX
        , BROWSE_EVEN
        , BROWSE_SOUR
        , BROWSE_UNK
};

struct tag_menuset;

/*
  Function Prototypes, alphabetical by module
*/

/* loadsave.c */
void load_gedcom(BOOLEAN picklist);
BOOLEAN save_gedcom(void);

/* screen.c */
void activate_uiwin(UIWINDOW uiwin);
void adjust_browse_menu_cols(INT delta);
void adjust_browse_menu_height(INT delta);
INT ask_for_char(STRING, STRING, STRING);
INT ask_for_char_msg(STRING, STRING, STRING, STRING);
BOOLEAN ask_for_db_filename(CNSTRING ttl, CNSTRING prmpt, CNSTRING basedir, STRING buffer, INT buflen);
INT aux_browse(RECORD rec, INT mode, BOOLEAN reuse);
INT choose_one_from_indiseq(STRING, INDISEQ);
void clear_hseg(WINDOW *, INT row, INT x1, INT x2);
void clear_stdout_hseg(INT row, INT x1, INT x2);
void create_newwin2(UIWINDOW * puiw, CNSTRING name, INT rows, INT cols);
void cycle_browse_menu(void);
void deactivate_uiwin_and_touch_all(void);
void display_2fam(RECORD frec1, RECORD frec2, INT mode);
void display_2indi(RECORD irec1, RECORD irec2, INT mode);
void display_fam(RECORD fam, INT mode, BOOLEAN reuse);
void display_indi(RECORD indi, INT mode, BOOLEAN reuse);
void display_screen(INT);
void dbprintf(STRING, ...);
void draw_win_box(WINDOW * win);
llchtype get_gr_ttee(void);
INT get_main_screen_width(void);
int get_uitime(void);
int init_screen(char * errmsg, int errsize);
INT interact_2fam(void);
INT interact_2indi(void);
INT interact_fam(void);
INT interact_indi(void);
INT interact_popup(UIWINDOW uiwin, STRING str);
INT list_browse(INDISEQ seq, INT top, INT *cur, INT mark);
void lock_status_msg(BOOLEAN lock);
void main_menu(void);
STRING message_string (void);
void paint_main_screen(void);
void paint_two_fam_screen(void);
void set_screen_graphical(BOOLEAN graphical);
void show_horz_line(UIWINDOW, INT, INT, INT);
void show_indi(UIWINDOW uiwin, RECORD indi, INT mode, LLRECT
	, INT * scroll, BOOLEAN reuse);
void show_indi_vitals(UIWINDOW uiwin, RECORD irec, LLRECT, INT *scroll, BOOLEAN reuse);
BOOLEAN show_record(UIWINDOW uiwin, STRING key, INT mode, LLRECT
	, INT * scroll, BOOLEAN reuse);
void show_vert_line(UIWINDOW, INT, INT, INT);
void term_screen(void);
void toggle_browse_menu(void);
INT twofam_browse(NODE, NODE, INT mode);
INT twoindi_browse(NODE, NODE, INT mode);
void uierase(UIWINDOW uiwin);
void wfield(INT, INT, STRING);
void wipe_window_rect(UIWINDOW uiwin, LLRECT rect);
void wpos (INT, INT);


/* show.c (curses specific) */
extern struct tag_rfmt disp_long_rfmt, disp_shrt_rfmt;
extern INT Scroll1;
void display_cache_stats(void);
void init_show_module(void);
void show_ancestors (UIWINDOW uiwin, RECORD irec, LLRECT
	, INT * scroll, BOOLEAN reuse);
void show_aux(UIWINDOW uiwin, RECORD rec, INT mode, LLRECT
	, INT * scroll, BOOLEAN reuse);
void show_big_list(INDISEQ, INT, INT, INT);
void show_childnumbers(void);
void show_descendants(UIWINDOW uiwin, RECORD rec, LLRECT
	, INT * scroll, BOOLEAN reuse);
void show_fam_vitals (UIWINDOW uiwin, RECORD frec, INT row, INT hgt
	, INT width, INT *scroll, BOOLEAN reuse);
void show_gedcom (UIWINDOW uiwin, RECORD rec, INT gdvw, LLRECT
	, INT * scroll, BOOLEAN reuse);
void show_reset_scroll(void);
void show_sour_display(NODE, INT, INT);
void show_scroll(INT delta);
void show_scroll2(INT delta);
void switch_scrolls(void);
void term_show_module(void);

#ifndef _FEEDBACK_H
#include "feedback.h"
#endif /* _FEEDBACK_H */

#endif /* _SCREEN_H */
