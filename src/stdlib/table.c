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

/*********************************************
 * local enums & defines
 *********************************************/

#define MAXHASH_DEF 512

enum TB_VALTYPE
	{
		TB_PTR /* VPTR values */
		, TB_INT /* INT values */
		, TB_STR /* STRING values */
		, TB_NULL /* placeholder for newly created tables */
	};
/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static UNION* access_value_impl (TABLE tab, STRING key);
static ENTRY fndentry(TABLE, CNSTRING);
static void free_contents(ENTRY ent, INT whattofree);
static void insert_table_impl(TABLE tab, STRING key, UNION uval);
static INT hash(TABLE tab, CNSTRING key);
static BOOLEAN next_element(TABLE_ITER tabit);
static void replace_table_impl(TABLE tab, STRING key, UNION uval, INT whattofree);
static UNION* valueofbool_impl(TABLE tab, STRING key);

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
 * create_table -- Create table
 *===========================*/
TABLE
create_table (void)
{
	TABLE tab = (TABLE) stdalloc(sizeof(*tab));
	INT i;
	memset(tab, 0, sizeof(*tab));
	tab->maxhash = MAXHASH_DEF;
	tab->entries = (ENTRY *)stdalloc(tab->maxhash*sizeof(ENTRY));
	tab->count = 0;
	tab->refcnt = 1;
	tab->valtype = TB_NULL;
	for (i = 0; i < tab->maxhash; i++)
		tab->entries[i] = NULL;
	return tab;
}
/*======================================
 * insert_table_impl -- Insert key & value into table
 * Caller is reponsible for both key & value memory
 *====================================*/
