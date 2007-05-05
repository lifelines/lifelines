/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * list.c -- Doubly-linked list data type
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *===========================================================*/

#include "standard.h"
#include "llstdlib.h"
#include "vtable.h"

/*********************************************
 * local types
 *********************************************/


/* list object itself */
struct tag_list {
	/* a LIST is an OBJECT */
	struct tag_vtable * vtable; /* generic object table (see vtable.h) */
	INT l_refcnt; /* reference counted object */
	LNODE l_head;
	LNODE l_tail;
	INT l_len;
	INT l_type;
	ELEMENT_DESTRUCTOR l_del_element;
};
/* typedef struct tag_list *LIST; */ /* in list.h */

/* list iterator */
struct tag_list_iter {
	struct tag_vtable *vtable; /* generic object */
	INT refcnt; /* ref-countable object */
	LNODE current;
	LIST list;
	INT status; /* 1=forward, 1=reverse, 0=EOF */
};
/* typedef struct tag_list_iter * LIST_ITER; */ /* in list.h */

/*********************************************
 * local enums & defines
 *********************************************/

#define ltype(l)   ((l)->l_type)
#define lhead(l)   ((l)->l_head)
#define ltail(l)   ((l)->l_tail)
#define llen(l)    ((l)->l_len)

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void free_list_element(VPTR vptr);
static void free_list_iter(LIST_ITER listit);
static void list_destructor(VTABLE *obj);
static void listit_destructor(VTABLE *obj);
void make_list_empty_impl(LIST list, ELEMENT_DESTRUCTOR func);
static LNODE nth_in_list_from_tail(LIST list, INT index1b, BOOLEAN createels
	, LIST_CREATE_VALUE createfnc);
static void validate_list(LIST list);

/*********************************************
 * local variables
 *********************************************/

static struct tag_vtable vtable_for_list = {
	VTABLE_MAGIC
	, "list"
	, &list_destructor
	, &refcountable_isref
	, &refcountable_addref
	, &refcountable_release
	, 0 /* copy_fnc */
	, &generic_get_type_name
};
static struct tag_vtable vtable_for_listit = {
	VTABLE_MAGIC
	, "list_iter"
	, &listit_destructor
	, &refcountable_isref
	, &refcountable_addref
	, &refcountable_release
	, 0 /* copy_fnc */
	, &generic_get_type_name
};

/*===========================
 * create_list_impl -- Create list
 * returns addref'd list
 *=========================*/
static LIST
create_list_impl (void)
{
	LIST list = (LIST) stdalloc(sizeof(*list));
	memset(list, 0, sizeof(*list));
	list->vtable = &vtable_for_list;
	list->l_refcnt = 1;
	ltype(list) = LISTNOFREE;
	lhead(list) = ltail(list) = NULL;
	llen(list) = 0;
	validate_list(list);
	return list;
}
/*===========================
 * create_list -- Create list (LISTNOFREE)
 * returns addref'd list
 *=========================*/
LIST
create_list (void)
{
	return create_list2(LISTNOFREE);
}
/*===========================
 * create_list2 -- Create list, with free type
 * returns addref'd list
 *=========================*/
LIST
create_list2 (INT whattofree)
{
	LIST list = create_list_impl();
	ltype(list) = whattofree;
	return list;
}
/*===========================
 * create_list3 -- Create list, with element destructor
 * returns addref'd list
 *=========================*/
LIST
create_list3 (ELEMENT_DESTRUCTOR func)
{
	LIST list = create_list_impl();
	list->l_del_element = func;
	return list;
}
/*===========================
 * destroy_list -- Delete all elements & destroy list
 *  list: [IN]  list to completely delete
 *=========================*/
void
destroy_list (LIST list)
{
	if (!list) return;
	ASSERT(list->vtable == &vtable_for_list);
	make_list_empty_impl(list, NULL);
	destroy_empty_list(list);
}
/*===========================
 * destroy_empty_list -- Destroy a list with no elements
 *  ASSERT check that list is in fact empty
 *=========================*/
