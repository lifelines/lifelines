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

#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "interpi.h"
#include "liflines.h"
#include "lloptions.h"
#include "feedback.h" /* call_system_cmd */
#include "zstr.h"
#include "array.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSnotonei, qSifonei;
extern STRING nonint1,nonintx,nonflox,nonstr1,nonstrx,nonlstx,nonvarx;
extern STRING nonind1,nonindx,nonfam1,nonrecx,nonnod1,nonnodx,badtrig;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static STRING allocsubstring(STRING s, INT i, INT j);
static void compute_pi(STRING pi, STRING sub);
static double deg2rad(double deg);
static INT ll_index(STRING str, STRING sub, INT num);
static INT kmp_search(STRING pi, STRING str, STRING sub, INT num);
static void makestring(PVALUE val, STRING str, INT len, BOOLEAN *eflg);
static double rad2deg(double rad);
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
 * llrpt_extractnames -- Extract name parts from person or NAME node
 * usage: extractnames(NODE, LIST, VARB, VARB) -> VOID
 *============================================================*/
PVALUE
llrpt_extractnames (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list=0;
	STRING str=0, str2=0;
	INT len=0, sind=0;
	PNODE nexp = (PNODE) iargs(node);
	PNODE lexp = inext(nexp);
	PNODE lvar = inext(lexp);
	PNODE svar = inext(lvar);
	NODE line=0;
	PVALUE val = eval_and_coerce(PGNODE, nexp, stab, eflg);

	if (*eflg) {
		prog_error(node, nonnodx, "extractnames", "1");
		return NULL;
	}
	line = pvalue_to_node(val);
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
	if (str && str[0]) {
		LIST temp = name_to_list(str, &len, &sind);
		/* list has string elements */
		FORLIST(temp, el)
			str2 = (STRING)el;
			push_list(list, create_pvalue_from_string(str2));
		ENDLIST
		destroy_list(temp);
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
 * llrpt_extractplaces -- Extract place parts from event or PLAC NODE
 * usage: extractplaces(NODE, LIST, VARB) -> VOID
 *=============================================================*/
PVALUE
llrpt_extractplaces (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list=0, temp=0;
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
	line = pvalue_to_node(val);
	delete_pvalue(val);
	val = eval_and_coerce(PLIST, lexp, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_error(node, nonlstx, "extractplaces", "2");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val); /* Could this inadvertently delete the list? */
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
	temp = place_to_list(str, &len);
	FORLIST(temp, el)
		str2 = (STRING)el; /* place_to_list made list of strings */
		push_list(list, create_pvalue_from_string(str2));
	ENDLIST
	insert_symtab(stab, iident(lvar), create_pvalue_from_int(len));
	return NULL;
}
/*==========================================================+
 * llrpt_extracttokens -- Extract tokens from a STRING value
 * usage: extracttokens(STRING, LIST, VARB, STRING) -> VOID
 *=========================================================*/
PVALUE
llrpt_extracttokens (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list=0, temp=0;
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
	temp = value_to_list(str, &len, dlm);
	FORLIST(temp, el)
		push_list(list, create_pvalue_from_string((STRING)el));
	ENDLIST
	insert_symtab(stab, iident(lvar), create_pvalue_from_int(len));
	delete_pvalue(val1);
	delete_pvalue(val2);
	return NULL;
}
/*===================================+
 * llrpt_database -- Return database name
 * usage: database([BOOL]) -> STRING
 *==================================*/
PVALUE
llrpt_database (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_index -- Find nth occurrence of substring
 * usage: index(STRING, STRING, INT) -> INT
 *==========================================*/
PVALUE
llrpt_index (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	set_pvalue_int(val3, ll_index(str, sub, num));
	delete_pvalue(val1);
	delete_pvalue(val2);
	return val3;
}
/*==============================================+
 * llrpt_substring -- Find substring of string.
 * usage: substring(STRING, INT, INT) -> STRING
 *=============================================*/
PVALUE
llrpt_substring (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	set_pvalue_string(val2, substr);
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
/*===============================================
 * llrpt_chooseindi -- Have user choose person from set
 * usage: chooseindi(SET) -> INDI
 *=============================================*/
PVALUE
llrpt_chooseindi (PNODE node, SYMTAB stab, BOOLEAN * eflg)
{
	NODE indi=0;
	INDISEQ seq=0;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to chooseindi is not a set of persons");
		return NULL;
	}
	seq = pvalue_to_seq(val);
	delete_pvalue(val);
	if (!seq || length_indiseq(seq) < 1) return NULL;
	indi = nztop(choose_from_indiseq(seq, DOASK1, _(qSifonei), _(qSnotonei)));
	if (!indi) return NULL;
	return create_pvalue_from_indi(indi);
}
/*================================================+
 * llrpt_choosesubset -- Have user choose subset from set
 * usage: choosesubset(SET) -> SET
 *===============================================*/
PVALUE
llrpt_choosesubset (PNODE node, SYMTAB stab, BOOLEAN * eflg)
{
	STRING msg;
	INDISEQ newseq, seq;
	PVALUE val = eval_and_coerce(PSET, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to choosesubset is not a set of persons");
		return NULL;
	}
	seq = pvalue_to_seq(val);
	delete_pvalue(val);
	if (!seq || length_indiseq(seq) < 1) return NULL;
	newseq = copy_indiseq(seq);
	msg = (length_indiseq(newseq) > 1) ? _(qSnotonei): _(qSifonei);
	if (-1 == choose_list_from_indiseq(msg, newseq)) {
		remove_indiseq(newseq);
		newseq = NULL;
	}
	return create_pvalue_from_seq(newseq);
}
/*=========================================================+
 * llrpt_choosechild -- Have user choose child of person or family
 * usage: choosechild(INDI|FAM) -> INDI
 *========================================================*/
PVALUE
llrpt_choosechild (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT type;
	CNSTRING key=0;
	CACHEEL cel;
	PVALUE val = evaluate(iargs(node), stab, eflg);
	if (*eflg || !val || ((type = which_pvalue_type(val)) != PINDI && type != PFAM)) {
		*eflg = TRUE;
		prog_error(node, "the arg to choosechild must be a person or family");
		return NULL;
	}
	cel = pvalue_to_cel(val);
	delete_pvalue(val);
	if (!cel) return create_pvalue_from_indi(NULL);
	key = cacheel_to_key(cel);
	if (*key == 'I') {
		NODE indi = cacheel_to_node(cel);
		INDISEQ seq = indi_to_children(indi);
		if (!seq || length_indiseq(seq) < 1)
			return create_pvalue_from_indi(NULL);
		indi = nztop(choose_from_indiseq(seq, DOASK1, _(qSifonei), _(qSnotonei)));
		remove_indiseq(seq);
		return create_pvalue_from_indi(indi); /* indi may be NULL */
	} else if (*key == 'F') {
		NODE fam = key_to_fam(key);
		NODE indi=0;
		INDISEQ seq = fam_to_children(fam);
		if (!seq || length_indiseq(seq) < 1)
			return create_pvalue_from_indi(NULL);
		indi = nztop(choose_from_indiseq(seq, DOASK1, _(qSifonei), _(qSnotonei)));
		remove_indiseq(seq);
		return create_pvalue_from_indi(indi); /* indi may be NULL */
	}
	*eflg = TRUE;
	prog_error(node, "major error in choosechild");
	return NULL;
}
/*=================================================+
 * llrpt_choosespouse -- Have user choose spouse of person
 * usage: choosespouse(INDI) -> INDI
 *================================================*/
PVALUE
llrpt_choosespouse (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	indi = nztop(choose_from_indiseq(seq, DOASK1, _(qSifonei), _(qSnotonei)));
	remove_indiseq(seq);
	return create_pvalue_from_indi(indi); /* indi may be NULL */
}
/*==============================================+
 * llrpt_choosefam -- Have user choose family of person
 * usage: choosefam (INDI) -> FAM
 *=============================================*/
PVALUE
llrpt_choosefam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	fam = nztop(choose_from_indiseq(seq, DOASK1, _(qSifonei), _(qSnotonei)));
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
	str[0]=0;

	switch(which_pvalue_type(val)) {
		case PNULL:
			llstrapps(str, len, uu8, "<NULL>");
			break;
		case PINT:
		case PFLOAT:
			llstrappf(str, len, uu8, "%f", pvalue_to_float(val));
			break;
		case PBOOL:
			/* TODO: Should we localize this ? */
			llstrapps(str, len, uu8, pvalue_to_bool(val) ? "True" : "False");
			break;
		case PSTRING:
			llstrapps(str, len, uu8, pvalue_to_string(val));
			break;
		case PGNODE:
			{
				/* TODO: report codeset conversion */
				NODE node = pvalue_to_node(val);
				if (ntag(node)) {
					llstrappf(str, len, uu8, "%s: ", ntag(node));
				}
				if (nval(node))
					llstrapps(str, len, uu8, nval(node));
			}
			break;
		case PINDI:
		case PFAM:
		case PSOUR:
		case PEVEN:
		case POTHR:
			{
				RECORD rec = pvalue_to_record(val);
				NODE node = nztop(rec);
				STRING txt = generic_to_list_string(node, NULL, len, " ", NULL, TRUE);
				llstrapps(str, len, uu8, txt);
			}
			break;
		case PLIST:
			llstrapps(str, len, uu8, "<LIST>");
			break;
		case PTABLE:
			llstrapps(str, len, uu8, "<TABLE>");
			break;
		case PSET:
			llstrapps(str, len, uu8, "<SET>");
			break;
		default:
			*eflg = TRUE;
	}
}
/*===================================================+
 * llrpt_menuchoose -- Have user choose from list of options
 * usage: menuchoose (LIST [,STRING]) -> INT
 *==================================================*/
PVALUE
llrpt_menuchoose (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
			STOPLIST
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
 * llrpt_runsystem -- Run shell command
 * usage: runsystem (STRING) -> VOID
 *===============================*/
PVALUE
llrpt_runsystem (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	if (!getlloptint("DenySystemCalls", 0)) {
		call_system_cmd(cmd);
	} else {
		/* llwprintf("Suppressing system(%s) call", cmd); */
	}
	delete_pvalue(val);
	return NULL;
}
/*============================================+
 * llrpt_firstindi -- Return first person in database
 * usage: firstindi() -> INDI
 *===========================================*/
PVALUE
llrpt_firstindi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_indi_keynum(xref_firsti());
}
/*==========================================+
 * llrpt_nextindi -- Return next person in database
 * usage: nextindi(INDI) -> INDI
 *=========================================*/
PVALUE
llrpt_nextindi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	NODE indi = eval_indi(arg, stab, eflg, NULL);
	static char key[10];
	INT i;
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonind1, "nextindi");
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
 * llrpt_previndi -- Return previous person in database
 * usage: previndi(INDI) -> INDI
 *=============================================*/
PVALUE
llrpt_previndi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	NODE indi = eval_indi(arg, stab, eflg, NULL);
	static char key[10];
	INT i;
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonind1, "previndi");
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
 * llrpt_lastindi -- Return last person in database
 * usage: lastindi() -> INDI
 *=========================================*/
