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
 * listui.c -- display code for popup list and list browse screen
 * Copyright(c) 2006
 *===========================================================*/

#include <time.h>
#include "llstdlib.h"
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "table.h"
#include "liflines.h"
#include "arch.h"
#include "lloptions.h"
#include "interp.h"
#include "llinesi.h"
#include "menuitem.h"
#include "screen.h"
#include "cscurses.h"
#include "zstr.h"
#include "cache.h"
#ifdef WIN32_ICONV_SHIM
#include "iconvshim.h"
#endif
#include "codesets.h"
#include "charprops.h"
#include "listui.h"



static INT LISTWIN_WIDTH=0;

 /*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSmn_quit;

/*********************************************
 * local types
 *********************************************/

/*
  Data for lists display
  Both popup & full-screen lists have 3 rectangles:
    details, list items, and menu
  But they are not in the same place
*/
typedef struct listdisp_s
{
	UIWINDOW uiwin;
	struct tag_llrect rectList;
	struct tag_llrect rectDetails;
	struct tag_llrect rectMenu;
	INT details; /* #rows of detail info */
	INT details_minhgt;
	INT details_beginhgt; /* increase from 0 goes to this */
	INT details_maxhgt;
	INT details_scroll; /* scroll offset in detail area */
	INT cur; /* current item selected, 0-based */
	INT listlen; /* #items total */
	INT top; /* current item at top of display, 0-based */
	INT mode; /* record display mode */
} listdisp;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void activate_popup_list_uiwin (listdisp * ld);
static void display_string(UIWINDOW uiwin, LLRECT rect, STRING text);
static INT handle_list_cmds(listdisp * ld, INT code);
static BOOLEAN handle_popup_list_resize(listdisp * ld, INT code);
static void print_list_title(char * buffer, INT len, const listdisp * ld, STRING ttl);
static void shw_array_of_strings(STRING *strings, listdisp *ld
	, DETAILFNC detfnc, void * param);
static void shw_popup_list(INDISEQ seq, listdisp * ld);
static void shw_recordlist_details(INDISEQ seq, listdisp * ld);
static void shw_recordlist_list(INDISEQ seq, listdisp * ld);

/*********************************************
 * local variables
 *********************************************/

/* the following values are default (larger screens get more) */
static int LIST_LINES_DEF = 6;       /* number of lines of person info in list */
static int POPUP_LINES_DEF = 17;     /* max lines in popup list */
/* working values */
static int LIST_LINES=0;
static int POPUP_LINES=0;

/*==============================================
 * listui_init_windows -- Initialize anything dependent on screen size
 *============================================*/
void
listui_init_windows (INT extralines)
{
	/* initialize list window heights to default */
	LIST_LINES = LIST_LINES_DEF;
	POPUP_LINES = POPUP_LINES_DEF;
	/* increase for larger screens */
	if(extralines > 0) {
		LIST_LINES = LIST_LINES_DEF + extralines;
		POPUP_LINES = POPUP_LINES_DEF + extralines;
	}

	LISTWIN_WIDTH = ll_cols-7;
}
/*==============================================
 * array_interact -- Interact with user over list
 *  ttl:        [IN]  title
 *  len:        [IN]  number of choices
 *  strings:    [IN]  array of choices
 *  selectable: [IN]  FALSE for view-only
 *  detfnc:     [IN]  callback for details about items
 *  param:      [IN]  opaque type for callback
 * returns 0-based index chosen, or -1 if cancelled
 *============================================*/
