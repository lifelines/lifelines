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
#include "screen.h"

/*********************************************
 * global variables (no header)
 *********************************************/

char badkeylist[100] = "";
int listbadkeys = 0;

/*********************************************
 * local function prototypes
 *********************************************/

static void add_node_to_direct(CACHE cache, NODE node, STRING key);
static void add_nod0_to_direct(CACHE cache, NOD0 nod0, STRING key);
static CACHE create_cache(STRING name, INT dirsize, INT indsize);
static void dereference(CACHEEL);
static CACHEEL key_to_cacheel(CACHE, STRING, STRING, INT);
static CACHEEL key_to_even_cacheel (STRING key);
static NODE key_to_node(CACHE cache, STRING key, STRING tag);
static NOD0 key_to_nod0(CACHE cache, STRING key, STRING tag);
static CACHEEL key_to_othr_cacheel (STRING key);
static CACHEEL key_to_sour_cacheel (STRING key);
static void prepare_direct_space(CACHE cache);
static NODE qkey_to_node(CACHE cache, STRING key, STRING tag);
static NOD0 qkey_to_nod0(CACHE cache, STRING key, STRING tag);


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
/*======================================================
 * qkeynum_to_fam -- Convert a numeric key to a fam node
 *  report mode - it returns NULL if failed
 *  (ie, no fam with that number)
 *====================================================*/
