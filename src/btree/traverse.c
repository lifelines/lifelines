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

#include "llstdlib.h"
#include "btreei.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static BOOLEAN traverse_block(BTREE btree, BLOCK block, RKEY lo, RKEY hi, BOOLEAN(*func)(RKEY, STRING, INT len, void *), void * param);
static BOOLEAN traverse_index(BTREE btree, INDEX index, RKEY lo, RKEY hi, BOOLEAN(*func)(RKEY, STRING, INT len, void *), void * param);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=====================================================
 * traverse_index_blocks - Traverse BTREE, doing things
 *===================================================*/
BOOLEAN
traverse_index_blocks (BTREE btree, INDEX index,
                       BOOLEAN (*ifunc)(BTREE, INDEX),
                       BOOLEAN (*dfunc)(BTREE, BLOCK))
{
	INDEX newdex;
	STRING bdir = bbasedir(btree);

	if (index == NULL)
		return FALSE;
	if (ixtype(index) == BTINDEXTYPE) {
		INT i, n;
		if (ifunc != NULL && !(*ifunc)(btree, index))
			return FALSE;
		n = nkeys(index);
		for (i = 0; i <= n; i++) {
			BOOLEAN rc;
			newdex = readindex(bdir, fkeys(index, i));
			rc = traverse_index_blocks(btree, newdex, ifunc, dfunc);
			stdfree(newdex);
			if (!rc) return FALSE;
		}
		return TRUE;
	}
	if (dfunc != NULL)
		return (*dfunc)(btree, (BLOCK)index);
	return TRUE;
}
/*====================================================
 * traverse_block -- traverse subtree whilst under key
 *  either lo or hi can have 0 as its first character
 * NB: This covers all records, including DELE records.
 *==================================================*/
static BOOLEAN
traverse_block (BTREE btree, BLOCK block, RKEY lo, RKEY hi,
	BOOLEAN(*func)(RKEY, STRING, INT len, void *), void * param)
{
	STRING p;
	INT i, len;
	RECORD rec;
	INT nkeys=nkeys(block); /* caller loaded block */
	FKEY nfkeyme = ixself(block);
	for (i=0; i<nkeys;i++)
	{
		if (i) {
			/* reload block (lest callback purged it from cache) */
			INDEX index1 = getindex(btree, nfkeyme);
			ASSERT(index1);
			ASSERT(ixtype(index1)==BTBLOCKTYPE);
			block=(BLOCK)index1;
		}
		if (lo.r_rkey[0] && ll_strncmp(lo.r_rkey, rkeys(block, i).r_rkey, 8) >= 0)
			continue;
		if (hi.r_rkey[0] && ll_strncmp(hi.r_rkey, rkeys(block, i).r_rkey, 8) < 0)
			continue;
		rec = readrec(btree, block, i, &len);
		/*
		NB: rec could be NULL if len of record was 0 
		I don't know if this would ever happen - Perry, 2001/05/26
		*/
		p = rec;
		if (!(*func)(rkeys(block,i), p, len, param))
			return FALSE;
	}
	return TRUE;
}
/*====================================================
 * traverse_index -- traverse subtree whilst under key
 *  either lo or hi can have 0 as its first character
 *==================================================*/
static BOOLEAN
traverse_index (BTREE btree, INDEX index, RKEY lo, RKEY hi,
	BOOLEAN(*func)(RKEY, STRING, INT len, void *), void * param)
{
	INDEX index1;
	BLOCK block1;
	SHORT i, n, ilo;
	FKEY nfkey;
	FKEY nfkeyme = ixself(index);

	i=1;
	n = nkeys(index); /* caller loaded index */
	/* advance over any below lo */
	if (lo.r_rkey[0]) {
		for (i=1; i<=n; i++) {
			if (ll_strncmp(lo.r_rkey, rkeys(index, i).r_rkey, 8) < 0)
				break;
		}
	}
	ilo=i;
	/* process all until above hi */
	for ( ; i<=n+1; i++) {
		if (i!=ilo) {
			/* reload index (lest callback purged it from cache) */
			index = getindex(btree, nfkeyme);
			ASSERT(index);
			ASSERT(ixtype(index)==BTINDEXTYPE);
		}
		if (hi.r_rkey[0] && ll_strncmp(hi.r_rkey, rkeys(index, i-1).r_rkey, 8) < 0)
			break;
		nfkey = fkeys(index, i-1);
		ASSERT(index1 = getindex(btree, nfkey));
		if (ixtype(index1) == BTINDEXTYPE) {
			if (!traverse_index(btree, index1, lo, hi, func, param))
				return FALSE;
		} else {
			ASSERT(ixtype(index1) == BTBLOCKTYPE);
			block1 = (BLOCK)index1;
			if (!traverse_block(btree, block1, lo, hi, func, param))
				return FALSE;
		}
	}
	return TRUE;
}
/*==============================================
 * traverse_db_rec_rkeys -- traverse a span of records
 *  using rkeys
 *  either lo or hi can have 0 as its first character
 * NB: This covers all records, including DELE records.
 *============================================*/
void
traverse_db_rec_rkeys (BTREE btree, RKEY lo, RKEY hi, 
	BOOLEAN(*func)(RKEY, STRING, INT len, void *), void * param)
{
	INDEX index;
	ASSERT(index = bmaster(btree));
	traverse_index(btree, index, lo, hi, func, param);
}

