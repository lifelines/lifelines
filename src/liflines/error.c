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

#include "standard.h"
#include "liflines.h"
#include "screen.h"
#include "arch.h"
#include "llstdlib.h"

char errorfile[MAXPATHLEN]="";

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
	if (errorfile[0]) {
		FILE * fp = fopen(errorfile, LLAPPENDTEXT);
		if (fp) {
			LLDATE creation;
			get_current_lldate(&creation);
			fprintf(fp, "\nFatal Error: %s\n    ", creation.datestr);
			if (details && details[0]) {
				fprintf(fp, details);
				fprintf(fp, "\n    AT: ");
			}
			fprintf(fp, "%s: line %d\n", file, line);
			fclose(fp);
		}
	}
	/* send to screen */
	llwprintf("FATAL ERROR: ");
	if (details && details[0]) {
		llwprintf(details);
		llwprintf("\nAT: ");
	}
	llwprintf("%s: line %d\n", file, line);
	close_lifelines();
	shutdown_ui(TRUE); /* pause */
	ll_abort(-1);
}
/*===============================
 * __crashlog -- Details preceding a fatal error
 * Created: 2001/09/02, Perry Rapp
 *=============================*/
void
crashlog (STRING fmt, ...)
{
	/*
	TO DO - there ought to be a configuration option
	to log this to a file
	*/

	va_list args;
	va_start(args, fmt);
	llvwprintf(fmt, args);
	va_end(args);
}
/*===============================
 * error_seterrorlog -- specify where to log alloc messages
 * Creatd: 2001/10/28, Perry Rapp
 *=============================*/
void
error_seterrorlog(STRING errorlog)
{
	llstrncpy(errorfile, errorlog, sizeof(errorfile)/sizeof(errorfile[0]));
}

