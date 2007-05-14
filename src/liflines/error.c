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
/*=============================================================
 * error.c -- Standard error routines
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 16 Oct 94
 *===========================================================*/

#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#include "liflines.h"
#include "feedback.h"
#include "arch.h"

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif

static void vcrashlog(int newline, const char * fmt, va_list args);


/*
 2002/10/05
 These routines do not depend on curses (llwprintf can be implemented w/o curses)
 and so could move out of the liflines subdir. llexec is using this file.
*/

/*===============================
 * __fatal -- Fatal error routine
 *  handles null or empty details input
 *=============================*/
void
__fatal (STRING file, int line, CNSTRING details)
{
	/* avoid reentrancy */
	static BOOLEAN failing=FALSE;
	if (failing) return;
	failing=TRUE;

	/* send to error log if one is specified */
	errlog_out(_("Fatal Error"), details, file, line);

	/* send to screen */
	llwprintf("%s\n", _("FATAL ERROR"));
	if (details && details[0]) {
		llwprintf("  %s\n", details);
	}
	llwprintf(_("  in file <%s> at line %d\n"), file, line);
	/* offer crash dump before closing database */
	ll_optional_abort(_("ASSERT failure"));
	close_lifelines();
	shutdown_ui(TRUE); /* pause */
	failing=FALSE;
	exit(1);
}
/*===============================
 * vcrashlog -- Send crash info to crash log and screen
 *  internal implementation
 *=============================*/
static void
vcrashlog (int newline, const char * fmt, va_list args)
{
	char buffer[2048];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	buffer[sizeof(buffer)-1] = 0;
	if (newline) {
		/* ensure room at end to append \n */
		buffer[sizeof(buffer)-2] = 0;
		strcat(buffer, "\n");
	}

	/* send to error log if one is specified */
	errlog_out(NULL, buffer, NULL, -1);

	/* send to screen */
	llwprintf(buffer);
}
/*===============================
 * vcrashlog -- Send crash info to crash log and screen
 *=============================*/
void
crashlog (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vcrashlog(0, fmt, args);
	va_end(args);
}
/*===============================
 * vcrashlog -- Send crash info to crash log and screen
 *  add carriage return to end line
 *=============================*/
void
crashlogn (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vcrashlog(1, fmt, args);
	va_end(args);
}