PVALUE
llrpt_lastindi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_indi_keynum(xref_lasti());
}
/*===========================================+
 * llrpt_firstfam -- Return first family in database
 * usage: firstfam() -> FAM
 *==========================================*/
PVALUE
llrpt_firstfam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_fam_keynum(xref_firstf());
}
/*=========================================+
 * llrpt_nextfam -- Return next family in database
 * usage: nextfam(FAM) -> FAM
 *========================================*/
PVALUE
llrpt_nextfam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	NODE fam = eval_fam(arg, stab, eflg, NULL);
	static char key[10];
	INT i;
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonfam1, "nextfam");
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
 * llrpt_prevfam -- Return previous family in database
 * usage: prevfam(FAM) -> FAM
 *============================================*/
PVALUE
llrpt_prevfam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	NODE fam = eval_fam(arg, stab, eflg, NULL);
	static char key[10];
	INT i;
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonfam1, "prevfam");
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
 * llrpt_lastfam -- Return last family in database
 * usage: lastfam() -> FAM
 *========================================*/
PVALUE
llrpt_lastfam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_fam_keynum(xref_lastf());
}
/*=============================================+
 * llrpt_dereference -- Read top node of GEDCOM record from database
 * usage: dereference(STRING) -> NODE
 * NOTE: persons and families NOT cached!
 *============================================*/