void
destroy_empty_list (LIST list)
{
	if (!list) return;
	ASSERT(list->vtable == &vtable_for_list);
	ASSERT(llen(list) == 0);
	stdfree(list);
}
/*===========================
 * in_list -- find first element returning true from check function
 *  list: [IN]  list to search
 *  el:   [IN]  parameter to pass thru to check function
 *  func: [IN]  check function
 * Calls check function on each element in turn until one returns TRUE
 * Returns index of element found, or -1 if none pass check
 *=========================*/
INT
in_list (LIST list, VPTR param, BOOLEAN (*func)(VPTR param, VPTR el))
{
	LNODE lnode;
	INT index=0;
	if (is_empty_list(list)) /* calls validate_list */
		return -1;
	lnode = lhead(list);
	while (lnode) {
		if ((*func)(param, lelement(lnode)))
			return index;
		lnode = lnext(lnode);
	}
	validate_list(list);
	return -1;
}
/*===================================
 * free_list_element -- Simple heap element destructor
 *=================================*/
static void
free_list_element (VPTR vptr)
{
	if (vptr) stdfree(vptr);
}
/*===================================
 * make_list_empty_impl -- Make list empty
 *=================================*/
void
make_list_empty_impl (LIST list, ELEMENT_DESTRUCTOR func)
{
	LNODE lnode0, lnode;

	if (!list) return;

	if (!func)
		func = list->l_del_element;
	if (!func) {
		if (ltype(list) == LISTDOFREE)
			func = &free_list_element;
	}
	
	lnode0 = lhead(list);
	while (lnode0) {
		lnode = lnext(lnode0);
		if (func) (*func)(lelement(lnode0));
		stdfree(lnode0);
		lnode0 = lnode;
	}
	lhead(list) = ltail(list) = NULL;
	llen(list) = 0;
	/* no effect on refcount */
	validate_list(list);
}
/*===================================
 * make_list_empty -- Make list empty
 *=================================*/
void
make_list_empty (LIST list)
{
	make_list_empty_impl(list, NULL);
}
/*===================================
 * is_empty_list -- Check for empty list
 *  list:  [IN]  list
 *=================================*/
BOOLEAN
is_empty_list (const LIST list)
{
	validate_list(list);
	return !list || !llen(list);
}
/*==================================
 * push_list -- Push element on head of list
 *  list:  [I/O]  list
 *  el:    [IN]   new element
 *================================*/
void
push_list (LIST list, VPTR el)
{
	LNODE node = NULL;

	if (!list) return;
	node = (LNODE) stdalloc(sizeof(*node));
	lelement(node) = el;
	if (is_empty_list(list)) {
		lprev(node) = lnext(node) = NULL;
		lhead(list) = ltail(list) = node;
	} else {
		lnext(node) = lhead(list);
		lprev(lhead(list)) = node;
		lprev(node) = NULL;
		lhead(list) = node;
	}
	++llen(list);
	validate_list(list);
}
/*=========================================
 * back_list -- Put element on tail of list
 *  list:  [I/O]  list
 *  el:    [IN]   new element
 *=======================================*/
void
back_list (LIST list, VPTR el)
{
	LNODE node = NULL;

	if (!list) return;
	node = (LNODE) stdalloc(sizeof(*node));
	lelement(node) = el;
	if (is_empty_list(list)) {
		lprev(node) = lnext(node) = NULL;
		lhead(list) = ltail(list) = node;
	} else {
		lprev(node) = ltail(list);
		lnext(ltail(list)) = node;
		lnext(node) = NULL;
		ltail(list) = node;
	}
	++llen(list);
	validate_list(list);
}
/*==================================
 * pop_list -- Pop element from head of list
 *  list:  [I/O]  list
 *================================*/
