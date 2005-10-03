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
 * table.c -- Provides a table container object
 *  This provides type-safe tables
 *  These tables are reference counted objects
 *  This uses either hashtbl or rbtree for storage
 *==============================================================*/

#include "llstdlib.h"
#include "vtable.h"
#include "table.h"
#include "object.h"
#include "hashtab.h"
#include "rbtree.h"
#include "lloptions.h"

/*********************************************
 * local enums & defines
 *********************************************/

enum TB_VALTYPE
	{
		TB_NULL /* (not used) */
		, TB_INT /* table of integers */
		, TB_STR /* table of strings */
		, TB_HPTR /* table of heap pointers (table frees them) */
		, TB_VPTR /* table of void pointers (table does not free) */
		, TB_OBJ /* table of objects */
	};

/*********************************************
 * local types
 *********************************************/

/* table object itself */
struct tag_table {
	struct tag_vtable *vtable; /* generic object */
	INT refcnt; /* ref-countable object */
	enum TB_VALTYPE valtype;
	DELFUNC destroyfunc; /* how to destroy elements */
	HASHTAB hashtab;
	RBTREE rbtree;
};
/* typedef struct tag_table *TABLE */ /* in table.h */

/* table iterator */
struct tag_table_iter {
	struct tag_vtable *vtable; /* generic object */
	INT refcnt; /* ref-countable object */
	HASHTAB_ITER hashtab_iter;
	RBITER rbit;
};
/* typedef struct tag_table_iter * TABLE_ITER; */ /* in table.h */

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static TABLE create_table_impl(enum TB_VALTYPE valtype, DELFUNC delfunc);
static void free_table_iter(TABLE_ITER tabit);
static VPTR get_rb_value(RBTREE rbtree, CNSTRING key);
static void * llalloc(size_t size);
static void llassert(int assertion, const char* error);
static int rbcompare(RBKEY key1, RBKEY key2);
static void rbdestroy(void * param, RBKEY key, RBVALUE info);
static void rbdestroy_value(TABLE tab, RBVALUE info);
static VPTR rb_valueof(RBTREE rbtree, CNSTRING key, BOOLEAN *there);
static void tabit_destructor(VTABLE *obj);
static void table_destructor(VTABLE *obj);
static void table_element_destructor(void * el);
static void table_element_obj_destructor(void *el);

/*********************************************
 * local variables
 *********************************************/

static struct tag_vtable vtable_for_table = {
	VTABLE_MAGIC
	, "table"
	, &table_destructor
	, &refcountable_isref
	, &refcountable_addref
	, &refcountable_release
	, 0 /* copy_fnc */
	, &generic_get_type_name
};
static struct tag_vtable vtable_for_tabit = {
	VTABLE_MAGIC
	, "table_iter"
	, &tabit_destructor
	, &refcountable_isref
	, &refcountable_addref
	, &refcountable_release
	, 0 /* copy_fnc */
	, &generic_get_type_name
};

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=============================
 * init_table_module -- Module initialization
 *===========================*/
void
init_table_module (void)
{
	/* Red/Black tree and stack modules need external assert & alloc handlers */
	RbInitModule(&llassert, &llalloc);
}
/*=============================
 * create_table_impl -- Create table
 * All tables are created in this function
 * returns addref'd table
 *===========================*/
static TABLE
create_table_impl (enum TB_VALTYPE valtype, DELFUNC delfunc)
{
	TABLE tab = (TABLE) stdalloc(sizeof(*tab));

	tab->vtable = &vtable_for_table;
	tab->refcnt = 1;
	tab->valtype = valtype;
	if (getlloptstr("rbtree", 0))
		tab->rbtree = RbTreeCreate(tab, rbcompare, rbdestroy);
	else
		tab->hashtab = create_hashtab();
	tab->destroyfunc = delfunc;
	return tab;
}
/*=============================
 * create_table_int -- Create table holding integers
 * returns addref'd table
 *===========================*/
TABLE
create_table_int (void)
{
	return create_table_impl(TB_INT, table_element_destructor);
}
/*=============================
 * create_table_str -- Create table holding heap strings
 * (table dup & frees strings)
 * returns addref'd table
 *===========================*/
TABLE
create_table_str (void)
{
	return create_table_impl(TB_STR, table_element_destructor);
}
/*=============================
 * create_table_hptr -- Create table holding heap pointers
 * (ie, table will free elements)
 * returns addref'd table
 *===========================*/
