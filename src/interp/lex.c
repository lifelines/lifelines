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

/*#define YYSTYPE PNODE*/

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
#include "parse.h"
#include "yacc.h"

static INT Lexmode = FILEMODE;
static STRING Lp;	/* pointer into program string */


static INT inchar(void *pactx);
static int lowyylex(void *pactx, YYSTYPE * lvalp);
static BOOLEAN reserved(STRING, INT*);
static void unreadchar(void *pactx, INT c);

/*============================
 * initlex -- Initialize lexer
 * TODO: 2002.07.14, Perry: This is not used -- find out intent
 *==========================*/
#if UNUSED_CODE
void
initlex (struct parseinfo *pinfo, int mode)
{
	ASSERT(mode == FILEMODE || mode == STRINGMODE);
	Lexmode = mode;
	if (Lexmode == STRINGMODE) Lp = pinfo->Pinstr;
}
#endif
/*===================================================
 * yylex -- High level lexer function (for debugging)
 *  lvalp:    [I/O] pointer to bison's data (primarily return value)
 *  yaccparm: [IN]  pointer to data passed by interp.c to bison's yyparse 
 *=================================================*/
int
yylex (YYSTYPE * lvalp, YYLTYPE *llocp, void * pactx)
{
	INT lex = lowyylex(pactx, lvalp);

#ifdef DEBUG
	if (isascii(lex))
		llwprintf("lex<%c> ", lex);
	else
		llwprintf("lex<%d> ", lex);
#endif

	return lex;
}
/*===========================
 * is_iden_char -- is still part of ongoing identifier ?
 *=========================*/
static BOOLEAN
is_iden_char (INT c, INT t)
{
	return t==LETTER || t==DIGIT || c=='_' || c=='.';
}
/*===========================
 * lowyylex -- Lexer function
 *=========================*/
static int
lowyylex (void *pactx, YYSTYPE * lvalp)
{
	INT c=0, t=0, retval, mul;
	extern INT Yival;
	extern FLOAT Yfval;
	static char tokbuf[200];	/* token buffer */
	STRING p = tokbuf;
	FILE * infp = get_infp(pactx);
	/* TODO: pass report codeset thru to here, & on to the
	string_node_in_internal_codeset calls below */

	while (TRUE) {
		while ((t = chartype(c = inchar(pactx))) == WHITE)
			;
		if (c != '/') break;
		if ((c = inchar(pactx)) != '*') {
			unreadchar(pactx, c);
			return '/';
		}
		/* inside a comment -- advance til end */
		while (TRUE) {
			while ((c = inchar(pactx)) != '*' && c != EOF)
				;
			if (c == EOF) return 0;
			while ((c = inchar(pactx)) == '*')
				;
			if (c == '/') break;
			if (c == EOF) return 0;
		}
	}
	if (t == LETTER) {
		p = tokbuf;
		while (is_iden_char(c, t)) {
			if (p-tokbuf < (int)sizeof(tokbuf) - 3) {
				*p++ = c;
			} else {
				/* token overlong -- ignore end of it */
				/* TODO: How can we force a parse error from here ? */
			}
			t = chartype(c = inchar(pactx));
		}
		*p = 0;
		unreadchar(pactx, c);
#ifdef DEBUG
		llwprintf("in lex.c -- IDEN is %s\n", tokbuf);
#endif
		if (reserved(tokbuf, &retval))  return retval;
		*lvalp = (PNODE) strsave(tokbuf);
		return IDEN;
	}
	if (t == '-' || t == DIGIT || t == '.') {
		BOOLEAN whole = FALSE;
		BOOLEAN frac = FALSE;
		FLOAT fdiv;
		mul = 1;
		if (t == '-') {
			t = chartype(c = inchar(pactx));
			if (t != '.' && t != DIGIT) {
				unreadchar(pactx, c);
				return '-';
			}
			mul = -1;
		}
		Yival = 0;
		while (t == DIGIT) {
			whole = TRUE;
			Yival = Yival*10 + c - '0';
			t = chartype(c = inchar(pactx));
		}
		if (t != '.') {
			unreadchar(pactx, c);
			Yival *= mul;
			*lvalp = NULL;
			return ICONS;
		}
		t = chartype(c = inchar(pactx));
		Yfval = 0.0;
		fdiv = 1.0;
		while (t == DIGIT) {
			frac = TRUE;
			Yfval = Yfval*10 + c - '0';
			fdiv *= 10.;
			t = chartype(c = inchar(pactx));
		}
		unreadchar(pactx, c);
		if (!whole && !frac) {
			unreadchar(pactx, c);
			if (mul == -1) {
				unreadchar(pactx, '.');
				return '-';
			} else
				return '.';
		}
		Yfval = mul*(Yival + Yfval/fdiv);
		*lvalp = NULL;
		return FCONS;
	}
	if (c == '"') {
		p = tokbuf;
		while (TRUE) {
			while ((c = inchar(pactx)) != EOF && c != '"' && c != '\\')
				*p++ = c;
			if (c == 0 || c == '"') {
				*p = 0;
				*lvalp = make_internal_string_node(pactx, tokbuf);
				return SCONS;
			}
			switch (c = inchar(pactx)) {
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
				*lvalp = make_internal_string_node(pactx, tokbuf);
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
static INT
inchar (void *pactx)
{
	INT c;
	FILE *infp = get_infp(pactx);
#ifdef SKIPCTRLZ
	do {
#endif
		if (Lexmode == FILEMODE)
			c = getc(infp);
		else
			c = (uchar)*Lp++;
#ifdef SKIPCTRLZ
	} while(c == 26);		/* skip CTRL-Z */
#endif
	if (c == '\n') {
		adj_lineno(pactx, +1);
	}
	return c;
}
/*================================================================
 * unreadchar -- Unread char from input file/string; track line number
 *==============================================================*/
static void
unreadchar (void *pactx, INT c)
{
	if (Lexmode == FILEMODE) {
		FILE *infp = get_infp(pactx);
		ungetc(c, infp);
	} else {
		Lp--;
	}
	if (c == '\n') {
		adj_lineno(pactx, -1);
	}
}