PVALUE
llrpt_dereference (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING key;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	INT len;
	STRING rawrec = NULL;
	NODE node2 = NULL;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "dereference");
		return NULL;
	}
	key = pvalue_to_string(val);
	if (*key == '@') key = rmvat(key);
	if (!key) key=""; /* rmvat can return null */
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
 * llrpt_reference -- Check if STRING is record reference
 * usage: reference(STRING) -> BOOLEAN
 *===============================================*/
PVALUE
llrpt_reference (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	set_pvalue_bool(val, rc);
	return val;
}
/*========================================+
 * llrpt_rjustify -- Right justify string value
 * usage: rjustify(STRING, INT) -> STRING
 *=======================================*/
PVALUE
llrpt_rjustify (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	str = rightjustify(str, len); /* newly alloc'd */
	set_pvalue_string(val1, str);
	strfree(&str);
	return val1;
}
/*===========================================
 * rightjustify -- Right justify string value
 *  returns heap-allocated string
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
 * llrpt_lock -- Lock person or family in memory
 * usage: lock(INDI|FAM) -> VOID
 *========================================*/
PVALUE
llrpt_lock (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	CACHEEL cel=0;
	PNODE arg = iargs(node);
	PVALUE val = evaluate(arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val
		  , _("error evaluating arg to lock"));
		return NULL;
	}
	if (!val) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val
		  , _("null arg in lock"));
		return NULL;
	}
	if (is_record_pvalue(val)) {
		cel = pvalue_to_cel(val);
	} else if (is_node_pvalue(val)) {
		NODE nd = pvalue_to_node(val);
		cel = ncel(nd);
		if (!cel) {
			*eflg = TRUE;
			prog_var_error(node, stab, arg, val
			  , _("node passed to lock must be inside a record"));
			return NULL;
		}
	} else {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val
		  , _("the arg to lock must be a record or node"));
		return NULL;
	}
	delete_pvalue(val);
	if (cel) {
		if (cel_rptlocks(cel)>999999) {
			*eflg = TRUE;
			prog_var_error(node, stab, arg, val
			  , _("Error: there are 999,999 locks on arg to lock"));
			return NULL;
		}
		lockrpt_cache(cel);
	}