TABLE
create_table_hptr (void)
{
	return create_table_impl(TB_HPTR, table_element_destructor);
}
/*=============================
 * create_table_vptr -- Create table holding shared pointers
 * (ie, table will not free elements)
 * returns addref'd table
 *===========================*/
TABLE
create_table_vptr (void)
{
	return create_table_impl(TB_VPTR, NULL);
}
/*=============================
 * create_table_custom_vptr -- Create table holding pointers
 * (table calls custom function to destroy elements)
 * returns addref'd table
 *===========================*/
TABLE
create_table_custom_vptr (void (*destroyel)(void *ptr))
{
	return create_table_impl(TB_VPTR, destroyel);
}
/*=============================
 * create_table_obj -- Create table holding objects
 * (ie, table will release them using release_object)
 * returns addref'd table
 *===========================*/
TABLE
create_table_obj (void)
{
	return create_table_impl(TB_OBJ, table_element_obj_destructor);
}
/*=================================================
 * destroy_table -- destroy all element & memory for table
 *===============================================*/
void
destroy_table (TABLE tab)
{
	if (!tab) return;
	ASSERT(tab->vtable == &vtable_for_table);

	/* should not be called for a shared table */
	ASSERT(tab->refcnt==1 || tab->refcnt==0);

	if (tab->rbtree)
		RbTreeDestroy(tab->rbtree);
	else
		destroy_hashtab(tab->hashtab, tab->destroyfunc);
	memset(tab, 0, sizeof(*tab));
	stdfree(tab);
}
/*=================================================
 * table_destructor -- destructor for table
 *  (destructor entry in vtable)
 *===============================================*/
static void
table_destructor (VTABLE *obj)
{
	TABLE tab = (TABLE)obj;
	destroy_table(tab);
}
/*=================================================
 * table_element_destructor -- element destructor function
 *  used for all pointer tables except TB_VPTR
 *===============================================*/
static void
table_element_destructor (void * el)
{
	stdfree(el);
}
/*=================================================
 * table_element_obj_destructor -- object element destructor function
 *  used for object tables (TB_OBJ)
 *===============================================*/
static void
table_element_obj_destructor (void *el)
{
	if (el) {
		release_object(el);
	}
}
/*=================================================
 * addref_table -- increment reference count of table
 *===============================================*/
void
addref_table (TABLE tab)
{
	ASSERT(tab->vtable == &vtable_for_table);
	++tab->refcnt;
}
/*=================================================
 * release_table -- decrement reference count of table
 *  and free if appropriate (ref count hits zero)
 *===============================================*/
void
release_table (TABLE tab)
{
	if (!tab) return;
	ASSERT(tab->vtable == &vtable_for_table);
	--tab->refcnt;
	if (!tab->refcnt) {
		destroy_table(tab);
	}
}
/*======================================
 * insert_table_int -- Insert key & INT value into table
 *====================================*/
void
insert_table_int (TABLE tab, CNSTRING key, INT ival)
{
	INT * newval = stdalloc(sizeof(*newval));

	*newval = ival;

	ASSERT(tab);
	ASSERT(tab->valtype == TB_INT);

	insert_table_ptr(tab, key, newval);
}
/*======================================
 * insert_table_str -- Insert key & string value into table
 *====================================*/
void
insert_table_str (TABLE tab, CNSTRING key, CNSTRING value)
{
	insert_table_ptr(tab, key, strsave(value));
}
/*======================================
 * insert_table_ptr -- Insert key & pointer value into table
 *====================================*/
void
insert_table_ptr (TABLE tab, CNSTRING key, VPTR ptr)
{
	INT * oldval = 0;

	ASSERT(tab);
	ASSERT(tab->vtable == &vtable_for_table);

	if (tab->rbtree) {
		/* first see if key already exists, so we can just change value */
		RBNODE node = RbExactQuery(tab->rbtree, key);
		if (node) {
			VPTR oldptr = RbGetInfo(node);
			if (oldptr != ptr) {
				RbSetInfo(node, ptr);
				rbdestroy_value(tab, oldptr);
			}
			return;
		}
		RbTreeInsert(tab->rbtree, strsave(key), ptr);
	} else {
		oldval = insert_hashtab(tab->hashtab, key, ptr);
		if (oldval && tab->destroyfunc) {
			(*tab->destroyfunc)(oldval);
		}
	}
}
/*======================================
 * insert_table_obj -- Insert key & object into table
 * table addrefs the object
 *====================================*/
