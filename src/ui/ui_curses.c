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
#include "ui.h"

/*********************************************
 * external variables (no header)
 *********************************************/

/*********************************************
 * external functions
 *********************************************/
extern void term_screen(void);

/*********************************************
 * local function prototypes
 *********************************************/

/*=============================================================
 * Initialization and Termination
 *===========================================================*/

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
