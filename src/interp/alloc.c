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
 * pnodes are parse nodes in the parse tree built by yacc.c
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *===========================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "sys_inc.h"
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interpi.h"
#include "feedback.h"
#include "liflines.h"
#include "codesets.h"
#include "zstr.h"
#include "vtable.h"

/*********************************************
 * global/exported variables
 *********************************************/

/* reused report language error strings */
STRING nonint1     = 0;
STRING nonintx     = 0;
STRING nonflox     = 0;
STRING nonstr1     = 0;
STRING nonstrx     = 0;
STRING nullarg1    = 0;
STRING nonfname1   = 0;
STRING nonnodstr1  = 0;
STRING nonind1     = 0;
STRING nonindx     = 0;
STRING nonfam1     = 0;
STRING nonfamx     = 0;
STRING nonrecx     = 0;
STRING nonnod1     = 0;
STRING nonnodx     = 0;
STRING nonvar1     = 0;
STRING nonvarx     = 0;
STRING nontabx     = 0;
STRING nonset1     = 0;
STRING nonsetx     = 0;
STRING nonboox     = 0;
STRING nonlst1     = 0;
STRING nonlstx     = 0;
STRING badargs     = 0;
STRING badarg1     = 0;
STRING badargx     = 0;
STRING badtrig     = 0;

/*********************************************
 * external/imported variables
 *********************************************/



/*********************************************
 * local types
 *********************************************/

struct tag_pn_block
{
	struct tag_pn_block * next;
	struct tag_pnode nodes[100]; /* arbitrary size may be adjusted */
};
typedef struct tag_pn_block *PN_BLOCK;
#define BLOCK_NODES (sizeof(((PN_BLOCK)0)->nodes)/sizeof(((PN_BLOCK)0)->nodes[0]))

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static PNODE alloc_pnode_memory(void);
static void clear_error_strings(void);
static void clear_pnode(PNODE node);
static PNODE create_pnode(PACTX pactx, INT type);
static void delete_pnode(PNODE node);
static void describe_pnodes(PNODE node, ZSTR zstr, INT max);
static void free_pnode_memory(PNODE node);
static void rptinfo_destructor(VTABLE *obj);
static void set_parents(PNODE body, PNODE node);
static void verify_builtins(void);

/*********************************************
 * local variables
 *********************************************/

static PN_BLOCK block_list = 0;
static PNODE free_list = 0;
static STRING interp_locale = 0;
static INT live_pnodes = 0;
static TABLE f_rptinfos=0;

