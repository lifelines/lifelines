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
/* traverse.c - traverse a btree, doing things */
/* Version 2.3.4 - controlled 24 June 1993 */

#include "standard.h"
#include "btree.h"

#include "index.h"

/*========================================
 * traverse - Traverse BTREE, doing things
 *======================================*/
BOOLEAN
traverse (BTREE btree, INDEX index,
          BOOLEAN (*ifunc)(BTREE, INDEX),
          BOOLEAN (*dfunc)(BTREE, INDEX))
{
	INDEX newdex;
	STRING bdir = bbasedir(btree);

	if (index == NULL)
		return FALSE;
	if (itype(index) == BTINDEXTYPE) {
		INT i, n;
		if (ifunc != NULL && !(*ifunc)(btree, index))
			return FALSE;
		n = nkeys(index);
		for (i = 0; i <= n; i++) {
			BOOLEAN rc;
			newdex = readindex(bdir, fkeys(index, i));
			rc = traverse(btree, newdex, ifunc, dfunc);
			stdfree(newdex);
			if (!rc) return FALSE;
		}
		return TRUE;
	}
	if (dfunc != NULL)
		return (*dfunc)(btree, index);
	return TRUE;
}