VPTR
pop_list (LIST list)
{
	LNODE node;
	VPTR el;
	if (is_empty_list(list)) /* calls validate_list */
		return NULL;
	node = lhead(list);
	lhead(list) = lnext(node);
	if (!lhead(list))
		ltail(list) = NULL;
	else
		lprev(lhead(list)) = NULL;
	el = lelement(node);
	stdfree(node);
	--llen(list);
	validate_list(list);
	return el;
}
/*========================================
 * validate_list -- Verify list ends don't disagree
 *======================================*/
static void
validate_list (LIST list)
{
#ifdef LIST_ASSERTS
	ASSERT(!list || (lhead(list)&&ltail(list)) || (!lhead(list)&&!ltail(list)));
#else
	list=list; /* unused */
#endif
}
/*========================================
 * enqueue_list -- Enqueue element on head of list
 *======================================*/
void
enqueue_list (LIST list, VPTR el)
{
	push_list(list, el);
}
/*==========================================
 * dequeue_list -- Dequeue element from tail of list
 *========================================*/
VPTR
dequeue_list (LIST list)
{
	return pop_list_tail(list);
}
/*==========================================
 * pop_list_tail -- Pop element from tail of list
 *========================================*/
VPTR
pop_list_tail (LIST list)
{
	LNODE node;
	VPTR el;
	if (is_empty_list(list)) /* calls validate_list */
		return NULL;
	node = ltail(list);
	ltail(list) = lprev(node);
	if (!ltail(list))
		lhead(list) = NULL;
	else
		lnext(ltail(list)) = NULL;
	el = lelement(node);
	stdfree(node);
	--llen(list);
	validate_list(list);
	return el;
}
/*=================================================
 * nth_in_list_from_tail -- Find nth node in list, relative 1
 *  start at tail & count towards head
 *  createels is FALSE if caller does not want elements added (delete_list_element)
 *===============================================*/
static LNODE
nth_in_list_from_tail (LIST list, INT index1b, BOOLEAN createels, LIST_CREATE_VALUE createfnc)
{
	if (!list) return NULL;
	validate_list(list);
	/* handle reverse indices */
	if (index1b < 1) index1b += llen(list);
	/* null if out of bounds */
	if (index1b < 1) return NULL;
	if (index1b < llen(list)/2) {
		/* want element in back half, so count back from tail */
		LNODE node = ltail(list);
		INT i = index1b;
		while (--i) {
			node = lprev(node);
		}
		return node;
	} else if (index1b <= llen(list)) {
		/* want element in front half, so count forward from head */
		LNODE node = lhead(list);
		INT i = llen(list) + 1 - index1b;
		while (--i) {
			node = lnext(node);
		}
		return node;
	} else if (createels) {
		/* want element beyond end, so add as required */
		INT i = index1b + 1 - llen(list);
		while (--i) {
			VPTR newv = createfnc ? (*createfnc)(list) : NULL;
			enqueue_list(list, newv);
		}
		validate_list(list);
		return lhead(list);
	} else {
		/* element beyond but caller said not to create */
		return NULL;
	}
}
/*==================================================
 * set_list_element - Set element using array access
 *================================================*/
void
set_list_element (LIST list, INT index1b, VPTR val, LIST_CREATE_VALUE createfnc)
{
	LNODE node = NULL;
	BOOLEAN createels = TRUE;
	if (!list) return;
	node = nth_in_list_from_tail(list, index1b, createels, createfnc);
	if (!node) return;
	lelement(node) = val;
	validate_list(list);
}
/*=======================================================
 * get_list_element - Retrieve element using array access
 *=====================================================*/
VPTR
get_list_element (LIST list, INT index1b, LIST_CREATE_VALUE createfnc)
{
	LNODE node = NULL;
	BOOLEAN createels = TRUE;
	if (!list) return 0;
	node = nth_in_list_from_tail(list, index1b, createels, createfnc);
	if (!node) return 0;
	return lelement(node);
}
/*==================================
 * length_list -- Return list length
 *================================*/
