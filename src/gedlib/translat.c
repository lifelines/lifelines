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
/*===========================================================
 * translat.c -- LifeLines character mapping functions
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 17 Jun 94    3.0.2 - 11 Nov 94
 *=========================================================*/

#include "standard.h"
#include "translat.h"

#ifdef max
#	undef max
#endif

XNODE create_xnode();
XNODE step_xnode();

/*=============================================
 * create_trantable -- Create translation table
 *===========================================*/
TRANTABLE create_trantable (lefts, rights, n)
STRING *lefts;	/* patterns */
STRING *rights;	/* replacements */
INT n;		/* num pairs */
{
	TRANTABLE tt = (TRANTABLE) stdalloc(sizeof(*tt));
	STRING left, right;
	INT i, c;
	XNODE node;
	for (i = 0; i < 256; i++)
		tt->start[i] = NULL;
	ASSERT(n > 0);
	for (i = 0; i < n; i++) {
		left = lefts[i];
		right = rights[i];
		ASSERT(left && *left && right);
		c = (unsigned char) *left++;
		if (tt->start[c] == NULL)
			tt->start[c] = create_xnode(NULL, c, NULL);
		node = tt->start[c];
		while (c = (unsigned char) *left++) {
			node = step_xnode(node, c);
		}
		node->count = strlen(right);
		node->replace = right;
	}
	return tt;
}
/*=============================
 * create_xnode -- Create XNODE
 *===========================*/
XNODE create_xnode (parent, achar, string)
XNODE parent;
INT achar;
STRING string;
{
	XNODE node = (XNODE) stdalloc(sizeof(*node));
	node->parent = parent;
	node->sibling = NULL;
	node->child = NULL;
	node->achar = achar;
	node->replace = string;
	node->count = string ? strlen(string) : 0;
#if 0
	if (string) {
		node->count = strlen(string);
		node->replace = string;
	} else {
		node->count = 0;
		node->replace = NULL;
	}
#endif
	return node;
}
/*==========================================
 * step_xnode -- Step to node from character
 *========================================*/
XNODE step_xnode (node, achar)
XNODE node;
INT achar;
{
	XNODE prev, node0 = node;
	if (node->child == NULL)
		return node->child = create_xnode(node0, achar, NULL);
	prev = NULL;
	node = node->child;
	while (node) {
		if (node->achar == achar) return node;
		prev = node;
		node = node->sibling;
	}
	return prev->sibling = create_xnode(node0, achar, NULL);
}
/*=============================================
 * remove_trantable -- Remove translation table
 *===========================================*/
remove_trantable (tt)
TRANTABLE tt;
{
	INT i;
	if (!tt) return;
	for (i = 0; i < 256; i++)
		remove_xnodes(tt->start[i]);
	stdfree(tt);
}
/*====================================
 * remove_xnodes -- Remove xnodes tree
 *==================================*/
remove_xnodes (node)
XNODE node;
{
	if (!node) return;
	remove_xnodes(node->child);
	remove_xnodes(node->sibling);
	if (node->replace) stdfree(node->replace);
	stdfree(node);
}
/*===================================================
 * translate_string -- Translate string via TRANTABLE
 *=================================================*/
BOOLEAN translate_string (tt, in, out, max)
TRANTABLE tt;	/* tran table */
STRING in;	/* in string */
STRING out;	/* out string */
INT max;	/* max len of out string */
{
	STRING p, q, r;
	STRING add;
	INT n, l, depth, nxtch;
	XNODE node, cnode;
	*out = 0;
	if (!in) return TRUE;
	if (!tt) {
		strcpy(out, in);
		return TRUE;
	}
	p = q = in;
	r = out;
	n = strlen(in);
	l = 0;
	while (n > 0) {
		node = tt->start[*p];
		if (!node) {	/* this char starts no patterns */
			add_char(out, &l, max, *p++);
			--n;
		} else {
			q = p;	/* 1-width window */
			depth = 1;
/* Match as far as possible */
			while (n > 1 && node->child) {
				nxtch = *(q + 1);
				cnode = node->child;
				while (cnode && cnode->achar != nxtch)
					cnode = cnode->sibling;
				if (!cnode) break;
				node = cnode;
				depth++;
				--n;
				q++;
			}
/* Output replacement string */
			while (TRUE) {
				if (add = node->replace) {
					add_string(out, &l, max, add);
					p = q = q + 1;
					n = strlen(p);
					break;
				}
				if (node->parent) {
					node = node->parent;
					depth--;
					--q;
					n++;
					continue;
				}
				ASSERT(depth==1);
				add_char(out, &l, max, *p++);
				--n;
				break;
			}
		}
	}
	add_char(out, &l, max, 0);
	return TRUE;
}
/*======================================
 * add_char -- Add char to output string
 *====================================*/
add_char (buf, plen, max, achar)
STRING buf;
INT *plen, max;
INT achar;
{
	if (*plen >= max - 1)
		buf[*plen] = 0;
	else
		buf[(*plen)++] = achar;
}
/*==========================================
 * add_string -- Add string to output string
 *========================================*/
add_string (buf, plen, max, str)
STRING buf;
INT *plen, max;
STRING str;
{
	INT len;
	ASSERT(str);
	len = strlen(str);
	if (*plen + len >= max - 1)
		buf[*plen] = 0;
	else {
		strncpy(buf + *plen, str, len);
		*plen += len;
	}
}
/*=======================================================
 * show_trantable -- DEBUG routine that shows a TRANTABLE
 *=====================================================*/
void show_trantable (tt)
TRANTABLE tt;
{
	INT i;
	XNODE node;
	if (tt == NULL) {
		wprintf("EMPTY TABLE\n");
		return;
	}
	for (i = 0; i < 256; i++) {
		node = tt->start[i];
		if (node) {
			show_xnodes(0, node);
		}
	}
}
/*===============================================
 * show_xnodes -- DEBUG routine that shows XNODEs
 *=============================================*/
show_xnodes (indent, node)
INT indent;
XNODE node;
{
	INT i;
	if (!node) return;
	for (i = 0; i < indent; i++)
		wprintf("  ");
	show_xnode(node);
	show_xnodes(indent+1, node->child);
	show_xnodes(indent,   node->sibling);
}
/*================================================
 * show_xnodes -- DEBUG routine that shows 1 XNODE
 *==============================================*/
show_xnode (node)
XNODE node;
{
	wprintf("%d(%c)", node->achar, node->achar);
	if (node->replace) {
		if (node->count)
			wprintf(" \"%s\"\n", node->replace);
		else
			wprintf(" \"\"\n");
	} else
		wprintf("\n");
}
