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
 * builtin.c -- Many interpreter builtin functions
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 07 Sep 93
 *   3.0.0 - 07 May 94    3.0.2 - 03 Jan 95
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "cache.h"
#include "interp.h"
#include "indiseq.h"
#include "translat.h"

extern STRING llprograms;

/*==========================================
 * __getint -- Have user provide integer
 *   usage: getint(IDEN [,STRING]) --> VOID
 *   usage: getintmsg(IDEN, STRING) --> VOID
 *========================================*/
WORD __getint (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	INT val;
	STRING ttl = (STRING) "Enter integer for report program";
	*eflg = TRUE;
	if (!iistype(arg, IIDENT)) return NULL;
	*eflg = FALSE;
	if (inext(arg)) ttl = (STRING) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	val = ask_for_int(ttl);
	assign_iden(stab, iident(arg), (WORD) val);
	return NULL;
}
/*===========================================
 * __getstr -- Have user provide string
 *   usage: getstr(IDEN [,STRING]) --> VOID
 *   usage: getstrmsg(IDEN, STRING]) --> VOID
 *=========================================*/
WORD __getstr (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	STRING val;
	static STRING ttl = (STRING) "Enter string for report program";
	*eflg = TRUE;
	if (!iistype(arg, IIDENT)) return NULL;
	*eflg = FALSE;
	if (inext(arg)) ttl = (STRING) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	val = (STRING) ask_for_string(ttl, "enter string: ");
	assign_iden(stab, iident(arg), (WORD) val);
	return NULL;
}
/*===========================================
 * __getindi -- Have user identify person
 *   usage: getindi(IDEN [,STRING]) --> VOID
 *   usage: getindimsg(IDEN, STRING) --> VOID
 *=========================================*/
WORD __getindi (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	CACHEEL cel;
	STRING ttl = (STRING) "Identify person for report program:";
	STRING key;
	*eflg = TRUE;
	if (!iistype(arg, IIDENT)) return NULL;
	*eflg = FALSE;
	if (inext(arg)) ttl = (STRING) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	key = ask_for_indi_key(ttl, FALSE, TRUE);
	if (!key) return NULL;
	cel = key_to_indi_cacheel(key);
	assign_iden(stab, iident(arg), (WORD) cel);
	return NULL;
}
#if 0
/*===================================================
 * __getindidate -- Have user identify person
 *   usage: getindidate(IDEN, INT [,STRING]) --> VOID
 *=================================================*/
WORD __getindidate (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	INT year;
	CACHEEL cel;
	STRING ttl = "Identify person for report program:";
	*eflg = TRUE;
	if (!iistype(arg, IIDENT)) return NULL;
	*eflg = FALSE;
	year = (INT) evaluate(arg = inext(arg), stab, eflg);
	if (*eflg) return NULL;
	if (inext(arg)) ttl = (STRING) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	cel = key_to_indi_cacheel(ask_for_indi_key(ttl, FALSE, TRUE));
	assign_iden(stab, iident(arg), (WORD) cel);
	return NULL;
}
#endif
/*=========================================
 * __getfam -- Have user identify family
 *   usage: getfam(IDEN [,STRING]) --> VOID
 *=======================================*/
static char *chfamily = "Choose family by selecting spouse:";
WORD __getfam (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	CACHEEL cel = NULL;
	NODE fam;
	STRING ttl = (STRING) "Identify family for report program:";
	*eflg = TRUE;
	if (!iistype(arg, IIDENT)) return NULL;
	*eflg = FALSE;
	if (inext(arg)) ttl = (STRING) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	fam = ask_for_fam("Enter a spouse from family.",
	    "Enter a sibling from family.");
	if (fam) cel = fam_to_cacheel(fam);
	assign_iden(stab, iident(arg), (WORD) cel);
	return NULL;
}
/*=================================================
 * __getindiset -- Have user identify set of persons
 *   usage: getindiset(IDEN [,STRING]) --> VOID
 *===============================================*/
WORD __getindiset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	INDISEQ seq;
	STRING ttl = (STRING) "Identify person list for report program:";
	*eflg = TRUE;
	if (!iistype(arg, IIDENT)) return NULL;
	*eflg = FALSE;
	if (inext(arg)) ttl = (STRING) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	seq = (INDISEQ) ask_for_indi_list(ttl, TRUE);
	assign_iden(stab, iident(arg), (WORD) seq);
	return NULL;
}
/*==================================
 * __gettoday -- Create today's event
 *   usage: gettoday() --> EVENT
 *================================*/
WORD __gettoday (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE prnt = create_node(NULL, "EVEN", NULL, NULL);
	NODE chil = create_node(NULL, "DATE", get_date(), prnt);
	nchild(prnt) = chil;
	*eflg = FALSE;
	return (WORD) prnt;
}
/*======================================
 * __name -- Find person's name
 *   usage: name(INDI [,BOOL]) -> STRING
 *====================================*/
WORD __name (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	NODE indi = eval_indi(arg, stab, eflg, NULL);
	BOOLEAN caps = TRUE;
	TRANTABLE ttr = tran_tables[MINRP];
	if (*eflg || !indi) return NULL;
	if (inext(arg))
		caps = (BOOLEAN) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	indi = find_tag(nchild(indi), "NAME");
	return (WORD) manip_name(nval(indi), ttr, caps, TRUE, 68);
}
/*===================================================
 * __fullname -- Process person's name
 *   usage: fullname(INDI, BOOL, BOOL, INT) -> STRING
 *=================================================*/
