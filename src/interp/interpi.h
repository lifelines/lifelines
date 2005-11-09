/*
   Copyright (c) 2000 Petter Reinholdtsen

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
/*======================================================================
 * interp-priv.h - Internal header file for report generator interpreter
 * Copyright(c) 2000 by Petter Reinholdtsen; all rights reserved
 *====================================================================*/

#ifndef _INTERP_PRIV_H
#define _INTERP_PRIV_H

#include "pvalue.h"
#include "interp.h"

#define LIFELINES_REPORTS_VERSION "1.3"

/************************************************************************/
/* Parser Structures and Functions                                      */
/************************************************************************/

/* parse global context */
struct tag_pactx {
	FILE *Pinfp;     /* file to read program from */
	STRING Pinstr;   /* string to read program from */
	TABLE filetab;   /* table of files called by current report (incl. itself) */
	                 /* a filetab entry holds optional table of string properties */
	STRING ifile;    /* user's requested program path (current report) */
	STRING fullpath; /* actual path of current program */
	INT lineno;      /* current line number (0-based) */
	INT charpos;     /* current offset on line (0-based) */
};
typedef struct tag_pactx *PACTX;

typedef struct tag_pathinfo {
	STRING fname;    /* filename as user specified */
	STRING fullpath; /* fully qualified path */
} *PATHINFO;

struct tag_rptinfo {
	struct tag_vtable *vtable; /* generic object */
	INT refcnt; /* ref-countable object */
	STRING fullpath; /* fully qualified path to file */
	ZSTR localpath;  /* directory file is in */
	ZSTR localepath; /* directory for message catalogs */
	ZSTR textdomain; /* domain for message catalog for this report */
	TABLE proctab;   /* all procs in this file */
	TABLE functab;   /* all funcs in this file */
	STRING codeset;  /* codeset of report */
};
typedef struct tag_rptinfo *RPTINFO;

/************************************************************************/
/* Symbol Table Structures and Prototypes                               */
/************************************************************************/


/* symbol table data is just table data */
typedef struct tag_symtab {
	TABLE tab;
} *SYMTAB;

SYMTAB create_symtab(void);
void delete_symtab_element(SYMTAB stab, STRING iden);
BOOLEAN in_symtab(SYMTAB stab, STRING key);
void insert_symtab(SYMTAB stab, STRING iden, PVALUE val);
void remove_symtab(SYMTAB stab);
void symbol_tables_end(void);
PVALUE symtab_valueofbool(SYMTAB, STRING, BOOLEAN*);


/* symbol table iteration */
typedef struct tag_symtab_iter *SYMTAB_ITER;
SYMTAB_ITER begin_symtab_iter(SYMTAB stab);
BOOLEAN next_symtab_entry(SYMTAB_ITER tabit, CNSTRING *pkey, PVALUE *ppval);
void end_symtab_iter(SYMTAB_ITER * psymtabit);

/************************************************************************/
/* Interpreter Structures and Functions                                 */
/************************************************************************/

typedef struct tag_pnode *PNODE;
struct tag_pnode {
	char     i_type;       /* type of node */
	PNODE    i_prnt;       /* parent of this node */
	INT      i_line;       /* line where text of this node begins */
	RPTINFO  i_rptinfo;    /* information about this report file */
	INT      i_flags;
	PNODE    i_next;       /* next node */
	VPTR     i_word1;      /* variable data associated with node type */
	VPTR     i_word2;      /* ... */
	VPTR     i_word3;
	VPTR     i_word4;
	VPTR     i_word5;
};

/* pnode types */
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
#define IPARENTUNIT 32   /* parentunit loop */
#define IFREED      99   /* returned to free list */

