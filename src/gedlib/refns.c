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
 * refns.c -- Handle user reference indexing
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 13 Dec 94    3.0.3 - 20 Jan 96
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "btree.h"
#include "translat.h"
#include "gedcom.h"

static RKEY refn2rkey(STRING);
static BOOLEAN resolve_traverse(NODE);

extern BTREE BTR;

/*=========================================================================
 * refn records -- User key indexing information is kept in the database in
 *   refn records; all records with user keys starting with the same first
 *   two characters are indexed together
 *=========================================================================
 * database record format -- The first INT of the record holds the
 *   number of refns indexed in the record
 *-------------------------------------------------------------------
 *        1 INT  nrefns  - number of refns indexed in this record
 *   nrefns RKEY rkeys   - RKEYs of the INDI records with the refns
 *   nrefns INT  noffs   - offsets into following strings where refns
 *			   begin
 *   nrefns STRING refns - char buffer where the refns are stored
 *			   based on char offsets
 *-------------------------------------------------------------------
 * internal format -- At any time there can be only one refn record
 *   stored internally; the data is stored in global data structures
 *-------------------------------------------------------------------
 *   RKEY    RRkey   - RKEY of the current refn record
 *   STRING  RRrec   - current refn record
 *   INT     RRsize  - size of current refn record
 *   INT     RRcount - number of entries in current refn record
 *   INT    *RRoffs  - char offsets to refnl in current refn record
 *   RKEY   *RRkeys  - RKEYs of the INDI records with the refn
 *   STRING *RRrefns - refn values from INDI records that the
 *			  index is based upon
 *   INT     RRmax   - max allocation size of internal arrays
 *-------------------------------------------------------------------
 * When a refn record is used to match a search refn, the internal
 *   structures are modified to remove all entries that don't match
 *   the refn; in addition, other global data structures are used
 *-------------------------------------------------------------------
 *   STRING *RMkeys  - keys (strings) of all INDI records that match
 *   INT     RMcount - number of entries in modified record arrays
 *   INT     RMmax   - max allocation size of RMkeys array
 *=================================================================*/

static RKEY    RRkey;
static STRING  RRrec = NULL;
static INT     RRsize;
static INT     RRcount;
static INT    *RRoffs;
static RKEY   *RRkeys;
static STRING *RRrefns;
static INT     RRmax = 0;

static STRING *RMkeys = NULL;
static INT     RMcount = 0;
static INT     RMmax = 0;

/*====================================================
 * parserefnrec -- Store refn rec in file buffers
 *==================================================*/
static void
parserefnrec (RKEY rkey, STRING p)
{
	INT i;
	RRkey = rkey;
/* Store refn record in data structures */
	memcpy (&RRcount, p, sizeof(INT));
	p += sizeof(INT);
	if (RRcount >= RRmax - 1) {
		if (RRmax != 0) {
			stdfree(RRkeys);
			stdfree(RRoffs);
			stdfree(RRrefns);
		}
		RRmax = RRcount + 10;
		RRkeys = (RKEY *) stdalloc((RRmax)*sizeof(RKEY));
		RRoffs = (INT *) stdalloc((RRmax)*sizeof(INT));
		RRrefns = (STRING *) stdalloc((RRmax)*sizeof(STRING));
	}
	for (i = 0; i < RRcount; i++) {
		memcpy(&RRkeys[i], p, sizeof(RKEY));
		p += sizeof(RKEY);
	}
	for (i = 0; i < RRcount; i++) {
		memcpy(&RRoffs[i], p, sizeof(INT));
		p += sizeof(INT);
	}
	for (i = 0; i < RRcount; i++)
		RRrefns[i] = p + RRoffs[i];
}
/*====================================================
 * getrefnrec -- Read refn record and store in globals
 *==================================================*/
