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
 * addkey.c -- Adds a new key to a BTREE
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    3.0.0 - 24 Sep 94
 *===========================================================*/

#include "llstdlib.h"
#include "btreei.h"

/*===========================
 * addkey -- Add key to BTREE
 *  btree: [in] handle to btree (memory representation)
 *  ikey:  [in] index file # (which index to add the key to)
 *  rkey:  [in] record key of new entry
 *  fkey:  [in] file key of new entry
 * Precondition: This key belongs in this index block
 *=========================*/
void
addkey (BTREE btree, FKEY ikey, RKEY rkey, FKEY fkey)
{
	INDEX index;
	SHORT lo, hi;

#ifdef DEBUG
	llwprintf("ADDKEY: ikey, rkey = %s, %s;", fkey2path(ikey), rkey2str(rkey));
	llwprintf("fkey = %s\n", fkey2path(fkey));
#endif

   /* Validate the operation */
	if (bwrite(btree) != 1) return;
	index = getindex(btree, ikey);
	if (nkeys(index) >= NOENTS - 1) {
		char msg[72];
		llstrncpyf(msg, sizeof(msg), uu8
			, "Index %d found overfull (%d entries > %d max)"
			, ikey, nkeys(index), NOENTS-2);
		FATAL2(msg);
	}

   /* Search for record key in index -- shouldn't be there */
	lo = 1;
	hi = nkeys(index);
	while (lo <= hi) {
		SHORT md = (lo + hi)/2;
		INT rel = cmpkeys(&rkey, &rkeys(index, md));
		if (rel < 0)
			hi = --md;
		else if (rel > 0)
			lo = ++md;
		else {
			char msg[64];
			llstrncpyf(msg, sizeof(msg), uu8, "addkey called with"
				" entry '%s' already present", rkey.r_rkey);
			FATAL2(msg);
		}
	}

   /* Add new RKEY, FKEY pair to index */
	for (hi = nkeys(index); hi >= lo; hi--) {
		rkeys(index, hi+1) = rkeys(index, hi);
		fkeys(index, hi+1) = fkeys(index, hi);
	}
	rkeys(index, lo) = rkey;
	fkeys(index, lo) = fkey;
	nkeys(index)++;

   /* If index is now full split it */
	if (nkeys(index) >= NOENTS - 1) {
		INDEX newdex = crtindex(btree);
		SHORT n = NOENTS/2 - 1;
		nkeys(newdex) = nkeys(index) - n - 1;
		nkeys(index) = n;
		putindex(btree, index);
		for (lo = 0, hi = n + 1; hi < NOENTS; lo++, hi++) {
			rkeys(newdex, lo) = rkeys(index, hi);
			fkeys(newdex, lo) = fkeys(index, hi);
		}
		putindex(btree, newdex);

   /* Special case -- split requires new master index */
		if (ixself(index) == ixself(bmaster(btree))) {
			INDEX master = crtindex(btree);
			nkeys(master) = 1;
			fkeys(master, 0) = ikey;
			rkeys(master, 1) = rkeys(newdex, 0);
			fkeys(master, 1) = ixself(newdex);
			newmaster(btree, master);
			writeindex(btree, master);
		} else	
			addkey(btree, ixparent(index), rkeys(newdex, 0),
				ixself(newdex));
	} else
		putindex(btree, index);
}
