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
 * interp.h - Header file for report interpreter
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *  2.3.4 - 24 Jun 93   2.3.5 - 16 Aug 93
 *  3.0.0 - 19 Jun 94   3.0.2 - 21 Dec 94
 *  3.0.3 - 07 Aug 95
 *===========================================================*/

#ifndef _INTERP_H
#define _INTERP_H

#ifndef _CACHE_H
#include "cache.h"
#endif

typedef struct itag *PNODE;
struct itag {
	char   i_type;	/* type of node */
	PNODE  i_prnt;	/* parent of this node */
	INT    i_line;	/* line where text of this node begins */
	STRING i_file;	/* file where text of this node begins */
	PNODE  i_next;	/* next node */
	WORD   i_word1;	/* variable data associated with node type */
	WORD   i_word2;	/* ... */
	WORD   i_word3;
	WORD   i_word4;
	WORD   i_word5;
};

#define IICONS		 1	/* integer constant */
#define IFCONS		 2	/* floating constant */
#define ILCONS		 3	/* long integer constant */
#define ISCONS		 4	/* string constant */
#define IIDENT     	 5	/* identifier */
#define IIF		 6	/* if statement */
#define IWHILE		 7	/* while loop */
#define IBREAK		 8	/* break statement */
#define ICONTINUE	 9	/* continue statement */
#define IRETURN		10	/* return statement */
#define IPDEFN		11	/* user-defined procedure defn */
#define IPCALL		12	/* user-defined procudure call */
#define IFDEFN		13	/* user-defined function defn */
#define IFCALL		14	/* user-defined function call */
#define IBCALL		15	/* built-in function call */
#define ITRAV		16	/* traverse loop */
#define INODES		17	/* fornodes loop */
#define IFAMILIES	18	/* families loop */
#define ISPOUSES	19	/* spouses loop */
#define ICHILDREN	20	/* children loop */
#define IINDI		21	/* person loop */
#define IFAM		22	/* family loop */
#define ISOUR		23	/* source loop */
#define IEVEN		24	/* event loop */
#define IOTHR		25	/* other loop */
#define ILIST		26	/* list loop */
#define ISET		27	/* set loop */
#define IFATHS		28	/* fathers loop */
#define IMOTHS		29	/* mothers loop */
#define IFAMCS		30	/* parents loop */
#define INOTES		31	/* notes loop */

#define itype(i)     ((i)->i_type)	/* node type - all nodes */
#define iprnt(i)     ((i)->i_prnt)	/* parent node - all nodes */
#define inext(i)     ((i)->i_next)	/* next node - all nodes */
#define iline(i)     ((i)->i_line)	/* program line - all nodes */
#define ifname(i)    ((i)->i_file)	/* program name - all nodes */

#define ivalue(i)    ((PVALUE)((i)->i_word1)) /* constant values */
#define iident(i)    ((i)->i_word1)	/* ident nodes */
#define iargs(i)     ((i)->i_word2)	/* param and arg lists */
#define icond(i)     ((i)->i_word1)	/* cond expr in if & while */
#define ithen(i)     ((i)->i_word2)	/* then statement in if */
#define ielse(i)     ((i)->i_word3)	/* else statement in if */

#define ifunc(i)     ((i)->i_word3)	/* func and builtin reference */
#define ichild(i)    ((i)->i_word2)	/* var in children loop */
#define ispouse(i)   ((i)->i_word2)	/* var in families and spouses loop */
#define ifamily(i)   ((i)->i_word3)	/* var in all families type loops */
#define iiparent(i)   ((i)->i_word2)	/* var in some families type loops */
#define ivalvar(i)   ((i)->i_word3)	/* var in indiset loop */
#define iname(i)     ((i)->i_word1)	/* proc, func and builtin names */
#define ilev(i)      ((i)->i_word3)	/* var traverse loop */

