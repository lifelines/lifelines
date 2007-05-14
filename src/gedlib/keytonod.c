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
 * keytonod.c -- Cache for lifelines custom btree database
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 08 May 94    3.0.2 - 23 Dec 94
 *   3.0.3 - 16 Jul 95
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "cache.h"
#include "liflines.h"
#include "feedback.h"
#include "zstr.h"

/*********************************************
 * global variables (no header)
 *********************************************/

char badkeylist[100] = "";
int listbadkeys = 0;



/*===============================
 * CACHEEL -- Cache element type.
 *=============================*/
/* typedef struct tag_cacheel *CACHEEL; */
struct tag_cacheel {
	CNSTRING c_magic; /* points to cel_magic */
	NODE c_node;      /* root node */
	CACHEEL c_prev;   /* previous el */
	CACHEEL c_next;   /* next el */
	STRING c_key;     /* record key */
	INT c_lock;       /* lock count (includes report locks) */
	INT c_rptlock;    /* report lock count */
	RECORD c_record;
};
#define cnode(e)      ((e)->c_node)
#define cprev(e)      ((e)->c_prev)
#define cnext(e)      ((e)->c_next)
#define ckey(e)       ((e)->c_key)
#define cclock(e)     ((e)->c_lock)
#define ccrptlock(e)  ((e)->c_rptlock)
#define crecord(e)    ((e)->c_record)

/*==============================
 * CACHE -- Internal cache type.
 *============================*/
typedef struct {
	char c_name[5];
	TABLE c_data;        /* table of keys */
	CACHEEL c_firstdir;  /* first direct */
	CACHEEL c_lastdir;   /* last direct */
	CACHEEL c_array;     /* big array of cacheels, all alloc'd in a block */
	CACHEEL c_free;      /* root of free list */
	INT c_maxdir;        /* max in direct */
	INT c_sizedir;       /* cur in direct */
} *CACHE;
#define cacname(c)     ((c)->c_name)
#define cacdata(c)     ((c)->c_data)
#define cacfirstdir(c) ((c)->c_firstdir)
#define caclastdir(c)  ((c)->c_lastdir)
#define cacarray(e) ((e)->c_array)
#define cacfree(e) ((e)->c_free)
#define cacmaxdir(c)   ((c)->c_maxdir)
#define cacsizedir(c)  ((c)->c_sizedir)


/*********************************************
 * local function prototypes
 *********************************************/

/* static void add_record_to_direct(CACHE cache, RECORD rec, STRING key); */
static void cache_get_lock_counts(CACHE ca, INT * locks);
static CACHE create_cache(STRING name, INT dirsize);
static void delete_cache(CACHE * pcache);
static void ensure_cel_has_record(CACHEEL cel);
static ZSTR get_cache_stats(CACHE ca);
static CACHEEL get_free_cacheel(CACHE cache);
static void init_cel(CACHEEL cel);
static CACHEEL key_to_cacheel(CACHE cache, CNSTRING key, STRING tag, INT reportmode);
static CACHEEL key_to_even_cacheel(CNSTRING key);
static NODE key_typed_to_node(CACHE cache, CNSTRING key, STRING tag);
static RECORD key_to_record_impl(CNSTRING key, INT reportmode);
static RECORD key_typed_to_record(CACHE cache, CNSTRING key, STRING tag);
static CACHEEL key_to_othr_cacheel(CNSTRING key);
static CACHEEL key_to_sour_cacheel(CNSTRING key);
static CACHEEL node_to_cache(CACHE, NODE);
static void put_node_in_cache(CACHE cache, CACHEEL cel, NODE node, STRING key);
static void remove_cel_from_cache(CACHE cache, CACHEEL cel);
static NODE qkey_to_node(CACHE cache, CNSTRING key, STRING tag);
static RECORD qkey_typed_to_record(CACHE cache, CNSTRING key, STRING tag);
/* static CACHEEL qkey_to_typed_cacheel(STRING key); */
static void remove_from_cache(CACHE, CNSTRING);