/* TO DO - ought to ensure this gets freed */
	return NULL;
}
/*===============================================+
 * llrpt_unlock -- Unlock person or family from memory
 * usage: unlock(INDI|FAM) -> VOID
 *==============================================*/
PVALUE
llrpt_unlock (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	CACHEEL cel=0;
	PNODE arg = iargs(node);
	PVALUE val = evaluate(arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val
		  , _("error evaluating arg to unlock"));
		return NULL;
	}
	if (!val) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val
		  , _("null arg in unlock"));
		return NULL;
	}
	if (is_record_pvalue(val)) {
		cel = pvalue_to_cel(val);
	} else if (is_node_pvalue(val)) {
		NODE nd = pvalue_to_node(val);
		cel = ncel(nd);
		if (!cel) {
			*eflg = TRUE;
			prog_var_error(node, stab, arg, val
			  , _("node passed to unlock must be inside a record"));
			return NULL;
		}
	} else {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val
		  , _("the arg to unlock must be a record or node"));
		return NULL;
	}
	delete_pvalue(val);
	if (cel) {
		unlockrpt_cache(cel);
	}
	return NULL;
}
/*==========================================+
 * llrpt_savenode -- Save GEDCOM tree permanently
 * usage: savenode(NODE) -> NODE
 *=========================================*/
