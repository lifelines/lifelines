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
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 26 Sep 93
 *   3.0.0 - 28 Jun 94    3.0.2 - 04 Apr 95
 *   3.0.3 - 25 Aug 95
 *===========================================================*/
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/* modified 2000-01-26 J.F.Chandler */

#include "sys_inc.h"
#include <curses.h>

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interp.h"
#include "interpi.h"
#include "indiseq.h"
#include "liflines.h"

extern STRING notone, ifone, progname;

static INT ll_index(STRING, STRING, INT);
static void compute_pi(STRING);
static STRING substring (STRING s, INT i, INT j);
static STRING rightjustify (STRING str, INT len);

BOOLEAN prog_debug = FALSE;

/*=============================================================+
 * __extractnames -- Extract name parts from person or NAME node
 *   usage: extractnames(NODE, LIST, VARB, VARB) -> VOID
 *============================================================*/
PVALUE
__extractnames (PNODE node,
                TABLE stab,
                BOOLEAN *eflg)
{
	LIST list, temp;
	STRING str;
	INT len, sind;
	PNODE nexp = (PNODE) iargs(node);
	PNODE lexp = inext(nexp);
	PNODE lvar = inext(lexp);
	PNODE svar = inext(lvar);
	NODE line;
	PVALUE val = eval_and_coerce(PGNODE, nexp, stab, eflg);

	if (*eflg) {
		prog_error(node, "1st arg to extractnames is not a record line");
		return NULL;
	}
	line = (NODE) pvalue(val);
	delete_pvalue(val);
	val = eval_and_coerce(PLIST, lexp, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to extractnames is not a list");
		return NULL;
	}
	list = (LIST) pvalue(val);
	delete_pvalue(val);
	if (list)
		make_list_empty(list);
	else
		list = create_list();
	*eflg = TRUE;
	if (!iistype(lvar, IIDENT)) {
		prog_error(node, "3rd arg to extractnames must be a variable");
		return NULL;
	}
	if (!iistype(svar, IIDENT)) {
		prog_error(node, "4th arg to extractnames must be a variable");
		return NULL;
	}
	if (strcmp("NAME", ntag(line)) && !(line = NAME(line))) {
		prog_error(node, "1st arg to extractnames doesn't lead to a NAME line");
		return NULL;
	}
	insert_pvtable(stab, iident(lvar), PINT, (WORD)0);
	*eflg = FALSE;
	str = nval(line);
	if (!str || *str == 0) return NULL;
	temp = create_list();
	name_to_list(str, temp, &len, &sind);
	FORLIST(temp, el)
		push_list(list, create_pvalue(PSTRING, (WORD)el));
	ENDLIST
	insert_pvtable(stab, iident(lvar), PINT, (WORD)len);
	insert_pvtable(stab, iident(svar), PINT, (WORD)sind);
	return NULL;
}
/*==============================================================+
 * __extractplaces -- Extract place parts from event or PLAC NODE
 *   usage: extractplaces(NODE, LIST, VARB) -> VOID
 *=============================================================*/
PVALUE
__extractplaces (PNODE node,
                 TABLE stab,
                 BOOLEAN *eflg)
{
	LIST list, temp;
	STRING str;
	INT len;
	PNODE nexp = (PNODE) iargs(node);
	PNODE lexp = inext(nexp);
	PNODE lvar = inext(lexp);
	NODE line;
	PVALUE val = eval_and_coerce(PGNODE, nexp, stab, eflg);

	if (*eflg) {
		prog_error(node, "1st arg to extractplaces must be a record line");
		return NULL;
	}
	line = (NODE) pvalue(val);
	delete_pvalue(val);
	val = eval_and_coerce(PLIST, lexp, stab, eflg);
	if (*eflg || !val || !pvalue(val)) {
		*eflg = TRUE;
		prog_error(node, "2nd arg to extractplaces must be a list");
		return NULL;
	}
	list = (LIST) pvalue(val);
	delete_pvalue(val);
	if (list)
		make_list_empty(list);
	else
		list = create_list();
	*eflg = TRUE;
	if (!iistype(lvar, IIDENT)) {
		prog_error(node, "3rd arg to extractplaces must be a variable");
		return NULL;
	}
	insert_pvtable(stab, iident(lvar), PINT, (WORD)0);
	*eflg = FALSE;
	if (!line) return NULL;
	if (strcmp("PLAC", ntag(line)) && !(line = PLAC(line))) return NULL;
	str = nval(line);
	if (!str || *str == 0) return NULL;
	temp = create_list();
	place_to_list(str, temp, &len);
	FORLIST(temp, el)
		push_list(list, create_pvalue(PSTRING, (WORD)el));
	ENDLIST
	insert_pvtable(stab, iident(lvar), PINT, (WORD)len);
	return NULL;
}
/*==========================================================+
 * __extracttokens -- Extract tokens from a STRING value
 *   usage: extracttokens(STRING, LIST, VARB, STRING) -> VOID
 *=========================================================*/
