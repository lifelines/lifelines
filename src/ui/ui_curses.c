/*
   Copyright (c) 2000-2002 Perry Rapp
   Copyright (c) 2023 Matt Emmerton
   "The MIT license"

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * ui_curse.c -- UI code for curses interface (GUI/CHUI)
 * Copyright(c) 2002 by Perry Rapp; all rights reserved
 * Copyright(c) 2023 by Matt Emmerton; all rights reserved
 *===========================================================*/

#include "mycurses.h"
#include "llstdlib.h"
#define UI_ENABLE_CURSES
#include "ui.h"

/*********************************************
 * external variables (no header)
 *********************************************/

/*********************************************
 * external functions
 *********************************************/
extern void term_screen(void);
extern int init_screen(char *, int);

/*********************************************
 * global/exported variables
 *********************************************/
BOOLEAN graphical=TRUE;

/* we ought to use chtype, but only if it is typedef'd, but there is no
test to see if a type is typedef'd */
llchtype gr_btee='+', gr_ltee='+', gr_rtee='+', gr_ttee='+';
llchtype gr_hline='-', gr_vline= '|';
llchtype gr_llx='*', gr_lrx='*', gr_ulx='*', gr_urx='*';

/*********************************************
 * local function prototypes
 *********************************************/
static void set_screen_graphical (BOOLEAN graphical);

/*=============================================================
 * Initialization and Termination
 *===========================================================*/

/*===================================================
 * startup_ui -- Do whatever is necessary to open GUI
 *=================================================*/
BOOLEAN
startup_ui (void)
{
	char errmsg[512];
	BOOLEAN success = FALSE;

	if (!init_screen(errmsg, sizeof(errmsg)/sizeof(errmsg[0])))
	{
		endwin();
		fprintf(stderr, "%s", errmsg);
		goto finish;
	}
	set_screen_graphical(graphical);
	success = TRUE;

finish:
	return success;
}

/*===================================================
 * shutdown_ui -- Do whatever is necessary to close GUI
 * Created: 2001/11/08, Perry Rapp
 *=================================================*/
void
shutdown_ui (BOOLEAN pause)
{
	term_screen();

	/* if error, give user a second to read it */
	if (pause)
		sleep(1);

	/* Terminate Curses UI */
	if (!isendwin())
		endwin();
}
/*============================
 * set_screen_graphical -- Specify whether to use ncurses box characters
 *  graphical:   [IN]  whether to use ncurses graphical box lines
 *==========================*/
static void
set_screen_graphical (BOOLEAN graphical)
{
	if (graphical) {
		gr_btee = ACS_BTEE;
		gr_hline = ACS_HLINE;
		gr_ltee = ACS_LTEE;
		gr_rtee = ACS_RTEE;
		gr_ttee = ACS_TTEE;
		gr_vline = ACS_VLINE;
		gr_llx = ACS_LLCORNER;
		gr_lrx = ACS_LRCORNER;
		gr_ulx = ACS_ULCORNER;
		gr_urx = ACS_URCORNER;
	}
	else {
		gr_btee = '+';
		gr_hline = '-';
		gr_ltee = '+';
		gr_rtee = '+';
		gr_ttee = '+';
		gr_vline = '|';
		gr_llx = '*';
		gr_lrx = '*';
		gr_ulx = '*';
		gr_urx = '*';
	}

}
/*==================================================
 * get_gr_ttee -- current character used for box corners
 *================================================*/
llchtype
get_gr_ttee (void)
{
	return gr_ttee; /* eg, '+' */
}
