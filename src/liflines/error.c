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
/*=============================================================
 * error.c -- Standard error routines
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 16 Oct 94
 *===========================================================*/

#include <stdarg.h>
#include "llstdlib.h"
#include "liflines.h"
#include "feedback.h"
#include "arch.h"

static char f_crashfile[MAXPATHLEN]="";
static char f_currentdb[MAXPATHLEN]="";

/*===============================
 * __fatal -- Fatal error routine
 *  handles null or empty details input
 * 2001/09/22, Perry: This uses llwprintf calls, so it belongs in the curses
 *  library (ie, in the liflines directory).
 *=============================*/
void
__fatal (STRING file, int line, STRING details)
{
	/* send to error log if one is specified */
	if (f_crashfile[0]) {
		FILE * fp = fopen(f_crashfile, LLAPPENDTEXT);
		if (fp) {
			LLDATE creation;
			get_current_lldate(&creation);
			fprintf(fp, "\n%s: %s\n", _("Fatal Error")
				, creation.datestr);
			if (details && details[0]) {
				fprintf(fp, "    %s\n", details);
			}
			if (!file || !file[0])
				file = "?";
			fprintf(fp, _("    in file <%s> at line %d\n"), file, line);
			if (f_currentdb[0])
				fprintf(fp, "    %s: %s\n", _("Current database"), f_currentdb);
			fclose(fp);
		}
	}
	/* send to screen */
	llwprintf("%s\n", _("FATAL ERROR"));
	if (details && details[0]) {
		llwprintf("  %s\n", details);
	}
	llwprintf(_("  in file <%s> at line %d\n"), file, line);
	if (f_currentdb[0])
		llwprintf("%s: %s\n", _("Current database"), f_currentdb);
	close_lifelines();
	shutdown_ui(TRUE); /* pause */
	ll_abort(_("ASSERT failure"));
}
/*===============================
 * __crashlog -- Details preceding a fatal error
 *=============================*/
void
crashlog (STRING fmt, ...)
{
	char buffer[2048];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	if (f_crashfile[0] && !allwhite(buffer)) {
		FILE * fp = fopen(f_crashfile, LLAPPENDTEXT);
		if (fp) {
			LLDATE creation;
			get_current_lldate(&creation);
			fprintf(fp, "%s: %s\n", creation.datestr, buffer);
			fclose(fp);
		}
	}

	llwprintf(buffer);
}
/*===============================
 * crash_setcrashlog -- specify where to log alloc messages
 *=============================*/
void
crash_setcrashlog (STRING crashlog)
{
	if (!crashlog)
		crashlog = "";
	llstrncpy(f_crashfile, crashlog, sizeof(f_crashfile), uu8);
}
/*===============================
 * crash_setdb -- record current database in case of a crash
 * Created: 2002/06/16, Perry Rapp
 *=============================*/
void
crash_setdb (STRING dbname)
{
	if (!dbname)
		dbname = "";
	llstrncpy(f_currentdb, dbname, 0, sizeof(f_currentdb));
}