BOOLEAN
getrefnrec (STRING refn)
{
	STRING p;
/* Convert refn to key and read refn record */
	RRkey = refn2rkey(refn);
	if (RRrec) stdfree(RRrec);
	p = RRrec = (STRING) getrecord(BTR, RRkey, &RRsize);
	if (!RRrec) {
		RRcount = 0;
		if (RRmax == 0) {
			RRmax = 10;
			RRkeys = (RKEY *) stdalloc(10*sizeof(RKEY));
			RRoffs = (INT *) stdalloc(10*sizeof(INT));
			RRrefns = (STRING *) stdalloc(10*sizeof(STRING));
		}
		return FALSE;
	}
	parserefnrec(RRkey, p);
	return TRUE;
}
/*============================================
 * refn2rkey - Convert refn to refn record key
 *==========================================*/
static RKEY
refn2rkey (STRING refn)
{
	RKEY rkey;
	rkey.r_rkey[0] = rkey.r_rkey[1] = ' ';
	rkey.r_rkey[2] = rkey.r_rkey[3] = ' ';
	rkey.r_rkey[4] = ' ';
	rkey.r_rkey[5] = 'R';
	rkey.r_rkey[6] = *refn++;
	rkey.r_rkey[7] = *refn;
	return rkey;
}
/*=======================================
 * refn_lo - Lower limit for name records
 *=====================================*/
static RKEY
refn_lo (void)
{
	RKEY rkey;
	INT i;
	for (i=0; i<8; i++)
		rkey.r_rkey[i] = ' ';
	rkey.r_rkey[5] = 'R';
	return rkey;
}
/*=======================================
 * refn_hi - Upper limit for name records
 *=====================================*/
static RKEY
refn_hi (void)
{
	RKEY rkey;
	INT i;
	for (i=0; i<8; i++)
		rkey.r_rkey[i] = ' ';
	rkey.r_rkey[5] = 'S';
	return rkey;
}
/*=========================================
 * add_refn -- Add new entry to refn record
 *=======================================*/
BOOLEAN
add_refn (STRING refn,  /* record's user refn key */
          STRING key)   /* record's GEDCOM key */
{
	STRING rec, p;
	INT i, len, off;
	RKEY rkey;

	rkey = str2rkey(key);
	(void) getrefnrec(refn);
	for (i = 0; i < RRcount; i++) {
		if (!ll_strncmp(rkey.r_rkey, RRkeys[i].r_rkey, 8) &&
		    eqstr(refn, RRrefns[i]))
			return TRUE;
	}
	RRkeys[RRcount] = rkey;
	RRrefns[RRcount] = refn;
	RRcount++;
	p = rec = (STRING) stdalloc(RRsize + sizeof(RKEY) +
	    sizeof(INT) + strlen(refn) + 10);
	len = 0;
	memcpy(p, &RRcount, sizeof(INT));
	p += sizeof(INT);
	len += sizeof(INT);
	for (i = 0; i < RRcount; i++) {
		memcpy(p, &RRkeys[i], sizeof(RKEY));
		p += sizeof(RKEY);
		len += sizeof(RKEY);
	}
	off = 0;
	for (i = 0; i < RRcount; i++) {
		memcpy(p, &off, sizeof(INT));
		p += sizeof(INT);
		len += sizeof(INT);
		off += strlen(RRrefns[i]) + 1;
	}
	for (i = 0; i < RRcount; i++) {
		memcpy(p, RRrefns[i], strlen(RRrefns[i]) + 1);
		p += strlen(RRrefns[i]) + 1;
		len += strlen(RRrefns[i]) + 1;
	}
	addrecord(BTR, RRkey, rec, len);
	stdfree(rec);
	return TRUE;
}
/*=============================================
 * remove_refn -- Remove entry from refn record
 *===========================================*/
