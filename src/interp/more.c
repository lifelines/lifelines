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
 * pre-SourceForge version information:
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
#include "lloptions.h"
#include "feedback.h" /* call_system_cmd */

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSnotone, qSifone, progname;
extern STRING nonstr1,nonstrx,nonlstx,nonvarx,nonnodx;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static STRING allocsubbytes(STRING s, INT start, INT len);
static STRING allocsubstring(STRING s, INT i, INT j);
static void compute_pi(STRING pi, STRING sub);
static INT ll_index(STRING str, STRING sub, INT num);
static INT kmp_search(STRING pi, STRING str, STRING sub, INT num);
static void makestring(PVALUE val, STRING str, INT len, BOOLEAN *eflg);
static STRING rightjustify(STRING str, INT len);

/*********************************************
 * local variables
 *********************************************/

BOOLEAN prog_trace = FALSE;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=============================================================+
 * __extractnames -- Extract name parts from person or NAME node
 *   usage: extractnames(NODE, LIST, VARB, VARB) -> VOID
 *============================================================*/
PVALUE
__extractnames (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list, temp;
	STRING str, str2;
	INT len, sind;
	PNODE nexp = (PNODE) iargs(node);
	PNODE lexp = inext(nexp);
	PNODE lvar = inext(lexp);
	PNODE svar = inext(lvar);
	NODE line;
	PVALUE val = eval_and_coerce(PGNODE, nexp, stab, eflg);

	if (*eflg) {
		prog_error(node, nonnodx, "extractnames", "1");
		return NULL;
	}
	line = (NODE) pvalue(val);
	delete_pvalue(val);
	val = eval_and_coerce(PLIST, lexp, stab, eflg);
	if (*eflg) {
		prog_error(node, nonlstx, "extractnames", "2");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	if (list)
		make_list_empty(list);
	else
		list = create_list();
	*eflg = TRUE;
	if (!iistype(lvar, IIDENT)) {
		prog_error(node, nonvarx, "extractnames", "3");
		return NULL;
	}
	if (!iistype(svar, IIDENT)) {
		prog_error(node, nonvarx, "extractnames", "4");
		return NULL;
	}
	/* if it isn't a NAME line, look under it for a NAME line */
	if (!eqstr("NAME", ntag(line)))
		line = NAME(line);
	/* now create all the values, whether or not we found a NAME line */
	*eflg = FALSE;
	str = (line ? nval(line) : 0);
	temp = create_list();
	if (str && str[0]) {
		name_to_list(str, temp, &len, &sind);
		/* list has string elements */
		FORLIST(temp, el)
			str2 = (STRING)el;
			push_list(list, create_pvalue_from_string(str2));
		ENDLIST
	} else {
		/* no NAME line or empty NAME line */
		len = 0;
		sind = 0;
	}
	insert_symtab(stab, iident(lvar), create_pvalue_from_int(len));
	insert_symtab(stab, iident(svar), create_pvalue_from_int(sind));
	return NULL;
}
/*==============================================================+
 * __extractplaces -- Extract place parts from event or PLAC NODE
 *   usage: extractplaces(NODE, LIST, VARB) -> VOID
 *=============================================================*/
PVALUE
__extractplaces (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list, temp;
	STRING str, str2;
	INT len;
	PNODE nexp = (PNODE) iargs(node);
	PNODE lexp = inext(nexp);
	PNODE lvar = inext(lexp);
	NODE line;
	PVALUE val = eval_and_coerce(PGNODE, nexp, stab, eflg);

	if (*eflg) {
		prog_error(node, nonnodx, "extractplaces", "1");
		return NULL;
	}
	line = (NODE) pvalue(val);
	delete_pvalue(val);
	val = eval_and_coerce(PLIST, lexp, stab, eflg);
	if (*eflg || !val || !pvalue(val)) {
		*eflg = TRUE;
		prog_error(node, nonlstx, "extractplaces", "2");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	if (list)
		make_list_empty(list);
	else
		list = create_list();
	*eflg = TRUE;
	if (!iistype(lvar, IIDENT)) {
		prog_error(node, nonvarx, "extractplaces", "3");
		return NULL;
	}
	insert_symtab(stab, iident(lvar), create_pvalue_from_int(0));
	*eflg = FALSE;
	if (!line) return NULL;
	if (strcmp("PLAC", ntag(line)) && !(line = PLAC(line))) return NULL;
	str = nval(line);
	if (!str || *str == 0) return NULL;
	temp = create_list();
	place_to_list(str, temp, &len);
	FORLIST(temp, el)
		str2 = (STRING)el; /* place_to_list made list of strings */
		push_list(list, create_pvalue_from_string(str2));
	ENDLIST
	insert_symtab(stab, iident(lvar), create_pvalue_from_int(len));
	return NULL;
}
/*==========================================================+
 * __extracttokens -- Extract tokens from a STRING value
 *   usage: extracttokens(STRING, LIST, VARB, STRING) -> VOID
 *=========================================================*/
PVALUE
__extracttokens (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
		prog_error(node, nonstrx, "extracttokens", "1");
		return NULL;
	}
	str = pvalue_to_string(val1);
	val2 = eval_and_coerce(PLIST, lexp, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to extracttokens must be a list");
		return NULL;
	}
	list = pvalue_to_list(val2);
	delete_pvalue(val2);
	make_list_empty(list);
	val2 = eval_and_coerce(PSTRING, dexp, stab, eflg);
	if (*eflg) {
		prog_error(node, nonstrx, "extracttokens", "4");
		return NULL;
	}
	dlm = pvalue_to_string(val2);
#ifdef DEBUG
	llwprintf("dlm = %s\n", dlm);
#endif
	*eflg = TRUE;
	if (!iistype(lvar, IIDENT)) {
		prog_error(node, "3rd arg to extracttokens must be a variable");
		return NULL;
	}
	*eflg = FALSE;
	insert_symtab(stab, iident(lvar), create_pvalue_from_int(0));
	temp = create_list();
	value_to_list(str, temp, &len, dlm);
	FORLIST(temp, el)
		push_list(list, create_pvalue_from_string((STRING)el));
	ENDLIST
	insert_symtab(stab, iident(lvar), create_pvalue_from_int(len));
	delete_pvalue(val1);
	delete_pvalue(val2);
	return NULL;
}
/*===================================+
 * __database -- Return database name
 *   usage: database([BOOL]) -> STRING
 *==================================*/
PVALUE
__database (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
		full = pvalue_to_bool(val);
		delete_pvalue(val);
	}
	return create_pvalue_from_string(
	    (full ? readpath : lastpathname(readpath)));
}
/*===========================================+
 * __index -- Find nth occurrence of substring
 *   usage: index(STRING, STRING, INT) -> INT
 *==========================================*/
