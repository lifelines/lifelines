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
/* added 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*===========================================================
 * heapused.c -- report how much heap space is in use (WIN32 - ONLY!)
 *===========================================================*/

#ifdef WIN32

#include <stdio.h>
#include <alloc.h>

#ifndef BOOLEAN
#	define BOOLEAN int
#endif
#ifndef TRUE
#	define TRUE 1
#endif
#ifndef FALSE
#	define FALSE 0
#endif
#define INT int 
typedef char *WORD;
struct dummy {
    int dum;
} dumstruct;

typedef struct dumstruct *PVALUE;
typedef struct dumstruct *TABLE;
typedef struct dumstruct *PNODE;

#define PINT    2	/* integer */

/*===============================================+
 * __heapused -- Return amount of heapspace in use
 *   usage: heapused() -> INT
 *==============================================*/
PVALUE
__heapused (PNODE node,
            TABLE stab,
            BOOLEAN *eflg)
{
	struct heapinfo hi;
	long heapused;
	long heapfree;
	long heapcnt;
	int repcnt;
	static FILE *errfp = NULL;

	if(errfp == NULL) errfp = fopen("pbm.err", "wt");

	repcnt = 4;
	heapused = 0;
	heapfree = 0;
	heapcnt = 0;
	hi.ptr = NULL;

	while(heapwalk(&hi) == _HEAPOK) {
	  heapcnt++;
	  if(errfp && (repcnt-- > 0))
	      fprintf(errfp, "%ld %ld %ld\n", (long)hi.ptr, (long)hi.size,
		      (long) hi.in_use);
	  if(hi.in_use) heapused += hi.size;
	  else heapfree += hi.size;
	}
	*eflg = FALSE;
	if(errfp)
	    fprintf(errfp, "%ld free, %ld used, %ld entries\n",
		    (long)heapused, (long)heapfree, (long)heapcnt);

	return create_pvalue(PINT, (WORD)heapfree);
}
#endif
