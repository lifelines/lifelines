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
 * keytonod.c -- Convert between keys and node trees
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 08 May 94    3.0.2 - 23 Dec 94
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "cache.h"

static CACHE create_cache();
static NODE key_to_node();
static CACHEEL key_to_cacheel();
static dereference();
static CACHE indicache, famcache, evencache, sourcache, othrcache;

/*=====================================
 * key_to_indi -- Convert key to person
 *===================================*/
NODE key_to_indi (key)
STRING key;
{
	return key_to_node(indicache, key, "INDI");
}
/*====================================
 * key_to_fam -- Convert key to family
 *==================================*/
NODE key_to_fam (key)
STRING key;
{
	return key_to_node(famcache, key, "FAM");
}
/*====================================
 * key_to_even -- Convert key to event
 *==================================*/
NODE key_to_even (key)
STRING key;
{
	return key_to_node(evencache, key, "EVEN");
}
/*=====================================
 * key_to_sour -- Convert key to source
 *===================================*/
NODE key_to_sour (key)
STRING key;
{
	return key_to_node(sourcache, key, "SOUR");
}
/*====================================
 * key_to_othr -- Convert key to other
 *==================================*/
NODE key_to_othr (key)
STRING key;
{
	return key_to_node(othrcache, key, NULL);
}
/*=====================================================
 * key_to_indi_cacheel -- Convert key to person cacheel
 *===================================================*/
CACHEEL key_to_indi_cacheel (key)
STRING key;
{
	return key_to_cacheel(indicache, key, "INDI");
}
/*====================================================
 * key_to_fam_cacheel -- Convert key to family_cacheel
 *==================================================*/
CACHEEL key_to_fam_cacheel (key)
STRING key;
{
	return key_to_cacheel(famcache, key, "FAM");
}
/*====================================================
 * key_to_even_cacheel -- Convert key to event_cacheel
 *==================================================*/
CACHEEL key_to_even_cacheel (key)
STRING key;
{
	return key_to_cacheel(evencache, key, "EVEN");
}
/*=====================================================
 * key_to_sour_cacheel -- Convert key to source_cacheel
 *===================================================*/
CACHEEL key_to_sour_cacheel (key)
STRING key;
{
	return key_to_cacheel(sourcache, key, "SOUR");
}
/*====================================================
 * key_to_othr_cacheel -- Convert key to other_cacheel
 *==================================================*/
CACHEEL key_to_othr_cacheel (key)
STRING key;
{
	return key_to_cacheel(othrcache, key, NULL);
}
/*======================================
 * init_caches -- Create and init caches
 *====================================*/
init_caches ()
{
	indicache = create_cache((INT)300, (INT)3000);
	famcache  = create_cache((INT)200, (INT)2000);
	evencache = create_cache((INT)200, (INT)2000);
	sourcache = create_cache((INT)200, (INT)2000);
	othrcache = create_cache((INT)200, (INT)2000);
}
/*=============================
 * create_cache -- Create cache
 *===========================*/