static struct tag_vtable vtable_for_rptinfo = {
	VTABLE_MAGIC
	, "rptinfo"
	, &rptinfo_destructor
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

/*========================================
 * alloc_pnode_memory -- return new pnode memory
 * We use a custom allocator, which lowers our overhead
 * (no heap overhead per pnode, only per block)
 * and also allows us to clean them all up after the
 * report.
 * NB: This is not traditional garbage collection - we're
 *  not doing any live/dead analysis; we depend entirely
 *  on carnal knowledge of the program.
 * As far as I know, no pnodes were ever being freed before.
 * Perry Rapp, 2001/01/20
 *======================================*/
static PNODE
alloc_pnode_memory (void)
{
	PNODE node;
	
	/*
	This assumes that all pnodes are scoped
	within report processing. If this ceases to
	be true, this breaks.
	*/
	if (!free_list) {
		PN_BLOCK new_block = stdalloc(sizeof(*new_block));
		INT i;
		new_block->next = block_list;
		block_list = new_block;
		for (i=0; i<(INT)BLOCK_NODES; i++) {
			PNODE node1 = &new_block->nodes[i];
			itype(node1) = IFREED;
			inext(node1) = free_list;
			free_list = node1;
		}
	}
	node = free_list;
	free_list = inext(node);
	live_pnodes++;
	return node;
}
/*========================================
 * free_pnode_memory -- return pnode to free-list
 * (see alloc_pnode_memory comments)
 * Created: 2001/01/21, Perry Rapp
 *======================================*/
static void
free_pnode_memory (PNODE node)
{
	/* put on free list */
	inext(node) = free_list;
	free_list = node;
	live_pnodes--;
	ASSERT(live_pnodes>=0);
}
/*======================================
 * free_all_pnodes -- Free every pnode
 * Created: 2001/01/21, Perry Rapp
 *====================================*/
void
free_all_pnodes (void)
{
	PN_BLOCK block;
	while ((block = block_list)) {
		PN_BLOCK next = block->next;
		free_list = 0;
		if (live_pnodes) {
			INT i;
			for (i=0; i<(INT)BLOCK_NODES; i++) {
				PNODE node1=&block->nodes[i];
				if (itype(node1) != IFREED) {
					delete_pnode(node1);
				}
			}
		}
		stdfree(block);
		block_list = next;
	}
	free_list = 0;
}
/*==================================
 * create_pnode -- Create PNODE node
 * 2001/01/21 changed to block allocator
 *================================*/
static PNODE
create_pnode (PACTX pactx, INT type)
{
	PNODE node = alloc_pnode_memory();
	itype(node) = type;
	iprnt(node) = NULL;
	inext(node) = NULL;
	iline(node) = pactx->lineno;
	/* Assumption -- pactx->fullpath stays live longer than all pnodes */
	irptinfo(node) = get_rptinfo(pactx->fullpath);
	node->i_word1 = node->i_word2 = node->i_word3 = NULL;
	node->i_word4 = node->i_word5 = NULL;
	return node;
}
/*========================================
 * clear_pnode -- Empty contents of pvalue
 * Created: 2001/01/20, Perry Rapp
 *======================================*/
static void
clear_pnode (PNODE node)
{
	if (node->i_flags & PN_IVALUEX_PVALUE) {
		PVALUE val = ivaluex(node);
		if (val) {
			delete_pvalue(val);
			ivaluex(node) = 0;
		}
	}
	if (node->i_flags & PN_INAME_HSTR) {
		STRING str = iname(node);
		if (str) {
			stdfree(str);
			iname(node) = 0;
		}
	}
	if (node->i_flags & PN_ICHILD_HPTR) {
		STRING str = ichild(node);
		if (str) {
			stdfree(str);
			ichild(node) = 0;
		}
	}
	if (node->i_flags & PN_INUM_HPTR) {
		STRING str = inum(node);
		if (str) {
			stdfree(str);
			inum(node) = 0;
		}
	}
	if (node->i_flags & PN_ISPOUSE_HPTR) {
		STRING str = ispouse(node);
		if (str) {
			stdfree(str);
			ispouse(node) = 0;
		}
	}
	if (node->i_flags & PN_IFAMILY_HPTR) {
		STRING str = ifamily(node);
		if (str) {
			stdfree(str);
			ifamily(node) = 0;
		}
	}
	if (node->i_flags & PN_IELEMENT_HPTR) {
		STRING str = ielement(node);
		if (str) {
			stdfree(str);
			ielement(node) = 0;
		}
	}
	if (node->i_flags & PN_IPARENT_HPTR) {
		STRING str = iiparent(node);
		if (str) {
			stdfree(str);
			iiparent(node) = 0;
		}
	}
	if (node->i_flags & PN_IVALVAR_HPTR) {
		STRING str = ivalvar(node);
		if (str) {
			stdfree(str);
			ivalvar(node) = 0;
		}
	}
	if (node->i_flags & PN_IIDENT_HSTR) {
		STRING str = iident(node);
		if (str) {
			stdfree(str);
			iident(node) = 0;
		}
	}

}
/*==================================
 * delete_pnode -- Create PNODE node
 * Created: 2001/01/21, Perry Rapp
 *================================*/
static void
delete_pnode (PNODE node)
{
	if (!node) return;
	clear_pnode(node);
	free_pnode_memory(node);
}
/*==================================
 * string_node -- Create string node
 *  We copy the string memory.
 *================================*/
PNODE
string_node (PACTX pactx, STRING str)
{
	PNODE node = create_pnode(pactx, ISCONS);
	ASSERT(str); /* we're not converting NULL to "" because nobody passes us NULL */
	ivaluex(node) = create_pvalue_from_string(str);
	node->i_flags = PN_IVALUEX_PVALUE;
	return node;
}
/*========================================
 * children_node -- Create child loop node
 *  pactx: [I/O] pointer to parseinfo structure (parse globals)
 *  fexpr: [IN]  expr
 *  cvar:  [IN]  child
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *======================================*/
PNODE
children_node (PACTX pactx, PNODE fexpr, STRING cvar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, ICHILDREN);
	iloopexp(node) = (VPTR) fexpr;
	ichild(node) = (VPTR) cvar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_ICHILD_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*========================================
 * familyspouses_node -- Create parent loop node
 *  pactx: [I/O] pointer to parseinfo structure (parse globals)
 *  fexpr: [IN]  expr
 *  cvar:  [IN]  child
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *======================================*/
PNODE
familyspouses_node (PACTX pactx, PNODE fexpr, STRING cvar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, IFAMILYSPOUSES);
	iloopexp(node) = (VPTR) fexpr;
	iiparent(node) = (VPTR) cvar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IPARENT_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*========================================
 * spouses_node -- Create spouse loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  pexpr: [IN]  expr
 *  svar:  [IN]  spouse
 *  fvar:  [IN]  family
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *======================================*/
PNODE
spouses_node (PACTX pactx, PNODE pexpr, STRING svar, STRING fvar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, ISPOUSES);
	iloopexp(node) = (VPTR) pexpr;
	ispouse(node) = (VPTR) svar;
	ifamily(node) = (VPTR) fvar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_ISPOUSE_HPTR + PN_IFAMILY_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*=========================================
 * families_node -- Create family loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  pexpr: [IN]  expr
 *  fvar:  [IN]  family
 *  svar:  [IN]  spouse
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *=======================================*/
PNODE
families_node (PACTX pactx, PNODE pexpr, STRING fvar, STRING svar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, IFAMILIES);
	iloopexp(node) = (VPTR) pexpr;
	ifamily(node) = (VPTR) fvar;
	ispouse(node) = (VPTR) svar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IFAMILY_HPTR + PN_ISPOUSE_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*=========================================
 * fathers_node -- Create fathers loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  pexpr, [IN]  expression
 *  pvar:  [IN]  father
 *  fvar:  [IN]  family
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *=======================================*/
PNODE
fathers_node (PACTX pactx, PNODE pexpr, STRING pvar, STRING fvar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, IFATHS);
	iloopexp(node) = (VPTR) pexpr;
	iiparent(node) = (VPTR) pvar;
	ifamily(node) = (VPTR) fvar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IPARENT_HPTR + PN_IFAMILY_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*=========================================
 * mothers_node -- Create mothers loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  pexpr, [IN]  expression
 *  pvar:  [IN]  mother
 *  fvar:  [IN]  family
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *=======================================*/
PNODE
mothers_node (PACTX pactx, PNODE pexpr, STRING pvar, STRING fvar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, IMOTHS);
	iloopexp(node) = (VPTR) pexpr;
	iiparent(node) = (VPTR) pvar;
	ifamily(node) = (VPTR) fvar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IPARENT_HPTR + PN_IFAMILY_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*=========================================
 * parents_node -- Create parents loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  pexpr, [IN]  expression
 *  fvar:  [IN]  family
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *=======================================*/
PNODE
parents_node (PACTX pactx, PNODE pexpr, STRING fvar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, IFAMCS);
	iloopexp(node) = (VPTR) pexpr;
	ifamily(node) = (VPTR) fvar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IFAMILY_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*========================================
 * forindiset_node -- Create set loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  iexpr, [IN]  expression
 *  ivar:  [IN]  person
 *  vvar:  [IN]  value
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *======================================*/
PNODE
forindiset_node (PACTX pactx, PNODE iexpr, STRING ivar, STRING vvar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, ISET);
	iloopexp(node) = (VPTR) iexpr;
	ielement(node) = (VPTR) ivar;
	ivalvar(node) = (VPTR) vvar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IELEMENT_HPTR + PN_IVALVAR_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*======================================
 * forlist_node -- Create list loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  iexpr, [IN]  expression
 *  evar:  [IN]  element
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *====================================*/
PNODE
forlist_node (PACTX pactx, PNODE iexpr, STRING evar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, ILIST);
	iloopexp(node) = (VPTR) iexpr;
	ielement(node) = (VPTR) evar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IELEMENT_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*=========================================
 * forindi_node -- Create forindi loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  ivar,  [IN]  person
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *=======================================*/
PNODE
forindi_node (PACTX pactx, STRING ivar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, IINDI);
	ielement(node) = (VPTR) ivar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IELEMENT_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*=========================================
 * forsour_node -- Create forsour loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  svar,  [IN]  source
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *=======================================*/
PNODE
forsour_node (PACTX pactx, STRING svar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, ISOUR);
	ielement(node) = (VPTR) svar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IELEMENT_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*=========================================
 * foreven_node -- Create foreven loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  evar,  [IN]  event
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *=======================================*/
PNODE
foreven_node (PACTX pactx, STRING evar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, IEVEN);
	ielement(node) = (VPTR) evar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IELEMENT_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*=========================================
 * forothr_node -- Create forothr loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  ovar,  [IN]  other record
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *=======================================*/
PNODE
forothr_node (PACTX pactx, STRING ovar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, IOTHR);
	ielement(node) = (VPTR) ovar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IELEMENT_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*=======================================
 * forfam_node -- Create forfam loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  fvar,  [IN]  family
 *  nvar:  [IN]  counter
 *  body:  [IN]  loop body statements
 *=====================================*/
PNODE
forfam_node (PACTX pactx, STRING fvar, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, IFAM);
	ielement(node) = (VPTR) fvar;
	inum(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IELEMENT_HPTR + PN_INUM_HPTR;
	set_parents(body, node);
	return node;
}
/*===========================================
 * fornotes_node -- Create fornotes loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  nexpr: [IN]  expression
 *  vvar:  [IN]  value
 *  body:  [IN]  loop body statements
 *=========================================*/
PNODE
fornotes_node (PACTX pactx, PNODE nexpr, STRING vvar, PNODE body)
{
	PNODE node = create_pnode(pactx, INOTES);
	iloopexp(node) = (VPTR) nexpr;
	ielement(node) = (VPTR) vvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IELEMENT_HPTR;
	set_parents(body, node);
	return node;
}
/*===========================================
 * fornodes_node -- Create fornodes loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  nexpr: [IN]  expression
 *  vvar:  [IN]  node (next level)
 *  body:  [IN]  loop body statements
 *=========================================*/
PNODE
fornodes_node (PACTX pactx, PNODE nexpr, STRING nvar, PNODE body)
{
	PNODE node = create_pnode(pactx, INODES);
	iloopexp(node) = (VPTR) nexpr;
	ielement(node) = (VPTR) nvar;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IELEMENT_HPTR;
	set_parents(body, node);
	return node;
}
/*===========================================
 * traverse_node -- Create traverse loop node
 *  pactx: [IN]  pointer to parseinfo structure (parse globals)
 *  nexpr: [IN]  node
 *  snode: [IN]  subnode
 *  levv:  [IN]  level
 *  body:  [IN]  loop body statements
 *=========================================*/
PNODE
traverse_node (PACTX pactx, PNODE nexpr, STRING snode, STRING levv, PNODE body)
{
	PNODE node = create_pnode(pactx, ITRAV);
	iloopexp(node) = (VPTR) nexpr;
	ielement(node) = (VPTR) snode;
	ilev(node) = (VPTR) levv;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_IELEMENT_HPTR;
	set_parents(body, node);
	return node;
}
/*====================================
 * iden_node -- Create identifier node
 *==================================*/
PNODE
iden_node (PACTX pactx, STRING iden)
{
	PNODE node = create_pnode(pactx, IIDENT);
	iident(node) = (VPTR) iden;
	node->i_flags = PN_IIDENT_HSTR;
	return node;
}
/*==================================
 * icons_node -- Create integer node
 *================================*/
PNODE
icons_node (PACTX pactx, INT ival)
{
	PNODE node = create_pnode(pactx, IICONS);
	ivaluex(node) = create_pvalue_from_int(ival);
	node->i_flags = PN_IVALUEX_PVALUE;
	return node;
}
/*===================================
 * fcons_node -- Create floating node
 *=================================*/
PNODE
fcons_node (PACTX pactx, FLOAT fval)
{
	PNODE node = create_pnode(pactx, IFCONS);
	ivaluex(node) = create_pvalue_from_float(fval);
	node->i_flags = PN_IVALUEX_PVALUE;
	return node;
}
/*===================================
 * proc_node -- Create procedure node
 *  pactx: [I/O] pointer to parseinfo structure (parse globals)
 *  name:  [IN]  proc name
 *  parms: [IN]  param/s
 *  body:  [IN]  body
 *=================================*/
PNODE
proc_node (PACTX pactx, CNSTRING name, PNODE parms, PNODE body)
{
	PNODE node = create_pnode(pactx, IPDEFN);
	iname(node) = (VPTR) name;
	iargs(node) = (VPTR) parms;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_INAME_HSTR;
	set_parents(body, node);
	return node;
}
/*==================================================
 * fdef_node -- Create user function definition node
 *  pactx: [I/O] pointer to parseinfo structure (parse globals)
 *  name:  [IN]  proc name
 *  parms: [IN]  param/s
 *  body:  [IN]  body
 *================================================*/
PNODE
fdef_node (PACTX pactx, CNSTRING name, PNODE parms, PNODE body)
{
	PNODE node = create_pnode(pactx, IFDEFN);
	iname(node) = (VPTR) name;
	iargs(node) = (VPTR) parms;
	ibody(node) = (VPTR) body;
	node->i_flags = PN_INAME_HSTR;
	set_parents(body, node);
	return node;
}
/*=======================================================
 * func_node -- Create builtin or user function call node
 *  pactx: [I/O] pointer to parseinfo structure (parse globals)
 *  name:  [IN]  function name
 *  elist: [IN]  param(s)
 * consumes name heap pointer
 *=====================================================*/
PNODE
func_node (PACTX pactx, STRING name, PNODE elist)
{
	PNODE node, func;
	INT lo, hi, md=0, n, r;
	BOOLEAN found = FALSE;
	INT count;

/* See if the function is user defined */
	/* find func in local or global table */
	func = get_proc_node(name, get_rptinfo(pactx->fullpath)->functab, gfunctab, &count);
	if (func) {
		node = create_pnode(pactx, IFCALL);
		iname(node) = (VPTR) name;
		iargs(node) = (VPTR) elist;
		node->i_flags = PN_INAME_HSTR;
		ifunc(node) = func;
		return node;
	} else if (count) {
		/* ambiguous call */
		goto func_node_bad;
	}

/*
	See if the function is builtin
	Assume that builtins[] is in alphabetic order
	and do binary search on it
*/
	
	lo = 0;
	hi = nobuiltins - 1;
	while (lo <= hi) {
		md = (lo + hi) >> 1;
		if ((r = cmpstr(name, builtins[md].ft_name)) < 0)
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
		    || n > builtins[md].ft_nparms_max) {
			llwprintf(_("Error: file \"%s\": line %d: "), pactx->ifile, pactx->lineno);
			llwprintf("%s: must have %d to %d parameters (found with %d).\n"
				, name, builtins[md].ft_nparms_min, builtins[md].ft_nparms_max
				, n);
			Perrors++;
		}
		node = create_pnode(pactx, IBCALL);
		iname(node) = (VPTR) name;
		iargs(node) = (VPTR) elist;
		ifunc(node) = (VPTR) builtins[md].ft_eval;
		node->i_flags = PN_INAME_HSTR;
		return node;
		
	}

/* If neither make it a user call to undefined function */
func_node_bad:
	node = create_pnode(pactx, IFCALL);
	iname(node) = (VPTR) name;
	iargs(node) = (VPTR) elist;
	ifunc(node) = NULL;
	return node;
}
/*=============================
 * init_interpreter -- any initialization needed by
 *  interpreter at program startup
 * Created: 2001/06/10, Perry Rapp
 *===========================*/