WORD __fullname (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	NODE name, indi = eval_indi(arg, stab, eflg, NULL);
	BOOLEAN caps, reg;
	INT len;
	TRANTABLE ttr = tran_tables[MINRP];
	if (*eflg || !indi) return NULL;
	*eflg = FALSE;
	caps = (BOOLEAN) evaluate(arg = inext(arg), stab, eflg);
	if (*eflg) return NULL;
	reg = (BOOLEAN) evaluate(arg = inext(arg), stab, eflg);
	if (*eflg) return NULL;
	len = (INT) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	if (!(name = NAME(indi)) || !nval(name)) {
		*eflg = TRUE;
		return NULL;
	}
	return (WORD) manip_name(nval(name), ttr, caps, reg, len);
}
/*==================================
 * __surname -- Find person's surname
 *   usage: surname(INDI) -> STRING
 *================================*/
WORD __surname (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE this = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !this) return NULL;
	if (!(this = NAME(this)) || !nval(this)) return "ERROR";
	return (WORD) getsurname(nval(this));
}
/*========================================
 * __soundex -- SOUNDEX function on persons
 *   usage: soundex(INDI) -> STRING
 *======================================*/
WORD __soundex (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE this = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !this) return NULL;
	if (!(this = NAME(this)) || !nval(this)) return "ERROR";
	return (WORD) soundex(getsurname(nval(this)));
}
/*===========================================
 * __strsoundex -- SOUNDEX function on strings
 *   usage: strsoundex(STRING) -> STRING
 *=========================================*/
WORD __strsoundex (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	STRING str = (STRING) evaluate(ielist(node), stab, eflg);
	if (*eflg || !str) return NULL;
	return (WORD) soundex(str);
}
/*================================
 * __givens -- Find given names
 *   usage: givens(INDI) -> STRING
 *==============================*/
WORD __givens (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE this = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !this) return NULL;
	if (!(this = NAME(this)) || !nval(this)) return "ERROR";
	return (WORD) givens(nval(this));
}
/*================================
 * __set -- Assignment operation
 *   usage: set(IDEN, ANY) -> VOID
 *==============================*/
WORD __set (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP var = (INTERP) ielist(node);
	INTERP expr = inext(var);
	WORD value;
	*eflg = TRUE;
	if (!iistype(var, IIDENT)) return NULL;
	value = evaluate(expr, stab, eflg);
	if (*eflg) return NULL;
	assign_iden(stab, iident(var), value);
	return NULL;
}
/*===================================
 * __husband -- Find husband of family
 *   usage: husband(FAM) -> INDI
 *=================================*/
WORD __husband (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE fam = eval_fam(ielist(node), stab, eflg, NULL);
	if (*eflg || !fam) return NULL;
	return (WORD) indi_to_cacheel(fam_to_husb(fam));
}
/*=============================
 * __wife -- Find wife of family
 *   usage: wife(FAM) -> INDI
 *===========================*/
WORD __wife (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE fam = eval_fam(ielist(node), stab, eflg, NULL);
	if (*eflg || !fam) return NULL;
	return (WORD) indi_to_cacheel(fam_to_wife(fam));
}
/*==========================================
 * __firstchild -- Find first child of family
 *   usage: firstchild(FAM) -> INDI
 *========================================*/
WORD __firstchild (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE fam = eval_fam(ielist(node), stab, eflg, NULL);
	if (*eflg || !fam) return NULL;
	return (WORD) indi_to_cacheel(fam_to_first_chil(fam));
}
/*========================================
 * __lastchild -- Find last child of family
 *   usage: lastchild(FAM) -> INDI
 *======================================*/
WORD __lastchild (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE fam = eval_fam(ielist(node), stab, eflg, NULL);
	if (*eflg || !fam) return NULL;
	return (WORD) indi_to_cacheel(fam_to_last_chil(fam));
}
/*=================================
 * __marr -- Find marriage of family
 *   usage: marriage(FAM) -> EVENT
 *===============================*/
WORD __marr (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE fam = eval_fam(ielist(node), stab, eflg, NULL);
	if (*eflg || !fam) return NULL;
	return (WORD) MARR(fam);
}
/*==============================
 * __birt -- Find birth of person
 *   usage: birth(INDI) -> EVENT
 *============================*/
WORD __birt (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	return (WORD) BIRT(indi);
}
/*==============================
 * __deat -- Find death of person
 *   usage: death(INDI) -> EVENT
 *============================*/
WORD __deat (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	return (WORD) DEAT(indi);
}
/*================================
 * __bapt -- Find baptism of person
 *   usage: baptism(INDI) -> EVENT
 *==============================*/
WORD __bapt (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	return (WORD) BAPT(indi);
}
/*===============================
 * __buri -- Find burial of person
 *   usage: burial(INDI) -> EVENT
 *=============================*/
WORD __buri (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	return (WORD) BURI(indi);
}
/*===============================
 * __titl -- Find title of person
 *   usage: title(INDI) -> STRING
 *=============================*/
WORD __titl (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	indi = find_tag(nchild(indi), "TITL");
	return (WORD) (indi ? nval(indi) : NULL);
}
/*===================================
 * __long -- Return long form of event
 *   usage: long(EVENT) -> STRING
 *=================================*/
WORD __long (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE event = (NODE) evaluate(ielist(node), stab, eflg);
	TRANTABLE ttr = tran_tables[MINRP];
	if (*eflg || !event) return NULL;
	return (WORD) event_to_string(event, ttr, FALSE);
}
/*=====================================
 * __short -- Return short form of event
 *   usage: short(EVENT) -> STRING
 *===================================*/
WORD __short (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE event = (NODE) evaluate(ielist(node), stab, eflg);
	TRANTABLE ttr = tran_tables[MINRP];
	if (*eflg || !event) return NULL;
	return (WORD) event_to_string(event, ttr, TRUE);
}
/*================================
 * __fath -- Find father of person
 *   usage: father(INDI) -> INDI
 *==============================*/
