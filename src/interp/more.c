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
 * more.c -- More builtin functions of report interpreter
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 26 Sep 93
 *   3.0.0 - 28 Jun 94    3.0.2 - 04 Apr 95
 *===========================================================*/

#include <stdio.h>
#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "cache.h"
#include "interp.h"
#include "indiseq.h"

extern STRING notone, ifone;
extern NODE format_and_choose_indi();

static INT index();
static compute_pi();

/*==============================================================
 * __extractnames -- Extract name parts from person or NAME node.
 *   usage: extractnames(NODE, LIST, VARB, VARB) -> VOID
 *============================================================*/
WORD __extractnames (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP nexp = (INTERP) ielist(node);
	INTERP lexp = inext(nexp);
	INTERP lvar = inext(lexp);
	INTERP svar = inext(lvar);
	NODE lin = (NODE) evaluate(nexp, stab, eflg);
	LIST list;
	STRING str;
	INT len, sind;
	if (*eflg || !lin) return NULL;
	list = (LIST) evaluate(lexp, stab, eflg);
	if (*eflg || !list) return NULL;
	*eflg = TRUE;
	if (!iistype(lvar, IIDENT)) return NULL;
	if (!iistype(svar, IIDENT)) return NULL;
	*eflg = FALSE;
	if (strcmp("NAME", ntag(lin)) && !(lin = NAME(lin))) return NULL;
	str = nval(lin);
	if (!str || *str == 0) return NULL;
	name_to_list(str, list, &len, &sind);
	assign_iden(stab, iident(lvar), len);
	assign_iden(stab, iident(svar), sind);
	return NULL;
}
/*===============================================================
 * __extractplaces -- Extract place parts from event or PLAC NODE.
 *   usage: extractplaces(NODE, LIST, VARB) -> VOID
 *=============================================================*/
WORD __extractplaces (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP nexp = (INTERP) ielist(node);
	INTERP lexp = inext(nexp);
	INTERP lvar = inext(lexp);
	NODE lin = (NODE) evaluate(nexp, stab, eflg);
	LIST list;
	STRING str;
	INT len;
	if (*eflg || !lin) return NULL;
	list = (LIST) evaluate(lexp, stab, eflg);
	if (*eflg || !list) return NULL;
	*eflg = TRUE;
	if (!iistype(lvar, IIDENT)) return NULL;
	assign_iden(stab, iident(lvar), 0);
	*eflg = FALSE;
	if (strcmp("PLAC", ntag(lin)) && !(lin = PLAC(lin))) return NULL;
	str = nval(lin);
	if (!str || *str == 0) return NULL;
	place_to_list(str, list, &len);
	assign_iden(stab, iident(lvar), len);
	return NULL;
}
/*===============================================================
 * __extracttokens -- Extract tokens from a STRING value
 *   usage: extracttokens(STRING, LIST, VARB, STRING) -> VOID
 *=============================================================*/
WORD __extracttokens (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP sexp = (INTERP) ielist(node);
	INTERP lexp = inext(sexp);
	INTERP lvar = inext(lexp);
	INTERP dexp = inext(lvar);
	STRING str = (STRING) evaluate(sexp, stab, eflg);
	LIST list;
	INT len;
	STRING dlm;
	if (*eflg || !str || *str == 0) return NULL;
	list = (LIST) evaluate(lexp, stab, eflg);
	if (*eflg || !list) return NULL;
	dlm = (STRING) evaluate(dexp, stab, eflg);
	if (*eflg || !dlm) return NULL;
	*eflg = TRUE;
	if (!iistype(lvar, IIDENT)) return NULL;
	assign_iden(stab, iident(lvar), 0);
	*eflg = FALSE;
	value_to_list(str, list, &len, dlm);
	assign_iden(stab, iident(lvar), len);
	return NULL;
}
/*====================================
 * __database -- Return database name
 *   usage: database([BOOL]) -> STRING
 *==================================*/
