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

/*===========================
 * create_list -- Create list
 *=========================*/
LIST create_list ()
{
	LIST list = (LIST) stdalloc(sizeof(*list));
	lfirst(list) = llast(list) = NULL;
	ltype(list) = LISTNOFREE;
	return list;
}
/*===============================
 * set_list_type -- Set list type
 *=============================*/
set_list_type (list, type)
LIST list;
INT type;
{
	ltype(list) = type;
}
/*===========================
 * remove_list -- Remove list
 *=========================*/
remove_list (list, func)
LIST list;
int (*func)();
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
 * in_list -- see if in list
 *=========================*/
BOOLEAN in_list (list, el, func)
LIST list;
WORD el;
int (*func)();
{
	LNODE lnode;
	if (!list) return FALSE;
	lnode = lfirst(list);
	while (lnode) {
		if((*func)(el, lelement(lnode))) return TRUE;
		lnode = lnext(lnode);
	}
	return FALSE;
}
/*===================================
 * make_list_empty -- Make list empty
 *=================================*/
make_list_empty (list)
LIST list;
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
}
/*===================================
 * empty_list -- Check for empty list
 *=================================*/
BOOLEAN empty_list (list)
LIST list;
{
	return !lfirst(list);
}
/*==================================
 * push_list -- Push element on list
 *================================*/
push_list (list, el)
LIST list;  WORD el;
{
	LNODE node = (LNODE) stdalloc(sizeof(*node));
	lelement(node) = el;
	if (empty_list(list)) {
		lprev(node) = lnext(node) = NULL;
		lfirst(list) = llast(list) = node;
	} else {
		lnext(node) = lfirst(list);
		lprev(lfirst(list)) = node;
		lprev(node) = NULL;
		lfirst(list) = node;
	}
}
/*=========================================
 * back_list -- Put element on back of list
 *=======================================*/
back_list (list, el)
LIST list;  WORD el;
{
	LNODE node = (LNODE) stdalloc(sizeof(*node));
	lelement(node) = el;
	if (empty_list(list)) {
		lprev(node) = lnext(node) = NULL;
		lfirst(list) = llast(list) = node;
	} else {
		lprev(node) = llast(list);
		lnext(llast(list)) = node;
		lnext(node) = NULL;
		llast(list) = node;
	}
}
/*==================================
 * pop_list -- Pop element from list
 *================================*/
WORD pop_list (list)
LIST list;
{
	LNODE node;
	WORD el;
	if (empty_list(list)) return NULL;
	node = lfirst(list);
	lfirst(list) = lnext(node);
	if (empty_list(list))
		llast(list) = NULL;
	else
		lprev(lfirst(list)) = NULL;
	el = lelement(node);
	stdfree(node);
	return el;
}
/*========================================
 * enqueue_list -- Enqueue element on list
 *======================================*/
enqueue_list (list, el)
LIST list;
WORD el;
{
	push_list(list, el);
}
/*==========================================
 * dequeue_list -- Dequeue element from list
 *========================================*/
WORD dequeue_list (list)
LIST list;
{
	LNODE node;
	WORD el;
	if (empty_list(list)) return NULL;
	node = llast(list);
	llast(list) = lprev(node);
	if (!llast(list))
		lfirst(list) = NULL;
	else
		lnext(llast(list)) = NULL;
	el = lelement(node);
	stdfree(node);
	return el;
}
/*=================================================
 * nth_in_list -- Find nth node in list, relative 1
 *===============================================*/
static LNODE nth_in_list (list, n)
LIST list;
INT n;
{
	INT i = 1;
	LNODE node = llast(list);
	while (i < n && node) {
		i++;
		node = lprev(node);
	}
	if (i == n && node) return node;
	while (i++ <= n)
		push_list(list, NULL);
	return lfirst(list);
}
/*==================================================
 * set_list_element - Set element using array access
 *================================================*/
set_list_element (list, ind, val)
LIST list;
INT ind;
WORD val;
{
	LNODE node = nth_in_list(list, ind);
	lelement(node) = val;
}
/*=======================================================
 * get_list_element - Retrieve element using array access
 *=====================================================*/
WORD get_list_element (list, ind)
LIST list;
INT ind;
{
	LNODE node = nth_in_list(list, ind);
	return lelement(node);
}
/*==================================
 * length_list -- Return list length
 *================================*/
INT length_list (list)
LIST list;
{
	LNODE node;
	INT len = 0;
	if (!list) return 0;
	node = lfirst(list);
	while (node) {
		len++;
		node = lnext(node);
	}
	return len;
}