PVALUE
__extracttokens (PNODE node,
                 TABLE stab,
                 BOOLEAN *eflg)
{
	LIST list, temp;
	INT len;
	STRING str, dlm;
	PNODE sexp = (PNODE) iargs(node);
	PNODE lexp = inext(sexp);
	PNODE lvar = inext(lexp);
	PNODE dexp = inext(lvar);
	PVALUE val2, val1 = eval_and_coerce(PSTRING, sexp, stab, eflg);

	if (*eflg) {
		prog_error(node, "1st arg to extracttokens must be a string");
		return NULL;
	}
	str = (STRING) pvalue(val1);
	val2 = eval_and_coerce(PLIST, lexp, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to extracttokens must be a list");
		return NULL;
	}
	list = (LIST) pvalue(val2);
	delete_pvalue(val2);
	val2 = eval_and_coerce(PSTRING, dexp, stab, eflg);
	if (*eflg) {
		prog_error(node, "4th arg to extracttokens must be a string");
		return NULL;
	}
	dlm = (STRING) pvalue(val2);
#ifdef DEBUG
	llwprintf("dlm = %s\n", dlm);
#endif
	*eflg = TRUE;
	if (!iistype(lvar, IIDENT)) {
		prog_error(node, "3rd arg to extracttokens must be a variable");
		return NULL;
	}
	*eflg = FALSE;
	insert_pvtable(stab, iident(lvar), PINT, (WORD)0);
	temp = create_list();
	value_to_list(str, temp, &len, dlm);
	FORLIST(temp, el)
		push_list(list, create_pvalue(PSTRING, (WORD)el));
	ENDLIST
	insert_pvtable(stab, iident(lvar), PINT, (WORD)len);
	delete_pvalue(val1);
	delete_pvalue(val2);
	return NULL;
}
/*===================================+
 * __database -- Return database name
 *   usage: database([BOOL]) -> STRING
 *==================================*/
PVALUE
__database (PNODE node,
            TABLE stab,
            BOOLEAN *eflg)
{
	extern STRING readpath;
	BOOLEAN full = FALSE;
	PVALUE val;
	*eflg = FALSE;
	if (iargs(node)) {
		val = eval_and_coerce(PBOOL, iargs(node), stab, eflg);
		if (*eflg) {
			prog_error(node, "the arg to database is not boolean");
			return NULL;
		}
		full = (BOOLEAN) pvalue(val);
		delete_pvalue(val);
	}
	return create_pvalue(PSTRING,
	    (WORD)(full ? readpath : lastpathname(readpath)));
}
/*===========================================+
 * __index -- Find nth occurrence of substring
 *   usage: index(STRING, STRING, INT) -> INT
 *==========================================*/
PVALUE
__index (PNODE node,
         TABLE stab,
         BOOLEAN *eflg)
{
	INT num;
	PNODE arg = (PNODE) iargs(node);
	STRING sub, str;
	PVALUE val3, val2, val1 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to index is not a string");
		return NULL;
	}
	str = (STRING) pvalue(val1);
	arg = inext(arg);
	val2 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to index is not a string");
		return NULL;
	}
	sub = (STRING) pvalue(val2);
	arg = inext(arg);
	val3 = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "3rd arg to index is not an integer");
		return NULL;
	}
	num = (INT) pvalue(val3);
	set_pvalue(val3, PINT, (WORD) ll_index(str, sub, num));
	delete_pvalue(val1);
	delete_pvalue(val2);
	return val3;
}
/*==============================================+
 * __substring -- Find substring of string.
 *   usage: substring(STRING, INT, INT) -> STRING
 *=============================================*/
