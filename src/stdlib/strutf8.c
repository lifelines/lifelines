/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * strutf.c -- UTF-8 specific string functions
 *==============================================================*/

#include "llstdlib.h"

/*==============================
 * utf8len -- Length of a utf-8 character
 *  ch: [in] starting byte
 * Created: 2001/08/02 (Perry Rapp)
 *============================*/
INT
utf8len (char ch)
{
	/* test short cases first, as probably much more common */
	if (!(ch & 0x80 && ch & 0x40))
		return 1;
	if (!(ch & 0x20))
		return 2;
	if (!(ch & 0x10))
		return 3;
	if (!(ch & 0x08))
		return 4;
	if (!(ch & 0x04))
		return 5;
	return 6;
}
/*=========================================
 * find_prev_char -- Back up to start of previous character.
 * Return pointer to start of previous character,
 *  and optionally its width.
 * (This is of course trivial if we're not in UTF-8 mode.)
 * If limit is non-zero, don't back up beyond this.
 * This will set *width=0 if it gets back to limit without 
 *  finding valid character, or backs up more than 6 characters
 * Created: 2002/06/12, Perry Rapp
 *=======================================*/
STRING
find_prev_char (STRING ptr, INT * width, STRING limit, int utf8)
{
	STRING init = ptr;
	INT len=0;
	if (utf8) {
		while (1) {
			INT tmp;
			if (ptr == limit)
				break;
			--ptr;
			if (init - ptr == 7)
				break;
			if (*ptr & 0x80)
				continue;
			tmp = utf8len(*ptr);
			if (tmp <= init-ptr) {
				len = tmp;
				break;
			}
		}
	} else {
		if (ptr != limit) {
			len = 1;
			--ptr;
		}
	}
	if (width)
		*width = len;
	return ptr;
}