INT csz_indi = 200;		/* cache size for indi */
INT csz_fam = 200;		/* cache size for fam */
INT csz_sour = 200;		/* cache size for sour */
INT csz_even = 200;		/* cache size for even */
INT csz_othr = 200;		/* cache size for othr */

/*********************************************
 * local variables
 *********************************************/

static CACHE indicache, famcache, evencache, sourcache, othrcache;

static CNSTRING cel_magic = "CEL_MAGIC"; /* fixed pointer to identify cel */

/* keybuf circular list of last 10 keys we looked up in cache 
 * kept for printing debug messages in crash log
 */
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
	return nztop(keynum_to_srecord(keynum));
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
	return nztop(keynum_to_erecord(keynum));
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
	RECORD rec = key_to_record_impl(key, reportmode);
	NODE node = nztop(rec);
	release_record(rec); /* avoiding leaking reference */
	return node;
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
 *  returns addref'd record
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
 *  returns addref'd record
 *===================================*/
RECORD
key_to_record (CNSTRING key)
{
	return key_to_record_impl(key, FALSE);
}
/*=====================================
 * qkey_to_record -- Convert key (any type) to RECORD
 * quiet -- that is, returns NULL if record not in database
 *  returns addref'd record
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
 * returns addref'd record
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
 * All return addref'd records
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
	indicache = create_cache("INDI", csz_indi);
	famcache  = create_cache("FAM", csz_fam);
	evencache = create_cache("EVEN", csz_even);
	sourcache = create_cache("SOUR", csz_sour);
	othrcache = create_cache("OTHR", csz_othr);
}
/*======================================
 * free_caches -- Release cache memory
 * Created: 2003-02-02 (Perry Rapp)
 *====================================*/
void
free_caches (void)
{
	delete_cache(&indicache);
	delete_cache(&famcache);
	delete_cache(&evencache);
	delete_cache(&sourcache);
	delete_cache(&othrcache);
}
/*=============================
 * create_cache -- Create cache
 *===========================*/
static CACHE
create_cache (STRING name, INT dirsize)
{
	CACHE cache;
	INT i;
	if (dirsize < 1) dirsize = 1;
	cache = (CACHE) stdalloc(sizeof(*cache));
	memset(cache, 0, sizeof(*cache));
	llstrncpy(cacname(cache), name, sizeof(cacname(cache)), uu8);
	/* 
	It would be nice to set the table hash size larger for large 
	caches, but right now (2003-10-08), tables do not expose a 
	method to set their hash size.
	*/
	cacdata(cache) = create_table_vptr(); /* pointers to cache elements, owned by cacarray */
	cacfirstdir(cache) = caclastdir(cache) = NULL;
	cacsizedir(cache) = 0;
	cacmaxdir(cache) = dirsize;
	/* Allocate all the cache elements in a big block */
	cacarray(cache) = (CACHEEL) stdalloc(cacmaxdir(cache) * sizeof(cacarray(cache)[0]));
	/* Link all the elements together on the free list */
	for (i=0; i<cacmaxdir(cache); ++i) {
		CACHEEL cel = &cacarray(cache)[i];
		CACHEEL celnext = cacfree(cache);
		init_cel(cel);
		if (celnext) {
			cnext(cel) = celnext;
			cprev(celnext) = cel;
		}
		cacfree(cache) = cel;
	}
	return cache;
}
/*=============================
 * delete_cache -- Delete cache entirely
 *===========================*/
static void
delete_cache (CACHE * pcache)
{
	INT num=0;
	CACHE cache = *pcache;
	CACHEEL frst=0;
	if (!cache) return;
	/* Loop through all cache elements, freeing each */
	while ((frst = cacfirstdir(cache)) != 0) {
		remove_cel_from_cache(cache, frst);
	}
	/* TODO: Need to delete cache elements on free list */
	num = get_table_count(cacdata(cache));
	ASSERT(num == 0);
	destroy_table(cacdata(cache));
	stdfree(cacarray(cache));
	stdfree(cache);
	*pcache = 0;
}
/*=============================
 * init_cel -- Initialize new cacheel
 *===========================*/
