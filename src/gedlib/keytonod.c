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
/* modified 06 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * keytonod.c -- Convert between keys and node trees
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 08 May 94    3.0.2 - 23 Dec 94
 *   3.0.3 - 16 Jul 95
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "liflines.h"
#include "feedback.h"

/*********************************************
 * global variables (no header)
 *********************************************/

char badkeylist[100] = "";
int listbadkeys = 0;

/*********************************************
 * local function prototypes
 *********************************************/

static void add_record_to_direct(CACHE cache, RECORD rec, STRING key);
static CACHE create_cache(STRING name, INT dirsize, INT indsize);
static void dereference(CACHEEL);
static CACHEEL key_to_cacheel(CACHE cache, CNSTRING key, STRING tag, INT reportmode);
static CACHEEL key_to_even_cacheel(CNSTRING key);
static NODE key_typed_to_node(CACHE cache, CNSTRING key, STRING tag);
static RECORD key_to_record_impl(CNSTRING key, INT reportmode);
static RECORD key_typed_to_record(CACHE cache, CNSTRING key, STRING tag);
static CACHEEL key_to_othr_cacheel(CNSTRING key);
static CACHEEL key_to_sour_cacheel(CNSTRING key);
static void prepare_direct_space(CACHE cache);
static NODE qkey_to_node(CACHE cache, CNSTRING key, STRING tag);
static RECORD qkey_typed_to_record(CACHE cache, CNSTRING key, STRING tag);
static void record_to_cache(CACHE cache, RECORD rec);


INT csz_indi = 200;		/* cache size for indi */
INT icsz_indi = 3000;		/* indirect cache size for indi */
INT csz_fam = 200;		/* cache size for fam */
INT icsz_fam = 2000;		/* indirect cache size for fam */
INT csz_sour = 200;		/* cache size for sour */
INT icsz_sour = 2000;		/* indirect cache size for sour */
INT csz_even = 200;		/* cache size for even */
INT icsz_even = 2000;		/* indirect cache size for even */
INT csz_othr = 200;		/* cache size for othr */
INT icsz_othr = 2000;		/* indirect cache size for othr */

/*********************************************
 * local variables
 *********************************************/

static CACHE indicache, famcache, evencache, sourcache, othrcache;

static char keybuf[10][32] = { "", "", "", "", "", "", "", "", "", ""};
static int keyidx = 0;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*================================================
 * node_to_record -- Find record from node
 *==============================================*/
RECORD
node_to_record (NODE node)
{
	STRING key = rmvat(nxref(node));
	return key_to_record(key);
}
/*================================================
 * keynum_to_indi -- Convert a numeric key to an indi node
 * assert if failed (ie, no indi with that number)
 *==============================================*/
NODE
keynum_to_indi (int keynum)
{
	char keystr[20];
	sprintf(keystr,"I%d",keynum);
	return key_to_indi(keystr);
}
RECORD
keynum_to_irecord (int keynum)
{
	char keystr[20];
	sprintf(keystr,"I%d",keynum);
	return key_to_irecord(keystr);
}
/*=========================================================
 * qkeynum_to_indi -- Convert a numeric key to an indi node
 *  report mode - it returns NULL if failed
 *  (ie, no indi with that number)
 *=======================================================*/
NODE
qkeynum_to_indi (int keynum)
{
	char keystr[20];
	sprintf(keystr,"I%d",keynum);
	return qkey_to_indi(keystr);
}
/*================================================
 * keynum_to_fam -- Convert a numeric key to a fam node
 *  assert if failed (ie, no fam with that number)
 *==============================================*/
NODE
keynum_to_fam (int keynum)
{
	char keystr[20];
	sprintf(keystr,"F%d",keynum);
	return key_to_fam(keystr);
}
RECORD
keynum_to_frecord (int keynum)
{
	char keystr[20];
	sprintf(keystr,"F%d",keynum);
	return key_to_frecord(keystr);
}
/*======================================================
 * qkeynum_to_frecord -- Convert a numeric key to a fam record
 *  report mode - it returns NULL if failed
 *  (ie, no fam with that number)
 *====================================================*/
RECORD
qkeynum_to_frecord (int keynum)
{
	char keystr[20];
	sprintf(keystr,"F%d",keynum);
	return qkey_to_frecord(keystr);
}
/*================================================
 * keynum_to_sour -- Convert a numeric key to a sour node
 *  assert if failed (ie, no sour with that number)
 *==============================================*/
