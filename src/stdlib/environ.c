/* 
   Copyright (c) 2000-2001 Perry Rapp

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
 * environ.c -- Fetch some values from environment
 * Copyright(c) 2000-2001 by Perry Rapp; all rights reserved
 * Created in 3.0.6.dev, 23 Dec 2001
 *===========================================================*/

#include "llstdlib.h"
#ifdef WIN32
#include <Windows.h>
#endif

/*=======================================================
 * environ_determine_tempfile -- calculate temporary file
 *  (fully qualified path)
 *  returns static buffer
 * Created: 2000/12/23, Perry Rapp
 *=====================================================*/
STRING
environ_determine_tempfile (void)
{
#ifdef WIN32
	static char win32_tempfile[_MAX_PATH];
	char win32_tempdir[_MAX_PATH];
	DWORD dirlen;
	UINT fileid;

	dirlen = GetTempPathA(sizeof(win32_tempdir), win32_tempdir);
	if (dirlen == 0 || dirlen >= sizeof(win32_tempdir))
		return 0;
	fileid = GetTempFileNameA(win32_tempdir, "llt", 0, win32_tempfile);
	if (!fileid)
		return 0;
	return win32_tempfile;
#else
	static char template[] = "/tmp/lltmpXXXXXX";
	int fd;
	static char unix_tempfile[sizeof(template)];
	strcpy(unix_tempfile, template);
	/* security precaution */
	/* fd = open(unix_tempfile, O_EXCL|O_CREAT|O_WRONLY, 0x600); */
	fd = mkstemp(unix_tempfile);
	if (-1 == fd) return 0;
	close(fd);
	return unix_tempfile;
#endif
}
/*============================================================
 * environ_determine_editor -- calculate editor program to use
 * Created: 2000/12/23, Perry Rapp
 *==========================================================*/
STRING
#ifdef WIN32
environ_determine_editor (INT program)
#else
environ_determine_editor (HINT_PARAM_UNUSED INT program)
#endif
{
	STRING e;
	static char vi[] = "vi";
#ifdef WIN32
	static char notepad[] = "notepad.exe";
#endif

	/* getenv is required for compatibility with the supported legacy MSVC. */
	e = getenv("LLEDITOR"); /* NOSONAR */
	if (ISNULL(e)) e = getenv("ED"); /* NOSONAR */
	if (ISNULL(e)) e = getenv("EDITOR"); /* NOSONAR */
#ifdef WIN32
	/* win32 fallback is notepad for LifeLines */
	if (ISNULL(e)) {
		if (program == PROGRAM_LIFELINES) {
			e = notepad;
		} else if (program == PROGRAM_BTEDIT) {
			/* btedit requires a binary editor */
			e = vi;
		}
	}
#else
	/* unix fallback is vi for all programs */
	if (ISNULL(e)) e = vi;
#endif
	return e;
}