PVALUE
llrpt_savenode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE line;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "savenode");
		return NULL;
	}
	line = pvalue_to_node(val);
	if (!line) return val;
	line = copy_nodes(line, TRUE, TRUE);
	set_pvalue_node(val, line);
	return val;
}
/*===================================================+
 * llrpt_genindiset -- Generate set of persons from a name
 * usage: genindiset(STRING, SET) -> VOID
 *==================================================*/
PVALUE
llrpt_genindiset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	STRING name;
	PVALUE seqval=0;
	PVALUE val1 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, nonstrx, "genindiset" , "1");
		delete_pvalue(val1);
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
	seqval = create_pvalue_from_seq(NULL);
	assign_iden(stab, iident(arg), seqval);
	if (!name || *name == 0) return NULL;
	seqval = create_pvalue_from_seq(str_to_indiseq(name, 'I'));
	assign_iden(stab, iident(arg), seqval);
	return NULL;
}
/*================================================+
 * llrpt_version -- Return the LifeLines version string
 * usage: version() -> STRING
 *===============================================*/
PVALUE
llrpt_version (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_string(get_lifelines_version(120));
}
/*========================================+
 * llrpt_pvalue -- Show a PVALUE -- Debug routine
 * usage: pvalue(ANY) -> STRING
 *=======================================*/
PVALUE
llrpt_pvalue (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE val = evaluate(iargs(node), stab, eflg);
	ZSTR zstr;
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
	zstr = describe_pvalue(val);
	val = create_pvalue_from_string(zs_str(zstr));
	zs_free(&zstr);
	return val;
}
/*============================================+
 * llrpt_program -- Returns name of current program
 * usage: program() -> STRING
 *===========================================*/
PVALUE
llrpt_program (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	stab=stab; /* unused */
	eflg=eflg; /* unused */
	return create_pvalue_from_string(irptinfo(node)->fullpath);
}
/*============================================+
 * llrpt_debug -- Turn on/off programming debugging
 * usage: debug(BOOLEAN) -> VOID
 *===========================================*/
PVALUE
llrpt_debug (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE val = eval_and_coerce(PBOOL, iargs(node), stab, eflg);
	prog_trace = pvalue_to_bool(val);
	/* leaking val ? */
	return NULL;
}
/*========================================
 * llrpt_getproperty -- Return property string
 * usage: getproperty(STRING) -> STRING
 *======================================*/
PVALUE
llrpt_getproperty(PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	STRING str;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "getproperty");
		delete_pvalue(val);
		return NULL;
	}
	str = pvalue_to_string(val);
	str = str ? get_property(str) : 0;
	set_pvalue_string(val, str);
	return val;
}
/*========================================
 * deg2rad -- trigonometric conversion: degrees to radians
 * Helper function since C trig functions expect radians
 *======================================*/
static double
deg2rad (double deg)
{
	return ((fmod(deg,360.0))/180.0*M_PI);
}
/*========================================
 * rad2deg -- trigonometric conversion: radians to degrees
 * Helper function since C trig functions return radians
 *======================================*/
static double
rad2deg (double rad)
{
	return (fmod((rad/M_PI*180.0),360.0));
}
/*========================================
 * llrpt_dms2deg -- convert degrees in DMS format to decimal degrees
 * usage: dms2deg(INT, INT, INT, VARB) -> VOID
 *======================================*/
