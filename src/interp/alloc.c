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
 * alloc.c -- Allocate nodes for report generator
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 17 Aug 93
 *   3.0.0 - 28 Jun 94    3.0.2 - 23 Dec 94
 *   3.0.3 - 10 Aug 95
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interp.h"
#include "liflines.h"
#include "screen.h"

extern STRING ierror;
extern STRING Pfname;

static void set_parents (PNODE body, PNODE node);

/*==================================
 * create_pnode -- Create PNODE node
 *================================*/
PNODE
create_pnode (INT type)
{
	PNODE node = (PNODE) stdalloc(sizeof(*node));
	itype(node) = type;
	iprnt(node) = NULL;
	inext(node) = NULL;
	iline(node) = Plineno;
	ifname(node) = Pfname;
	node->i_word1 = node->i_word2 = node->i_word3 = NULL;
	node->i_word4 = node->i_word5 = NULL;
	return node;
}
/*==================================
 * string_node -- Create string node
 *================================*/
PNODE
string_node (STRING str)
{
	PNODE node = create_pnode(ISCONS);
	ivalue(node) = create_pvalue(PSTRING, (WORD) str);
	return node;
}
/*========================================
 * children_node -- Create child loop node
 *======================================*/
PNODE
children_node (PNODE fexpr,     /* expr */
               STRING cvar,     /* child */
               STRING nvar,     /* counter */
               PNODE body)      /* loop body */
{
	PNODE node = create_pnode(ICHILDREN);
	iloopexp(node) = (WORD) fexpr;
	ichild(node) = (WORD) cvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*========================================
 * spouses_node -- Create spouse loop node
 *======================================*/
PNODE
spouses_node (PNODE pexpr,      /* expr */
              STRING svar,      /* spouse */
              STRING fvar,      /* family */
              STRING nvar,      /* counter */
              PNODE body)       /* body */
{
	PNODE node = create_pnode(ISPOUSES);
	iloopexp(node) = (WORD) pexpr;
	ispouse(node) = (WORD) svar;
	ifamily(node) = (WORD) fvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=========================================
 * families_node -- Create family loop node
 *=======================================*/
PNODE
families_node (PNODE pexpr,     /* expr */
               STRING fvar,     /* family */
               STRING svar,     /* spouse */
               STRING nvar,     /* counter */
               PNODE body)      /* body */
{
	PNODE node = create_pnode(IFAMILIES);
	iloopexp(node) = (WORD) pexpr;
	ifamily(node) = (WORD) fvar;
	ispouse(node) = (WORD) svar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=========================================
 * fathers_node -- Create fathers loop node
 *=======================================*/
PNODE
fathers_node (PNODE pexpr,      /* expr */
              STRING pvar,      /* father */
              STRING fvar,      /* family */
              STRING nvar,      /* counter */
              PNODE body)       /* body */
{
	PNODE node = create_pnode(IFATHS);
	iloopexp(node) = (WORD) pexpr;
	iiparent(node) = (WORD) pvar;
	ifamily(node) = (WORD) fvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=========================================
 * mothers_node -- Create mothers loop node
 *=======================================*/
PNODE
mothers_node (PNODE pexpr,      /* expr */
              STRING pvar,      /* mother */
              STRING fvar,      /* family */
              STRING nvar,      /* counter */
              PNODE body)       /* body */
{
	PNODE node = create_pnode(IMOTHS);
	iloopexp(node) = (WORD) pexpr;
	iiparent(node) = (WORD) pvar;
	ifamily(node) = (WORD) fvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=========================================
 * parents_node -- Create parents loop node
 *=======================================*/
PNODE
parents_node (PNODE pexpr,      /* expr */
              STRING fvar,      /* family */
              STRING nvar,      /* counter */
              PNODE body)       /* body */
{
	PNODE node = create_pnode(IFAMCS);
	iloopexp(node) = (WORD) pexpr;
	ifamily(node) = (WORD) fvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*========================================
 * forindiset_node -- Create set loop node
 *======================================*/
PNODE
forindiset_node (PNODE iexpr,   /* expr */
                 STRING ivar,   /* person */
                 STRING vvar,   /* value */
                 STRING nvar,   /* counter */
                 PNODE body)    /* body */
{
	PNODE node = create_pnode(ISET);
	iloopexp(node) = (WORD) iexpr;
	ielement(node) = (WORD) ivar;
	ivalvar(node) = (WORD) vvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*======================================
 * forlist_node -- Create list loop node
 *====================================*/
PNODE
forlist_node (PNODE iexpr,      /* expr */
              STRING evar,      /* element */
              STRING nvar,      /* counter */
              PNODE body)       /* body */
{
	PNODE node = create_pnode(ILIST);
	iloopexp(node) = (WORD) iexpr;
	ielement(node) = (WORD) evar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=========================================
 * forindi_node -- Create forindi loop node
 *=======================================*/
PNODE
forindi_node (STRING ivar,      /* pers */
              STRING nvar,      /* counter */
              PNODE body)       /* body */
{
	PNODE node = create_pnode(IINDI);
	ielement(node) = (WORD) ivar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=========================================
 * forsour_node -- Create forsour loop node
 *=======================================*/
PNODE
forsour_node (STRING fvar,    /* fam */
              STRING nvar,    /* counter */
              PNODE body)     /* body */
{
	PNODE node = create_pnode(ISOUR);
	ielement(node) = (WORD) fvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=========================================
 * foreven_node -- Create foreven loop node
 *=======================================*/
PNODE
foreven_node (STRING fvar,    /* fam */
              STRING nvar,    /* counter */
              PNODE body)     /* body */
{
	PNODE node = create_pnode(IEVEN);
	ielement(node) = (WORD) fvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=========================================
 * forothr_node -- Create forothr loop node
 *=======================================*/
PNODE
forothr_node (STRING fvar,    /* fam */
              STRING nvar,    /* counter */
              PNODE body)     /* body */
{
	PNODE node = create_pnode(IOTHR);
	ielement(node) = (WORD) fvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=======================================
 * forfam_node -- Create forfam loop node
 *=====================================*/
PNODE
forfam_node (STRING fvar,    /* fam */
             STRING nvar,    /* counter */
             PNODE body)     /* body */
{
	PNODE node = create_pnode(IFAM);
	ielement(node) = (WORD) fvar;
	inum(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*===========================================
 * fornotes_node -- Create fornotes loop node
 *=========================================*/
PNODE
fornotes_node (PNODE nexpr,   /* expr */
               STRING vvar,   /* value */
               PNODE body)    /* body */
{
        PNODE node = create_pnode(INOTES);
        iloopexp(node) = (WORD) nexpr;
        ielement(node) = (WORD) vvar;
        ibody(node) = (WORD) body;
        set_parents(body, node);
        return node;
}
/*===========================================
 * fornodes_node -- Create fornodes loop node
 *=========================================*/
PNODE
fornodes_node (PNODE nexpr,    /* expr */
               STRING nvar,    /* node (next level) */
               PNODE body)     /* body */
{
	PNODE node = create_pnode(INODES);
	iloopexp(node) = (WORD) nexpr;
	ielement(node) = (WORD) nvar;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*===========================================
 * traverse_node -- Create traverse loop node
 *=========================================*/
PNODE
traverse_node (PNODE nexpr,    /* node */
               STRING snode,   /* subnode */
               STRING levv,    /* level */
               PNODE body)     /* body */
{
	PNODE node = create_pnode(ITRAV);
	iloopexp(node) = (WORD) nexpr;
	ielement(node) = (WORD) snode;
	ilev(node) = (WORD) levv;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*====================================
 * iden_node -- Create identifier node
 *==================================*/
PNODE
iden_node (STRING iden)
{
	PNODE node = create_pnode(IIDENT);
	iident(node) = (WORD) iden;
	return node;
}
/*==================================
 * icons_node -- Create integer node
 *================================*/
PNODE
icons_node (INT ival)
{
	PNODE node = create_pnode(IICONS);
	ivalue(node) = create_pvalue(PINT, (WORD) ival);
	return node;
}
/*===================================
 * fcons_node -- Create floating node
 *=================================*/
PNODE
fcons_node (FLOAT fval)
{
	PNODE node = create_pnode(IFCONS);
	UNION u;
	u.f = fval;
	ivalue(node) = create_pvalue(PFLOAT, u.w);
	return node;
}
/*===================================
 * proc_node -- Create procedure node
 *=================================*/
PNODE
proc_node (STRING name,    /* proc name */
           PNODE parms,    /* param/s */
           PNODE body)     /* body */
{
	PNODE node = create_pnode(IPDEFN);
	iname(node) = (WORD) name;
	iargs(node) = (WORD) parms;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*==================================================
 * fdef_node -- Create user function definition node
 *================================================*/
PNODE
fdef_node (STRING name,    /* proc name */
           PNODE parms,    /* param/s */
           PNODE body)     /* body */
{
	PNODE node = create_pnode(IFDEFN);
	iname(node) = (WORD) name;
	iargs(node) = (WORD) parms;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*=======================================================
 * func_node -- Create builtin or user function call node
 *=====================================================*/
PNODE
func_node (STRING name,    /* function name */
           PNODE elist)    /* param/s */
{
	PNODE node;
	INT lo, hi, md, n, r;
	BOOLEAN found = FALSE;

/* See if the function is user defined */
	if (in_table(functab, name)) {
		node = create_pnode(IFCALL);
		iname(node) = (WORD) name;
		iargs(node) = (WORD) elist;
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
		    || n > builtins[md].ft_nparms_max) {
			llwprintf(ierror, Pfname, Plineno);
			llwprintf("%s: must have %d to %d parameters.\n", name,
		    	builtins[md].ft_nparms_min, builtins[md].ft_nparms_max);
			Perrors++;
		}
		node = create_pnode(IBCALL);
		iname(node) = (WORD) name;
		iargs(node) = (WORD) elist;
		ifunc(node) = (WORD) builtins[md].ft_eval;
		return node;
		
	}

/* If neither make it a user call to undefined function */
	node = create_pnode(IFCALL);
	iname(node) = (WORD) name;
	iargs(node) = (WORD) elist;
	ifunc(node) = NULL;
	return node;
}
/*=============================
 * if_node -- Create an if node
 *===========================*/
PNODE
if_node (PNODE cond,     /* cond expr */
         PNODE tnode,    /* then */
         PNODE enode)    /* else */
{
	PNODE node = create_pnode(IIF);
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
PNODE
while_node (PNODE cond,     /* cond expr */
            PNODE body)     /* body */
{
	PNODE node = create_pnode(IWHILE);
	icond(node) = (WORD) cond;
	ibody(node) = (WORD) body;
	set_parents(body, node);
	return node;
}
/*===================================
 * call_node -- Create proc call node
 *=================================*/
PNODE
call_node (STRING name,    /* proc name */
           PNODE args)     /* arg/s */
{
	PNODE node = create_pnode(IPCALL);
	iname(node) = (WORD) name;
	iargs(node) = (WORD) args;
	return node;
}
/*================================
 * break_node -- Create break node
 *==============================*/
PNODE break_node (void)
{
	PNODE node = create_pnode(IBREAK);
	return node;
}
/*======================================
 * continue_node -- Create continue node
 *====================================*/
PNODE continue_node (void)
{
	PNODE node = create_pnode(ICONTINUE);
	return node;
}
/*==================================
 * return_node -- Create return node
 *================================*/
PNODE
return_node (PNODE args)
{
	PNODE node = create_pnode(IRETURN);
	iargs(node) = (WORD) args;
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
	void show_one_pnode();
	while (node) {
		show_one_pnode(node);
		node = inext(node);
	}
}
/*==========================================================
 * show_pnodes -- DEBUG routine that shows expression PNODEs
 *========================================================*/
void
show_pnodes (PNODE node)
{
	void show_one_pnode();

	while (node) {
		show_one_pnode(node);
		node = inext(node);
		if (node) llwprintf(",");
	}
}
/*====================================================
 * show_one_pnode -- DEBUG routine that show one PNODE
 *==================================================*/
void
show_one_pnode (PNODE node)     /* node to print */
{
	UNION u;

	switch (itype(node)) {

	case IICONS:
		llwprintf("%d", pvalue(ivalue(node)));
		break;
	case IFCONS:
		u.w = pvalue(ivalue(node));
		llwprintf("%f", u.f);
		break;
	case ILCONS:
		llwprintf("*ni*");
		break;
	case ISCONS:
		llwprintf("^^%s^^", pvalue(ivalue(node)));
		break;
	case IIDENT:
		llwprintf("%s", iident(node));
		break;
	case IIF:
		llwprintf("if(");
		show_pnodes(icond(node));
		llwprintf("){");
		show_pnodes(ithen(node));
		llwprintf("}");
		if (ielse(node)) {
			llwprintf("else{");
			show_pnodes(ielse(node));
			llwprintf("}");
		}
		break;
	case IWHILE:
		llwprintf("while(");
		show_pnodes(icond(node));
		llwprintf("){");
		show_pnodes(ibody(node));
		llwprintf("}");
		break;
	case IBREAK:
		llwprintf("break ");
		break;
	case ICONTINUE:
		llwprintf("continue ");
		break;
	case IRETURN:
		llwprintf("return(");
		show_pnodes(iargs(node));
		llwprintf(")");
		break;
	case IPDEFN:
		llwprintf("*PDefn *");
		break;
	case IPCALL:
		llwprintf("%s(", iname(node));
		show_pnodes(iargs(node));
		llwprintf(")");
		break;
	case IFDEFN:
		llwprintf("*FDefn *");
		break;
	case IFCALL:
		llwprintf("%s(", iname(node));
		show_pnodes(iargs(node));
		llwprintf(")");
		break;
	case IBCALL:
		llwprintf("%s(", iname(node));
		show_pnodes(iargs(node));
		llwprintf(")");
		break;
	case ITRAV:
		llwprintf("*Traverse *");
		break;
	case INODES:
		llwprintf("*Fornodes *");
		break;
	case IFAMILIES:
		llwprintf("*FamiliesLoop *");
		break;
	case ISPOUSES:
		llwprintf("*SpousesLoop *");
		break;
	case ICHILDREN:
		llwprintf("*ChildrenLoop *");
		break;
	case IINDI:
		llwprintf("*PersonLoop *");
		break;
	case IFAM:
		llwprintf("*FamilyLoop *");
		break;
	case ISOUR:
		llwprintf("*SourceLoop *");
		break;
	case IEVEN:
		llwprintf("*EventLoop *");
		break;
	case IOTHR:
		llwprintf("*OtherLoop *");
		break;
	case ILIST:
		llwprintf("*ListLoop *");
		break;
	case ISET:
		llwprintf("*IndisetLoop *");
		break;
	case IFATHS:
		llwprintf("*FathersLoop *");
		break;
	case IMOTHS:
		llwprintf("*MothersLoop *");
		break;
	case IFAMCS:
		llwprintf("*ParentsLoop *");
		break;
	case INOTES:
		llwprintf("*NotesLoop *");
		break;
	default:
		break;
	}
}