void
insert_table_obj (TABLE tab, CNSTRING key, VPTR obj)
{
	if (obj)
		addref_object(obj);
	insert_table_ptr(tab, key, obj);
}
/*======================================
 * replace_table_str -- Insert or replace
 *  key & value
 * Created: 2001/11/23, Perry Rapp
 *====================================*/
void
replace_table_str (TABLE tab, CNSTRING key, CNSTRING str)
{
	/* insert function handles deleting old value */
	insert_table_str(tab, key, str);
}
/*==========================================
 * delete_table_element -- Delete element from table
 *  tab: [I/O]  table from which to remove element
 *  key: [IN]   key to find element to remove
 * Destroys key and value as appropriate
 *========================================*/
void
delete_table_element (TABLE tab, CNSTRING key)
{
	void * oldval=0;

	ASSERT(tab);
	ASSERT(tab->vtable == &vtable_for_table);

	if (tab->rbtree) {
		RBNODE old = RbExactQuery(tab->rbtree, key);
		if (old)
			RbDeleteNode(tab->rbtree, old);
	} else {
		oldval = remove_hashtab(tab->hashtab, key);
		if (oldval && tab->destroyfunc) {
			(*tab->destroyfunc)(oldval);
		}
	}
}
/*======================================
 * in_table() - Check for entry in table
 *====================================*/
BOOLEAN
in_table (TABLE tab, CNSTRING key)
{
	ASSERT(tab);
	ASSERT(tab->vtable == &vtable_for_table);

	if (tab->rbtree) {
		return RbExactQuery(tab->rbtree, key) != 0;
	} else {
		return in_hashtab(tab->hashtab, key);
	}
}
/*===============================
 * valueof_int -- Find int value of entry
 * return 0 if missing
 * Created: 2001/06/03 (Perry Rapp)
 *=============================*/
INT
valueof_int (TABLE tab, CNSTRING key)
{
	INT * val=0;

	ASSERT(tab);
	ASSERT(tab->vtable == &vtable_for_table);
	ASSERT(tab->valtype == TB_INT);

	if (tab->rbtree) {
		val = get_rb_value(tab->rbtree, key);
	} else {
		val = find_hashtab(tab->hashtab, key, NULL);
	}
	return (val ? *val : 0);
}
/*===============================
 * valueof_str -- Find string value of entry
 *=============================*/
STRING
valueof_str (TABLE tab, CNSTRING key)
{
	ASSERT(tab);
	ASSERT(tab->vtable == &vtable_for_table);
	ASSERT(tab->valtype == TB_STR);

	if (tab->rbtree) {
		return (STRING)get_rb_value(tab->rbtree, key);
	} else {
		return (STRING)find_hashtab(tab->hashtab, key, NULL);
	}


}
/*===============================
 * valueof_ptr -- Find pointer value of entry
 *=============================*/
VPTR
valueof_ptr (TABLE tab, CNSTRING key)
{
	ASSERT(tab);
	ASSERT(tab->vtable == &vtable_for_table);
	ASSERT(tab->valtype == TB_HPTR || tab->valtype == TB_VPTR);

	if (tab->rbtree) {
		return (STRING)get_rb_value(tab->rbtree, key);
	} else {
		return find_hashtab(tab->hashtab, key, NULL);
	}
}
/*===============================
 * valueof_obj -- Find object value of entry
 *=============================*/
VPTR
valueof_obj (TABLE tab, CNSTRING key)
{
	ASSERT(tab);
	ASSERT(tab->vtable == &vtable_for_table);
	ASSERT(tab->valtype == TB_OBJ);

	if (tab->rbtree) {
		return get_rb_value(tab->rbtree, key);
	} else {
		return find_hashtab(tab->hashtab, key, NULL);
	}
}
/*===================================
 * valueofbool_int -- Find pointer value of entry
 * BOOLEAN *there:   [OUT] FALSE if not found
 *=================================*/
