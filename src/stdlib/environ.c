/* 
   Copyright (c) 2000 Perry Rapp

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


#include "sys_inc.h"
#include "llstdlib.h"


/*====================================================
 * environ_figure_tempfile -- calculate temporary file
 *  (fully qualified path)
 *==================================================*/
STRING environ_figure_tempfile ()
{
#ifdef WIN32
	STRING e;
	/* NB: this string must be modifiable - ie, in data segment */
	static char win32_tempfile[_MAX_PATH];
	/* windows has per-user temporary directory, depending on version */
	e = (STRING)getenv("TEMP");
	if (!e || *e == 0) e = (STRING)getenv("TMP");
	if (!e || *e == 0) e = "\\temp"; /* fallback is \temp */
	strcpy(win32_tempfile, e);
	strcat(win32_tempfile, "\\lltmpXXXXXX");
	return mktemp(win32_tempfile);
#else
	static char unix_tempfile[] = "/tmp/lltmpXXXXXX";
	return mktemp(unix_tempfile);
#endif
}

/*=========================================================
 * environ_figure_editor -- calculate editor program to use
 *=======================================================*/
STRING environ_figure_editor ()
{
	STRING e;

	e = (STRING) getenv("LLEDITOR");
	if (!e || *e == 0) e = (STRING) getenv("ED");
	if (!e || *e == 0) e = (STRING) getenv("EDITOR");
#ifdef WIN32
	/* win32 fallback is notepad */
	if (!e || *e == 0) e = (STRING) "notepad.exe";
#else
	/* unix fallback is vi */
	if (!e || *e == 0) e = (STRING) "vi";
#endif
	return e;
}

