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

#include "standard.h"
#include "btree.h"

/*======================================
 * crtindex - Create new index for btree
 *====================================*/
INDEX crtindex (btree)
BTREE btree;
{
	INDEX index;
	ASSERT(bwrite(btree));
	index = (INDEX) stdalloc(BUFLEN);
	nkeys(index) = 0;
	iparent(index) = 0;
	itype(index) = BTINDEXTYPE;
	iself(index) = btree->b_kfile.k_fkey;
	nextfkey(btree);
	rewind(bkfp(btree));
	if (fwrite(&bkfile(btree), sizeof(KEYFILE), 1, bkfp(btree)) != 1)
		FATAL();
	writeindex(bbasedir(btree), index);
	return index;
}
/*=================================
 * readindex - Read index from file
 *===============================*/
INDEX readindex (basedir, ikey)
STRING basedir;  /* basedir of btree */
FKEY ikey;       /* index file key */
{
	FILE *fp;
	INDEX index;
	char scratch[200];
	sprintf(scratch, "%s/%s", basedir, fkey2path(ikey));
	if ((fp = fopen(scratch, LLREADBINARY)) == NULL) {
		bterrno = BTERRINDEX;
		return NULL;
	}
	index = (INDEX) stdalloc(BUFLEN);
	if (fread(index, BUFLEN, 1, fp) != 1) FATAL();
	fclose(fp);
	return index;
}
/*=================================
 * writeindex - Write index to file
 *===============================*/
writeindex (basedir, index)
STRING basedir; /* base directory of btree */
INDEX index;    /* index block */
{
	FILE *fp;
	char scratch[200];
	sprintf(scratch, "%s/%s", basedir, fkey2path(iself(index)));
	if ((fp = fopen(scratch, LLWRITEBINARY)) == NULL) FATAL();
	if (fwrite(index, BUFLEN, 1, fp) != 1) FATAL();
	fclose(fp);
}
/*==============================================
 * initcache -- Initialize index cache for btree
 *============================================*/
initcache (btree, n)
BTREE btree; /* btree handle */
INT n;       /* num cache blocks to allow */
{
	INT i;
	n = (n < 5) ? 5 : n;
	bncache(btree) = n;
	bcache(btree) = (INDEX *) stdalloc(n*sizeof(INDEX));
	for (i = 0;  i < n;  i++)
		bcache(btree)[i] = NULL;
}
/*============================================
 * cacheindex -- Place INDEX or BLOCK in cache
 *==========================================*/
BOOLEAN cacheindex (btree, index)
BTREE btree; /* btree handle */
INDEX index; /* INDEX or BLOCK */
{
	INT n = bncache(btree);
	INDEX *indices = bcache(btree);
	INT j = incache(btree, iself(index));
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
}
/*================================
 * getindex - Get index from btree
 *==============================*/
INDEX getindex (btree, fkey)
BTREE btree;
FKEY fkey;
{
	INT j;
	INDEX index;
	if (fkey == iself(bmaster(btree))) return bmaster(btree);
	if ((j = incache(btree, fkey)) == -1) {	/* not in cache */
		if ((index = readindex(bbasedir(btree), fkey)) == NULL)
			return NULL;
		cacheindex(btree, index);
		return index;
	}
	return (bcache(btree))[j];
}
/*=====================================
 * putindex -- Put out index - cache it
 *===================================*/
putindex (btree, index)
BTREE btree;
INDEX index;
{
	writeindex(bbasedir(btree), index);
	if (iself(index) == iself(bmaster(btree))) return;
	cacheindex(btree, index);
}
/*=================================================
 * putheader -- Cache block header - don't write it
 *===============================================*/
putheader (btree, block)
BTREE btree;
BLOCK block;
{
	cacheindex(btree, block);
}
/*============================================================
 * incache -- If INDEX is in cache return index else return -1
 *==========================================================*/
INT incache (btree, fkey)
BTREE btree;
FKEY fkey;
{
	INT i, n = bncache(btree);
	INDEX index, *indices = bcache(btree);
	for (i = 0; i < n; i++) {
		if ((index = *indices++) == NULL) return -1;
		if (iself(index) == fkey) return i;
	}
	return -1;
}
