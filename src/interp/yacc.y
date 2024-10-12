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

/*===========================================================*/
/* Bison Prologue (Part 1)                                   */
/*===========================================================*/

%{
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "indiseq.h"
#include "interpi.h"
#include "liflines.h"
#include "screen.h"
#include <stdlib.h>

static PNODE this, prev;
INT Yival;
FLOAT Yfval;

/* Bison 2.7 */
#define YYSTYPE PNODE

/* Local Functions */
static void join (PNODE list, PNODE last);

/* Global Prototypes (Part 1) */

/* Parser: Provided by Bison in yacc.c (generated from yacc.y) */
int yyparse(PACTX pactx);

/* Parser Error Handler: Provided by LifeLines in interp.c */
void parse_error(PACTX pactx, STRING str);
#define yyerror(pactx, msg) parse_error(pactx, msg)

%}

/*===========================================================*/
/* Bison Declarations (Part 1)                               */
/*===========================================================*/

// Bison 3.x.
%define api.value.type {PNODE}

// Debugging
%verbose
%printer { fprintf (yyo, "IDEN"); } IDEN;
%printer { fprintf (yyo, "PROC"); } PROC;
%printer { fprintf (yyo, "FUNC"); } FUNC_TOK;
%printer { fprintf (yyo, "%g",    $$->vars.ifcons.value->value.fxd); } FCONS;
%printer { fprintf (yyo, FMT_INT, $$->vars.iicons.value->value.ixd); } ICONS;
%printer { fprintf (yyo, "%s",    $$->vars.iscons.value->value.sxd); } SCONS;

/*===========================================================*/
/* Bison Prologue (Part 2)                                   */
/*===========================================================*/

%{

/* Global Prototypes (Part 2) */

/* Lexer: Provided by LifeLines in lex.c */
int yylex(YYSTYPE * lvalp, PACTX pactx);

%}

/*===========================================================*/
/* Bison Declarations (Part 2)                               */
/*===========================================================*/

// Declare pure parser
%define api.pure full

// Additional parameters to parser and lexer
%parse-param {PACTX pactx}
%lex-param {PACTX pactx}

// Tokens
%token  PROC FUNC_TOK IDEN SCONS CHILDREN SPOUSES IF ELSE ELSIF
%token  FAMILIES ICONS WHILE CALL FORINDISET FORINDI FORNOTES
%token  TRAVERSE FORNODES FORLIST_TOK FORFAM FORSOUR FOREVEN FOROTHR
%token  BREAK CONTINUE RETURN FATHERS MOTHERS PARENTS FCONS

/*===========================================================*/
/* Grammar Rules                                             */
/*===========================================================*/

%%

rspec	:	defn
	|	rspec defn
	;

defn 	:	proc
	|	func
	|	IDEN '(' IDEN ')' {
			/* consumes $1 and $3 */
			if (eqstr("global", (STRING) $1))
				pa_handle_global((STRING) $3);
			free_iden($1);
			free_iden($3);
		}
	|	IDEN '(' SCONS ')' {
			/* consumes $1 */
			if (eqstr("include", (STRING) $1))
				pa_handle_include(pactx, (PNODE) $3);
			if (eqstr("option", (STRING) $1))
				pa_handle_option(get_internal_string_node_value((PNODE) $3));
			if (eqstr("char_encoding", (STRING) $1))
				pa_handle_char_encoding(pactx, (PNODE) $3);
			if (eqstr("require", (STRING) $1))
				pa_handle_require(pactx, (PNODE) $3);
			free_iden($1);
		}
	;

proc	:	PROC IDEN '(' idenso ')' '{' tmplts '}' {
			/* consumes $2 */
			pa_handle_proc(pactx, (STRING) $2, (PNODE) $4, (PNODE) $7);
		}

	;
func	:	FUNC_TOK IDEN '(' idenso ')' '{' tmplts '}' {
			/* consumes $2 */
			pa_handle_func(pactx, (STRING) $2, (PNODE) $4, (PNODE) $7);
		}
	;
idenso	:	/* empty */ {
			$$ = 0;
		}
	|	idens {
			$$ = $1;
		}
	;
