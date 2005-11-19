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
 * index.c -- Handle BTREE indices
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 1993    3.0.0 - 04 Oct 1994
 *   3.0.2 - 24 Nov 1994
 *===========================================================*/

#include "llstdlib.h"
#include "btreei.h"

static INT incache (BTREE, FKEY);
static BOOLEAN cacheindex (BTREE, INDEX);

/*======================================
 * crtindex - Create new index for btree
 *====================================*/
INDEX
crtindex (BTREE btree)
{
	INDEX index;
	ASSERT(bwrite(btree) == 1);
	index = (INDEX) stdalloc(BUFLEN);
	nkeys(index) = 0;
	ixparent(index) = 0;
	ixtype(index) = BTINDEXTYPE;
	ixself(index) = btree->b_kfile.k_fkey;
	nextfkey(btree);
	rewind(bkfp(btree));
	if (fwrite(&bkfile(btree), sizeof(bkfile(btree)), 1, bkfp(btree)) != 1) {
		char scratch[200];
		sprintf(scratch, "Error updating keyfile for new index");
		FATAL2(scratch);
	}
	writeindex(btree, index);
	return index;
}
/*=================================
 * get_index_file - Read index from file
 *  path:    [OUT] path for this index file
 *  btr:     [IN]  database btree
 *  ikey:    [IN]  index file key (number which indicates a file)
 *===============================*/
void
get_index_file (STRING path, BTREE btr, FKEY ikey)
{
	sprintf(path, "%s/%s", bbasedir(btr), fkey2path(ikey));
}
/*=================================
 * readindex - Read index from file
 *  btr:     [IN] btree structure
 *  ikey:    [IN] index file key (number which indicates a file)
 *  robust:  [IN] flag to tell this function to return (not abort) on errors
 * this is below the level of the index cache
 *===============================*/
INDEX
readindex (BTREE btr, FKEY ikey, BOOLEAN robust)
{
	FILE *fi=NULL;
	INDEX index=NULL;
	char scratch[400];
	get_index_file(scratch, btr, ikey);
	if ((fi = fopen(scratch, LLREADBINARY LLFILERANDOM)) == NULL) {
		if (robust) {
			/* fall to end & return NULL */
			goto readindex_end;
		}
		sprintf(scratch, "Missing index file: %s", fkey2path(ikey));
		FATAL2(scratch);
	}
	index = (INDEX) stdalloc(BUFLEN);
	if (fread(index, BUFLEN, 1, fi) != 1) {
		if (robust) {
			goto readindex_end;
		}
		sprintf(scratch, "Undersized (<%d) index file: %s", BUFLEN, fkey2path(ikey));
		FATAL2(scratch);
	}
	if (fi) fclose(fi);
readindex_end:
	return index;
}
/*=================================
 * writeindex - Write index to file
 *  btr:      [IN]  btree structure
 *  index:    [IN]  index block
 *===============================*/
void
writeindex (BTREE btr, INDEX index)
{
	FILE *fi=NULL;
	char scratch[400];
	get_index_file(scratch, btr, ixself(index));
	if ((fi = fopen(scratch, LLWRITEBINARY LLFILERANDOM)) == NULL) {
		sprintf(scratch, "Error opening index file: %s", fkey2path(ixself(index)));
		FATAL2(scratch);
	}
	if (fwrite(index, BUFLEN, 1, fi) != 1) {
		sprintf(scratch, "Error writing index file: %s", fkey2path(ixself(index)));
		FATAL2(scratch);
	}
	if (fclose(fi) != 0) FATAL2(scratch);
}
/*==============================================
 * initcache -- Initialize index cache for btree
 *============================================*/
void
initcache (BTREE btree, /* btree handle */
           INT n)       /* num cache blocks to allow */
{
	INT i=0;
	n = (n < 5) ? 5 : n;
	bncache(btree) = n;
	bcache(btree) = (INDEX *) stdalloc(n*sizeof(INDEX));
	for (i = 0;  i < n;  i++)
		bcache(btree)[i] = NULL;
}
/*========================================
 * freecache -- Free index cache for btree
 *======================================*/
void
freecache (BTREE btree)
{
	INT i=0;
	INT n = bncache(btree);
	for (i=0; i<n; ++i) {
		if (bcache(btree)[i]) {
			stdfree(bcache(btree)[i]);
			bcache(btree)[i] = NULL;
		}
	}
	stdfree(bcache(btree));
}
/*============================================
 * cacheindex -- Place INDEX or BLOCK in cache
 *==========================================*/
static BOOLEAN
cacheindex (BTREE btree, /* btree handle */
            INDEX index) /* INDEX or BLOCK */
{
	INT n = bncache(btree);
	INDEX *indices = bcache(btree);
	INT j = incache(btree, ixself(index));
	if (j == -1) {	/* index not in cache */
		if (indices[n-1]) stdfree(indices[n-1]);
		for (j = n - 1; j > 0; --j)
			indices[j] = indices[j - 1];
		indices[0] = index;
	} else {	/* index is in cache */
		for (; j > 0; --j)
			indices[j] = indices[j - 1];
		indices[0] = index;
	}
	return TRUE;
}
/*================================
 * getindex - Get index from btree
 *  checks cache first to avoid disk read if possible
 *  also loads cache if read from disk
 *  does not return if error
 *==============================*/
INDEX
getindex (BTREE btree, FKEY fkey)
{
	INT j;
	INDEX index;
	if (fkey == ixself(bmaster(btree))) return bmaster(btree);
	if ((j = incache(btree, fkey)) == -1) {	/* not in cache */
		BOOLEAN robust = FALSE; /* abort on error */
		index = readindex(btree, fkey, robust);
		cacheindex(btree, index);
		return index;
	}
	return (bcache(btree))[j];
}
/*=====================================
 * putindex -- Put out index - cache it
 *===================================*/
void
putindex (BTREE btree,
          INDEX index)
{
	writeindex(btree, index);
	if (ixself(index) == ixself(bmaster(btree))) return;
	cacheindex(btree, index);
}
/*=================================================
 * putheader -- Cache block header - don't write it
 *===============================================*/
void
putheader (BTREE btree,
           BLOCK block)
{
	cacheindex(btree, (INDEX) block);
}
/*============================================================
 * incache -- If INDEX is in cache return index else return -1
 *==========================================================*/
static INT
incache (BTREE btree,
         FKEY fkey)
{
	INT i, n = bncache(btree);
	INDEX index, *indices = bcache(btree);
	for (i = 0; i < n; i++) {
		if ((index = *indices++) == NULL) return -1;
		if (ixself(index) == fkey) return i;
	}
	return -1;
}
