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

struct tag_errorinfo {
	struct tag_vtable * vtable; /* generic object table (see vtable.h) */
	INT errnum;
	CNSTRING errstr; /* heap allocated */
};
typedef struct tag_errorinfo *ERRORINFO;


struct tag_lldatabase {
	BTREE btree;
	LIST errorlist; /* LIST of ERRORINFOs */
};

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void errorinfo_destructor(VTABLE *obj);
static void init_errorinfo_vtable(ERRORINFO errorinfo);


/*********************************************
 * local variables
 *********************************************/

/* class vtable for ERRORINFO objects */
static struct tag_vtable vtable_for_errorinfo = {
	VTABLE_MAGIC
	, "errorinfo"
	, &errorinfo_destructor
	, &nonrefcountable_isref
	, 0
	, 0
	, 0 /* copy_fnc */
	, &generic_get_type_name
};

/*========================================
 * init_errorinfo_vtable -- set this errorinfo's vtable
 *======================================*/
static void
init_errorinfo_vtable (ERRORINFO errorinfo)
{
	errorinfo->vtable = &vtable_for_errorinfo;
}
/*=================================================
 * errorinfo_destructor -- destructor for zstr
 *===============================================*/
static void
errorinfo_destructor (VTABLE *obj)
{
	ERRORINFO errorinfo = (ERRORINFO)obj;
	ASSERT((*obj)->vtable_class == vtable_for_errorinfo.vtable_class);
	/* any error string was allocated with stdalloc */
	if (errorinfo->errstr) {
		stdfree((char *)(errorinfo->errstr));
		errorinfo->errstr = 0;
	}
	/* errorinfo itself was allocated with stdalloc */
	stdfree(obj);
}

/*========================================
 * lldb_adderror -- Add new error to stack for this database
 *  errstr must be heap-allocated
 *======================================*/
void
lldb_adderror (LLDATABASE lldb, int errnum, CNSTRING errstr)
{
	ERRORINFO errorinfo = 0;
	ASSERT(lldb);
	if (!lldb->errorlist) {
		lldb->errorlist = create_list();
		/* because object destructors aren't used by containers yet */
		set_list_type(lldb->errorlist, LISTNOFREE);
	}
	errorinfo = (ERRORINFO) malloc(sizeof(*errorinfo));
	memset(errorinfo, 0, sizeof(*errorinfo));
	init_errorinfo_vtable(errorinfo);
	errorinfo->errnum = errnum;
	errorinfo->errstr = errstr;
	push_list(lldb->errorlist, errorinfo);
}
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
	ASSERT(lldb && !lldb->btree);
	lldb->btree = btree;
	BTR = btree;
}
/*========================================
 * lldb_close -- Close any database contained. 
 *  Free LLDATABASE structure.
 *  Safe to call even if not opened
 *======================================*/
void lldb_close (LLDATABASE lldb)
{
	if (!lldb) return;

	if (tagtable)
		destroy_table(tagtable);
	tagtable = 0;
	/* TODO: reverse the rest of init_lifelines_db -- Perry, 2002.06.05 */
	if (placabbvs) {
		destroy_table(placabbvs);
		placabbvs = NULL;
	}
	free_caches();
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
}
