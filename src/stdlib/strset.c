/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * strset.c -- string assignment functions
 *  These provide a consistent API mirroring the strapp API
 *==============================================================*/


#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#include "arch.h" /* vsnprintf */

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif

/*==================================
 * llstrset -- llstrncpy adopted to strset/strapp API
 * handles UTF-8
 *================================*/
char *
llstrsets (char *dest, size_t limit, int utf8, const char *src)
{
	return llstrncpy(dest, src, limit, utf8);
}
/*==================================
 * llstrsetc -- llstrset with a character argument
 *================================*/
char *
llstrsetc (char *dest, size_t limit, char ch)
{
	int utf8 = 0;
	char src[2];
	src[0] = ch;
	src[1] = 0;
	return llstrsets(dest, limit, utf8, src);
}
/*==================================
 * llstrsetf -- snprintf style assignment to string,
 * subject to length limit (including current contents)
 * handles UTF-8
 *================================*/
char *
llstrsetf (char * dest, int limit, int utf8, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	llstrsetvf(dest, limit, utf8, fmt, args);
	va_end(args);
	return dest;
}
/*==================================
 * llstrsetvf -- vsnprintf style assignment to string,
 * subject to length limit (including current contents)
 * handles UTF-8
 *================================*/
char *
llstrsetvf (char * dest, int limit, int utf8, const char * fmt, va_list args)
{
	return llstrncpyvf(dest, limit, utf8, fmt, args);
}