PVALUE
__substring (PNODE node,
             TABLE stab,
             BOOLEAN *eflg)
{
	INT lo, hi;
	PNODE arg = (PNODE) iargs(node);
	STRING str;
	PVALUE val2, val1 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to substring is not a string");
		return NULL;
	}
	str = (STRING) pvalue(val1);
	arg = inext(arg);
	val2 = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to substring is not an integer");
		return NULL;
	}
	lo = (INT) pvalue(val2);
	delete_pvalue(val2);
	arg = inext(arg);
	val2 = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "3rd arg to substring is not an integer");
		return NULL;
	}
	hi = (INT) pvalue(val2);
	set_pvalue(val2, PSTRING, (WORD) substring(str, lo, hi));
	delete_pvalue(val1);
	return val2;
}
/*======================================================
 * index -- Find nth occurrence of sub in str (uses KMP)
 *====================================================*/
static char pi[MAXLINELEN];
static INT
ll_index (STRING str,
          STRING sub,
          INT num)
{
        INT i, n, m, q = 0, found = 0;

	if (!str || !sub || *str == 0 || *sub == 0) return 0;
        n = strlen(str);
	m = strlen(sub);
        compute_pi(sub);
        for (i = 1; i <= n; i++) {
                while (q > 0 && sub[q] != str[i-1])
                        q = pi[q];
                if (sub[q] == str[i-1]) q++;
                if (q == m) {
                        if (++found == num) return i - m + 1;
                        q = pi[q];
                }
        }
        return 0;
}
/*========================================
 * compute_pi -- Support routine for index
 *======================================*/
static void
compute_pi (STRING sub)
{
        INT m = strlen(sub), k = 0, q;
        pi[1] = 0;
        for (q = 2; q <= m; q++) {
                while (k > 0 && sub[k] != sub[q-1])
                        k = pi[k];
                if (sub[k] == sub[q-1]) k++;
                pi[q] = k;
        }
}
/*==============================
 * substring -- Return substring
 *  returns static buffer
 *============================*/
static STRING
substring (STRING s,
           INT i,
           INT j)
{
	static char scratch[MAXLINELEN+1];
	if (!s || *s == 0 || i <= 0 || i > j || j > (INT)strlen(s)) return NULL;
	strncpy(scratch, &s[i-1], j-i+1);
	scratch[j-i+1] = 0;
	return (STRING) scratch;
}
/*===============================================
 * chooseindi -- Have user choose person from set
 *   usage: chooseindi(SET) -> INDI
 *=============================================*/
PVALUE
__chooseindi (PNODE node,
              TABLE stab,
              BOOLEAN * eflg)
{
	NODE indi;
	INDISEQ seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to chooseindi is not a set of persons");
		return NULL;
	}
	seq = (INDISEQ) pvalue(val);
	delete_pvalue(val);
	if (!seq || length_indiseq(seq) < 1) return NULL;
	indi = choose_from_indiseq(seq, TRUE, ifone, notone);
	if (!indi) return NULL;
	return create_pvalue(PINDI, (WORD)indi_to_cacheel(indi));
}
/*================================================+
 * choosesubset -- Have user choose subset from set
 *   usage: choosesubset(SET) -> SET
 *===============================================*/
PVALUE
__choosesubset (PNODE node,
                TABLE stab,
                BOOLEAN * eflg)
{
	STRING msg;
	INDISEQ newseq, seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to choosesubset is not a set of persons");
		return NULL;
	}
	seq = (INDISEQ) pvalue(val);
	delete_pvalue(val);
	if (!seq || length_indiseq(seq) < 1) return NULL;
	newseq = copy_indiseq(seq);
	msg = (length_indiseq(newseq) > 1) ? notone : ifone;
	newseq = (INDISEQ) choose_list_from_indiseq(msg, newseq);
	return create_pvalue(PSET, (WORD)newseq);
}
/*=========================================================+
 * choosechild -- Have user choose child of person or family
 *   usage: choosechild(INDI|FAM) -> INDI
 *========================================================*/
