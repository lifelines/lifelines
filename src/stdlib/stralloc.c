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
 * stralloc.c -- heap allocation & deallocation for strings
 *==============================================================*/

#include "llstdlib.h"

/*===============================
 * strsave -- Save copy of string
 * returns stdalloc'd memory
 *=============================*/
STRING
strsave (CNSTRING str)
{
	/* some OSs (made in Redmond) do not handle strdup(0) very well */
	ASSERT(str);
	return strcpy(stdalloc(strlen(str) + 1), str);
}
/*===============================
 * strfree -- Free & clear a STRING by ref
 *  (STRING may be NULL)
 *=============================*/
void
strfree (STRING * str)
{
	if (*str) {
		stdfree(*str);
		*str = NULL;
	}
}
/*===============================
 * strupdate -- strfree followed by strsave
 *  (STRING may be NULL)
 *=============================*/
void
strupdate (STRING * str, CNSTRING value)
{
	strfree(str);
	if (value)
		*str = strsave(value);
}
/*==================================
 * strconcat -- Catenate two strings
 * Either (but not both) args may be null
 * returns stdalloc'd memory
 *================================*/
STRING
strconcat (STRING s1, STRING s2)
{
	INT c, len;
	STRING s3, p;
	if (!s1) return strsave(s2);
	if (!s2) return strsave(s1);
	len = strlen(s1) + strlen(s2);
	p = s3 = (STRING) stdalloc(len+1);
	while ((c = *s1++)) *p++ = c;
	while ((c = *s2++)) *p++ = c;
	*p = '\0';
	return s3;
}
/*=============================================+
 * free_array_strings -- Free all strings in an array
 *  n:   [IN]  size of array
 *  arr: [I/O] array
 *============================================*/
void
free_array_strings (INT n, STRING * arr)
{
	INT i;
	for (i=0; i<n; ++i)
	{
		strfree(&arr[i]); /* frees & zeros pointer */
	}
}