INT
array_interact (STRING ttl, INT len, STRING *strings
	, BOOLEAN selectable, DETAILFNC detfnc, void * param)
{
	WINDOW *win=0;
	INT row, done;
	char fulltitle[128];
	STRING responses = len<10 ? "jkiq123456789[]()$^" : "jkiq[]()$^";
	STRING promptline=0;
	listdisp ld; /* structure used in resizable list displays */

	if (selectable)
		promptline = _("Commands:   j Move down     k Move up    i Select     q Quit");
	else
		promptline = _("Commands:   j Move down     k Move up    q Quit");

	memset(&ld, 0, sizeof(ld));
	ld.listlen = len;
	ld.mode = 'n'; /* irrelevant for array list */

resize_win: /* we come back here if we resize the window */
	activate_popup_list_uiwin(&ld);
	win = uiw_win(ld.uiwin);
	uierase(ld.uiwin);
	draw_win_box(win);
	row = ld.rectMenu.top-1;
	show_horz_line(ld.uiwin, row++, 0, uiw_cols(ld.uiwin));
	mvccwaddstr(win, row, ld.rectMenu.left, promptline);
	done = FALSE;
	while (!done) {
		INT code=0, ret=0;
		print_list_title(fulltitle, sizeof(fulltitle), &ld, _(ttl));
		mvccwaddstr(win, 1, 1, fulltitle);
		shw_array_of_strings(strings, &ld, detfnc, param);
		wrefresh(win);
		code = interact_popup(ld.uiwin, responses);
		if (handle_list_cmds(&ld, code))
			continue;
		if (handle_popup_list_resize(&ld, code)) {
			deactivate_uiwin_and_touch_all();
			/* we're going to repick window & activate */
			goto resize_win;
		}
		if (ret == 0) { /* not handled yet */
			switch(code) {
			case 'i': /* select current item */
			case CMD_KY_ENTER:
				if (selectable) {
					done=TRUE;
				}
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
				if (len < 10 && selectable && code - '1' < len) {
					done=TRUE;
					ld.cur = code - '1';
				}
				break;
			case 'q':
				done=TRUE;
				ld.cur = -1; /* ld.cur == -1 means cancelled */
			}
		}
	}
	deactivate_uiwin_and_touch_all();
	return ld.cur;
}
/*=============================================================
 * choose_one_or_list_from_indiseq -- 
 * Implements the two choose_xxx_from_indiseq
 *  @ttl:   [IN]  title/caption for choice list
 *  @seq:   [IN]  list from which to choose
 *  @multi: [IN]  if true, selecting a sublist
 * returns index of selected (or -1 for quit)
 * Rewritten to allow dynamic resizing (so user can
 *  resize detail area, ie, the [] functions), 2000/12, Perry Rapp
 * Localizes ttl
 *===========================================================*/
INT
choose_one_or_list_from_indiseq (STRING ttl, INDISEQ seq, BOOLEAN multi)
{
	WINDOW *win=0;
	INT row, done;
	char fulltitle[128];
	INT elemwidth;
	listdisp ld; /* structure used in resizable list displays */
	STRING menu, choices;
	BOOLEAN first=TRUE;

	ASSERT(seq);

	calc_indiseq_names(seq); /* we certainly need the names */
	
	memset(&ld, 0, sizeof(ld));
	ld.listlen = length_indiseq(seq);
	ld.mode = 'n';

	/* TO DO: connect this to menuitem system */
	if (multi) {
		menu = _("Commands:  j Move down   k Move up  d Delete   i Select   q Quit");
		choices = "jkriq123456789()[]$^udUD";
	} else {
		menu = _("Commands:   j Move down     k Move up    i Select     q Quit");
		choices = "jkiq123456789()[]$^udUD";
	}

resize_win: /* we come back here if we resize the window */
	activate_popup_list_uiwin(&ld);
	win = uiw_win(ld.uiwin);
	if (first) {
		elemwidth = ld.rectDetails.right - ld.rectDetails.left + 1;
		if (length_indiseq(seq)<50)
			preprint_indiseq(seq, elemwidth, &disp_shrt_rfmt);
		first=FALSE;
	}
	uierase(ld.uiwin);
	draw_win_box(win);
	row = ld.rectMenu.top-1;
	show_horz_line(ld.uiwin, row++, 0, uiw_cols(ld.uiwin));
	mvccwaddstr(win, row, ld.rectMenu.left, menu);
	done = FALSE;
	while (!done) {
		INT code=0, ret=0;
		print_list_title(fulltitle, sizeof(fulltitle), &ld, ttl);
		mvccwaddstr(win, 1, 1, fulltitle);
		shw_popup_list(seq, &ld);
		wmove(win, row, 11);
		wrefresh(win);
		code = interact_popup(ld.uiwin, choices);
		if (handle_list_cmds(&ld, code))
			continue;
		if (handle_popup_list_resize(&ld, code)) {
			deactivate_uiwin_and_touch_all(); /* kills ld.uiwin */
			ld.uiwin = NULL;
			/* we're going to repick window & activate */
			goto resize_win;
		}
		if (ret == 0) { /* not handled yet */
			switch (code) {
			case 'r':
				if (!multi)
					break;
				delete_indiseq(seq, NULL, NULL, ld.cur);
				if (!(ld.listlen = length_indiseq(seq))) {
					done=TRUE;
					ld.cur = -1;
				}
				if (ld.cur == ld.listlen) ld.cur--;
				if (ld.cur < ld.top) ld.top = ld.cur;
				break;
			case 'i':
			case CMD_KY_ENTER:
				done=TRUE;
				/* ld.cur points to currently selected */
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
				if (ld.listlen < 10 && code - '1' < ld.listlen) {
					done=TRUE;
					ld.cur = code - '1';
				}
				break;
			case 'q':
				done=TRUE;
				ld.cur = -1; /* ld.cur == -1 means cancelled */
				break;
			}
		}
	}
	deactivate_uiwin_and_touch_all(); /* kills ld.uiwin */
	ld.uiwin = NULL;
	
	return ld.cur;
}
/*=============================================================
 * handle_list_cmds -- Process choices from list display
 *  This handles moving up & down, adjusting size of detail,
 *  and scrolling detail.
 *  @listdisp: [I/O] array of info about list display
 *  @code:     [IN]  command to process
 * Returns -1 if resized window, 1 if handled, 0 if unhandled.
 *===========================================================*/