WORD __database (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	extern STRING readpath, lastpathname();
	BOOLEAN full = FALSE;
	*eflg = FALSE;
	if (ielist(node)) full = (BOOLEAN) evaluate(ielist(node), stab, eflg);
	if (*eflg) return NULL;
	return (WORD) (full ? readpath : lastpathname(readpath));
}
/*===========================================
 * __index -- Find nth occurrence of substring
 *   usage: index(STRING, STRING, INT) -> INT
 *=========================================*/
WORD __index (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	STRING sub, str = (STRING) evaluate(arg, stab, eflg);
	INT num, index();
	if (*eflg) return NULL;
	arg = inext(arg);
	sub = (STRING) evaluate(arg, stab, eflg);
	if (*eflg) return NULL;
	arg = inext(arg);
	num = (INT) evaluate(arg, stab, eflg);
	if (*eflg) return NULL;
	return (WORD) index(str, sub, num);
}
/*===============================================
 * __substring -- Find substring of string.
 *   usage: substring(STRING, INT, INT) -> STRING
 *=============================================*/
WORD __substring (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	STRING substring(), str = (STRING) evaluate(arg, stab, eflg);
	INT lo, hi;
	if (*eflg) return NULL;
	arg = inext(arg);
	lo = (INT) evaluate(arg, stab, eflg);
	if (*eflg) return NULL;
	arg = inext(arg);
	hi = (INT) evaluate(arg, stab, eflg);
	if (*eflg) return NULL;
	return (WORD) substring(str, lo, hi);
}
/*=======================================================
 * index -- Find nth occurrence of sub in str (uses KMP).
 *=====================================================*/
static char pi[MAXLINELEN];
static INT index (str, sub, num)
STRING str, sub;
INT num;
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
/*=========================================
 * compute_pi -- Support routine for index.
 *=======================================*/
static compute_pi (sub)
STRING sub;
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
/*===============================
 * substring -- Return substring.
 *=============================*/
STRING substring (s, i, j)
STRING s;
INT i, j;
{
	static char scratch[MAXLINELEN+1];
	STRING strncpy();
	if (!s || *s == 0 || i <= 0 || i > j || j > strlen(s)) return NULL;
	strncpy(scratch, &s[i-1], j-i+1);
	scratch[j-i+1] = 0;
	return (STRING) scratch;
}
/*================================================
 * chooseindi -- Have user choose person from set.
 *   usage: chooseindi(SET) -> INDI
 *==============================================*/
WORD __chooseindi (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INDISEQ seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	NODE indi;
	if (!seq || *eflg || length_indiseq(seq) < 1) return NULL;
	indi = format_and_choose_indi(seq, FALSE, FALSE, TRUE, ifone, notone);
	if (!indi) return NULL;
	return (WORD) indi_to_cacheel(indi);
}
/*==================================================
 * choosesubset -- Have user choose subset from set.
 *   usage: choosesubset(SET) -> SET
 *================================================*/
WORD __choosesubset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	STRING msg;
	INDISEQ new, seq = (INDISEQ) evaluate(ielist(node), stab, eflg);
	if (!seq || *eflg || length_indiseq(seq) < 1) return NULL;
	new = copy_indiseq(seq);
	format_indiseq(new, FALSE, FALSE);
	msg = (length_indiseq(new) > 1) ? notone : ifone;
	new = (INDISEQ) choose_list_from_indiseq(msg, new);
	return (WORD) new;
}
/*===========================================================
 * choosechild -- Have user choose child of person or family.
 *   usage: choosechild(INDI|FAM) -> INDI
 *=========================================================*/
