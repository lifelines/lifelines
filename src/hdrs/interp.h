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
 * pre-SourceForge version information:
 *  2.3.4 - 24 Jun 93   2.3.5 - 16 Aug 93
 *  3.0.0 - 19 Jun 94   3.0.2 - 21 Dec 94
 *  3.0.3 - 07 Aug 95
 *===========================================================*/

#ifndef _INTERP_H
#define _INTERP_H

#define LIFELINES_REPORTS_VERSION "1.1"


#include "cache.h"

struct rptinfo_s {
	STRING fullpath;
	TABLE proctab;   /* all procs in this file */
	TABLE functab;   /* all funcs in this file */
	STRING codeset;  /* codeset of report */
	STRING requires; /* version of lifelines report language required */
};
typedef struct rptinfo_s *RPTINFO;

typedef struct itag *PNODE;
struct itag {
	char     i_type;       /* type of node */
	PNODE    i_prnt;       /* parent of this node */
	INT      i_line;	     /* line where text of this node begins */
	RPTINFO  i_rptinfo;    /* information about this report file */
	PNODE    i_next;       /* next node */
	VPTR     i_word1;      /* variable data associated with node type */
	VPTR     i_word2;	     /* ... */
	VPTR     i_word3;
	VPTR     i_word4;
	VPTR     i_word5;
};

#define IICONS       1   /* integer constant */
#define IFCONS       2   /* floating constant */
#define ILCONS       3   /* long integer constant */
#define ISCONS       4   /* string constant */
#define IIDENT       5   /* identifier */
#define IIF          6   /* if statement */
#define IWHILE       7   /* while loop */
#define IBREAK       8   /* break statement */
#define ICONTINUE    9   /* continue statement */
#define IRETURN     10   /* return statement */
#define IPDEFN      11   /* user-defined procedure defn */
#define IPCALL      12   /* user-defined procudure call */
#define IFDEFN      13   /* user-defined function defn */
#define IFCALL      14   /* user-defined function call */
#define IBCALL      15   /* built-in function call */
#define ITRAV       16   /* traverse loop */
#define INODES      17   /* fornodes loop */
#define IFAMILIES   18   /* families loop */
#define ISPOUSES    19   /* spouses loop */
#define ICHILDREN   20   /* children loop */
#define IINDI       21   /* person loop */
#define IFAM        22   /* family loop */
#define ISOUR       23   /* source loop */
#define IEVEN       24   /* event loop */
#define IOTHR       25   /* other loop */
#define ILIST       26   /* list loop */
#define ISET        27   /* set loop */
#define IFATHS      28   /* fathers loop */
#define IMOTHS      29   /* mothers loop */
#define IFAMCS      30   /* parents loop */
#define INOTES      31   /* notes loop */
#define IFREED      99   /* returned to free list */

#define itype(i)     ((i)->i_type)  /* node type - all nodes */
#define iprnt(i)     ((i)->i_prnt)  /* parent node - all nodes */
#define inext(i)     ((i)->i_next)  /* next node - all nodes */
#define iline(i)     ((i)->i_line)  /* program line - all nodes */
#define irptinfo(i)  ((i)->i_rptinfo)  /* program name - all nodes */

#define ivalue(i)    ((PVALUE)((i)->i_word1)) /* constant values */
#define ivaluex(i)   ((i)->i_word1)     /* constant values, for setting */
#define iident(i)    ((i)->i_word1)     /* ident nodes */
#define iargs(i)     ((i)->i_word2)     /* param and arg lists */
#define icond(i)     ((i)->i_word1)     /* cond expr in if & while */
#define ithen(i)     ((i)->i_word2)     /* then statement in if */
#define ielse(i)     ((i)->i_word3)     /* else statement in if */