static void
insert_table_impl (TABLE tab, STRING key, UNION uval)
{
	ENTRY entry = fndentry(tab, key);
	if (entry)
		entry->uval = uval;
	else {
		INT hval = hash(tab, key);
		entry = (ENTRY) stdalloc(sizeof(*entry));
		entry->ekey = key;
		entry->uval = uval;
		entry->enext = tab->entries[hval];
		tab->entries[hval] = entry;
		++tab->count;
	}
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
		entry = (ENTRY) stdalloc(sizeof(*entry));
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
insert_table_ptr (TABLE tab, STRING key, VPTR ptr)
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
insert_table_int (TABLE tab, STRING key, INT ival)
{
	UNION uval;
	uval.i = ival;
	if (tab->valtype == TB_NULL)
		tab->valtype = TB_INT;
	/* table must be homogenous, not mixed-type */
	ASSERT(tab->valtype == TB_INT);
	insert_table_impl(tab, key, uval);
}
/*======================================
 * insert_table_str -- Insert key & STRING value into table
 * Caller is responsible for both key & ptr memory (no dups here)
 * Created: 2001/06/03 (Perry Rapp)
 *====================================*/
void
insert_table_str (TABLE tab, STRING key, STRING str)
{
	UNION uval;
	uval.w = str;
	if (tab->valtype == TB_NULL)
		tab->valtype = TB_STR;
	/* table must be homogenous, not mixed-type */
	ASSERT(tab->valtype == TB_STR);
	insert_table_impl(tab, key, uval);
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
delete_table (TABLE tab, STRING key)
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
in_table (TABLE tab, STRING key)
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
	ENTRY entry;
	if (!tab->count || !key) return defval;
	ASSERT(tab->valtype == TB_INT);
	if ((entry = fndentry(tab, key)))
		return entry->uval.i;
	else
		return defval;
}
/*===============================
 * valueof_str -- Find string value of entry
 * Created: 2001/06/03 (Perry Rapp)
 *=============================*/
STRING
valueof_str (TABLE tab, CNSTRING key)
{
	ENTRY entry;
	if (!tab->count || !key) return NULL;
	ASSERT(tab->valtype == TB_STR);
	if ((entry = fndentry(tab, key)))
		return entry->uval.w;
	else
		return NULL;
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
	UNION * val = valueofbool_impl(tab, key);
	if (val) {
		ASSERT(tab->valtype == TB_INT);
		*there = TRUE;
		return val->i;
	} else {
		*there = FALSE;
		return 0;
	}
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
/*===================================
 * access_value_impl -- get pointer to value
 * returns NULL if not found
 * Created: 2001/01/01, Perry Rapp
 *=================================*/
static UNION *
access_value_impl (TABLE tab, STRING key)
{
	ENTRY entry;
	if (!tab->count || !key) return NULL;
	if (NULL == (entry = fndentry(tab, key)))
		return 0;
	return &entry->uval;
}
/*===================================
 * access_value_ptr -- get pointer to VPTR value
 * returns NULL if not found
 * Created: 2001/06/03 (Perry Rapp)
 *=================================*/
VPTR *
access_value_ptr (TABLE tab, STRING key)
{
	UNION * val = access_value_impl(tab, key);
	if (!val) return NULL;
	ASSERT(tab->valtype == TB_PTR);
	return &val->w;
}
/*===================================
 * access_value_int -- get pointer to int value
 * returns NULL if not found
 * Created: 2001/06/03 (Perry Rapp)
 *=================================*/
INT *
access_value_int (TABLE tab, STRING key)
{
	UNION * val = access_value_impl(tab, key);
	if (!val) return NULL;
	ASSERT(tab->valtype == TB_INT);
	return &val->i;
}
/*
Can't implement access_value_str 
because void ** is not compatible with STRING *
but client can use valueofbool_str to get old value,
then insert_table_str to replace with new value
*/
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
	if (tab->valtype == TB_INT) {
		/* can't free INTs */
		ASSERT(whattofree != FREEBOTH && whattofree != FREEVALUE);
	}
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
	if (whattofree==FREEBOTH || whattofree==FREEKEY)
		stdfree(ent->ekey);
	if (whattofree==FREEBOTH || whattofree==FREEVALUE) {
		if (ent->uval.w)
			stdfree(ent->uval.w);
	}
}
/*=================================================
 * traverse_table -- Traverse table doing something
 * tproc: callback for each entry
 *===============================================*/
void
traverse_table (TABLE tab, void (*tproc)(ENTRY))
{
	INT i;
	ENTRY ent, nxt;
	if (!tab || !tproc) return;
	for (i = 0; i < tab->maxhash; i++) {
		nxt = tab->entries[i];
		while ((ent = nxt)) {
			nxt = ent->enext;
			(*tproc)(ent);
		}
	}
}
/*=================================================
 * traverse_table_param -- Traverse table doing something, with extra callback param
 * also, use return value of proc to allow abort (proc returns 0 for abort)
 * Created: 2001/01/01, Perry Rapp
 *===============================================*/
void
traverse_table_param (TABLE tab,
                INT (*tproc)(ENTRY, VPTR),
                VPTR param)
{
	INT i;
	ENTRY ent, nxt;
	if (!tab || !tproc) return;
	for (i = 0; i < tab->maxhash; i++) {
		nxt = tab->entries[i];
		while ((ent = nxt)) {
			nxt = ent->enext;
			if (!(*tproc)(ent, param))
				return;
		}
	}
}
/*=================================================
 * begin_table -- Begin iteration of table
 * Created: 2002/06/16, Perry Rapp
 *===============================================*/
BOOLEAN
begin_table (TABLE tab, TABLE_ITER tabit)
{
	memset(tabit, 0, sizeof(*tabit));
	tabit->table = tab;
	tabit->enext = 0;
	tabit->index = 0;
	return tabit->table->count>0;
}
/*=================================================
 * next_element -- Find next element in table (iterating)
 * Created: 2002/06/16, Perry Rapp
 *===============================================*/
static BOOLEAN
next_element (TABLE_ITER tabit)
{
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
 * next_table_str -- Iterating table with strings
 * Created: 2002/06/16, Perry Rapp
 *===============================================*/
BOOLEAN
next_table_str (TABLE_ITER tabit, STRING *pkey, STRING *pstr)
{
	ASSERT(tabit->table->valtype == TB_STR || tabit->table->valtype == TB_NULL);
	if (!next_element(tabit)) {
		*pkey = 0;
		*pstr = 0;
		return FALSE;
	}
	*pkey = tabit->enext->ekey;
	*pstr = tabit->enext->uval.w;
	return TRUE;
}
/*=================================================
 * next_table_ptr -- Iterating table with pointers
 * Created: 2002/06/16, Perry Rapp
 *===============================================*/
BOOLEAN
next_table_ptr (TABLE_ITER tabit, STRING *pkey, VPTR *pptr)
{
	ASSERT(tabit->table->valtype == TB_PTR || tabit->table->valtype == TB_NULL);
	if (!next_element(tabit)) {
		*pkey = 0;
		*pptr = 0;
		return FALSE;
	}
	*pkey = tabit->enext->ekey;
	*pptr = tabit->enext->uval.w;
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
			if (dupval) uval.w = strsave(uval.w);
			if (src->maxhash == dest->maxhash) {
				/* copy src item directly into dest (skip hashing) */
				ENTRY entry = (ENTRY) stdalloc(sizeof(*entry));
				entry->ekey = key;
				entry->uval = uval;
				entry->enext = dest->entries[i];
				dest->entries[i] = entry;
			} else {
				/* insert src item into dest */
				insert_table_impl(dest, key, uval);
			}
			nxt = ent->enext;
		}
	}
}