WORD __choosechild (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	CACHEEL cel = (CACHEEL) evaluate(ielist(node), stab, eflg);
	STRING key;
	NODE indi, fam;
	INDISEQ seq;
	if (*eflg || !cel) return NULL;
	key = ckey(cel);
	if (*key == 'I') {
		indi = key_to_indi(key);
		seq = indi_to_children(indi);
		if (!seq || length_indiseq(seq) < 1) return NULL;
		indi = format_and_choose_indi(seq, FALSE, FALSE, TRUE,
		    ifone, notone);
		remove_indiseq(seq, FALSE);
		if (!indi) return NULL;
		return (WORD) indi_to_cacheel(indi);
	} else if (*key == 'F') {
		fam = key_to_fam(key);
		seq = fam_to_children(fam);
		if (!seq || length_indiseq(seq) < 1) return NULL;
		indi = format_and_choose_indi(seq, FALSE, FALSE, TRUE,
		    ifone, notone);
		remove_indiseq(seq, FALSE);
		if (!indi) return NULL;
		return (WORD) indi_to_cacheel(indi);
	}
	return NULL;
}
/*===================================================
 * choosespouse -- Have user choose spouse of person.
 *   usage: choosespouse(INDI) -> INDI
 *=================================================*/
WORD __choosespouse (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	INDISEQ seq;
	if (*eflg) return NULL;
	seq = indi_to_spouses(indi);
	if (!seq || length_indiseq(seq) < 1) return NULL;
	indi = format_and_choose_indi(seq, FALSE, TRUE, TRUE, ifone, notone);
	remove_indiseq(seq, FALSE);
	if (!indi) return NULL;
	return (WORD) indi_to_cacheel(indi);
}
/*================================================
 * choosefam -- Have user choose family of person.
 *   usage: choosefam (INDI) -> FAM
 *==============================================*/
WORD __choosefam (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE fam, indi = eval_indi(ielist(node), stab, eflg, NULL);
	INDISEQ seq;
	if (*eflg) return NULL;
	seq = indi_to_families(indi, TRUE);
	if (!seq || length_indiseq(seq) < 1) return NULL;
	fam = format_and_choose_indi(seq, TRUE, TRUE, TRUE, ifone, notone);
	remove_indiseq(seq, FALSE);
	if (!fam) return NULL;
	return (WORD) fam_to_cacheel(fam);
}
/*====================================================
 * menuchoose -- Have user choose from list of options
 *   usage: menuchoose (LIST [,STRING]) -> INT
 *==================================================*/
WORD __menuchoose (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INT i, len;
	STRING msg, *strngs;
	STRING ttl = (STRING) "Please choose from the following list.";
	INTERP arg = (INTERP) ielist(node);
	LIST list = (LIST) evaluate(arg, stab, eflg);
	if (!list || *eflg || length_list(list) < 1) return (WORD) 0;
	msg = NULL;
	arg = (INTERP) inext(arg);
	if (arg) msg = (STRING) evaluate(arg, stab, eflg);
	if (*eflg) return (WORD) 0;
	if (msg && *msg) ttl = msg;
	len = length_list(list);
	strngs = (STRING *) stdalloc(len*sizeof(STRING));
	i = 0;
	FORLIST(list, el)
		strngs[i++] = (STRING) el;
	ENDLIST
	i = choose_from_list(ttl, len, strngs);
	stdfree(strngs);
	return (WORD) (i + 1);
}
/*=================================
 * system -- Run shell command
 *   usage: system (STRING) -> VOID
 *===============================*/
WORD __system (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	STRING cmd = (STRING) evaluate(ielist(node), stab, eflg);
	if (*eflg || !cmd || *cmd == 0) return NULL;
	endwin();
	system("clear");
	system(cmd);
	return NULL;
}
/*=============================================
 * firstindi -- Return first person in database
 *   usage: firstindi() -> INDI
 *===========================================*/
WORD __firstindi (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi;
	static char key[10];
	STRING record;
	INT len, i = 0;
	*eflg = FALSE;
	while (TRUE) {
		sprintf(key, "I%d", ++i);
		if (!(record = retrieve_record(key, &len)))
			return NULL;
		if (!(indi = string_to_node(record))) {
			stdfree(record);
			continue;
		}
		stdfree(record);
		free_nodes(indi);/*yes*/
		return (WORD) indi_to_cacheel(indi);
	}
}
/*============================================
 * nextindi -- Return next person in database.
 *   usage: nextindi(INDI) -> INDI
 *==========================================*/