PVALUE
__index (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT num;
	PNODE arg = (PNODE) iargs(node);
	STRING sub, str;
	PVALUE val3, val2, val1 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to index is not a string");
		return NULL;
	}
	str = pvalue_to_string(val1);
	arg = inext(arg);
	val2 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to index is not a string");
		return NULL;
	}
	sub = pvalue_to_string(val2);
	arg = inext(arg);
	val3 = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "3rd arg to index is not an integer");
		return NULL;
	}
	num = pvalue_to_int(val3);
	set_pvalue(val3, PINT, (VPTR) ll_index(str, sub, num));
	delete_pvalue(val1);
	delete_pvalue(val2);
	return val3;
}
/*==============================================+
 * __substring -- Find substring of string.
 *   usage: substring(STRING, INT, INT) -> STRING
 *=============================================*/
PVALUE
__substring (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT lo, hi;
	PNODE arg = (PNODE) iargs(node);
	STRING str, substr;
	PVALUE val2, val1 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to substring is not a string");
		return NULL;
	}
	str = pvalue_to_string(val1);
	arg = inext(arg);
	val2 = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to substring is not an integer");
		return NULL;
	}
	lo = pvalue_to_int(val2);
	delete_pvalue(val2);
	arg = inext(arg);
	val2 = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "3rd arg to substring is not an integer");
		return NULL;
	}
	hi = pvalue_to_int(val2);
	/* substr can handle str==NULL */
	substr = allocsubstring(str, lo, hi);
	set_pvalue(val2, PSTRING, substr);
	stdfree(substr);
	delete_pvalue(val1);
	return val2;
}
/*======================================================
 * index -- Find nth occurrence of sub in str (uses KMP)
 * STRING str:  the text being searched
 * STRING sub:  the substring being sought
 * INT num:     which occurrence we want
 *  return value is 1-based index (or 0 if not found)
 *====================================================*/
