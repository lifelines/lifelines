/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * sprintpic.c -- snprintf versions which handle reordered arguments
 *  (because not everyone is fortunate enough to have glibc)
 *==============================================================*/

#include "llstdlib.h"
#include "zstr.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static BOOLEAN printpic_arg(STRING *b, INT max, INT utf8, CNSTRING arg, INT arglen);

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==============================
 * printpic_arg -- Print an arg to a string
 *  This is the heart of sprintpic, here so we
 *  can use it in all sprintpic's.
 *  b:      [I/O] pointer to output buffer
 *  max:    [IN]  space left in output buffer
 *  arg:    [IN]  arg to insert
 *  arglen: [IN]  (precomputed) length of arg
 * returns FALSE if can't fit entire arg.
 *============================*/
static BOOLEAN
printpic_arg (STRING *b, INT max, INT utf8, CNSTRING arg, INT arglen)
{
	if (!arglen) return TRUE;
	if (arglen > max) {
		/* can't fit it all */
		llstrncpy(*b, arg, max+1, utf8); 
		b[max] = 0;
		return FALSE;
	} else {
		/* it fits */
		strcpy(*b, arg);
		*b += arglen;
		return TRUE;
	}
}
/*=========================================
 * sprintpic0 -- Print using a picture string
 *  with no arguments
 * This is just snprintf, but fixed to always
 * zero-terminate.
 *=======================================*/
void
sprintpic0 (STRING buffer, INT len, INT utf8, CNSTRING pic)
{
	if (len == snprintf(buffer, len, pic)) {
		/* overflowed -- back up to last character that fits */
		INT width=0;
		STRING prev = find_prev_char(&buffer[len-1], &width, buffer, utf8);
		prev[width]=0;
	}

}
/*==============================
 * sprintpic1 -- Print using a picture string
 *  with one argument, eg "From %1"
 *  buffer:  [I/O] output buffer
 *  len:     [IN]  size of output buffer
 *  pic:     [IN]  picture string
 *  arg1:    [IN]  argument (for %1)
 * (Multiple occurrences of %1 are allowed.)
 * returns FALSE if it couldn't fit whole thing.
 *============================*/
BOOLEAN
sprintpic1 (STRING buffer, INT len, INT utf8, CNSTRING pic, CNSTRING arg1)
{
	STRING b = buffer, bmax = &buffer[len-1];
	CNSTRING p=pic;
	INT arg1len = arg1 ? strlen(arg1) : 0; /* precompute */
	while (1) {
		if (p[0]=='%' && p[1]=='1') {
			if (!printpic_arg(&b, bmax-b, utf8, arg1, arg1len))
				return FALSE;
			p += 2;
		} else {
			*b++ = *p++;
		}
		if (!p[0]) { /* ran out of input */
			*b=0;
			return TRUE;
		}
		if (b == bmax) { /* ran out of output room */
			*b=0;
			return FALSE;
		}
	}
}
/*==============================
 * zprintpic1 -- Print using a picture string
 *  with one argument, eg "From %1"
 *  pic:     [IN]  picture string
 *  arg1:    [IN]  argument (for %1)
 * (Multiple occurrences of %1 are allowed.)
 * returns result as zstring
 *============================*/
ZSTR
zprintpic1 (CNSTRING pic, CNSTRING arg1)
{
	ZSTR zstr = zs_newn(strlen(pic)+strlen(arg1));
	CNSTRING p=pic;
	while (*p) {
		if (p[0]=='%' && p[1]=='1') {
			zs_apps(zstr, arg1);
			p += 2;
		} else {
			zs_appc(zstr, *p++);
		}
	}
	return zstr;
}
/*==============================
 * sprintpic2 -- Print using a picture string
 *  with two arguments, eg "From %1 To %s"
 * See sprintpic1 for argument explanation.
 *============================*/
