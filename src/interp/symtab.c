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
#include "interp.h"
#include "liflines.h"
#include "feedback.h"


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
	PVALUE oldval = (PVALUE) valueof_ptr(stab.tab, iden);
	if (oldval) delete_pvalue(oldval);
	insert_table_ptr(stab.tab, iden, val);
}
/*======================================================
 * delete_symtab -- Delete a value from a symbol table
 * SYMTAB stab:  symbol table
 * STRING iden: variable in symbol table
 * Created: 2001/03/17, Perry Rapp
 *====================================================*/
void
delete_symtab (SYMTAB stab, STRING iden)
{
	PVALUE val = (PVALUE) valueof_ptr(stab.tab, iden);
	if (val) delete_pvalue(val);
	delete_table(stab.tab, iden);
}
/*========================================
 * null_symtab -- Return null symbol table 
 * Created: 2001/03/24, Perry Rapp
 *======================================*/
SYMTAB
null_symtab (void)
{
	SYMTAB stab;
	stab.tab = NULL;
	return stab;
}
/*========================================
 * remove_symtab -- Remove symbol table 
 * TABLE stab:  symbol table
 * Created: 2001/03/22, Perry Rapp
 *======================================*/
void
remove_symtab (SYMTAB * stab)
{
	if (stab->tab)
	{
		STRING key=0;
		VPTR ptr=0;
		struct table_iter_s tabits;
		TABLE_ITER tabit = &tabits;
		begin_table(stab->tab, tabit);
		while (next_table_ptr(tabit, &key, &ptr)) {
			if (ptr) {
				PVALUE val = ptr;
				ASSERT(is_pvalue(val));
				delete_pvalue(val);
				change_table_ptr(tabit, 0);
			}
		}
		remove_table(stab->tab, DONTFREE);
		stab->tab = 0; /* same as *stab=null_symtab(); */
	}
}
/*======================================================
 * create_symtab -- Create a symbol table
 * Created: 2001/03/22, Perry Rapp
 *====================================================*/
void
create_symtab (SYMTAB * stab)
{
	remove_symtab(stab);
	stab->tab = create_table();
}
/*======================================================
 * in_symtab -- Does symbol table have this entry ?
 * Created: 2001/03/23, Perry Rapp
 *====================================================*/
BOOLEAN
in_symtab (SYMTAB stab, STRING key)
{
	return in_table(stab.tab, key);
}
/*======================================================
 * symtab_valueofbool -- Convert pvalue to boolean if present
 *  stab:   [IN]  symbol table
 *  key:    [IN] key desired
 *  there:  [OUT] whether or not key was found
 *  returns PVALUE assigned to key in symbol table, if found
 * Created: 2001/03/22, Perry Rapp
 *====================================================*/
PVALUE
symtab_valueofbool (SYMTAB stab, STRING key, BOOLEAN *there)
{
	return (PVALUE)valueofbool_ptr(stab.tab, key, there);
}
/*======================================================
 * begin_symtab -- Begin iterating a symbol table
 * Created: 2002/06/17, Perry Rapp
 *====================================================*/
BOOLEAN
begin_symtab (SYMTAB stab, SYMTAB_ITER stabit)
{
	memset(stabit, 0, sizeof(*stabit));
	return begin_table(stab.tab, &stabit->tabiters);
}
/*======================================================
 * next_symtab_entry -- Continue iterating a symbol table
 * Created: 2002/06/17, Perry Rapp
 *====================================================*/
BOOLEAN
next_symtab_entry (SYMTAB_ITER tabit, STRING *pkey, PVALUE *ppval)
{
	VPTR vptr=0;
	*pkey=0;
	*ppval=0;
	if (!next_table_ptr(&tabit->tabiters, pkey, &vptr))
		return FALSE;
	*ppval = vptr;
	return TRUE;
}