static CACHE create_cache (dirsize, indsize)
INT dirsize, indsize;
{
	CACHE cache;
	if (dirsize < 1) dirsize = 1;
	if (indsize < 1) indsize = 1;
	cache = (CACHE) stdalloc(sizeof(*cache));
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
static remove_direct (cache, cel)
CACHE cache;
CACHEEL cel;
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
static remove_indirect (cache, cel)
CACHE cache;
CACHEEL cel;
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
static first_direct (cache, cel)
CACHE cache;
CACHEEL cel;
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
 *=============================================================*/
static first_indirect (cache, cel)
CACHE cache;
CACHEEL cel;
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
static remove_last (cache)
CACHE cache;
{
	CACHEEL cel = clastind(cache);
	STRING key;
	ASSERT(cel);
	remove_indirect(cache, cel);
	key = ckey(cel);
	stdfree(cel);
	delete_table(cdata(cache), key);
	stdfree(key);
}
/*============================================================
 * direct_to_first -- Make direct CACHEEL first in direct list
 *==========================================================*/
static direct_to_first (cache, cel)
CACHE cache;
CACHEEL cel;
{
	ASSERT(cache && cel);
	if (cel == cfirstdir(cache)) return;
	remove_direct(cache, cel);
	first_direct(cache, cel);
}
/*==================================================================
 * indirect_to_first -- Make indirect CACHEEL first in indirect list
 *================================================================*/
static indirect_to_first (cache, cel)
CACHE cache;
CACHEEL cel;
{
	ASSERT(cache && cel);
	remove_indirect(cache, cel);
	dereference(cel);
	first_direct(cache, cel);
}
/*==============================================================
 * direct_to_indirect -- Make last direct CACHEEL first indirect
 *============================================================*/
static direct_to_indirect (che)
CACHE che;
{
	CACHEEL cel = clastdir(che);
	for (cel = clastdir(che); cel && clock(cel); cel = cprev(cel))
		;
	ASSERT(cel);
	remove_direct(che, cel);
	free_nodes(cnode(cel));
	cnode(cel) = NULL;
	first_indirect(che, cel);
}
/*=====================================================
 * dereference -- Dereference cel by reading its record
 *===================================================*/
static dereference (cel)
CACHEEL cel;
{
	STRING rec;
	INT len;
	NODE node;
	ASSERT(cel);
	ASSERT(rec = retrieve_record(ckey(cel), &len));
	ASSERT(node = string_to_node(rec));
	cnode(cel) = node;
	stdfree(rec);
}
/*========================================================
 * add_to_direct -- Add new CACHEL to direct part of cache
 *======================================================*/
static CACHEEL add_to_direct (cache, key)
CACHE cache;
STRING key;
{
	STRING record;
	INT len;
	CACHEEL cel;
	NODE node;
/*wprintf("add_to_direct: key == %s\n", key);/*DEBUG*/
	ASSERT(cache && key);
	ASSERT(csizedir(cache) < cmaxdir(cache));
	cel = (CACHEEL) stdalloc(sizeof(*cel));
	insert_table(cdata(cache), key = strsave(key), cel);
	ASSERT(record = retrieve_record(key, &len));
	ASSERT(node = string_to_node(record));
	cnode(cel) = node;
	ckey(cel) = key;
	clock(cel) = FALSE;
	first_direct(cache, cel);
	stdfree(record);
	return cel;
}
/*======================================================
 * key_to_cacheel -- Return CACHEEL corresponding to key
 *====================================================*/
static CACHEEL key_to_cacheel (cache, key, tag)
CACHE cache;
STRING key;
STRING tag;
{
	CACHEEL cel;
	if (cel = (CACHEEL) valueof(cdata(cache), key)) {
		if (cnode(cel))
			direct_to_first(cache, cel);
		else {
			if (csizedir(cache) >= cmaxdir(cache))
				direct_to_indirect(cache);
			indirect_to_first(cache, cel);
		}
		if (tag) {
/*wprintf("BEFORE ASSERT: tag, ntag(cnode(cel)) = %s, %s\n", tag,
ntag(cnode(cel)));/*DEBUG*/
			ASSERT(eqstr(tag, ntag(cnode(cel))));
		}
		return cel;
	}
	if (csizedir(cache) >= cmaxdir(cache)) {
		if (csizeind(cache) >= cmaxind(cache))
			remove_last(cache);
		direct_to_indirect(cache);
	}
	cel = add_to_direct(cache, key);
	if (tag) {
		ASSERT(eqstr(tag, ntag(cnode(cel))));
	}
	return cel;
}
/*===============================================================
 * key_to_node -- Return tree from key; add to cache if not there
 *=============================================================*/
static NODE key_to_node (cache, key, tag)
CACHE cache;
STRING key;
STRING tag;
{
	CACHEEL cel;
	ASSERT(cache && key);
	if (!(cel = key_to_cacheel(cache, key, tag))) return NULL;
	return cnode(cel);
}
/*======================================
 * lock_cache -- Lock CACHEEL into cache
 *====================================*/
lock_cache (cel)
CACHEEL cel;
{
	ASSERT(cnode(cel));
	clock(cel)++;
}
/*==========================================
 * unlock_cache -- Unlock CACHEEL from cache
 *========================================*/
unlock_cache (cel)
CACHEEL cel;
{
	ASSERT(cnode(cel));
	clock(cel)--;
}
/*================================
 * cache_stats -- Show cache stats
 *==============================*/
cache_stats ()
{
	CACHE c = indicache;
	CACHE f = famcache;
	INT n = 0;
	CACHEEL cel;
	for (cel = cfirstdir(c); cel; cel = cnext(cel)) {
		if (clock(cel)) n++;
	}
	mprintf("Cache contents -- I: %dD  %dI  %dL;  F: %dD  %dI",
	    csizedir(c), csizeind(c), n, csizedir(f), csizeind(f));
}
/*============================================
 * indi_to_cache -- Add person to person cache
 *==========================================*/
indi_to_cache (node)
NODE node;
{
	node_to_cache(indicache, node);
}
/*===========================================
 * fam_to_cache -- Add family to family cache
 *=========================================*/
fam_to_cache (node)
NODE node;
{
	node_to_cache(famcache, node);
}
/*==========================================
 * even_to_cache -- Add event to event cache
 *========================================*/
even_to_cache (node)
NODE node;
{
	node_to_cache(evencache, node);
}
/*============================================
 * sour_to_cache -- Add source to source cache
 *==========================================*/
sour_to_cache (node)
NODE node;
{
	node_to_cache(sourcache, node);
}
/*===========================================
 * othr_to_cache -- Add other record to cache
 *=========================================*/
othr_to_cache (node)
NODE node;
{
	node_to_cache(othrcache, node);
}
/*========================================
 * node_to_cache -- Add node tree to cache
 *======================================*/
node_to_cache (cache, node)
CACHE cache;
NODE node;
{
	STRING key = rmvat(nxref(node));
	ASSERT(cache && node);
	ASSERT(!valueof(cdata(cache), key));
	if (csizedir(cache) >= cmaxdir(cache)) {
		if (csizeind(cache) >= cmaxind(cache)) {
			remove_last(cache);
			direct_to_indirect(cache);
			add_node_to_direct(cache, node);
		} else {
			direct_to_indirect(cache);
			add_node_to_direct(cache, node);
		}
	} else {
		add_node_to_direct(cache, node);
	}
}
/*=======================================================
 * add_node_to_direct -- Add node to direct part of cache
 *=====================================================*/
add_node_to_direct(cache, node)
CACHE cache;
NODE node;
{
	CACHEEL cel;
	STRING key;
	ASSERT(cache && node);
	ASSERT(csizedir(cache) < cmaxdir(cache));
	cel = (CACHEEL) stdalloc(sizeof(*cel));
	insert_table(cdata(cache), key = strsave(rmvat(nxref(node))), cel);
	cnode(cel) = node;
	ckey(cel) = key;
	clock(cel) = FALSE;
	first_direct(cache, cel);
}
/*==============================================
 * remove_indi_cache -- Remove person from cache
 *============================================*/
remove_indi_cache (key)
STRING key;
{
	remove_from_cache(indicache, key);
}
/*=============================================
 * remove_fam_cache -- Remove family from cache
 *===========================================*/
remove_fam_cache (key)
STRING key;
{
	remove_from_cache(famcache, key);
}
/*=============================================
 * remove_from_cache -- Remove entry from cache
 *===========================================*/
remove_from_cache (cache, key)
CACHE cache;
STRING key;
{
	CACHEEL cel;
	if (!key || *key == 0 || !cache) return;
	if (!(cel = (CACHEEL) valueof(cdata(cache), key))) return;
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
STRING value_to_xref (val)
STRING val;
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
CACHEEL indi_to_cacheel (indi)
NODE indi;
{
        CACHEEL cel;
        if (!indi) return NULL;
/*wprintf("indi_to_cacheel: %s\n", nxref(indi));/*DEBUG*/
        cel = key_to_indi_cacheel(rmvat(nxref(indi)));
        ASSERT(cel);
        return cel;
}
/*==================================================
 * fam_to_cacheel -- Convert family to cache element
 *================================================*/
CACHEEL fam_to_cacheel (fam)
NODE fam;
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
CACHEEL sour_to_cacheel (node)
NODE node;
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
CACHEEL even_to_cacheel (even)
NODE even;
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
CACHEEL othr_to_cacheel (othr)
NODE othr;
{
        CACHEEL cel;
        if (!othr) return NULL;
	cel = key_to_othr_cacheel(rmvat(nxref(othr)));
        ASSERT(cel);
        return cel;
}
/*==============================================
 * key_of_record -- Return display key of record
 *============================================*/
STRING key_of_record (node)
NODE node;
{
        NODE refn;
        ASSERT(node);
        refn = REFN(node);
        if (refn && nval(refn)) return nval(refn);
        return rmvat(nxref(node)) + 1;
}