static INT
ll_index (STRING str, STRING sub, INT num)
{
	INT result;
	STRING pi;
	if (!str || !sub || *str == 0 || *sub == 0) return 0;
	pi = stdalloc(strlen(sub)+1);
	compute_pi(pi, sub);
	result = kmp_search(pi, str, sub, num);
	stdfree(pi);
	return result;
}
/*===============================================
 * kmp_search -- Perform KMP search for substring
 * STRING pi:   the KMP index to avoid backtracking
 * STRING str:  the text being searched
 * STRING sub:  the substring being sought
 * INT num:     which occurrence we want
 *  return value is 1-based index (or 0 if not found)
 *=============================================*/
static INT
kmp_search (STRING pi, STRING str, STRING sub, INT num)
{
	INT i, n, m, q = 0, found = 0;
	n = strlen(str);
	m = strlen(sub);
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
compute_pi (STRING pi, STRING sub)
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
 * allocsubstring -- Return substring
 *  handles NULL input
 *  returns alloc'd memory or NULL
 * i is 1-based start character, j is 1-based end char
 *============================*/
static STRING
allocsubstring (STRING s, INT i, INT j)
{
	INT startch=i-1; /* startch is 0-based, validated below */
	INT numch=j+1-i; /* #characters to copy */
	INT maxlen = s ? strlen(s) : 0;
	/* NULL if NULL or empty string or nonpositive range */
	if (!s || !s[0] || numch<1)
		return NULL;
	/* validate startch */
	if (startch<0)
		startch=0;
	if (0 && uu8) { /* don't turn this on -- index isn't ready for UTF-8 */
		INT start=0, num=0; /* byte units */
		STRING ptr = s;
		while (startch) {
			start += utf8len(ptr[start]);
			if (start >= maxlen)
				return NULL;
			--startch;
		}
		ptr = s + start;
		while (numch) {
			num += utf8len(ptr[0]);
			if (start+num>maxlen) {
				num=maxlen-start;
				break;
			}
			ptr += num;
			--numch;
		}
		return allocsubbytes(s, start, num);
	} else {
		/* 1 byte codeset */
		if (startch + numch > maxlen)
			numch=maxlen-startch;
		return allocsubbytes(s, startch, numch);
	}
}
/*==============================
 * allocsubbytes -- Return subbytestring
 *  assumes valid inputs
 *  returns alloc'd memory
 * start is 0-based start byte, len is # bytes
 * strictly at the byte level
 * client is responsible for codeset
 * Created: 2001/08/02 (Perry Rapp)
 *============================*/
static STRING
allocsubbytes (STRING s, INT start, INT num)
{
	STRING substr;
	substr = stdalloc(num+1);
	strncpy(substr, &s[start], num);
	substr[num] = 0;
	return substr;
}
/*===============================================
 * chooseindi -- Have user choose person from set
 *   usage: chooseindi(SET) -> INDI
 *=============================================*/
PVALUE
__chooseindi (PNODE node, SYMTAB stab, BOOLEAN * eflg)
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
	indi = nztop(choose_from_indiseq(seq, DOASK1, _(qSifone), _(qSnotone)));
	if (!indi) return NULL;
	return create_pvalue_from_indi(indi);
}
/*================================================+
 * choosesubset -- Have user choose subset from set
 *   usage: choosesubset(SET) -> SET
 *===============================================*/
PVALUE
__choosesubset (PNODE node, SYMTAB stab, BOOLEAN * eflg)
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
	msg = (length_indiseq(newseq) > 1) ? _(qSnotone ): _(qSifone);
	if (-1 == choose_list_from_indiseq(msg, newseq)) {
		remove_indiseq(newseq);
		newseq = NULL;
	}
	return create_pvalue(PSET, (VPTR)newseq);
}
/*=========================================================+
 * choosechild -- Have user choose child of person or family
 *   usage: choosechild(INDI|FAM) -> INDI
 *========================================================*/