INT
length_list (LIST list)
{
	return !list ? 0 : llen(list);
}
/*=======================================================
 * peek_list_head - Retrieve head element without removing it
 *=====================================================*/
VPTR
peek_list_head (LIST list)
{
	LNODE node;
	if (!list) return 0;
	node = lhead(list);
	if (!node) return 0;
	return lelement(node);
}
/*=================================================
 * create_list_iter -- Create new list iterator
 *===============================================*/
static LIST_ITER
create_list_iter (LIST list)
{
	LIST_ITER listit = (LIST_ITER)stdalloc(sizeof(*listit));
	memset(listit, 0, sizeof(*listit));
	listit->list = list;
	listit->refcnt = 1;
	return listit;
}
/*=================================================
 * begin_list -- Begin iteration of list
 *===============================================*/
LIST_ITER
begin_list (LIST list)
{
	LIST_ITER listit = create_list_iter(list);
	/* current=0 is signal to next_list_element that we're starting */
	listit->status = (lhead(listit->list) ? 1 : 0);
	return listit;
}
/*=================================================
 * begin_list_rev -- Begin reverse iteration of list
 *===============================================*/
LIST_ITER
begin_list_rev (LIST list)
{
	LIST_ITER listit = create_list_iter(list);
	/* current=0 is signal to next_list_element that we're starting */
	listit->status = (ltail(listit->list) ? -1 : 0);
	return listit;
}
/*=================================================
 * next_element -- Find next element in list (iterating)
 *===============================================*/
static BOOLEAN
next_list_element (LIST_ITER listit)
{
	if (!listit->status)
		return FALSE;
	if (!listit->current) {
		/* beginning */
		if (listit->status > 0)
			listit->current = lhead(listit->list);
		else
			listit->current = ltail(listit->list);
	} else {
		unlock_list_node(listit->current);
		if (listit->status > 0)
			listit->current = lnext(listit->current);
		else
			listit->current = lprev(listit->current);
	}
	if (listit->current)
		lock_list_node(listit->current);
	else
		listit->status = 0;
	return !!listit->status;
}
/*=================================================
 * next_list_ptr -- Iterating list with pointers
 *===============================================*/
BOOLEAN
next_list_ptr (LIST_ITER listit, VPTR *pptr)
{
	if (!next_list_element(listit)) {
		*pptr = 0;
		return FALSE;
	}
	*pptr = lelement(listit->current);
	return TRUE;
}
/*=================================================
 * change_list_ptr -- User changing pointer during iteration
 *===============================================*/
BOOLEAN
change_list_ptr (LIST_ITER listit, VPTR newptr)
{
	if (!listit || !listit->current)
		return FALSE;
	lelement(listit->current) = newptr;
	return TRUE;
}
/*=================================================
 * end_list_iter -- Release reference to list iterator object
 *===============================================*/
void
end_list_iter (LIST_ITER * plistit)
{
	ASSERT(plistit);
	ASSERT(*plistit);
	--(*plistit)->refcnt;
	if (!(*plistit)->refcnt) {
		free_list_iter(*plistit);
	}
	*plistit = 0;
}
/*=================================================
 * free_list_iter -- Delete & free table iterator object
 *===============================================*/
static void
free_list_iter (LIST_ITER listit)
{
	if (!listit) return;
	ASSERT(!listit->refcnt);
	memset(listit, 0, sizeof(*listit));
	stdfree(listit);
}
/*=================================================
 * lock_list_node -- Increment node lock count
 *===============================================*/
void
lock_list_node (LNODE node)
{
	++llocks(node);
}
/*=================================================
 * unlock_list_node -- Decrement node lock count
 *===============================================*/
void
unlock_list_node (LNODE node)
{
	--llocks(node);
	ASSERT(llocks(node) >= 0);
}
/*==================================================
 * delete_list_element - Delete element using array access
 *  Call func (unless NULL) on element before deleting
 *================================================*/
