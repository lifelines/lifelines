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
 * table.c -- Hash table operations
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 29 Aug 93
 *   3.0.0 - 04 Sep 94    3.0.2 - 22 Dec 94
 *   3.0.3 - 19 Sep 95
 *===========================================================*/

#include "standard.h"
#include "llstdlib.h"
#include "table.h"
#include "vtable.h"
#include "generic.h"

/*********************************************
 * local enums & defines
 *********************************************/

#define MAXHASH_DEF 512

enum TB_VALTYPE
	{
		TB_PTR /* VPTR values */
		, TB_STR /* STRING values */
		, TB_NULL /* placeholder for newly created tables */
		, TB_GENERIC /* new table holding only generics */
	};

/*********************************************
 * local types
 *********************************************/

/*
2005-01-17
 Perry is in process of converting table to use generics instead of unions
 So right now entry has both generic and union.
 The rule is that if the generic is non-null, it holds the value.
 Otherwise the value is in the union.
 Table types (TB_VALTYPE, a table-level property) are only relevant to
 unions -- generics have their own type info.
 NB: generic object pointers can only be used with objects that correctly
 implement destructors -- I'm not sure if we have any such yet; I've not
 tested them well. That is, I've not tested objects containing other
 objects, because that is not yet possible. Once a table can contain an
 object such as a list, we have object hierarchies and need destructors
 to all work correctly.
2005-02-01
 All ints are stored in generic tables, so TB_INT is gone.
*/
struct tag_entry {
	STRING ekey;
	GENERIC generic; /* holds generic null value if uval in use */
	UNION uval;
	struct tag_entry *enext;
};
typedef struct tag_entry *ENTRY;

/* table object itself */
struct tag_table {
	struct tag_vtable *vtable; /* generic object */
	INT refcnt; /* ref-countable object */
	ENTRY *entries;
	INT count; /* #entries */
	INT valtype; /* TB_VALTYPE enum in table.c */
	INT maxhash;
	INT whattofree; /* TODO: set always in constructor */
};
/* typedef struct tag_table *TABLE */ /* in table.h */

/* table iterator */
struct tag_table_iter {
	struct tag_vtable *vtable; /* generic object */
	INT refcnt; /* ref-countable object */
	INT index;
	ENTRY enext;
	TABLE table;
};
/* typedef struct tag_table_iter * TABLE_ITER; */ /* in table.h */

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static TABLE create_table_impl(INT whattofree);
static ENTRY fndentry(TABLE, CNSTRING);
static void free_contents(ENTRY ent, INT whattofree);
static void free_table_iter(TABLE_ITER tabit);
static void insert_table_impl(TABLE tab, CNSTRING key, UNION uval);
static INT hash(TABLE tab, CNSTRING key);
static BOOLEAN next_element(TABLE_ITER tabit);
static ENTRY new_entry(void);
static void replace_table_impl(TABLE tab, STRING key, UNION uval, INT whattofree);
static void tabit_destructor(VTABLE *obj);
static void table_destructor(VTABLE *obj);
static UNION* valueofbool_impl(TABLE tab, STRING key);

/*********************************************
 * local variables
 *********************************************/

static struct tag_vtable vtable_for_table = {
	VTABLE_MAGIC
	, "table"
	, &table_destructor
	, &refcountable_isref
	, &refcountable_addref
	, &refcountable_delref
	, 0 /* copy_fnc */
	, &generic_get_type_name
};
static struct tag_vtable vtable_for_tabit = {
	VTABLE_MAGIC
	, "table_iter"
	, &tabit_destructor
	, &refcountable_isref
	, &refcountable_addref
	, &refcountable_delref
	, 0 /* copy_fnc */
	, &generic_get_type_name
};

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*======================
 * hash -- Hash function
 *====================*/
static INT
hash (TABLE tab, CNSTRING key)
{
	INT hval = 0;
	while (*key)
		hval += *key++;
	hval %= tab->maxhash;
	if (hval < 0) hval += tab->maxhash;
	if (hval < 0) FATAL();
	if(hval >= tab->maxhash) FATAL();
	return hval;
}
/*================================
 * fndentry -- Find entry in table
 *==============================*/
static ENTRY
fndentry (TABLE tab, CNSTRING key)
{
	ENTRY entry;
	if (!tab || !key) return NULL;
	entry = tab->entries[hash(tab, key)];
	while (entry) {
		if (eqstr(key, entry->ekey)) return entry;
		entry = entry->enext;
	}
	return NULL;
}
/*=============================
 * create_table_impl -- Create table
 * returns addref'd table
 *===========================*/
