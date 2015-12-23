/* 
   Copyright (c) 2003 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*=============================================================
 * lldatabase.c -- Interface to lifelines databases
 *   Created: 2003/10 by Perry Rapp
 *==============================================================*/

#include "llstdlib.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "btree.h"
#include "vtable.h"
#include "dbcontext.h"


/*********************************************
 * global/exported variables
 *********************************************/

LLDATABASE def_lldb = 0;
BTREE BTR=NULL;	/* database */

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING readpath,readpath_file;

/*********************************************
 * local types
 *********************************************/

struct tag_lldatabase {
	BTREE btree;
};

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*========================================
 * lldb_alloc -- Create lldb structure
 * This does not actually create or open a database.
 * It only creates this structure to hold the errors
 * and point to the database when it is opened (or
 * created) later.
 *======================================*/
LLDATABASE
lldb_alloc (void)
{
	LLDATABASE lldb = (LLDATABASE)stdalloc(sizeof(*lldb));
	memset(lldb, 0, sizeof(*lldb));
	return lldb;
}
/*========================================
 * lldb_set_btree -- Make this lldb point to 
 * an actual btree database
 *======================================*/
void
lldb_set_btree (LLDATABASE lldb, void *btree)
{
	ASSERT(lldb);
	ASSERT(!lldb->btree);
	lldb->btree = btree;
	BTR = btree;
}
/*========================================
 * lldb_close -- Close any database contained. 
 *  Free LLDATABASE structure.
 *  Safe to call even if not opened
 *======================================*/
void lldb_close (LLDATABASE *plldb)
{
	LLDATABASE lldb=0;
	ASSERT(plldb);
	lldb = *plldb;

	if (!lldb) return;

	if (tagtable)
		destroy_table(tagtable);
	tagtable = 0;
	/* TODO: reverse the rest of init_lifelines_postdb -- Perry, 2002.06.05 */
	if (placabbvs) {
		destroy_table(placabbvs);
		placabbvs = NULL;
	}
	free_caches();
	check_node_leaks();
	check_record_leaks();
	closexref();
	ASSERT(BTR == lldb->btree);
	if (lldb->btree) {
		closebtree(lldb->btree);
		lldb->btree = 0;
		BTR = 0;
	}
	dbnotify_close();
	transl_free_predefined_xlats(); /* clear any active legacy translation tables */
	strfree(&readpath_file);
	strfree(&readpath);
	stdfree(lldb);
	*plldb = 0;
}
