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
/* 07 Jan 2000 Modified by Paul B. McBride pmcbride@tiac.net
 * add "#define YACC_C" so function prototypes could be suppressed.
 * Casts on function arguments and return values need to be added
 * if prototypes are to be used, or pointers and integers are of
 * different sizes. */
/*=============================================================
 * yacc.y - Grammar of LifeLines programming language
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 31 Aug 93
 *   3.0.0 - 12 Sep 94    3.0.2 - 22 Dec 94
 *   3.0.3 - 08 Aug 95
 *===========================================================*/
%{
/*#define YACC_C */
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "indiseq.h"
#include "interp.h"
#include "liflines.h"
#include "screen.h"
#include <stdlib.h>

extern TABLE proctab, functab;
static PNODE this, prev;
extern LIST Plist;
INT Yival;
FLOAT Yfval;

static void join (PNODE list, PNODE last);
static void yyerror (STRING str);

/* Make sure it can hold any pointer */
#define YYSTYPE void *
%}

%token  PROC FUNC_TOK IDEN SCONS CHILDREN SPOUSES IF ELSE ELSIF
%token  FAMILIES ICONS WHILE CALL FORINDISET FORINDI FORNOTES
%token  TRAVERSE FORNODES FORLIST_TOK FORFAM FORSOUR FOREVEN FOROTHR
%token  BREAK CONTINUE RETURN FATHERS MOTHERS PARENTS FCONS

%%

rspec	:	defn
	|	rspec defn
	;

defn 	:	proc
	|	func
	|	IDEN '(' IDEN ')' {
			if (eqstr("global", (STRING) $1))
				insert_pvtable(globtab, (STRING)$3, PANY, NULL);
		}/*HERE*/
	|	IDEN '(' SCONS ')' {
			if (eqstr("include", (STRING) $1))
/*
				enqueue_list(Plist, iliteral(((PNODE) $3)));
*/
enqueue_list(Plist, pvalue((PVALUE) ivalue((PNODE) $3)));

		}
	;

proc	:	PROC IDEN '(' idenso ')' '{' tmplts '}' {
			insert_table(proctab, (STRING)$2, (VPTR)proc_node((STRING)$2, (PNODE)$4, (PNODE)$7));
		}

func	:	FUNC_TOK IDEN '(' idenso ')' '{' tmplts '}' {
			insert_table(functab, (STRING)$2, (VPTR)fdef_node((STRING)$2, (PNODE)$4, (PNODE)$7));
		}
	;
idenso	:	/* empty */ {
			$$ = 0;
		}
	|	idens {
			$$ = $1;
		}
idens	:	IDEN {
			$$ = iden_node((STRING)$1);
		}
	|	IDEN ',' idens {
			$$ = iden_node((STRING)$1);
			inext(((PNODE)$$)) = (PNODE) $3;
		}
	;
tmplts	:	tmplt {
			$$ = $1;
		}
	|	tmplts  tmplt {
			join((PNODE) $1, (PNODE) $2);
			$$ = $1;
		}
	;