#define iloopexp(i)  ((i)->i_word1)	/* top loop expression */
#define ielement(i)  ((i)->i_word2)	/* loop element */
#define ibody(i)     ((i)->i_word5)	/* body of proc, func, loops */
#define inum(i)      ((i)->i_word4)	/* counter used by many loops */

#define PNONE	0	/* needed? - remove later if not */
#define PANY    1	/* any value -- no type restriction */
#define PINT    2	/* integer */
#define PLONG   3	/* long integer */
#define PFLOAT  4	/* floating point */
#define PBOOL	5	/* boolean */
#define PSTRING 6	/* string */
#define PGNODE	7	/* GEDCOM node */
#define PINDI	8	/* GEDCOM person record */
#define PFAM	9	/* GEDCOM family record */
#define PSOUR	10	/* GEDCOM source record */
#define PEVEN	11	/* GEDCOM event record */
#define POTHR	12	/* GEDCOM other record */
#define PLIST	13	/* list */
#define PTABLE	14	/* table */
#define PSET	15	/* set */

typedef struct ptag *PVALUE;
struct ptag {
	char type;	/* type of value */
	WORD value;	/* value */
};

typedef PVALUE (*PFUNC)();

#define ptype(p)	((p)->type)	/* type of expression */
#define pvalue(p)	((p)->value)	/* value of expression */

#define pitype(i)	ptype(ivalue(i))
#define pivalue(i)	pvalue(ivalue(i))

typedef struct {
	char *ft_name;
	INT ft_nparms_min;
	INT ft_nparms_max;
	WORD (*ft_eval)();
} BUILTINS;

extern BUILTINS builtins[];
extern INT nobuiltins;
extern BOOLEAN prog_debug;

extern TABLE functab;
extern TABLE globtab;
extern TABLE proctab;

/* PVALUE Arithmetic Functions */
void add_pvalues(PVALUE, PVALUE, BOOLEAN*);
void sub_pvalues(PVALUE, PVALUE, BOOLEAN*);
void mul_pvalues(PVALUE, PVALUE, BOOLEAN*);
void div_pvalues(PVALUE, PVALUE, BOOLEAN*);
void mod_pvalues(PVALUE, PVALUE, BOOLEAN*);
void neg_pvalue(PVALUE, BOOLEAN*);
void decr_pvalue(PVALUE, BOOLEAN*);
void incr_pvalue(PVALUE, BOOLEAN*);
void exp_pvalues(PVALUE, PVALUE, BOOLEAN*);
void gt_pvalues(PVALUE, PVALUE, BOOLEAN*);
void ge_pvalues(PVALUE, PVALUE, BOOLEAN*);
void lt_pvalues(PVALUE, PVALUE, BOOLEAN*);
void le_pvalues(PVALUE, PVALUE, BOOLEAN*);
void ne_pvalues(PVALUE, PVALUE, BOOLEAN*);
void eq_pvalues(PVALUE, PVALUE, BOOLEAN*);

/* PVALUE Functions */
void coerce_pvalue(INT, PVALUE, BOOLEAN*);
PVALUE copy_pvalue(PVALUE);
PVALUE create_pvalue(INT, WORD);
void delete_pvalue(PVALUE);
void eq_conform_pvalues(PVALUE, PVALUE, BOOLEAN*);
BOOLEAN eqv_pvalues(PVALUE, PVALUE);
BOOLEAN is_pvalue(PVALUE);
BOOLEAN is_record_pvalue(PVALUE);
BOOLEAN is_zero(PVALUE);
void num_conform_pvalues(PVALUE, PVALUE, BOOLEAN*);
BOOLEAN numeric_pvalue(PVALUE);
void set_pvalue(PVALUE, INT, WORD);
void show_pvalue(PVALUE);
STRING pvalue_to_string(PVALUE);
void insert_pvtable(TABLE, STRING, INT, WORD);
void remove_pvtable(TABLE);
#ifndef HOGMEMORY
void zero_pventry(ENTRY);
#endif

