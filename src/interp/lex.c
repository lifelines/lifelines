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
 * lex.c -- Low level lexing code of report interpreter
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 07 Sep 93
 *   3.0.0 - 19 Jun 94    3.0.2 - 04 Jan 95
 *===========================================================*/

#define YYSTYPE INTERP

#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "interp.h"
#include "yacc.h"

extern FILE *Pinfp;	/* file that holds program */
extern STRING Pinstr;	/* string that holds program */
extern INT Plineno;	/* program line number */

static INT Lexmode = FILEMODE;
static STRING Lp;	/* pointer into program string */

/*============================
 * initlex -- Initialize lexer
 *==========================*/
initlex (mode)
{
	ASSERT(mode == FILEMODE || mode == STRINGMODE);
	Lexmode = mode;
	if (Lexmode == STRINGMODE) Lp = Pinstr;
}
/*===================================================
 * yylex -- High level lexer function (for debugging)
 *=================================================*/
int yylex()
{
	INT lex = lowyylex();

/* Place token based debugging here */

	return lex;
}
/*===========================
 * lowyylex -- Lexer function
 *=========================*/
int lowyylex ()
{
	INT c, t, retval, ivalue;
	static unsigned char tokbuf[200];	/* token buffer */
	STRING p = tokbuf;
	while (TRUE) {
		while ((t = chartype(c = inchar())) == WHITE)
			;
		if (c != '/') break;
		if ((c = inchar()) != '*') {
			unchar(c);
			return '/';
		}
		while (TRUE) {
			while ((c = inchar()) != '*' && c != EOF)
				;
			if (c == EOF) return 0;
			while ((c = inchar()) == '*')
				;
			if (c == '/') break;
			if (c == EOF) return 0;
		}
	}
	if (t == LETTER) {
		p = tokbuf;
		while (t == LETTER || t == DIGIT || c == '_') {
			*p++ = c;
			t = chartype(c = inchar());
		}
		*p = 0;
		unchar(c);
		if (reserved(tokbuf, &retval)) return retval;
		yylval = (YYSTYPE) strsave(tokbuf);
		return IDEN;
	}
	if (t == DIGIT) {
		ivalue = 0;
		while (t == DIGIT) {
			ivalue = ivalue*10 + c - '0';
			t = chartype(c = inchar());
		}
		unchar(c);
		yylval = (YYSTYPE) ivalue;
		return ICONS;
	}
	if (c == '"') {
		p = tokbuf;
		while (TRUE) {
			while ((c = inchar()) != EOF && c != '"' && c != '\\')
				*p++ = c;
			if (c == 0 || c == '"') {
				*p = 0;
				yylval = (YYSTYPE) literal_node(tokbuf);
				return LITERAL;
			}
			switch (c = inchar()) {
			case 'n': *p++ = '\n'; break;
			case 't': *p++ = '\t'; break;
			case 'v': *p++ = '\v'; break;
			case 'r': *p++ = '\r'; break;
			case 'b': *p++ = '\b'; break;
			case 'f': *p++ = '\f'; break;
			case '"': *p++ = '"'; break;
			case '\\': *p++ = '\\'; break;
			case EOF:
				*p = 0;
				yylval = (YYSTYPE) literal_node(tokbuf);
				return LITERAL;
			default:
				*p++ = c; break;
			}
		}
	}
	if (c == EOF) return 0;
	return c;
}
/*==================================
 * rwordtable -- Reserved word table
 *================================*/
static struct {
	char *rword;
	INT val;
} rwordtable[] = {
	"break",	BREAK,
	"call",		CALL,
	"children",	CHILDREN,
	"continue",	CONTINUE,
	"else",		ELSE,
	"elsif",	ELSIF,
	"families",	FAMILIES,
	"forfam",	FORFAM,
	"forindiset",	FORINDISET,
	"forindi",	FORINDI,
	"forlist",	FORLIST_TOK,
	"fornodes",	FORNODES,
	"fornotes",	FORNOTES,
	"func",		FUNC_TOK,
	"if",		IF,
	"proc",		PROC,
	"return",	RETURN,
	"spouses",	SPOUSES,
	"traverse",	TRAVERSE,
	"while",	WHILE,
};
static nrwords = 20;
/*===========================================
 * reserved -- See if string is reserved word
 *=========================================*/
BOOLEAN reserved (word, pval)
STRING word;
INT *pval;
{
	INT i;
	for (i = 0; i < nrwords; i++) {
		if (eqstr(word, rwordtable[i].rword)) {
			*pval = rwordtable[i].val;
			return TRUE;
		}
	}
	return FALSE;
}
/*==============================================================
 * inchar -- Read char from input file/string; track line number
 *============================================================*/
inchar ()
{
	INT c;
	if (Lexmode == FILEMODE)
		c = getc(Pinfp);
	else
		c = *Lp++;
	if (c == '\n') Plineno++;
	return c;
}
/*================================================================
 * unchar -- Unread char from input file/string; track line number
 *==============================================================*/
unchar (c)
INT c;
{
	if (Lexmode == FILEMODE)
		ungetc(c, Pinfp);
	else
		Lp--;
	if (c == '\n') Plineno--;
}
