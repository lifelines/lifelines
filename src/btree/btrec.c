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
 * btrec.c -- Routines to handle BTREE records
 * (formerly btree/record.c)
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Jul 93
 *   3.0.0 - 24 Sep 94    3.0.2 - 26 Mar 95
 *   3.0.3 - 07 May 95
 *===========================================================*/

#include <errno.h>
#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#include "btreei.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void check_offset(BLOCK block, RKEY rkey, INT i);
static void filecopy(FILE*fpsrc, INT len, FILE*fpdest);
static void movefiles(STRING, STRING);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=================================
 * check_offset -- Sanity check one entry (off & len)
 *  block:  [IN]  block in memory
 *  rkey:   [IN]  key of record (eg, "     I67")
 *  inum:   [IN]  record index
 *===============================*/
static void
check_offset (BLOCK block, RKEY rkey, INT i)
{
	INT offlo, lenlo, offhi;

	if (!(i>=0 && i<nkeys(block))) {
		char msg[256];
		sprintf(msg, "Working on rkey=%s ", rkey2str(rkey));
		sprintf(msg+strlen(msg), "Bad index (%ld) passed to check_offset", i);
		FATAL2(msg);
	}
	
	offlo = offs(block, i-1);
	lenlo = lens(block, i-1);
	offhi = offs(block, i);

	if (offlo + lenlo != offhi) {
		char msg[256];
		INT blocknum = ixself(block);
		sprintf(msg, "Working on rkey=%s ", rkey2str(rkey));
		sprintf(msg+strlen(msg)
			, "Found corrupt block#%lx: index(%ld) off=%ld, len=%ld, key(%ld) off=%ld"
			, blocknum, i-1, offlo, lenlo, i, offhi);
		FATAL2(msg);
	}
}

/*=================================
 * bt_addrecord -- Add record to BTREE
 *  btree:  [in] btree to add record to
 *  rkey:   [in] key of record (eg, "     I67")
 *  record: [in] record data to add
 *  len:    [in] record length
 *===============================*/