static INT
handle_list_cmds (listdisp * ld, INT code)
{
	INT rows = ld->rectList.bottom - ld->rectList.top + 1;
	INT tmp;
	switch(code) {
	case 'j': /* next item */
	case CMD_KY_DN:
		if (ld->cur < ld->listlen - 1) {
			ld->cur++;
			if (ld->cur >= ld->top + rows)
				ld->top = ld->cur + 1 - rows;
		}
		return TRUE; /* handled */
	case 'd':
	case CMD_KY_PGDN:
		if (ld->top + rows < ld->listlen) {
			ld->top += rows;
			ld->cur += rows;
			if (ld->cur > ld->listlen - 1)
				ld->cur = ld->listlen - 1;
		}
		return TRUE; /* handled */
	case 'D':
	case CMD_KY_SHPGDN:
		if (ld->top + rows < ld->listlen) {
			tmp = (ld->listlen)/10;
			if (tmp < rows*2) tmp = rows*2;
			if (tmp > ld->listlen - rows - ld->top)
				tmp = ld->listlen - rows - ld->top;
			ld->top += tmp;
			ld->cur += tmp;
			if (ld->cur > ld->listlen - 1)
				ld->cur = ld->listlen - 1;
		}
		return TRUE; /* handled */
	case '$': /* jump to end of list */
	case CMD_KY_END:
		ld->top = ld->listlen - rows;
		if (ld->top < 0)
			ld->top = 0;
		ld->cur = ld->listlen-1;
		return TRUE; /* handled */
	case 'k': /* previous item */
	case CMD_KY_UP:
		if (ld->cur > 0) {
			ld->cur--;
			if (ld->cur < ld->top)
				ld->top = ld->cur;
		}
		return TRUE; /* handled */
	case 'u':
	case CMD_KY_PGUP:
		tmp = rows;
		if (tmp > ld->top) tmp = ld->top;
		ld->top -= tmp;
		ld->cur -= tmp;
		return TRUE; /* handled */
	case 'U':
	case CMD_KY_SHPGUP:
		tmp = (ld->listlen)/10;
		if (tmp < rows*2) tmp = rows*2;
		if (tmp > ld->top) tmp = ld->top;
		ld->cur -= tmp;
		ld->top -= tmp;
		return TRUE; /* handled */
	case '^': /* jump to top of list */
	case CMD_KY_HOME:
		ld->top = ld->cur = 0;
		return TRUE; /* handled */
	case '(': /* scroll detail area up */
		if (ld->details_scroll)
			ld->details_scroll--;
		return TRUE; /* handled */
	case ')': /* scroll detail area down */
		if (ld->details_scroll<2)
			ld->details_scroll++;
		return 1; /* handled */
	}
	return FALSE; /* unhandled */
}
/*=====================================================
 * shw_popup_list -- Draw list & details of popup list
 *===================================================*/
