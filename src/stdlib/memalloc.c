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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * memalloc.c -- Standard memory routines
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 16 Oct 94
 *===========================================================*/

#include "sys_inc.h"
#include "standard.h"

static BOOLEAN logopen = FALSE;
FILE *log = NULL;
extern BOOLEAN alloclog;
static char scratch[80];
static void alloc_out(STRING str);

/*=================================================
 * __allocate -- Allocate memory - used by sdtalloc
 *===============================================*/
void *
__allocate (int len,       /* num of bytes to alloc */
            STRING file,   /* file requesting */
            int line)      /* line num in file */
{
	char *p;
	int i;
	if (len == 0) return NULL;
	p = malloc(len);
	if((p == NULL) && alloclog)
		{
		sprintf(scratch, "%8p ? %s\t%d\t%d", p, file, line, len);
		alloc_out(scratch);
		}
	ASSERT(p);
	for(i = 0; i <len; i++) p[i] = '\0';
	if (alloclog) {
		sprintf(scratch, "%8p A %s\t%d\t%d", p, file, line, len);
		alloc_out(scratch);
	}
	return (void*)p;
}
/*====================================================
 * __deallocate -- Deallocate memory - used by sdtfree
 *==================================================*/
void
__deallocate (void *ptr,      /* memory to return */
              STRING file,    /* file returning */
              int line)       /* line num in file */
{
	if (alloclog) {
		sprintf(scratch, "%8p F %s\t%d", ptr, file, line);
		alloc_out(scratch);
	}
	if(ptr) free(ptr);
}
/*=======================================
 * alloc_out -- Output allocation message
 *=====================================*/
static void
alloc_out (STRING str)
{
	if (!alloclog) return;
	if (!logopen) {
		log = fopen("alloc.log", LLWRITETEXT);
		logopen = TRUE;
	}
	fprintf(log, "%s\n", str);
	fflush(log);
}
