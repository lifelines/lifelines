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
static void free_symtable_iter(SYMTAB_ITER symtabit);
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
	PVALUE oldval = (PVALUE) valueof_ptr(stab->tab, iden);
	if (oldval) delete_pvalue(oldval);
	insert_table_ptr(stab->tab, iden, val);
}
/*======================================================
 * delete_symtab -- Delete a value from a symbol table
 *  @stab:  symbol table
 *  @iden: variable in symbol table
 *====================================================*/
void
delete_symtab (SYMTAB stab, STRING iden)
{
	PVALUE val = (PVALUE) valueof_ptr(stab->tab, iden);
	/* delete_table doesn't free the key or value */
	/* our key belongs to lexer, I think, but free our value */
	if (val) delete_pvalue(val);
	delete_table(stab->tab, iden);
}
/*========================================
 * remove_symtab -- Remove symbol table 
 *  @stab:  [IN] symbol table to remove
 *======================================*/
void
remove_symtab (SYMTAB stab)
{
	STRING key=0;
	VPTR ptr=0;
	TABLE_ITER tabit=0;

	ASSERT(stab);

	tabit = begin_table_iter(stab->tab);

	while (next_table_ptr(tabit, &key, &ptr))
	{
		if (ptr) {
			PVALUE val = ptr;
			ASSERT(is_pvalue(val));
			delete_pvalue(val);
			change_table_ptr(tabit, 0);
		}
	}
	end_table_iter(&tabit);
	remove_table(stab->tab, DONTFREE);
	stdfree(stab);
}
/*======================================================
 * create_symtab -- Create a symbol table
 *  returns allocated SYMTAB
 *====================================================*/
SYMTAB
create_symtab (void)
{
	SYMTAB symtab = (SYMTAB)stdalloc(sizeof(*symtab));
	memset(symtab, 0, sizeof(*symtab));

	symtab->tab = create_table_old();

	return symtab;
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
next_symtab_entry (SYMTAB_ITER symtabit, STRING *pkey, PVALUE *ppval)
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