NODE
keynum_to_sour (int keynum)
{
	char keystr[20];
	sprintf(keystr,"S%d",keynum);
	return key_to_sour(keystr);
}
RECORD
keynum_to_srecord (int keynum)
{
	char keystr[20];
	sprintf(keystr,"S%d",keynum);
	return key_to_srecord(keystr);
}
/*================================================
 * keynum_to_even -- Convert a numeric key to a even node
 *  assert if failed (ie, no sour with that number)
 *==============================================*/
NODE
keynum_to_even (int keynum)
{
	char keystr[MAXKEYWIDTH+1];
	sprintf(keystr,"E%d",keynum);
	return key_to_even(keystr);
}
RECORD
keynum_to_erecord (int keynum)
{
	char keystr[MAXKEYWIDTH+1];
	sprintf(keystr,"E%d",keynum);
	return key_to_erecord(keystr);
}
/*================================================
 * keynum_to_othr -- Convert a numeric key to an other node
 *  assert if failed (ie, no sour with that number)
 *==============================================*/
NODE
keynum_to_othr (int keynum)
{
	char keystr[20];
	sprintf(keystr,"X%d",keynum);
	return key_to_othr(keystr);
}
RECORD
keynum_to_orecord (int keynum)
{
	char keystr[20];
	sprintf(keystr,"X%d",keynum);
	return key_to_orecord(keystr);
}
/*=====================================
 * keynum_to_node -- Convert keynum to node
 *===================================*/
NODE
keynum_to_node (char ntype, int keynum)
{
	switch(ntype) {
	case 'I': return keynum_to_indi(keynum);
	case 'F': return keynum_to_fam(keynum);
	case 'S': return keynum_to_sour(keynum);
	case 'E': return keynum_to_even(keynum);
	case 'X': return keynum_to_othr(keynum);
	}
	ASSERT(0);
	return 0;
}
RECORD
keynum_to_record (char ntype, int keynum)
{
	switch(ntype) {
	case 'I': return keynum_to_irecord(keynum);
	case 'F': return keynum_to_frecord(keynum);
	case 'S': return keynum_to_srecord(keynum);
	case 'E': return keynum_to_erecord(keynum);
	case 'X': return keynum_to_orecord(keynum);
	}
	ASSERT(0);
	return 0;
}
/*=====================================
 * key_to_type -- Convert key to node
 * TO DO - should become obsoleted by key_to_record
 *===================================*/
NODE
key_to_type (CNSTRING key, INT reportmode)
{
	switch(key[0])
	{
	case 'I': return reportmode ? qkey_to_indi(key) : key_to_indi(key);
	case 'F': return reportmode ? qkey_to_fam(key) : key_to_fam(key);
	case 'S': return reportmode ? qkey_to_sour(key) : key_to_sour(key);
	case 'E': return reportmode ? qkey_to_even(key) : key_to_even(key);
	}
	return reportmode ? qkey_to_othr(key) : key_to_othr(key);
}
/*=====================================
 * qkey_to_type -- Convert key to node
 * quiet -- that is, returns NULL if record not in database
 *===================================*/
NODE
qkey_to_type (CNSTRING key)
{
	return key_to_type(key, TRUE);
}
/*=====================================
 * key_to_record_impl -- Convert key (any type) to RECORD
 *===================================*/
static RECORD
key_to_record_impl (CNSTRING key, INT reportmode)
{
	switch(key[0])
	{
	case 'I': return reportmode ? qkey_to_irecord(key) : key_to_irecord(key);
	case 'F': return reportmode ? qkey_to_frecord(key) : key_to_frecord(key);
	case 'S': return reportmode ? qkey_to_srecord(key) : key_to_srecord(key);
	case 'E': return reportmode ? qkey_to_erecord(key) : key_to_erecord(key);
	}
	return reportmode ? qkey_to_orecord(key) : key_to_orecord(key);
}
/*=====================================
 * key_to_record -- Convert key (any type) to RECORD
 * ASSERTS if record not found in database
 *===================================*/
RECORD
key_to_record (CNSTRING key)
{
	return key_to_record_impl(key, FALSE);
}
/*=====================================
 * qkey_to_record -- Convert key (any type) to RECORD
 * quiet -- that is, returns NULL if record not in database
 *===================================*/
RECORD
qkey_to_record (CNSTRING key)
{
	return key_to_record_impl(key, TRUE);
}
/*=====================================
 * key_to_??? -- Convert key to person
 *  (asserts if failure)
 *  5 symmetric versions
 * TO DO - should become obsoleted by key_to_???0
 *===================================*/