PVALUE
__choosechild (PNODE node,
               TABLE stab,
               BOOLEAN *eflg)
{
	INT type;
	STRING key;
	NODE indi, fam;
	INDISEQ seq;
	CACHEEL cel;
	PVALUE val = evaluate(iargs(node), stab, eflg);
	if (*eflg || !val || ((type = ptype(val)) != PINDI && type != PFAM)) {
		*eflg = TRUE;
		prog_error(node, "the arg to choosechild must be a person or family");
		return NULL;
	}
	cel = (CACHEEL) pvalue(val);
	delete_pvalue(val);
	if (!cel) return create_pvalue(PINDI, (WORD)NULL);
	key = ckey(cel);
	if (*key == 'I') {
		indi = key_to_indi(key);
		seq = indi_to_children(indi);
		if (!seq || length_indiseq(seq) < 1)
			return create_pvalue(PINDI, (WORD)NULL);
		indi = choose_from_indiseq(seq, TRUE, ifone, notone);
		remove_indiseq(seq, FALSE);
		if (!indi) return create_pvalue(PINDI, (WORD)NULL);
		return create_pvalue(PINDI, (WORD)indi_to_cacheel(indi));
	} else if (*key == 'F') {
		fam = key_to_fam(key);
		seq = fam_to_children(fam);
		if (!seq || length_indiseq(seq) < 1)
			return create_pvalue(PINDI, (WORD)NULL);
		indi = choose_from_indiseq(seq, TRUE, ifone, notone);
		remove_indiseq(seq, FALSE);
		if (!indi) return create_pvalue(PINDI, (WORD)NULL);
		return create_pvalue(PINDI, (WORD)indi_to_cacheel(indi));
	}
	*eflg = TRUE;
	prog_error(node, "major error in choosechild");
	return NULL;
}
/*=================================================+
 * choosespouse -- Have user choose spouse of person
 *   usage: choosespouse(INDI) -> INDI
 *================================================*/
PVALUE
__choosespouse (PNODE node,
                TABLE stab,
                BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	INDISEQ seq;
	if (*eflg) {
		prog_error(node, "the arg to choosespouse must be a person");
		return NULL;
	}
	seq = indi_to_spouses(indi);
	if (!seq || length_indiseq(seq) < 1)
		return create_pvalue(PINDI, (WORD)NULL);
	indi = choose_from_indiseq(seq, TRUE, ifone, notone);
	remove_indiseq(seq, FALSE);
	if (!indi) return create_pvalue(PINDI, (WORD)NULL);
	return create_pvalue(PINDI, (WORD)indi_to_cacheel(indi));
}
/*==============================================+
 * choosefam -- Have user choose family of person
 *   usage: choosefam (INDI) -> FAM
 *=============================================*/
PVALUE
__choosefam (PNODE node,
             TABLE stab,
             BOOLEAN *eflg)
{
	NODE fam, indi = eval_indi(iargs(node), stab, eflg, NULL);
	INDISEQ seq;
	if (*eflg) {
		prog_error(node, "the arg to choosefam must be a person");
		return NULL;
	}
	seq = indi_to_families(indi, TRUE);
	if (!seq || length_indiseq(seq) < 1)
		return create_pvalue(PFAM, (WORD)NULL);
	fam = choose_from_indiseq(seq, TRUE, ifone, notone);
	remove_indiseq(seq, FALSE);
	if (!fam) return create_pvalue(PFAM, (WORD)NULL);
	return create_pvalue(PFAM, (WORD)fam_to_cacheel(fam));
}
/*===================================================+
 * menuchoose -- Have user choose from list of options
 *   usage: menuchoose (LIST [,STRING]) -> INT
 *==================================================*/
