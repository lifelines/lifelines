/* 
   Copyright (c) 2005 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * fileops.c -- Wrappers around some file operations, to exit if fail
 *==============================================================*/


#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */



/*********************************************
 * local function prototypes
 *********************************************/

static void report_fatal_fileop(STRING call, INT code, CNSTRING filename, STRING srcfile, int srcline);

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*======================================================
 * report_fatal_fileop -- Describe error and call __fatal to exit
 *  filename may be null (only used for error message)
 *====================================================*/
static void
report_fatal_fileop (STRING call, INT code, CNSTRING filename, STRING srcfile, int srcline)
{
	char details[4096];
	if (!filename || !filename[0]) filename = "?";
	snprintf(details, sizeof(details), "%s failed code %ld, to file %s",
		call, code, filename);
	__fatal(srcfile, srcline, details); /* exits */
}
/*======================================================
 * do_checked_fwrite -- fwrite, and report & exit if fails
 *  filename may be null (only used for error message)
 *====================================================*/
size_t
do_checked_fwrite (const void *buf, size_t size, size_t count, FILE *fp,
	STRING filename, STRING srcfile, int srcline)
{
	size_t rtn = fwrite(buf, size, count, fp);
	if (rtn != count) {
		report_fatal_fileop("fwrite", rtn, filename, srcfile, srcline); /* exits */
	}
	return rtn;
}
/*======================================================
 * do_checked_fwrite -- fwrite, and report & exit if fails
 *  filename may be null (only used for error message)
 *====================================================*/
int
do_checked_fflush (FILE *fp, STRING filename, STRING srcfile, int srcline)
{
	INT rtn = fflush(fp);
	if (rtn != 0) {
		report_fatal_fileop("fflush", rtn, filename, srcfile, srcline); /* exits */
	}
	return rtn;
}
/*======================================================
 * do_checked_fclose -- fclose, and report & exit if fails
 *  filename may be null (only used for error message)
 *====================================================*/
int
do_checked_fclose (FILE *fp, STRING filename, STRING srcfile, int srcline)
{
	INT rtn = fclose(fp);
	if (rtn != 0) {
		report_fatal_fileop("fclose", rtn, filename, srcfile, srcline); /* exits */
	}
	return rtn;
}
/*======================================================
 * do_checked_fseek -- fseek, and report & exit if fails
 *  filename may be null (only used for error message)
 *====================================================*/
int
do_checked_fseek (FILE *fp, long offset, int whence, STRING filename, STRING srcfile, int srcline)
{
	INT rtn = fseek(fp, offset, whence);
	if (rtn != 0) {
		report_fatal_fileop("fseek", rtn, filename, srcfile, srcline); /* exits */
	}
	return rtn;
}