/* Prototypes */
void assign_iden(TABLE, STRING, WORD);
INT bool_to_int(BOOLEAN);
FLOAT bool_to_float(BOOLEAN);
PNODE break_node(void);
PNODE call_node(STRING, PNODE);
PNODE children_node(PNODE, STRING, STRING, PNODE);
PNODE children_node(PNODE, STRING, STRING, PNODE);
PNODE create_pnode(INT);
PVALUE evaluate(PNODE, TABLE, BOOLEAN*);
BOOLEAN evaluate_cond(PNODE, TABLE, BOOLEAN*);
PVALUE evaluate_func(PNODE, TABLE, BOOLEAN*);
PVALUE evaluate_iden(PNODE, TABLE, BOOLEAN*);
PVALUE evaluate_ufunc(PNODE, TABLE, BOOLEAN*);
PVALUE eval_and_coerce(INT, PNODE, TABLE, BOOLEAN*);
NODE eval_indi(PNODE, TABLE, BOOLEAN*, CACHEEL*);
NODE eval_fam(PNODE, TABLE, BOOLEAN*, CACHEEL*);
void extract_date(STRING, INT*, INT*, INT*, INT*, STRING*);
PNODE families_node(PNODE, STRING, STRING, STRING, PNODE);
PNODE fcons_node(FLOAT);
PNODE fdef_node(STRING, PNODE, PNODE);
PNODE foreven_node(STRING, STRING, PNODE);
PNODE forfam_node(STRING, STRING, PNODE);
PNODE forindi_node(STRING, STRING, PNODE);
PNODE forindiset_node(PNODE, STRING, STRING, STRING, PNODE);
PNODE forlist_node(PNODE, STRING, STRING, PNODE);
STRING format_date(STRING, INT, INT, INT, INT, BOOLEAN);
PNODE forsour_node(STRING, STRING, PNODE);
PNODE fornotes_node(PNODE, STRING, PNODE);
PNODE forothr_node(STRING, STRING, PNODE);
PNODE func_node(STRING, PNODE);
STRING get_date(void);
PNODE icons_node(INT);
PNODE iden_node(STRING);
PNODE if_node(PNODE, PNODE, PNODE);
BOOLEAN iistype(PNODE, INT);
PNODE mothers_node(PNODE, STRING, STRING, STRING, PNODE);
INT num_params(PNODE);
PNODE parents_node(PNODE, STRING, STRING, PNODE);
PNODE proc_node(STRING, PNODE, PNODE);
void prog_error(PNODE, STRING);
PNODE return_node(PNODE);
void set_parents(PNODE, PNODE);
void show_one_pnode(PNODE);
void show_pnode(PNODE);
void show_pnodes(PNODE);
PNODE spouses_node(PNODE, STRING, STRING, STRING, PNODE);
PNODE string_node(STRING);
PNODE traverse_node(PNODE, STRING, STRING, PNODE);
PVALUE valueof_iden(TABLE, STRING);
PNODE while_node(PNODE, PNODE);

extern INT Plineno;
extern FILE *Pinfp;
extern FILE *Poutfp;
extern INT curcol;
extern INT currow;
extern INT _rows;
extern INT _cols;
extern INT Perrors;
extern INT nobuiltins;

/* Input and output modes */

#define FILEMODE   0	/* input from a file */
#define STRINGMODE 1	/* input or output from or to a string */
#define UNBUFFERED 2	/* output unbuffered to a file */
#define BUFFERED   3	/* output buffered to a file */
#define PAGEMODE   4	/* output page buffered to a file */

#define INTERPTYPE INT
#define INTERROR    0
#define INTOKAY     1
#define INTBREAK    2
#define INTCONTINUE 3
#define INTRETURN   4

#define TYPE_CHECK(t, v) \
	if (ptype(v) != t) {\
		*eflg = TRUE;\
		return NULL;\
	}

#endif /* _INTERP_H */

void poutput(STRING);