PVALUE
__choosechild (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	cel = get_cel_from_pvalue(val);
	delete_pvalue(val);
	if (!cel) return create_pvalue_from_indi(NULL);
	key = ckey(cel);
	if (*key == 'I') {
		indi = cnode(cel);
		seq = indi_to_children(indi);
		if (!seq || length_indiseq(seq) < 1)
			return create_pvalue_from_indi(NULL);
		indi = nztop(choose_from_indiseq(seq, DOASK1, _(qSifone), _(qSnotone)));
		remove_indiseq(seq);
		return create_pvalue_from_indi(indi); /* indi may be NULL */
	} else if (*key == 'F') {
		fam = key_to_fam(key);
		seq = fam_to_children(fam);
		if (!seq || length_indiseq(seq) < 1)
			return create_pvalue_from_indi(NULL);
		indi = nztop(choose_from_indiseq(seq, DOASK1, _(qSifone), _(qSnotone)));
		remove_indiseq(seq);
		return create_pvalue_from_indi(indi); /* indi may be NULL */
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
__choosespouse (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	INDISEQ seq;
	if (*eflg) {
		prog_error(node, "the arg to choosespouse must be a person");
		return NULL;
	}
	seq = indi_to_spouses(indi);
	if (!seq || length_indiseq(seq) < 1)
		return create_pvalue_from_indi(NULL);
	indi = nztop(choose_from_indiseq(seq, DOASK1, _(qSifone), _(qSnotone)));
	remove_indiseq(seq);
	return create_pvalue_from_indi(indi); /* indi may be NULL */
}
/*==============================================+
 * choosefam -- Have user choose family of person
 *   usage: choosefam (INDI) -> FAM
 *=============================================*/
PVALUE
__choosefam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE fam, indi = eval_indi(iargs(node), stab, eflg, NULL);
	INDISEQ seq;
	if (*eflg) {
		prog_error(node, "the arg to choosefam must be a person");
		return NULL;
	}
	seq = indi_to_families(indi, TRUE);
	if (!seq || length_indiseq(seq) < 1)
		return create_pvalue_from_fam(NULL);
	fam = nztop(choose_from_indiseq(seq, DOASK1, _(qSifone), _(qSnotone)));
	remove_indiseq(seq);
	return create_pvalue_from_fam(fam); /* fam may be NULL */
}
/*===================================================+
 * makestring -- turn any pvalue into a string
 *  val is input; val,len, and eflg are outputs
 * Created: 2001/04/13, Perry Rapp
 *==================================================*/
static void
makestring (PVALUE val, STRING str, INT len, BOOLEAN *eflg)
{
	UNION u;
	CACHEEL cel;
	STRING txt;

	str[0]=0;
	/*
	Here we treat pvalue(val) as thought it were a UNION
	but it is really a VPTR. This is only safe if sizeof(void *)
	is no less than sizeof(u.i) and sizeof(u.w).

	The real fix is to make pvalue(val) a UNION, but this would
	be difficult, as PVALUE occurs in many places, and there are
	probably lots of casts (which the compiler will not help us find).
	Perry, 2002/01/09
	*/
	u.w = pvalue(val);
	switch(ptype(val)) {
		case PNONE: 
			llstrapp(str, len, uu8, "<NONE>");
			break;
		case PANY:
			llstrapp(str, len, uu8, "<NULL>");
			break;
		case PINT:
		case PFLOAT:
			llstrappf(str, len, uu8, "%f", pvalue_to_float(val));
			break;
		case PBOOL:
			/* TODO: Should we localize this ? */
			llstrappf(str, len, uu8, u.w ? "True" : "False");
			break;
		case PSTRING:
			llstrapp(str, len, uu8, (STRING)pvalue(val));
			break;
		case PGNODE:
			{
				/* TODO: report codeset conversion */
				NODE node = (NODE)pvalue(val);
				if (ntag(node)) {
					llstrappf(str, len, uu8, "%s: ", ntag(node));
				}
				if (nval(node))
					llstrapp(str, len, uu8, nval(node));
			}
			break;
		case PINDI:
		case PFAM:
		case PSOUR:
		case PEVEN:
		case POTHR:
			{
				NODE node;
				cel = get_cel_from_pvalue(val);
				node = cnode(cel);
				txt = generic_to_list_string(node, NULL, len, " ", NULL);
				llstrapp(str, len, uu8, txt);
			}
			break;
		case PLIST:
			llstrapp(str, len, uu8, "<LIST>");
			break;
		case PTABLE:
			llstrapp(str, len, uu8, "<TABLE>");
			break;
		case PSET:
			llstrapp(str, len, uu8, "<SET>");
			break;
		default:
			*eflg = TRUE;
	}
}
/*===================================================+
 * menuchoose -- Have user choose from list of options
 *   usage: menuchoose (LIST [,STRING]) -> INT
 *==================================================*/
PVALUE
__menuchoose (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT i, j, len;
	STRING msg, *strngs;
	STRING ttl = _("Please choose from the following list.");
	PNODE arg = (PNODE) iargs(node);
	LIST list;
	PVALUE vel, val;
	INT nsize;
	val = eval_and_coerce(PLIST, arg, stab, eflg);

	if (*eflg) {
		prog_var_error(node, stab, arg, val, "menuchoose", "1");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	val = NULL;
	if (!list || length_list(list) < 1)
		return create_pvalue_from_int(0);
	msg = NULL;
	arg = (PNODE) inext(arg);
	if (arg) {
		val = eval_and_coerce(PSTRING, arg, stab, eflg);
		if (*eflg) {
			prog_var_error(node, stab, arg, val, nonstrx, "menuchoose", "2");
			return NULL;
		}
		msg = pvalue_to_string(val);
	}
	if (msg && *msg) ttl = msg;
	len = length_list(list);
	strngs = (STRING *) stdalloc(len*sizeof(STRING));
	i = 0;
	nsize = 80;
	FORLIST(list, el)
		vel = (PVALUE) el;
		strngs[i] = (STRING)stdalloc(nsize);
		makestring(vel, strngs[i], nsize, eflg);
		if (*eflg) {
			prog_error(node, _("Illegal type found in list in menuchoose"));
			return NULL;
		}
		++i;
	ENDLIST
	i = choose_from_array(ttl, len, strngs);
	for (j=0; j<len; j++)
		stdfree(strngs[j]);
	stdfree(strngs);
	delete_pvalue(val);
	return create_pvalue_from_int(i + 1);
}
/*================================+
 * runsystem -- Run shell command
 *   usage: runsystem (STRING) -> VOID
 *===============================*/
PVALUE
__runsystem (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING cmd;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "system");
		return NULL;
	}
	cmd = pvalue_to_string(val);
	if (!cmd || *cmd == 0) {
		delete_pvalue(val);
		return NULL;
	}
	if (!getoptint("DenySystemCalls", 0)) {
		call_system_cmd(cmd);
	} else {
		/* llwprintf("Suppressing system(%s) call", cmd); */
	}
	delete_pvalue(val);
	return NULL;
}
/*============================================+
 * firstindi -- Return first person in database
 *   usage: firstindi() -> INDI
 *===========================================*/