#ifdef UNUSED_CODE
BOOLEAN
delete_list_element (LIST list, INT index1b, ELEMENT_DESTRUCTOR func)
{
	LNODE node = NULL;
	BOOLEAN createels = FALSE;
	if (!list) return FALSE;
	node = nth_in_list_from_tail(list, index1b, createels, 0);
	if (!node) return FALSE;
	if (llocks(node)) return FALSE;
	if (llen(list) == 1) {
		/* removing last element of list */
		llen(list) = 0;
		lhead(list) = ltail(list) = 0;
		if (func)
			(*func)(lelement(node));
		stdfree(node);
		return TRUE;
	}
	detach_node_from_list(list, node);
	return TRUE;
}
#endif
/*==================================================
 * detach_node_from_list - Remove node from list
 *  does not delete node, simply detaches it from the list
 *  and decrements list's count
 *================================================*/
static void
detach_node_from_list (LIST list, LNODE node)
{
	validate_list(list);
	if (lprev(node)) {
		lnext(lprev(node)) = lnext(node);
	}
	if (lnext(node)) {
		lprev(lnext(node)) = lprev(node);
	}
	if (lhead(list) == node) {
		lhead(list) = lnext(node);
	}
	if (ltail(list) == node) {
		ltail(list) = lprev(node);
	}
	--llen(list);
	validate_list(list);
}
/*==================================================
 * find_delete_list_elements - Delete qualifying element(s)
 *  list:      [I/O] list to change
 *  func:      [IN]  test function to qualify elements (return TRUE to choose)
 *  deleteall: [IN]  true to delete all qualifying, false to delete first
 * returns number elements deleted
 *================================================*/
INT
find_delete_list_elements (LIST list, VPTR param,
	BOOLEAN (*func)(VPTR param, VPTR el), BOOLEAN deleteall)
{
	LNODE lnode = NULL;
	INT count = 0;
	if (is_empty_list(list)) /* calls validate_list */
		return 0;
	ASSERT(func);
	lnode = lhead(list);
	while (lnode) {
		LNODE lnext = lnext(lnode);
		if ((*func)(param, lelement(lnode))) {
			detach_node_from_list(list, lnode);
			++count;
			if (ltype(list) == LISTDOFREE) {
				free_list_element(lelement(lnode));
			}
			if (!deleteall)
				return count;
		}
		lnode = lnext;
	}
	return count;

}
/*==================================================
 * trav_list_head - Return tail node of list
 *  Only for internal use in FORLIST implementation
 *================================================*/
LNODE
trav_list_head (LIST list)
{
	ASSERT(list);
	return list->l_head;
}
/*==================================================
 * trav_list_tail - Return tail node of list
 *  Only for internal use in FORLIST implementation
 *================================================*/
LNODE
trav_list_tail (LIST list)
{
	ASSERT(list);
	return list->l_tail;
}
/*=================================================
 * list_destructor -- destructor for list
 *  (destructor entry in vtable)
 *===============================================*/
static void
list_destructor (VTABLE *obj)
{
	LIST list = (LIST)obj;
	ASSERT((*obj) == &vtable_for_list);
	destroy_list(list);
}
/*=================================================
 * listit_destructor -- destructor for list iterator
 *  (vtable destructor)
 *===============================================*/
static void
listit_destructor (VTABLE *obj)
{
	LIST_ITER listit = (LIST_ITER)obj;
	ASSERT(listit->vtable == &vtable_for_listit);
	free_list_iter(listit);
}

/*=================================================
 * addref_list -- increment reference count of list
 *===============================================*/
void
addref_list (LIST list)
{
	ASSERT(list->vtable == &vtable_for_list);
	++list->l_refcnt;
}
/*=================================================
 * release_list -- decrement reference count of list
 *  and free if appropriate (ref count hits zero)
 *===============================================*/
void
release_list (LIST list)
{
	ASSERT(list->vtable == &vtable_for_list);
	--list->l_refcnt;
	if (!list->l_refcnt) {
		destroy_list(list);
	}
}
