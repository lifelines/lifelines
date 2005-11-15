/* 
   Copyright (c) 2005 Perry Rapp
   "The X11 License"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * errlog.c -- Routines for writing to crash log
 *==============================================================*/

#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif

/*********************************************
 * local variables
 *********************************************/

static char f_crashfile[MAXPATHLEN]="";
static char f_currentdb[MAXPATHLEN]="";

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*===============================
 * errlog_out -- Send message to log file (if one exists)
 *=============================*/
void
errlog_out (CNSTRING title, CNSTRING msg, CNSTRING file, int line)
{
	/* avoid reentrancy */
	static BOOLEAN failing=FALSE;
	if (failing) return;
	failing=TRUE;

	/* send to error log if one is specified */
	if (f_crashfile[0]) {
		FILE * fp = fopen(f_crashfile, LLAPPENDTEXT);
		if (fp) {
			LLDATE creation;
			get_current_lldate(&creation);
			if (title) {
				fprintf(fp, "\n%s: %s\n", title, creation.datestr);
				if (msg && msg[0]) {
					fprintf(fp, "    %s\n", msg);
				}
			} else {
				if (msg && msg[0]) {
					fprintf(fp, "%s: %s\n", creation.datestr, msg);
				} else {
					fprintf(fp, "%s\n", creation.datestr);
				}
			}
			if (line>=1 && file && file[0])
				fprintf(fp, _("    in file <%s> at line %d\n"), file, line);
			if (f_currentdb[0])
				fprintf(fp, "    %s: %s\n", _("Current database"), f_currentdb);
			fclose(fp);
		}
	}
	failing=FALSE;
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
	llstrncpy(f_currentdb, dbname, sizeof(f_currentdb), 0);
}
