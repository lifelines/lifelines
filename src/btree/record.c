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
 * record.c -- Routines to handle BTREE records
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Jul 93
 *   3.0.0 - 24 Sep 94    3.0.2 - 26 Mar 95
 *   3.0.3 - 07 May 95
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "btreei.h"

static void filecopy (FILE*, INT, FILE*);
static void movefiles (STRING, STRING);

/*=================================
 * addrecord -- Add record to BTREE
 *===============================*/
BOOLEAN
addrecord (BTREE btree,         /* btree to add record to */
           RKEY rkey,           /* key of record */
           RECORD record,       /* record to add */
           INT len)             /* record length */
{
	INDEX index;
	BLOCK old, new, xtra;
	FKEY nfkey, last = 0, parent;
	SHORT i, j, k, l, n, lo, hi;
	BOOLEAN found = FALSE;
	INT off = 0;
	FILE *fo, *fn1, *fn2;
	char scratch1[200], scratch2[200], *p = record;

/* search for data block that does/should hold record */
	ASSERT(bwrite(btree));
	ASSERT(index = bmaster(btree));
	while (itype(index) == BTINDEXTYPE) {

/* maintain "lazy" parent chaining in btree */
		if (iparent(index) != last) {
			ASSERT(index != bmaster(btree));
			iparent(index) = last;
			writeindex(bbasedir(btree), index);
		}
		last = iself(index);
		n = nkeys(index);
		nfkey = fkeys(index, 0);
		for (i = 1; i <= n; i++) {
			if (ll_strncmp(rkey.r_rkey, rkeys(index, i).r_rkey, 8) < 0)
				break;
			nfkey = fkeys(index, i);
		}
		ASSERT(index = getindex(btree, nfkey));
	}

/* have block that may hold older version of record */
	iparent(index) = last;
	old = (BLOCK) index;
	ASSERT(nkeys(old) < NORECS);

/* see if block has earlier version of record */
	lo = 0;
	hi = nkeys(old) - 1;
	found = FALSE;
	while (lo <= hi) {
		SHORT md = (lo + hi)/2;
		INT rel = ll_strncmp(rkey.r_rkey, rkeys(old, md).r_rkey, 8);
		if (rel < 0)
			hi = --md;
		else if (rel > 0)
			lo = ++md;
		else {
			found = TRUE;
			lo = md;
			break;
		}
	}

/* construct header for updated data block */
	new = allocblock();
	itype(new) = itype(old);
	iparent(new) = iparent(old);
	iself(new) = iself(old);
	n = nkeys(new) = nkeys(old);

/* put info about all records up to new one in new header */
	for (i = 0; i < lo; i++) {
		rkeys(new, i) = rkeys(old, i);
		lens(new, i) = lens(old, i);
		offs(new, i) = off;
		off += lens(old, i);
	}

/* put info about added record in new header; may be new record */
	rkeys(new, lo) = rkey;
	lens(new, lo) = len;
	offs(new, lo) = off;
	off += len;

/* put info about all records after new one in new header */
	if (found)
		j = 0, i++;
	else
		j = 1;
	for (; i < n; i++) {
		rkeys(new, i + j) = rkeys(old, i);
		lens(new, i + j) = lens(old, i);
		offs(new, i + j) = off;
		off += lens(old, i);
	}
	if (!found) nkeys(new) = n + 1;

/* must rewrite data block with new record; open original and new */
	sprintf(scratch1, "%s/%s", bbasedir(btree), fkey2path(iself(old)));
	ASSERT(fo = fopen(scratch1, LLREADBINARY));
	sprintf(scratch1, "%s/tmp1", bbasedir(btree));
	ASSERT(fn1 = fopen(scratch1, LLWRITEBINARY));

/* see if new record must cause data block split */
	if (!found && n == NORECS - 1) goto splitting;

/* no split; write new header and preceding records to temp file */
	ASSERT(fwrite(new, BUFLEN, 1, fn1) == 1);
	putheader(btree, new);
	for (i = 0; i < lo; i++) {
		if (fseek(fo, (long)(offs(old, i) + BUFLEN), 0))
			FATAL();
		filecopy(fo, lens(old, i), fn1);
	}

/* write new record to temp file */
	p = record;
	while (len >= BUFLEN) {
		ASSERT(fwrite(p, BUFLEN, 1, fn1) == 1);
		len -= BUFLEN;
		p += BUFLEN;
	}
	if (len && fwrite(p, len, 1, fn1) != 1) FATAL();

/* write rest of records to temp file */
	if (found) i++;
	for ( ; i < n; i++) {
		if (fseek(fo, (long)(offs(old, i)+BUFLEN), 0)) FATAL();
		filecopy(fo, lens(old, i), fn1);
	}

/* make changes permanent in database */
	fclose(fn1);
	fclose(fo);
	sprintf(scratch1, "%s/tmp1", bbasedir(btree));
	sprintf(scratch2, "%s/%s", bbasedir(btree), fkey2path(iself(old)));
	stdfree(old);
	movefiles(scratch1, scratch2);
	return TRUE;	/* return point for non-splitting case */

/* data block must be split for new record; open second temp file */
splitting:
	sprintf(scratch1, "%s/tmp2", bbasedir(btree));
	ASSERT(fn2 = fopen(scratch1, LLWRITEBINARY));

/* write header and 1st half of records; don't worry where new record goes */
	nkeys(new) = n/2;	/* temporary */
	ASSERT(fwrite(new, BUFLEN, 1, fn1) == 1);
	putheader(btree, new);
	for (i = j = 0; j < n/2; j++) {
		if (j == lo) {
			p = record;
			while (len >= BUFLEN) {
				ASSERT(fwrite(p, BUFLEN, 1, fn1) == 1);
				len -= BUFLEN;
				p += BUFLEN;
			}
			if (len && fwrite(p, len, 1, fn1) != 1)
				FATAL();
		} else {
			if (fseek(fo, (long)(offs(old, i) + BUFLEN), 0))
				FATAL();
			filecopy(fo, lens(old, i), fn1);
			i++;
		}
	}

/* create and write new block header */
	nfkey = iself(new);
	parent = iparent(new);
	xtra = crtblock(btree);
	iparent(xtra) = parent;
	off = 0;
	for (k = 0, l = n/2; k < n - n/2 + 1; k++, l++) {
		rkeys(xtra, k) = rkeys(new, l);
		lens(xtra, k) = lens(new, l);
		offs(xtra, k) = off;
		off += lens(new, l);
	}
	nkeys(xtra) = n - n/2 + 1;
	ASSERT(fwrite(xtra, BUFLEN, 1, fn2) == 1);
	putheader(btree, xtra);

/* write second half of records to second temp file */
	for (j = n/2; j <= n; j++) {
		if (j == lo) {
			p = record;
			while (len >= BUFLEN) {
				ASSERT(fwrite(p, BUFLEN, 1, fn2) == 1);
				len -= BUFLEN;
				p += BUFLEN;
			}
			if (len && fwrite(p, len, 1, fn2) != 1)
				FATAL();
		} else {
			if (fseek(fo, (long)(offs(old, i) + BUFLEN), 0))
				FATAL();
			filecopy(fo, lens(old, i), fn2);
			i++;
		}
	}

/* make changes permanent in database */
	fclose(fo);
	fclose(fn1);
	fclose(fn2);
	stdfree(old);
	sprintf(scratch1, "%s/tmp1", bbasedir(btree));
	sprintf(scratch2, "%s/%s", bbasedir(btree), fkey2path(nfkey));
	movefiles(scratch1, scratch2);
	sprintf(scratch1, "%s/tmp2", bbasedir(btree));
	sprintf(scratch2, "%s/%s", bbasedir(btree), fkey2path(iself(xtra)));
	movefiles(scratch1, scratch2);

/* add index of new data block to its parent (may cause more splitting) */
	addkey(btree, parent, rkeys(xtra, 0), iself(xtra));
	return TRUE;
}
/*======================================================
 * filecopy -- Copy record from one data file to another
 *====================================================*/
