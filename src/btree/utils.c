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
 * utils.c - Low level BTREE utilities
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    3.0.0 - 04 Oct 94
 *   3.0.2 - 09 Nov 94
 *===========================================================*/

#include "standard.h"
#include "btree.h"

#include "opnbtree.h"
#include "utils.h"

INT bterrno = 0;

/*=========================================
 * rkey2str -- Convert record key to STRING
 *=======================================*/
STRING
rkey2str (RKEY rkey)
{
	static unsigned char rbuf[9];
	SHORT i;
	for (i = 0; i < 8; i++)
		rbuf[i] = rkey.r_rkey[i];
	rbuf[8] = 0;
	for (i = 0; rbuf[i] == ' '; i++)
		;
	return &rbuf[i];
}
/*=========================================
 * str2rkey -- Convert STRING to record key
 *=======================================*/
RKEY
str2rkey (STRING str)
{
	RKEY rkey;
	SHORT i = 0, n = strlen(str);
	ASSERT(n > 0);
	n = 8 - n;
	i = 0;
	if (n > 0)
		for (; i < n; i++)
			rkey.r_rkey[i] = ' ';
	for (; i < 8; i++) 
		rkey.r_rkey[i] = *str++;
	return rkey;
}
/*======================================
 * path2fkey -- Convert path to file key
 *====================================*/
FKEY
path2fkey (STRING path)
{
	SHORT hi = (path[0] -'a')*26 + path[1] - 'a';
	SHORT lo = (path[3] -'a')*26 + path[4] - 'a';
	return (hi<<16) + lo;
}
/*======================================
 * fkey2path -- Convert file key to path
 *====================================*/
STRING
fkey2path (FKEY fkey)
{
	static unsigned char path[6];
	SHORT hi = (fkey & 0xffff0000) >> 16;
	SHORT lo = fkey & 0xffff;
	path[0] = hi/26 + 'a';
	path[1] = hi%26 + 'a';
	path[2] = '/';
	path[3] = lo/26 + 'a';
	path[4] = lo%26 + 'a';
	path[5] = 0;
	return path;
}
/*==============================================
 * nextfkey -- Increment next file key for BTREE
 *============================================*/
void
nextfkey (BTREE btree)
{
	FKEY fkey = btree->b_kfile.k_fkey;
	SHORT hi = (fkey & 0xffff0000) >> 16;
	SHORT lo = fkey & 0xffff;
	char scratch[200];
	if (lo == hi)
		fkey = lo;
	else if (hi > lo)
		++fkey;
	else if (hi < lo-1)
		fkey += 0x10000;
	else {
		fkey += 0x20000;
		fkey &= 0xffff0000;
		sprintf(scratch, "%s/%s", btree->b_basedir, fkey2path(fkey));
		if (!mkalldirs(scratch))
			FATAL();
	}
	btree->b_kfile.k_fkey = fkey;
}
/*==========================================
 * newmaster -- Change master index of BTREE
 *========================================*/
BOOLEAN
newmaster (BTREE btree,  /*btree handle*/
           INDEX master)
{
	btree->b_kfile.k_mkey = iself(master);
	rewind(btree->b_kfp);
	if (fwrite(&btree->b_kfile, sizeof(KEYFILE), 1, btree->b_kfp) != 1) {
		bterrno = BTERRKFILE;
		return FALSE;
	}
	btree->b_master = master;
	return TRUE;
}