WORD __fath (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	return (WORD) indi_to_cacheel(indi_to_fath(indi));
}
/*===============================
 * __moth -- Find mother of person
 *   usage: mother(INDI) -> INDI
 *=============================*/
WORD __moth (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	return (WORD) indi_to_cacheel(indi_to_moth(indi));
}
/*===========================================
 * __parents -- Find parents' family of person
 *   usage: parents(INDI) -> FAM
 *=========================================*/
WORD __parents (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	return (WORD) fam_to_cacheel(indi_to_famc(indi));
}
/*==========================================
 * __nextsib -- Find person's younger sibling
 *   usage: nextsib(INDI) -> INDI
 *========================================*/
WORD __nextsib (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	return (WORD) indi_to_cacheel(indi_to_next_sib(indi));
}
/*========================================
 * __prevsib -- Find person's older sibling
 *   usage: prevsib(INDI) -> INDI
 *======================================*/
WORD __prevsib (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	return (WORD) indi_to_cacheel(indi_to_prev_sib(indi));
}
/*========================================
 * __d -- Return cardinal integer as string
 *   usage: d(INT) -> STRING
 *======================================*/
WORD __d (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	static char scratch[20];
	WORD value = evaluate(ielist(node), stab, eflg);
	if (*eflg) return "";
	sprintf(scratch, "%d", value);
	return scratch;
}
/*==========================================
 * __alpha -- Convert small integer to letter
 *   usage: alpha(INT) -> STRING
 *========================================*/
WORD __alpha (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	static char scratch[2];
	INT value = (INT) evaluate(ielist(node), stab, eflg);
	if (*eflg) return NULL;
	if (value < 1 || value > 26) return "XX";
	sprintf(scratch, "%c", 'a' + value - 1);
	return scratch;
}
/*================================================
 * __ord -- Convert small integer to ordinal string
 *   usage: ord(INT) -> STRING
 *==============================================*/
static char *ordinals[] = {
	"first", "second", "third", "fourth", "fifth",
	"sixth", "seventh", "eighth", "ninth", "tenth",
	"eleventh", "twelfth"
};
WORD __ord (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	static char scratch[12];
	INT value = (INT) evaluate(ielist(node), stab, eflg);
	if (*eflg || value < 1) return NULL;
	if (value > 12) {
		sprintf(scratch, "%dth", value);
		return (WORD) scratch;
	}
	return (WORD) ordinals[value - 1];
}
/*==================================================
 * __card -- Convert small integer to cardinal string
 *   usage: card(INT) -> STRING
 *================================================*/
static char *cardinals[] = {
	"zero", "one", "two", "three", "four", "five",
	"six", "seven", "eight", "nine", "ten",
	"eleven", "twelve"
};
WORD __card (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	static char scratch[8];
	INT value = (INT) evaluate(ielist(node), stab, eflg);
	if (*eflg) return NULL;
	if (value < 0 || value > 12) {
		sprintf(scratch, "%d", value);
		return (WORD) scratch;
	}
	return (WORD) cardinals[value];
}
/*===========================================
 * __roman -- Convert integer to Roman numeral
 *   usage: roman(INT) -> STRING
 *=========================================*/
static char *rodigits[] = {
	"", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix"
};
static char *rotens[] = {
	"", "x", "xx", "xxx", "xl", "l", "lx", "lxx", "lxxx", "xc"
};
WORD __roman (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	static char scratch[10];
	INT value = (INT) evaluate(ielist(node), stab, eflg);
	if (*eflg) return NULL;
	if (value < 1 || value >= 99) return __d(node, stab, eflg);
	sprintf(scratch, "%s%s", rotens[value/10], rodigits[value%10]);
	return scratch;
}
/*================================================
 * __nchildren -- Find number of children in family
 *   usage: nchildren(FAM) -> INT
 *==============================================*/
WORD __nchildren (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE fam = eval_fam(ielist(node), stab, eflg, NULL);
	if (*eflg || !fam) return NULL;
	return (WORD) node_list_length(CHIL(fam));
}
/*===================================================
 * __nfamilies -- Find number of families person is in
 *   usage: nfamilies(INDI) -> INT
 *=================================================*/
WORD __nfamilies (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	return (WORD) node_list_length(FAMS(indi));
}
/*===============================================
 * __nspouses -- Find number of spouses person has
 *   usage: nspouses(INDI) -> INT
 *=============================================*/
WORD __nspouses (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	INT nspouses;
	if (*eflg || !indi) return NULL;
	FORSPOUSES(indi,spouse,fam,nspouses) ENDSPOUSES
	return (WORD) nspouses;
}
/*==============================
 * __eq -- Equal operation
 *   usage: eq(ANY, ANY) -> BOOL
 *============================*/
WORD __eq (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INT val1, val2;
	eval_binary(node, &val1, &val2, stab, eflg);
	return (WORD) (val1 == val2);
}
/*==============================
 * __ne -- Not equal operation
 *   usage: ne(ANY, ANY) -> BOOL
 *============================*/
WORD __ne (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INT val1, val2;
	eval_binary(node, &val1, &val2, stab, eflg);
	return (WORD) (val1 != val2);
}
/*===============================
 * __le -- Less or equal operation
 *   usage: le(ANY, ANY) -> BOOL
 *=============================*/
WORD __le (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INT val1, val2;
	eval_binary(node, &val1, &val2, stab, eflg);
	return (WORD) (val1 <= val2);
}
/*==================================
 * __ge -- Greater or equal operation
 *   usage: ge(ANY, ANY) -> BOOL
 *================================*/