static void
init_cel (CACHEEL cel)
{
	memset(cel, 0, sizeof(*cel));
	cel->c_magic = cel_magic;
}
/*=================================================
 * remove_direct -- Unlink CACHEEL from direct list
 *===============================================*/
static void
remove_direct (CACHE cache, CACHEEL cel)
{
	CACHEEL prev = cprev(cel);
	CACHEEL next = cnext(cel);
	ASSERT(cache);
	ASSERT(cel);
	if (prev) cnext(prev) = next;
	if (next) cprev(next) = prev;
	if (!prev) cacfirstdir(cache) = next;
	if (!next) caclastdir(cache) = prev;
	cacsizedir(cache)--;
}
/*===========================================================
 * first_direct -- Make unlinked CACHEEL first in direct list
 *=========================================================*/
static void
first_direct (CACHE cache, CACHEEL cel)
{
	CACHEEL frst = cacfirstdir(cache);
	ASSERT(cache);
	ASSERT(cel);
	cacsizedir(cache)++;
	cprev(cel) = NULL;
	cnext(cel) = frst;
	if (frst) cprev(frst) = cel;
	if (!frst) caclastdir(cache) = cel;
	cacfirstdir(cache) = cel;
}
/*============================================================
 * direct_to_first -- Make direct CACHEEL first in direct list
 *==========================================================*/
static void
direct_to_first (CACHE cache, CACHEEL cel)
{
	ASSERT(cache);
	ASSERT(cel);
	if (cel == cacfirstdir(cache)) return;
	remove_direct(cache, cel);
	first_direct(cache, cel);
}
/*========================================================
 * add_to_direct -- Add new CACHEEL to direct part of cache
 * reportmode: if True, then return NULL rather than aborting
 *   if there is no record. Also return NULL for deleted
 *   records (of length less than 6???)
 *  cache:      [IN]  which cache to which to add
 *  key:        [IN]  key of record to be added
 *  reportmode: [IN] if non-zero, failures should be silent
 * Only called if record is not already in cache (caller's responsibility)
 *======================================================*/
static CACHEEL
add_to_direct (CACHE cache, CNSTRING key, INT reportmode)
{
	STRING rawrec=0;
	INT len=0;
	CACHEEL cel=0;
	RECORD rec=0;
	int i, j;

	ASSERT(cache);
	ASSERT(key);
	rec = NULL;
	if ((rawrec = retrieve_raw_record(key, &len))) 
		/* 2003-11-22, we should use string_to_node here */
		rec = string_to_record(rawrec, key, len);
	if (!rec)
	{
		ZSTR zstr=zs_newn(256);
		if(listbadkeys) {
			if(strlen(badkeylist) < 80 - strlen(key) - 2) {
				if (badkeylist[0])
					strcat(badkeylist, ",");
				strcat(badkeylist, key);
			}
			return(NULL);
		}
		if (reportmode) return(NULL);
		crashlogn(_("Database error caused by reference to nonexisting key <%s>."), (char *)key);
		crashlogn(_("It might be possible to fix this with btedit."));
		zs_sets(zstr, _("Neighboring keys include:"));
		for(i = 0; i < 10; i++)
		{
			j = keyidx + i;
			if(j >= 10) j -= 10;
			if (keybuf[j][0]) {
				zs_appc(zstr, ' ');
				zs_apps(zstr, keybuf[j]);
			}
		}
		crashlogn(zs_str(zstr));
		zs_free(&zstr);
		/* deliberately fall through to let ASSERT(rec) fail */
	}
	ASSERT(rec);
	/* record was just loaded, nztop should not need to load it */
	cel = node_to_cache(cache, nztop(rec));
	ASSERT(!crecord(cel));
	/* node_to_cache did a first_direct call, so record in cache */
	record_set_cel(rec, cel);
	/* our new rec above has one reference, which is held by cel */
	crecord(cel) = rec;
	stdfree(rawrec);
	ASSERT(cel->c_magic == cel_magic);
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
	if ((cel = (CACHEEL) valueof_ptr(cacdata(cache), key))) {
		ASSERT(cnode(cel));
		ASSERT(cel->c_magic == cel_magic);
		direct_to_first(cache, cel);
		if (tag) {
			ASSERT(eqstr(tag, ntag(cnode(cel))));
			ASSERT(crecord(cel));
			ASSERT(eqstr(key, nzkey(crecord(cel))));
		}
		return cel;
	}
	cel = add_to_direct(cache, key, reportmode);
	if (cel && tag) {
		ASSERT(eqstr(tag, ntag(cnode(cel))));
		ASSERT(crecord(cel));
		ASSERT(eqstr(key, nzkey(crecord(cel))));
	}
	return cel;
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
	ASSERT(cache);
	ASSERT(key);
	if (!(cel = key_to_cacheel(cache, key, tag, FALSE)))
		return NULL;
	return cnode(cel);
}
/*===============================================================
 * key_typed_to_record -- Return record from key; add to cache if not there
 * asserts if failure
 * returns addref'd record
 *=============================================================*/
