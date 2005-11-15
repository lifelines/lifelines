/* 
   Copyright (c) 2005 Perry Rapp
   "The X11 License"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * llabort.c -- Routine for offering user chance to abort
 *==============================================================*/

#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */

/*********************************************
 * external/imported variables
 *********************************************/
extern STRING qScoredump;
extern STRING qSaskyY;

/*********************************************
 * local function prototypes
 *********************************************/

static BOOLEAN is_yes(INT ch);

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*================================
 * ll_optional_abort -- print msg & stop if caller wants to abort
 * else return (& caller should exit)
 *  caller translated msg
 *===============================*/
void
ll_optional_abort (STRING msg)
{
	INT ch;
	if (msg)
		printf(msg);
	printf(_(qScoredump));
	fflush(stdout);

	/* TODO: how do we i18n this ? This getchar assumes that 
	the answer is one byte */

	ch = getchar();
	putchar(ch);
	if (is_yes(ch))
		abort();
}
/*==================================================
 * is_yes -- is this an abbreviated yes answer ?
 *=================================================*/
static BOOLEAN
is_yes (INT ch)
{
	STRING ptr;
	for (ptr = _(qSaskyY); *ptr; ptr++) {
		if (ch == *ptr) return TRUE;
	}
	return FALSE;
}