#define ifunc(i)     ((i)->i_word3)     /* func and builtin reference */
#define ichild(i)    ((i)->i_word2)     /* var in children loop */
#define ispouse(i)   ((i)->i_word2)     /* var in families and spouses loop */
#define ifamily(i)   ((i)->i_word3)     /* var in all families type loops */
#define iiparent(i)  ((i)->i_word2)    /* var in some families type loops */
#define ivalvar(i)   ((i)->i_word3)     /* var in indiset loop */
#define iname(i)     ((i)->i_word1)     /* proc, func and builtin names */
#define ilev(i)      ((i)->i_word3)     /* var traverse loop */

#define iloopexp(i)  ((i)->i_word1)     /* top loop expression */
#define ielement(i)  ((i)->i_word2)     /* loop element */
#define ibody(i)     ((i)->i_word5)     /* body of proc, func, loops */
#define inum(i)      ((i)->i_word4)     /* counter used by many loops */

#define PNONE      0  /* needed? - remove later if not */
#define PANY       1  /* any value -- no type restriction - should be NULL value*/
#define PINT       2  /* integer */
#define PLONG      3  /* long integer */
#define PFLOAT     4  /* floating point */
#define PBOOL      5  /* boolean */
#define PSTRING    6  /* string */
#define PGNODE	    7  /* GEDCOM node */
#define PINDI      8  /* GEDCOM person record */
#define PFAM       9  /* GEDCOM family record */
#define PSOUR     10  /* GEDCOM source record */
#define PEVEN     11  /* GEDCOM event record */
#define POTHR     12  /* GEDCOM other record */
#define PLIST     13  /* list */
#define PTABLE    14  /* table */
#define PSET      15  /* set */
#define PFREED    99  /* returned to free list */
#define PUNINT   100  /* just allocated */

/*
 * ptag should be using a UNION not a VPTR
 * First I'm removing the zillions of casts using PVALUE->value tho
 * so the compiler will help find any misuse
 * Perry, 2002.02.17
 */
typedef struct ptag *PVALUE;
struct ptag {
	unsigned char type;	/* type of value */
	VPTR value;	/* value */
/*	UNION uval;*/
};

typedef struct symtab_s {
	TABLE tab;
} SYMTAB;
typedef struct symtab_iter_s {
	struct table_iter_s tabiters;
} *SYMTAB_ITER;


typedef PVALUE (*PFUNC)(PNODE, SYMTAB, BOOLEAN *);

#define ptype(p)	((p)->type)	/* type of expression */
#define pvalue(p)	((p)->value)	/* value of expression */

#define pitype(i)	ptype(ivalue(i))
#define pivalue(i)	pvalue(ivalue(i))


typedef struct {
	char *ft_name;
	INT ft_nparms_min;
	INT ft_nparms_max;
	PVALUE (*ft_eval)(PNODE, SYMTAB, BOOLEAN *);
} BUILTINS;

extern BUILTINS builtins[];
extern INT nobuiltins;
extern BOOLEAN prog_trace;

extern TABLE gfunctab;
extern SYMTAB globtab;
extern TABLE gproctab;

extern FILE *Poutfp;
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

typedef struct pactx_s *PACTX;

