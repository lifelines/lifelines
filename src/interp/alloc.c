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
 * alloc.c -- Allocate nodes for report generator
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 17 Aug 93
 *   3.0.0 - 28 Jun 94    3.0.2 - 23 Dec 94
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "interp.h"

extern STRING ierror;
extern STRING Pfname;

/*==================================
 * alloc_interp -- Alloc INTERP node
 *================================*/
INTERP alloc_interp (type)
char type;
{
	INTERP node = (INTERP) stdalloc(sizeof(*node));
	itype(node) = type;
	iprnt(node) = NULL;
	inext(node) = NULL;
	iline(node) = Plineno;
	node->i_word1 = node->i_word2 = node->i_word3 = NULL;
	node->i_word4 = node->i_word5 = NULL;
	return node;
}
/*====================================
 * literal_node -- Create literal node
 *==================================*/
INTERP literal_node (str)
STRING str;
{
	INTERP node = alloc_interp(ILITERAL);
	iliteral(node) = (WORD) strsave(str);
	return node;
}
/*========================================
 * children_node -- Create child loop node
 *======================================*/
INTERP children_node (fexpr, cvar, nvar, body)
INTERP fexpr;		/* expr */
STRING cvar, nvar;	/* child, counter */
INTERP body;		/* loop body */
{
	INTERP node = alloc_interp(ICHILDREN);
	ifamily(node) = (WORD) fexpr;
	ichild(node) = (WORD) cvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*========================================
 * spouses_node -- Create spouse loop node
 *======================================*/
INTERP spouses_node (pexpr, svar, fvar, nvar, body)
INTERP pexpr;	/* expr */
STRING svar;	/* spouse */
STRING fvar;	/* family */
STRING nvar;	/* counter */
INTERP body;	/* body */
{
	INTERP node = alloc_interp(ISPOUSES);
	iprinc(node) = (WORD) pexpr;
	ispouse(node) = (WORD) svar;
	ifamvar(node) = (WORD) fvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=========================================
 * families_node -- Create family loop node
 *=======================================*/
INTERP families_node (pexpr, fvar, svar, nvar, body)
INTERP pexpr;	/* expr */
STRING fvar;	/* family */
STRING svar;	/* spouse */
STRING nvar;	/* counter */
INTERP body;	/* body */
{
	INTERP node = alloc_interp(IFAMILIES);
	iprinc(node) = (WORD) pexpr;
	ifamvar(node) = (WORD) fvar;
	ispouse(node) = (WORD) svar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*==========================================
 * forindiset_node -- Create index loop node
 *========================================*/
INTERP forindiset_node (iexpr, ivar, vvar, nvar, body)
INTERP iexpr;	/* expr */
STRING ivar;	/* person */
STRING vvar;	/* value */
STRING nvar;	/* counter */
INTERP body;	/* body */
{
	INTERP node = alloc_interp(IINDICES);
	iindex(node) = (WORD) iexpr;
	iindivar(node) = (WORD) ivar;
	ivalvar(node) = (WORD) vvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*======================================
 * forlist_node -- Create list loop node
 *====================================*/
INTERP forlist_node (iexpr, evar, nvar, body)
INTERP iexpr;	/* expr */
STRING evar;	/* element */
STRING nvar;	/* counter */
INTERP body;	/* body */
{
	INTERP node = alloc_interp(ILIST);
	ilist(node) = (WORD) iexpr;
	ielement(node) = (WORD) evar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=========================================
 * forindi_node -- Create forindi loop node
 *=======================================*/
INTERP forindi_node (ivar, nvar, body)
STRING ivar;	/* pers */
STRING nvar;	/* counter */
INTERP body;	/* body */
{
	INTERP node = alloc_interp(IINDI);
	iindivar(node) = (WORD) ivar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=======================================
 * forfam_node -- Create forfam loop node
 *=====================================*/
INTERP forfam_node (fvar, nvar, body)
STRING fvar;	/* fam */
STRING nvar;	/* counter */
INTERP body;	/* body */
{
	INTERP node = alloc_interp(IFAM);
	iindivar(node) = (WORD) fvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*===========================================
 * fornotes_node -- Create fornotes loop node
 *=========================================*/
INTERP fornotes_node (nexpr, vvar, body)
INTERP nexpr;	/* expr */
STRING vvar;	/* value */
INTERP body;	/* body */
{
	INTERP node = alloc_interp(INOTES);
	inode(node) = (WORD) nexpr;
	istrng(node) = (WORD) vvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*===========================================
 * fornodes_node -- Create fornodes loop node
 *=========================================*/
INTERP fornodes_node (nexpr, nvar, body)
INTERP nexpr;	/* expr */
STRING nvar;	/* node (next level) */
INTERP body;	/* body */
{
	INTERP node = alloc_interp(INODES);
	inode(node) = (WORD) nexpr;
	isubnode(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*===========================================
 * traverse_node -- Create traverse loop node
 *=========================================*/
INTERP traverse_node (nexpr, snode, levv, body)
INTERP nexpr;	/* node */
STRING snode;	/* subnode */
STRING levv;	/* level */
INTERP body;	/* body */
{
	INTERP node = alloc_interp(ITRAV);
	inode(node) = (WORD) nexpr;
	isubnode(node) = (WORD) snode;
	ilev(node) = (WORD) levv;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*====================================
 * iden_node -- Create identifier node
 *==================================*/
INTERP iden_node (iden)
STRING iden;
{
	INTERP node = alloc_interp(IIDENT);
	iident(node) = (WORD) iden;
	return node;
}
/*==================================
 * icons_node -- Create integer node
 *================================*/
INTERP icons_node (ival)
INT ival;
{
	INTERP node = alloc_interp(IICONS);
	iicons(node) = (WORD) ival;
	return node;
}
/*===================================
 * proc_node -- Create procedure node
 *=================================*/
INTERP proc_node (name, parms, body)
STRING name;	/* proc name */
INTERP parms;	/* param/s */
INTERP body;	/* body */
{
	INTERP node = alloc_interp(IPDEFN);
	iname(node) = (WORD) name;
	iparams(node) = (WORD) parms;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*==================================================
 * fdef_node -- Create user function definition node
 *================================================*/
INTERP fdef_node (name, parms, body)
STRING name;	/* proc name */
INTERP parms;	/* param/s */
INTERP body;	/* body */
{
	INTERP node = alloc_interp(IFDEFN);
	iname(node) = (WORD) name;
	iparams(node) = (WORD) parms;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=======================================================
 * func_node -- Create builtin or user function call node
 *=====================================================*/
INTERP func_node (name, elist)
STRING name;	/* function name */
INTERP elist;	/* param/s */
{
	INTERP node;
	INT lo, hi, md, n, r;
	BOOLEAN found = FALSE;

/* See if the function is user defined */
	if (in_table(functab, name)) {
		node = alloc_interp(IFCALL);
		iname(node) = (WORD) name;
		ielist(node) = (WORD) elist;
		ifunc(node) = (WORD) valueof(functab, name);
		return node;
	}

/* See if the function is builtin */
	lo = 0;
	hi = nobuiltins - 1;
	while (lo <= hi) {
		md = (lo + hi) >> 1;
		if ((r = nestr(name, builtins[md].ft_name)) < 0)
			hi = md - 1;
		else if (r > 0)
			lo = md + 1;
		else {
			found = TRUE;
			break;
		}
	}
	if (found) {
		if ((n = num_params(elist)) < builtins[md].ft_nparms_min
		    && n > builtins[md].ft_nparms_max) {
			wprintf(ierror, Pfname, Plineno);
			wprintf("%s: must have %d to %d parameters.\n", name,
		    	builtins[md].ft_nparms_min, builtins[md].ft_nparms_max);
			Perrors++;
		}
		node = alloc_interp(IBCALL);
		iname(node) = (WORD) name;
		ielist(node) = (WORD) elist;
		ifunc(node) = (WORD) builtins[md].ft_eval;
		return node;
		
	}

/* If neither make it a user call to undefined function */
	node = alloc_interp(IFCALL);
	iname(node) = (WORD) name;
	ielist(node) = (WORD) elist;
	ifunc(node) = NULL;
	return node;
}
/*=============================
 * if_node -- Create an if node
 *===========================*/
INTERP if_node (cond, tnode, enode)
INTERP cond;	/* cond expr */
INTERP tnode;	/* then */
INTERP enode;	/* else */
{
	INTERP node = alloc_interp(IIF);
	icond(node) = (WORD) cond;
	ithen(node) = (WORD) tnode;
	ielse(node) = (WORD) enode;
	set_parents(tnode, node);
	set_parents(enode, node);
	return node;
}
/*================================
 * while_node -- Create while node
 *==============================*/
INTERP while_node (cond, body)
INTERP cond;	/* cond expr */
INTERP body;	/* body */
{
	INTERP node = alloc_interp(IWHILE);
	icond(node) = (WORD) cond;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*===================================
 * call_node -- Create proc call node
 *=================================*/
INTERP call_node (name, args)
STRING name;	/* proc name */
INTERP args;	/* arg/s */
{
	INTERP node = alloc_interp(IPCALL);
	iname(node) = (WORD) name;
	iargs(node) = (WORD) args;
	return node;
}
/*================================
 * break_node -- Create break node
 *==============================*/
INTERP break_node ()
{
	INTERP node = alloc_interp(IBREAK);
	return node;
}
/*======================================
 * continue_node -- Create continue node
 *====================================*/
INTERP continue_node ()
{
	INTERP node = alloc_interp(ICONTINUE);
	return node;
}
/*==================================
 * return_node -- Create return node
 *================================*/
INTERP return_node (args)
INTERP args;
{
	INTERP node = alloc_interp(IRETURN);
	iargs(node) = (WORD) args;
	return node;
}
/*==============================================
 * set_parents -- Link body nodes to parent node
 *============================================*/
set_parents (body, node)
INTERP body;
INTERP node;
{
	while (body) {
		iprnt(body) = node;
		body = inext(body);
	}
}
