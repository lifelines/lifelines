/* 
   Copyright (c) 1991-2005 Thomas T. Wetmore IV
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "llstdlib.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "vtable.h"
#include "cache.h"


/*==============================
 * RECORD -- in-memory representation of GEDCOM record
 *============================*/
struct tag_record { /* RECORD */
	/* a LIST is an OBJECT */
	struct tag_vtable * vtable; /* generic object table (see vtable.h) */
	INT refcnt; /* reference counted object */
	NODE rec_top;           /* for non-cache records */
	NKEY rec_nkey;
	CACHEEL rec_cel; /* cache wrapper */
};

/*********************************************
 * local function prototypes
 *********************************************/

static void free_rec(RECORD rec);
static BOOLEAN is_record_loaded (RECORD rec);
static void record_destructor(VTABLE *obj);

/*********************************************
 * local variables
 *********************************************/

static struct tag_vtable vtable_for_record = {
	VTABLE_MAGIC
	, "record"
	, &record_destructor
	, &refcountable_isref
	, &refcountable_addref
	, &refcountable_delref
	, 0 /* copy_fnc */
	, &generic_get_type_name
};
static int f_nrecs=0;
/*===================================
 * alloc_new_record -- record allocator
 *  perhaps should use special allocator like nodes
 * returns addref'd record
 *=================================*/
RECORD
alloc_new_record (void)
{
	RECORD rec;
	++f_nrecs;
	rec = (RECORD)stdalloc(sizeof(*rec));
	memset(rec, 0, sizeof(*rec));
	/* these must be filled in by caller */
	strcpy(rec->rec_nkey.key, "");
	rec->rec_nkey.keynum = 0;
	rec->rec_nkey.ntype = 0;
	++rec->refcnt;
	return rec;
}
/*===================================
 * free_rec -- record deallocator 
 *  Works for both free records (not in cache, have own node tree)
 *  and bound records (in cache, point to cache element)
 * Created: 2000/12/30, Perry Rapp
 *=================================*/
static void
free_rec (RECORD rec)
{
	--f_nrecs;
	if (rec->rec_cel) {
		/* cached record */
		/* cel memory belongs to cache, but we must tell it
		that we're dying, so it doesn't point to us anymore */
		cel_remove_record(rec->rec_cel, rec);
		rec->rec_cel = 0; /* cel memory belongs to cache */
	} else {
		/* free record */
		ASSERT(rec->rec_top);
		free_nodes(rec->rec_top);
		rec->rec_top = 0;
	}
	strcpy(rec->rec_nkey.key, "");
	stdfree(rec);
}
/*==============================================
 * nztop -- Return first NODE of a RECORD
 *  handle NULL input
 *============================================*/
NODE
nztop (RECORD rec)
{
	if (!rec) return 0;
	if (rec->rec_top) {
		/* This is only for records not in the cache */
		ASSERT(!rec->rec_cel);
		return rec->rec_top;
	}
	if (!is_record_loaded(rec)) {
		/* Presumably we're out-of-date because our record fell out of cache */
		/* Anyway, load via cache (actually just point to cache data) */
		CACHEEL cel = key_to_unknown_cacheel(rec->rec_nkey.key);
		rec->rec_cel = cel;
	}
	return is_cel_loaded(rec->rec_cel);
}
/*==============================================
 * nzkey -- Return key of record
 *  handle NULL input
 * eg, "I85" for a person I85
 *============================================*/
CNSTRING
nzkey (RECORD rec)
{
	if (!rec) return 0;
	return rec->rec_nkey.key;
}
/*==============================================
 * nzkeynum -- Return record number of record
 *  handle NULL input
 * eg, 85 for a person I85
 *============================================*/
INT
nzkeynum (RECORD rec)
{
	if (!rec) return 0;
	return rec->rec_nkey.keynum;
}
/*==============================================
 * nztype -- Return type number (char) of record
 *  handle NULL input
 * eg, 'I' for a person I85
 *============================================*/
char
nztype (RECORD rec)
{
	if (!rec) return 0;
	return rec->rec_nkey.ntype;
}
/*==============================================
 * nzcel -- Return cache element associated with record
 *  handle NULL input
 * does not load cache if not associated
 *============================================*/
CACHEEL
nzcel (RECORD rec)
{
	if (!rec) return 0;
	return rec->rec_cel;
}
/*==============================================
 * set_record_cache_info -- Assign cache element to record
 *  both inputs must be valid (non-null)
 *============================================*/
