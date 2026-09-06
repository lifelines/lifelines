/*
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
 * vsnprintf.c -- substitute implementation of vsnprintf
 */


#include <sys/types.h>
#include <stdio.h>
#include "standard.h"

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif

#ifdef vsnprintf
#undef vsnprintf
#endif

#if defined(HAVE__VSNPRINTF)
int
vsnprintf(char *buffer, size_t count, const char *fmt, va_list args)
{
	int retval = _vsnprintf(buffer, count, fmt, args);
	if (count && (retval < 0 || (size_t)retval >= count))
		buffer[count-1] = 0;
	return retval;
}
#else
int
vsnprintf(char *buffer, size_t count, HINT_PARAM_UNUSED const char *fmt,
	HINT_PARAM_UNUSED va_list args)
{
	if (count)
		buffer[0] = 0;
	return -1;
}
#endif