NODE
qkeynum_to_fam (int keynum)
{
	char keystr[20];
	sprintf(keystr,"F%d",keynum);
	return qkey_to_fam(keystr);
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
/*================================================
 * keynum_to_even -- Convert a numeric key to a even node
 *  assert if failed (ie, no sour with that number)
 * Created: 2001/01/27, Perry Rapp
 *==============================================*/
NODE
keynum_to_even (int keynum)
{
	char keystr[MAXKEYWIDTH+1];
	sprintf(keystr,"E%d",keynum);
	return key_to_even(keystr);
}
/*================================================
 * keynum_to_othe -- Convert a numeric key to an other node
 *  assert if failed (ie, no sour with that number)
 * Created: 2001/01/27, Perry Rapp
 *==============================================*/
NODE
keynum_to_othe (int keynum)
{
	char keystr[20];
	sprintf(keystr,"X%d",keynum);
	return key_to_othr(keystr);
}
/*=====================================
 * keynum_to_node -- Convert keynum to node
 * Created: 2001/01/27, Perry Rapp
 *===================================*/
NODE
keynum_to_node (char ntype, int keynum)
{
	switch(ntype) {
	case 'I': return keynum_to_indi(keynum);
	case 'F': return keynum_to_fam(keynum);
	case 'S': return keynum_to_sour(keynum);
	case 'E': return keynum_to_even(keynum);
	case 'X': return keynum_to_othe(keynum);
	}
	ASSERT(0);
	return 0;
}
/*=====================================
 * key_to_type -- Convert key to node
 * TO DO - should become obsoleted by key_to_typ0
 *===================================*/
NODE
key_to_type (STRING key, INT reportmode)
{
	switch(key[0])
	{
	case 'I': return reportmode ? qkey_to_indi(key) : key_to_indi(key);
	case 'F': return reportmode ? qkey_to_fam(key) : key_to_fam(key);
	case 'E': return reportmode ? qkey_to_even(key) : key_to_even(key);
	case 'S': return reportmode ? qkey_to_sour(key) : key_to_sour(key);
	}
	return reportmode ? qkey_to_othr(key) : key_to_othr(key);
}
/*=====================================
 * qkey_to_type -- Convert key to node
 * Created: 2001/02/11, Perry Rapp (for naming consistency)
 *===================================*/
NODE
qkey_to_type (STRING key)
{
	return key_to_type(key, TRUE);
}
/*=====================================
 * key_to_typ0 -- Convert key to nod0
 *===================================*/
NOD0
key_to_typ0 (STRING key, INT reportmode)
{
	switch(key[0])
	{
	case 'I': return reportmode ? qkey_to_indi0(key) : key_to_indi0(key);
	case 'F': return reportmode ? qkey_to_fam0(key) : key_to_fam0(key);
	case 'E': return reportmode ? qkey_to_even0(key) : key_to_even0(key);
	case 'S': return reportmode ? qkey_to_sour0(key) : key_to_sour0(key);
	}
	return reportmode ? qkey_to_othr0(key) : key_to_othr0(key);
}
/*=====================================
 * key_to_??? -- Convert key to person
 *  (asserts if failure)
 *  5 symmetric versions
 * TO DO - should become obsoleted by key_to_???0
 *===================================*/
NODE
key_to_indi (STRING key)
{
	return key_to_node(indicache, key, "INDI");
}
NODE key_to_fam (STRING key)
{
	return key_to_node(famcache, key, "FAM");
}
NODE key_to_even (STRING key)
{
	return key_to_node(evencache, key, "EVEN");
}
NODE key_to_sour (STRING key)
{
	return key_to_node(sourcache, key, "SOUR");
}
NODE key_to_othr (STRING key)
{
	return key_to_node(othrcache, key, NULL);
}
/*=====================================
 * key_to_???0 -- Convert key to person
 *  (asserts if failure)
 *  5 symmetric versions
 *===================================*/
NOD0
key_to_indi0 (STRING key)
{
	return key_to_nod0(indicache, key, "INDI");
}
NOD0 key_to_fam0 (STRING key)
{
	return key_to_nod0(famcache, key, "FAM");
}
NOD0 key_to_even0 (STRING key)
{
	return key_to_nod0(evencache, key, "EVEN");
}
NOD0 key_to_sour0 (STRING key)
{
	return key_to_nod0(sourcache, key, "SOUR");
}
NOD0 key_to_othr0 (STRING key)
{
	return key_to_nod0(othrcache, key, NULL);
}
/*========================================
 * qkey_to_??? -- Convert key to node type
 *  report mode (returns NULL if failure)
 *  5 symmetric versions
 * TO DO - should become obsoleted by key_to_???0
 *======================================*/
NODE qkey_to_indi (STRING key)
{
	return qkey_to_node(indicache, key, "INDI");
}
NODE qkey_to_fam (STRING key)
{
	return qkey_to_node(famcache, key, "FAM");
}
NODE qkey_to_even (STRING key)
{
	return qkey_to_node(evencache, key, "EVEN");
}
NODE qkey_to_sour (STRING key)
{
	return qkey_to_node(sourcache, key, "SOUR");
}
NODE qkey_to_othr (STRING key)
{
	return qkey_to_node(othrcache, key, NULL);
}
/*========================================
 * qkey_to_???0 -- Convert key to node type
 *  report mode (returns NULL if failure)
 *  5 symmetric versions
 *======================================*/
NOD0 qkey_to_indi0 (STRING key)
{
	return qkey_to_nod0(indicache, key, "INDI");
}
NOD0 qkey_to_fam0 (STRING key)
{
	return qkey_to_nod0(famcache, key, "FAM");
}
NOD0 qkey_to_even0 (STRING key)
{
	return qkey_to_nod0(evencache, key, "EVEN");
}
NOD0 qkey_to_sour0 (STRING key)
{
	return qkey_to_nod0(sourcache, key, "SOUR");
}
NOD0 qkey_to_othr0 (STRING key)
{
	return qkey_to_nod0(othrcache, key, NULL);
}
/*=====================================================
 * key_to_unknown_cacheel -- Convert any key to cacheel
 * Created: 2001/04/13, Perry Rapp
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
key_to_sour_cacheel (STRING key)
{
	return key_to_cacheel(sourcache, key, "SOUR", FALSE);
}
/*====================================================
 * key_to_even_cacheel -- Convert key to event_cacheel
 *==================================================*/
static CACHEEL
key_to_even_cacheel (STRING key)
{
	return key_to_cacheel(evencache, key, "EVEN", FALSE);
}
/*====================================================
 * key_to_othr_cacheel -- Convert key to other_cacheel
 *==================================================*/
static CACHEEL
key_to_othr_cacheel (STRING key)
{
	return key_to_cacheel(othrcache, key, NULL, FALSE);
}
/*======================================
 * init_caches -- Create and init caches
 *====================================*/
void
init_caches (void)
{
	indicache = create_cache("indi", csz_indi, icsz_indi);
	famcache  = create_cache("fam", csz_fam, icsz_fam);
	evencache = create_cache("even", (INT)csz_even, (INT)csz_even);
	sourcache = create_cache("sour", (INT)csz_sour, (INT)icsz_sour);
	othrcache = create_cache("othr", (INT)csz_othr, (INT)icsz_othr);
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
	llstrncpy(cname(cache), name, sizeof(cname(cache)));
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
	free_nod0(cnod0(cel)); /* this frees the nodes */
	cnod0(cel) = NULL;
	cnode(cel) = NULL;
	first_indirect(cache, cel);
}
/*=====================================================
 * dereference -- Dereference cel by reading its record
 *===================================================*/
static void
dereference (CACHEEL cel)
{
	STRING rec;
	INT len;
	NOD0 nod0;
	ASSERT(cel);
	ASSERT(rec = retrieve_record(ckey(cel), &len));
	ASSERT(nod0 = string_to_nod0(rec, ckey(cel), len));
	cnod0(cel) = nod0;
	cnode(cel) = nod0->top;
	stdfree(rec);
}
/*========================================================
 * add_to_direct -- Add new CACHEEL to direct part of cache
 * reportmode: if True, then return NULL rather than aborting
 *   if there is no record. Also return NULL for deleted
 *   records (of length less than 6???)
 *======================================================*/
static CACHEEL
add_to_direct (CACHE cache,
               STRING key,
               INT reportmode)
{
	STRING record;
	INT len;
	CACHEEL cel;
	NOD0 nod0;
	int i, j;

#ifdef DEBUG
	llwprintf("add_to_direct: key == %s\n", key);
#endif
	ASSERT(cache && key);
	prepare_direct_space(cache);
	nod0 = NULL;
	if ((record = retrieve_record(key, &len))) 
		nod0 = string_to_nod0(record, key, len);
	if (!nod0)
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
		llwprintf("key %s is not in database. Use \"btedit <database> <key>\" to fix.\n", (char *) key);
		llwprintf("where <key> is probably one of the following:\n");
		for(i = 0; i < 10; i++)
		{
			j = keyidx + i;
			if(j >= 10) j -= 10;
			llwprintf(" %s", (char *)keybuf[j]);
		}
		llwprintf("\n");
		/* deliberately fall through to let ASSERT(nod0) fail */
	}
	ASSERT(nod0);
	ASSERT(csizedir(cache) < cmaxdir(cache));
	cel = (CACHEEL) stdalloc(sizeof(*cel));
	insert_table_ptr(cdata(cache), key = strsave(key), cel);
	cnod0(cel) = nod0;
	cnode(cel) = nod0->top;
	ckey(cel) = key;
	cclock(cel) = 0;
	csemilock(cel) = 0;
	first_direct(cache, cel);
	stdfree(record);
	return cel;
}
/*======================================================
 * key_to_cacheel -- Return CACHEEL corresponding to key
 *====================================================*/
