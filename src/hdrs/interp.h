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
 * interp.h - Header file for report interpreter
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *  2.3.4 - 24 Jun 93   2.3.5 - 16 Aug 93
 *  3.0.0 - 19 Jun 94   3.0.2 - 21 Dec 94
 *===========================================================*/

typedef struct itag *INTERP;
struct itag {
	char   i_type;
	INTERP i_prnt;
	INT    i_line;
	INTERP i_next;
	WORD   i_word1;
	WORD   i_word2;
	WORD   i_word3;
	WORD   i_word4;
	WORD   i_word5;
};

#define ILITERAL   1	/* literal */
#define IIDENT     2	/* identifier */
#define ICHILDREN  3	/* children loop */
#define ISPOUSES   4	/* spouses loop */
#define IIF        7	/* if */
#define IFAMILIES  8	/* families loop */
#define IICONS     9	/* integer constant */
#define IWHILE    10	/* while */
#define IINDICES  12	/* */
#define IINDI     13	/* */
#define INOTES    14	/* */
#define INODES    15	/* */
#define ILIST     16	/* */
#define ITRAV     17	/* traverse */
#define IFAM      18	/* */
#define IBREAK    19	/* break */
#define ICONTINUE 20	/* continue */
#define IRETURN   21	/* return */

#define IPDEFN     5	/* user-defined procedure definition */
#define IPCALL    11	/* user-defined procudure call */
#define IBCALL     6	/* built-in function call */
#define IFDEFN    23    /* user-defined function definition */
#define IFCALL    22	/* user-defined function call */

#define itype(i)     ((i)->i_type)	/* all types */
#define iprnt(i)     ((i)->i_prnt)
#define inext(i)     ((i)->i_next)
#define iline(i)     ((i)->i_line)
#define iliteral(i)  ((i)->i_word1)	/* literal nodes */
#define iident(i)    ((i)->i_word1)	/* ident nodes */
#define ielist(i)    ((i)->i_word2)	/* icons nodes */
#define ifunc(i)     ((i)->i_word3)	/* children nodes */
#define iicons(i)    ((i)->i_word1)
#define ifamily(i)   ((i)->i_word1)
#define ichild(i)    ((i)->i_word2)
#define inum(i)      ((i)->i_word4)
#define ibody(i)     ((i)->i_word5)
#define iprinc(i)    ((i)->i_word1)	/* children, spouse nodes */
#define ispouse(i)   ((i)->i_word2)
#define ifamvar(i)   ((i)->i_word3)
#define iindex(i)    ((i)->i_word1)	/* forindex loop nodes */
#define iindivar(i)  ((i)->i_word2)
#define ivalvar(i)   ((i)->i_word3)
#define iname(i)     ((i)->i_word1)	/* proc and func nodes */
#define iparams(i)   ((i)->i_word2)
#define icond(i)     ((i)->i_word1)	/* if nodes */
#define ithen(i)     ((i)->i_word2)
#define ielse(i)     ((i)->i_word3)
#define iargs(i)     ((i)->i_word2)	/* call and func nodes */
#define inode(i)     ((i)->i_word1)	/* fornotes nodes */
#define istrng(i)    ((i)->i_word2)
#define isubnode(i)  ((i)->i_word2)
#define ilev(i)      ((i)->i_word3)	/* traverse nodes */
#define ilist(i)     ((i)->i_word1)	/* forlist nodes */
#define ielement(i)  ((i)->i_word2)

extern INTERP literal_node();
extern INTERP children_node();
extern INTERP iden_node();
extern INTERP proc_node();

typedef struct {
	char *ft_name;
	INT ft_nparms_min;
	INT ft_nparms_max;
	WORD (*ft_eval)();
} BUILTINS;

extern BUILTINS builtins[];
extern INT nobuiltins;

extern TABLE functab;
extern TABLE globtab;
extern TABLE proctab;

extern WORD evaluate();
extern WORD evaluate_iden();
extern WORD evaluate_func();
extern WORD evaluate_ufunc();
extern WORD evaluate_cond();
extern NODE eval_indi();
extern NODE eval_fam();
extern WORD valueof_iden();
extern STRING get_date();

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