tmplt	:	CHILDREN m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = children_node((PNODE)$4, (STRING)$6, (STRING)$8, (PNODE)$11);
			((PNODE)$$)->i_line = (INT)$2;
		}
	|	SPOUSES m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = spouses_node((PNODE)$4, (STRING)$6, (STRING)$8, (STRING)$10, (PNODE)$13);
			((PNODE)$$)->i_line = (INT)$2;
		}
	|	FAMILIES m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = families_node((PNODE)$4, (STRING)$6, (STRING)$8, (STRING)$10, (PNODE)$13);
			((PNODE)$$)->i_line = (INT)$2;
		}
	|	FATHERS m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = fathers_node((PNODE)$4, (STRING)$6, (STRING)$8, (STRING)$10, (PNODE)$13);
			((PNODE)$$)->i_line = (INT)$2;
		}
	|	MOTHERS m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = mothers_node((PNODE)$4, (STRING)$6, (STRING)$8, (STRING)$10, (PNODE)$13);
			((PNODE)$$)->i_line = (INT)$2;
		}
	|	PARENTS m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = parents_node((PNODE)$4, (STRING)$6, (STRING)$8, (PNODE)$11);
			((PNODE)$$)->i_line = (INT)$2;
		}
	|	FORINDISET m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = forindiset_node((PNODE)$4, (STRING)$6, (STRING)$8, (STRING)$10, (PNODE)$13);
			((PNODE)$$)->i_line = (INT) $2;
		}
	|	FORLIST_TOK m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = forlist_node((PNODE)$4, (STRING)$6, (STRING)$8, (PNODE)$11);
			((PNODE)$$)->i_line = (INT) $2;
		}
	|	FORINDI m '(' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = forindi_node((STRING)$4, (STRING)$6, (PNODE)$9);
			((PNODE)$$)->i_line = (INT) $2;
		}
 	|       FORNOTES m '(' expr ',' IDEN ')' '{' tmplts '}' {
                        $$ = fornotes_node((PNODE)$4, (STRING)$6, (PNODE)$9);
                        ((PNODE)$$)->i_line = (INT) $2;
                }
	|	FORFAM m '(' IDEN ',' IDEN ')' '{' tmplts '}' {
			$$ = forfam_node((STRING)$4, (STRING)$6, (PNODE)$9);
			((PNODE)$$)->i_line = (INT) $2;
		}
	|	FORSOUR m '(' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = forsour_node((STRING)$4, (STRING)$6, (PNODE)$9);
			((PNODE)$$)->i_line = (INT) $2;
		}
	|	FOREVEN m '(' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = foreven_node((STRING)$4, (STRING)$6, (PNODE)$9);
			((PNODE)$$)->i_line = (INT) $2;
		}
	|	FOROTHR m '(' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = forothr_node((STRING)$4, (STRING)$6, (PNODE)$9);
			((PNODE)$$)->i_line = (INT) $2;
		}
	|	TRAVERSE m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}' {
			$$ = traverse_node((PNODE)$4, (STRING)$6, (STRING)$8, (PNODE)$11);
			((PNODE)$$)->i_line = (INT) $2;
		}
	|	FORNODES m '(' expr ',' IDEN ')' '{' tmplts '}' {
			$$ = fornodes_node((PNODE)$4, (STRING)$6, (PNODE)$9);
			((PNODE)$$)->i_line = (INT) $2;
		}
	|	IF m '(' expr secondo ')' '{' tmplts '}' elsifso elseo {
			inext(((PNODE)$4)) = (PNODE)$5;
			prev = NULL;  this = (PNODE)$10;
			while (this) {
				prev = this;
				this = (PNODE) ielse(this);
			}
			if (prev) {
				ielse(prev) = (VPTR)$11;
				$$ = if_node((PNODE)$4, (PNODE)$8,
				    (PNODE)$10);
			} else
				$$ = if_node((PNODE)$4, (PNODE)$8,
				    (PNODE)$11);
			((PNODE)$$)->i_line = (INT) $2;
		}
	|	WHILE m '(' expr secondo ')' '{' tmplts '}' {
			inext(((PNODE)$4)) = (PNODE)$5;
			$$ = while_node((PNODE)$4, (PNODE)$8);
			((PNODE)$$)->i_line = (INT) $2;
		}
	|	CALL IDEN m '(' exprso ')' {
			$$ = call_node((STRING)$2, (PNODE)$5);
			((PNODE)$$)->i_line = (INT) $3;
		}
	|	BREAK m '(' ')' {
			$$ = break_node();
			((PNODE)$$)->i_line = (INT) $2;
		}
	|	CONTINUE m '(' ')' {
			$$ = continue_node();
			((PNODE)$$)->i_line = (INT) $2;
		}
	|	RETURN m '(' exprso ')' {
			$$ = return_node((PNODE)$4);
			((PNODE)$$)->i_line = (INT) $2;
		}
	|	expr {
			$$ = $1;
		}
	;
elsifso	:	/* empty */ {
			$$ = 0;
		}
	|	elsifs {
			$$ = $1;
		}
	;
elsifs	:	elsif {
			$$ = $1;
		}
	|	elsif  elsifs {
			ielse((PNODE)$1) = (VPTR)$2;
			$$ = $1;
		}
	;
elsif	:	ELSIF '(' expr secondo ')' '{' tmplts '}' {
			inext(((PNODE)$3)) = (PNODE)$4;
			$$ = if_node((PNODE)$3, (PNODE)$7, (PNODE)NULL);
		}
elseo	:	/* empty */ {
			$$ = 0;
		}
	|	ELSE '{' tmplts '}' {
			$$ = $3;
		}
	;
expr	:	IDEN {
			$$ = iden_node((STRING)$1);
			iargs(((PNODE)$$)) = NULL;
		}
	|	IDEN m '(' exprso ')' {
			$$ = func_node((STRING)$1, (PNODE)$4);
			((PNODE)$$)->i_line = (INT)$2;
		}
	|	SCONS {
			$$ = $1;
		}
	|	ICONS {
			$$ = icons_node(Yival);
		}
	|	FCONS {
			$$ = fcons_node(Yfval);
		}
	;
exprso	:	/* empty */ {
			$$ = 0;
		}
	|	exprs {
			$$ = $1;
		}
	;
exprs	:	expr {
			$$ = $1;
		}
	|	expr ',' exprs {
			inext(((PNODE)$1)) = (PNODE) $3;
			$$ = $1;
		}
	;
secondo	:	/* empty */ {
			$$ = 0;
		}
	|	',' expr {
			$$ = $2;
		}
	;
m	:	/* empty */ {
			$$ = (YYSTYPE)Plineno;
		}
%%

void
join (PNODE list,
      PNODE last)
{
	PNODE prev = NULL;
	while (list) {
		prev = list;
		list = inext(list);
	}
	ASSERT(prev);
	inext(prev) = last;
}

void
yyerror (STRING str)
{
	extern INT Plineno;
	extern STRING Pfname;

	llwprintf("Syntax Error (%s): %s: line %d\n", str, Pfname, Plineno);
	Perrors++;
}