static CACHEEL
key_to_cacheel (CACHE cache,
                STRING key,
                STRING tag,
                INT reportmode)
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
 * Created: 2001/03/19, Perry Rapp
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
 * TO DO - should become obsoleted by key_to_nod0
 *=============================================================*/
static NODE
key_to_node (CACHE cache, STRING key, STRING tag)
{
	CACHEEL cel;
	ASSERT(cache && key);
	if (!(cel = key_to_cacheel(cache, key, tag, FALSE)))
		return NULL;
	return cnode(cel);
}
/*===============================================================
 * key_to_nod0 -- Return tree from key; add to cache if not there
 * asserts if failure
 *=============================================================*/
static NOD0
key_to_nod0 (CACHE cache, STRING key, STRING tag)
{
	CACHEEL cel;
	ASSERT(cache && key);
	if (!(cel = key_to_cacheel(cache, key, tag, FALSE)))
		return NULL;
	return cnod0(cel);
}
/*===============================================================
 * qkey_to_node -- Return tree from key; add to cache if not there
 * report mode - returns NULL if failure
 * TO DO - should become obsoleted by qkey_to_nod0
 *=============================================================*/
static NODE
qkey_to_node (CACHE cache, STRING key, STRING tag)
{
	CACHEEL cel;
	ASSERT(cache && key);
	if (!(cel = key_to_cacheel(cache, key, tag, TRUE)))
		return NULL;
	return cnode(cel);
}
/*===============================================================
 * qkey_to_nod0 -- Return tree from key; add to cache if not there
 * report mode - returns NULL if failure
 *=============================================================*/