BOOLEAN
remove_refn (STRING refn,       /* record's refn */
             STRING key)        /* record's GEDCOM key */
{
	STRING rec, p;
	INT i, len, off;
	BOOLEAN found;
	RKEY rkey;
	rkey = str2rkey(key);
	(void) getrefnrec(refn);
	found = FALSE;
	for (i = 0; i < RRcount; i++) {
		if (!ll_strncmp(rkey.r_rkey, RRkeys[i].r_rkey, 8) &&
		    eqstr(refn, RRrefns[i])) {
			found = TRUE;
			break;
		}
	}
	if (!found) return FALSE;
	RRcount--;
	for ( ; i < RRcount; i++) {
		RRkeys[i] = RRkeys[i+1];
		RRrefns[i] = RRrefns[i+1];
	}
	p = rec = (STRING) stdalloc(RRsize);
	len = 0;
	memcpy(p, &RRcount, sizeof(INT));
	p += sizeof(INT);
	len += sizeof(INT);
	for (i = 0; i < RRcount; i++) {
		memcpy(p, &RRkeys[i], sizeof(RKEY));
		p += sizeof(RKEY);
		len += sizeof(RKEY);
	}
	off = 0;
	for (i = 0; i < RRcount; i++) {
		memcpy(p, &off, sizeof(INT));
		p += sizeof(INT);
		len += sizeof(INT);
		off += strlen(RRrefns[i]) + 1;
	}
	for (i = 0; i < RRcount; i++) {
		memcpy(p, RRrefns[i], strlen(RRrefns[i]) + 1);
		p += strlen(RRrefns[i]) + 1;
		len += strlen(RRrefns[i]) + 1;
	}
	addrecord(BTR, RRkey, rec, len);
	stdfree(rec);
	return TRUE;
}
/*====================================================
 * get_refns -- Find all records who match refn or key
 *==================================================*/
void
get_refns (STRING refn,
           INT *pnum,
           STRING **pkeys,
           INT letr)
{
	INT i, n;

	*pnum = 0;
	if (!refn) return;

/* Clean up allocated memory from last call */

	if (RMcount) {
		for (i = 0; i < RMcount; i++)
			stdfree(RMkeys[i]);
	}
	RMcount = 0;

/* Load static refn buffers; return if no match */

	if (!getrefnrec(refn)) return;

/* Compare user's refn with all refns in record; the refn
      record data structures are modified */

	n = 0;
	for (i = 0; i < RRcount; i++) {
		if (eqstr(refn, RRrefns[i])) {
			if (letr == 0 || *(rkey2str(RRkeys[i])) == letr) {
				if (i != n) {
					RRrefns[n] = RRrefns[i];
					RRkeys[n] = RRkeys[i];
				}
				n++;
			}
		}
	}
	*pnum = RRcount = n;
	if (RRcount > RMmax) {
		if (RMmax) stdfree(RMkeys);
		RMkeys = (STRING *) stdalloc(RRcount*sizeof(STRING));
		RMmax = RRcount;
	}
	for (i = 0; i < RRcount; i++)
		RMkeys[i] = strsave(rkey2str(RRkeys[i]));
	*pkeys = RMkeys;
}
/*==========================================================
 * resolve_links -- Resolve and check all links in node tree
 *========================================================*/
static BOOLEAN unresolved;
void
resolve_links (NODE node)
{
	tlineno = 0;
	unresolved = FALSE;
	if (!node) return;
	traverse_nodes(node, resolve_traverse);
}
/*=======================================================
 * resolve_traverse -- Traverse routine for resolve_links
 *=====================================================*/
static BOOLEAN
resolve_traverse (NODE node)
{
	STRING refn, val = nval(node);
	INT letr;
	NODE refr;

	if (!val) return TRUE;
	if (symbolic_link(val)) {
		refn = rmvbrackets(val);
		letr = record_letter(ntag(node));
		refr = refn_to_record(refn, letr);
		if (refr) {
			stdfree(nval(node));
			nval(node) = strsave(nxref(refr));
		}
	}
	return TRUE;
}
/*===============================================
 * symbolic_link -- See if value is symbolic link
 *=============================================*/
BOOLEAN
symbolic_link (STRING val)
{
	if (!val || *val != '<' || strlen(val) < 3) return FALSE;
	return val[strlen(val)-1] == '>';
}
/*===============================================
 * record_letter -- Return letter for record type
 *=============================================*/