PVALUE
__firstindi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_indi_keynum(xref_firsti());
}
/*==========================================+
 * nextindi -- Return next person in database
 *   usage: nextindi(INDI) -> INDI
 *=========================================*/
PVALUE
__nextindi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	NODE indi = eval_indi(arg, stab, eflg, NULL);
	static char key[10];
	INT i;
	if (*eflg) {
		prog_var_error(node, stab, arg, indi, nonind1, "nextindi");
		return NULL;
	}
	if (!indi)
		return create_pvalue_from_indi_keynum(0);
	strcpy(key, indi_to_key(indi));
	i = atoi(&key[1]);
	i = xref_nexti(i);
	return create_pvalue_from_indi_keynum(i);
}
/*==============================================+
 * previndi -- Return previous person in database
 *   usage: previndi(INDI) -> INDI
 *=============================================*/
PVALUE
__previndi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	NODE indi = eval_indi(arg, stab, eflg, NULL);
	static char key[10];
	INT i;
	if (*eflg) {
		prog_var_error(node, stab, arg, indi, nonind1, "previndi");
		return NULL;
	}
	if (!indi)
		return create_pvalue_from_indi_keynum(0);
	strcpy(key, indi_to_key(indi));
	i = atoi(&key[1]);
	i = xref_previ(i);
	return create_pvalue_from_indi_keynum(i);
}
/*===========================================
 * lastindi -- Return last person in database
 *   usage: lastindi() -> INDI
 *=========================================*/
