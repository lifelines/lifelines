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
/*=============================================================
 * memalloc.c -- Standard memory routines
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 16 Oct 94
 *===========================================================*/

#include "standard.h"

static BOOLEAN logopen = FALSE;
static FILE *log = NULL;
extern BOOLEAN alloclog;
static char scratch[80];

/*=================================================
 * __allocate -- Allocate memory - used by sdtalloc
 *===============================================*/
char *__allocate (len, file, line)
int len;	/* num of bytes to alloc */
STRING file;	/* file requesting */
int line;	/* line num in file */
{
	char *p;
	if (len == 0) return NULL;
	ASSERT(p = malloc(len));
	if (alloclog) {
		sprintf(scratch, "A  %s\t%d\t%d\t%d", file, line, len, p);
		alloc_out(scratch);
	}
	return p;
}
/*====================================================
 * __deallocate -- Deallocate memory - used by sdtfree
 *==================================================*/
__deallocate (ptr, file, line)
char *ptr;	/* memory to return */
STRING file;	/* file returning */
int line;	/* line num in file */
{
	if (alloclog) {
		sprintf(scratch, "F  %s\t%d\t\t%d", file, line, ptr);
		alloc_out(scratch);
	}
	free(ptr);
}
/*=======================================
 * alloc_out -- Output allocation message
 *=====================================*/
alloc_out (str)
STRING str;
{
	if (!alloclog) return;
	if (!logopen) {
		log = fopen("alloc.log", "w");
		logopen = TRUE;
	}
	fprintf(log, "%s\n", str);
}