NODE
key_to_indi (CNSTRING key)
{
	return key_typed_to_node(indicache, key, "INDI");
}
NODE key_to_fam (CNSTRING key)
{
	return key_typed_to_node(famcache, key, "FAM");
}
NODE key_to_even (CNSTRING key)
{
	return key_typed_to_node(evencache, key, "EVEN");
}
NODE key_to_sour (CNSTRING key)
{
	return key_typed_to_node(sourcache, key, "SOUR");
}
NODE key_to_othr (CNSTRING key)
{
	return key_typed_to_node(othrcache, key, NULL);
}
/*=====================================
 * key_to_???0 -- Convert key to person
 *  (asserts if failure)
 *  5 symmetric versions
 *===================================*/
RECORD key_to_irecord (CNSTRING key)
{
	return key_typed_to_record(indicache, key, "INDI");
}
RECORD key_to_frecord (CNSTRING key)
{
	return key_typed_to_record(famcache, key, "FAM");
}
RECORD key_to_erecord (CNSTRING key)
{
	return key_typed_to_record(evencache, key, "EVEN");
}
RECORD key_to_srecord (CNSTRING key)
{
	return key_typed_to_record(sourcache, key, "SOUR");
}
RECORD key_to_orecord (CNSTRING key)
{
	return key_typed_to_record(othrcache, key, NULL);
}
/*========================================
 * qkey_to_??? -- Convert key to node type
 *  report mode (returns NULL if failure)
 *  5 symmetric versions
 * TO DO - should become obsoleted by key_to_???0
 *======================================*/
NODE qkey_to_indi (CNSTRING key)
{
	return qkey_to_node(indicache, key, "INDI");
}
NODE qkey_to_fam (CNSTRING key)
{
	return qkey_to_node(famcache, key, "FAM");
}
NODE qkey_to_even (CNSTRING key)
{
	return qkey_to_node(evencache, key, "EVEN");
}
NODE qkey_to_sour (CNSTRING key)
{
	return qkey_to_node(sourcache, key, "SOUR");
}
NODE qkey_to_othr (CNSTRING key)
{
	return qkey_to_node(othrcache, key, NULL);
}
/*========================================
 * qkey_to_???0 -- Convert key to node type
 *  report mode (returns NULL if failure)
 *  5 symmetric versions
 *======================================*/
RECORD qkey_to_irecord (CNSTRING key)
{
	return qkey_typed_to_record(indicache, key, "INDI");
}
RECORD qkey_to_frecord (CNSTRING key)
{
	return qkey_typed_to_record(famcache, key, "FAM");
}
RECORD qkey_to_erecord (CNSTRING key)
{
	return qkey_typed_to_record(evencache, key, "EVEN");
}
RECORD qkey_to_srecord (CNSTRING key)
{
	return qkey_typed_to_record(sourcache, key, "SOUR");
}
RECORD qkey_to_orecord (CNSTRING key)
{
	return qkey_typed_to_record(othrcache, key, NULL);
}
/*=====================================================
 * key_to_unknown_cacheel -- Convert any key to cacheel
 *===================================================*/
CACHEEL
key_to_unknown_cacheel (STRING key)
{
	switch(key[0]) {
		case 'I': return key_to_indi_cacheel(key);
		case 'F': return key_to_fam_cacheel(key);
		case 'S': return key_to_sour_cacheel(key);
		case 'E': return key_to_even_cacheel(key);
		default: return key_to_othr_cacheel(key);
	}
}
/*=====================================================
 * key_to_indi_cacheel -- Convert key to person cacheel
 *===================================================*/
CACHEEL
key_to_indi_cacheel (STRING key)
{
	return key_to_cacheel(indicache, key, "INDI", FALSE);
}
/*====================================================
 * key_to_fam_cacheel -- Convert key to family_cacheel
 *==================================================*/
CACHEEL
key_to_fam_cacheel (STRING key)
{
	return key_to_cacheel(famcache, key, "FAM", FALSE);
}
/*=====================================================
 * key_to_sour_cacheel -- Convert key to source_cacheel
 *===================================================*/
static CACHEEL
key_to_sour_cacheel (CNSTRING key)
{
	return key_to_cacheel(sourcache, key, "SOUR", FALSE);
}
/*====================================================
 * key_to_even_cacheel -- Convert key to event_cacheel
 *==================================================*/
static CACHEEL
key_to_even_cacheel (CNSTRING key)
{
	return key_to_cacheel(evencache, key, "EVEN", FALSE);
}
/*====================================================
 * key_to_othr_cacheel -- Convert key to other_cacheel
 *==================================================*/