static TABLE
create_table_impl (INT whattofree)
{
	TABLE tab = (TABLE) stdalloc(sizeof(*tab));
	INT i;

	memset(tab, 0, sizeof(*tab));
	tab->vtable = &vtable_for_table;
	tab->refcnt = 1;
	tab->maxhash = MAXHASH_DEF;
	tab->entries = (ENTRY *)stdalloc(tab->maxhash*sizeof(ENTRY));
	tab->count = 0;
	tab->valtype = TB_NULL;
	if (whattofree == -2)
		tab->valtype = TB_GENERIC;
	tab->whattofree = whattofree;
	for (i = 0; i < tab->maxhash; i++)
		tab->entries[i] = NULL;
	return tab;
}
/*=============================
 * create_table_old2 -- Create table
 * Caller specifies allocation strategy
 * Will be obsoleted by using generics
 * (in which entries themselves control their allocation)
 * returns addref'd table
 *===========================*/
TABLE
create_table_old2 (INT whattofree)
{
	return create_table_impl(whattofree);
}
/*=============================
 * create_table_old -- Create table
 * Caller will specify whether keys or values are to be freed
 * at remove_table time
 * returns addref'd table
 *===========================*/
TABLE
create_table_old (void)
{
	return create_table_impl(-1);
}
/*=============================
 * create_table -- Create table
 * Will only use new generic elements (manage their own memory)
 * All keys will be heap-allocated (to be freed by table)
 * returns addref'd table
 *===========================*/
TABLE
create_table (void)
{
	return create_table_impl(-2);
}
/*======================================
 * insert_table_impl -- Insert key & value into table
 * Caller is reponsible for both key & value memory
 *====================================*/
static void
insert_table_impl (TABLE tab, CNSTRING key, UNION uval)
{
	ENTRY entry = fndentry(tab, key);
	if (entry)
		entry->uval = uval;
	else {
		INT hval = hash(tab, key);
		entry = new_entry();
		entry->ekey = (STRING)key;
		entry->uval = uval;
		entry->enext = tab->entries[hval];
		tab->entries[hval] = entry;
		++tab->count;
	}
}
/*======================================
 * new_entry -- Make new empty ENTRY for table
 *====================================*/
static ENTRY
new_entry (void)
{
	ENTRY entry = (ENTRY) stdalloc(sizeof(*entry));
	entry->ekey = 0;
	init_generic_null(&entry->generic);
	entry->uval.w = 0;
	entry->enext = 0;
	return entry;
}
/*======================================
 * new_table_entry_impl -- Insert key & value into table
 * Caller must have checked that key is not present
 * Only used for new generic values (as is obvious from signature)
 * key is duplicated here
 *====================================*/
static void
new_table_entry_impl (TABLE tab, CNSTRING key, GENERIC * generic)
{
	INT hval = hash(tab, key);
	ENTRY entry = new_entry();
	entry->ekey = strdup(key);
	copy_generic_value(&entry->generic, generic);
	entry->uval.i = 0;
	entry->enext = tab->entries[hval];
	tab->entries[hval] = entry;
	++tab->count;
	/* used by old-style tables which use generics for int values */
}
/*======================================
 * replace_table_impl -- Insert key & value into table
 *  replacing existing key & value if key already present
 * Created: 2001/11/23, Perry Rapp
 *====================================*/
static void
replace_table_impl (TABLE tab, STRING key, UNION uval, INT whattofree)
{
	ENTRY entry = fndentry(tab, key);
	if (entry) {
		free_contents(entry, whattofree);
		entry->ekey = key;
		entry->uval = uval;
	} else {
		INT hval = hash(tab, key);
		entry = new_entry();
		entry->ekey = key;
		entry->uval = uval;
		entry->enext = tab->entries[hval];
		tab->entries[hval] = entry;
		++tab->count;
	}
}
/*======================================
 * insert_table_ptr -- Insert key & pointer value into table
 * Caller is responsible for both key & ptr memory (no dups here)
 * Created: 2001/06/03 (Perry Rapp)
 *====================================*/
void
insert_table_ptr (TABLE tab, CNSTRING key, VPTR ptr)
{
	UNION uval;
	uval.w = ptr;
	if (tab->valtype == TB_NULL)
		tab->valtype = TB_PTR;
	/* table must be homogenous, not mixed-type */
	ASSERT(tab->valtype == TB_PTR);
	insert_table_impl(tab, key, uval);
}
/*======================================
 * insert_table_int -- Insert key & INT value into table
 * Caller is responsible for key memory (no dups here)
 * Created: 2001/06/03 (Perry Rapp)
 *====================================*/