PVALUE
__lastindi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_indi_keynum(xref_lasti());
}
/*===========================================+
 * firstfam -- Return first family in database
 *   usage: firstfam() -> FAM
 *==========================================*/
PVALUE
__firstfam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_fam_keynum(xref_firstf());
}
/*=========================================+
 * nextfam -- Return next family in database
 *   usage: nextfam(FAM) -> FAM
 *========================================*/
PVALUE
__nextfam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	NODE fam = eval_fam(arg, stab, eflg, NULL);
	static char key[10];
	INT i;
	if (*eflg) {
		prog_var_error(node, stab, arg, indi, nonfam1, "nextfam");
		return NULL;
	}
	if (!fam)
		return create_pvalue_from_fam_keynum(0);
	strcpy(key, fam_to_key(fam));
	i = atoi(&key[1]);
	i = xref_nextf(i);
	return create_pvalue_from_fam_keynum(i);
}
/*=============================================+
 * prevfam -- Return previous family in database
 *   usage: prevfam(FAM) -> FAM
 *============================================*/
PVALUE
__prevfam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	NODE fam = eval_fam(arg, stab, eflg, NULL);
	static char key[10];
	INT i;
	if (*eflg) {
		prog_var_error(node, stab, arg, indi, nonfam1, "prevfam");
		return NULL;
	}
	if (!fam)
		return create_pvalue_from_fam_keynum(0);
	strcpy(key, fam_to_key(fam));
	i = atoi(&key[1]);
	i = xref_prevf(i);
	return create_pvalue_from_fam_keynum(i);
}
/*=========================================+
 * lastfam -- Return last family in database
 *   usage: lastfam() -> FAM
 *========================================*/
PVALUE
__lastfam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_fam_keynum(xref_lastf());
}
/*=============================================+
 * getrecord -- Read GEDCOM record from database
 *  usage: getrecord(STRING) -> NODE
 *  usage: dereference(STRING) -> NODE
 *  NOTE: persons and families NOT cached!
 *============================================*/
PVALUE
__getrecord (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING key;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	INT len;
	STRING rawrec = NULL;
	NODE node2 = NULL;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "getrecord");
		return NULL;
	}
	key = pvalue_to_string(val);
	if (*key == '@') key = rmvat(key);
	if (*key == 'I' || *key == 'F' || *key == 'S' ||
	    *key == 'E' || *key == 'X') {
		rawrec = retrieve_raw_record(key, &len);
		if (rawrec)
			node2 = string_to_node(rawrec);
	}
	delete_pvalue(val);
	val = create_pvalue_from_node(node2);
	if (rawrec) stdfree(rawrec);
	return val;
}
/*================================================+
 * reference -- Check if STRING is record reference
 *  usage: reference(STRING) -> BOOLEAN
 *===============================================*/
PVALUE
__reference (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING key;
	BOOLEAN rc;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "reference");
		return NULL;
	}
	key = pvalue_to_string(val);
	rc = (key && *key && (strlen(key) > 2) && (*key == '@') &&
	    (key[strlen(key)-1] == '@'));
	set_pvalue(val, PBOOL, (VPTR) rc);
	return val;
}
/*========================================+
 * rjustify -- Right justify string value
 *   usage: rjustify(STRING, INT) -> STRING
 *=======================================*/
PVALUE
__rjustify (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE sarg = (PNODE) iargs(node);
	PNODE larg = inext(sarg);
	INT len;
	STRING str;
	PVALUE val2, val1 = eval_and_coerce(PSTRING, sarg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, sarg, val1, nonstrx, "rjustify", "1");
		return NULL;
	}
	str = pvalue_to_string(val1);
	val2 = eval_and_coerce(PINT, larg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, larg, val2, nonintx, "rjustify", "2");
		return NULL;
	}
	len = pvalue_to_int(val2);
	delete_pvalue(val2);
	set_pvalue(val1, PSTRING, (VPTR) rightjustify(str, len));
	return val1;
}
/*===========================================
 * rightjustify -- Right justify string value
 *=========================================*/
static STRING
rightjustify (STRING str, INT len)
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
__lock (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT type;
	CACHEEL cel;
	PNODE arg = iargs(node);
	PVALUE val = evaluate(arg, stab, eflg);
	if (*eflg || !val || ((type = ptype(val)) != PINDI && type != PFAM)) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val
		  , _("the arg to lock must be a person or family"));
		return NULL;
	}
	cel = get_cel_from_pvalue(val);
	delete_pvalue(val);
	if (cel) lock_cache(cel);