WORD __ge (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INT val1, val2;
	eval_binary(node, &val1, &val2, stab, eflg);
	return (WORD) (val1 >= val2);
}
/*=============================
 * __lt -- Less than operation
 *   usage: lt(ANY,ANY) -> BOOL
 *===========================*/
WORD __lt (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INT val1, val2;
	eval_binary(node, &val1, &val2, stab, eflg);
	return (WORD) (val1 < val2);
}
/*==============================
 * __gt -- Greater than operation
 *   usage: gt(ANY, ANY) -> BOOL
 *============================*/
WORD __gt (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INT val1, val2;
	eval_binary(node, &val1, &val2, stab, eflg);
	return (WORD) (val1 > val2);
}
/*==================================
 * __and -- And operation
 *   usage: and(ANY [,ANY]+) -> INT
 *================================*/
WORD __and (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	INT val = (INT) evaluate(arg, stab, eflg);
	if (*eflg) return NULL;
	if (!val) return (WORD) FALSE;
	while (arg = inext(arg)) {
		val = val && (INT) evaluate(arg, stab, eflg);
		if (*eflg) return NULL;
		if (!val) return (WORD) FALSE;
	}
	return (WORD) val;
}
/*================================
 * __or -- Or operation
 *   usage: or(ANY [,ANY]+) -> INT
 *==============================*/
WORD __or (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	INT val = (INT) evaluate(arg, stab, eflg);
	if (*eflg) return NULL;
	if (val) return (WORD) TRUE;
	while (arg = inext(arg)) {
		val = val || (INT) evaluate(arg, stab, eflg);
		if (*eflg) return NULL;
		if (val) return (WORD) TRUE;
	}
	return (WORD) val;
}
/*==================================
 * __add -- Add operation
 *   usage: add(INT [,INT]+) -> INT
 *================================*/
WORD __add (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	INT val = (INT) evaluate(arg, stab, eflg);
	if (*eflg) return NULL;
	while (arg = inext(arg)) {
		val += (INT) evaluate(arg, stab, eflg);
		if (*eflg) return NULL;
	}
	return (WORD) val;
}
/*==============================
 * __sub -- Subtract operation
 *   usage: sub(INT, INT) -> INT
 *============================*/
WORD __sub (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INT val1, val2;
	eval_binary(node, &val1, &val2, stab, eflg);
	return (WORD) (val1 - val2);
}
/*=================================
 * __mul -- Multiply operation
 *   usage: mul(INT [,INT]+) -> INT
 *===============================*/
WORD __mul (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	INT val = (INT) evaluate(arg, stab, eflg);
	if (*eflg) return NULL;
	while (arg = inext(arg)) {
		val *= (INT) evaluate(arg, stab, eflg);
		if (*eflg) return NULL;
	}
	return (WORD) val;
}
/*==============================
 * __div -- Divide operation
 *   usage: div(INT, INT) -> INT
 *============================*/
WORD __div (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INT val1, val2;
	eval_binary(node, &val1, &val2, stab, eflg);
	if (*eflg || val2 == 0) {
		*eflg = TRUE;
		return NULL;
	}
	return (WORD) (val1/val2);
}
/*==============================
 * __mod -- Modulus operation
 *   usage: mod(INT, INT) -> INT
 *============================*/
WORD __mod (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INT val1, val2;
	eval_binary(node, &val1, &val2, stab, eflg);
	if (*eflg || val2 == 0) {
		*eflg = TRUE;
		return NULL;
	}
	return (WORD) (val1%val2);
}
/*=================================
 * __exp -- Exponentiation operation
 *   usage: exp(INT, INT) -> INT
 *===============================*/
WORD __exp (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INT i, val1, val2, val3;
	eval_binary(node, &val1, &val2, stab, eflg);
	if (*eflg) return NULL;
	if (val2 < 0) {
		*eflg = TRUE;
		return NULL;
	}
	if (*eflg || val2 == 0) {
		*eflg = TRUE;
		return NULL;
	}
	val3 = 1;
	for (i = 0; i < val2; i++)
		val3 *= val1;
	return (WORD) val3;
}
/*===========================
 * __neg -- Negation operation
 *   usage: neg(INT) -> INT
 *=========================*/
WORD __neg (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	WORD value = evaluate(ielist(node), stab, eflg);
	if (*eflg) return NULL;
	return (WORD) - (INT) value;
}
/*============================
 * __incr -- Increment variable
 *   usage: incr(VARB) -> VOID
 *==========================*/
WORD __incr (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP var = (INTERP) ielist(node);
	INT val;
	*eflg = TRUE;
	if (!iistype(var, IIDENT)) return NULL;
	val = (INT) evaluate(var, stab, eflg);
	if (*eflg) return NULL;
	assign_iden(stab, iident(var), val + 1);
	return NULL;
}
/*============================
 * __decr -- Decrement variable
 *   usage: decr(VARB) -> VOID
 *==========================*/
WORD __decr (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP var = (INTERP) ielist(node);
	INT val;
	*eflg = TRUE;
	if (!iistype(var, IIDENT)) return NULL;
	val = (INT) evaluate(var, stab, eflg);
	if (*eflg) return NULL;
	assign_iden(stab, iident(var), val - 1);
	return NULL;
}
/*==========================================
 * __strcmp -- Compare two strings
 *   usage: strcmp(STRING, STRING) -> INT
 *   usage: nestr(STRING, STRING) -> BOOLEAN
 *========================================*/
WORD __strcmp (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	STRING str2, str1 = (STRING) evaluate(arg, stab, eflg);
	STRING emp = (STRING) "";
	if (*eflg) return NULL;
	str2 = (STRING) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	if (!str1) str1 = emp;
	if (!str2) str2 = emp;
	return (WORD) strcmp(str1, str2);
}
/*==========================================
 * __eqstr -- Compare two strings
 *   usage: eqstr(STRING, STRING) -> BOOLEAN
 *========================================*/