void
insert_table_int (TABLE tab, CNSTRING key, INT ival)
{
	ENTRY entry = fndentry(tab, key);
	if (entry) {
		/* update existing */
		set_generic_int(&entry->generic, ival);
	} else {
		/* insert new */
		GENERIC gen;
		init_generic_int(&gen, ival);
		new_table_entry_impl(tab, key, &gen);
		clear_generic(&gen);
	}
}
/*======================================
 * insert_table_str -- Insert key & STRING value into table
 * Caller is responsible for both key & ptr memory (no dups here)
 * Created: 2001/06/03 (Perry Rapp)
 *====================================*/
void
insert_table_str (TABLE tab, CNSTRING key, STRING str)
{
	UNION uval;
	uval.w = str;
	ASSERT(tab->whattofree != -2);
	if (tab->valtype == TB_NULL)
		tab->valtype = TB_STR;
	/* table must be homogenous, not mixed-type */
	ASSERT(tab->valtype == TB_STR);
	insert_table_impl(tab, key, uval);
}
/*======================================
 * table_insert_string -- Insert key & STRING value into table
 * Table copies (allocates) both
 *====================================*/
void
table_insert_string (TABLE tab, CNSTRING key, CNSTRING value)
{
	ENTRY entry = fndentry(tab, key);
	ASSERT(tab->whattofree == -2 && tab->valtype == TB_GENERIC);
	if (!entry) {
		/* insert new entries as generics */
		GENERIC gen;
		init_generic_string(&gen, value);
		new_table_entry_impl(tab, key, &gen);
		clear_generic(&gen);
	} else {
		/* Only new-style tables should call table_insert_string */
		ASSERT(!is_generic_null(&entry->generic));
		/* update existing value */
		set_generic_string(&entry->generic, value);
	}
}
/*======================================
 * table_insert_ptr -- Insert key & vptr (void ptr) value into table
 * Table copies (allocates) the key
 *====================================*/
void
table_insert_ptr (TABLE tab, CNSTRING key, const VPTR value)
{
	ENTRY entry = fndentry(tab, key);
	ASSERT(tab->whattofree == -2 && tab->valtype == TB_GENERIC);
	if (!entry) {
		/* insert new entries as generics */
		GENERIC gen;
		init_generic_vptr(&gen, value);
		new_table_entry_impl(tab, key, &gen);
		clear_generic(&gen);
	} else {
		/* Only new-style tables should call table_insert_string */
		ASSERT(!is_generic_null(&entry->generic));
		/* update existing value */
		set_generic_vptr(&entry->generic, value);
	}
}
/*======================================
 * replace_table_str -- Insert or replace
 *  key & value
 * Created: 2001/11/23, Perry Rapp
 *====================================*/
void
replace_table_str (TABLE tab, STRING key, STRING str, INT whattofree)
{
	UNION uval;
	uval.w = str;
	if (tab->valtype == TB_NULL)
		tab->valtype = TB_STR;
	/* table must be homogenous, not mixed-type */
	ASSERT(tab->valtype == TB_STR);
	replace_table_impl(tab, key, uval, whattofree);
}
/*==========================================
 * delete_table -- Remove element from table
 *  tab: [I/O]  table from which to remove element
 *  key: [IN]   key to find element to remove
 *========================================*/
void
delete_table (TABLE tab, CNSTRING key)
{
	INT hval = hash(tab, key);
	ENTRY preve = NULL;
	ENTRY thise = tab->entries[hval];
	while (thise && nestr(key, thise->ekey)) {
		preve = thise;
		thise = thise->enext;
	}
	if (!thise) return;
	if (preve)
		preve->enext = thise->enext;
	else
		tab->entries[hval] = thise->enext;
	stdfree(thise);
	--tab->count;
}
/*======================================
 * in_table() - Check for entry in table
 *====================================*/
BOOLEAN
in_table (TABLE tab, CNSTRING key)
{
	return fndentry(tab, key) != NULL;
}
/*===============================
 * valueof_ptr -- Find pointer value of entry
 * Created: 2001/06/03 (Perry Rapp)
 *=============================*/