static void
shw_popup_list (INDISEQ seq, listdisp * ld)
{
	WINDOW *win = uiw_win(ld->uiwin);
	ASSERT(ld->listlen == length_indiseq(seq));
	if (ld->details) {
		INT row = ld->rectDetails.top-1;
		clear_hseg(win, row, ld->rectDetails.left, ld->rectDetails.right);
		mvccwaddstr(win, row, 2, _("--- CURRENT SELECTION ---"));
		shw_recordlist_details(seq, ld);
		row = ld->rectDetails.bottom+1;
		mvccwaddstr(win, row, ld->rectDetails.left, _("--- LIST ---"));
	}
	shw_recordlist_list(seq, ld);
}
/*=====================================================
 * shw_recordlist_details -- Draw record details for a list
 * For either popup list or full-screen list (list browse)
 *===================================================*/
static void
shw_recordlist_details (INDISEQ seq, listdisp * ld)
{
	WINDOW *win = uiw_win(ld->uiwin);
	INT i;
	STRING key, name;
	BOOLEAN reuse=FALSE; /* don't reuse display strings in list */
	for (i=ld->rectDetails.top; i<=ld->rectDetails.bottom; ++i) {
		clear_hseg(win, i, ld->rectDetails.left, ld->rectDetails.right-10);
	}
	element_indiseq(seq, ld->cur, &key, &name);
	if (!show_record(ld->uiwin, key, ld->mode, &ld->rectDetails
		, &ld->details_scroll, reuse)) {
		/* if couldn't find record, just display record key */
		display_string(ld->uiwin, &ld->rectDetails, key);
	}
}
/*=============================================================
 * handle_popup_list_resize -- Process resizes of popup list
 * In popup list, details & list compete, & menu is fixed
 * Returns TRUE if handled, FALSE if not
 *===========================================================*/
static BOOLEAN
handle_popup_list_resize (listdisp * ld, INT code)
{
	INT delta;
	switch(code) {
	case '[': /* shrink detail area */
		if (ld->details) {
			delta = (ld->details > ld->details_minhgt) ? 1 : ld->details;
			ld->details -= delta;
			ld->rectDetails.bottom -= delta;
			ld->rectList.top -= delta;
			return -1; /* handled & needs resize */
		}
		return 1; /* handled (nothing) */
	case ']': /* enlarge detail area */
		if (ld->details < ld->details_maxhgt) {
			delta = ld->details ? 1 : ld->details_beginhgt;
			ld->details += delta;
			ld->rectDetails.bottom += delta;
			ld->rectList.top += delta;
			return TRUE; /* handled */
		}
		return TRUE; /* handled (nothing) */
	}
	return FALSE; /* unhandled */
}
/*=====================================================
 * shw_recordlist_list -- Draw actual list items
 * For either popup list or full-screen list (list browse)
 *===================================================*/
static void
shw_recordlist_list (INDISEQ seq, listdisp * ld)
{
	WINDOW *win = uiw_win(ld->uiwin);
	INT width = (ld->rectList.right - ld->rectList.left + 1) - 4;
	INT rows = ld->rectList.bottom - ld->rectList.top + 1;
	INT i, j, row;
	INT offset=4;
	char buffer[160];
	BOOLEAN scrollable = (rows < ld->listlen);
	/* for short lists, use leading numbers */
	if (ld->listlen < 10) {
		sprintf(buffer, "%ld: ", ld->listlen);
		i = strlen(buffer);
		width -= i; /* for "1: " */
		offset += i;
	}
	if (width > (INT)sizeof(buffer)-1)
		width = sizeof(buffer)-1;
	for (j=0; j<rows; j++) {
		/* j is zero-based iterator */
		/* i is actual offset into indiseq */
		i = ld->top + j;
		/* row is row on screen */
		row = ld->rectList.top + j;
		clear_hseg(win, row, ld->rectList.left, ld->rectList.right);
		if (i<ld->listlen) {
			if (i == 0 && scrollable)
				mvwaddch(win, row, ld->rectList.left, '^');
			if (i == ld->listlen-1 && scrollable)
				mvwaddch(win, row, ld->rectList.left, '$');
			if (i == ld->cur) mvwaddch(win, row, ld->rectList.left+3, '>');
			if (ld->listlen < 10) {
				char numstr[12];
				sprintf(numstr, "%d:", i+1);
				mvccwaddstr(win, row, ld->rectList.left+4, numstr);
			}
			print_indiseq_element(seq, i, buffer, width, &disp_shrt_rfmt);
			mvccwaddstr(win, row, ld->rectList.left+offset, buffer);
		}
	}
}
/*==================================
 * print_list_title -- Print title line of an array list
 * Adds suffix such as (5/11)
 * Truncates title if necessary (leaving room for suffix)
 *  buffer:  [OUT] output string
 *  len:     [IN]  size of buffer
 *  ld:      [IN]  list display structure
 *  ttl:     [IN]  title to print (localized)
 *================================*/
