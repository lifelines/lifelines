/* 
   Copyright (c) 2001-2002 Perry Rapp

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
 * strapp.c -- string catenation functions that use original size of buffer
 *  These are replacements for strncat, which use the total buffer size 
 *  rather than the max. remaining length as a parameter.
 *==============================================================*/


#include "llstdlib.h"
#include "arch.h" /* vsnprintf */


/*==================================
 * llstrapp -- llstrncat except limit includes existing string
 *  ie, strncat except it always terminates, it handles UTF-8,
 *  and the limit is inclusive of existing contents
 * handles UTF-8
 *================================*/
char *
llstrapp (char *dest, size_t limit, const char *src)
{
	size_t len;
	int n;
	if (!src || !dest || !src[0]) return dest;
	len = strlen(dest);
	n = limit-len;
	ASSERT(n >= 0);
	if (n < 2) /* must fit trailing zero */
		return dest;

	llstrncpy(dest+len, src, n);
	return dest;
}
/*==================================
 * llstrappc -- llstrapp with a character argument
 *================================*/
char *
llstrappc (char *dest, size_t limit, char ch)
{
	char src[2];
	src[0] = ch;
	src[1] = 0;
	return llstrapp(dest, limit, src);
}
/*==================================
 * llstrappf -- snprintf style append to string,
 * subject to length limit (including current contents)
 * handles UTF-8
 *================================*/
char *
llstrappf (char * dest, int limit, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	llstrappvf(dest, limit, fmt, args);
	va_end(args);
	return dest;
}
/*==================================
 * llstrappvf -- vsnprintf style append to string,
 * subject to length limit (including current contents)
 * handles UTF-8
 *================================*/
char *
llstrappvf (char * dest, int limit, const char * fmt, va_list args)
{
	/* TODO: Revise for UTF-8 */
	size_t len = strlen(dest);
	size_t n = limit-len;
	int rtn;
	if (n < 2) /* must fit trailing zero */
		return dest;

	rtn = vsnprintf(dest+len, n, fmt, args);
	if (rtn == (int)(len-1) || rtn == -1) {
		/* overflowed -- back up to last character that fits */
		INT width=0;
		STRING prev;
		dest[len-1] = 0; /* make sure we're zero-terminated! */
		prev = find_prev_char(&dest[n-1], &width, dest);
		prev[width]=0;
	}
	return dest;
}