/* pnode flags */
enum {
	PN_IVALUEX_PVALUE = 0x1 /* ivaluex is a pvalue */
	, PN_INAME_HSTR = 0x2 /* iname is a heap string */
	, PN_ICHILD_HPTR = 0x4 /* ichild is a heap string */
	, PN_INUM_HPTR = 0x8 /* inum is a heap string */
	, PN_ISPOUSE_HPTR = 0x10 /* ispouse is a heap string */
	, PN_IFAMILY_HPTR = 0x20 /* ifamily is a heap string */
	, PN_IELEMENT_HPTR = 0x40 /* ielement is a heap string */
	, PN_IPARENT_HPTR = 0x80 /* iiparent is a heap string */
	, PN_IVALVAR_HPTR = 0x100 /* ivalvar is a heap string */
	, PN_IIDENT_HSTR = 0x200 /* iident is a heap string */
};

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
#define iiparent(i)  ((i)->i_word2)     /* var in some families type loops */
#define ivalvar(i)   ((i)->i_word3)     /* var in indiset loop */
#define iname(i)     ((i)->i_word1)     /* proc, func and builtin names */
#define ilev(i)      ((i)->i_word3)     /* var traverse loop */

#define iloopexp(i)  ((i)->i_word1)     /* top loop expression */
#define ielement(i)  ((i)->i_word2)     /* loop element */
#define ibody(i)     ((i)->i_word5)     /* body of proc, func, loops */
#define inum(i)      ((i)->i_word4)     /* counter used by many loops */

typedef PVALUE (*PFUNC)(PNODE, SYMTAB, BOOLEAN *);

#define pitype(i)	ptype(ivalue(i))
#define pivalue(i)	pvalue(ivalue(i))

typedef struct {
	char *ft_name;
	INT ft_nparms_min;
	INT ft_nparms_max;
	PVALUE (*ft_eval)(PNODE, SYMTAB, BOOLEAN *);
} BUILTINS;

#define INTERPTYPE INT
#define INTERROR    0
#define INTOKAY     1
#define INTBREAK    2
#define INTCONTINUE 3
#define INTRETURN   4

void initinterp(void);
void initrassa(void);
void finishinterp(void);
void finishrassa(void);