static void
print_list_title (char * buffer, INT len, const listdisp * ld, STRING ttl)
{
	STRING ptr = buffer;
	char suffix[30];
	if (len > uiw_cols(ld->uiwin)-2)
		len = uiw_cols(ld->uiwin)-2;
	sprintf(suffix, " (%ld/%ld)", ld->cur+1, ld->listlen);
	len -= strlen(suffix)+1; /* reserve room for suffix */
	ptr[0] = 0;
	if ((INT)strlen(ttl)>len-1) {
		len -= 4;
		llstrcatn(&ptr, ttl, &len);
		len += 4;
		llstrcatn(&ptr, "...", &len);
	} else {
		llstrcatn(&ptr, ttl, &len);
	}
	len += strlen(suffix)+1; /* we reserved this room above */
	llstrcatn(&ptr, suffix, &len);
}
/*================================================================
 * shw_array_of_strings -- Show string list in list interact window
 *  strings: [IN]  array (of choices) to be listed
 *  ld:      [IN]  structure of info for variable-sized list
 *  detfnc:  [IN]  callback for detail area
 *  param:   [IN]  opaque data for callback
 *==============================================================*/
static void
shw_array_of_strings (STRING *strings, listdisp * ld, DETAILFNC detfnc
	, void * param)
{
	/* 120 spaces */
	STRING empstr120 = "                                                                                                                        ";
	WINDOW *win = uiw_win(ld->uiwin);
	INT i, j, row, lines;
	INT rows = ld->rectList.bottom - ld->rectList.top + 1;
	INT overflag=FALSE;
	char buffer[120];
	INT width = uiw_cols(ld->uiwin);
	if (width > (INT)sizeof(buffer)-1)
		width = sizeof(buffer)-1;
	/* clear current lines */
	lines = rows + (ld->details ? ld->details+2 : 0);
	for (i = 0; i<lines; ++i) {
		row = i+2;
		llstrncpy(buffer, empstr120, width-1, uu8);
		mvccwaddstr(win, row, 1, buffer);
	}
	row = 2;
	if (ld->details) {
		row = 3+ld->details;
		mvccwaddstr(win, row++, 2, _("--- LIST ---"));
	}
	for (j=0; j<rows;++j) {
		INT nlen=0,temp;
		i = ld->top + j;
		if (i>=ld->listlen)
			break;
		/* for short lists, we show leading numbers */
		if (ld->listlen<10) {
			char numstr[12]="";
			llstrncpyf(numstr, sizeof(numstr), uu8, "%d: ", i+1);
			if (i == ld->cur) mvwaddch(win, row, 3, '>');
			mvccwaddstr(win, row, 4, numstr);
			nlen = strlen(numstr);
		} else {
			if (i == ld->cur) mvwaddch(win, row, 3, '>');
		}
		temp = width-6-nlen;
		llstrncpy(buffer, strings[i], temp, uu8);
		if ((INT)strlen(buffer) > temp-2) {
			if (i==ld->cur)
				overflag=TRUE;
			strcpy(&buffer[temp-3], "...");
		}
		mvccwaddstr(win, row, 4+nlen, buffer);
		row++;
	}
	if (ld->details) {
		STRING ptr = strings[ld->cur];
		INT count;
		row = 2;
		mvccwaddstr(win, row++, 2, _("-- CURRENT SELECTION --"));
		for (i=0; i<ld->details; ++i) {
			/* TODO: scroll */
			if (!ptr[0]) break;
			llstrncpy(buffer, ptr, width-5, uu8);
			mvccwaddstr(win, row++, 4, buffer);
			ptr += strlen(buffer);
		}
		count = ld->details-1;
		if (count && detfnc) {
			/* caller gave us a detail callback, so we set up the
			data needed & call it */
			STRING * linestr = (STRING *)stdalloc(count * sizeof(STRING));
			struct tag_array_details dets;
			for (j=0; j<count; ++j) {
				linestr[j] = stdalloc(width);
				linestr[j][0] = 0;
			}
			memset(&dets, 0, sizeof(dets));
			dets.list = strings;
			dets.cur = ld->cur;
			dets.lines = linestr;
			dets.count = count;
			dets.maxlen = width;
			(*detfnc)(&dets, param);
			for (j=0 ; j<count; ++j) {
				mvccwaddstr(win, row++, 4, linestr[j]);
			}
			for (j=0; j<count; ++j)
				stdfree(linestr[j]);
			stdfree(linestr);
		}
	}
}
/*=============================================================
 * activate_popup_list_uiwin --
 *  Choose list uiwin & activate
 *  @listdisp:  [I/O]  caller must have filled this in
 *    This routine sets the uiwin, height, rows members
 *===========================================================*/