void
init_interpreter (void)
{
	verify_builtins();
}
/*=============================
 * clear_error_strings -- Free all error strings
 *  Used at end, and also if language change needed
 *===========================*/
static void
clear_error_strings (void)
{
	strfree(&nonint1);
	strfree(&nonintx);
	strfree(&nonflox);
	strfree(&nonstr1);
	strfree(&nonstrx);
	strfree(&nullarg1);
	strfree(&nonfname1);
	strfree(&nonnodstr1);
	strfree(&nonind1);
	strfree(&nonindx);
	strfree(&nonfam1);
	strfree(&nonfamx);
	strfree(&nonrecx);
	strfree(&nonnod1);
	strfree(&nonnodx);
	strfree(&nonvar1);
	strfree(&nonvarx);
	strfree(&nonboox);
	strfree(&nonlst1);
	strfree(&nonlstx);
	strfree(&nontabx);
	strfree(&nonset1);
	strfree(&nonsetx);
	strfree(&badargs);
	strfree(&badarg1);
	strfree(&badargx);
	strfree(&badtrig);
	strfree(&interp_locale);
}
/*=============================
 * shutdown_interpreter -- shutdown code for
 *  interpreter at program end
 * Created: 2002/02/16, Perry Rapp
 *===========================*/
void
shutdown_interpreter (void)
{
	clear_error_strings();
}
/*=============================
 * interp_load_lang -- Load the common
 *  error msgs for current locale
 * These are used by many report functions.
 * This avoids having to localize dozens of strings
 * just like these.
 * Created: 2002/02/16, Perry Rapp
 *===========================*/