INT
valueofbool_int (TABLE tab, CNSTRING key, BOOLEAN *there)
{
	INT * val=0;

	ASSERT(tab);
	ASSERT(tab->vtable == &vtable_for_table);
	ASSERT(tab->valtype == TB_INT);

	if (tab->rbtree) {
		val = (INT *)rb_valueof(tab->rbtree, key, there);
	} else {
		val = find_hashtab(tab->hashtab, key, there);
	}
	return val ? *val : 0;
}
/*===================================
 * valueofbool_str -- Find string value of entry
 * BOOLEAN *there:   [OUT] FALSE if not found
 *=================================*/
STRING
valueofbool_str (TABLE tab, CNSTRING key, BOOLEAN *there)
{
	ASSERT(tab);
	ASSERT(tab->vtable == &vtable_for_table);
	ASSERT(tab->valtype == TB_STR);

	if (tab->rbtree) {
		return rb_valueof(tab->rbtree, key, there);
	} else {
		return (STRING)find_hashtab(tab->hashtab, key, there);
	}
}
/*===================================
 * valueofbool_ptr -- Find pointer value of entry
 * BOOLEAN *there:   [OUT] FALSE if not found
 *=================================*/
VPTR
valueofbool_ptr (TABLE tab, CNSTRING key, BOOLEAN *there)
{
	ASSERT(tab);
	ASSERT(tab->vtable == &vtable_for_table);
	ASSERT(tab->valtype == TB_HPTR || tab->valtype == TB_VPTR);

	if (tab->rbtree) {
		return rb_valueof(tab->rbtree, key, there);
	} else {
		return find_hashtab(tab->hashtab, key, there);
	}
}
/*===================================
 * valueofbool_obj -- Find object value of entry
 * BOOLEAN *there:   [OUT] FALSE if not found
 *=================================*/