VPTR
valueof_ptr (TABLE tab, CNSTRING key)
{
	ENTRY entry;
	if (!tab->count || !key) return NULL;
	ASSERT(tab->valtype == TB_PTR);
	if ((entry = fndentry(tab, key)))
		return entry->uval.w;
	else
		return NULL;
}
/*===============================
 * valueof_int -- Find int value of entry
 * return defval if missing
 * Created: 2001/06/03 (Perry Rapp)
 *=============================*/
INT
valueof_int (TABLE tab, CNSTRING key, INT defval)
{
	ENTRY entry=0;
	if (!tab->count || !key) 
		return defval;
	entry = fndentry(tab, key);
	if (!entry)
		return defval;
	if (!is_generic_int(&entry->generic))
		return defval;
	return get_generic_int(&entry->generic);
}
/*===============================
 * valueof_str -- Find string value of entry
 * Created: 2001/06/03 (Perry Rapp)
 *=============================*/
STRING
valueof_str (TABLE tab, CNSTRING key)
{
	ENTRY entry=0;
	if (!tab->count || !key) return NULL;
	entry = fndentry(tab, key);
	if (!entry)
		return NULL;
	if (!is_generic_null(&entry->generic)) {
		if (!is_generic_string(&entry->generic))
			return NULL;
		return get_generic_string(&entry->generic);
	} else {
		ASSERT(tab->valtype == TB_STR);
		return entry->uval.w;
	}
}
/*===================================
 * valueofbool_impl -- Find value of entry
 *=================================*/
static UNION *
valueofbool_impl (TABLE tab, STRING key)
{
	ENTRY entry;
	if (!tab->count || !key) return NULL;
	if ((entry = fndentry(tab, key))) {
		return &entry->uval;
	}
	return NULL;
}
/*===================================
 * valueofbool_ptr -- Find pointer value of entry
 * BOOLEAN *there:   [OUT] FALSE if not found
 * Created: 2001/06/03 (Perry Rapp)
 *=================================*/
VPTR
valueofbool_ptr (TABLE tab, STRING key, BOOLEAN *there)
{
	UNION * val = valueofbool_impl(tab, key);
	if (val) {
		ASSERT(tab->valtype == TB_PTR);
		*there = TRUE;
		return val->w;
	} else {
		*there = FALSE;
		return NULL;
	}
}
/*===================================
 * valueofbool_int -- Find pointer value of entry
 * BOOLEAN *there:   [OUT] FALSE if not found
 * Created: 2001/06/03 (Perry Rapp)
 *=================================*/
INT
valueofbool_int (TABLE tab, STRING key, BOOLEAN *there)
{
	ENTRY entry = fndentry(tab, key);
	INT defval=0;
	if (!entry) {
		if (there)
			*there = FALSE;
		return defval;
	}
	if (there)
		*there = TRUE;
	if (!is_generic_int(&entry->generic))
		return defval;
	return get_generic_int(&entry->generic);
}
/*===================================
 * valueofbool_str -- Find string value of entry
 * BOOLEAN *there:   [OUT] FALSE if not found
 * Created: 2001/06/03 (Perry Rapp)
 *=================================*/
STRING
valueofbool_str (TABLE tab, STRING key, BOOLEAN *there)
{
	UNION * val = valueofbool_impl(tab, key);
	if (val) {
		ASSERT(tab->valtype == TB_STR);
		*there = TRUE;
		return val->w;
	} else {
		*there = FALSE;
		return NULL;
	}
}
/*=============================
 * remove_table -- Remove table
 * TABLE tab
 * INT whattofree:  FREEKEY, DONTFREE, etc
 *===========================*/
void
remove_table (TABLE tab, INT whattofree)
{
	INT i;
	ENTRY ent, nxt;
	if (!tab) return;
	if (tab->whattofree!=-1) { ASSERT(whattofree==tab->whattofree); }
	for (i = 0; i < tab->maxhash; i++) {
		nxt = tab->entries[i];
		while ((ent = nxt)) {
			nxt = ent->enext;
			free_contents(ent, whattofree);
			stdfree(ent);
		}
	}
	stdfree(tab->entries);
	stdfree(tab);
}
/*=================================================
 * free_contents -- Free key and/or value as appropriate
 *===============================================*/
static void
free_contents (ENTRY ent, INT whattofree)
{
	if (whattofree==FREEBOTH || whattofree==FREEKEY || whattofree==-2)
		stdfree(ent->ekey);
	if (!is_generic_null(&ent->generic)) {
		clear_generic(&ent->generic);
	} else {
		if (whattofree==FREEBOTH || whattofree==FREEVALUE) {
			if (ent->uval.w)
				stdfree(ent->uval.w);
		}
	}
}
/*=================================================
 * traverse_table -- Traverse table doing something
 * tproc: callback for each entry
 * callback is passed a pointer into memory owned by table
 * so callback must not free data received
 *===============================================*/