WORD __eqstr (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	STRING str2, str1 = (STRING) evaluate(arg, stab, eflg);
	STRING emp = (STRING) "";
	if (*eflg) return NULL;
	str2 = (STRING) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	if (!str1) str1 = emp;
	if (!str2) str2 = emp;
	return (WORD) eqstr(str1, str2);
}
/*=======================================
 * __strtoint -- Convert string to integer
 *  usage: strtoint(STRING) -> INT
 *=====================================*/
WORD __strtoint (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	STRING str = (STRING) evaluate(ielist(node), stab, eflg);
	if (*eflg || !str || *str == 0) return NULL;
	return (WORD) atoi(str);
}
/*=============================
 * __list -- Create list
 *   usage: list(IDENT) -> VOID
 *===========================*/
WORD __list (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	LIST list;
	INTERP var = (INTERP) ielist(node);
	*eflg = TRUE;
	if (!iistype(var, IIDENT)) return NULL;
	*eflg = FALSE;
	list = create_list();
	assign_iden(stab, iident(var), (WORD) list);
	return NULL;
}
/*=======================================
 * __push -- Push element on front of list
 *   usage: push(LIST, ANY) -> VOID
 *   usage: enqueue(LIST, ANY) -> VOID
 *=====================================*/
WORD __push (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	LIST list = (LIST) evaluate(arg, stab, eflg);
	WORD el;
	if (*eflg || !list) return NULL;
	el = evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	push_list(list, el);
	return NULL;
}
/*========================================
 * __requeue -- Add element to back of list
 *   usage: requeue(LIST, ANY) -> VOID
 *======================================*/
WORD __requeue (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	LIST list = (LIST) evaluate(arg, stab, eflg);
	WORD el;
	if (*eflg || !list) return NULL;
	el = evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	back_list(list, el);
	return NULL;
}
/*=======================================
 * __pop -- Pop element from front of list
 *   usage: pop(LIST) -> ANY
 *=====================================*/
WORD __pop (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	LIST list = (LIST) evaluate(ielist(node), stab, eflg);
	if (*eflg || !list) return NULL;
	return pop_list(list);
}
/*=============================================
 * __dequeue -- Remove element from back of list
 *   usage dequeue(LIST) -> ANY
 *===========================================*/
WORD __dequeue (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	LIST list = (LIST) evaluate(ielist(node), stab, eflg);
	if (*eflg || !list) return NULL;
	return dequeue_list(list);
}
/*=================================
 * __empty -- Check if list is empty
 *   usage: empty(LIST) -> BOOL
 *===============================*/
WORD __empty (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	LIST list = (LIST) evaluate(ielist(node), stab, eflg);
	if (*eflg || !list) return NULL;
	return (WORD) empty_list(list);
}
/*==================================
 * __getel -- Get nth value from list
 *   usage: getel(LIST, INT) -> ANY
 *================================*/
WORD __getel (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	LIST list = (LIST) evaluate(arg, stab, eflg);
	INT ind;
	if (*eflg || !list) return NULL;
	ind = (INT) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	return get_list_element(list, ind);
}
/*========================================
 * __setel -- Set nth value in list
 *   usage: setel(LIST, INT, ANY) -> VOID
 *======================================*/
WORD __setel (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	LIST list = (LIST) evaluate(arg, stab, eflg);
	INT ind;
	WORD val;
	if (*eflg || !list) return NULL;
	arg = inext(arg);
	ind = (INT) evaluate(arg, stab, eflg);
	if (*eflg || ind < 1) return NULL;
	val = evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	set_list_element(list, ind, val);
	return NULL;
}
/*===============================
 * __length -- Find length of list
 *   usage: length(LIST) -> INT
 *=============================*/
WORD __length (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	LIST list = (LIST) evaluate(ielist(node), stab, eflg);
	if (*eflg || !list) return NULL;
	return (WORD) length_list(list);
}
/*=========================
 * __not -- Not operation
 *   usage: not(INT) -> INT
 *=======================*/
WORD __not (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	WORD value = evaluate(ielist(node), stab, eflg);
	if (*eflg) return NULL;
	return (WORD) !value;
}
/*================================
 * __save -- Copy string
 *   usage: save(STRING) -> STRING
 *==============================*/
WORD __save (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	WORD value = evaluate(ielist(node), stab, eflg);
	if (*eflg) return NULL;
	return value? (WORD) strsave(value) : NULL;
}
/*=================================
 * __strlen -- Find length of string
 *   usage: strlen(STRING) -> INT
 *===============================*/
WORD __strlen (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	STRING str = (STRING) evaluate(ielist(node), stab, eflg);
	if (*eflg || !str) return NULL;
	return (WORD) strlen(str);
}
/*==============================================
 * __concat -- Catenate strings
 *   usage: concat(STRING [, STRING]+) -> STRING
 *============================================*/
WORD __concat (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
        INTERP arg = (INTERP) ielist(node);
        INT len = 0, i, nstrs = 0;
        STRING hold[32];
        STRING p, new, str;

        while (arg) {
                str = (STRING) evaluate(arg, stab, eflg);
                if (*eflg) return NULL;
                if (str) {
                        len += strlen(str);
                        hold[nstrs++] = strsave(str);
                } else
                        hold[nstrs++] = NULL;
                arg = inext(arg);
        }
        p = new = (STRING) stdalloc(len + 1);
        for (i = 0; i < nstrs; i++) {
                str = hold[i];
                if (str) {
                        strcpy(p, str);
                        p += strlen(p);
                        stdfree(str);
                }
        }
        return (WORD) new;
}
/*=======================================
 * __lower -- Convert string to lower case
 *   usage: lower(STRING) -> STRING
 *=====================================*/