static RECORD
key_typed_to_record (CACHE cache, CNSTRING key, STRING tag)
{
	CACHEEL cel;
	ASSERT(cache);
	ASSERT(key);
	if (!(cel = key_to_cacheel(cache, key, tag, FALSE)))
		return NULL;
	return get_record_for_cel(cel);
}
/*===================================
 * get_record_for_cel -- get or create new record from cacheel
 * returns addref'd record
 *=================================*/
RECORD
get_record_for_cel (CACHEEL cel)
{
	RECORD rec=0;
	NODE node=0;
	STRING key=0;

	ASSERT(cel);
	if (crecord(cel)) {
		rec = crecord(cel);
		addref_record(rec);
		return rec;
	}
	ASSERT(cnode(cel));
	node = cnode(cel);
	ASSERT(nxref(node));

	key = ckey(cel);

	rec = make_new_record_with_info(key, cel);
	record_set_cel(rec, cel);
	crecord(cel) = rec;
	key = node_to_key(node);

	set_record_key_info(rec, key);
	addref_record(rec);
	return rec;
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
	ASSERT(cache);
	if (!key) return NULL;
	if (!(cel = key_to_cacheel(cache, key, tag, TRUE)))
		return NULL;
	return cnode(cel);
}
/*===============================================================
 * qkey_typed_to_record -- Return record from key; add to cache if not there
 * report mode - returns NULL if failure
 * returns addref'd record
 *=============================================================*/
static RECORD
qkey_typed_to_record (CACHE cache, CNSTRING key, STRING tag)
{
	CACHEEL cel=0;
	RECORD rec=0;

	ASSERT(cache);
	ASSERT(key);
	if (!(cel = key_to_cacheel(cache, key, tag, TRUE)))
		return NULL;
	rec = get_record_for_cel(cel);
	return rec;
}
/*======================================
 * lock_cache -- Lock CACHEEL into direct cache
 *====================================*/
void
lock_cache (CACHEEL cel)
{
	ASSERT(cnode(cel)); /* must be in direct */
	++cclock(cel);
	ASSERT(cclock(cel)>0);
}
/*======================================
 * lockrpt_cache -- Lock CACHEEL into direct cache (for report program)
 *====================================*/
void
lockrpt_cache (CACHEEL cel)
{
	ASSERT(cnode(cel)); /* must be in direct */
	/* put real lock on to actually lock it */
	++cclock(cel);
	/* put report lock on, so we can free orphaned report locks */
	++ccrptlock(cel);
	ASSERT(cclock(cel)>0);
}
/*======================================
 * cel_rptlocks -- return report lock count for specified cache element
 *====================================*/