static CACHEEL
key_to_othr_cacheel (CNSTRING key)
{
	return key_to_cacheel(othrcache, key, NULL, FALSE);
}
/*======================================
 * init_caches -- Create and init caches
 *====================================*/
void
init_caches (void)
{
	indicache = create_cache("INDI", csz_indi, icsz_indi);
	famcache  = create_cache("FAM", csz_fam, icsz_fam);
	evencache = create_cache("EVEN", (INT)csz_even, (INT)csz_even);
	sourcache = create_cache("SOUR", (INT)csz_sour, (INT)icsz_sour);
	othrcache = create_cache("OTHR", (INT)csz_othr, (INT)icsz_othr);
}
/*=============================
 * create_cache -- Create cache
 *===========================*/
static CACHE
create_cache (STRING name, INT dirsize, INT indsize)
{
	CACHE cache;
	if (dirsize < 1) dirsize = 1;
	if (indsize < 1) indsize = 1;
	cache = (CACHE) stdalloc(sizeof(*cache));
	llstrncpy(cname(cache), name, sizeof(cname(cache)), uu8);
	cdata(cache) = create_table();
	cfirstdir(cache) = clastdir(cache) = NULL;
	cfirstind(cache) = clastind(cache) = NULL;
	csizedir(cache) = csizeind(cache) = 0;
	cmaxdir(cache) = dirsize;
	cmaxind(cache) = indsize;
	return cache;
}
/*=================================================
 * remove_direct -- Unlink CACHEEL from direct list
 *===============================================*/
static void
remove_direct (CACHE cache, CACHEEL cel)
{
	CACHEEL prev = cprev(cel);
	CACHEEL next = cnext(cel);
	ASSERT(cache && cel);
	if (prev) cnext(prev) = next;
	if (next) cprev(next) = prev;
	if (!prev) cfirstdir(cache) = next;
	if (!next) clastdir(cache) = prev;
	csizedir(cache)--;
}
/*=====================================================
 * remove_indirect -- Unlink CACHEEL from indirect list
 *===================================================*/
static void
remove_indirect (CACHE cache, CACHEEL cel)
{
	CACHEEL prev = cprev(cel);
	CACHEEL next = cnext(cel);
	ASSERT(cache && cel);
	if (prev) cnext(prev) = next;
	if (next) cprev(next) = prev;
	if (!prev) cfirstind(cache) = next;
	if (!next) clastind(cache) = prev;
	csizeind(cache)--;
}
/*===========================================================
 * first_direct -- Make unlinked CACHEEL first in direct list
 *=========================================================*/
static void
first_direct (CACHE cache, CACHEEL cel)
{
	CACHEEL frst = cfirstdir(cache);
	ASSERT(cache && cel);
	csizedir(cache)++;
	cprev(cel) = NULL;
	cnext(cel) = frst;
	if (frst) cprev(frst) = cel;
	if (!frst) clastdir(cache) = cel;
	cfirstdir(cache) = cel;
}
/*===============================================================
 * first_indirect -- Make unlinked CACHEEL first in indirect list
 *  Does not check for overflow (caller's responsibility)
 *=============================================================*/
static void
first_indirect (CACHE cache, CACHEEL cel)
{
	CACHEEL frst = cfirstind(cache);
	ASSERT(cache && cel);
	csizeind(cache)++;
	cprev(cel) = NULL;
	cnext(cel) = frst;
	if (frst) cprev(frst) = cel;
	if (!frst) clastind(cache) = cel;
	cfirstind(cache) = cel;
}
/*=======================================================
 * remove_last -- Remove last indirect element from cache
 *=====================================================*/
static void
remove_last (CACHE cache)
{
	CACHEEL cel=0;
	STRING key;
	for (cel = clastind(cache); cel && csemilock(cel); cel = cprev(cel))
		;
	if (!cel) {
		llwprintf("Indirect cache overflow! (cache=%s, size=%d)\n", cname(cache), cmaxind(cache));
		ASSERT(cel);
	}
	ASSERT(!cclock(cel)); /* locked elements should never leave direct */
	remove_indirect(cache, cel);
	key = ckey(cel);
	stdfree(cel);
	delete_table(cdata(cache), key);
	stdfree(key);
}
/*============================================================
 * direct_to_first -- Make direct CACHEEL first in direct list
 *==========================================================*/