/* TO DO - ought to ensure this gets freed */
	return NULL;
}
/*===============================================+
 * __unlock -- Unlock person or family from memory
 *   usage: unlock(INDI|FAM) -> VOID
 *==============================================*/
PVALUE
__unlock (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT type;
	CACHEEL cel;
	PNODE arg = iargs(node);
	PVALUE val = evaluate(arg, stab, eflg);
	if (*eflg || !val || ((type = ptype(val)) != PINDI && type != PFAM)) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val
		  , _("the arg to unlock must be a person or family"));
		return NULL;
	}
	cel = get_cel_from_pvalue(val);
	delete_pvalue(val);
	if (cel) unlock_cache(cel);
	return NULL;
}
/*==========================================+
 * __savenode -- Save GEDCOM tree permanently
 *   usage: savenode(NODE) -> NODE
 *=========================================*/
PVALUE
__savenode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE line;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonrec1, "savenode");
		return NULL;
	}
	line = (NODE) pvalue(val);
	if (!line) return val;
	set_pvalue(val, PGNODE, (VPTR) copy_nodes(line, TRUE, TRUE));
	return val;
}
/*===================================================+
 * __genindiset -- Generate set of persons from a name
 *   usage: genindiset(STRING, SET) -> VOID
 *==================================================*/
PVALUE
__genindiset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	STRING name;
	PVALUE val1 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
/* TODO: 2002-10-06: Fix msgs to prog_var_error as above */
		prog_error(node, "1st arg to genindiset must be a string");
		return NULL;
	}
	name = pvalue_to_string(val1);
	if(name) name = strsave(name);
	delete_pvalue(val1);
	arg = inext(arg);
	if (!iistype(arg, IIDENT)) {
		*eflg = TRUE;
		prog_error(node, "2nd arg to genindiset must be a variable");
		return NULL;
	}
	assign_iden(stab, iident(arg), create_pvalue(PSET, (VPTR)NULL));
	if (!name || *name == 0) return NULL;
	assign_iden(stab, iident(arg), create_pvalue(PSET,
	    (VPTR)str_to_indiseq(name, 'I')));
	return NULL;
}
/*POINT*/
/*================================================+
 * __version -- Return the LifeLines version string
 *   usage: version() -> STRING
 *===============================================*/
PVALUE
__version (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_string(get_lifelines_version(120));
}
/*========================================+
 * __pvalue -- Show a PVALUE -- Debug routine
 *   usage: pvalue(ANY) -> STRING
 *=======================================*/
PVALUE
__pvalue (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE val = evaluate(iargs(node), stab, eflg);
#ifdef DEBUG
	debug_show_one_pnode(node);
	llwprintf("\npvalue: %d ",val);
	if(val)
		llwprintf("%d\n",ptype(val));
	else
		printf("BLECH\n");
	show_pvalue(val);
	llwprintf("\n");
#endif
	return create_pvalue_from_string(debug_pvalue_as_string(val));
}
/*============================================+
 * __program -- Returns name of current program
 *   usage: program() -> STRING
 *===========================================*/
PVALUE
__program (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	eflg=eflg; /* unused */
	return create_pvalue_from_string(progname);
}
/*============================================+
 * __debug -- Turn on/off programming debugging
 *   usage: debug(BOOLEAN) -> VOID
 *===========================================*/
PVALUE
__debug (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE val = eval_and_coerce(PBOOL, iargs(node), stab, eflg);
	prog_trace = pvalue_to_bool(val);
	return NULL;
}
/*========================================
 * __getproperty -- Return property string
 *   usage: getproperty(STRING) -> STRING
 *======================================*/
PVALUE
__getproperty(PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE val = eval_and_coerce(PSTRING, iargs(node), stab, eflg);
	if (*eflg || !val || ptype(val) != PSTRING) {
		*eflg = TRUE;
		prog_error(node, nonstr1, "getproperty");
		return NULL;
	}
	if (!pvalue(val))
		return NULL;
	else
		set_pvalue(val, PSTRING, (STRING)get_property(pvalue(val)));
	return val;
}