INT
record_letter (STRING tag)
{
	if (eqstr("FATH", tag)) return 'I';
	if (eqstr("MOTH", tag)) return 'I';
	if (eqstr("HUSB", tag)) return 'I';
	if (eqstr("WIFE", tag)) return 'I';
	if (eqstr("INDI", tag)) return 'I';
	if (eqstr("CHIL", tag)) return 'I';
	if (eqstr("FAMC", tag)) return 'F';
	if (eqstr("FAMS", tag)) return 'F';
	if (eqstr("FAM",  tag)) return 'F';
	if (eqstr("SOUR", tag)) return 'S';
	if (eqstr("EVEN", tag)) return 'E';
	if (eqstr("EVID", tag)) return 'E';
	return 0;
}
/*=========================================
 * key_to_record -- Returns record with key
 *=======================================*/
NOD0 key_to_record (STRING str, /* string that may be a key */
                    INT let)    /* if string starts with letter it
                                   must be this */
{
	char kbuf[MAXNAMELEN];
	INT i = 0, c;

	if (!str || *str == 0) return NULL;
	c = *str++;
	if (c != let && chartype(c) != DIGIT) return NULL;
	kbuf[i++] = let;
	if (c != let) kbuf[i++] = c;
	while ((c = *str++) && chartype(c) == DIGIT)
		kbuf[i++] = c;
	if (c != 0) return NULL;
	kbuf[i] = 0;
	if (!isrecord(BTR, str2rkey(kbuf))) return NULL;
	switch (let) {
	case 'I': return key_to_indi0(kbuf);
	case 'F': return key_to_fam0(kbuf);
	case 'S': return key_to_sour0(kbuf);
	case 'E': return key_to_even0(kbuf);
	case 'X': return key_to_othr0(kbuf);
	default:  FATAL();
	}
	FATAL();
	return NULL;
}
/*================================================
 * refn_to_record - Get record from user reference
 *==============================================*/
NODE
refn_to_record (STRING ukey,    /* user refn key */
                INT letr)       /* type of record */
{
	STRING *keys;
	INT num;

	if (!ukey || *ukey == 0) return NULL;
	get_refns(ukey, &num, &keys, letr);
	if (!num) return NULL;
	return nztop(key_to_record(keys[0], *keys[0]));
}
/*===============================================
 * index_by_refn - Index node tree by REFN values
 *=============================================*/
void
index_by_refn (NODE node,
               STRING key)
{
	if (!node || !key) return;
	for (node = nchild(node); node; node = nsibling(node)) {
		if (eqstr("REFN", ntag(node)) && nval(node))
			add_refn(nval(node), key);
	}
}
/*====================================================
 * traverse_refns -- traverse refns in db
 *  delegates to traverse_db_rec_rkeys
 *   passing callback function: traverse_refn_callback
 *   and using local data in a TRAV_REFN_PARAM
 *==================================================*/
typedef struct
{
	BOOLEAN(*func)(STRING key, STRING refn, void *param);
	void * param;
} TRAV_REFN_PARAM;
/* see above */
static BOOLEAN
traverse_refn_callback (RKEY rkey, STRING data, INT len, void *param)
{
	TRAV_REFN_PARAM *tparam = (TRAV_REFN_PARAM *)param;
	INT i;

	parserefnrec(rkey, data);

	for (i=0; i<RRcount; i++)
	{
		if (!tparam->func(rkey2str(RRkeys[i]), RRrefns[i], tparam->param))
			return FALSE;
	}
	return TRUE;
}
/* see above */
void
traverse_refns (BOOLEAN(*func)(STRING key, STRING refn, void *param), void *param)
{
	TRAV_REFN_PARAM tparam;
	tparam.param = param;
	tparam.func = func;
	traverse_db_rec_rkeys(BTR, refn_lo(), refn_hi(), &traverse_refn_callback, &tparam);
}
