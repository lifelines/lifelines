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
 * interact.c -- code to wait for and return a single letter or keypress
 * Copyright(c) 2006
 *===========================================================*/

#include <time.h>
#include "llstdlib.h"
#include "liflines.h"
#include "llinesi.h"
#include "menuitem.h"
#include "screen.h"
#include "screeni.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSmn_unkcmd;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static INT interact_worker(UIWINDOW uiwin, STRING str, INT screen);
static INT translate_control_key(INT c);
static INT translate_hdware_key(INT c);

/*********************************************
 * local variables
 *********************************************/

static int ui_time_elapsed = 0; /* total time waiting for user input */

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*===============================
 * interact_choice_string -- Interact with user
 * Handle string of choices as passed
 * also returns hardware and control keys, so caller must check & loop
 *=============================*/
INT
interact_choice_string (UIWINDOW uiwin, STRING str)
{
	return interact_worker(uiwin, str, -1);
}
/*===============================
 * interact_screen_menu -- Interact with user
 * This is just for browse screens (witness argument "screen")
 * and uses the preconfigured menu for that screen
 * also returns hardware and control keys, so caller must check & loop
 *=============================*/
INT
interact_screen_menu (UIWINDOW uiwin, INT screen)
{
	return interact_worker(uiwin, NULL, screen);
}
/*===============================
 * interact_worker -- Interact with user
 * This is just for browse screens (witness argument "screen")
 *=============================*/
static INT
interact_worker (UIWINDOW uiwin, STRING str, INT screen)
{
	char buffer[4]; /* 3 char cmds max */
	INT offset=0;
	INT cmdnum;
	INT c, i, n = str ? strlen(str) : 0;

	/* Menu Loop */
	while (TRUE)
	{
		INT time_start=time(NULL);
		crmode();
		keypad(uiw_win(uiwin),1);
		c = wgetch(uiw_win(uiwin));
		ui_time_elapsed += time(NULL) - time_start;
		if (c == EOF) c = 'q';
		nocrmode();
		/* after they chose off the menu, we wipe any
		status message still lingering from before they chose */
		clear_status_display();
		/*
		return hardware and control keys
		in case caller handles them
		*/
		if (c<0x20) {
			return translate_control_key(c);
		}
		if (has_key(c)) {
			return translate_hdware_key(c);
		}
		if (str) { /* traditional: list of choice letters */
			for (i = 0; i < n; i++) {
				if (c == (uchar)str[i]) return c;
			}
		} else { /* new menus (in menuitem.c) */
			if (offset < (INT)sizeof(buffer)-1) {
				buffer[offset] = c;
				buffer[offset+1] = 0;
				offset++;
			} else {
				buffer[0] = c;
				buffer[1] = 0;
				offset = 1;
			}

			/* Get Menu Command */
			cmdnum = menuset_check_cmd(get_screen_menuset(screen), buffer);

			/* Act On Menu Command */
			if (cmdnum != CMD_NONE && cmdnum != CMD_PARTIAL) {
				return cmdnum;
			}
			if (cmdnum != CMD_PARTIAL) {
				msg_error(_(qSmn_unkcmd));
				offset = 0;
			}
		}
		/* choice was no good, we loop & wait for another choice */
	}
}
/*===============================
 * translate_hdware_key -- 
 *  translate curses keycode into menuitem.h constant
 *=============================*/
struct hdkeycvt { int key; int cmd; };
static INT
translate_hdware_key (INT c)
{
	/* curses constant, menuitem constant */
	static struct hdkeycvt hdkey[] = {
		{ KEY_UP, CMD_KY_UP }
		, { KEY_DOWN, CMD_KY_DN }
		, { KEY_NPAGE, CMD_KY_PGDN }
		, { KEY_PPAGE, CMD_KY_PGUP }
		, { KEY_SNEXT, CMD_KY_SHPGDN }
		, { KEY_SPREVIOUS, CMD_KY_SHPGUP }
		, { KEY_HOME, CMD_KY_HOME }
		, { KEY_END, CMD_KY_END }
		, { KEY_ENTER, CMD_KY_ENTER }
	};
	int i;
	for (i=0; i<ARRSIZE(hdkey); ++i) {
		if (c == hdkey[i].key)
			return hdkey[i].cmd;
	}
	return CMD_NONE;
}
/*===============================
 * translate_control_key -- 
 *  translate control keys into menuitem.h constant
 *=============================*/
static INT
translate_control_key (INT c)
{
	static struct hdkeycvt hdkey[] = {
		{ '\r', CMD_KY_ENTER } /* Win32 */
		, { '\n', CMD_KY_ENTER } /* UNIX */
	};
	int i;
	for (i=0; i<ARRSIZE(hdkey); ++i) {
		if (c == hdkey[i].key)
			return hdkey[i].cmd;
	}
	return CMD_NONE;
}
/*============================
 * get_uitime -- return cumulative elapsed time waiting 
 *  for user input (since start of program)
 *==========================*/
int
get_uitime (void)
{
	return ui_time_elapsed;
}
