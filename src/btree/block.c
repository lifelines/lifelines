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
 * block.c -- Low level BTREE code dealing with BLOCKs
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    3.0.0 - 24 Sep 94
 *===========================================================*/

#include "llstdlib.h"
#include "btreei.h"

/*========================================
 * allocblock -- Allocate memory for BLOCK
 *======================================*/
BLOCK
allocblock (void)
{
	BLOCK block = (BLOCK) stdalloc(BUFLEN);
	ixtype(block) = BTBLOCKTYPE;
	ixself(block) = 0;
	ixparent(block) = 0;
	nkeys(block) = 0;
	return block;
}
/*=======================================
 * crtblock -- Create new BLOCK for BTREE
 *=====================================*/
BLOCK
crtblock (BTREE btree)  /*btree handle*/
{
	BLOCK block;
	ASSERT(bwrite(btree) == 1);
	block = allocblock();
	ixself(block) = btree->b_kfile.k_fkey;
	nextfkey(btree);
	rewind(bkfp(btree));
	if (fwrite(&bkfile(btree), sizeof(bkfile(btree)), 1, bkfp(btree)) != 1)
		FATAL();
	return block;
}