static void
activate_popup_list_uiwin (listdisp * ld)
{
	INT asklen, hgt, rows, waste;
	/* 
	How many rows do we want ?
	One for each item in list
	+5: top line, title, bottom row, menu, bottom line
	if details, need also line above & below details
	*/
	asklen = ld->listlen;
	if (ld->details)
		asklen += ld->details+2;
	hgt = asklen+5;

	if (hgt>POPUP_LINES)
		hgt = POPUP_LINES;
	create_newwin2(&ld->uiwin, "list", hgt, LISTWIN_WIDTH);
	uiw_dynamic(ld->uiwin) = TRUE; /* delete when finished */
	/* list is below details to nearly bottom */
	ld->rectList.left = 1;
	ld->rectList.right = LISTWIN_WIDTH-2;
	ld->rectList.top = 2;
	if (ld->details) /* leave room for --DETAILS-- & --LIST-- lines */
		ld->rectList.top += 2+ld->details;
	ld->rectList.bottom = hgt-4;
	/* details is from top down as far as #details */
	ld->rectDetails.top = 2;
	if (ld->details) /* leave room for --DETAILS-- line */
		++ld->rectDetails.top;
	ld->rectDetails.bottom = ld->rectDetails.top + ld->details-1;
	ld->rectDetails.left = 1;
	ld->rectDetails.right = LISTWIN_WIDTH-2;
	/* menu is at bottom, single-line */
	ld->rectMenu.top = hgt-2;
	ld->rectMenu.bottom = hgt-2;
	ld->rectMenu.left = 1;
	ld->rectMenu.right = LISTWIN_WIDTH-2;
	ld->details_beginhgt = 4;
	ld->details_maxhgt = POPUP_LINES-10;
	ld->details_minhgt = 3;

	activate_uiwin(ld->uiwin);
	/* ensure cur is on-screen */
	/* (growing detail area can push current off-screen) */
	rows = ld->rectList.bottom + 1 - ld->rectList.top;
	if (ld->cur < ld->top)
		ld->top = ld->cur;
	else if (ld->cur >= ld->top + rows)
		ld->top = ld->cur + 1 - rows;
	/* don't waste space by scrolling end up */
	waste = ld->top + rows - ld->listlen;
	if (waste>0 && ld->top) {
		ld->top -= waste;
		if (ld->top < 0)
			ld->top = 0;
	}
}
/*=====================================================
 * display_string -- Draw string in rectangle
 *  handle embedded carriage returns
 *===================================================*/
static void
display_string (UIWINDOW uiwin, LLRECT rect, STRING text)
{
	INT max = rect->right - rect->left + 2;
	STRING str = stdalloc(max), p2;
	WINDOW *win = uiw_win(uiwin);
	INT row = rect->top;
	wipe_window_rect(uiwin, rect);
	for (row = rect->top; row <= rect->bottom; ++row) {
		str[0] = 0;
		p2 = str;
		while (p2<str+max && text[0] && !islinebreak(text[0]))
			*p2++ = *text++;
		*p2 = 0;
		mvccwaddstr(win, row, rect->left, str);
		if (!text[0])
			break;
		else
			++text;
	}
	stdfree(str);
}
/*==========================================
 * show_big_list - Show name list in list screen
 *========================================*/