PVALUE
llrpt_dms2deg (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg1 = (PNODE) iargs(node);
	PNODE arg2 = inext(arg1);
	PNODE arg3 = inext(arg2);
	PNODE ret1 = inext(arg3);
	FLOAT decdeg = 0.0;
	INT neg=0;

	PVALUE val = eval_and_coerce(PINT, arg1, stab, eflg);
	if (*eflg) {
		prog_error(node, nonflox, "dms2deg", "1");
		return NULL;
	}
	decdeg += pvalue_to_int(val);
	if (decdeg < 0) {
		decdeg *= -1;
		neg = 1;
	}

	val = eval_and_coerce(PINT, arg2, stab, eflg);
	if (*eflg) {
		prog_error(node, nonflox, "dms2deg", "2");
		return NULL;
	}
	decdeg += (pvalue_to_int(val) / 60.0);

	val = eval_and_coerce(PINT, arg3, stab, eflg);
	if (*eflg) {
		prog_error(node, nonflox, "dms2deg", "3");
		return NULL;
	}
	decdeg += (pvalue_to_int(val) / 3600.0);

	if (neg == 1) {
		decdeg *= -1;
	}

	insert_symtab(stab, iident(ret1), create_pvalue_from_float(decdeg));
	return NULL;
}
/*========================================
 * llrpt_deg2dms -- convert decimal degrees to DMS format
 * usage: deg2dms(FLOAT, VARB, VARB, VARB) -> VOID
 *======================================*/
PVALUE
llrpt_deg2dms (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg1 = (PNODE) iargs(node);
	PNODE ret1 = inext(arg1);
	PNODE ret2 = inext(ret1);
	PNODE ret3 = inext(ret2);
	FLOAT decdeg;
	INT deg, min, sec, neg=0;

	PVALUE val = eval_and_coerce(PFLOAT, arg1, stab, eflg);

	if (*eflg) {
		prog_error(node, nonflox, "deg2dms", "1");
		return NULL;
	}

	decdeg = pvalue_to_float(val);
	if (decdeg < 0) {
		decdeg *= -1;
		neg = 1;
	}
	
	deg = (int)(decdeg);
	decdeg -= deg;
	decdeg *= 60;
	min = (int)(decdeg);
	decdeg -= min;
	decdeg *= 60;
	sec = (int)(decdeg);

	if (neg == 1) { deg *= -1; }

	insert_symtab(stab, iident(ret1), create_pvalue_from_int(deg));
	insert_symtab(stab, iident(ret2), create_pvalue_from_int(min));
	insert_symtab(stab, iident(ret3), create_pvalue_from_int(sec));
	return NULL;
}
/*========================================
 * llrpt_sin -- trigonometric SINE function
 * usage: sin(FLOAT) -> FLOAT
 *======================================*/
PVALUE
llrpt_sin (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PFLOAT, arg, stab, eflg);

	if (*eflg) {
		prog_error(node, nonflox, "sin", "1");
		return NULL;
	}
	
	return create_pvalue_from_float(sin(deg2rad(pvalue_to_float(val))));
}
/*========================================
 * llrpt_cos -- trigonometric COSINE function
 * usage: cos(FLOAT) -> FLOAT
 *======================================*/
PVALUE
llrpt_cos (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PFLOAT, arg, stab, eflg);
	
	if (*eflg) {
		prog_error(node, nonflox, "cos", "1");
		return NULL;
	}
	
	return create_pvalue_from_float(cos(deg2rad(pvalue_to_float(val))));
}
/*========================================
 * llrpt_tan -- trigonometric TANGENT function
 * usage: tan(FLOAT) -> FLOAT
 *======================================*/
PVALUE
llrpt_tan (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PFLOAT, arg, stab, eflg);
	FLOAT val2;

	if (*eflg) {
		prog_error(node, nonflox, "tan", "1");
		return NULL;
	}

	val2 = pvalue_to_float(val);

	/* avoid SIGFPE caused by invalid input */
	if (fmod((val2-90),180) == 0) {
		*eflg = 1;
		prog_error(node, badtrig, "tan", "1");
		return NULL;
	}

	return create_pvalue_from_float(tan(deg2rad(val2)));
}
/*========================================
 * llrpt_arcsin -- trigonometric inverse SINE function
 * usage: arcsin(FLOAT) -> FLOAT
 *======================================*/