static NOD0
qkey_to_nod0 (CACHE cache, STRING key, STRING tag)
{
	CACHEEL cel;
	ASSERT(cache && key);
	if (!(cel = key_to_cacheel(cache, key, tag, TRUE)))
		return NULL;
	return cnod0(cel);
}
/*======================================
 * load_cacheel -- Load CACHEEL into direct cache
 *  if needed & valid
 * Created: 2001/03/24, Perry Rapp
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
 * semilock_cache -- Lock CACHEEL into cache (either part)
 * Created: 2001/03/17, Perry Rapp
 *====================================*/
void
semilock_cache (CACHEEL cel)
{
	csemilock(cel)++;
}
/*==========================================
 * unsemilock_cache -- Unlock CACHEEL from cache (either part)
 * Created: 2001/03/17, Perry Rapp
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
 * indi0_to_cache -- Add person to person cache
 *==========================================*/
void
indi0_to_cache (NOD0 nod0)
{
	nod0_to_cache(indicache, nod0);
}
/*============================================
 * indi_to_cache -- Add person to person cache
 *==========================================*/
void
indi_to_cache (NODE node)
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
node_to_cache (CACHE cache,
               NODE node)
{
	NOD0 nod0 = create_nod0(node);
	nod0_to_cache(cache, nod0);
}
/*========================================
 * nod0_to_cache -- Add node tree to cache
 *======================================*/
void
nod0_to_cache (CACHE cache,
               NOD0 nod0)
{
	STRING key;
	ASSERT(cache && nod0 && nztop(nod0));
	key = node_to_key(nztop(nod0));
	ASSERT(!valueof_ptr(cdata(cache), key));
	prepare_direct_space(cache);
	add_nod0_to_direct(cache, nod0, key);
}
/*=======================================================
 * add_node_to_direct -- Add node to direct part of cache
 *=====================================================*/
static void
add_node_to_direct (CACHE cache, NODE node, STRING key)
{
	NOD0 nod0 = create_nod0(node);
	add_nod0_to_direct(cache, nod0, key);
}
/*=======================================================
 * add_nod0_to_direct -- Add node to direct part of cache
 *=====================================================*/
static void
add_nod0_to_direct (CACHE cache, NOD0 nod0, STRING key)
{
	CACHEEL cel;
	STRING keynew;
	NODE node = nztop(nod0);
	ASSERT(cache && node);
	ASSERT(csizedir(cache) < cmaxdir(cache));
	cel = (CACHEEL) stdalloc(sizeof(*cel));
	insert_table_ptr(cdata(cache), keynew=strsave(key), cel);
	cnod0(cel) = nod0;
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
indi_to_cacheel (NODE indi)
{
	CACHEEL cel;
	if (!indi) return NULL;
#ifdef DEBUG
	llwprintf("indi_to_cacheel: %s\n", nxref(indi));
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
	case 'I': return indi_to_cacheel(node);
	case 'F': return fam_to_cacheel(node);
	case 'S': return sour_to_cacheel(node);
	case 'E': return even_to_cacheel(node);
	case 'X': return othr_to_cacheel(node);
	}
	ASSERT(0); return NULL;
}
/*==============================================
 * key_of_record -- Return display key of record
 *============================================*/
STRING
key_of_record (NODE node)
{
	NODE refn;
	ASSERT(node);
	refn = REFN(node);
	if (refn && nval(refn)) return nval(refn);
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
 * nztop -- Return first NODE of a NOD0
 *  handle NULL input
 *============================================*/
NODE
nztop (NOD0 nod0)
{
	return nod0 ? nod0->top : 0;
}
