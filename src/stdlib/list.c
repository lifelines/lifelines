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

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static LNODE nth_in_list_from_tail(LIST list, INT index1b, LIST_CREATE_VALUE createfnc4);
static void validate_list(LIST list);

/*===========================
 * create_list -- Create list
 *=========================*/
LIST
create_list (void)
{
	LIST list = (LIST) stdalloc(sizeof(*list));
	memset(list, 0, sizeof(*list));
	ltype(list) = LISTNOFREE;
	lhead(list) = ltail(list) = NULL;
	llen(list) = 0;
	validate_list(list);
	return list;
}
/*===============================
 * set_list_type -- Set list type
 *  list: [I/O] list to change
 *  type: [IN]  new type (LISTNOFREE or LISTDOFREE)
 *=============================*/
void
set_list_type (LIST list, int type)
{
	ltype(list) = type;
}
/*===========================
 * remove_list -- Delete all elements & delete list
 *  list: [IN]  list to completely delete
 *  func: [IN]  function to call on each element first (may be NULL)
 *=========================*/
void
remove_list (LIST list, void (*func)(VPTR))
{
	LNODE lnode0, lnode;
	if (!list) return;
	lnode0 = lhead(list);
	while (lnode0) {
		lnode = lnext(lnode0);
		if (func) (*func)(lelement(lnode0));
		stdfree(lnode0);
		lnode0 = lnode;
	}
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
	if (!list) return FALSE;
	lnode = lhead(list);
	while (lnode) {
		if((*func)(param, lelement(lnode))) return index;
		lnode = lnext(lnode);
	}
	validate_list(list);
	return -1;
}
/*===================================
 * make_list_empty -- Make list empty
 *=================================*/
void
make_list_empty (LIST list)
{
	LNODE lnode0, lnode;
	BOOLEAN free;
	if (!list) return;
	free = (ltype(list) == LISTDOFREE);
	lnode0 = lhead(list);
	while (lnode0) {
		lnode = lnext(lnode0);
		if (free && lelement(lnode0)) stdfree(lelement(lnode0));
		stdfree(lnode0);
		lnode0 = lnode;
	}
	lhead(list) = ltail(list) = NULL;
	ltype(list) = LISTNOFREE;
	llen(list) = 0;
	/* no effect on refcount */
	validate_list(list);
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
	if (is_empty_list(list)) return NULL;
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
	if (is_empty_list(list)) return NULL;
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
 *===============================================*/
static LNODE
nth_in_list_from_tail (LIST list, INT index1b, LIST_CREATE_VALUE createfnc)
{
	INT i = 1;
	LNODE node = NULL;
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
	} else {
		/* want element beyond end, so add as required */
		INT i = index1b + 1 - llen(list);
		while (--i) {
			VPTR newv = createfnc ? (*createfnc)(list) : NULL;
			enqueue_list(list, newv);
		}
		validate_list(list);
		return lhead(list);
	}
}
/*==================================================
 * set_list_element - Set element using array access
 *================================================*/
void
set_list_element (LIST list, INT index1b, VPTR val, LIST_CREATE_VALUE createfnc)
{
	LNODE node = NULL;
	if (!list) return;
	node = nth_in_list_from_tail(list, index1b, createfnc);
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
	if (!list) return 0;
	node = nth_in_list_from_tail(list, index1b, createfnc);
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
 * begin_list -- Begin iteration of list
 *===============================================*/
BOOLEAN
begin_list (LIST list, LIST_ITER listit)
{
	memset(listit, 0, sizeof(*listit));
	listit->list = list;
	listit->current = 0;
	listit->status = (lhead(listit->list) ? 1 : 0);
	return !!listit->status;
}
/*=================================================
 * begin_list_rev -- Begin reverse iteration of list
 *===============================================*/
BOOLEAN
begin_list_rev (LIST list, LIST_ITER listit)
{
	memset(listit, 0, sizeof(*listit));
	listit->list = list;
	listit->current = 0;
	listit->status = (ltail(listit->list) ? -1 : 0);
	return !!listit->status;
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
		if (listit->status > 0)
			listit->current = lnext(listit->current);
		else
			listit->current = lprev(listit->current);
	}
	if (!listit->current)
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