void
show_big_list (INDISEQ seq,
           INT top,
           INT cur,
           INT mark)
{
/*
TODO: To be resizable like popup list, need a listdisp structure,
	and need to repaint that RHS menu, as its height will vary
	just to be scrollable details doesn't require repainting the RHS menu
But in any case the real problem is that 
show_big_list (screen.c) is called by list_browse (screen.c)
which is called by browse_list (lbrowse.c!), and it handles menus
and listdisp is local to screen.c right now, so browse_list can't have one
A solution would be to pass in what is known from browse_list, and then
manufacture a listdisp here
- Perry, 2002/01/01
*/
	static STRING empstr51 = (STRING) "                                                   ";
	UIWINDOW uiwin = main_win;
	WINDOW *win = uiw_win(uiwin);
	INT i, j, row, len = length_indiseq(seq);
	STRING key, name;
	NODE recnode=0;
	char scratch[200];
	INT mode = 'n';
	INT viewlines = 13;
	BOOLEAN scrollable = (viewlines < len);

	calc_indiseq_names(seq); /* we certainly need the names */
	
	for (i = LIST_LINES+2; i < LIST_LINES+2+viewlines; i++)
		mvccwaddstr(win, i, 1, empstr51);
	row = LIST_LINES+2;
	for (i = top, j = 0; j < viewlines && i < len; i++, j++) {
		element_indiseq(seq, i, &key, &name);
		recnode = key_to_type(key, 0);
		if (i == 0 && scrollable) mvwaddch(win, row, 1, '^');
		if (i == len-1 && scrollable) mvwaddch(win, row, 1, '$');
		if (i == mark) mvwaddch(win, row, 2, 'x');
		if (i == cur) {
			INT drow=1;
			INT scroll=0;
			BOOLEAN reuse=FALSE;
			struct tag_llrect rectList;
			rectList.top = drow;
			rectList.bottom = drow + LIST_LINES-1;
			rectList.left = 1;
			rectList.right = get_main_screen_width()-2;
			mvwaddch(win, row, 3, '>');
			show_record(main_win, key, mode, &rectList, &scroll, reuse);
		}
		scratch[0] =0;
		if (name) {
			SURCAPTYPE surcaptype = DOSURCAP;
			if (!getlloptint("UppercaseSurnames", 1))
				surcaptype = NOSURCAP;
			name = manip_name(name, surcaptype, REGORDER, 40);
			llstrapps(scratch, sizeof(scratch), uu8, name);
			llstrapps(scratch, sizeof(scratch), uu8, " ");
		}
		if(getlloptint("DisplayKeyTags", 0) > 0) {
			llstrappf(scratch, sizeof(scratch), uu8, "(i%s)", key_of_record(recnode));
		} else {
			llstrappf(scratch, sizeof(scratch), uu8, "(%s)", key_of_record(recnode));
		}
		mvccwaddstr(win, row, 4, scratch);
		row++;
	}
}
/*==============================================
 * paint_list_screen -- Paint list browse screen
 *============================================*/
void
paint_list_screen (void)
{
	UIWINDOW uiwin = main_win;
	WINDOW *win = uiw_win(uiwin);
	INT row, col;
	uierase(uiwin);
	draw_win_box(win);
	show_horz_line(uiwin, LIST_LINES+1, 0, ll_cols);
	show_horz_line(uiwin, ll_lines-3, 0, ll_cols);
	show_vert_line(uiwin, LIST_LINES+1, 52, 15);
	mvwaddch(win, LIST_LINES+1, 52, get_gr_ttee());
	mvccwaddstr(win, LIST_LINES+2, 54, _("Choose an operation:"));
	row = LIST_LINES+3; col = 55;
	mvccwaddstr(win, row++, col, _("j  Move down list"));
	mvccwaddstr(win, row++, col, _("k  Move up list"));
	mvccwaddstr(win, row++, col, _("e  Edit this person"));
	mvccwaddstr(win, row++, col, _("i  Browse this person"));
	mvccwaddstr(win, row++, col, _("m  Mark this person"));
	mvccwaddstr(win, row++, col, _("r  Remove from list"));
	mvccwaddstr(win, row++, col, _("t  Enter tandem mode"));
	mvccwaddstr(win, row++, col, _("n  Name this list"));
	mvccwaddstr(win, row++, col, _("b  Browse new persons"));
	mvccwaddstr(win, row++, col, _("a  Add to this list"));
	mvccwaddstr(win, row++, col, _("x  Swap mark/current"));
	mvccwaddstr(win, row++, col, _(qSmn_quit));
}
/*==============================================
 * listui_placecursor_main -- Get location for cursor in the large list browse window
 *============================================*/
void
listui_placecursor_main (INT * prow, INT * pcol)
{
	*prow = LIST_LINES+2;
	*pcol = 75;
}
