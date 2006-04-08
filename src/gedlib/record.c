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
#include "lloptions.h"


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

static RECORD alloc_new_record(void);
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
	, &refcountable_release
	, 0 /* copy_fnc */
	, &generic_get_type_name
};
static int f_nrecs=0;

/*===================================
 * alloc_new_record -- record allocator
 *  perhaps should use special allocator like nodes
 * returns addref'd record
 *=================================*/
static RECORD
alloc_new_record (void)
{
	RECORD rec;
	++f_nrecs;
	rec = (RECORD)stdalloc(sizeof(*rec));
	memset(rec, 0, sizeof(*rec));
	/* these must be filled in by caller */
	rec->vtable = &vtable_for_record;
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
	}
	if (rec->rec_top) {
		/* free node tree */
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
 * record_set_cel -- Assign cache element to record
 *  both inputs must be valid (non-null)
 *============================================*/
void
record_set_cel (RECORD rec, CACHEEL cel)
{
	ASSERT(rec);
	ASSERT(cel);
	rec->rec_top = 0;
	rec->rec_cel = cel;
}
/*==============================================
 * record_remove_cel -- cache element is being cleared
 *  both inputs must be valid (non-null)
 *============================================*/
void
record_remove_cel (RECORD rec, CACHEEL cel)
{
	ASSERT(rec);
	ASSERT(cel);
	ASSERT(rec->rec_cel == cel);
	rec->rec_cel = 0;
}
/*===================================
 * make_new_record_with_info -- creates new record
 *  with key and cacheel set
 * returns addref'd record
 *=================================*/
RECORD
make_new_record_with_info (CNSTRING key, CACHEEL cel)
{
	RECORD rec = alloc_new_record();
	set_record_key_info(rec, key);
	record_set_cel(rec, cel);
	return rec;
}
/*===================================
 * set_record_key_info -- put key info into record
 * Also set the xref of the root node correctly
 *=================================*/
void
set_record_key_info (RECORD rec, CNSTRING key)
{
	char xref[12];
	NODE node=0;
	INT keynum = atoi(key+1);
	char ntype = key[0];
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
init_new_record (RECORD rec, CNSTRING key)
{
	set_record_key_info(rec, key);
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
	if (!key)
		key = nxref(node);
	rec->rec_top = node;
	init_new_record(rec, key);
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
	ASSERT(rec->vtable == &vtable_for_record);
	++rec->refcnt;
}
/*=================================================
 * release_record -- decrement reference count of record
 *  and free if appropriate (ref count hits zero)
 *===============================================*/
void
release_record (RECORD rec)
{
	if (!rec) return;
	ASSERT(rec->vtable == &vtable_for_record);
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
/*=================================================
 * check_record_leaks -- Called when database closing
 *  for debugging
 *===============================================*/
void
check_record_leaks (void)
{
	if (f_nrecs) {
		STRING report_leak_path = getlloptstr("ReportLeakLog", NULL);
		FILE * fp=0;
		if (report_leak_path)
			fp = fopen(report_leak_path, LLAPPENDTEXT);
		if (fp) {
			LLDATE date;
			get_current_lldate(&date);
			fprintf(fp, _("Record memory leaks:"));
			fprintf(fp, " %s", date.datestr);
			fprintf(fp, "\n  ");
			fprintf(fp, _pl("%d item leaked", "%d items leaked", f_nrecs), f_nrecs);
			fprintf(fp, "\n");
			fclose(fp);
			fp = 0;
		}
	}
}