PVALUE
__menuchoose (PNODE node,
              TABLE stab,
              BOOLEAN *eflg)
{
	INT i, len;
	STRING msg, *strngs;
	STRING ttl = (STRING) "Please choose from the following list.";
	PNODE arg = (PNODE) iargs(node);
	LIST list;
	PVALUE vel, val = eval_and_coerce(PLIST, arg, stab, eflg);

	if (*eflg) {
		prog_error(node, "1st arg to menuchoose must be a list of strings");
		return NULL;
	}
	list = (LIST) pvalue(val);
	delete_pvalue(val);
	val = NULL;
	if (!list || length_list(list) < 1)
		return create_pvalue(PINT, (WORD)0);
	msg = NULL;
	arg = (PNODE) inext(arg);
	if (arg) {
		val = eval_and_coerce(PSTRING, arg, stab, eflg);
		if (*eflg) {
			prog_error(node, "2nd arg to menuchoose must be a string");
			return NULL;
		}
		msg = (STRING) pvalue(val);
	}
	if (msg && *msg) ttl = msg;
	len = length_list(list);
	strngs = (STRING *) stdalloc(len*sizeof(STRING));
	i = 0;
	FORLIST(list, el)
		vel = (PVALUE) el;
		strngs[i++] = (STRING) pvalue(vel);
	ENDLIST
	i = choose_from_list(ttl, len, strngs);
	stdfree(strngs);
	delete_pvalue(val);
	return create_pvalue(PINT, (WORD)(i + 1));
}
/*================================+
 * system -- Run shell command
 *   usage: system (STRING) -> VOID
 *===============================*/
PVALUE
__system (PNODE node,
          TABLE stab,
          BOOLEAN *eflg)
{
	STRING cmd;
	PVALUE val = eval_and_coerce(PSTRING, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to system must be a string");
		return NULL;
	}
	cmd = (STRING) pvalue(val);
	if (!cmd || *cmd == 0) {
		delete_pvalue(val);
		return NULL;
	}
	endwin();
#ifndef WIN32
	system("clear");
#endif
	system(cmd);
	touchwin(curscr);
	wrefresh(curscr);
	delete_pvalue(val);
	return NULL;
}
/*=====================================================
 * get_indi_pvalue_from_keynum -- Return indi as pvalue
 *  helper for __firstindi etc
 *  handles i==0
 *===================================================*/
static PVALUE
get_indi_pvalue_from_keynum (INT i)
{
	static char key[10];
	NODE indi;
	PVALUE val;
	STRING record;
	INT len;
	if (!i)
		return create_pvalue(PINDI, (WORD)NULL);
	sprintf(key, "I%d", i);
	ASSERT((record = retrieve_record(key, &len)));
	ASSERT((indi = string_to_node(record)));
	stdfree(record);
	val = create_pvalue(PINDI, (WORD)indi_to_cacheel(indi));
	free_nodes(indi);/*yes*/
	return val;
}
/*============================================+
 * firstindi -- Return first person in database
 *   usage: firstindi() -> INDI
 *===========================================*/
PVALUE
__firstindi (PNODE node,
             TABLE stab,
             BOOLEAN *eflg)
{
	*eflg = FALSE;
	return get_indi_pvalue_from_keynum(xref_firsti());
}
/*==========================================+
 * nextindi -- Return next person in database
 *   usage: nextindi(INDI) -> INDI
 *=========================================*/
PVALUE
__nextindi (PNODE node,
            TABLE stab,
            BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	static char key[10];
	INT i;
	if (*eflg) {
		prog_error(node, "the arg to nextindi is not a person");
		return NULL;
	}
	if (!indi)
		return create_pvalue(PINDI, (WORD)NULL);
	strcpy(key, indi_to_key(indi));
	i = atoi(&key[1]);
	i = xref_nexti(i);
	return get_indi_pvalue_from_keynum(i);
}
/*==============================================+
 * previndi -- Return previous person in database
 *   usage: previndi(INDI) -> INDI
 *=============================================*/
PVALUE
__previndi (PNODE node,
            TABLE stab,
            BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	static char key[10];
	INT i;
	if (*eflg) {
		prog_error(node, "the arg to previndi must be a person");
		return NULL;
	}
	if (!indi)
		return create_pvalue(PINDI, (WORD)NULL);
	strcpy(key, indi_to_key(indi));
	i = atoi(&key[1]);
	i = xref_previ(i);
	return get_indi_pvalue_from_keynum(i);
}
/*===========================================
 * lastindi -- Return last person in database
 *   usage: lastindi() -> INDI
 *=========================================*/