void
set_record_cache_info (RECORD rec, CACHEEL cel)
{
	ASSERT(rec && cel);
	rec->rec_top = 0;
	rec->rec_cel = cel;
	cel_set_record(rec->rec_cel, rec);
}
/*===================================
 * set_record_key_info -- put key info into record
 * Also set the xref of the root node correctly
 * Created: 2001/02/04, Perry Rapp
 *=================================*/
void
set_record_key_info (RECORD rec, char ntype, INT keynum)
{
	char xref[12];
	char key[MAXKEYWIDTH+1];
	NODE node;
	sprintf(key, "%c%d", ntype, keynum);
	sprintf(xref, "@%s@", key);
	strcpy(rec->rec_nkey.key, key);
	rec->rec_nkey.keynum = keynum;
	rec->rec_nkey.ntype = ntype;
	if ((node = rec->rec_top) != 0) {
		if (!nxref(node) || !eqstr(nxref(node), xref)) {
			if (nxref(node)) stdfree(nxref(node));
			nxref(node) = strsave(xref);
		}
	}
}
/*==============================================
 * is_record_loaded -- Check if record has its node tree
 *============================================*/
static BOOLEAN
is_record_loaded (RECORD rec)
{
	STRING xref=0, temp=0;
	NODE root=0;

		/* does the record have a nodetree ? */
	if (!rec || !rec->rec_cel)
		return FALSE;
	root = is_cel_loaded(rec->rec_cel);
	if (!root)
		return FALSE;

		/* Now, is it the correct nodetree ? */

	temp = rec->rec_nkey.key; /* eg, "I50" */

	xref = nxref(root); /* eg, "@I50@" */
	ASSERT(xref[0] == '@');

	/* now xref="@I50@" and temp="I50" */
	++xref;

	/* now xref="I50@" and temp="I50" */

	while (1) {
			/* distinguish I50@ vs I50 from I50@ vs I500 */
		if (*xref == '@') return (*temp == 0);
			/* distinguish I50@ vs I50 from I500@ vs I50 */
		if (*temp == 0) return (*xref == '@');
		if (*xref != *temp) return FALSE;
		++xref;
		++temp;
	}
}
/*==============================================
 * is_record_temp -- Return node tree root if 
 *  this is a temporary record (one not in cache)
 *============================================*/
NODE
is_record_temp (RECORD rec)
{
	if (!rec) return NULL;
	return rec->rec_top;
}
/*===================================
 * init_new_record -- put key info into record
 *  of brand new record
 * Created: 2001/02/04, Perry Rapp
 *=================================*/
void
init_new_record (RECORD rec, char ntype, INT keynum)
{
	set_record_key_info(rec, ntype, keynum);
}
/*===================================
 * create_record_for_keyed_node -- 
 *  Given a node just read from disk, wrap it in an uncached record
 * returns addref'd record
 *=================================*/
RECORD
create_record_for_keyed_node (NODE node, CNSTRING key)
{
	RECORD rec = alloc_new_record();
	INT keynum = 0;
	if (!key)
		key = nxref(node);
	keynum = ll_atoi(&key[1], 0);
	rec->rec_top = node;
	init_new_record(rec, key[0], keynum);
	return rec;
}
/*===================================
 * create_record_for_unkeyed_node -- 
 *  Given a new node tree with no key, wrap it in an uncached record
 * returns addref'd record
 *=================================*/
RECORD
create_record_for_unkeyed_node (NODE node)
{
	RECORD rec = alloc_new_record();
	rec->rec_top = node;
	return rec;
}
/*=================================================
 * addref_record -- increment reference count of record
 *===============================================*/
void
addref_record (RECORD rec)
{
	if (!rec) return;
	++rec->refcnt;
}
/*=================================================
 * delref_record -- decrement reference count of record
 *  and free if appropriate (ref count hits zero)
 *===============================================*/
void
delref_record (RECORD rec)
{
	if (!rec) return;
	--rec->refcnt;
	if (!rec->refcnt) {
		free_rec(rec);
	}
}
/*=================================================
 * record_destructor -- destructor for record
 *  (destructor entry in vtable)
 *===============================================*/
static void
record_destructor (VTABLE *obj)
{
	RECORD rec = (RECORD)obj;
	ASSERT((*obj) == &vtable_for_record);
	free_rec(rec);
}
