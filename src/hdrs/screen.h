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
 *   2.3.4 - 24 Jun 93    2.3.5 - 20 Aug 93
 *   3.0.0 - 26 Jul 94    3.0.2 - 16 Dec 94
 *============================================================*/

#undef TRUE
#undef FALSE

#ifdef WIN32
#include <mycurses.h>
#else
#include <curses.h>
#endif

#ifndef TRUE
#	define TRUE 1
#	define FALSE 0
#endif

#define MAIN_SCREEN    1
#define ONE_PER_SCREEN 2
#define ONE_FAM_SCREEN 3
#define TWO_PER_SCREEN 4
#define TWO_FAM_SCREEN 5
#define PED_SCREEN     6
#define LIST_SCREEN    7
#define AUX_SCREEN     8

#ifndef ACS_TTEE
#	define ACS_TTEE '+'
#	define ACS_RTEE '+'
#	define ACS_LTEE '+'
#	define ACS_BTEE '+'
#	define ACS_VLINE '|'
#	define ACS_HLINE '-'
#endif

#ifdef BSD
#	undef ACS_TTEE
#	undef ACS_RTEE
#	undef ACS_LTEE
#	undef ACS_BTEE
#	undef ACS_VLINE
#	undef ACS_HLINE

#	define ACS_TTEE ' '
#	define ACS_RTEE ' '
#	define ACS_LTEE ' '
#	define ACS_BTEE ' '
#	define ACS_VLINE ' '
#	define ACS_HLINE '-'
#endif

/*=========================================
 * WINDOWs -- Main screen, menus and popups
 *=======================================*/
extern INT cur_screen;
extern WINDOW *stdout_win;
extern WINDOW *main_win;
extern WINDOW *ask_win;
extern WINDOW *ask_msg_win;
extern WINDOW *choose_from_list_win;
extern WINDOW *start_menu_win;
extern WINDOW *add_menu_win;
extern WINDOW *del_menu_win;
extern WINDOW *utils_menu_win;