WORD __nextindi (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	static char key[10];
	STRING record;
	INT len, i;
	if (*eflg) return NULL;
	strcpy(key, indi_to_key(indi));
	i = atoi(&key[1]);
	while (TRUE) {
		sprintf(key, "I%d", ++i);
		if (!(record = retrieve_record(key, &len)))
			return NULL;
		if (!(indi = string_to_node(record))) {
			stdfree(record);
			continue;
		}
		stdfree(record);
		free_nodes(indi);/*yes*/
		return (WORD) indi_to_cacheel(indi);
	}
}
/*================================================
 * previndi -- Return previous person in database.
 *   usage: previndi(INDI) -> INDI
 *==============================================*/
WORD __previndi (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE indi = eval_indi(ielist(node), stab, eflg, NULL);
	static char key[10];
	STRING record;
	INT len, i;
	if (*eflg) return NULL;
	strcpy(key, indi_to_key(indi));
	i = atoi(&key[1]);
	while (TRUE) {
		sprintf(key, "I%d", --i);
		if (!(record = retrieve_record(key, &len)))
			return NULL;
		if (!(indi = string_to_node(record))) {
			stdfree(record);
			continue;
		}
		stdfree(record);
		free_nodes(indi);/*yes*/
		return (WORD) indi_to_cacheel(indi);
	}
}
/*============================================
 * lastindi -- Return last person in database.
 *   usage: lastindi() -> INDI
 *==========================================*/
WORD __lastindi (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
}
/*=============================================
 * firstfam -- Return first family in database.
 *   usage: firstfam() -> FAM
 *===========================================*/
WORD __firstfam (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE fam;
	static char key[10];
	STRING record;
	INT len, i = 0;
	*eflg = FALSE;
	while (TRUE) {
		sprintf(key, "F%d", ++i);
		if (!(record = retrieve_record(key, &len)))
			return NULL;
		if (!(fam = string_to_node(record))) {
			stdfree(record);
			continue;
		}
		stdfree(record);
		free_nodes(fam);/*yes*/
		return (WORD) fam_to_cacheel(fam);
	}
}
/*===========================================
 * nextfam -- Return next family in database.
 *   usage: nextfam(FAM) -> FAM
 *=========================================*/
WORD __nextfam (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE fam = eval_fam(ielist(node), stab, eflg, NULL);
	static char key[10];
	STRING record;
	INT len, i;
	if (*eflg) return NULL;
	strcpy(key, fam_to_key(fam));
	i = atoi(&key[1]);
	while (TRUE) {
		sprintf(key, "F%d", ++i);
		if (!(record = retrieve_record(key, &len)))
			return NULL;
		if (!(fam = string_to_node(record))) {
			stdfree(record);
			continue;
		}
		stdfree(record);
		free_nodes(fam);/*yes*/
		return (WORD) fam_to_cacheel(fam);
	}
}
/*===============================================
 * prevfam -- Return previous family in database.
 *   usage: prevfam(FAM) -> FAM
 *=============================================*/
WORD __prevfam (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE fam = eval_fam(ielist(node), stab, eflg, NULL);
	static char key[10];
	STRING record;
	INT len, i;
	if (*eflg) return NULL;
	strcpy(key, fam_to_key(fam));
	i = atoi(&key[1]);
	while (TRUE) {
		sprintf(key, "F%d", --i);
		if (!(record = retrieve_record(key, &len)))
			return NULL;
		if (!(fam = string_to_node(record))) {
			stdfree(record);
			continue;
		}
		stdfree(record);
		free_nodes(fam);/*yes*/
		return (WORD) fam_to_cacheel(fam);
	}
}
/*============================================
 * lastfam -- Return last family in database.
 *   usage: lastfam() -> FAM
 *==========================================*/