/* PVALUE Arithmetic Functions */
void add_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void sub_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void mul_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void div_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void mod_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void neg_pvalue(PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void decr_pvalue(PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void incr_pvalue(PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void exp_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void gt_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void ge_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void lt_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void le_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void ne_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void eq_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);

/* PVALUE & SYMTAB Functions */
void bad_type_error(CNSTRING op, ZSTR *zerr, PVALUE val1, PVALUE val2);
BOOLEAN begin_symtab(SYMTAB stab, SYMTAB_ITER stabit);
void coerce_pvalue(INT, PVALUE, BOOLEAN*);
PVALUE copy_pvalue(PVALUE);
void create_symtab(SYMTAB * stab);
PVALUE create_pvalue(INT, VPTR);
PVALUE create_pvalue_any(void);
PVALUE create_pvalue_from_bool(BOOLEAN bval);
PVALUE create_pvalue_from_cel(CACHEEL cel);
PVALUE create_pvalue_from_float(float fval);
PVALUE create_pvalue_from_even_keynum(INT i);
PVALUE create_pvalue_from_fam(NODE fam);
PVALUE create_pvalue_from_fam_keynum(INT i);
PVALUE create_pvalue_from_indi(NODE indi);
PVALUE create_pvalue_from_indi_key(STRING key);
PVALUE create_pvalue_from_indi_keynum(INT i);
PVALUE create_pvalue_from_int(INT ival);
PVALUE create_pvalue_from_node(NODE node);
PVALUE create_pvalue_from_othr_keynum(INT i);
PVALUE create_pvalue_from_set(VPTR seq);
PVALUE create_pvalue_from_sour_keynum(INT i);
PVALUE create_pvalue_from_string(STRING str);
ZSTR describe_pvalue(PVALUE);
void delete_vptr_pvalue(VPTR ptr);
void delete_pvalue(PVALUE);
void delete_pvalue_ptr(PVALUE * valp);
void delete_symtab(SYMTAB stab, STRING iden);
void eq_conform_pvalues(PVALUE, PVALUE, BOOLEAN*);
BOOLEAN eqv_pvalues(VPTR, VPTR);
CACHEEL get_cel_from_pvalue(PVALUE val);
BOOLEAN in_symtab(SYMTAB stab, STRING key);
void insert_symtab(SYMTAB stab, STRING iden, PVALUE val);
BOOLEAN is_numeric_pvalue(PVALUE);
BOOLEAN is_pvalue(PVALUE);
BOOLEAN is_record_pvalue(PVALUE);
BOOLEAN is_zero(PVALUE);
BOOLEAN next_symtab_entry(SYMTAB_ITER tabit, STRING *pkey, PVALUE *ppval);
SYMTAB null_symtab(void);
void pvalues_begin(void);
void pvalues_end(void);
BOOLEAN pvalue_to_bool(PVALUE);
float pvalue_to_float(PVALUE val);
INT pvalue_to_int(PVALUE);
LIST pvalue_to_list(PVALUE val);
float* pvalue_to_pfloat(PVALUE);
INT* pvalue_to_pint(PVALUE);
STRING pvalue_to_string(PVALUE);
void remove_symtab(SYMTAB *);
void set_pvalue(PVALUE, INT, VPTR);
void show_pvalue(PVALUE);
PVALUE symtab_valueofbool(SYMTAB, STRING, BOOLEAN*);
#ifndef HOGMEMORY
void zero_pventry(ENTRY);
#endif

/* Report Interpreter */
void initinterp(void);
void initset(void);
void initrassa(void);
void interp_program_list(STRING, INT, VPTR*, LIST, STRING, BOOLEAN picklist);
void finishinterp(void);
void finishrassa(void);
BOOLEAN set_output_file(STRING outfilename, BOOLEAN append);

INTERPTYPE interpret(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_children(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_spouses(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_families(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_fathers(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_mothers(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_parents(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_fornotes(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_fornodes(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_forindi(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_forsour(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_foreven(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_forothr(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_forfam(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_indisetloop(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_forlist(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_if(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_while(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_call(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_traverse(PNODE, SYMTAB, PVALUE*);

/* Prototypes */
void assign_iden(SYMTAB, STRING, PVALUE);
PNODE break_node(PACTX pactx);
PNODE call_node(PACTX pactx, STRING, PNODE);
PNODE children_node(PACTX pactx, PNODE, STRING, STRING, PNODE);
void clear_rptinfos(void);
PNODE continue_node(PACTX pactx);
void debug_show_one_pnode(PNODE);
PVALUE evaluate(PNODE, SYMTAB, BOOLEAN*);
BOOLEAN evaluate_cond(PNODE, SYMTAB, BOOLEAN*);
PVALUE evaluate_func(PNODE, SYMTAB, BOOLEAN*);
PVALUE evaluate_iden(PNODE, SYMTAB, BOOLEAN*);
PVALUE evaluate_ufunc(PNODE, SYMTAB, BOOLEAN*);
PVALUE eval_and_coerce(INT, PNODE, SYMTAB, BOOLEAN*);
NODE eval_indi(PNODE, SYMTAB, BOOLEAN*, CACHEEL*);
NODE eval_fam(PNODE, SYMTAB, BOOLEAN*, CACHEEL*);
PVALUE eval_without_coerce(PNODE node, SYMTAB stab, BOOLEAN *eflg);
PNODE families_node(PACTX pactx, PNODE, STRING, STRING, STRING, PNODE);
PNODE fathers_node(PACTX pactx, PNODE, STRING, STRING, STRING, PNODE);
PNODE fcons_node(PACTX pactx, FLOAT);
PNODE fdef_node(PACTX pactx, CNSTRING, PNODE, PNODE);
PNODE foreven_node(PACTX pactx, STRING, STRING, PNODE);
PNODE forfam_node(PACTX pactx, STRING, STRING, PNODE);
PNODE forindi_node(PACTX pactx, STRING, STRING, PNODE);
PNODE forindiset_node(PACTX pactx, PNODE, STRING, STRING, STRING, PNODE);
PNODE forlist_node(PACTX pactx, PNODE, STRING, STRING, PNODE);
PNODE fornodes_node(PACTX pactx, PNODE, STRING, PNODE);
PNODE fornotes_node(PACTX pactx, PNODE, STRING, PNODE);
PNODE forothr_node(PACTX pactx, STRING, STRING, PNODE);
PNODE forsour_node(PACTX pactx, STRING, STRING, PNODE);
void free_all_pnodes(void);
void free_pnode_tree(PNODE);
PNODE func_node(PACTX pactx, STRING, PNODE);
PNODE get_proc_node(CNSTRING procname, TABLE loctab, TABLE gtab, INT * count);
RPTINFO get_rptinfo(CNSTRING fullpath);
PNODE icons_node(PACTX pactx, INT ival);
PNODE iden_node(PACTX pactx, STRING);
PNODE if_node(PACTX pactx, PNODE, PNODE, PNODE);
BOOLEAN iistype(PNODE, INT);
void init_interpreter(void);
void interp_load_lang(void);
PNODE make_internal_string_node(PACTX pactx, STRING);
PNODE mothers_node(PACTX pactx, PNODE, STRING, STRING, STRING, PNODE);
INT num_params(PNODE);
void pa_handle_char_encoding(PACTX pactx, PNODE node);
void pa_handle_include(PACTX pactx, PNODE node);
void pa_handle_func(PACTX pactx, CNSTRING funcname, PNODE nd_args, PNODE nd_body);
void pa_handle_global(STRING iden);
void pa_handle_option(PVALUE optval);
void pa_handle_proc(PACTX pactx, CNSTRING procname, PNODE nd_args, PNODE nd_body);
void pa_handle_require(PACTX pactx, PNODE node);
PNODE parents_node(PACTX pactx, PNODE, STRING, STRING, PNODE);
PNODE proc_node(PACTX pactx, CNSTRING, PNODE, PNODE);
void prog_error(PNODE, STRING, ...);
void prog_var_error(PNODE node, SYMTAB stab, PNODE arg, PVALUE val, STRING fmt, ...);
STRING prot(STRING str);
BOOLEAN record_to_node(PVALUE val);
PNODE return_node(PACTX pactx, PNODE);
void set_rptfile_prop(PACTX pactx, STRING fname, STRING key, STRING value);
void show_pnode(PNODE);
void show_pnodes(PNODE);
void shutdown_interpreter(void);
PNODE spouses_node(PACTX pactx, PNODE, STRING, STRING, STRING, PNODE);
PNODE string_node(PACTX pactx, STRING);
void trace_endl(void);
void trace_out(STRING fmt, ...);
void trace_outl(STRING fmt, ...);
void trace_pnode(PNODE node);
void trace_pvalue(PVALUE val);
PNODE traverse_node(PACTX pactx, PNODE, STRING, STRING, PNODE);
PVALUE valueof_iden(PNODE node, SYMTAB stab, STRING iden, BOOLEAN *eflg);
PNODE while_node(PACTX pactx, PNODE, PNODE);

void poutput(STRING, BOOLEAN *eflg);
void interp_main(BOOLEAN picklist);


#endif /* _INTERP_H */