PVALUE
llrpt_arcsin (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PFLOAT, arg, stab, eflg);
	FLOAT val2;

	if (*eflg) {
		prog_error(node, nonflox, "arcsin", "1");
		return NULL;
	}

	val2 = pvalue_to_float(val);

	/* avoid SIGFPE caused by invalid input */
	if (val2 > 1.0 || val2 < -1.0) {
		*eflg = 1;
		prog_error(node, badtrig, "arcsin", "1");
		return NULL;
	}

	return create_pvalue_from_float(rad2deg(asin(val2)));
}
/*========================================
 * llrpt_arccos -- trigonometric inverse COSINE function
 * usage: arccos(FLOAT) -> FLOAT
 *======================================*/
PVALUE
llrpt_arccos (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PFLOAT, arg, stab, eflg);
	FLOAT val2;

	if (*eflg) {
		prog_error(node, nonflox, "arccos", "1");
		return NULL;
	}

	val2 = pvalue_to_float(val);

	/* avoid SIGFPE caused by invalid input */
	if (val2 > 1.0 || val2 < -1.0) {
		*eflg = 1;
		prog_error(node, badtrig, "arccos", "1");
		return NULL;
	}

        return create_pvalue_from_float(rad2deg(acos(val2)));
}
/*========================================
 * llrpt_arctan -- trigonometric inverse TANGENT function
 * usage: arctan(FLOAT) -> FLOAT
 *======================================*/
PVALUE
llrpt_arctan (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PFLOAT, arg, stab, eflg);
	
	if (*eflg) {
		prog_error(node, nonflox, "arctan", "1");
		return NULL;
	}
	
	return create_pvalue_from_float(rad2deg(atan(pvalue_to_float(val))));
}
/*========================================
 * llrpt_spdist -- spherical distance calculator
 * usage: spdist(FLOAT, FLOAT, FLOAT, FLOAT) -> FLOAT
 *          (lat0, lon0, lat1, lon1) -> distance (in km)
 *======================================*/
PVALUE
llrpt_spdist (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg1 = (PNODE) iargs(node);
	PNODE arg2 = inext(arg1);
	PNODE arg3 = inext(arg2);
	PNODE arg4 = inext(arg3);
	PVALUE val1, val2, val3, val4;
	FLOAT lat0, lon0, lat1, lon1;
	FLOAT dist, dist1, dist2;

	val1 = eval_and_coerce(PFLOAT, arg1, stab, eflg);
	if (*eflg) {
		prog_error(node, nonflox, "spdist", "1");
		return NULL;
	}
	
	val2 = eval_and_coerce(PFLOAT, arg2, stab, eflg);
	if (*eflg) {
		prog_error(node, nonflox, "spdist", "2");
		return NULL;
	}
	
	val3 = eval_and_coerce(PFLOAT, arg3, stab, eflg);
	if (*eflg) {
		prog_error(node, nonflox, "spdist", "3");
		return NULL;
	}
	
	val4 = eval_and_coerce(PFLOAT, arg4, stab, eflg);
	if (*eflg) {
		prog_error(node, nonflox, "spdist", "4");
		return NULL;
	}

	lat0 = pvalue_to_float(val1);
	lon0 = pvalue_to_float(val2);
	lat1 = pvalue_to_float(val3);
	lon1 = pvalue_to_float(val4);

	/* Suggested by Patrick Texier, and verified by the following sites*/
	/* 1) http://www.skimountaineer.com/CascadeSki/CascadeDistance.php */
	/* 2) http://www.indo.com/distance/dist.pl */
	/* NOTE: Must convert degrees to radians, since that's what the C */
	/* library functions expect! */
	dist1 = cos(deg2rad(lat0)) * cos(deg2rad(lat1)) * cos(deg2rad(lon0)-deg2rad(lon1));
	dist2 = sin(deg2rad(lat0)) * sin(deg2rad(lat1));
	dist = 6380.0 * acos(dist1 + dist2);

	return create_pvalue_from_float(dist);
}
