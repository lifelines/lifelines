/* 
   Copyright (c) 1996-2000 Paul B. McBride

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#if defined(HAVE_ALLOC_H) && defined(HAVE_HEAPWALK)
#  include <alloc.h>
#  define HEAPINFO struct heapinfo
#  define WALK(ent) heapwalk(&(ent))
#  define SIZE(ent) ((ent).size)
#  define PTR(ent)  ((ent).ptr)
#  define USED(ent) ((ent).in_use)
#  define WORKING_HEAPUSED
#endif

#if defined(HAVE_MALLOC_H) && defined(HAVE__HEAPWALK)
#  include <malloc.h>
#  define HEAPINFO _HEAPINFO
#  define WALK(ent) _heapwalk(&(ent))
#  define SIZE(ent) ((ent)._size)
#  define PTR(ent)  ((ent)._pentry)
#  define USED(ent) ((ent)._useflag)
#  define WORKING_HEAPUSED
#endif

#include "llstdlib.h"
#include "table.h"
#include "gedcom.h"
#include "interpi.h"

/*===============================================+
 * llrpt_heapused -- Return amount of heapspace in use
 * usage: heapused() -> INT
 *==============================================*/
PVALUE
llrpt_heapused (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
#if defined(WORKING_HEAPUSED)
	HEAPINFO hi;
	long heapused;
	long heapfree;
	long heapcnt;
	int repcnt;
	static FILE *errfp = NULL;
	node=node; /* unused */
	stab=stab; /* unused */

	if(errfp == NULL) errfp = fopen("pbm.err", LLWRITETEXT);

	repcnt = 4;
	heapused = 0;
	heapfree = 0;
	heapcnt = 0;
	PTR(hi) = NULL;

	while(WALK(hi) == _HEAPOK) {
	  heapcnt++;
	  if(errfp && (repcnt-- > 0))
	      fprintf(errfp, "%ld %ld %ld\n", (long)PTR(hi), (long)SIZE(hi),
		      (long)USED(hi));
	  if(USED(hi)) heapused += SIZE(hi);
	  else heapfree += SIZE(hi);
	}
	*eflg = FALSE;
	if(errfp)
	    fprintf(errfp, "%ld free, %ld used, %ld entries\n",
		    (long)heapused, (long)heapfree, (long)heapcnt);
	fflush(errfp);

	return create_pvalue_from_int(heapfree);
#else
	/* Unsupported, what should we do? return error or give bogus value? */
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = TRUE;
	return NULL;
#endif
}