void
interp_load_lang (void)
{
#ifdef HAVE_SETLOCALE
	STRING cur_locale = setlocale(LC_COLLATE, NULL);
	if (interp_locale) {
		/* using LC_COLLATE because Win32 lacks LC_MESSAGES */
		if (eqstr(interp_locale, cur_locale))
			return;
		stdfree(interp_locale);
	}
	interp_locale = strsave(cur_locale);
#else
	if (interp_locale)
		return;
	interp_locale = strsave("C");
#endif
	clear_error_strings();
	nonint1     = strsave(_("%s: the arg must be an integer."));
	nonintx     = strsave(_("%s: the arg #%s must be an integer."));
	nonflox     = strsave(_("%s: the arg #%s must be a float."));
	nonstr1     = strsave(_("%s: the arg must be a string."));
	nonstrx     = strsave(_("%s: the arg #%s must be a string."));
	nullarg1    = strsave(_("%s: null arg not permissible."));
	nonfname1   = strsave(_("%s: the arg must be a filename."));
	nonnodstr1  = strsave(_("%s: the arg must be a node or string."));
	nonind1     = strsave(_("%s: the arg must be a person."));
	nonindx     = strsave(_("%s: the arg #%s must be a person."));
	nonfam1     = strsave(_("%s: the arg must be a family."));
	nonfamx     = strsave(_("%s: the arg #%s must be a family."));
	nonrecx     = strsave(_("%s: the arg #%s must be a record."));
	nonnod1     = strsave(_("%s: the arg must be a node."));
	nonnodx     = strsave(_("%s: the arg #%s must be a node."));
	nonvar1     = strsave(_("%s: the arg must be a variable."));
	nonvarx     = strsave(_("%s: the arg #%s must be a variable."));
	nonboox     = strsave(_("%s: the arg #%s must be a boolean."));
	nonlst1     = strsave(_("%s: the arg must be a list."));
	nonlstx     = strsave(_("%s: the arg #%s must be a list."));
	nontabx     = strsave(_("%s: the arg #%s must be a table."));
	nonset1     = strsave(_("%s: the arg must be a set."));
	nonsetx     = strsave(_("%s: the arg #%s must be a set."));
	badargs     = strsave(_("%s: Bad arguments"));
	badarg1     = strsave(_("%s: the arg had a major error."));
	badargx     = strsave(_("%s: the arg #%s had a major error."));
	badtrig     = strsave(_("%s: the arg #%s would cause an arithmetic exception."));
}

