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
 * lex.c -- Low level lexer of program interpreter
 * Copyright(c) 1992-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 07 Sep 93
 *   3.0.0 - 19 Jun 94    3.0.2 - 04 Jan 95
 *   3.0.3 - 21 Sep 95
 *===========================================================*/

#define YYSTYPE PNODE

#ifdef __OpenBSD__
#define _XOPEN_SOURCE
#define _POSIX_SOURCE
#endif

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interp.h"
#include "yacc.h"

void initlex (int mode);

extern FILE *Pinfp;	/* file that holds program */
extern STRING Pinstr;	/* string that holds program */
extern INT Plineno;	/* program line number */

static INT Lexmode = FILEMODE;
static STRING Lp;	/* pointer into program string */

static int lowyylex(void);
static INT inchar(void);
static void unreadchar(INT);
static BOOLEAN reserved(STRING, INT*);

/*============================
 * initlex -- Initialize lexer
 *==========================*/
void
initlex (int mode)
{
	ASSERT(mode == FILEMODE || mode == STRINGMODE);
	Lexmode = mode;
	if (Lexmode == STRINGMODE) Lp = Pinstr;
}
/*===================================================
 * yylex -- High level lexer function (for debugging)
 *=================================================*/
int
yylex(void)
{
	INT lex = lowyylex();

#ifdef DEBUG
	if (isascii(lex))
		llwprintf("lex<%c> ", lex);
	else
		llwprintf("lex<%d> ", lex);
#endif

	return lex;
}
/*===========================
 * lowyylex -- Lexer function
 *=========================*/
int
lowyylex (void)
{
	INT c=0, t=0, retval, mul;
	extern INT Yival;
	extern FLOAT Yfval;
	extern YYSTYPE yylval;
	static char tokbuf[200];	/* token buffer */
	STRING p = tokbuf;
	while (TRUE) {
		while ((t = chartype(c = inchar())) == WHITE)
			;
		if (c != '/') break;
		if ((c = inchar()) != '*') {
			unreadchar(c);
			return '/';
		}
		/* inside a comment -- advance til end */
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
		unreadchar(c);
#ifdef DEBUG
		llwprintf("in lex.c -- IDEN is %s\n", tokbuf);
#endif
		if (reserved(tokbuf, &retval))  return retval;
		yylval = (PNODE) strsave(tokbuf);
		return IDEN;
	}
	if (t == '-' || t == DIGIT || t == '.') {
		BOOLEAN whole = FALSE;
		BOOLEAN frac = FALSE;
		FLOAT fdiv;
		mul = 1;
		if (t == '-') {
			t = chartype(c = inchar());
			if (t != '.' && t != DIGIT) {
				unreadchar(c);
				return '-';
			}
			mul = -1;
		}
		Yival = 0;
		while (t == DIGIT) {
			whole = TRUE;
			Yival = Yival*10 + c - '0';
			t = chartype(c = inchar());
		}
		if (t != '.') {
			unreadchar(c);
			Yival *= mul;
			yylval = NULL;
			return ICONS;
		}
		t = chartype(c = inchar());
		Yfval = 0.0;
		fdiv = 1.0;
		while (t == DIGIT) {
			frac = TRUE;
			Yfval = Yfval*10 + c - '0';
			fdiv *= 10.;
			t = chartype(c = inchar());
		}
#if 0
		if (frac) unreadchar(c);
#endif
		unreadchar(c);	/* REPLACED BY THIS */
		if (!whole && !frac) {
			unreadchar(c);
			if (mul == -1) {
				unreadchar('.');
				return '-';
			} else
				return '.';
		}
		Yfval = mul*(Yival + Yfval/fdiv);
		yylval = NULL;
		return FCONS;
	}
	if (c == '"') {
		p = tokbuf;
		while (TRUE) {
			while ((c = inchar()) != EOF && c != '"' && c != '\\')
				*p++ = c;
			if (c == 0 || c == '"') {
				*p = 0;
				yylval = string_node(tokbuf);
				return SCONS;
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
				yylval = string_node(tokbuf);
				return SCONS;
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
	{ "break",	BREAK },
	{ "call",	CALL },
	{ "children",	CHILDREN },
	{ "continue",	CONTINUE },
	{ "else",	ELSE },
	{ "elsif",	ELSIF },
	{ "families",	FAMILIES },
	{ "fathers",	FATHERS },
	{ "foreven",	FOREVEN },
	{ "forfam",	FORFAM },
	{ "forindiset",	FORINDISET },
	{ "forindi",	FORINDI },
	{ "forlist",	FORLIST_TOK },
	{ "fornodes",	FORNODES },
	{ "fornotes",	FORNOTES },
	{ "forothr",	FOROTHR },
	{ "forsour",	FORSOUR },
	{ "func",	FUNC_TOK },
	{ "if",		IF },
	{ "mothers",	MOTHERS },
	{ "Parents",	PARENTS },
	{ "proc",	PROC },
	{ "return",	RETURN },
	{ "spouses",	SPOUSES },
	{ "traverse",	TRAVERSE },
	{ "while",	WHILE },
};

INT nrwords = ARRSIZE(rwordtable);

/*======================================
 * reserved -- See if string is reserved
 *====================================*/
BOOLEAN
reserved (STRING word,
          INT *pval)
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
INT
inchar (void)
{
	INT c;
#ifdef SKIPCTRLZ
	do {
#endif
	   if (Lexmode == FILEMODE)
		c = getc(Pinfp);
	   else
		c = *Lp++;
#ifdef SKIPCTRLZ
 	   } while(c == 26);		/* skip CTRL-Z */
#endif
	if (c == '\n') Plineno++;
	return c;
}
/*================================================================
 * unreadchar -- Unread char from input file/string; track line number
 *==============================================================*/
void
unreadchar (INT c)
{
	if (Lexmode == FILEMODE)
		ungetc(c, Pinfp);
	else
		Lp--;
	if (c == '\n') Plineno--;
}