INTERPTYPE interpret(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_children(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_spouses(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_families(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_fathers(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_mothers(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_parents(PNODE, SYMTAB, PVALUE*);
INTERPTYPE interp_parentunit(PNODE, SYMTAB, PVALUE*);
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

/************************************************************************/
/* Language Keyword Implementations                                     */
/************************************************************************/

PVALUE ___alpha(PNODE, SYMTAB, BOOLEAN *);
PVALUE __add(PNODE, SYMTAB, BOOLEAN *);
PVALUE __addnode(PNODE, SYMTAB, BOOLEAN *);
PVALUE __addtoset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __ancestorset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __and(PNODE, SYMTAB, BOOLEAN *);
PVALUE __bapt(PNODE, SYMTAB, BOOLEAN *);
PVALUE __birt(PNODE, SYMTAB, BOOLEAN *);
PVALUE __buri(PNODE, SYMTAB, BOOLEAN *);
PVALUE __bytecode(PNODE, SYMTAB, BOOLEAN *);
PVALUE __capitalize(PNODE, SYMTAB, BOOLEAN *);
PVALUE __card(PNODE, SYMTAB, BOOLEAN *);
PVALUE __child(PNODE, SYMTAB, BOOLEAN *);
PVALUE __childset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __choosechild(PNODE, SYMTAB, BOOLEAN *);
PVALUE __choosefam(PNODE, SYMTAB, BOOLEAN *);
PVALUE __chooseindi(PNODE, SYMTAB, BOOLEAN *);
PVALUE __choosespouse(PNODE, SYMTAB, BOOLEAN *);
PVALUE __choosesubset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __clear(PNODE, SYMTAB, BOOLEAN *);
PVALUE __col(PNODE, SYMTAB, BOOLEAN *);
PVALUE __complexdate(PNODE, SYMTAB, BOOLEAN *);
PVALUE __complexformat(PNODE, SYMTAB, BOOLEAN *);
PVALUE __complexpic(PNODE, SYMTAB, BOOLEAN *);
PVALUE __concat(PNODE, SYMTAB, BOOLEAN *);
PVALUE __convertcode(PNODE, SYMTAB, BOOLEAN *);
PVALUE __copyfile(PNODE, SYMTAB, BOOLEAN *);
PVALUE __createnode(PNODE, SYMTAB, BOOLEAN *);
PVALUE __d(PNODE, SYMTAB, BOOLEAN *);
PVALUE __database(PNODE, SYMTAB, BOOLEAN *);
PVALUE __date(PNODE, SYMTAB, BOOLEAN *);
PVALUE __dateformat(PNODE, SYMTAB, BOOLEAN *);
PVALUE __datepic(PNODE, SYMTAB, BOOLEAN *);
PVALUE __dayformat(PNODE, SYMTAB, BOOLEAN *);
PVALUE __deat(PNODE, SYMTAB, BOOLEAN *);
PVALUE __debug(PNODE, SYMTAB, BOOLEAN *);
PVALUE __decr(PNODE, SYMTAB, BOOLEAN *);
PVALUE __deletefromset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __dequeue(PNODE, SYMTAB, BOOLEAN *);
PVALUE __descendentset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __detachnode(PNODE, SYMTAB, BOOLEAN *);
PVALUE __difference(PNODE, SYMTAB, BOOLEAN *);
PVALUE __div(PNODE, SYMTAB, BOOLEAN *);
PVALUE __dup(PNODE, SYMTAB, BOOLEAN *);
PVALUE __empty(PNODE, SYMTAB, BOOLEAN *);
PVALUE __enqueue(PNODE, SYMTAB, BOOLEAN *);
PVALUE __eq(PNODE, SYMTAB, BOOLEAN *);
PVALUE __eqstr(PNODE, SYMTAB, BOOLEAN *);
PVALUE __exp(PNODE, SYMTAB, BOOLEAN *);
PVALUE __extractdate(PNODE, SYMTAB, BOOLEAN *);
PVALUE __extractdatestr(PNODE, SYMTAB, BOOLEAN *);
PVALUE __extractnames(PNODE, SYMTAB, BOOLEAN *);
PVALUE __extractplaces(PNODE, SYMTAB, BOOLEAN *);
PVALUE __extracttokens(PNODE, SYMTAB, BOOLEAN *);
PVALUE __f(PNODE, SYMTAB, BOOLEAN *);
PVALUE __fam(PNODE, SYMTAB, BOOLEAN *);
PVALUE __fath(PNODE, SYMTAB, BOOLEAN *);
PVALUE __female(PNODE, SYMTAB, BOOLEAN *);
PVALUE __firstchild(PNODE, SYMTAB, BOOLEAN *);
PVALUE __firstfam(PNODE, SYMTAB, BOOLEAN *);
PVALUE __firstindi(PNODE, SYMTAB, BOOLEAN *);
PVALUE __float(PNODE, SYMTAB, BOOLEAN *);
PVALUE __fnode(PNODE, SYMTAB, BOOLEAN *);
PVALUE __free(PNODE, SYMTAB, BOOLEAN *);
PVALUE __free(PNODE, SYMTAB, BOOLEAN *);
PVALUE __fullname(PNODE, SYMTAB, BOOLEAN *);
PVALUE __ge(PNODE, SYMTAB, BOOLEAN *);
PVALUE __gengedcom(PNODE, SYMTAB, BOOLEAN *);
PVALUE __gengedcomweak(PNODE, SYMTAB, BOOLEAN *);
PVALUE __gengedcomstrong(PNODE, SYMTAB, BOOLEAN *);
PVALUE __genindiset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __getcol(PNODE, SYMTAB, BOOLEAN *);
PVALUE __getel(PNODE, SYMTAB, BOOLEAN *);
PVALUE __getfam(PNODE, SYMTAB, BOOLEAN *);
PVALUE __getindi(PNODE, SYMTAB, BOOLEAN *);
PVALUE __getindiset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __getint(PNODE, SYMTAB, BOOLEAN *);
PVALUE __getproperty(PNODE, SYMTAB, BOOLEAN *);
PVALUE __dereference(PNODE, SYMTAB, BOOLEAN *);
PVALUE __getstr(PNODE, SYMTAB, BOOLEAN *);
PVALUE __gettext(PNODE, SYMTAB, BOOLEAN *);
PVALUE __gettoday(PNODE, SYMTAB, BOOLEAN *);
PVALUE __givens(PNODE, SYMTAB, BOOLEAN *);
PVALUE __gt(PNODE, SYMTAB, BOOLEAN *);
PVALUE __heapused(PNODE, SYMTAB, BOOLEAN *);
PVALUE __husband(PNODE, SYMTAB, BOOLEAN *);
PVALUE __incr(PNODE, SYMTAB, BOOLEAN *);
PVALUE __index(PNODE, SYMTAB, BOOLEAN *);
PVALUE __indi(PNODE, SYMTAB, BOOLEAN *);
PVALUE __indiset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __inlist(PNODE, SYMTAB, BOOLEAN *);
PVALUE __inlist(PNODE, SYMTAB, BOOLEAN *);
PVALUE __inode(PNODE, SYMTAB, BOOLEAN *);
PVALUE __insert(PNODE, SYMTAB, BOOLEAN *);
PVALUE __inset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __int(PNODE, SYMTAB, BOOLEAN *);
PVALUE __intersect(PNODE, SYMTAB, BOOLEAN *);
PVALUE __key(PNODE, SYMTAB, BOOLEAN *);
PVALUE __keysort(PNODE, SYMTAB, BOOLEAN *);
PVALUE __lastchild(PNODE, SYMTAB, BOOLEAN *);
PVALUE __lastfam(PNODE, SYMTAB, BOOLEAN *);
PVALUE __lastindi(PNODE, SYMTAB, BOOLEAN *);
PVALUE __le(PNODE, SYMTAB, BOOLEAN *);
PVALUE __length(PNODE, SYMTAB, BOOLEAN *);
PVALUE __lengthset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __level(PNODE, SYMTAB, BOOLEAN *);
PVALUE __linemode(PNODE, SYMTAB, BOOLEAN *);
PVALUE __list(PNODE, SYMTAB, BOOLEAN *);
PVALUE __lock(PNODE, SYMTAB, BOOLEAN *);
PVALUE __long(PNODE, SYMTAB, BOOLEAN *);
PVALUE __lookup(PNODE, SYMTAB, BOOLEAN *);
PVALUE __lower(PNODE, SYMTAB, BOOLEAN *);
PVALUE __lt(PNODE, SYMTAB, BOOLEAN *);
PVALUE __male(PNODE, SYMTAB, BOOLEAN *);
PVALUE __marr(PNODE, SYMTAB, BOOLEAN *);
PVALUE __menuchoose(PNODE, SYMTAB, BOOLEAN *);
PVALUE __mod(PNODE, SYMTAB, BOOLEAN *);
PVALUE __monthformat(PNODE, SYMTAB, BOOLEAN *);
PVALUE __moth(PNODE, SYMTAB, BOOLEAN *);
PVALUE __mul(PNODE, SYMTAB, BOOLEAN *);
PVALUE __name(PNODE, SYMTAB, BOOLEAN *);
PVALUE __namesort(PNODE, SYMTAB, BOOLEAN *);
PVALUE __nchildren(PNODE, SYMTAB, BOOLEAN *);
PVALUE __ne(PNODE, SYMTAB, BOOLEAN *);
PVALUE __neg(PNODE, SYMTAB, BOOLEAN *);
PVALUE __nestr(PNODE, SYMTAB, BOOLEAN *);
PVALUE __newfile(PNODE, SYMTAB, BOOLEAN *);
PVALUE __nextfam(PNODE, SYMTAB, BOOLEAN *);
PVALUE __nextindi(PNODE, SYMTAB, BOOLEAN *);
PVALUE __nextsib(PNODE, SYMTAB, BOOLEAN *);
PVALUE __nfamilies(PNODE, SYMTAB, BOOLEAN *);
PVALUE __nl(PNODE, SYMTAB, BOOLEAN *);
PVALUE __not(PNODE, SYMTAB, BOOLEAN *);
PVALUE __nspouses(PNODE, SYMTAB, BOOLEAN *);
PVALUE __or(PNODE, SYMTAB, BOOLEAN *);
PVALUE __ord(PNODE, SYMTAB, BOOLEAN *);
PVALUE __eraformat(PNODE, SYMTAB, BOOLEAN *);
PVALUE __outfile(PNODE, SYMTAB, BOOLEAN *);
PVALUE __pagemode(PNODE, SYMTAB, BOOLEAN *);
PVALUE __pageout(PNODE, SYMTAB, BOOLEAN *);
PVALUE __parent(PNODE, SYMTAB, BOOLEAN *);
PVALUE __parents(PNODE, SYMTAB, BOOLEAN *);
PVALUE __parentset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __place(PNODE, SYMTAB, BOOLEAN *);
PVALUE __pn(PNODE, SYMTAB, BOOLEAN *);
PVALUE __pop(PNODE, SYMTAB, BOOLEAN *);
PVALUE __pos(PNODE, SYMTAB, BOOLEAN *);
PVALUE __prevfam(PNODE, SYMTAB, BOOLEAN *);
PVALUE __previndi(PNODE, SYMTAB, BOOLEAN *);
PVALUE __prevsib(PNODE, SYMTAB, BOOLEAN *);
PVALUE __print(PNODE, SYMTAB, BOOLEAN *);
PVALUE __program(PNODE, SYMTAB, BOOLEAN *);
PVALUE __push(PNODE, SYMTAB, BOOLEAN *);
PVALUE __pvalue(PNODE, SYMTAB, BOOLEAN *);
PVALUE __qt(PNODE, SYMTAB, BOOLEAN *);
PVALUE __reference(PNODE, SYMTAB, BOOLEAN *);
PVALUE __requeue(PNODE, SYMTAB, BOOLEAN *);
PVALUE __rjustify(PNODE, SYMTAB, BOOLEAN *);
PVALUE __roman(PNODE, SYMTAB, BOOLEAN *);
PVALUE __rot(PNODE, SYMTAB, BOOLEAN *);
PVALUE __row(PNODE, SYMTAB, BOOLEAN *);
PVALUE __rsort(PNODE, SYMTAB, BOOLEAN *);
PVALUE __save(PNODE, SYMTAB, BOOLEAN *);
PVALUE __savenode(PNODE, SYMTAB, BOOLEAN *);
PVALUE __set(PNODE, SYMTAB, BOOLEAN *);
PVALUE __setel(PNODE, SYMTAB, BOOLEAN *);
PVALUE __setlocale(PNODE, SYMTAB, BOOLEAN *);
PVALUE __sex(PNODE, SYMTAB, BOOLEAN *);
PVALUE __short(PNODE, SYMTAB, BOOLEAN *);
PVALUE __sibling(PNODE, SYMTAB, BOOLEAN *);
PVALUE __siblingset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __sort(PNODE, SYMTAB, BOOLEAN *);
PVALUE __soundex(PNODE, SYMTAB, BOOLEAN *);
PVALUE __space(PNODE, SYMTAB, BOOLEAN *);
PVALUE __spouseset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __stddate(PNODE, SYMTAB, BOOLEAN *);
PVALUE __strcmp(PNODE, SYMTAB, BOOLEAN *);
PVALUE __strlen(PNODE, SYMTAB, BOOLEAN *);
PVALUE __strsoundex(PNODE, SYMTAB, BOOLEAN *);
PVALUE __strtoint(PNODE, SYMTAB, BOOLEAN *);
PVALUE __sub(PNODE, SYMTAB, BOOLEAN *);
PVALUE __substring(PNODE, SYMTAB, BOOLEAN *);
PVALUE __surname(PNODE, SYMTAB, BOOLEAN *);
PVALUE __runsystem(PNODE, SYMTAB, BOOLEAN *);
PVALUE __table(PNODE, SYMTAB, BOOLEAN *);
PVALUE __tag(PNODE, SYMTAB, BOOLEAN *);
PVALUE __test(PNODE, SYMTAB, BOOLEAN *);
PVALUE __titl(PNODE, SYMTAB, BOOLEAN *);
PVALUE __titlcase(PNODE, SYMTAB, BOOLEAN *);
PVALUE __trim(PNODE, SYMTAB, BOOLEAN *);
PVALUE __trimname(PNODE, SYMTAB, BOOLEAN *);
PVALUE __union(PNODE, SYMTAB, BOOLEAN *);
PVALUE __uniqueset(PNODE, SYMTAB, BOOLEAN *);
PVALUE __unlock(PNODE, SYMTAB, BOOLEAN *);
PVALUE __upper(PNODE, SYMTAB, BOOLEAN *);
PVALUE __value(PNODE, SYMTAB, BOOLEAN *);
PVALUE __valuesort(PNODE, SYMTAB, BOOLEAN *);
PVALUE __version(PNODE, SYMTAB, BOOLEAN *);
PVALUE __wife(PNODE, SYMTAB, BOOLEAN *);
PVALUE __writefam(PNODE, SYMTAB, BOOLEAN *);
PVALUE __writeindi(PNODE, SYMTAB, BOOLEAN *);
PVALUE __xref(PNODE, SYMTAB, BOOLEAN *);
PVALUE __year(PNODE, SYMTAB, BOOLEAN *);
PVALUE __yearformat(PNODE, SYMTAB, BOOLEAN *);

/* **************************************************************** */

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

#define TYPE_CHECK(t, v) \
	if (ptype(v) != t) {\
		*eflg = TRUE;\
		return NULL;\
	}

void dolock_node_in_cache(NODE, BOOLEAN lock);

/* Prototypes */
void assign_iden(SYMTAB, STRING, PVALUE);
PNODE break_node(PACTX pactx);
PNODE call_node(PACTX pactx, STRING, PNODE);
PNODE children_node(PACTX pactx, PNODE, STRING, STRING, PNODE);
void clear_rptinfos(void);
PNODE continue_node(PACTX pactx);
void debug_show_one_pnode(PNODE);
CNSTRING get_pvalue_type_name(INT ptype);
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
void free_iden(void *iden);
void free_all_pnodes(void);
void free_pnode_tree(PNODE);
PNODE func_node(PACTX pactx, STRING, PNODE);
PNODE get_proc_node(CNSTRING procname, TABLE loctab, TABLE gtab, INT * count);
RPTINFO get_rptinfo(CNSTRING fullpath);
PNODE icons_node(PACTX pactx, INT ival);
PNODE iden_node(PACTX pactx, STRING);
PNODE if_node(PACTX pactx, PNODE, PNODE, PNODE);
BOOLEAN iistype(PNODE, INT);
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
PNODE parentunit_node(PACTX pactx, PNODE, STRING, STRING, PNODE);
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
PNODE spouses_node(PACTX pactx, PNODE, STRING, STRING, STRING, PNODE);
BOOLEAN start_output_file (STRING outfname);
PNODE string_node(PACTX pactx, STRING);
void trace_endl(void);
void trace_out(STRING fmt, ...);
void trace_outl(STRING fmt, ...);
void trace_pnode(PNODE node);
void trace_pvalue(PVALUE val);
PNODE traverse_node(PACTX pactx, PNODE, STRING, STRING, PNODE);
PVALUE valueof_iden(PNODE node, SYMTAB stab, STRING iden, BOOLEAN *eflg);
PNODE while_node(PACTX pactx, PNODE, PNODE);

#endif /* _INTERP_PRIV_H */
