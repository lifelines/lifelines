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
 * nodeutls.c -- Various node utility functions
 * Copyright(c) 1995-96 by T.T. Wetmore IV; all rights reserved
 *   3.0.3 - 15 Feb 96
 *===========================================================*/

#include "llstdlib.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static BOOLEAN nvaldiff(NODE node1, NODE node2);

/*======================================================================
 * unique_nodes -- Remove duplicates from list of nodes -- original list
 *   is modified
 *====================================================================*/
NODE
unique_nodes (NODE node,
              BOOLEAN kids)     /* children matter */
{
	NODE node0 = node, prev, this, next;

	if (!node) return NULL;
	while (node) {
		prev = node;
		this = nsibling(node);
		while (this) {
			if (iso_nodes(node, this, kids, FALSE)) {
				nsibling(prev) = next = nsibling(this);
				nsibling(this) = NULL;
				free_nodes(this);
				this = next;
			} else {
				prev = this;
				this = nsibling(this);
			}
		}
		node = nsibling(node);
	}
	return node0;
}
/*==============================================
 * union_nodes -- Return union of two node trees
 *============================================*/
NODE
union_nodes (NODE node1,
             NODE node2,
             BOOLEAN kids,      /* children matter */
             BOOLEAN copy)      /* copy operands first */
{
	NODE curs1, next1, prev1, curs2, prev2;

	if (copy) node1 = copy_nodes(node1, TRUE, TRUE);
	if (copy) node2 = copy_nodes(node2, TRUE, TRUE);
	prev2 = NULL;
	curs2 = node2;
	while (curs2) {
		prev1 = NULL;
		curs1 = node1;
		while (curs1 && !iso_nodes(curs1, curs2, kids, FALSE)) {
			prev1 = curs1;
			curs1 = nsibling(curs1);
		}
		if (curs1) {
			next1 = nsibling(curs1);
			nsibling(curs1) = NULL;
			free_nodes(curs1);
			if (prev1)
				nsibling(prev1) = next1;
			else
				node1 = next1;
		}
		prev2 = curs2;
		curs2 = nsibling(curs2);
	}
	if (prev2) {
		nsibling(prev2) = node1;
		return node2;
	}
	return node1;
}

#ifdef UNUSED_CODE
/*=========================================================
 * intersect_nodes -- Return intersection of two node trees
 * UNUSED CODE
 *=======================================================*/
NODE
intersect_nodes (NODE node1,
                 NODE node2,
                 BOOLEAN kids)  /* children matter */
{
	NODE prev1, curs1, next1, prev2, curs2, next2;
	NODE node3, curs3;

	if (!node1 || !node2) return NULL;
	node1 = copy_nodes(node1, TRUE, TRUE);
	node2 = copy_nodes(node2, TRUE, TRUE);
	node3 = curs3 = NULL;

	prev1 = NULL;
	curs1 = node1;
	while (curs1) {
		prev2 = NULL;
		curs2 = node2;
		while (curs2 && !iso_nodes(curs1, curs2, kids, FALSE)) {
			prev2 = curs2;
			curs2 = nsibling(curs2);
		}
		if (curs2) {
			next2 = nsibling(curs2);
			nsibling(curs2) = NULL;

			if (node3)
				curs3 = nsibling(curs3) = curs2;
			else
				node3 = curs3 = curs2;
			if (prev2)
				nsibling(prev2) = next2;
			else
				node2 = next2;

			next1 = nsibling(curs1);
			nsibling(curs1) = NULL;
			free_nodes(curs1);
			if (prev1)
				nsibling(prev1) = next1;
			else
				node1 = next1;
			curs1 = next1;

		} else {
			prev1 = curs1;
			curs1 = nsibling(curs1);
		}
	}
	free_nodes(node1);
	free_nodes(node2);
	return node3;
}
#endif /* UNUSED_CODE */
/*========================================================================
 * classify_nodes -- Convert two value lists to three lists - first
 *   returned list holds all values that were only in original first
 *   list - second returned list holds all values that were just in
 *   original second list - third returned list holds all values that were
 *   in both original lists
 * pnode1 & pnode2 are the input lists to compare
 * pnode1, pnode2, & pnode3 are the output lists.
 *======================================================================*/
void
classify_nodes (NODE *pnode1, NODE *pnode2, NODE *pnode3)
{
	NODE node1, node2, node3, curs1, curs2, curs3;
	NODE prev1, prev2, next2;

	curs1 = node1 = unique_nodes(*pnode1, FALSE);
	curs2 = node2 = unique_nodes(*pnode2, FALSE);
	curs3 = node3 = prev1 = prev2 = NULL;

	while (curs1 && curs2) {
		if (!nvaldiff(curs1, curs2)) {
			if (node3)
				curs3 = nsibling(curs3) = curs1;
			else
				node3 = curs3 = curs1;

			curs1 = nsibling(curs1);
			nsibling(curs3) = NULL;

			if (prev1)
				nsibling(prev1) = curs1;
			else
				node1 = curs1;

			next2 = nsibling(curs2);
			if (prev2)
				nsibling(prev2) = next2;
			else
				node2 = next2;
			nsibling(curs2) = NULL;
			free_nodes(curs2);
			curs2 = next2;
			continue;
		}
		prev2 = curs2;
		if ((curs2 = nsibling(curs2))) continue;
		prev1 = curs1;
		curs1 = nsibling(curs1);
		prev2 = NULL;
		curs2 = node2;
	}
	*pnode1 = node1;
	*pnode2 = node2;
	*pnode3 = node3;
}
/*===============================================
 * nvaldiff -- Do nodes have different values ?
 *  handles NULLs in either
 * Created: 2001/04/08, Perry Rapp
 *=============================================*/
static BOOLEAN
nvaldiff (NODE node1, NODE node2)
{
	if (!nval(node1) && !nval(node2)) return FALSE;
	if (!nval(node1) || !nval(node2)) return TRUE;
	return strcmp(nval(node1), nval(node2));
}
#ifdef UNUSED_CODE
/*========================================================================
 * difference_nodes -- Return difference of two node lists -- all in node1
 *   that are not in node2
 * UNUSED CODE
 *======================================================================*/
NODE
difference_nodes (NODE node1,
                  NODE node2,
                  BOOLEAN kids) /* children matter */
{
	NODE prev1, next1, curs1, curs2;
	node1 = copy_nodes(node1, TRUE, TRUE);
	prev1 = NULL;
	curs1 = node1;
	while (curs1) {
		curs2 = node2;
		while (curs2 && !iso_nodes(curs1, curs2, kids, FALSE))
			curs2 = nsibling(curs2);
		if (curs2) {
			next1 = nsibling(curs1);
			nsibling(curs1) = NULL;
			free_nodes(curs1);
			if (!prev1)
				node1 = next1;
			else
				nsibling(prev1) = next1;
			curs1 = next1;
		} else {
			prev1 = curs1;
			curs1 = nsibling(curs1);
		}
	}
	return node1;
}
#endif /* UNUSED_CODE */
#ifdef UNUSED_CODE
/*================================================================
 * value_in_nodes -- See if a list of nodes contains a given value
 * UNUSED CODE
 *==============================================================*/
BOOLEAN
value_in_nodes (NODE node,
                STRING value)
{
	while (node) {
		if (eqstr(value, nval(node))) return TRUE;
		node = nsibling(node);
	}
	return FALSE;
}
#endif /* UNUSED_CODE */