void
traverse_table (TABLE tab, void (*tproc)(CNSTRING key, UNION uval))
{
	INT i;
	ENTRY ent, nxt;
	if (!tab || !tproc) return;
	for (i = 0; i < tab->maxhash; i++) {
		nxt = tab->entries[i];
		while ((ent = nxt)) {
			nxt = ent->enext;
			(*tproc)(ent->ekey, ent->uval);
		}
	}
}
/*=================================================
 * traverse_table_param -- Traverse table doing something, with extra callback param
 * also, use return value of proc to allow abort (proc returns 0 for abort)
 * callback is passed a pointer into memory owned by table
 * so callback must not free data received
 * Created: 2001/01/01, Perry Rapp
 *===============================================*/
void
traverse_table_param (TABLE tab,
                INT (*tproc)(CNSTRING key, UNION uval, GENERIC * pgeneric, VPTR param),
                VPTR param)
{
	INT i;
	ENTRY ent, nxt;
	if (!tab || !tproc) return;
	for (i = 0; i < tab->maxhash; i++) {
		nxt = tab->entries[i];
		while ((ent = nxt)) {
			nxt = ent->enext;
			if (!(*tproc)(ent->ekey, ent->uval, &ent->generic, param))
				return;
		}
	}
}
/*=================================================
 * begin_table_iter -- Begin iteration of table
 *  returns addref'd iterator object
 *===============================================*/
TABLE_ITER
begin_table_iter (TABLE tab)
{
	TABLE_ITER tabit = (TABLE_ITER)stdalloc(sizeof(*tabit));
	memset(tabit, 0, sizeof(*tabit));
	tabit->table = tab;
	++tabit->refcnt;
	return tabit;
}
/*=================================================
 * next_element -- Find next element in table (iterating)
 * This doesn't know or need to know about element types or generics
 *===============================================*/
static BOOLEAN
next_element (TABLE_ITER tabit)
{
	ASSERT(tabit);
	if (!tabit->table) return FALSE;
	if (tabit->index == -1 || tabit->table->count == 0)
		return FALSE;
	if (tabit->enext) {
		tabit->enext = tabit->enext->enext;
		if (tabit->enext)
			return TRUE;
		++tabit->index;
	}
	for ( ; tabit->index < tabit->table->maxhash; ++tabit->index) {
			tabit->enext = tabit->table->entries[tabit->index];
			if (tabit->enext)
				return TRUE;
	}
	tabit->index = -1;
	tabit->enext = 0;
	return FALSE;
}
/*=================================================
 * next_table_ptr -- Advance to next pointer in table
 * skips over any other types of table elements
 * returns FALSE if runs out of table elements
 *===============================================*/
BOOLEAN
next_table_ptr (TABLE_ITER tabit, STRING *pkey, VPTR *pptr)
{
advance:
	if (!next_element(tabit)) {
		*pkey = 0;
		*pptr = 0;
		return FALSE;
	}
	if (!is_generic_null(&tabit->enext->generic)) {
		if (is_generic_vptr(&tabit->enext->generic)) {
			*pkey = tabit->enext->ekey;
			*pptr = get_generic_vptr(&tabit->enext->generic);
			return TRUE;
		} else {
			/* wrong type of element, skip it */
			goto advance;
		}
	} else {
		if (tabit->table->valtype == TB_PTR) {
			*pkey = tabit->enext->ekey;
			*pptr = tabit->enext->uval.w;
			return TRUE;
		} else {
			/* wrong type of table, only generic elements might be ok */
			/* so skip this element */
			goto advance;
		}
	}
	return TRUE;
}
/*=================================================
 * change_table_ptr -- User changing value in iteration
 * Created: 2002/06/17, Perry Rapp
 *===============================================*/
BOOLEAN
change_table_ptr (TABLE_ITER tabit, VPTR newptr)
{
	if (!tabit || !tabit->enext)
		return FALSE;
	tabit->enext->uval.w = newptr;
	return TRUE;
}
/*=================================================
 * end_table_iter -- Release reference to table iterator object
 *===============================================*/
