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
 * equaliso.c -- Equality operations on node trees
 * Copyright(c) 1992-95 by T.T. Wetmore IV; all rights reserved
 *   3.0.3 - 15 Aug 95
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"

/*=========================================
 * equal_tree -- See if two trees are equal
 *=======================================*/
BOOLEAN
equal_tree (NODE root1,
            NODE root2)
{
	STRING str1, str2;
	if (!root1 && !root2) return TRUE;
	if (!root1 || !root2) return FALSE;
	if (length_nodes(root1) != length_nodes(root2)) return FALSE;
	while (root1) {
		if (nestr(ntag(root1), ntag(root2))) return FALSE;
		str1 = nval(root1);
		str2 = nval(root2);
		if (str1 && !str2) return FALSE;
		if (str2 && !str1) return FALSE;
		if (str1 && str2 && nestr(str1, str2)) return FALSE;
		if (!equal_tree(nchild(root1), nchild(root2))) return FALSE;
		root1 = nsibling(root1);
		root2 = nsibling(root2);
	}
	return TRUE;
}
/*=========================================
 * equal_node -- See if two nodes are equal
 *=======================================*/
BOOLEAN
equal_node (NODE node1,
            NODE node2)
{
	STRING str1, str2;
	if (!node1 && !node2) return TRUE;
	if (!node1 || !node2) return FALSE;
	if (nestr(ntag(node1), ntag(node2))) return FALSE;
	str1 = nval(node1);
	str2 = nval(node2);
	if (str1 && !str2) return FALSE;
	if (str2 && !str1) return FALSE;
	if (str1 && str2 && nestr(str1, str2)) return FALSE;
	return TRUE;
}
/*=================================================
 * iso_list -- See if two node lists are isomorphic
 *===============================================*/
BOOLEAN
iso_list (NODE root1,
          NODE root2)
{
	INT len1, len2;
	NODE node1, node2;
	if (!root1 && !root2) return TRUE;
	if (!root1 || !root2) return FALSE;
	len1 = length_nodes(root1);
	len2 = length_nodes(root2);
	if (len1 != len2) return FALSE;
	if (len1 == 0) return TRUE;
	node1 = root1;
	while (node1) {
		node2 = root2;
		while (node2) {
			if (equal_node(node1, node2))
				break;
			node2 = nsibling(node2);
		}
		if (!node2) return FALSE;
		node1 = nsibling(node1);
	}
	return TRUE;
}

/*====================================================
 * equal_nodes -- See if two node structures are equal
 *==================================================*/
BOOLEAN
equal_nodes (NODE root1,
             NODE root2,
             BOOLEAN kids,
             BOOLEAN sibs)
{
	if (!root1 && !root2) return TRUE;
	while (root1) {
		if (!equal_node(root1, root2)) return FALSE;
		if (kids && !equal_nodes(nchild(root1), nchild(root2), 1, 1))
			return FALSE;
		if (!sibs) return TRUE;
		root1 = nsibling(root1);
		root2 = nsibling(root2);
	}
	return (root2 == NULL);
}
/*=======================================================
 * iso_nodes -- See if two node structures are isomorphic
 *=====================================================*/
BOOLEAN
iso_nodes (NODE root1,
           NODE root2,
           BOOLEAN kids,
           BOOLEAN sibs)
{
	INT len1, len2;
	NODE node1, node2;

	if (!root1 && !root2) return TRUE;
	if (!root1 || !root2) return FALSE;

	if (!kids && !sibs) return equal_node(root1, root2);
	if ( kids && !sibs)
		return equal_node(root1, root2) && iso_nodes(nchild(root1),
		    nchild(root2), 1, 1);

	len1 = length_nodes(root1);
	len2 = length_nodes(root2);
	if (len1 != len2) return FALSE;
	if (len1 == 0) return TRUE;

	node1 = root1;
	while (node1) {
		node2 = root2;
		while (node2) {
			if (iso_nodes(node1, node2, kids, 0))
				break;
			node2 = nsibling(node2);
		}
		if (!node2) return FALSE;
		node1 = nsibling(node1);
	}
	return TRUE;
}