static void
direct_to_first (CACHE cache,
                 CACHEEL cel)
{
	ASSERT(cache && cel);
	if (cel == cfirstdir(cache)) return;
	remove_direct(cache, cel);
	first_direct(cache, cel);
}
/*==================================================================
 * indirect_to_first -- Make indirect CACHEEL first in direct list
 *================================================================*/
static void
indirect_to_first (CACHE cache,
                   CACHEEL cel)
{
	ASSERT(cache && cel);
	semilock_cache(cel);
	prepare_direct_space(cache);
	remove_indirect(cache, cel);
	dereference(cel);
	first_direct(cache, cel);
	unsemilock_cache(cel);
}
/*==============================================================
 * direct_to_indirect -- Make last direct CACHEEL first indirect
 *============================================================*/
static void
direct_to_indirect (CACHE cache)
{
	CACHEEL cel = clastdir(cache);
	for (cel = clastdir(cache); cel && cclock(cel); cel = cprev(cel))
		;
	if (!cel) {
		llwprintf("Cache overflow! (Cache=%s, size=%d)\n", cname(cache), cmaxdir(cache));
		ASSERT(cel);
	}
	remove_direct(cache, cel);
	free_rec(crecord(cel)); /* this frees the nodes */
	crecord(cel) = NULL;
	cnode(cel) = NULL;
	first_indirect(cache, cel);
}
/*=====================================================
 * dereference -- Dereference cel by reading its record
 *===================================================*/
static void
dereference (CACHEEL cel)
{
	STRING rawrec;
	INT len;
	RECORD rec;
	ASSERT(cel);
	ASSERT(rawrec = retrieve_raw_record(ckey(cel), &len));
	ASSERT(rec = string_to_record(rawrec, ckey(cel), len));
	crecord(cel) = rec;
	cnode(cel) = nztop(rec);
	stdfree(rawrec);
}
/*========================================================
 * add_to_direct -- Add new CACHEEL to direct part of cache
 * reportmode: if True, then return NULL rather than aborting
 *   if there is no record. Also return NULL for deleted
 *   records (of length less than 6???)
 *  cache:      [IN]  which cache to which to add
 *  key:        [IN]  key of record to be added
 *  reportmode: [IN] if non-zero, failures should be silent
 *======================================================*/
static CACHEEL
add_to_direct (CACHE cache, CNSTRING key, INT reportmode)
{
	STRING rawrec;
	INT len;
	CACHEEL cel;
	RECORD rec;
	int i, j;
	STRING keycopy;

#ifdef DEBUG
	llwprintf("add_to_direct: key == %s\n", key);
#endif
	ASSERT(cache && key);
	prepare_direct_space(cache);
	rec = NULL;
	if ((rawrec = retrieve_raw_record(key, &len))) 
		rec = string_to_record(rawrec, key, len);
	if (!rec)
	{
		if(listbadkeys) {
			if(strlen(badkeylist) < 80 - strlen(key) - 2) {
				if (badkeylist[0])
					strcat(badkeylist, ",");
				strcat(badkeylist, key);
			}
			return(NULL);
		}
		if (reportmode) return(NULL);
		crashlog("key %s is not in database. Use \"btedit <database> <key>\" to fix.\n", (char *) key);
		crashlog("where <key> is probably one of the following:\n");
		for(i = 0; i < 10; i++)
		{
			j = keyidx + i;
			if(j >= 10) j -= 10;
			if (keybuf[j][0])
				crashlog(" %s", (char *)keybuf[j]);
		}
		crashlog("\n");
		/* deliberately fall through to let ASSERT(rec) fail */
	}
	ASSERT(rec);
	ASSERT(csizedir(cache) < cmaxdir(cache));
	cel = (CACHEEL) stdalloc(sizeof(*cel));
	keycopy = strsave(key);
	insert_table_ptr(cdata(cache), keycopy, cel);
	crecord(cel) = rec;
	cnode(cel) = nztop(rec);
	ckey(cel) = keycopy;
	cclock(cel) = 0;
	csemilock(cel) = 0;
	first_direct(cache, cel);
	stdfree(rawrec);
	return cel;
}
/*======================================================
 * key_to_cacheel -- Return CACHEEL corresponding to key
 *====================================================*/
