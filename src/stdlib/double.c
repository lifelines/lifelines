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
 * double.c -- Doubly-linked list data type
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 18 May 94
 *===========================================================*/

#include "standard.h"
#include "llstdlib.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void validate_list(LIST list);
static void free_heapstring_el(VPTR w);


/*===========================
 * create_list -- Create list
 *=========================*/
LIST
create_list (void)
{
	LIST list = (LIST) stdalloc(sizeof(*list));
	memset(list, 0, sizeof(*list));
	ltype(list) = LISTNOFREE;
	lfirst(list) = llast(list) = NULL;
	llen(list) = 0;
	lrefcnt(list) = 1;
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
	lnode0 = lfirst(list);
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
in_list (LIST list, VPTR el, int (*func)(VPTR, VPTR))
{
	LNODE lnode;
	INT index=0;
	if (!list) return FALSE;
	lnode = lfirst(list);
	while (lnode) {
		if((*func)(el, lelement(lnode))) return index;
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
	lnode0 = lfirst(list);
	while (lnode0) {
		lnode = lnext(lnode0);
		if (free && lelement(lnode0)) stdfree(lelement(lnode0));
		stdfree(lnode0);
		lnode0 = lnode;
	}
	lfirst(list) = llast(list) = NULL;
	ltype(list) = LISTNOFREE;
	llen(list) = 0;
	/* no effect on refcount */
	validate_list(list);
}
/*===================================
 * is_empty_list -- Check for empty list
 *=================================*/
BOOLEAN
is_empty_list (LIST list)
{
	validate_list(list);
	return !list || !llen(list);
}
/*==================================
 * push_list -- Push element on list
 *================================*/
void
push_list (LIST list,
           VPTR el)
{
	LNODE node = NULL;

	if (!list) return;
	node = (LNODE) stdalloc(sizeof(*node));
	lelement(node) = el;
	if (is_empty_list(list)) {
		lprev(node) = lnext(node) = NULL;
		lfirst(list) = llast(list) = node;
	} else {
		lnext(node) = lfirst(list);
		lprev(lfirst(list)) = node;
		lprev(node) = NULL;
		lfirst(list) = node;
	}
	++llen(list);
	validate_list(list);
}
/*=========================================
 * back_list -- Put element on back of list
 *=======================================*/
void
back_list (LIST list,
           VPTR el)
{
	LNODE node = NULL;

	if (!list) return;
	node = (LNODE) stdalloc(sizeof(*node));
	lelement(node) = el;
	if (is_empty_list(list)) {
		lprev(node) = lnext(node) = NULL;
		lfirst(list) = llast(list) = node;
	} else {
		lprev(node) = llast(list);
		lnext(llast(list)) = node;
		lnext(node) = NULL;
		llast(list) = node;
	}
	++llen(list);
	validate_list(list);
}
/*==================================
 * pop_list -- Pop element from list
 *================================*/
VPTR
pop_list (LIST list)
{
	LNODE node;
	VPTR el;
	if (is_empty_list(list)) return NULL;
	node = lfirst(list);
	lfirst(list) = lnext(node);
	if (!lfirst(list))
		llast(list) = NULL;
	else
		lprev(lfirst(list)) = NULL;
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
	ASSERT(!list || (lfirst(list)&&llast(list)) || (!lfirst(list)&&!llast(list)));
#else
	list=list; /* unused */
#endif
}
/*========================================
 * enqueue_list -- Enqueue element on list
 *======================================*/
void
enqueue_list (LIST list, VPTR el)
{
	push_list(list, el);
}
/*==========================================
 * dequeue_list -- Dequeue element from list
 *========================================*/
VPTR
dequeue_list (LIST list)
{
	LNODE node;
	VPTR el;
	if (is_empty_list(list)) return NULL;
	node = llast(list);
	llast(list) = lprev(node);
	if (!llast(list))
		lfirst(list) = NULL;
	else
		lnext(llast(list)) = NULL;
	el = lelement(node);
	stdfree(node);
	--llen(list);
	validate_list(list);
	return el;
}
/*=================================================
 * nth_in_list -- Find nth node in list, relative 1
 *===============================================*/
static LNODE
nth_in_list (LIST list, INT n)
{
	INT i = 1;
	LNODE node = NULL;
	if (!list) return NULL;
	node = llast(list);
	while (i < n && node) {
		i++;
		node = lprev(node);
	}
	validate_list(list);
	if (i == n && node) return node;
	while (i++ <= n)
		push_list(list, NULL);
	validate_list(list);
	return lfirst(list);
}
/*==================================================
 * set_list_element - Set element using array access
 *================================================*/
void
set_list_element (LIST list, INT ind, VPTR val)
{
	LNODE node = NULL;
	if (!list) return;
	node = nth_in_list(list, ind);
	if (!node) return;
	lelement(node) = val;
	validate_list(list);
}
/*=======================================================
 * get_list_element - Retrieve element using array access
 *=====================================================*/
VPTR
get_list_element (LIST list, INT ind)
{
	LNODE node = NULL;
	if (!list) return 0;
	node = nth_in_list(list, ind);
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
/*====================================================
 * free_heapstring_el -- free alloc'd string element of list
 *==================================================*/
static void
free_heapstring_el (VPTR w)
{
	stdfree((STRING)w);
}
/*====================================================
 * remove_heapstring_list -- free a dblist (caller is done with it)
 *==================================================*/
void
remove_heapstring_list (LIST list)
{
	if (list) {
		remove_list(list, free_heapstring_el);
	}
}