WORD __lower (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	WORD val = evaluate(ielist(node), stab, eflg);
	if (*eflg || !val) return NULL;
        return (WORD) lower(val);
}
/*=======================================
 * __upper -- Convert string to upper case
 *   usage: upper(STRING) -> STRING
 *=====================================*/
WORD __upper (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	WORD val = evaluate(ielist(node), stab, eflg);
	if (*eflg || !val) return NULL;
	return (WORD) upper(val);
}
/*======================================
 * __capitalize -- Capitalize string
 *   usage: capitalize(STRING) -> STRING
 *====================================*/
WORD __capitalize (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	WORD val = evaluate(ielist(node), stab, eflg);
	if (*eflg || !val) return NULL;
	return (WORD) capitalize(val);
}
/*=================================
 * __pn -- Generate pronoun
 *   usage: pn(INDI, INT) -> STRING
 *===============================*/
static char *mpns[] = {  "He",  "he", "His", "his", "him" };
static char *fpns[] = { "She", "she", "Her", "her", "her" };
WORD __pn (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	NODE indi;
	INT typ;
	indi = eval_indi(arg, stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	typ = (INT) evaluate(inext(arg), stab, eflg);
	if (*eflg || typ < 0 || typ > 4) return NULL;
	if (SEX(indi) == SEX_FEMALE)
		return (WORD) fpns[typ];
	else
		return (WORD) mpns[typ];
}
/*===================================
 * __print -- Print to stdout window
 *   usage: print([STRING]+,) -> VOID
 *=================================*/
WORD __print (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	WORD val;
	while (arg) {
		val = evaluate(arg, stab, eflg);
		if (*eflg) return NULL;
		if (val) wprintf("%s", (STRING) val);
		arg = inext(arg);
	}
	return NULL;
}
/*=================================================
 * __sex -- Find sex, as string M, F or U, of person
 *   usage: sex(INDI) -> STRING
 *===============================================*/
WORD __sex (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	INT sex;
	if (*eflg || !indi) return NULL;
	if ((sex = SEX(indi)) == SEX_MALE) return "M";
	if (sex == SEX_FEMALE) return "F";
	return "U";
}
/*=================================
 * __male -- Check if person is male
 *   usage: male(INDI) -> BOOL
 *===============================*/
WORD __male (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	return (WORD) (SEX(indi) == SEX_MALE);
}
/*=====================================
 * __female -- Check if person is female
 *   usage: female(INDI) -> BOOL
 *===================================*/
WORD __female (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	if (*eflg || !indi) return NULL;
	return (WORD) (SEX(indi) == SEX_FEMALE);
}
/*=========================================
 * __key -- Return person or family key
 *   usage: key(INDI|FAM [,BOOL]) -> STRING
 *=======================================*/
WORD __key (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	CACHEEL cel = (CACHEEL) evaluate(arg, stab, eflg);
	BOOLEAN strip = FALSE;
	STRING key;
	if (*eflg || !cel) return NULL;
	if (inext(arg)) strip = (BOOLEAN) evaluate(inext(arg), stab, eflg);
	key = (STRING) ckey(cel);
	return (WORD) (strip ? key + 1 : key);
}
/*===============================================
 * __root -- Return root of cached record
 *   usage: root(INDI|FAM|EVEN|SOUR|OTHR) -> NODE
 *=============================================*/
WORD __rot (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	STRING key;
 	CACHEEL cel = (CACHEEL) evaluate(ielist(node), stab, eflg);
        if (*eflg || !cel) return NULL;
        if (cnode(cel))
		return (WORD) cnode(cel);
	key = ckey(cel);
	switch (*key) {
	case 'I': return (WORD) key_to_indi(key); break;
	case 'F': return (WORD) key_to_fam(key);  break;
	case 'E': return (WORD) key_to_even(key); break;
	case 'S': return (WORD) key_to_sour(key); break;
	case 'X': return (WORD) key_to_othr(key); break;
	default:  FATAL();
	}
}
/*================================
 * __inode -- Return root of person
 *   usage: inode(INDI) -> NODE
 *==============================*/
WORD __inode (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	return (WORD) eval_indi(ielist(node), stab, eflg, NULL);
}
/*================================
 * __fnode -- Return root of family
 *   usage: fnode(FAM) -> NODE
 *==============================*/
WORD __fnode (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	return (WORD) eval_fam(ielist(node), stab, eflg, NULL);
}
/*==============================
 * __table -- Create table
 *   usage: table(IDENT) -> VOID
 *============================*/
WORD __table (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	TABLE tab;
	INTERP var = (INTERP) ielist(node);
	*eflg = TRUE;
	if (!iistype(var, IIDENT)) return NULL;
	*eflg = FALSE;
	tab = create_table();
	assign_iden(stab, iident(var), (WORD) tab);
	return NULL;
}
/*==========================================
 * __insert -- Add element to table
 *   usage: insert(TAB, STRING, ANY) -> VOID
 *========================================*/
WORD __insert (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	TABLE tab = (TABLE) evaluate(arg, stab, eflg);
	STRING str;
	WORD val;
	if (*eflg || !tab) return NULL;
	arg = inext(arg);
	str = (STRING) evaluate(arg, stab, eflg);
	if (*eflg) return NULL;
	val = (WORD) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	insert_table(tab, str, val);
	return NULL;
}
/*====================================
 * __lookup -- Look up element in table
 *   usage: lookup(TAB, STRING) -> ANY
 *==================================*/
WORD __lookup (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	TABLE tab = (TABLE) evaluate(arg, stab, eflg);
	STRING str;
	if (*eflg || !tab) return NULL;
	str = (STRING) evaluate(inext(arg), stab, eflg);
	if (*eflg || !str) return NULL;
	return (WORD) valueof(tab, str);
}
/*=====================================
 * __trim -- Trim string if too long
 *   usage: trim(STRING, INT) -> STRING
 *===================================*/
WORD __trim (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	STRING str = (STRING) evaluate(arg, stab, eflg);
	INT len;
	if (*eflg) return NULL;
	len = (INT) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	return (WORD) trim(str, len);
}
/*=======================================
 * __trimname -- Trim name if too long
 *   usage: trimname(INDI, INT) -> STRING
 *=====================================*/
WORD __trimname (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	INT len;
	NODE indi = eval_indi(arg, stab, eflg, (CACHEEL *) NULL);
	if (*eflg || !indi) return NULL;
	*eflg = TRUE;
	if (!(indi = NAME(indi)) || !nval(indi)) return NULL;
	*eflg = FALSE;
	len = (INT) evaluate(inext(arg), stab, eflg);
	if (*eflg) return NULL;
	return (WORD) name_string(trim_name(nval(indi), len));
}
/*===============================
 * __date -- Return date of event
 *   usage: date(EVENT) -> STRING
 *=============================*/
WORD __date (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE evnt = (NODE) evaluate(ielist(node), stab, eflg);
	TRANTABLE ttr = tran_tables[MINRP];
	if (*eflg || !evnt) return NULL;
	return (WORD) event_to_date(evnt, ttr, FALSE);
}
/*=====================================================
 * __extractdate -- Extract date from EVENT or DATE NODE
 *   usage: extractdate(NODE, VARB, VARB, VARB) -> VOID
 *===================================================*/
WORD __extractdate (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INT da = 0, mo = 0, yr = 0;
	STRING str;
	INTERP arg = (INTERP) ielist(node);
	INTERP dvar = inext(arg);
	INTERP mvar = inext(dvar);
	INTERP yvar = inext(mvar);
	NODE lin = (NODE) evaluate(arg, stab, eflg);
	if (*eflg) return NULL;
	*eflg = TRUE;
	if (!lin) return NULL;
	if (!iistype(dvar, IIDENT)) return NULL;
	if (!iistype(mvar, IIDENT)) return NULL;
	if (!iistype(yvar, IIDENT)) return NULL;
	if (nestr("DATE", ntag(lin)))
		str = event_to_date(lin, NULL, FALSE);
	else
		str = nval(lin);
	extract_date(str, &da, &mo, &yr);
	assign_iden(stab, iident(dvar), (WORD) da);
	assign_iden(stab, iident(mvar), (WORD) mo);
	assign_iden(stab, iident(yvar), (WORD) yr);
	*eflg = FALSE;
	return NULL;
}
/*=================================================
 * __stddate -- Return standard date format of event
 *   usage: stddate(EVENT) -> STRING
 *===============================================*/
static INT daycode = 0;
static INT monthcode = 3;
static INT datecode = 0;
WORD __stddate (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	extern STRING format_date();
	NODE evnt = (NODE) evaluate(ielist(node), stab, eflg);
	if (*eflg || !evnt) return NULL;
	return (WORD) format_date(event_to_date(evnt, NULL, FALSE),
	    daycode, monthcode, 1, datecode);
}
/*===============================================
 * __dayformat -- Set day format for standard date
 *   usage: dayformat(INT) -> NULL
 *=============================================*/
WORD __dayformat (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	INT value = (INT) evaluate(arg, stab, eflg);
	if (*eflg || value < 0) value = 0;
	if (value > 2) value = 2;
	daycode = value;
	return NULL;
}
/*===============================================
 * __monthformat -- Set month format standard date
 *   usage: dayformat(INT) -> NULL
 *=============================================*/
WORD __monthformat (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	INT value = (INT) evaluate(arg, stab, eflg);
	if (*eflg || value < 0) value = 0;
	if (value > 6) value = 6;
	monthcode = value;
	return NULL;
}
/*=================================================
 * __dateformat -- Set date format for standard date
 *   usage: dateformat(INT) -> NULL
 *===============================================*/
WORD __dateformat (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
        INTERP arg = (INTERP) ielist(node);
        INT value = (INT) evaluate(arg, stab, eflg);
        if (*eflg || value < 0) value = 0;
        if (value > 11) value = 11;
        datecode = value;
        return NULL;
}
/*===============================
 * __year -- Return year of event
 *   usage: year(EVENT) -> STRING
 *=============================*/
WORD __year (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE evnt = (NODE) evaluate(ielist(node), stab, eflg);
	if (*eflg || !evnt) return NULL;
	return (WORD) event_to_date(evnt, NULL, TRUE);
}
/*=================================
 * __place -- Return place of event
 *   usage: place(EVENT) -> STRING
 *==============================*/
WORD __place (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE evnt = (NODE) evaluate(ielist(node), stab, eflg);
	if (*eflg || !evnt) return NULL;
	return (WORD) event_to_plac(evnt, FALSE);
}
/*=============================
 * __tag -- Return tag of node
 *   usage: tag(NODE) -> STRING
 *===========================*/
WORD __tag (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE ged = (NODE) evaluate(ielist(node), stab, eflg);
	if (*eflg || !ged) return NULL;
	return (WORD) ntag(ged);
}
/*===============================
 * __value -- Return value of node
 *   usage: value(NODE) -> STRING
 *=============================*/
WORD __value (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE ged = (NODE) evaluate(ielist(node), stab, eflg);
	if (*eflg || !ged) return NULL;
	return (WORD) nval(ged);
}
/*==============================
 * __xref -- Return xref of node
 *   usage: xref(NODE) -> STRING
 *============================*/
WORD __xref (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE ged = (NODE) evaluate(ielist(node), stab, eflg);
	if (*eflg || !ged) return NULL;
	return (WORD) nxref(ged);
}
/*===============================
 * __child -- Return child of node
 *   usage: child(NODE) -> NODE
 *=============================*/
WORD __child (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE ged = (NODE) evaluate(ielist(node), stab, eflg);
	if (*eflg || !ged) return NULL;
	return (WORD) nchild(ged);
}
/*=================================
 * __parent -- Return parent of node
 *   usage: parent(NODE) -> NODE
 *===============================*/
WORD __parent (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE ged = (NODE) evaluate(ielist(node), stab, eflg);
	if (*eflg || !ged) return NULL;
	return (WORD) nparent(ged);
}
/*========================================
 * __sibling -- Return next sibling of node
 *   usage: sibling(NODE) -> NODE
 *======================================*/
WORD __sibling (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE ged = (NODE) evaluate(ielist(node), stab, eflg);
	if (*eflg || !ged) return NULL;
	return (WORD) nsibling(ged);
}
/*==================================
 * __copyfile -- Copy file to output
 *   usage: copyfile(STRING) -> VOID
 *================================*/
WORD __copyfile (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	FILE *cfp, *fopenpath();
	int c;
	STRING fname = (STRING) evaluate(ielist(node), stab, eflg);
	char buffer[1024];
	if (*eflg || !fname) return NULL;
	if (!(cfp = fopenpath(fname, "r", llprograms))) {
		*eflg = TRUE;
		return NULL;
	}
	while (fgets(buffer, 1024, cfp))
		poutput(buffer);
	fclose(cfp);
	return NULL;
}
/*========================
 * __nl -- Newline function
 *   usage: nl() -> STRING
 *======================*/
WORD __nl (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	*eflg = FALSE;
	return "\n";
}
/*=========================
 * __space -- Space function
 *   usage: sp() -> STRING
 *=======================*/
WORD __space (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	*eflg = FALSE;
	return " ";
}
/*=============================
 * __qt -- Double quote function
 *   usage: qt() -> STRING
 *===========================*/
WORD __qt (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	*eflg = FALSE;
	return "\"";
}
/*===============================
 * __indi -- Convert key to INDI
 *   usage: indi(STRING) -> INDI
 *============================*/
WORD __indi (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	STRING str = (STRING) evaluate(ielist(node), stab, eflg);
	STRING rec;
	unsigned char scratch[200], *p = str, *q = scratch;
	INT c, len;
	if (*eflg || !str) return NULL;
	while ((c = *p++) && chartype(c) != DIGIT)
		;
	if (c == 0) return NULL;
	*q++ = 'I';
	*q++ = c;
	while (chartype(c = *p++) == DIGIT)
		*q++ = c;
	*q = 0;
	if (strlen(scratch) == 1) return NULL;
	if (rec = (STRING) retrieve_record(scratch, &len)) {
		stdfree(rec);
		return (WORD) key_to_indi_cacheel(scratch);
	}
	return NULL;
}
/*=============================
 * __fam -- Convert key to FAM
 *   usage: fam(STRING) -> FAM
 *==========================*/
WORD __fam (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	STRING str = (STRING) evaluate(ielist(node), stab, eflg);
	STRING rec;
	char unsigned scratch[200], *p = str, *q = scratch;
	INT c, len;
	if (*eflg || !str) return NULL;
	while ((c = *p++) && chartype(c) != DIGIT)
		;
	if (c == 0) return NULL;
	*q++ = 'F';
	*q++ = c;
	while (chartype(c = *p++) == DIGIT)
		*q++ = c;
	*q = 0;
	if (strlen(scratch) == 1) return NULL;
	if (rec = (STRING) retrieve_record(scratch, &len)) {
		stdfree(rec);
		return (WORD) key_to_fam_cacheel(scratch);
	}
	return NULL;
}
/*==========================================
 * eval_binary -- Evaluate binary expression
 *========================================*/
eval_binary (node, pval1, pval2, stab, eflg)
INTERP node; TABLE stab; INT *pval1, *pval2; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	*pval1 = (INT) evaluate(arg, stab, eflg);
	if (*eflg) return;
	*pval2 = (INT) evaluate(inext(arg), stab, eflg);
}
/*========================================
 * eval_indi -- Evaluate person expression
 *======================================*/
NODE eval_indi (expr, stab, eflg, pcel)
INTERP expr; TABLE stab; BOOLEAN *eflg; CACHEEL *pcel;
{
	NODE indi;
	CACHEEL cel = (CACHEEL) evaluate(expr, stab, eflg);
	if (*eflg || !cel) return NULL;
	if (!cnode(cel)) cel = key_to_indi_cacheel(ckey(cel));
	indi = cnode(cel);
	if (nestr("INDI", ntag(indi))) return NULL;
	if (pcel) *pcel = cel;
	return indi;
}
/*=======================================
 * eval_fam -- Evaluate family expression
 *=====================================*/
NODE eval_fam (expr, stab, eflg, pcel)
INTERP expr; TABLE stab; BOOLEAN *eflg; CACHEEL *pcel;
{
	NODE fam;
	CACHEEL cel = (CACHEEL) evaluate(expr, stab, eflg);
	if (*eflg || !cel) return NULL;
	if (!cnode(cel)) cel = key_to_fam_cacheel(ckey(cel));
	fam = cnode(cel);
	if (nestr("FAM", ntag(fam))) return NULL;
	if (pcel) *pcel = cel;
	return fam;
}