static CACHEEL
key_to_cacheel (CACHE cache, CNSTRING key, STRING tag, INT reportmode)
{
	CACHEEL cel;

	strncpy(keybuf[keyidx], (key ? (char *)key : "NULL"), 31);
	keybuf[keyidx][31] = '\0';
	keyidx++;
	if(keyidx >= 10) keyidx = 0;
	if ((cel = (CACHEEL) valueof_ptr(cdata(cache), key))) {
		if (cnode(cel))
			direct_to_first(cache, cel);
		else
			indirect_to_first(cache, cel);
		if (tag) {
#ifdef DEBUG
			llwprintf("BEFORE ASSERT: tag, ntag(cnode(cel)) = %s, %s\n", tag, ntag(cnode(cel)));
#endif
			ASSERT(eqstr(tag, ntag(cnode(cel))));
		}
		return cel;
	}
	cel = add_to_direct(cache, key, reportmode);
	if (cel && tag) {
		ASSERT(eqstr(tag, ntag(cnode(cel))));
	}
	return cel;
}
/*===============================================================
 * prepare_direct_space -- Make space in direct
 *  Moves a direct entry to indirect if necessary
 *=============================================================*/
static void
prepare_direct_space (CACHE cache)
{
	if (csizedir(cache) >= cmaxdir(cache)) {
		if (csizeind(cache) >= cmaxind(cache))
			remove_last(cache);
		direct_to_indirect(cache);
	}
}
/*===============================================================
 * key_to_node -- Return tree from key; add to cache if not there
 * asserts if failure
 * TO DO - should become obsoleted by key_typed_to_record
 *=============================================================*/
static NODE
key_typed_to_node (CACHE cache, CNSTRING key, STRING tag)
{
	CACHEEL cel;
	ASSERT(cache && key);
	if (!(cel = key_to_cacheel(cache, key, tag, FALSE)))
		return NULL;
	return cnode(cel);
}
/*===============================================================
 * key_typed_to_record -- Return tree from key; add to cache if not there
 * asserts if failure
 *=============================================================*/
static RECORD
key_typed_to_record (CACHE cache, CNSTRING key, STRING tag)
{
	CACHEEL cel;
	ASSERT(cache && key);
	if (!(cel = key_to_cacheel(cache, key, tag, FALSE)))
		return NULL;
	return crecord(cel);
}
/*===============================================================
 * qkey_to_node -- Return tree from key; add to cache if not there
 * report mode - returns NULL if failure
 * TO DO - should become obsoleted by qkey_typed_to_record
 *=============================================================*/
static NODE
qkey_to_node (CACHE cache, CNSTRING key, STRING tag)
{
	CACHEEL cel;
	ASSERT(cache && key);
	if (!(cel = key_to_cacheel(cache, key, tag, TRUE)))
		return NULL;
	return cnode(cel);
}
/*===============================================================
 * qkey_typed_to_record -- Return tree from key; add to cache if not there
 * report mode - returns NULL if failure
 *=============================================================*/
static RECORD
qkey_typed_to_record (CACHE cache, CNSTRING key, STRING tag)
{
	CACHEEL cel;
	ASSERT(cache && key);
	if (!(cel = key_to_cacheel(cache, key, tag, TRUE)))
		return NULL;
	return crecord(cel);
}
/*======================================
 * load_cacheel -- Load CACHEEL into direct cache
 *  if needed & valid
 * (ie, this handles null input, or input already in cache)
 *====================================*/
void
load_cacheel (CACHEEL cel)
{
	CACHE cache = NULL;
	if (!cel || cnode(cel)) return;
	switch (ckey(cel)[0]) {
	case 'I': cache = indicache; break;
	case 'F': cache = famcache; break;
	case 'S': cache = sourcache; break;
	case 'E': cache = evencache; break;
	case 'X': cache = othrcache; break;
	default: ASSERT(0); break;
	}
	indirect_to_first(cache, cel);
}
/*======================================
 * lock_cache -- Lock CACHEEL into direct cache
 *====================================*/
void
lock_cache (CACHEEL cel)
{
	ASSERT(cnode(cel));
	cclock(cel)++;
}
/*==========================================
 * unlock_cache -- Unlock CACHEEL from direct cache
 *========================================*/
void
unlock_cache (CACHEEL cel)
{
	ASSERT(cnode(cel));
	cclock(cel)--;
}
/*======================================
 * semilock_cache -- Lock CACHEEL into cache (indirect is ok)
 *====================================*/
void
semilock_cache (CACHEEL cel)
{
	csemilock(cel)++;
}
/*==========================================
 * unsemilock_cache -- Unlock CACHEEL from cache (indirect is ok)
 *========================================*/
void
unsemilock_cache (CACHEEL cel)
{
	csemilock(cel)--;
}
/*=========================================
 * get_cache_stats -- Calculate cache stats
 *  returns static buffer
 *=======================================*/