VPTR
valueofbool_obj (TABLE tab, CNSTRING key, BOOLEAN *there)
{
	ASSERT(tab);
	ASSERT(tab->vtable == &vtable_for_table);
	ASSERT(tab->valtype == TB_OBJ);

	if (tab->rbtree) {
		return rb_valueof(tab->rbtree, key, there);
	} else {
		return find_hashtab(tab->hashtab, key, there);
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
	++tabit->refcnt;
	if (tab->rbtree) {
		tabit->rbit = RbBeginIter(tab->rbtree, 0, 0);
	} else {
		tabit->hashtab_iter = begin_hashtab(tab->hashtab);
	}
	return tabit;
}
/*=================================================
 * next_table_ptr -- Advance to next pointer in table
 * skips over any other types of table elements
 * returns FALSE if runs out of table elements
 *===============================================*/
BOOLEAN
next_table_ptr (TABLE_ITER tabit, CNSTRING *pkey, VPTR *pptr)
{
	if (tabit->rbit)
		return RbNext(tabit->rbit, (RBKEY *)pkey, pptr);
	else
		return next_hashtab(tabit->hashtab_iter, pkey, pptr);
}
/*=================================================
 * next_table_int -- Advance to next int in table
 * skips over any other types of table elements
 * returns FALSE if runs out of table elements
 *===============================================*/
BOOLEAN
next_table_int (TABLE_ITER tabit, CNSTRING *pkey, INT * pival)
{
	VPTR val=0;
	if (tabit->rbit) {
		if (!RbNext(tabit->rbit, (RBKEY *)pkey, &val))
			return FALSE;
	} else {
		if (!next_hashtab(tabit->hashtab_iter, pkey, &val))
			return FALSE;
	}

	*pival = *(INT *)val;
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
        if (!tabit->rbit) {
                end_hashtab(&tabit->hashtab_iter);
	}
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

	ASSERT(tab->vtable == &vtable_for_table);

	if (tab->rbtree)
		return RbGetCount(tab->rbtree);
	else
		return get_hashtab_count(tab->hashtab);
}
/*=================================================
 * copy_table -- Copy all elements from src to dest
 *===============================================*/
void
copy_table (const TABLE src, TABLE dest)
{
	if (!src || !dest) return;

	ASSERT(src->vtable == &vtable_for_table);
	ASSERT(dest->vtable == &vtable_for_table);

	if (src->valtype == TB_INT) {
		TABLE_ITER tabit = begin_table_iter(src);
		CNSTRING key=0;
		INT ival=0;
		ASSERT(dest->valtype == TB_INT);
		while (next_table_int(tabit, &key, &ival)) {
			insert_table_int(dest, key, ival);
		}
		end_table_iter(&tabit);
		return;
	}

	if (src->valtype == TB_STR) {
		TABLE_ITER tabit = begin_table_iter(src);
		CNSTRING key=0;
		VPTR val=0;
		ASSERT(dest->valtype == src->valtype);
		while (next_table_ptr(tabit, &key, &val)) {
			insert_table_str(dest, key, val);
		}
		end_table_iter(&tabit);
		return;
	}

	if (src->valtype == TB_HPTR) {
		ASSERT(0);
	}

	if (src->valtype == TB_VPTR) {
		TABLE_ITER tabit = begin_table_iter(src);
		CNSTRING key=0;
		VPTR val=0;
		ASSERT(dest->valtype == src->valtype);
		while (next_table_ptr(tabit, &key, &val)) {
			insert_table_ptr(dest, key, val);
		}
		end_table_iter(&tabit);
		return;
	}

	if (src->valtype == TB_OBJ) {
		TABLE_ITER tabit = begin_table_iter(src);
		CNSTRING key=0;
		VPTR val=0;
		ASSERT(dest->valtype == src->valtype);
		while (next_table_ptr(tabit, &key, &val)) {
			addref_object(val);
			insert_table_obj(dest, key, val);
		}
		end_table_iter(&tabit);
		return;
	}

	ASSERT(0);
}
/*=================================================
 * tabit_destructor -- destructor for table iterator
 *  (vtable destructor)
 *===============================================*/
static void
tabit_destructor (VTABLE *obj)
{
	TABLE_ITER tabit = (TABLE_ITER)obj;
	ASSERT(tabit->vtable == &vtable_for_tabit);
	free_table_iter(tabit);
}
/*=================================================
 * increment_table_int -- increment an integer element value
 * set to 1 if not found
 *===============================================*/
void
increment_table_int (TABLE tab, CNSTRING key)
{
	BOOLEAN found=FALSE;
	INT value = valueofbool_int(tab, key, &found);
	if (found) {
		insert_table_int(tab, key, value+1);
	} else {
		insert_table_int(tab, key, 1);
	}
}
/*=================================================
 * llassert -- Implement assertion
 *  (for rbtree module, which does not include lifelines headers)
 *===============================================*/
static void
llassert (int assertion, const char* error)
{
	if (assertion)
		return;
	FATAL2(error);
}
/*=================================================
 * llalloc -- Implement alloc
 *  (for rbtree module, which does not include lifelines headers)
 *===============================================*/
static void *
llalloc (size_t size)
{
	return stdalloc(size);
}
/*=================================================
 * rbcompare -- compare two rbtree keys
 *  (for rbtree module, which treats keys as opaque)
 *===============================================*/
static int
rbcompare (RBKEY key1, RBKEY key2)
{
	return cmpstrloc(key1, key2);
}
/*=================================================
 * rbdestroy -- Destructor for key & value of rbtree
 *  (for rbtree module, which does not manage key or value memory)
 *===============================================*/
static void
rbdestroy (void * param, RBKEY key, RBVALUE info)
{
	stdfree((void *)key);
	rbdestroy_value((TABLE)param, info);
}
/*=================================================
 * rbdestroy_value -- Destroy value stored in rbtree
 *  (for rbtree module, which does not manage key or value memory)
 *===============================================*/
static void
rbdestroy_value (TABLE tab, RBVALUE info)
{
	if (tab->destroyfunc) {
		(*tab->destroyfunc)(info);
		return;
	}
	switch(tab->valtype) {
	case TB_INT:
	case TB_STR:
	case TB_HPTR:
		table_element_destructor(info);
		break;
	case TB_OBJ:
		table_element_obj_destructor(info);
	case TB_NULL:
	case TB_VPTR:
	        ;
	}
}
/*=================================================
 * get_rb_value -- Shortcut fetch of value from rbtree key
 *===============================================*/
static VPTR
get_rb_value (RBTREE rbtree, CNSTRING key)
{
	RBNODE node = RbExactQuery(rbtree, key);
	return node ? RbGetInfo(node) : 0;
}
/*=================================================
 * rb_valueof -- Get value & set *there to indicate presence
 *===============================================*/
static VPTR
rb_valueof (RBTREE rbtree, CNSTRING key, BOOLEAN *there)
{
	RBNODE node = RbExactQuery(rbtree, key);
	if (there)
		*there = !!node;
	return node ? RbGetInfo(node) : 0;
}