idens	:	IDEN {
			/* consumes $1 */
			$$ = create_iden_node(pactx, (STRING)$1);
		}
	|	IDEN ',' idens {
			/* consumes $1 */
			$$ = create_iden_node(pactx, (STRING)$1);
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
			/* consumes $6 and $8 */
			$$ = children_node(pactx, (PNODE)$4, (STRING)$6, (STRING)$8, (PNODE)$11);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	SPOUSES m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $6 and $8 */
			$$ = familyspouses_node(pactx, (PNODE)$4, (STRING)$6, (STRING)$8, (PNODE)$11);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	SPOUSES m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $6 and $8 and $10 */
			$$ = spouses_node(pactx, (PNODE)$4, (STRING)$6, (STRING)$8, (STRING)$10, (PNODE)$13);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	FAMILIES m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $6 and $8 and $10 */
			$$ = families_node(pactx, (PNODE)$4, (STRING)$6, (STRING)$8, (STRING)$10, (PNODE)$13);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	FATHERS m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $6 and $8 and $10 */
			$$ = fathers_node(pactx, (PNODE)$4, (STRING)$6, (STRING)$8, (STRING)$10, (PNODE)$13);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	MOTHERS m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $6 and $8 and $10 */
			$$ = mothers_node(pactx, (PNODE)$4, (STRING)$6, (STRING)$8, (STRING)$10, (PNODE)$13);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	PARENTS m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $6 and $8 */
			$$ = parents_node(pactx, (PNODE)$4, (STRING)$6, (STRING)$8, (PNODE)$11);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	FORINDISET m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $6 and $8 and $10 */
			$$ = forindiset_node(pactx, (PNODE)$4, (STRING)$6, (STRING)$8, (STRING)$10, (PNODE)$13);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	FORLIST_TOK m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $6 and $8 */
			$$ = forlist_node(pactx, (PNODE)$4, (STRING)$6, (STRING)$8, (PNODE)$11);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	FORINDI m '(' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $4 and $6 */
			$$ = forindi_node(pactx, (STRING)$4, (STRING)$6, (PNODE)$9);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	FORNOTES m '(' expr ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $6 */
			$$ = fornotes_node(pactx, (PNODE)$4, (STRING)$6, (PNODE)$9);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	FORFAM m '(' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $4 and $6 */
			$$ = forfam_node(pactx, (STRING)$4, (STRING)$6, (PNODE)$9);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	FORSOUR m '(' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $4 and $6 */
			$$ = forsour_node(pactx, (STRING)$4, (STRING)$6, (PNODE)$9);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	FOREVEN m '(' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $4 and $6 */
			$$ = foreven_node(pactx, (STRING)$4, (STRING)$6, (PNODE)$9);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	FOROTHR m '(' IDEN ',' IDEN ')' '{' tmplts '}'
		{
			/* consumes $4 and $6 */
			$$ = forothr_node(pactx, (STRING)$4, (STRING)$6, (PNODE)$9);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	TRAVERSE m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}' {
			/* consumes $6 and $8 */
			$$ = traverse_node(pactx, (PNODE)$4, (STRING)$6, (STRING)$8, (PNODE)$11);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	FORNODES m '(' expr ',' IDEN ')' '{' tmplts '}' {
			/* consumes $6 */
			$$ = fornodes_node(pactx, (PNODE)$4, (STRING)$6, (PNODE)$9);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	IF m '(' expr secondo ')' '{' tmplts '}' elsifso elseo {
			inext(((PNODE)$4)) = (PNODE)$5;
			prev = NULL;  this = (PNODE)$10;
			while (this) {
				prev = this;
				this = this->vars.iif.ielse;
			}
			if (prev) {
				prev->vars.iif.ielse = (VPTR)$11;
				$$ = if_node(pactx, (PNODE)$4, (PNODE)$8,
				    (PNODE)$10);
			} else
				$$ = if_node(pactx, (PNODE)$4, (PNODE)$8,
				    (PNODE)$11);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	WHILE m '(' expr secondo ')' '{' tmplts '}' {
			inext(((PNODE)$4)) = (PNODE)$5;
			$$ = while_node(pactx, (PNODE)$4, (PNODE)$8);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	CALL IDEN m '(' exprso ')' {
			/* consumes $2 */
			$$ = create_call_node(pactx, (STRING)$2, (PNODE)$5);
			((PNODE)$$)->i_line = (INTPTR)$3;
		}
	|	BREAK m '(' ')' {
			$$ = break_node(pactx);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	CONTINUE m '(' ')' {
			$$ = continue_node(pactx);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	RETURN m '(' exprso ')' {
			$$ = return_node(pactx, (PNODE)$4);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	expro {
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
			((PNODE)$1)->vars.iif.ielse = $2;
			$$ = $1;
		}
	;
elsif	:	ELSIF '(' expr secondo ')' '{' tmplts '}' {
			inext(((PNODE)$3)) = (PNODE) $4;
			$$ = if_node(pactx, (PNODE)$3, (PNODE)$7, (PNODE)NULL);
		}
	;
elseo	:	/* empty */ {
			$$ = 0;
		}
	|	ELSE '{' tmplts '}' {
			$$ = $3;
		}
	;
expr	:	IDEN {
			/* consumes $1 */
			$$ = create_iden_node(pactx, (STRING)$1);
		}
	|	IDEN m '(' exprso ')' {
			/* consumes $1 */
			$$ = func_node(pactx, (STRING)$1, (PNODE)$4);
			((PNODE)$$)->i_line = (INTPTR)$2;
		}
	|	SCONS {
			$$ = $1;
		}
	|	ICONS {
			$$ = create_icons_node(pactx, Yival);
		}
	|	FCONS {
			$$ = create_fcons_node(pactx, Yfval);
		}
	;
expro	:	/* empty */ {
			$$ = 0;
		}
	|	expr {
			$$ = $1;
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
			$$ = (YYSTYPE)(INTPTR)pactx->lineno;
		}
	;
%%

/*===========================================================*/
/* Bison Epilogue                                            */
/*===========================================================*/

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