STRING
get_cache_stats (void)
{
	static char buffer[64];
	CACHE c = indicache;
	CACHE f = famcache;
	INT nlocks = 0, nsemilocks = 0;
	CACHEEL cel;
	for (cel = cfirstdir(c); cel; cel = cnext(cel)) {
		if (cclock(cel)) nlocks++;
		if (csemilock(cel)) nsemilocks++;
	}
	sprintf(buffer, "Cache contents -- I: %dD %d %dL; %dS; F: %dD  %dI",
	    csizedir(c), csizeind(c), nlocks, nsemilocks
			, csizedir(f), csizeind(f));
	return buffer;
}
/*============================================
 * indi_to_cache -- Add person to person cache
 *==========================================*/
void
indi_to_cache (RECORD rec)
{
	record_to_cache(indicache, rec);
}
/*============================================
 * indi_to_cache_old -- Add person to person cache
 *  should be obsoleted by indi_to_cache
 *==========================================*/
void
indi_to_cache_old (NODE node)
{
	node_to_cache(indicache, node);
}
/*===========================================
 * fam_to_cache -- Add family to family cache
 *=========================================*/
void
fam_to_cache (NODE node)
{
	node_to_cache(famcache, node);
}
/*==========================================
 * even_to_cache -- Add event to event cache
 *========================================*/
void
even_to_cache (NODE node)
{
	node_to_cache(evencache, node);
}
/*============================================
 * sour_to_cache -- Add source to source cache
 *==========================================*/
void
sour_to_cache (NODE node)
{
	node_to_cache(sourcache, node);
}
/*===========================================
 * othr_to_cache -- Add other record to cache
 *=========================================*/
void
othr_to_cache (NODE node)
{
	node_to_cache(othrcache, node);
}
/*========================================
 * node_to_cache -- Add node tree to cache
 *======================================*/
void
node_to_cache (CACHE cache, NODE node)
{
	RECORD rec = create_record(node);
	record_to_cache(cache, rec);
}
/*========================================
 * record_to_cache -- Add record to cache
 *  record expected to be valid
 *  INDI records may only be added to INDI cache, etc
 *======================================*/
static void
record_to_cache (CACHE cache, RECORD rec)
{
	STRING key;
	NODE top;
	ASSERT(cache && rec);
	top = nztop(rec);
	ASSERT(top);
	if (nestr(cname(cache), "OTHR")) {
		/* only INDI records in INDI cache, etc */
		ASSERT(eqstr(cname(cache), ntag(top)));
	}
	key = node_to_key(top);
	ASSERT(key);
	ASSERT(!valueof_ptr(cdata(cache), key));
	prepare_direct_space(cache);
	add_record_to_direct(cache, rec, key);
}
/*=======================================================
 * add_record_to_direct -- Add node to direct part of cache
 *=====================================================*/
static void
add_record_to_direct (CACHE cache, RECORD rec, STRING key)
{
	CACHEEL cel;
	STRING keynew;
	NODE node = nztop(rec);
	ASSERT(cache && node);
	ASSERT(csizedir(cache) < cmaxdir(cache));
	cel = (CACHEEL) stdalloc(sizeof(*cel));
	insert_table_ptr(cdata(cache), keynew=strsave(key), cel);
	crecord(cel) = rec;
	cnode(cel) = node;
	ckey(cel) = keynew;
	cclock(cel) = FALSE;
	first_direct(cache, cel);
}
/*==============================================
 * remove_indi_cache -- Remove person from cache
 *============================================*/
void
remove_indi_cache (STRING key)
{
	remove_from_cache(indicache, key);
}
/*=============================================
 * remove_fam_cache -- Remove family from cache
 *===========================================*/
void
remove_fam_cache (STRING key)
{
	remove_from_cache(famcache, key);
}
/*=============================================
 * remove_from_cache -- Remove entry from cache
 *===========================================*/
void
remove_from_cache (CACHE cache, STRING key)
{
	CACHEEL cel;
	if (!key || *key == 0 || !cache)
		return;
	if (!(cel = (CACHEEL) valueof_ptr(cdata(cache), key)))
		return;
	ASSERT(!cclock(cel) && !csemilock(cel));
	if (cnode(cel))
		remove_direct(cache, cel);
	else
		remove_indirect(cache, cel);
	stdfree(cel);
	delete_table(cdata(cache), key);
}
/*================================================================
 * value_to_xref -- Converts a string to a record key, if possible
 *==============================================================*/