/*=============================
 * verify_builtins -- check that builtins are in order
 * Created: 2001/06/10, Perry Rapp
 *===========================*/
static void
verify_builtins (void)
{
	INT i;
	for (i=0; i<nobuiltins-1; ++i) {
		if (strcmp(builtins[i].ft_name, builtins[i+1].ft_name)>0) {
			char msg[64];
			sprintf(msg, "builtins array out of order ! (entries %ld,%ld)"
				, i, i+1);
			FATAL2(msg);
		}
		if (builtins[i].ft_nparms_min > builtins[i].ft_nparms_max) {
			char msg[64];
			sprintf(msg, "builtins array bad min,max (%ld,%ld, entry %ld)"
				, builtins[i].ft_nparms_min, builtins[i].ft_nparms_max
				, i);
			FATAL2(msg);
		}
	}
}
/*=============================
 * if_node -- Create an if node
 *  pactx: [I/O] pointer to parseinfo structure (parse globals)
 *  cond:  [IN]  conditional expression governing if
 *  tnode: [IN]  then statements
 *  enode: [IN]  else statements
 *===========================*/
PNODE
if_node (PACTX pactx, PNODE cond, PNODE tnode, PNODE enode)
{
	PNODE node = create_pnode(pactx, IIF);
	icond(node) = (VPTR) cond;
	ithen(node) = (VPTR) tnode;
	ielse(node) = (VPTR) enode;
	set_parents(tnode, node);
	set_parents(enode, node);
	return node;
}
/*================================
 * while_node -- Create while node
 *  pactx: [I/O] pointer to parseinfo structure (parse globals)
 *  cond:  [IN]  conditional expression governing while loop
 *  body:  [IN]  body statements of while loop
 *==============================*/