BOOLEAN
sprintpic2 (STRING buffer, INT len, INT utf8, CNSTRING pic, CNSTRING arg1, CNSTRING arg2)
{
	STRING b = buffer, bmax = &buffer[len-1];
	CNSTRING p=pic;
	INT arg1len = arg1 ? strlen(arg1) : 0; /* precompute */
	INT arg2len = arg2 ? strlen(arg2) : 0;
	while (1) {
		if (p[0]=='%' && p[1]=='1') {
			if (!printpic_arg(&b, bmax-b, utf8, arg1, arg1len))
				return FALSE;
			p += 2;
		} else if (p[0]=='%' && p[1]=='2') {
			if (!printpic_arg(&b, bmax-b, utf8, arg2, arg2len))
				return FALSE;
			p += 2;
		} else {
			*b++ = *p++;
		}
		if (!p[0]) { /* ran out of input */
			*b=0;
			return TRUE;
		}
		if (b == bmax) { /* ran out of output room */
			*b=0;
			return FALSE;
		}
	}
}
/*==============================
 * zprintpic2 -- Print using a picture string
 *  with two arguments, eg "From %1 To %s"
 * returning result as zstring
 *============================*/
ZSTR
zprintpic2 (CNSTRING pic, CNSTRING arg1, CNSTRING arg2)
{
	ZSTR zstr = zs_newn(strlen(pic)+strlen(arg1)+strlen(arg2));
	CNSTRING p=pic;
	while (*p) {
		if (p[0]=='%' && p[1]=='1') {
			zs_apps(zstr, arg1);
			p += 2;
		} else if (p[0]=='%' && p[1]=='2') {
			zs_apps(zstr, arg2);
			p += 2;
		} else {
			zs_appc(zstr, *p++);
		}
	}
	return zstr;
}
/*==============================
 * sprintpic3 -- Print using a picture string
 *  with three arguments, eg "%1/%2/%3"
 * See sprintpic1 for argument explanation.
 *============================*/
BOOLEAN
sprintpic3 (STRING buffer, INT len, INT utf8, CNSTRING pic, CNSTRING arg1, CNSTRING arg2
	, CNSTRING arg3)
{
	STRING b = buffer, bmax = &buffer[len-1];
	CNSTRING p=pic;
	INT arg1len = arg1 ? strlen(arg1) : 0; /* precompute */
	INT arg2len = arg2 ? strlen(arg2) : 0;
	INT arg3len = arg3 ? strlen(arg3) : 0;
	while (1) {
		if (p[0]=='%' && p[1]=='1') {
			if (!printpic_arg(&b, bmax-b, utf8, arg1, arg1len))
				return FALSE;
			p += 2;
		} else if (p[0]=='%' && p[1]=='2') {
			if (!printpic_arg(&b, bmax-b, utf8, arg2, arg2len))
				return FALSE;
			p += 2;
		} else if (p[0]=='%' && p[1]=='3') {
			if (!printpic_arg(&b, bmax-b, utf8, arg3, arg3len))
				return FALSE;
			p += 2;
		} else {
			*b++ = *p++;
		}
		if (!p[0]) { /* ran out of input */
			*b=0;
			return TRUE;
		}
		if (b == bmax) { /* ran out of output room */
			*b=0;
			return FALSE;
		}
	}
}
/*==============================
 * zprintpic3 -- Print using a picture string
 *  with three arguments, eg "%1/%2/%3"
 * returning result as zstring
 *============================*/
ZSTR
zprintpic3 (CNSTRING pic, CNSTRING arg1, CNSTRING arg2, CNSTRING arg3)
{
	ZSTR zstr = zs_newn(strlen(pic)+strlen(arg1)+strlen(arg2)+strlen(arg3));
	CNSTRING p=pic;
	while (*p) {
		if (p[0]=='%' && p[1]=='1') {
			zs_apps(zstr, arg1);
			p += 2;
		} else if (p[0]=='%' && p[1]=='2') {
			zs_apps(zstr, arg2);
			p += 2;
		} else if (p[0]=='%' && p[1]=='3') {
			zs_apps(zstr, arg3);
			p += 2;
		} else {
			zs_appc(zstr, *p++);
		}
	}
	return zstr;
}