INT
cel_rptlocks (CACHEEL cel)
{
	return ccrptlock(cel);
}
/*======================================
 * lock_record_in_cache -- Load record into cache & lock it
 *====================================*/
void
lock_record_in_cache (RECORD rec)
{
	NODE node=0;
	CACHEEL cel=0;
	ASSERT(rec);
	node = nztop(rec); /* force record to be loaded in cache */
	cel = nzcel(rec);
	++cclock(cel);
	ASSERT(cclock(cel) > 0);
}
/*==========================================
 * unlock_cache -- Unlock CACHEEL from direct cache
 *========================================*/
void
unlock_cache (CACHEEL cel)
{
	ASSERT(cclock(cel) > 0);
	ASSERT(cnode(cel));
	--cclock(cel);
}
/*==========================================
 * unlockrpt_cache -- Unlock CACHEEL from direct cache (report program call)
 *========================================*/
void
unlockrpt_cache (CACHEEL cel)
{
	ASSERT(cclock(cel) > 0);
	ASSERT(ccrptlock(cel) > 0);
	ASSERT(cnode(cel));
	--cclock(cel);
	--ccrptlock(cel);
}
/*======================================
 * unlock_record_from_cache -- Remove lock on record
 *====================================*/
void
unlock_record_from_cache (RECORD rec)
{
	CACHEEL cel=0;
	ASSERT(rec);
	cel = nzcel(rec);
	ASSERT(cel);
	ASSERT(cclock(cel) > 0);
	--cclock(cel);
}
/*=========================================
 * cache_get_lock_counts -- Fill in lock counts
 * does not include report locks (but, this isn't
 *  called during reports anyway)
 *=======================================*/
static void
cache_get_lock_counts (CACHE ca, INT * locks)
{
	CACHEEL cel;
	for (cel = cacfirstdir(ca); cel; cel = cnext(cel)) {
		if (cclock(cel) && locks) ++(*locks);
	}
}
/*=========================================
 * get_cache_stats -- Calculate cache stats
 *  returns static buffer
 *=======================================*/
static ZSTR
get_cache_stats (CACHE ca)
{
	ZSTR zstr = zs_new();
	INT lo=0;
	cache_get_lock_counts(ca, &lo);
	zs_appf(zstr
		, "d:%d/%d (l:%d)"
		, cacsizedir(ca), cacmaxdir(ca), lo
		);
	return zstr;
}
/*=========================================
 * get_cache_stats_indi -- Return indi cache stats
 *=======================================*/
ZSTR
get_cache_stats_indi (void)
{
	return get_cache_stats(indicache);
}
/*=========================================
 * get_cache_stats_fam -- Return fam cache stats
 *=======================================*/
ZSTR
get_cache_stats_fam (void)
{
	return get_cache_stats(famcache);
}
/*============================================
 * ensure_cel_has_record -- Make sure cache element has record
 *  (node_to_cache, which creates cels, doesn't create records)
 *==========================================*/
static void
ensure_cel_has_record (CACHEEL cel)
{
	RECORD rec = get_record_for_cel(cel); /* addref'd */
	release_record(rec);
}
/*============================================
 * add_new_indi_to_cache -- Add person to person cache
 *==========================================*/
void
add_new_indi_to_cache (RECORD rec)
{
	CACHEEL cel=0;
	NODE root = is_record_temp(rec);
	cel = node_to_cache(indicache, root);
	ASSERT(!crecord(cel));
	record_set_cel(rec, cel);
	crecord(cel) = rec;
	addref_record(rec); /* cel holds reference */
}
/*===========================================
 * fam_to_cache -- Add family to family cache
 *=========================================*/
void
fam_to_cache (NODE node)
{
	CACHEEL cel = node_to_cache(famcache, node);
	ensure_cel_has_record(cel);
}
/*==========================================
 * even_to_cache -- Add event to event cache
 *========================================*/