PNODE
while_node (PACTX pactx, PNODE cond, PNODE body)
{
	PNODE node = create_pnode(pactx, IWHILE);
	icond(node) = (VPTR) cond;
	ibody(node) = (VPTR) body;
	set_parents(body, node);
	return node;
}
/*===================================
 * call_node -- Create proc call node
 *  pactx: [I/O] pointer to parseinfo structure (parse globals)
 *  name:  [IN]  procedure name
 *  args:  [IN]  argument(s)
 *=================================*/
PNODE
call_node (PACTX pactx, STRING name, PNODE args)
{
	PNODE node = create_pnode(pactx, IPCALL);
	iname(node) = (VPTR) name;
	iargs(node) = (VPTR) args;
	node->i_flags = PN_INAME_HSTR;
	return node;
}
/*================================
 * break_node -- Create break node
 *==============================*/
PNODE break_node (PACTX pactx)
{
	PNODE node = create_pnode(pactx, IBREAK);
	return node;
}
/*======================================
 * continue_node -- Create continue node
 *====================================*/
PNODE continue_node (PACTX pactx)
{
	PNODE node = create_pnode(pactx, ICONTINUE);
	return node;
}
/*==================================
 * return_node -- Create return node
 *================================*/
PNODE
return_node (PACTX pactx, PNODE args)
{
	PNODE node = create_pnode(pactx, IRETURN);
	iargs(node) = (VPTR) args;
	return node;
}
/*==============================================
 * set_parents -- Link body nodes to parent node
 *============================================*/
