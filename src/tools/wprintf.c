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
#include <varargs.h>
/*===========================================================
 * wprintf -- Called as wprintf(fmt, arg, arg, arg, ...)
 *===========================================================*/
/*VARARGS*/
wprintf (va_alist)
va_dcl
{
	va_list args;
	char *fmt;
	va_start(args);
	fmt = va_arg(args, char *);
	vprintf(fmt, args);
}
/*===========================================================
 * mprintf -- Called as mprintf(fmt, arg, arg, arg, ...)
 *===========================================================*/
/*VARARGS*/
mprintf (va_alist)
va_dcl
{
	va_list args;
	char *fmt;
	va_start(args);
	fmt = va_arg(args, char *);
	vprintf(fmt, args);
}
/*======================================
 * poutput -- Print argument as a string
 *====================================*/
poutput (str)
char *str;
{
	printf("%s", str);
}