PVALUE
__lastindi (PNODE node,
            TABLE stab,
            BOOLEAN *eflg)
{
	*eflg = FALSE;
	return get_indi_pvalue_from_keynum(xref_lasti());
}
/*====================================================
 * get_fam_pvalue_from_keynum -- Return indi as pvalue
 *  helper for __firstfam etc
 *  handles i==0
 *==================================================*/
static PVALUE
get_fam_pvalue_from_keynum (INT i)
{
	static char key[10];
	NODE fam;
	PVALUE val;
	STRING record;
	INT len;
	if (!i)
		return create_pvalue(PFAM, (WORD)NULL);
	sprintf(key, "F%d", i);
	ASSERT((record = retrieve_record(key, &len)));
	ASSERT((fam = string_to_node(record)));
	stdfree(record);
	val = create_pvalue(PFAM, (WORD)fam_to_cacheel(fam));
	free_nodes(fam);/*yes*/
	return val;
}
/*===========================================+
 * firstfam -- Return first family in database
 *   usage: firstfam() -> FAM
 *==========================================*/
PVALUE
__firstfam (PNODE node,
            TABLE stab,
            BOOLEAN *eflg)
{
	*eflg = FALSE;
	return get_fam_pvalue_from_keynum(xref_firstf());
}
/*=========================================+
 * nextfam -- Return next family in database
 *   usage: nextfam(FAM) -> FAM
 *========================================*/
PVALUE
__nextfam (PNODE node,
           TABLE stab,
           BOOLEAN *eflg)
{
	NODE fam = eval_fam(iargs(node), stab, eflg, NULL);
	static char key[10];
	INT i;
	if (*eflg) {
		prog_error(node, "the arg to nextfam must be a family");
		return NULL;
	}
	if (!fam)
		return create_pvalue(PFAM, (WORD)NULL);
	strcpy(key, fam_to_key(fam));
	i = atoi(&key[1]);
	i = xref_nextf(i);
	return get_fam_pvalue_from_keynum(i);
}
/*=============================================+
 * prevfam -- Return previous family in database
 *   usage: prevfam(FAM) -> FAM
 *============================================*/
PVALUE
__prevfam (PNODE node,
           TABLE stab,
           BOOLEAN *eflg)
{
	NODE fam = eval_fam(iargs(node), stab, eflg, NULL);
	static char key[10];
	INT i;
	if (*eflg) {
		prog_error(node, "the arg to prevfam must be a family");
		return NULL;
	}
	if (!fam)
		return create_pvalue(PFAM, (WORD)NULL);
	strcpy(key, fam_to_key(fam));
	i = atoi(&key[1]);
	i = xref_prevf(i);
	return get_fam_pvalue_from_keynum(i);
}
/*=========================================+
 * lastfam -- Return last family in database
 *   usage: lastfam() -> FAM
 *========================================*/
PVALUE
__lastfam (PNODE node,
           TABLE stab,
           BOOLEAN *eflg)
{
	*eflg = FALSE;
	return get_fam_pvalue_from_keynum(xref_lastf());
}
/*=============================================+
 * getrecord -- Read GEDCOM record from database
 *  usage: getrecord(STRING) -> NODE
 *  usage: dereference(STRING) -> NODE
 *  NOTE: persons and families NOT cached!
 *============================================*/