STRING
value_to_xref (STRING val)
{
	INT c;

	if (!val || (*val != '@') || (strlen(val) < 4) ||
		(val[strlen(val)-1] != '@')) return NULL;
	val = rmvat(val);
	if ((c = *val) != 'I' && c != 'F' && c != 'S' && c != 'E' &&
		c != 'X') return NULL;
	if (!isnumeric(val + 1)) return NULL;
	return val;
}
/*===================================================
 * indi_to_cacheel -- Convert person to cache element
 *=================================================*/
CACHEEL
indi_to_cacheel (RECORD indi)
{
	CACHEEL cel;
	if (!indi || !nztop(indi)) return NULL;
	cel = key_to_indi_cacheel(rmvat(nxref(nztop(indi))));
	ASSERT(cel);
	return cel;
}
/*===================================================
 * indi_to_cacheel_old -- Convert person to cache element
 *  should be obsoleted by indi_to_cacheel
 *=================================================*/
CACHEEL
indi_to_cacheel_old (NODE indi)
{
	CACHEEL cel;
	if (!indi) return NULL;
#ifdef DEBUG
	llwprintf("indi_to_cacheel_old: %s\n", nxref(indi));
#endif
	cel = key_to_indi_cacheel(rmvat(nxref(indi)));
	ASSERT(cel);
	return cel;
}
/*==================================================
 * fam_to_cacheel -- Convert family to cache element
 *================================================*/
CACHEEL
fam_to_cacheel (NODE fam)
{
	CACHEEL cel;
	if (!fam) return NULL;
	cel = key_to_fam_cacheel(rmvat(nxref(fam)));
	ASSERT(cel);
	return cel;
}
/*===================================================
 * sour_to_cacheel -- Convert source to cache element
 *=================================================*/
CACHEEL
sour_to_cacheel (NODE node)
{
	CACHEEL cel;
	if (!node) return NULL;
	cel = key_to_sour_cacheel(rmvat(nxref(node)));
	ASSERT(cel);
	return cel;
}
/*==================================================
 * even_to_cacheel -- Convert event to cache element
 *================================================*/
CACHEEL
even_to_cacheel (NODE even)
{
	CACHEEL cel;
	if (!even) return NULL;
	cel = key_to_even_cacheel(rmvat(nxref(even)));
	ASSERT(cel);
	return cel;
}
/*==================================================
 * othr_to_cacheel -- Convert other to cache element
 *================================================*/
CACHEEL
othr_to_cacheel (NODE othr)
{
	CACHEEL cel;
	if (!othr) return NULL;
	cel = key_to_othr_cacheel(rmvat(nxref(othr)));
	ASSERT(cel);
	return cel;
}
/*==================================================
 * node_to_cacheel -- Convert any node to cache element
 *================================================*/
CACHEEL
node_to_cacheel (NODE node)
{
	STRING key = rmvat(nxref(node));
	switch(key[0]) {
	case 'I': return indi_to_cacheel_old(node);
	case 'F': return fam_to_cacheel(node);
	case 'S': return sour_to_cacheel(node);
	case 'E': return even_to_cacheel(node);
	case 'X': return othr_to_cacheel(node);
	}
	ASSERT(0); return NULL;
}
/*==============================================
 * key_of_record -- Return display key of record
 *  returns static buffer
 *============================================*/
STRING
key_of_record (NODE node)
{
	NODE refn;
	ASSERT(node);
	refn = REFN(node);
	if (refn && nval(refn)) {
		return nval(refn);
	}
	return rmvat(nxref(node)) + 1;
}
/*==============================================
 * qkey_to_???_cacheel -- Convert key to cacheel
 *  (report mode - returns NULL if failure)
 *  5 symmetric versions
 *============================================*/
CACHEEL qkey_to_indi_cacheel (STRING key)
{
	return key_to_cacheel(indicache, key, "INDI", TRUE);
}
CACHEEL qkey_to_fam_cacheel (STRING key)
{
	return key_to_cacheel(famcache, key, "FAM", TRUE);
}
CACHEEL qkey_to_even_cacheel (STRING key)
{
	return key_to_cacheel(evencache, key, "EVEN", TRUE);
}
CACHEEL qkey_to_sour_cacheel (STRING key)
{
	return key_to_cacheel(sourcache, key, "SOUR", TRUE);
}
CACHEEL qkey_to_othr_cacheel (STRING key)
{
	return key_to_cacheel(othrcache, key, NULL, TRUE);
}
/*==============================================
 * nztop -- Return first NODE of a RECORD
 *  handle NULL input
 *============================================*/
NODE
nztop (RECORD rec)
{
	return rec ? rec->top : 0;
}