static void
filecopy (FILE *fo,
          INT len,
          FILE *fn)
{
	char buffer[BUFLEN];
	while (len >= BUFLEN) {
		ASSERT(fread(buffer, BUFLEN, 1, fo) == 1);
		ASSERT(fwrite(buffer, BUFLEN, 1, fn) == 1);
		len -= BUFLEN;
	}
	if (len) {
		ASSERT(fread((char *)buffer, len, 1, fo) == 1);
		ASSERT(fwrite(buffer, len, 1, fn) == 1);
	}
}
/*==================================
 * readrec -- read record from block
 *================================*/
RECORD
readrec(BTREE btree, BLOCK block, INT i, INT *plen)
{
	char scratch[200];
	FILE *fr;
	RECORD record;
	INT len;

	sprintf(scratch, "%s/%s", bbasedir(btree), fkey2path(iself(block)));
	ASSERT(fr = fopen(scratch, LLREADBINARY));
	if (fseek(fr, (long)(offs(block, i) + BUFLEN), 0)) FATAL();
	if ((len = lens(block, i)) == 0) {
		*plen = 0;
		fclose(fr);
		return NULL;
	}
	record = (RECORD) stdalloc(len + 1);
	ASSERT(fread(record, len, 1, fr) == 1);
	fclose(fr);
	record[len] = 0;
	*plen = len;
	return record;
}
/*===================================
 * getrecord -- Get record from BTREE
 *=================================*/