PVALUE
__getrecord (PNODE node,
             TABLE stab,
             BOOLEAN *eflg)
{
	STRING key, rec;
	PVALUE val = eval_and_coerce(PSTRING, iargs(node), stab, eflg);
	INT len;
	if (*eflg) {
		prog_error(node, "the arg to getrecord must be a string");
		return NULL;
	}
	key = (STRING) pvalue(val);
	if (*key == '@') key = rmvat(key);
#ifdef DEBUG
	llwprintf("__getrecord: key = %s\n", key);
#endif
	if (*key == 'I' || *key == 'F' || *key == 'S' ||
	    *key == 'E' || *key == 'X') {
		rec = retrieve_record(key, &len);
		delete_pvalue(val);
		if (rec == NULL) return create_pvalue(PGNODE, (WORD)NULL);
		val = create_pvalue(PGNODE, (WORD)string_to_node(rec));
		stdfree(rec);
		return val;
	}
	delete_pvalue(val);
	return create_pvalue(PGNODE, (WORD)NULL);
}
#if UNUSED
/*==================================================+
 * freerecord -- Free GEDCOM node tree from getrecord
 *  usage: getrecord(STRING) -> NODE
 *=================================================*/
PVALUE
__freerecord (PNODE node,
              TABLE stab,
              BOOLEAN *eflg)
{
#  error "Unimplemented function!"
}
#endif
/*================================================+
 * reference -- Check if STRING is record reference
 *  usage: reference(STRING) -> BOOLEAN
 *===============================================*/
PVALUE
__reference (PNODE node,
             TABLE stab,
             BOOLEAN *eflg)
{
	STRING key;
	BOOLEAN rc;
	PVALUE val = eval_and_coerce(PSTRING, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to reference must be a string");
		return NULL;
	}
	key = (STRING) pvalue(val);
	rc = (key && *key && (strlen(key) > 2) && (*key == '@') &&
	    (key[strlen(key)-1] == '@'));
	set_pvalue(val, PBOOL, (WORD) rc);
	return val;
}
/*========================================+
 * rjustify -- Right justify string value
 *   usage: rjustify(STRING, INT) -> STRING
 *=======================================*/
PVALUE
__rjustify (PNODE node,
            TABLE stab,
            BOOLEAN *eflg)
{
	PNODE sarg = (PNODE) iargs(node);
	PNODE larg = inext(sarg);
	INT len;
	STRING str;
	PVALUE val2, val1 = eval_and_coerce(PSTRING, sarg, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to rjustify must be a string");
		return NULL;
	}
	str = (STRING) pvalue(val1);
	val2 = eval_and_coerce(PINT, larg, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to rjustify must be an integer");
		return NULL;
	}
	len = (INT) pvalue(val2);
	delete_pvalue(val2);
	set_pvalue(val1, PSTRING, (WORD) rightjustify(str, len));
	return val1;
}
/*===========================================
 * rightjustify -- Right justify string value
 *=========================================*/
static STRING
rightjustify (STRING str,
              INT len)
{
	STRING new;
	INT lstr, nsp, i, j;
	if (len < 1) return NULL;
	if (len > 512) len = 512;
	new = (STRING) stdalloc(len + 1);
	lstr = strlen(str);
	nsp = len - lstr;
	if (nsp < 0) nsp = 0;
	for (i = 0; i < nsp; i++)
		new[i] = ' ';
	for (i = nsp, j = 0; i < len; i++, j++)
		new[i] = str[j];
	new[i] = 0;
	return new;
}
/*=========================================+
 * __lock -- Lock person or family in memory
 *   usage: lock(INDI|FAM) -> VOID
 *========================================*/
PVALUE
__lock (PNODE node,
        TABLE stab,
        BOOLEAN *eflg)
{
	INT type;
	CACHEEL cel;
	PVALUE val = evaluate(iargs(node), stab, eflg);
	if (*eflg || !val || ((type = ptype(val)) != PINDI && type != PFAM)) {
		*eflg = TRUE;
		prog_error(node, "the arg to lock must be a person or family");
		return NULL;
	}
	cel = (CACHEEL) pvalue(val);
	delete_pvalue(val);
	if (cel) lock_cache(cel);
	return NULL;
}
/*===============================================+
 * __unlock -- Unlock person or family from memory
 *   usage: unlock(INDI|FAM) -> VOID
 *==============================================*/