void
set_parents (PNODE body,
             PNODE node)
{
	while (body) {
		iprnt(body) = node;
		body = inext(body);
	}
}
/*=========================================================
 * show_pnode -- DEBUG routine that shows a PNODE structure
 *=======================================================*/
void
show_pnode (PNODE node)
{
	while (node) {
		debug_show_one_pnode(node);
		node = inext(node);
	}
}
/*==========================================================
 * show_pnodes -- DEBUG routine that shows expression PNODEs
 *========================================================*/
void
show_pnodes (PNODE node)
{

	while (node) {
		debug_show_one_pnode(node);
		node = inext(node);
		if (node) llwprintf(",");
	}
}
/*==========================================================
 * describe_pnodes -- DEBUG routine that describes expression PNODE chain
 *  into zstring
 *========================================================*/
static void
describe_pnodes (PNODE node, ZSTR zstr, INT max)
{
	while (node) {
		describe_pnode(node, zstr, max);
		node = inext(node);
		if (node)
			zs_appc(zstr, ',');
	}
}
/*====================================================
 * debug_show_one_pnode -- DEBUG routine that show one PNODE
 *==================================================*/
void
debug_show_one_pnode (PNODE node)     /* node to print */
{
	ZSTR zstr = zs_newn(512);
	INT max = 512;
	describe_pnode(node, zstr, max);
	llwprintf(zs_str(zstr));
}
/*====================================================
 * debug_show_one_pnode -- DEBUG routine to describe one node
 *  appending description into zstring
 *  but not more than max chars
 *==================================================*/
void
describe_pnode (PNODE node, ZSTR zstr, INT max)
{
	if ((INT)zs_len(zstr) >= max-2)
		return;
	if ((INT)zs_len(zstr) >= max-7) {
		if (zs_str(zstr)[zs_len(zstr)-1] != '.')
			zs_apps(zstr, "...");
		return;
	}

	switch (itype(node)) {

	case IICONS:
		zs_appf(zstr, "%d", pvalue_to_int(ivalue(node)));
		break;
	case IFCONS:
		zs_appf(zstr, "%f", pvalue_to_float(ivalue(node)));
		break;
	case ILCONS:
		zs_apps(zstr, "*ni*");
		break;
	case ISCONS:
		zs_appf(zstr, "^^%s^^", pvalue_to_string(ivalue(node)));
		break;
	case IIDENT:
		zs_appf(zstr, "%s", iident(node));
		break;
	case IIF:
		zs_apps(zstr, "if(");
		describe_pnodes(icond(node), zstr, max);
		zs_apps(zstr, "){");
		describe_pnodes(ithen(node), zstr, max);
		zs_apps(zstr, "}");
		if (ielse(node)) {
			zs_apps(zstr, "else{");
			describe_pnodes(ielse(node), zstr, max);
			zs_apps(zstr, "}");
		}
		break;
	case IWHILE:
		zs_apps(zstr, "while(");
		describe_pnodes(icond(node), zstr, max);
		zs_apps(zstr, "){");
		describe_pnodes(ibody(node), zstr, max);
		zs_apps(zstr, "}");
		break;
	case IBREAK:
		zs_apps(zstr, "break ");
		break;
	case ICONTINUE:
		zs_apps(zstr, "continue ");
		break;
	case IRETURN:
		zs_apps(zstr, "return(");
		describe_pnodes(iargs(node), zstr, max);
		zs_apps(zstr, ")");
		break;
	case IPDEFN:
		zs_apps(zstr, "*PDefn *");
		break;
	case IPCALL:
		zs_appf(zstr, "%s(", iname(node));
		describe_pnodes(iargs(node), zstr, max);
		zs_apps(zstr, ")");
		break;
	case IFDEFN:
		zs_apps(zstr, "*FDefn *");
		break;
	case IFCALL:
		zs_appf(zstr, "%s(", iname(node));
		describe_pnodes(iargs(node), zstr, max);
		zs_apps(zstr, ")");
		break;
	case IBCALL:
		zs_appf(zstr, "%s(", iname(node));
		describe_pnodes(iargs(node), zstr, max);
		zs_apps(zstr, ")");
		break;
	case ITRAV:
		zs_apps(zstr, "*Traverse *");
		break;
	case INODES:
		zs_apps(zstr, "*Fornodes *");
		break;
	case IFAMILIES:
		zs_apps(zstr, "*FamiliesLoop *");
		break;
	case ISPOUSES:
		zs_apps(zstr, "*SpousesLoop *");
		break;
	case ICHILDREN:
		zs_apps(zstr, "*ChildrenLoop *");
		break;
	case IFAMILYSPOUSES:
		zs_apps(zstr, "*FamilySpousesLoop *");
		break;
	case IINDI:
		zs_apps(zstr, "*PersonLoop *");
		break;
	case IFAM:
		zs_apps(zstr, "*FamilyLoop *");
		break;
	case ISOUR:
		zs_apps(zstr, "*SourceLoop *");
		break;
	case IEVEN:
		zs_apps(zstr, "*EventLoop *");
		break;
	case IOTHR:
		zs_apps(zstr, "*OtherLoop *");
		break;
	case ILIST:
		zs_apps(zstr, "*ListLoop *");
		break;
	case ISET:
		zs_apps(zstr, "*IndisetLoop *");
		break;
	case IFATHS:
		zs_apps(zstr, "*FathersLoop *");
		break;
	case IMOTHS:
		zs_apps(zstr, "*MothersLoop *");
		break;
	case IFAMCS:
		zs_apps(zstr, "*ParentsLoop *");
		break;
	case INOTES:
		zs_apps(zstr, "*NotesLoop *");
		break;
	default:
		break;
	}
}
/*==========================================================
 * create_rptinfo -- Create new empty report info object
 * returns addref'd rptinfo
 *========================================================*/