WORD __lastfam (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
}
/*===============================================
 * getrecord -- Read GEDCOM record from database.
 *  usage: getrecord(STRING) -> NODE
 *  usage: dereference(STRING) -> NODE
 *  NOTE: persons and families NOT cached!
 *=============================================*/
WORD __getrecord (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	STRING key = (STRING) evaluate(ielist(node), stab, eflg);
	STRING rec;
	INT len;
	if (*eflg) return NULL;
	if (*key == '@') key = rmvat(key);
	if (*key == 'I' || *key == 'F' || *key == 'S' ||
	    *key == 'E' || *key == 'X') {
		rec = retrieve_record(key, &len);
		if (rec == NULL) return NULL;
		return (WORD) string_to_node(rec);
	}
	*eflg = TRUE;
	return NULL;
}
/*====================================================
 * freerecord -- Free GEDCOM node tree from getrecord.
 *  usage: getrecord(STRING) -> NODE
 *==================================================*/
WORD __freerecord (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
}
/*==================================================
 * reference -- Check if STRING is record reference.
 *  usage: reference(STRING) -> BOOLEAN
 *================================================*/
WORD __reference (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	STRING key = (STRING) evaluate(ielist(node), stab, eflg);
	if (*eflg) return NULL;
	return (WORD) (*key && (strlen(key)) > 2 && (*key == '@') &&
	    (key[strlen(key)-1] == '@'));
}
/*=========================================
 * __rjustify -- Right justify string value
 *   usage: rjustify(STRING, INT) -> STRING
 *=======================================*/
WORD __rjustify (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP sarg = (INTERP) ielist(node);
	INTERP larg = inext(sarg);
	INT len;
	STRING rightjustify();
	STRING str = (STRING) evaluate(sarg, stab, eflg);
	if (*eflg || !str) return NULL;
	len = (INT) evaluate(larg, stab, eflg);
	if (*eflg || len < 1 || len > 512) return NULL;
	return (WORD) rightjustify(str, len);
}
/*===========================================
 * rightjustify -- Right justify string value
 *=========================================*/
STRING rightjustify (str, len)
STRING str;
INT len;
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
/*=========================================
 * __lock -- Lock person or family in memory
 *   usage: lock(INDI|FAM) -> VOID
 *=======================================*/
WORD __lock (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	CACHEEL cel = (CACHEEL) evaluate(ielist(node), stab, eflg);
	if (cel) lock_cache(cel);
	return NULL;
}
/*===============================================
 * __unlock -- Unlock person or family from memory
 *   usage: unlock(INDI|FAM) -> VOID
 *=============================================*/
WORD __unlock (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	CACHEEL cel = (CACHEEL) evaluate(ielist(node), stab, eflg);
	if (cel) unlock_cache(cel);
	return NULL;
}
/*==========================================
 * __savenode -- Save GEDCOM tree permanently
 *   usage: savenode(NODE) -> NODE
 *========================================*/
WORD __savenode (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	NODE this = (NODE) evaluate(ielist(node), stab, eflg);
	if (!this) return NULL;
	return (WORD) copy_nodes(this, TRUE, TRUE);
}
/*===================================================
 * __genindiset -- Generate set of persons from a name
 *   usage: genindiset(STRING, SET) -> VOID
 *=================================================*/
WORD __genindiset (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	INTERP arg = (INTERP) ielist(node);
	STRING name = (STRING) evaluate(arg, stab, eflg);
	if (!name || *name == 0 || *eflg) return NULL;
	arg = inext(arg);
	*eflg = TRUE;
	if (!iistype(arg, IIDENT)) return NULL;
	*eflg = FALSE;
	assign_iden(stab, iident(arg), name_to_indiseq(name));
	return NULL;
}
/*=================================================
 * __version -- Return the LifeLines version string
 *   usage: version() -> STRING
 *===============================================*/
WORD __version (node, stab, eflg)
INTERP node; TABLE stab; BOOLEAN *eflg;
{
	extern STRING version;
	*eflg = FALSE;
	return (WORD) version;
}