void
even_to_cache (NODE node)
{
	CACHEEL cel = node_to_cache(evencache, node);
	ensure_cel_has_record(cel);
}
/*============================================
 * sour_to_cache -- Add source to source cache
 *==========================================*/
void
sour_to_cache (NODE node)
{
	CACHEEL cel = node_to_cache(sourcache, node);
	ensure_cel_has_record(cel);
}
/*===========================================
 * othr_to_cache -- Add other record to cache
 *=========================================*/
void
othr_to_cache (NODE node)
{
	CACHEEL cel = node_to_cache(othrcache, node);
	ensure_cel_has_record(cel);
}
/*========================================
 * node_to_cache -- Add node tree to cache
 *  This is a high-level entry point, which validates
 *  and delegates the work
 *  node tree must be valid, and of the correct type
 *  (INDI node trees may only be added to INDI cache, etc)
 *  This puts node into cache (first_direct)
 *======================================*/
static CACHEEL
node_to_cache (CACHE cache, NODE top)
{
	STRING key=0;
	CACHEEL cel=0;
	ASSERT(cache);
	ASSERT(top);
	ASSERT(!nparent(top));	/* should be a root */
	ASSERT(!nsibling(top)); /* should be a root */
	if (nestr(cacname(cache), "OTHR")) {
		/* only INDI records in INDI cache, etc */
		if (!eqstr(cacname(cache), ntag(top))) {
			crashlog(_("Bad cache entry <%s> != <%s>"), cacname(cache), ntag(top));
			ASSERT(0);
		}
	}
	key = node_to_key(top);
	ASSERT(key);
	/* ASSERT that record is not in cache */
	/* We're not supposed to be called if record in cache */
	ASSERT(!valueof_ptr(cacdata(cache), key));
	cel = get_free_cacheel(cache);
	put_node_in_cache(cache, cel, top, key);
	return cel;
}
/*=======================================================
 * get_free_cacheel -- Remove and return entry from free list
 *=====================================================*/
static CACHEEL
get_free_cacheel (CACHE cache)
{
	CACHEEL cel=0, celnext=0;

	/* If free list is empty, move least recently used entry to free list */
	if (!cacfree(cache)) {
		/* find least recently used by unlocked entry */
		for (cel = caclastdir(cache); cel && cclock(cel); cel = cprev(cel)) {
		}
		if (!cel) {
			crashlog(_("Cache [%s] overflowed its max size (%d)"), cacname(cache), cacmaxdir(cache));
			ASSERT(0);
		}
		remove_from_cache(cache, ckey(cel));
	}

	cel = cacfree(cache);
	ASSERT(cel);

	/* remove entry from free list */
	celnext = cnext(cel);
	cacfree(cache) = celnext;
	if (celnext)
		cprev(celnext) = 0;
	/* reinitialize entry */
	init_cel(cel);

	return cel;
}
/*=======================================================
 * set_all_nodetree_to_cel -- clear all the cel pointers in a node tree
 *=====================================================*/
static void
set_all_nodetree_to_cel (NODE node, CACHEEL cel)
{
	BOOLEAN travdone = FALSE;
	/* Now set all nodes in tree to point to cache record */
	while (!travdone) {
		node->n_cel = cel;
		/* go to bottom of tree */
		while (nchild(node)) {
			node = nchild(node);
			node->n_cel = cel;
		}
		/* find next node in traversal/ascent */
		while (!nsibling(node)) {
			if (!nparent(node)) {
				travdone=TRUE;
				break;
			}
			node = nparent(node);
		}
		node = nsibling(node);
	}
}
/*=======================================================
 * put_node_in_cache -- Low-level work of loading node into cacheel supplied
 *=====================================================*/
