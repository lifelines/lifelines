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
 * symtab.c -- Symbol tables (lexical scopes)
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 * Created: 2002.02.17 by Perry Rapp, out of pvalue.c
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interpi.h"
#include "liflines.h"
#include "feedback.h"
#include "vtable.h"


/*********************************************
 * local types
 *********************************************/

struct tag_symtab_iter {
	struct tag_vtable *vtable; /* generic object */
	INT refcnt; /* ref-countable object */
	TABLE_ITER tabit; /* iterator over our table of symbols */
};
/* typedef struct tag_symtab_iter *SYMTAB_ITER; */ /* in interpi.h */

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static SYMTAB create_symtab(CNSTRING title, SYMTAB parstab);
static void free_symtable_iter(SYMTAB_ITER symtabit);
static void record_dead_symtab(SYMTAB symtab);
static void record_live_symtab(SYMTAB symtab);
static void symtabit_destructor(VTABLE *obj);

/*********************************************
 * local variables
 *********************************************/

static struct tag_vtable vtable_for_symtabit = {
	VTABLE_MAGIC
	, "symtab_iter"
	, &symtabit_destructor
	, &refcountable_isref
	, &refcountable_addref
	, &refcountable_release
	, 0 /* copy_fnc */
	, &generic_get_type_name
};

LIST live_symtabs=0; /* list of symbol tables, to check for leaks */

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*======================================================
 * insert_symtab -- Update symbol table with PVALUE
 *  stab: [I/O] symbol table
 *  iden: [IN] variable in symbol table
 *  val:  [IN]  already created PVALUE
 *====================================================*/
void
insert_symtab (SYMTAB stab, STRING iden, PVALUE val)
{
	insert_table_ptr(stab->tab, iden, val);
}
/*======================================================
 * delete_symtab_element -- Delete a value from a symbol table
 *  stab: [I/O] symbol table
 *  iden: [IN]  variable in symbol table
 *====================================================*/
void
delete_symtab_element (SYMTAB stab, STRING iden)
{
	delete_table_element(stab->tab, iden);
}
/*========================================
 * remove_symtab -- Remove symbol table 
 *  @stab:  [IN] symbol table to remove
 *======================================*/
void
remove_symtab (SYMTAB stab)
{
	ASSERT(stab);

	record_dead_symtab(stab);

	destroy_table(stab->tab);

	stdfree(stab);
}
/*======================================================
 * create_symtab_proc -- Create a symbol table for a procedure
 *  returns allocated SYMTAB
 *====================================================*/
SYMTAB
create_symtab_proc (CNSTRING procname, SYMTAB parstab)
{
	char title[128];
	llstrncpyf(title, sizeof(title), uu8, "proc: %s", procname);
	return create_symtab(title, parstab);
}
/*======================================================
 * create_symtab_global -- Create a global symbol table
 *  returns allocated SYMTAB
 *====================================================*/
SYMTAB
create_symtab_global (void)
{
	return create_symtab("global", NULL);
}
/*======================================================
 * create_symtab -- Create a symbol table
 *  @title:    [IN]  title (procedure or func name)
 *  @parstab:  [IN]  (dynamic) parent symbol table
 *                    only for debugging, not for scope
 *  returns allocated SYMTAB
 *====================================================*/
static SYMTAB
create_symtab (CNSTRING title, SYMTAB parstab)
{
	SYMTAB symtab = (SYMTAB)stdalloc(sizeof(*symtab));
	memset(symtab, 0, sizeof(*symtab));

	symtab->tab = create_table_custom_vptr(delete_vptr_pvalue);
	symtab->parent = parstab;
	llstrncpyf(symtab->title, sizeof(symtab->title), uu8, title);

	record_live_symtab(symtab);

	return symtab;
}
/*======================================================
 * record_live_symtab -- Add symbol table to live list
 *====================================================*/
static void
record_live_symtab (SYMTAB symtab)
{
	if (!live_symtabs)
		live_symtabs = create_list2(LISTNOFREE);
	enqueue_list(live_symtabs, symtab);
}
/*======================================================
 * record_dead_symtab -- Remove symbol table from live list
 *====================================================*/
