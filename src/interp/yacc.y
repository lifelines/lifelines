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
 * yacc.y - Grammar of LifeLines programming language
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 31 Aug 93
 *   3.0.0 - 12 Sep 94    3.0.2 - 22 Dec 94
 *===========================================================*/
%{
#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "interp.h"

extern TABLE proctab, functab;
static INTERP this, prev;
extern LIST Plist;
%}

%token  PROC FUNC_TOK IDEN LITERAL CHILDREN SPOUSES IF ELSE ELSIF
%token  FAMILIES ICONS WHILE CALL FORINDISET FORINDI FORNOTES
%token  TRAVERSE FORNODES FORLIST_TOK FORFAM
%token  BREAK CONTINUE RETURN

%%

rspec	:	defn
	|	rspec defn
	;

defn 	:	proc
	|	func
	|	IDEN '(' IDEN ')' {
			if (eqstr("global", (STRING) $1))
				insert_table(globtab, $3, NULL);
		}
	|	IDEN '(' LITERAL ')' {
			if (eqstr("include", (STRING) $1))
				enqueue_list(Plist, iliteral(((INTERP) $3)));
		}
	;

proc	:	PROC IDEN '(' idenso ')' '{' tmplts '}' {
			insert_table(proctab, $2, proc_node($2, $4, $7));
		}

func	:	FUNC_TOK IDEN '(' idenso ')' '{' tmplts '}' {
			insert_table(functab, $2, fdef_node($2, $4, $7));
		}
	;
idenso	:	/* empty */ {
			$$ = 0;
		}
	|	idens {
			$$ = $1;
		}
idens	:	IDEN {
			$$ = (INT) iden_node($1);
		}
	|	IDEN ',' idens {
			$$ = (INT) iden_node($1);
			inext(((INTERP)$$)) = (INTERP) $3;
		}
	;
tmplts	:	tmplt {
			$$ = $1;
		}
	|	tmplts  tmplt {
			join((INTERP) $1, (INTERP) $2);
			$$ = $1;
		}
	;
tmplt	:	CHILDREN m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = (INT) children_node($4, $6, $8, $11);
			((INTERP)$$)->i_line = $2;
		}
	|	SPOUSES m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = spouses_node($4, $6, $8, $10, $13);
			((INTERP)$$)->i_line = $2;
		}
	|	FAMILIES m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = families_node($4, $6, $8, $10, $13);
			((INTERP)$$)->i_line = $2;
		}
	|	FORINDISET m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = forindiset_node($4, $6, $8, $10, $13);
			((INTERP)$$)->i_line = (INT) $2;
		}
	|	FORLIST_TOK m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = forlist_node($4, $6, $8, $11);
			((INTERP)$$)->i_line = (INT) $2;
		}
	|	FORINDI m '(' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			$$ = forindi_node($4, $6, $9);
			((INTERP)$$)->i_line = (INT) $2;
		}
	|	FORFAM m '(' IDEN ',' IDEN ')' '{' tmplts '}' {
			$$ = forfam_node($4, $6, $9);
			((INTERP)$$)->i_line = (INT) $2;
		}
	|	FORNOTES m '(' expr ',' IDEN ')' '{' tmplts '}' {
			$$ = fornotes_node($4, $6, $9);
			((INTERP)$$)->i_line = (INT) $2;
		}
	|	TRAVERSE m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}' {
			$$ = traverse_node($4, $6, $8, $11);
			((INTERP)$$)->i_line = (INT) $2;
		}
	|	FORNODES m '(' expr ',' IDEN ')' '{' tmplts '}' {
			$$ = fornodes_node($4, $6, $9);
			((INTERP)$$)->i_line = (INT) $2;
		}
	|	IF m '(' expr secondo ')' '{' tmplts '}' elsifso elseo {
			inext(((INTERP)$4)) = (INTERP)$5;
			prev = NULL;  this = (INTERP)$10;
			while (this) {
				prev = this;
				this = (INTERP) ielse(this);
			}
			if (prev) {
				ielse(prev) = (WORD)$11;
				$$ = if_node((INTERP)$4, (INTERP)$8,
				    (INTERP)$10);
			} else
				$$ = if_node((INTERP)$4, (INTERP)$8,
				    (INTERP)$11);
			((INTERP)$$)->i_line = (INT) $2;
		}
	|	WHILE m '(' expr secondo ')' '{' tmplts '}' {
			inext(((INTERP)$4)) = (INTERP)$5;
			$$ = while_node($4, $8);
			((INTERP)$$)->i_line = (INT) $2;
		}
	|	CALL IDEN m '(' exprso ')' {
			$$ = call_node($2, $5);
			((INTERP)$$)->i_line = (INT) $3;
		}
	|	BREAK m '(' ')' {
			$$ = break_node();
			((INTERP)$$)->i_line = (INT) $2;
		}
	|	CONTINUE m '(' ')' {
			$$ = continue_node();
			((INTERP)$$)->i_line = (INT) $2;
		}
	|	RETURN m '(' exprso ')' {
			$$ = return_node($4);
			((INTERP)$$)->i_line = (INT) $2;
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
			ielse((INTERP)$1) = (WORD)$2;
			$$ = $1;
		}
	;
elsif	:	ELSIF '(' expr secondo ')' '{' tmplts '}' {
			inext(((INTERP)$3)) = (INTERP)$4;
			$$ = if_node((INTERP)$3, (INTERP)$7, (INTERP)NULL);
		}
elseo	:	/* empty */ {
			$$ = 0;
		}
	|	ELSE '{' tmplts '}' {
			$$ = $3;
		}
	;
expr	:	IDEN {
			$$ = (INT) iden_node($1);
			ielist(((INTERP)$$)) = NULL;
		}
	|	IDEN m '(' exprso ')' {
			$$ = (INT) func_node($1, $4);
			((INTERP)$$)->i_line = $2;
		}
	|	LITERAL {
			$$ = $1;
		}
	|	ICONS {
			$$ = (INT) icons_node($1);
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
			inext(((INTERP)$1)) = (INTERP) $3;
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
			$$ = Plineno;
		}
%%

join (list, last)
INTERP list, last;
{
	INTERP prev = NULL;
	while (list) {
		prev = list;
		list = inext(list);
	}
	ASSERT(prev);
	inext(prev) = last;
}

yyerror (str)
STRING str;
{
	extern INT Plineno;
	extern STRING Pfname;

	wprintf("Syntax Error: %s: line %d\n", Pfname, Plineno);
	Perrors++;
}