void
end_table_iter (TABLE_ITER * ptabit)
{
	ASSERT(ptabit);
	ASSERT(*ptabit);
	--(*ptabit)->refcnt;
	if (!(*ptabit)->refcnt) {
		free_table_iter(*ptabit);
	}
	*ptabit = 0;
}
/*=================================================
 * free_table_iter -- Delete & free table iterator object
 *===============================================*/
static void
free_table_iter (TABLE_ITER tabit)
{
	if (!tabit) return;
	ASSERT(!tabit->refcnt);
	memset(tabit, 0, sizeof(*tabit));
	stdfree(tabit);
}
/*=================================================
 * get_table_count -- Return #elements
 * Created: 2002/02/17, Perry Rapp
 *===============================================*/
INT
get_table_count (TABLE tab)
{
	if (!tab) return 0;
	return tab->count;
}
/*=================================================
 * copy_table -- Copy all elements from src to dest
 * Created: 2002/02/17, Perry Rapp
 *===============================================*/
void
copy_table (const TABLE src, TABLE dest, INT whattodup)
{
	INT i;
	ENTRY ent, nxt;
	BOOLEAN dupkey = whattodup&FREEBOTH || whattodup&FREEKEY;
	BOOLEAN dupval = whattodup&FREEBOTH || whattodup&FREEVALUE;

	if (dest->whattofree == -2)
		dupkey = FALSE;

	ASSERT(get_table_count(dest)==0);
	if (get_table_count(src)==0)
		return;
	ASSERT(!dupval || src->valtype==TB_STR); /* can only dup strings */

	dest->valtype = src->valtype;
	if (src->maxhash == dest->maxhash)
		dest->count = src->count;
	
	for (i = 0; i < src->maxhash; i++) {
		nxt = src->entries[i];
		while ((ent = nxt)) {
			UNION uval = ent->uval;
			STRING key = ent->ekey;
			if (dupkey) key = strsave(key);
			if (dest->whattofree != -2 && is_generic_null(&ent->generic)) {
				if (dupval) uval.w = strsave(uval.w);
			}
			if (src->maxhash == dest->maxhash) {
				/* copy src item directly into dest (skip hashing) */
				ENTRY entry = new_entry();
				entry->ekey = key;
				copy_generic_value(&entry->generic, &ent->generic);
				entry->uval = uval;
				entry->enext = dest->entries[i];
				dest->entries[i] = entry;
			} else {
				/* insert src item into dest */
				if (is_generic_null(&ent->generic)) {
					if (dest->whattofree == -2) {
						/* copying from old-style value to generics table */
						switch(src->valtype) {
						case TB_PTR: table_insert_ptr(dest, key, uval.w); break;
						case TB_STR: table_insert_string(dest, key, uval.s); break;
						default: new_table_entry_impl(dest, key, &ent->generic); break;
						}
					} else {
						/* old style direct copy */
						insert_table_impl(dest, key, uval);
					}
				} else {
					new_table_entry_impl(dest, key, &ent->generic);
				}
			}
			nxt = ent->enext;
		}
	}
}
/*=================================================
 * destroy_table -- public destructor for a table
 *===============================================*/
void
destroy_table (TABLE tab)
{
	if (!tab) return;
	/* to use this (and I'd prefer all code to use this)
	the whattofree must have been set, so I plan to revise
	all code to set it at creation time */
	ASSERT(tab->whattofree != -1);
	/* tab->whattofree == -2 for new generic tables */
	remove_table(tab, tab->whattofree);
}
/*=================================================
 * table_destructor -- destructor for table
 *  (destructor entry in vtable)
 *===============================================*/
static void
table_destructor (VTABLE *obj)
{
	TABLE tab = (TABLE)obj;
	ASSERT((*obj) == &vtable_for_table);
	destroy_table(tab);
}
/*=================================================
 * tabit_destructor -- destructor for table iterator
 *  (vtable destructor)
 *===============================================*/
static void
tabit_destructor (VTABLE *obj)
{
	TABLE_ITER tabit = (TABLE_ITER)obj;
	ASSERT((*obj) == &vtable_for_tabit);
	free_table_iter(tabit);
}
/*=================================================
 * addref_table -- increment reference count of table
 *===============================================*/
void
addref_table (TABLE tab)
{
	++tab->refcnt;
}
/*=================================================
 * delref_table -- decrement reference count of table
 *  and free if appropriate (ref count hits zero)
 *===============================================*/
void
delref_table (TABLE tab, void (*tproc)(CNSTRING key, UNION uval))
{
	--tab->refcnt;
	if (!tab->refcnt) {
		traverse_table(tab, tproc);
		destroy_table(tab);
	}
}
