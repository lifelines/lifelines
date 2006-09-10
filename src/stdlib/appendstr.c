/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * appendstr.c -- replacements for strncat, snprintf, and strncat
 *  These actually advance caller's pointers & decrement caller's count
 *  (of remaining bytes).
 *==============================================================*/


#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */


/*==================================
 * appendstr -- Append to string, subject to length limit
 * Advance target pointer and decrease length.
 * Safe to call after length goes to zero (nothing happens).
 *  pdest:  [I/O] output buffer
 *  len:    [I/O] bytes remaining in output
 *  src:    [IN]  source to append
 * Created: 2000/11/29, Perry Rapp
 * NB: Need one byte for terminating zero, so len==1 is same as len==0.
 *================================*/
void
appendstr (STRING * pdest, INT * len, int utf8, CNSTRING src)
{
	int amount;
	*pdest[0]=0; /* so client doesn't have to initialize */
	if (*len<=1) { *len=0; return; }

	llstrncpy(*pdest, src, *len, utf8);
	amount = strlen(*pdest);
	*pdest += amount;
	*len -= amount;
	if (*len<=1) *len=0;
}
