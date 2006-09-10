/* 
   Copyright (c) 2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * platform.c -- Some miscellaneous platform-specific code
 *   Created: 2002/11 by Perry Rapp
 *==============================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "arch.h"

#if defined(_WIN32) || defined(__CYGWIN__)
/* Win32 code also needed by cygwin */
#include <windows.h>

static const char * GetWinSysError(INT nerr);

/*=================================================
 * w_get_codepage -- get current Windows codeset
 * Created: 2002/11/27 (Perry Rapp)
 *===============================================*/
int
w_get_codepage (void)
{
	return GetACP();
}
/*=================================================
 * w_get_oemout_codepage -- get current output console codeset
 * Created: 2002/11/27 (Perry Rapp)
 *===============================================*/
int
w_get_oemout_codepage (void)
{
	return GetConsoleOutputCP();
}
/*=================================================
 * w_get_oemin_codepage -- get current input console codeset
 * Created: 2002/11/27 (Perry Rapp)
 *===============================================*/
int
w_get_oemin_codepage (void)
{
	return GetConsoleCP();
}
/*=================================================
 * w_set_oemout_codepage -- set current output console codeset
 * Created: 2003/04/19 (Perry Rapp)
 *===============================================*/
void
w_set_oemout_codepage (int codepage)
{
	INT rtn = SetConsoleOutputCP(codepage);
	if (!rtn) {
		INT errnum = GetLastError();
		const char * desc = GetWinSysError(errnum);
		LocalFree((char *)desc);
	}
}
/*=================================================
 * w_set_oemin_codepage -- set current input console codeset
 * Created: 2003/04/19 (Perry Rapp)
 *===============================================*/
void
w_set_oemin_codepage (int codepage)
{
	INT rtn = SetConsoleCP(codepage);
	if (!rtn) {
		INT errnum = GetLastError();
		const char * desc = GetWinSysError(errnum);
		LocalFree((char *)desc);
	}
}
/*=================================================
 * w_get_has_console -- does process have a console ?
 * Created: 2002/11/27 (Perry Rapp)
 *===============================================*/
int
w_get_has_console (void)
{
	return (GetStdHandle(STD_INPUT_HANDLE) != 0);
}
/*=================================================
 * GetWinSysError -- Get description of system error
 * returned value must be freed with LocalAlloc
 *===============================================*/
static const char *
GetWinSysError (INT nerr)
{
	const char * str = 0;
/* Get user language description of error, if available */
	LPVOID lpMsgBuf;
	if (FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		nerr,
		0, /* Default language */
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		))
	{
		str = (LPCTSTR)lpMsgBuf;
	}
	/* caller must call LocalFree on string */
	return str;
}

#endif /* defined(_WIN32) || defined(__CYGWIN__) */
