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
 * utils.c -- Low level BTREE utilities
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    3.0.0 - 04 Oct 94
 *   3.0.2 - 09 Nov 94
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "btreei.h"


/*=========================================
 * rkey2str -- Convert record key to STRING
 *  returns static buffer
 *=======================================*/
STRING
rkey2str (RKEY rkey)
{
	static char rbuf[RKEYLEN+1];
	INT i;
	for (i = 0; i < RKEYLEN; i++)
		rbuf[i] = rkey.r_rkey[i];
	rbuf[RKEYLEN] = 0;
	for (i = 0; rbuf[i] == ' '; i++)
		;
	return &rbuf[i];
}
/*=========================================
 * str2rkey -- Convert STRING to record key
 * eg, str2rkey("I34") => "     I34"
 *=======================================*/
RKEY
str2rkey (CNSTRING str)
{
	RKEY rkey;
	INT i = 0, n = strlen(str);
	ASSERT(n > 0);
	n = RKEYLEN - n;
	i = 0;
	if (n > 0)
		for (; i < n; i++)
			rkey.r_rkey[i] = ' ';
	for (; i < RKEYLEN; i++) 
		rkey.r_rkey[i] = *str++;
	return rkey;
}
/*======================================
 * path2fkey -- Convert path to file key
 *====================================*/
FKEY
path2fkey (STRING path)
{
	INT16 hi = (path[0] - 'a') * 26 + (path[1] - 'a');
	INT16 lo = (path[3] - 'a') * 26 + (path[4] - 'a');
	return (FKEY)((hi<<16) + lo);
}
/*======================================
 * fkey2path -- Convert file key to path
 *====================================*/
STRING
fkey2path (FKEY fkey)
{
	static char path[6];
	INT16 hi = (fkey & 0xffff0000) >> 16;
	INT16 lo = fkey & 0x0000ffff;
	path[0] = (hi / 26) + 'a';
	path[1] = (hi % 26) + 'a';
	path[2] = LLCHRDIRSEPARATOR;
	path[3] = (lo / 26) + 'a';
	path[4] = (lo % 26) + 'a';
	path[5] = 0;
	return path;
}
/*==============================================
 * nextfkey -- Increment next file key for BTREE
 *============================================*/
/* Explanation of FKEY algorithm, by example.
 *
 * Iter  Text    Hex (HI/LO)  Text   Hex (HI/LO)  Notes 
 * ----+-------+------------+------+------------+------------
 *  00                        ab/ab  0x00010001   First index key
 *  01   ab/ab   0x00010001   aa/ab  0x00000001   Case 1
 *  02   aa/ab   0x00000001   ac/aa  0x00020000   Case 4
 *  03   ac/aa   0x00020000   ac/ab  0x00020001   Case 2
 *  04   ac/ab   0x00020001   ac/ac  0x00020002   Case 2
 *  05   ac/ac   0x00020002   aa/ac  0x00000002   Case 1
 *  06   aa/ac   0x00000002   ab/ac  0x00010002   Case 3
 *  07   ab/ac   0x00010002   ad/aa  0x00030000   Case 4
 *  08   ad/aa   0x00030000   ad/ab  0x00030001   Case 2
 *  09   ad/ab   0x00030001   ad/ac  0x00030002   Case 2
 *  10   ad/ac   0x00030002   ad/ad  0x00030003   Case 2
 *  11   ad/ad   0x00030003   aa/ad  0x00000003   Case 1
 *  12   aa/ad   0x00000003   ab/ad  0x00010003   Case 3
 *  13   ab/ad   0x00010003   ac/ad  0x00020003   Case 3
 *  14   ac/ad   0x00020003   ae/aa  0x00040000   Case 4
 *  15   ae/aa   0x00040000   ae/ab  0x00040001   Case 2
 * 
 * and so on ...
 * 
 * This is curious pattern and will be difficult to optimize.
 */
void
nextfkey (BTREE btree)
{
	FKEY fkey = btree->b_kfile.k_fkey;
	INT16 hi = (fkey & 0xffff0000) >> 16;
	INT16 lo = fkey & 0x0000ffff;
	char scratch[MAXPATHLEN];

	/* Case 1: HI=0000 LO=xxxx */
	if (hi == lo)
		fkey = lo;

	/* Case 2: HI=HI, LO=LO+1 */
	else if (hi > lo)
		++fkey;

	/* Case 3: HI++, LO=LO */
	else if (hi < lo-1)
		fkey += 0x00010000;

	/* Case 4: HI+=2, LO=00 */
	else /* (hi == lo-1) */
        {
		fkey += 0x00020000;
		fkey &= 0xffff0000;
		snprintf(scratch, sizeof(scratch), "%s/%s", btree->b_basedir, fkey2path(fkey));
		if (!mkalldirs(scratch))
			FATAL();
	}
	btree->b_kfile.k_fkey = fkey;
}
/*==========================================
 * newmaster -- Change master index of BTREE
 *========================================*/
void
newmaster (BTREE btree, INDEX master)
{
	/*
	Assumes our keyfile is valid
	so it is important that we got an exclusive writer lock
	*/
	btree->b_kfile.k_mkey = ixself(master);
	rewind(btree->b_kfp);
	if (fwrite(&btree->b_kfile, sizeof(btree->b_kfile), 1, btree->b_kfp) != 1) {
		char scratch[400];
		snprintf(scratch, sizeof(scratch), "Error rewriting master block: %s", fkey2path(ixself(master)));
		FATAL2(scratch);
	}
	btree->b_master = master;
}