RECORD
getrecord (BTREE btree,
           RKEY rkey,
           INT *plen)
{
	INDEX index;
	SHORT i, n, lo, hi;
	FKEY nfkey;
	BLOCK block;
	BOOLEAN found = FALSE;

#ifdef DEBUG
	llwprintf("GETRECORD: rkey: %s\n", rkey2str(rkey));
#endif
	*plen = 0;
	ASSERT(index = bmaster(btree));

/* search for data block that does/should hold record */
	while (itype(index) == BTINDEXTYPE) {
		n = nkeys(index);
		nfkey = fkeys(index, 0);
		for (i = 1; i <= n; i++) {
			if (ll_strncmp(rkey.r_rkey, rkeys(index, i).r_rkey, 8) < 0)
				break;
			nfkey = fkeys(index, i);
		}
		ASSERT(index = getindex(btree, nfkey));
	}

/* Found block that may hold record - search for key */
	block = (BLOCK) index;
	lo = 0;
	hi = nkeys(block) - 1;
	while (lo <= hi) {
		SHORT md = (lo + hi)/2;
		INT rel = ll_strncmp(rkey.r_rkey, rkeys(block, md).r_rkey, 8);
		if (rel < 0)
			hi = --md;
		else if (rel > 0)
			lo = ++md;
		else {
			found = TRUE;
			lo = md;
			break;
		}
	}
	if (!found) return NULL;

	return readrec(btree, block, lo, plen);
}
/*=======================================
 * movefiles -- Move first file to second
 *=====================================*/
static void
movefiles (STRING from,
           STRING to)
{
	unlink(to);
	rename(from, to);
}
/*====================================================
 * isrecord -- See if there is a record with given key
 *==================================================*/
BOOLEAN
isrecord (BTREE btree,
          RKEY rkey)
{
	INDEX index;
	SHORT i, n, lo, hi;
	FKEY nfkey;
	BLOCK block;

/* search for data block that does/should hold record */
	ASSERT(index = bmaster(btree));
	while (itype(index) == BTINDEXTYPE) {
		n = nkeys(index);
		nfkey = fkeys(index, 0);
		for (i = 1; i <= n; i++) {
			if (ll_strncmp(rkey.r_rkey, rkeys(index, i).r_rkey, 8) < 0)
				break;
			nfkey = fkeys(index, i);
		}
		ASSERT(index = getindex(btree, nfkey));
	}

/* Found block that may hold record - search for key */
	block = (BLOCK) index;
	lo = 0;
	hi = nkeys(block) - 1;
	while (lo <= hi) {
		SHORT md = (lo + hi)/2;
		INT rel = ll_strncmp(rkey.r_rkey, rkeys(block, md).r_rkey, 8);
		if (rel < 0)
			hi = --md;
		else if (rel > 0)
			lo = ++md;
		else
			return TRUE;
	}
	return FALSE;
}