BOOLEAN
bt_addrecord (BTREE btree, RKEY rkey, RAWRECORD rec, INT len)
{
	INDEX index;
	BLOCK old, newb, xtra;
	FKEY nfkey, last = 0, parent;
	SHORT i, j, k, l, n, lo, hi;
	BOOLEAN found = FALSE;
	INT off = 0;
	FILE *fo=NULL, *ft1=NULL, *ft2=NULL;
	char scratch0[MAXPATHLEN], scratch1[MAXPATHLEN], scratch2[MAXPATHLEN];
	char *p = rec;

/* search for data block that does/should hold record */
	ASSERT(bwrite(btree));
	ASSERT(index = bmaster(btree));
	while (ixtype(index) == BTINDEXTYPE) {

/* maintain "lazy" parent chaining in btree */
		if (ixparent(index) != last) {
			ASSERT(index != bmaster(btree));
			ixparent(index) = last;
			writeindex(btree, index);
		}
		last = ixself(index);
		n = nkeys(index);
		nfkey = fkeys(index, 0);
		for (i = 1; i <= n; i++) {
			if (cmpkeys(&rkey, &rkeys(index, i)) < 0)
				break;
			nfkey = fkeys(index, i);
		}
		index = getindex(btree, nfkey);
	}
/* have block that may hold older version of record */
	ixparent(index) = last;
	old = (BLOCK) index;
	if (!(nkeys(old) < NORECS)) {
		char msg[256];
		sprintf(msg, "Corrupt block (rkey=%s): nkeys (%d) exceeds maximum (%d)"
			, rkey2str(rkey), nkeys(old), NORECS-1);
		FATAL2(msg);
	}

/* see if block has earlier version of record */
	lo = 0;
	hi = nkeys(old) - 1;
	found = FALSE;
	while (lo <= hi) {
		SHORT md = (lo + hi)/2;
		INT rel = cmpkeys(&rkey, &rkeys(old, md));
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
	newb = allocblock();
	ixtype(newb) = ixtype(old);
	ixparent(newb) = ixparent(old);
	ixself(newb) = ixself(old);
	n = nkeys(newb) = nkeys(old);

/* put info about all records up to new one in new header */
	for (i = 0; i < lo; i++) {
		rkeys(newb, i) = rkeys(old, i);
		lens(newb, i) = lens(old, i);
		offs(newb, i) = off;
		off += lens(old, i);
		if (i) { /* sanity check */
			check_offset(newb, rkey, i-1);
		}
	}

/* put info about added record in new header; may be new record */
	rkeys(newb, lo) = rkey;
	lens(newb, lo) = len;
	offs(newb, lo) = off;
	off += len;

/* put info about all records after new one in new header */
	if (found)
		j = 0, i++;
	else
		j = 1;
	for (; i < n; i++) {
		rkeys(newb, i + j) = rkeys(old, i);
		lens(newb, i + j) = lens(old, i);
		offs(newb, i + j) = off;
		off += lens(old, i);
		if (i+j) { /* sanity check */
			check_offset(newb, rkey, i+j-1);
		}
	}
	if (!found) nkeys(newb) = n + 1;

/* must rewrite data block with new record; open original and new */
	sprintf(scratch0, "%s/%s", bbasedir(btree), fkey2path(ixself(old)));
	if (!(fo = fopen(scratch0, LLREADBINARY LLFILERANDOM))) {
		char msg[sizeof(scratch0)+64];
		sprintf(msg, "Corrupt db (rkey=%s) -- failed to open blockfile: %s"
			, rkey2str(rkey), scratch0);
		FATAL2(msg);
	}
	sprintf(scratch1, "%s/tmp1", bbasedir(btree));
	if (!(ft1 = fopen(scratch1, LLWRITEBINARY LLFILETEMP LLFILERANDOM))) {
		char msg[sizeof(scratch1)+64];
		sprintf(msg, "Corrupt db (rkey=%s) -- failed to open temp blockfile: %s"
			, rkey2str(rkey), scratch1);
		FATAL2(msg);
	}

/* see if new record must cause data block split */
	if (!found && n == NORECS - 1) goto splitting;

/* no split; write new header and preceding records to temp file */
	ASSERT(fwrite(newb, BUFLEN, 1, ft1) == 1);
	putheader(btree, newb);
	for (i = 0; i < lo; i++) {
		if (fseek(fo, (long)(offs(old, i) + BUFLEN), 0))
			FATAL();
		filecopy(fo, lens(old, i), ft1);
	}

/* write new record to temp file */
	p = rec;
	while (len >= BUFLEN) {
		ASSERT(fwrite(p, BUFLEN, 1, ft1) == 1);
		len -= BUFLEN;
		p += BUFLEN;
	}
	if (len) {
		CHECKED_fwrite(p, len, 1, ft1, scratch1);
	}

/* write rest of records to temp file */
	if (found) i++;
	for ( ; i < n; i++) {
		if (fseek(fo, (long)(offs(old, i)+BUFLEN), 0)) FATAL();
		filecopy(fo, lens(old, i), ft1);
	}

/* make changes permanent in database */
	CHECKED_fclose(ft1, scratch1);
	fclose(fo); /* was opened read-only */
	sprintf(scratch0, "%s/tmp1", bbasedir(btree));
	sprintf(scratch1, "%s/%s", bbasedir(btree), fkey2path(ixself(old)));
	stdfree(old);
	movefiles(scratch0, scratch1);
	return TRUE;	/* return point for non-splitting case */

/* data block must be split for new record; open second temp file */
splitting:
	sprintf(scratch2, "%s/tmp2", bbasedir(btree));
	ASSERT(ft2 = fopen(scratch2, LLWRITEBINARY LLFILETEMP LLFILERANDOM));

/* write header and 1st half of records; don't worry where new record goes */
	nkeys(newb) = n/2;	/* temporary */
	ASSERT(fwrite(newb, BUFLEN, 1, ft1) == 1);
	putheader(btree, newb);
	for (i = j = 0; j < n/2; j++) {
		if (j == lo) {
			p = rec;
			while (len >= BUFLEN) {
				ASSERT(fwrite(p, BUFLEN, 1, ft1) == 1);
				len -= BUFLEN;
				p += BUFLEN;
			}
			if (len && fwrite(p, len, 1, ft1) != 1)
				FATAL();
		} else {
			if (fseek(fo, (long)(offs(old, i) + BUFLEN), 0))
				FATAL();
			filecopy(fo, lens(old, i), ft1);
			i++;
		}
	}

/* create and write new block header */
	nfkey = ixself(newb);
	parent = ixparent(newb);
	xtra = crtblock(btree);
	ixparent(xtra) = parent;
	off = 0;
	for (k = 0, l = n/2; k < n - n/2 + 1; k++, l++) {
		rkeys(xtra, k) = rkeys(newb, l);
		lens(xtra, k) = lens(newb, l);
		offs(xtra, k) = off;
		off += lens(newb, l);
	}
	nkeys(xtra) = n - n/2 + 1;
	ASSERT(fwrite(xtra, BUFLEN, 1, ft2) == 1);
	putheader(btree, xtra);

/* write second half of records to second temp file */
	for (j = n/2; j <= n; j++) {
		if (j == lo) {
			p = rec;
			while (len >= BUFLEN) {
				ASSERT(fwrite(p, BUFLEN, 1, ft2) == 1);
				len -= BUFLEN;
				p += BUFLEN;
			}
			if (len && fwrite(p, len, 1, ft2) != 1)
				FATAL();
		} else {
			if (fseek(fo, (long)(offs(old, i) + BUFLEN), 0))
				FATAL();
			filecopy(fo, lens(old, i), ft2);
			i++;
		}
	}

/* make changes permanent in database */
	fclose(fo); /* was opened read-only */
	CHECKED_fclose(ft1, scratch1);
	CHECKED_fclose(ft2, scratch2);
	stdfree(old);
	sprintf(scratch1, "%s/tmp1", bbasedir(btree));
	sprintf(scratch2, "%s/%s", bbasedir(btree), fkey2path(nfkey));
	movefiles(scratch1, scratch2);
	sprintf(scratch1, "%s/tmp2", bbasedir(btree));
	sprintf(scratch2, "%s/%s", bbasedir(btree), fkey2path(ixself(xtra)));
	movefiles(scratch1, scratch2);

/* add index of new data block to its parent (may cause more splitting) */
	addkey(btree, parent, rkeys(xtra, 0), ixself(xtra));
	return TRUE;
}
/*======================================================
 * filecopy -- Copy record from one data file to another
 * Copy from source file (already opened) to destination
 * file (already opened).
 *====================================================*/
static void
filecopy (FILE* fpsrc, INT len, FILE* fpdest)
{
	char buffer[BUFLEN];
	INT blklen;
	while (len) {
		/* copy BUFLEN at a time til the last little bit */
		/* assumes full buffer copy each time, so only appropriate
		for binary file copies (because of \r\n translation on Win32) */
		blklen = (len > BUFLEN) ? BUFLEN : len;
		ASSERT(fread(buffer, blklen, 1, fpsrc) == 1);
		ASSERT(fwrite(buffer, blklen, 1, fpdest) == 1);
		len -= blklen;
	}
}
/*==================================
 * readrec -- read record from block
 *  btree: [in]  database pointer
 *  block: [in]  block of data from disk (leaf of btree)
 *  i:     [in]  index in leaf block desired
 *  plen:  [out] length of returned data
 *================================*/
RAWRECORD
readrec (BTREE btree, BLOCK block, INT i, INT *plen)
{
	char scratch[MAXPATHLEN];
	FILE *fd=NULL;
	RAWRECORD rawrec;
	INT len;

	snprintf(scratch, sizeof(scratch)
		, "%s%c%s"
		, bbasedir(btree), LLCHRDIRSEPARATOR, fkey2path(ixself(block)));
	if (!(fd = fopen(scratch, LLREADBINARY LLFILERANDOM))) {
		char msg[sizeof(scratch)+64];
		sprintf(msg, _("Failed (errno=%d) to open blockfile (rkey=%s): %s")
			, errno, rkey2str(rkeys(block, i)), scratch);
		FATAL2(msg);
	}
	if (fseek(fd, (long)(offs(block, i) + BUFLEN), 0)) {
		char msg[sizeof(scratch)+64];
		sprintf(msg, "Seek to offset (%ld) failed for blockfile (rkey=%s)"
			, offs(block,i), rkey2str(rkeys(block, i)));
		FATAL2(msg);
	}
	if ((len = lens(block, i)) == 0) {
		*plen = 0;
		fclose(fd); /* readonly */
		return NULL;
	}
	if (len < 0) {
		char msg[sizeof(scratch)+64];
		sprintf(msg, "Bad len (%ld) for blockfile (rkey=%s)"
			, len, rkey2str(rkeys(block, i)));
		FATAL2(msg);
	}
	rawrec = (RAWRECORD) stdalloc(len + 1);
	if (!(fread(rawrec, len, 1, fd) == 1)) {
		char msg[sizeof(scratch)+64];
		sprintf(msg, "Read for %ld bytes failed for blockfile (rkey=%s)"
			, len, rkey2str(rkeys(block, i)));
		FATAL2(msg);
	}
	fclose(fd); /* readonly */
	rawrec[len] = 0;
	*plen = len;
	return rawrec;
}
/*===================================
 * bt_getrecord -- Get record from BTREE
 * (ignore deleted records)
 *  btree: [in]  database pointer
 *  rkey:  [in]  key to data (eg, "    I313")
 *  plen:  [out] length of returned data
 *=================================*/
RAWRECORD
bt_getrecord (BTREE btree, const RKEY * rkey, INT *plen)
{
	INDEX index;
	SHORT i, n, lo, hi;
	FKEY nfkey;
	BLOCK block;
	BOOLEAN found = FALSE;
	RAWRECORD rawrec;

#ifdef DEBUG
	llwprintf("GETRECORD: rkey: %s\n", rkey2str(*rkey));
#endif
	*plen = 0;
	ASSERT(index = bmaster(btree));

/* search for data block that does/should hold record */
	while (ixtype(index) == BTINDEXTYPE) {
		n = nkeys(index);
		nfkey = fkeys(index, 0);
		for (i = 1; i <= n; i++) {
			if (cmpkeys(rkey, &rkeys(index, i)) < 0)
				break;
			nfkey = fkeys(index, i);
		}
		index = getindex(btree, nfkey);
		/* should never revisit the master node */
		if (ixself(index) == ixself(bmaster(btree))) {
			char msg[400];
			sprintf(msg, _("Btree lookup looped back to master (%ld)!"), ixself(index));

			FATAL2(msg);
		}
	}

/* Found block that may hold record - search for key */
	block = (BLOCK) index;
	lo = 0;
	hi = nkeys(block) - 1;
	while (lo <= hi) {
		SHORT md = (lo + hi)/2;
		INT rel = cmpkeys(rkey, &rkeys(block, md));
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

	rawrec = readrec(btree, block, lo, plen);
	if (rawrec && !strcmp(rawrec, "DELE\n")) {
		stdfree(rawrec);
		rawrec=NULL;
	}
	return rawrec;
}
/*=======================================
 * movefiles -- Move first file to second
 * failure handled with FATAL2 macro, which exits
 *=====================================*/
static void
movefiles (STRING from_file, STRING to_file)
{
	INT rtn;
	unlink(to_file);
	rtn = rename(from_file, to_file);
	if (rtn) {
		char temp[1024];
		snprintf(temp, sizeof(temp),
			"rename failed code %ld, from <%s> to <%s>",
			rtn, from_file, to_file);
		FATAL2(temp);
	}
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
	while (ixtype(index) == BTINDEXTYPE) {
		n = nkeys(index);
		nfkey = fkeys(index, 0);
		for (i = 1; i <= n; i++) {
			if (cmpkeys(&rkey, &rkeys(index, i)) < 0)
				break;
			nfkey = fkeys(index, i);
		}
		index = getindex(btree, nfkey);
	}

/* Found block that may hold record - search for key */
	block = (BLOCK) index;
	lo = 0;
	hi = nkeys(block) - 1;
	while (lo <= hi) {
		SHORT md = (lo + hi)/2;
		INT rel = cmpkeys(&rkey, &rkeys(block, md));
		if (rel < 0)
			hi = --md;
		else if (rel > 0)
			lo = ++md;
		else
			return TRUE;
	}
	return FALSE;
}
/*====================================================
 * cmpkeys -- Compare two keys of btree
 *==================================================*/
INT
cmpkeys (const RKEY * rk1, const RKEY * rk2)
{
	INT rel = ll_strncmp(rk1->r_rkey, rk2->r_rkey, 8);
	return rel;
}