PVALUE
__unlock (PNODE node,
          TABLE stab,
          BOOLEAN *eflg)
{
	INT type;
	CACHEEL cel;
	PVALUE val = evaluate(iargs(node), stab, eflg);
	if (*eflg || !val || ((type = ptype(val)) != PINDI && type != PFAM)) {
		*eflg = TRUE;
		prog_error(node, "the arg to unlock must be a person or family");
		return NULL;
	}
	cel = (CACHEEL) pvalue(val);
	delete_pvalue(val);
	if (cel) unlock_cache(cel);
	return NULL;
}
/*==========================================+
 * __savenode -- Save GEDCOM tree permanently
 *   usage: savenode(NODE) -> NODE
 *=========================================*/
PVALUE
__savenode (PNODE node,
            TABLE stab,
            BOOLEAN *eflg)
{
	NODE line;
	PVALUE val = eval_and_coerce(PGNODE, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to savenode must be a record line");
		return NULL;
	}
	line = (NODE) pvalue(val);
	if (!line) return val;
	set_pvalue(val, PGNODE, (WORD) copy_nodes(line, TRUE, TRUE));
	return val;
}
/*===================================================+
 * __genindiset -- Generate set of persons from a name
 *   usage: genindiset(STRING, SET) -> VOID
 *==================================================*/
PVALUE
__genindiset (PNODE node,
              TABLE stab,
              BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	STRING name;
	PVALUE val1 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to genindiset must be a string");
		return NULL;
	}
	name = (STRING) pvalue(val1);
	if(name) name = strsave(name);
	delete_pvalue(val1);
	arg = inext(arg);
	if (!iistype(arg, IIDENT)) {
		*eflg = TRUE;
		prog_error(node, "2nd arg to genindiset must be a variable");
		return NULL;
	}
	assign_iden(stab, iident(arg), create_pvalue(PSET, (WORD)NULL));
	if (!name || *name == 0) return NULL;
	assign_iden(stab, iident(arg), create_pvalue(PSET,
	    (WORD)str_to_indiseq(name)));
	return NULL;
}
/*POINT*/
/*================================================+
 * __version -- Return the LifeLines version string
 *   usage: version() -> STRING
 *===============================================*/
PVALUE
__version (PNODE node,
           TABLE stab,
           BOOLEAN *eflg)
{
	extern STRING version;
	*eflg = FALSE;
	return create_pvalue(PSTRING, (WORD)version);
}
/*========================================+
 * __pvalue -- Show a PVALUE -- Debug routine
 *   usage: pvalue(ANY) -> STRING
 *=======================================*/
PVALUE
__pvalue (PNODE node,
          TABLE stab,
          BOOLEAN *eflg)
{
	PVALUE val = evaluate(iargs(node), stab, eflg);
#ifdef DEBUG
	show_one_pnode(node);
	llwprintf("\npvalue: %d ",val);
	if(val)
		llwprintf("%d\n",ptype(val));
	else
		printf("BLECH\n");
	show_pvalue(val);
	llwprintf("\n");
#endif
	return create_pvalue(PSTRING, (WORD)pvalue_to_string(val));
}
/*============================================+
 * __program -- Returns name of current program
 *   usage: program() -> STRING
 *===========================================*/
PVALUE
__program (PNODE node,
           TABLE stab,
           BOOLEAN *eflg)
{
	return create_pvalue(PSTRING, (WORD)progname);
}
/*============================================+
 * __debug -- Turn on/off programming debugging
 *   usage: debug(BOOLEAN) -> VOID
 *===========================================*/
PVALUE
__debug (PNODE node,
         TABLE stab,
         BOOLEAN *eflg)
{
	PVALUE val = eval_and_coerce(PBOOL, iargs(node), stab, eflg);
	prog_debug = (BOOLEAN) pvalue(val);
	return NULL;
}
/*========================================
 * __getproperty -- Return property string
 *   usage: getproperty(STRING) -> STRING
 *======================================*/
PVALUE
__getproperty(PNODE node,
              TABLE stab,
              BOOLEAN *eflg)
{
	PVALUE val = eval_and_coerce(PSTRING, iargs(node), stab, eflg);
	if (*eflg || !val || ptype(val) != PSTRING) {
		*eflg = TRUE;
		prog_error(node, "the arg to getproperty is not a string");
		return NULL;
	}
	if (!pvalue(val))
		return NULL;
	else
		set_pvalue(val, PSTRING, (STRING)get_property(pvalue(val)));
	return val;
}