static void
record_dead_symtab (SYMTAB symtab)
{
	LIST newlist = create_list2(LISTNOFREE);
	ASSERT(live_symtabs);
	ASSERT(length_list(live_symtabs)>0);

	/* hard to delete an element, so create a new shorter list */
	FORLIST(live_symtabs, e)
		if (e != symtab)
			enqueue_list(newlist, e);
	ENDLIST
	destroy_list(live_symtabs);
	live_symtabs = newlist;
}
/*=================================================
 * symbol_tables_end -- interpreter just finished running report
 *===============================================*/
void
symbol_tables_end (void)
{
	/* for debugging check that no symbol tables leaked */
	INT leaked_symtabs = length_list(live_symtabs);
	leaked_symtabs = leaked_symtabs; /* remove unused warning */
	/* 2005-02-06, 2200Z, Perry: No leaks here */
}
/*======================================================
 * in_symtab -- Does symbol table have this entry ?
 *  @stab: [IN]  symbol table
 *  @key:  [IN]  key sought
 *====================================================*/
BOOLEAN
in_symtab (SYMTAB stab, STRING key)
{
	return in_table(stab->tab, key);
}
/*======================================================
 * symtab_valueofbool -- Convert pvalue to boolean if present
 *  @stab:   [IN]  symbol table
 *  @key:    [IN] key desired
 *  @there:  [OUT] whether or not key was found
 *  returns PVALUE assigned to key in symbol table, if found
 *====================================================*/
PVALUE
symtab_valueofbool (SYMTAB stab, STRING key, BOOLEAN *there)
{
	return (PVALUE)valueofbool_ptr(stab->tab, key, there);
}
/*======================================================
 * begin_symtab_iter -- Begin iterating a symbol table
 *  @stab:   [IN]  symbol table to iterate
 * returns iterator object ready to start
 *====================================================*/
SYMTAB_ITER
begin_symtab_iter (SYMTAB stab)
{
	SYMTAB_ITER symtabit = (SYMTAB_ITER)stdalloc(sizeof(*symtabit));
	memset(symtabit, 0, sizeof(*symtabit));
	symtabit->vtable = &vtable_for_symtabit;
	++symtabit->refcnt;
	symtabit->tabit = begin_table_iter(stab->tab);
	return symtabit;
}
/*======================================================
 * next_symtab_entry -- Continue iterating a symbol table
 *  @tabit: [I/O]  symbol table iterator
 *  @pkey:  [OUT]  key of next value
 *  @ppval: [OUT]  next value
 *====================================================*/
BOOLEAN
next_symtab_entry (SYMTAB_ITER symtabit, CNSTRING *pkey, PVALUE *ppval)
{
	VPTR vptr=0;
	*pkey=0;
	*ppval=0;
	if (!next_table_ptr(symtabit->tabit, pkey, &vptr))
		return FALSE;
	*ppval = vptr;
	return TRUE;
}
/*=================================================
 * end_symtab_iter -- Release reference to symbol table iterator object
 *===============================================*/
void
end_symtab_iter (SYMTAB_ITER * psymtabit)
{
	ASSERT(psymtabit);
	ASSERT(*psymtabit);
	end_table_iter(&(*psymtabit)->tabit);
	--(*psymtabit)->refcnt;
	if (!(*psymtabit)->refcnt) {
		free_symtable_iter(*psymtabit);
	}
	*psymtabit = 0;
}
/*=================================================
 * free_symtable_iter -- Delete & free symbol table iterator object
 *===============================================*/
static void
free_symtable_iter (SYMTAB_ITER symtabit)
{
	if (!symtabit) return;
	ASSERT(!symtabit->refcnt);
	memset(symtabit, 0, sizeof(*symtabit));
	stdfree(symtabit);
}
/*=================================================
 * symtabit_destructor -- destructor for symbol table iterator
 *  (destructor entry in vtable)
 *===============================================*/
static void
symtabit_destructor (VTABLE *obj)
{
	SYMTAB_ITER symtabit = (SYMTAB_ITER)obj;
	ASSERT((*obj) == &vtable_for_symtabit);
	free_symtable_iter(symtabit);
}