static RPTINFO
create_rptinfo (void)
{
	RPTINFO rptinfo = (RPTINFO)stdalloc(sizeof(*rptinfo));
	memset(rptinfo, 0, sizeof(*rptinfo));
	rptinfo->vtable = &vtable_for_rptinfo;
	rptinfo->refcnt = 1;
	return rptinfo;
}
/*==========================================================
 * get_rptinfo -- Fetch info about report file
 *  create if not yet known
 *========================================================*/
RPTINFO
get_rptinfo (CNSTRING fullpath)
{
	RPTINFO rptinfo;
	if (!f_rptinfos)
		f_rptinfos = create_table_obj();
	rptinfo = (RPTINFO)valueof_obj(f_rptinfos, fullpath);
	if (!rptinfo) {
		STRING filename=0;
		ZSTR zstr=0;

		rptinfo = create_rptinfo();
		rptinfo->fullpath = strsave(fullpath);
		rptinfo->functab = create_table_vptr(); /* PNODES owned elsewhere */
		rptinfo->proctab = create_table_vptr(); /* PNODES owned elsewhere */
		rptinfo->codeset = strsave(report_codeset_in);

		/* calculate localpath & localepath for report gettext */
		filename = lastpathname(fullpath);
		zstr = zs_newsubs(fullpath, strlen(fullpath)-strlen(filename)-1);
		rptinfo->localpath = zstr;
		filename = concat_path_alloc(zs_str(zstr), "locale");
		rptinfo->localepath = zs_news(filename);
		strfree(&filename);
		rptinfo->textdomain = zs_news("llreports"); /* for now, fixed textdomain */
		
		insert_table_obj(f_rptinfos, fullpath, rptinfo);
		--rptinfo->refcnt; /* release our reference on rptinfo */
		ASSERT(rptinfo->refcnt>0);
	}
	return rptinfo;
}
/*==========================================================
 * clear_rptinfos -- Delete all allocated rptinfos
 *========================================================*/
void
clear_rptinfos (void)
{
	if (f_rptinfos) {
		destroy_table(f_rptinfos);
		f_rptinfos = 0;
	}
}
/*=================================================
 * rptinfo_destructor -- destructor for rptinfo
 *  (destructor entry in vtable)
 *===============================================*/
static void
rptinfo_destructor (VTABLE *obj)
{
	RPTINFO rptinfo = (RPTINFO)obj;
	ASSERT(rptinfo->vtable == &vtable_for_rptinfo);

	destroy_table(rptinfo->proctab); /* values are vptr PNODES */
	destroy_table(rptinfo->functab); /* values are vptr PNODES */
	strfree(&rptinfo->fullpath);
	strfree(&rptinfo->codeset);
	zs_free(&rptinfo->localpath);
	zs_free(&rptinfo->localepath);
	zs_free(&rptinfo->textdomain);
	stdfree(rptinfo);
}