static void
put_node_in_cache (CACHE cache, CACHEEL cel, NODE node, STRING key)
{
	BOOLEAN travdone = FALSE;
	ASSERT(cache);
	ASSERT(node);
	ASSERT(cacsizedir(cache) < cacmaxdir(cache));
	init_cel(cel);
	insert_table_ptr(cacdata(cache), key, cel);
	cnode(cel) = node;
	ckey(cel) = strsave(key);
	cclock(cel) = FALSE;
	first_direct(cache, cel);
	/* Now set all nodes in tree to point to cache record */
	while (!travdone) {
		node->n_cel = cel;
		/* go to bottom of tree */
		while (nchild(node)) {
			node = nchild(node);
			node->n_cel = cel;
		}
		/* find next node in traversal/ascent */
		while (!nsibling(node)) {
			if (!nparent(node)) {
				travdone=TRUE;
				break;
			}
			node = nparent(node);
		}
		node = nsibling(node);
	}
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
 * remove_from_cache_by_key -- Remove record from cache
 *===========================================*/
void
remove_from_cache_by_key (CNSTRING key)
{
	switch(key[0]) {
	case 'I': remove_from_cache(indicache, key); break;
	case 'F': remove_from_cache(famcache, key); break;
	case 'S': remove_from_cache(sourcache, key); break;
	case 'E': remove_from_cache(evencache, key); break;
	case 'X': remove_from_cache(othrcache, key); break;
	default: ASSERT(0); break;
	}
}
/*=============================================
 * remove_from_cache -- Move cache entry to free list
 *===========================================*/
static void
remove_from_cache (CACHE cache, CNSTRING key)
{
	CACHEEL cel=0;

	if (!key || *key == 0 || !cache)
		return;
	/* If it has a key, it is in the cache */
	cel = valueof_ptr(cacdata(cache), key);
	remove_cel_from_cache(cache, cel);
}
/*=============================================
 * remove_from_cache -- Move cache entry to free list
 *  Requires non-null input
 *===========================================*/
static void
remove_cel_from_cache (CACHE cache, CACHEEL cel)
{
	CACHEEL celnext=0;
	STRING key = ckey(cel);

	/* caller ensured cache && key are non-null */
	ASSERT(cel);

	ASSERT(!cclock(cel)); /* not supposed to remove locked elements */
	ASSERT(cnode(cel));
	remove_direct(cache, cel);

	/* Clear all node tree info */
	if (1) {
		NODE node = cnode(cel);
		if (node)
			set_all_nodetree_to_cel(node, 0);
		cnode(cel) = 0;
		free_nodes(node);
	}

	celnext = cacfree(cache);
	cnext(cel) = celnext;
	if (celnext)
		cprev(celnext) = cel;
	cprev(cel) = 0;
	ckey(cel) = 0;
	if (crecord(cel)) {
		/* cel holds the original reference to the record */
		RECORD rec = crecord(cel);
		record_remove_cel(rec, cel);
		release_record(rec);
		crecord(cel) = 0;
	}
	cacfree(cache) = cel;
	delete_table_element(cacdata(cache), key);
	stdfree(key); /* alloc'd when assigned to ckey(cel) */
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
	CACHEEL cel=0;
	if (!indi || !nztop(indi)) return NULL;
	cel = nzcel(indi);
	if (cel) return cel;
	/*
	This is not efficient, rereading the record
	But we can't just steal the record given us
	without some transfer of memory ownership
	*/
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
fam_to_cacheel (RECORD frec)
{
	CACHEEL cel;
	if (!frec) return NULL;
	cel = key_to_fam_cacheel(rmvat(nxref(nztop(frec))));
	ASSERT(cel);
	return cel;
}
/*==================================================
 * fam_to_cacheel_old -- Convert family to cache element
 *================================================*/
CACHEEL
fam_to_cacheel_old (NODE fam)
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
 * record_to_cacheel -- Convert any record to cache element
 *================================================*/
CACHEEL
record_to_cacheel (RECORD rec)
{
	STRING key = rmvat(nxref(nztop(rec)));
	switch(key[0]) {
	case 'I': return indi_to_cacheel(rec);
	case 'F': return fam_to_cacheel(rec);
	case 'S': return sour_to_cacheel(nztop(rec));
	case 'E': return even_to_cacheel(nztop(rec));
	case 'X': return othr_to_cacheel(nztop(rec));
	}
	ASSERT(0); return NULL;
}
/*==================================================
 * node_to_cacheel_old -- Convert any node to cache element
 *================================================*/
CACHEEL
node_to_cacheel_old (NODE node)
{
	STRING key = rmvat(nxref(node));
	switch(key[0]) {
	case 'I': return indi_to_cacheel_old(node);
	case 'F': return fam_to_cacheel_old(node);
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
 * qkey_to_typed_cacheel -- Lookup/load key
 *============================================*/
/* unused
static CACHEEL
qkey_to_typed_cacheel (STRING key)
{
	char ntype;
	ASSERT(key);
	ASSERT(key[0]);
	ntype = key[0];
	switch(ntype) {
	case 'I': return qkey_to_indi_cacheel(key);
	case 'F': return qkey_to_fam_cacheel(key);
	case 'S': return qkey_to_even_cacheel(key);
	case 'E': return qkey_to_sour_cacheel(key);
	case 'X': return qkey_to_othr_cacheel(key);
	}
	ASSERT(0);
}
unused */
/*==============================================
 * cacheel_to_key -- Return key of record inside of cache element
 *  handle NULL input
 *============================================*/
CNSTRING
cacheel_to_key (CACHEEL cel)
{
	CNSTRING key = cel ? ckey(cel) : 0;
	return key;
}
/*==============================================
 * cacheel_to_node -- Return root node of record inside of cache element
 *  loads cache element if needed
 *  handle NULL input
 *============================================*/
NODE
cacheel_to_node (CACHEEL cel)
{
	if (!cel) return NULL;
	if (!cnode(cel)) {
		CACHEEL cel2 = key_to_indi_cacheel(ckey(cel));
		ASSERT(cel2 == cel);
		ASSERT(cnode(cel));
	}
	return cnode(cel);
}
/*==============================================
 * is_cel_loaded -- If cache element is loaded, return root
 *  handle NULL input
 *============================================*/
NODE
is_cel_loaded (CACHEEL cel)
{
	if (!cel) return NULL;
	return cnode(cel);
}
/*==============================================
 * cel_remove_record -- Our record informing us it is destructing
 *  Requires non-null inputs
 *============================================*/
void
cel_remove_record (CACHEEL cel, RECORD rec)
{
	ASSERT(cel);
	ASSERT(cel->c_magic == cel_magic);
	ASSERT(rec);
	if (crecord(cel) == rec) {
		crecord(cel) = 0;
	}
}
/*=======================================================
 * set_all_nodetree_to_root_cel -- Propagate cel from root to all descendants
 *=====================================================*/
void
set_all_nodetree_to_root_cel (NODE root)
{
	set_all_nodetree_to_cel(root, root->n_cel);
}
/*=======================================================
 * free_all_rprtlocks_in_cache -- Remove any rptlocks on any
 *  elements in this cache, and return number removed
 *=====================================================*/
static int
free_all_rprtlocks_in_cache (CACHE cache)
{
	INT ct=0;
	CACHEEL cel=0;

	for (cel = caclastdir(cache); cel; cel = cprev(cel)) {
		if (ccrptlock(cel)) {
			INT delta = ccrptlock(cel);
			ccrptlock(cel) = 0;
			ASSERT(cclock(cel) >= delta);
			cclock(cel) -= delta;
			++ct;
		}
	}
	return ct;
}
/*=======================================================
 * free_all_rprtlocks -- Remove any rptlocks on any
 *  elements in all caches, and return number removed
 *=====================================================*/
int
free_all_rprtlocks (void)
{
	int ct=0;
	ct += free_all_rprtlocks_in_cache(indicache);
	ct += free_all_rprtlocks_in_cache(famcache);
	ct += free_all_rprtlocks_in_cache(sourcache);
	ct += free_all_rprtlocks_in_cache(sourcache);
	ct += free_all_rprtlocks_in_cache(othrcache);
	return ct;
}
