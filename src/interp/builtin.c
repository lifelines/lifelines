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
 * Copyright(c) 1992-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 07 Sep 93
 *   3.0.0 - 07 May 94    3.0.2 - 03 Jan 95
 *   3.0.3 - 02 Jul 96
 *===========================================================*/
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/* modified 2000-04-12 J.F.Chandler */

#include "sys_inc.h"
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interp.h"
#include "indiseq.h"
#include "liflines.h"
#include "feedback.h"
#include "lloptions.h"
#include "date.h"
#include "zstr.h"
#include "codesets.h"

#include "interpi.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING nonint1,nonintx,nonflox,nonstr1,nonstrx,nullarg1,nonfname1;
extern STRING nonnodstr1;
extern STRING nonind1,nonindx,nonfam1,nonrecx,nonnod1,nonnodx;
extern STRING nonvar1,nonvarx,nonboox,nonlst1,nonlstx;
extern STRING nontabx;
extern STRING badargs,badargx,badarg1;
extern STRING qSaskstr,qSchoostrttl;

/*********************************************
 * local function prototypes
 *********************************************/

static VPTR create_list_value_pvalue(LIST list);
static INT normalize_year(struct dnum_s yr);
static ZSTR decode(STRING str, INT * offset);

/*********************************************
 * local variables
 *********************************************/

static struct rfmt_s rpt_long_rfmt; /* short form report format */
static struct rfmt_s rpt_shrt_rfmt; /* long form report format */

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*========================================+
 * __getint -- Have user provide integer
 *   usage: getint(IDEN [,STRING]) --> VOID
 *=======================================*/
PVALUE
__getint (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PNODE arg2;
	INT num;
	STRING msg = 0;
	PVALUE val = NULL;
	if (!iistype(arg, IIDENT)) {
		prog_var_error(node, stab, arg, NULL, nonvarx, "getint", "1");
		*eflg = TRUE;
		return NULL;
	}
	if ((arg2 = inext(arg)) != NULL) {
		val = eval_and_coerce(PSTRING, arg2, stab, eflg);
		if (*eflg) {
			prog_var_error(node, stab, arg2, val, nonstrx, "getint", "2");
			delete_pvalue(val);
			return NULL;
		}
		msg = pvalue_to_string(val);
	}
	if (!msg)
		msg = _("Enter integer for program");
	if (!ask_for_int(msg, &num)) {
		*eflg = TRUE;
		return NULL;
	}
	assign_iden(stab, iident(arg), create_pvalue_from_int(num));
	delete_pvalue(val);
	return NULL;
}
/*========================================+
 * __getstr -- Have user provide string
 *   usage: getstr(IDEN [,STRING]) --> VOID
 *=======================================*/
PVALUE
__getstr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PNODE arg2;
	STRING msg = _(qSchoostrttl);
	PVALUE val = NULL, ansval;
	char buffer[MAXPATHLEN];
	if (!iistype(arg, IIDENT)) {
		prog_var_error(node, stab, arg, NULL, nonvarx, "getstr", "1");
		*eflg = TRUE;
		return NULL;
	}
	if ((arg2 = inext(arg)) != NULL) {
		val = eval_and_coerce(PSTRING, arg2, stab, eflg);
		if (*eflg) {
			prog_var_error(node, stab, arg2, val, nonstrx, "getstr", "2");
			delete_pvalue(val);
			return NULL;
		}
		msg = pvalue_to_string(val);
	}
	if (!ask_for_string(msg, _(qSaskstr), buffer, sizeof(buffer))) {
		/* Cancel yields empty string */
		buffer[0]=0;
	}
	ansval = create_pvalue_from_string(buffer);
	assign_iden(stab, iident(arg), ansval);
	delete_pvalue(val);
	return NULL;
}
/*=========================================+
 * __getindi -- Have user identify person
 *   usage: getindi(IDEN [,STRING]) --> VOID
 *========================================*/
PVALUE
__getindi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PNODE arg2;
	STRING key, msg = 0;
	PVALUE val = NULL;
	if (!iistype(arg, IIDENT)) {
		prog_var_error(node, stab, arg, NULL, nonvarx, "getindi", "1");
		*eflg = TRUE;
		return NULL;
	}
	if ((arg2 = inext(arg)) != 0) {
		val = eval_and_coerce(PSTRING, arg2, stab, eflg);
		if (*eflg) {
			prog_var_error(node, stab, arg2, val, nonstrx, "getindi", "2");
			delete_pvalue(val);
			return NULL;
		}
		msg = pvalue_to_string(val);
	}
	if (!msg)
		msg = _("Identify person for program:");
	assign_iden(stab, iident(arg), create_pvalue_from_indi(NULL));
	uilocale();
	key = ask_for_indi_key(msg, NOCONFIRM, DOASK1);
	rptlocale();
	if (key) {
		assign_iden(stab, iident(arg)
			, create_pvalue_from_indi_key(key));
	}
	if (val) delete_pvalue(val);
	return NULL;
}
/*=====================================+
 * __getfam -- Have user identify family
 *   usage: getfam(IDEN) --> VOID
 *====================================*/
PVALUE
__getfam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE fam;
	if (!iistype(arg, IIDENT)) {
		prog_var_error(node, stab, arg, NULL, nonvar1, "getfam");
		*eflg = TRUE;
		return NULL;
	}
	assign_iden(stab, iident(arg), NULL);
	uilocale();
	fam = nztop(ask_for_fam(_("Enter a spouse from family."),
	    _("Enter a sibling from family.")));
	rptlocale();
	assign_iden(stab, iident(arg), create_pvalue_from_fam(fam));
	return NULL;
}
/*=================================================+
 * __getindiset -- Have user identify set of persons
 *   usage: getindiset(IDEN [,STRING]) --> VOID
 * This introduces both null value indiseqs and null
 * indiseqs into reports so report code must handle them
 *================================================*/
PVALUE
__getindiset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PNODE arg2;
	INDISEQ seq;
	STRING msg = 0;
	PVALUE val = NULL;
	if (!iistype(arg, IIDENT)) {
		prog_var_error(node, stab, arg, NULL, nonvarx, "getindiset", "1");
		*eflg = TRUE;
		return NULL;
	}
	if ((arg2 = inext(arg)) != NULL) {
		val = eval_and_coerce(PSTRING, arg2, stab, eflg);
		if (*eflg) {
			prog_var_error(node, stab, arg2, val, nonstrx, "getindiset", "2");
			delete_pvalue(val);
			return NULL;
		}
		msg = pvalue_to_string(val);
	}
	if (!msg)
		msg = _("Identify list of persons for program:");
	uilocale();
	seq = ask_for_indi_list(msg, TRUE);
	rptlocale();
	if (seq)
		namesort_indiseq(seq); /* in case uilocale != rptlocale */
	delete_pvalue(val);
	assign_iden(stab, iident(arg), create_pvalue_from_set(seq));
	return NULL;
}
/*==================================+
 * __gettoday -- Create today's event
 *   usage: gettoday() --> EVENT
 *=================================*/
PVALUE
__gettoday (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE prnt = create_temp_node(NULL, "EVEN", NULL, NULL);
	NODE chil = create_temp_node(NULL, "DATE", get_todays_date(), prnt);
	node=node; /* unused */
	stab=stab; /* unused */
	eflg=eflg; /* unused */

	nchild(prnt) = chil;
	return create_pvalue_from_node(prnt);
}
/*====================================+
 * __name -- Find person's name
 *   usage: name(INDI[,BOOL]) -> STRING
 *===================================*/
PVALUE __name (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PNODE arg2;
	NODE name, indi = eval_indi(arg, stab, eflg, NULL);
	BOOLEAN caps = TRUE;
	PVALUE val;
	STRING outname = 0;
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonindx, "name", "1");
		return NULL;
	}
	if (!indi) return create_pvalue_from_string("");
	if ((arg2 = inext(arg)) != NULL) {
		val = eval_and_coerce(PBOOL, arg2, stab, eflg);
		if (*eflg) {
			prog_var_error(node, stab, arg2, val, nonboox, "name", "2");
			delete_pvalue(val);
			return NULL;
		}
		caps = pvalue_to_bool(val);
		delete_pvalue(val);
	}
	if (!(name = find_tag(nchild(indi), "NAME"))) {
		if (getoptint("RequireNames", 0)) {
			*eflg = TRUE;
			prog_var_error(node, stab, arg, NULL, _("name: person does not have a name"));
			return NULL;
		}
		return create_pvalue_from_string(0);
	}
	outname = manip_name(nval(name), caps, TRUE, 68);
	return create_pvalue_from_string(outname);
}
/*==================================================+
 * __fullname -- Process person's name
 *   usage: fullname(INDI, BOOL, BOOL, INT) -> STRING
 *=================================================*/
PVALUE
__fullname (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE name, indi;
	PVALUE val;
	BOOLEAN caps;
	BOOLEAN myreg;
	INT len;
	STRING outname;

	indi = eval_indi(arg, stab, eflg, NULL);
	if (*eflg || !indi) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, NULL, nonindx, "fullname", "1");
		return NULL;
	}
	val = eval_and_coerce(PBOOL, arg = inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonboox, "fullname", "2");
		delete_pvalue(val);
		return NULL;
	}
	caps = pvalue_to_bool(val);
	delete_pvalue(val);
	val = eval_and_coerce(PBOOL, arg = inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonboox, "fullname", "3");
		delete_pvalue(val);
		return NULL;
	}
	myreg = pvalue_to_bool(val);
	delete_pvalue(val);
	val = eval_and_coerce(PINT, arg = inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonintx, "fullname", "4");
		delete_pvalue(val);
		return NULL;
	}
	len = pvalue_to_int(val);
	delete_pvalue(val);
	if (!(name = NAME(indi)) || !nval(name)) {
		if (getoptint("RequireNames", 0)) {
			*eflg = TRUE;
			prog_var_error(node, stab, NULL, NULL, _("fullname: person does not have a name"));
			return NULL;
		}
		return create_pvalue_from_string(0);
	}
	outname = manip_name(nval(name), caps, myreg, len);
	return create_pvalue_from_string(outname);
}
/*==================================+
 * __surname -- Find person's surname using new getasurname() routine.
 *   usage: surname(INDI) -> STRING
 *=================================*/
PVALUE
__surname (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE name, indi = eval_indi(arg, stab, eflg, NULL);
	STRING str;

	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonvar1, "surname");
		return NULL;
	}
	if (!indi) return create_pvalue_from_string("");
	if (!(name = NAME(indi)) || !nval(name)) {
		if (getoptint("RequireNames", 0)) {
			*eflg = TRUE;
			prog_var_error(node, stab, arg, NULL, _("surname: person does not have a name"));
			return NULL;
		}
		return create_pvalue_from_string(0);
	}
	str = getasurname(nval(name));
	return create_pvalue_from_string(str);
}
/*========================================+
 * __soundex -- SOUNDEX function on persons
 *   usage: soundex(INDI) -> STRING
 *=======================================*/
PVALUE
__soundex (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE name, indi = eval_indi(arg, stab, eflg, NULL);
	if (*eflg || !indi) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, NULL, nonvar1, "soundex");
		return NULL;
	}
	if (!(name = NAME(indi)) || !nval(name)) {
		if (getoptint("RequireNames", 0)) {
			*eflg = TRUE;
			prog_var_error(node, stab, arg, NULL, _("soundex: person does not have a name"));
			return NULL;
		}
		return create_pvalue_from_string(0);
	}
	return create_pvalue_from_string(soundex(getsxsurname(nval(name))));
}
/*===========================================+
 * __strsoundex -- SOUNDEX function on strings
 *   usage: strsoundex(STRING) -> STRING
 *==========================================*/
PVALUE
__strsoundex (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE newval, val = NULL;
	STRING str;
	val = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "strsoundex");
		delete_pvalue(val);
		return NULL;
	}
	str = soundex(pvalue_to_string(val));
	newval = create_pvalue_from_string(str);
	delete_pvalue(val);
	return newval;
}
/*===========================================+
 * __bytecode -- Input string with escape codes
 *  and optionally specified codeset
 *  eg, bytecode("I$C3$B1$C3$A1rritu", "UTF-8")
 *   usage: bytecode(STRING, [STRING]) -> STRING
 *==========================================*/
PVALUE
__bytecode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	PVALUE newval=0;
	STRING codeset=0;
	INT offset;
	ZSTR zstr=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstrx, "bytecode", "1");
		goto bytecode_exit;
	}
	if (arg) {
		PVALUE val2 = eval_and_coerce(PSTRING, arg, stab, eflg);
		if (*eflg) {
			prog_var_error(node, stab, arg, NULL, nonstrx, "bytecode", "2");
			goto bytecode_exit;
		}
		codeset = strsave(pvalue_to_string(val2));
		delete_pvalue(val2);
	} else {
		codeset = strsave(report_codeset_in);
	}
	zstr = decode(pvalue_to_string(val), &offset);
	if (offset >= 0) {
		prog_var_error(node, stab, arg, val
			, _("Bad escape code at offset %d in bytecode string <%s>")
			, offset+1, pvalue(val));
		*eflg = TRUE;
		goto bytecode_exit;
	}
	/* raw is a special case meaning do NOT go to internal */
	/* raw is for use in test scripts, testing codeconvert */
	if (!eqstr(codeset, "raw")) {
	/* now translate to internal, if possible */
		XLAT xlat = transl_get_xlat_to_int(codeset);
		if (xlat)
			transl_xlat(xlat, zstr);
	}
	newval = create_pvalue_from_string(zs_str(zstr));
bytecode_exit:
	zs_free(&zstr);
	delete_pvalue(val);
	strfree(&codeset);
	return newval;
}
/*===========================================+
 * __convertcode -- Convert string to another codeset
 *  eg, convertcode(str, "UTF-8//html")
 *  or for use in self-tests, convertcode(bytecode("$C3$B1$C3$A1"), "UTF-8", "ISO-8859-1")
 *  (which should come out "ñá"
 *   usage: convertcode(STRING, STRING, [STRING]) -> STRING
 *==========================================*/
PVALUE
__convertcode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	PVALUE newval=0, tempval;
	ZSTR zstr=0;
	STRING cs_src=0, cs_dest=0;
	XLAT xlat=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonstrx, "convertcode", "1");
		goto convertcode_exit;
	}
	arg = inext(arg);
	ASSERT(arg);
	tempval = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonstrx, "convertcode", "2");
		goto convertcode_exit;
	}
	cs_dest = strsave(pvalue_to_string(tempval));
	delete_pvalue(tempval);
	arg = inext(arg);
	if (arg) {
		cs_src = cs_dest;
		tempval = eval_and_coerce(PSTRING, arg, stab, eflg);
		if (*eflg) {
			prog_var_error(node, stab, arg, NULL, nonstrx, "convertcode", "3");
			goto convertcode_exit;
		}
		cs_dest = strsave(pvalue_to_string(tempval));
		delete_pvalue(tempval);
	}
	if (!cs_src)
		cs_src = strsave(int_codeset);
	zstr = zs_news(pvalue_to_string(val));
	xlat = transl_get_xlat(cs_src, cs_dest);
	if (xlat)
		transl_xlat(xlat, zstr);
	newval = create_pvalue_from_string(zs_str(zstr));
convertcode_exit:
	strfree(&cs_src);
	strfree(&cs_dest);
	zs_free(&zstr);
	delete_pvalue(val);
	return newval;
}

/*===============================+
 * decode -- Convert any embedded escape codes into bytes
 *  str:    [IN]  string with embedded escape codes, eg:  "I$C3$B1$C3$A1rritu" 
 *  offset: [OUT] -1 if ok, else 0-based offset of failure
 *==============================*/
static ZSTR
decode (STRING str, INT * offset)
{
	ZSTR zstr = zs_newn((unsigned int)((strlen(str)*2+2)));
	STRING ptr;
	*offset = -1;
	for (ptr=str; *ptr; ++ptr) {
		if (*ptr == '$') {
			INT n = get_hexidecimal(ptr+1);
			/* error if bad hex escape */
			if (n == -1) {
				*offset = ptr - str;
				goto decode_exit;
			}
			ptr += 2;
			zs_appc(zstr, (uchar)(unsigned int)n);
		} else {
			zs_appc(zstr, *ptr);
		}
	}
decode_exit:
	return zstr;
}
/*===========================================+
 * __setlocale -- Set current locale
 *   usage: setlocale(STRING) -> STRING
 *==========================================*/
PVALUE
__setlocale (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE newval, val = eval_and_coerce(PSTRING, arg, stab, eflg);
	STRING str;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "setlocale");
		delete_pvalue(val);
		return NULL;
	}
	str = rpt_setlocale(pvalue(val));
	str = str ? str : "C";
	newval = create_pvalue_from_string(str);
	delete_pvalue(val);
	return newval;
}
/*===============================+
 * __givens -- Find given names
 *   usage: givens(INDI) -> STRING
 *==============================*/
PVALUE
__givens (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE name, indi = eval_indi(arg, stab, eflg, NULL);
	STRING str;
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, _(nonindx), "givens", "1");
		return NULL;
	}
	if (!indi) return create_pvalue_from_string("");
	if (!(name = NAME(indi)) || !nval(name)) {
		if (getoptint("RequireNames", 0)) {
			*eflg = TRUE;
			prog_error(node, _("(givens) person does not have a name"));
			return NULL;
		}
		return create_pvalue_from_string(0);
	}
	str = givens(nval(name));
	return create_pvalue_from_string(str);
}
/*===============================+
 * __set -- Assignment operation
 *   usage: set(IDEN, ANY) -> VOID
 *==============================*/
PVALUE
__set (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE argvar = iargs(node);
	PNODE argexpr = inext(argvar);
	PVALUE val;
	if (!iistype(argvar, IIDENT)) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, NULL, nonvarx, "set", "1");
		return NULL;
	}
	val = evaluate(argexpr, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_var_error(node, stab, argexpr, val, badargx, "set", "2");
		return NULL;
	}
	assign_iden(stab, iident(argvar), val);
	return NULL;
}
/*=========================================+
 * __husband -- Find first husband of family
 *   usage: husband(FAM) -> INDI
 *========================================*/
PVALUE
__husband (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE fam = eval_fam(arg, stab, eflg, NULL);
	if (*eflg || !fam) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, NULL, nonfam1, "husband");
		return NULL;
	}
	return create_pvalue_from_indi(fam_to_husb_node(fam));
}
/*===================================+
 * __wife -- Find first wife of family
 *   usage: wife(FAM) -> INDI
 *==================================*/
PVALUE
__wife (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE fam = eval_fam(arg, stab, eflg, NULL);
	if (*eflg || !fam) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, NULL, nonfam1, "wife");
		return NULL;
	}
	return create_pvalue_from_indi(fam_to_wife_node(fam));
}
/*==========================================+
 * __firstchild -- Find first child of family
 *   usage: firstchild(FAM) -> INDI
 *=========================================*/
PVALUE
__firstchild (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE fam = eval_fam(arg, stab, eflg, NULL);
	if (*eflg || !fam) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, NULL, nonfam1, "firstchild");
		return NULL;
	}
	return create_pvalue_from_indi(fam_to_first_chil(fam));
}
/*========================================+
 * __lastchild -- Find last child of family
 *   usage: lastchild(FAM) -> INDI
 *=======================================*/
PVALUE
__lastchild (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE fam = eval_fam(arg, stab, eflg, NULL);
	if (*eflg || !fam) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, NULL, nonfam1, "lastchild");
		return NULL;
	}
	return create_pvalue_from_indi(fam_to_last_chil(fam));
}
/*=================================+
 * __marr -- Find marriage of family
 *   usage: marriage(FAM) -> EVENT
 *================================*/
PVALUE
__marr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE fam = eval_fam(arg, stab, eflg, NULL);
	NODE event = NULL;
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonfam1, "marriage");
		return NULL;
	}
	if (fam)
		event = MARR(fam);
	return create_pvalue_from_node(event);
}
/*==========================================+
 * __birt -- Find first birth event of person
 *   usage: birth(INDI) -> EVENT
 *=========================================*/
PVALUE
__birt (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE indi = eval_indi(arg, stab, eflg, NULL);
	NODE event = NULL;
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, _(nonind1), "birth");
		return NULL;
	} 
	if (indi)
		event = BIRT(indi);
	return create_pvalue_from_node(event);
}
/*==========================================+
 * __deat -- Find first death event of person
 *   usage: death(INDI) -> EVENT
 *=========================================*/
PVALUE
__deat (PNODE node, SYMTAB stab, BOOLEAN  *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE indi = eval_indi(arg, stab, eflg, NULL);
	NODE event = NULL;
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, _(nonind1), "death");
		return NULL;
	}
	if (indi)
		event = DEAT(indi);
	return create_pvalue_from_node(event);
}
/*============================================+
 * __bapt -- Find first baptism event of person
 *   usage: baptism(INDI) -> EVENT
 *===========================================*/
PVALUE
__bapt (PNODE node, SYMTAB stab, BOOLEAN  *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE indi = eval_indi(arg, stab, eflg, NULL);
	NODE event = NULL;
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, _(nonind1), "baptism");
		return NULL;
	}
	if (indi)
		event = BAPT(indi);
	return create_pvalue_from_node(event);
}
/*===========================================+
 * __buri -- Find first burial event of person
 *   usage: burial(INDI) -> EVENT
 *==========================================*/
PVALUE
__buri (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE indi = eval_indi(arg, stab, eflg, NULL);
	NODE event = NULL;
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, _(nonind1), "burial");
		return NULL;
	}
	if (indi)
		event = BURI(indi);
	return create_pvalue_from_node(event);
}
/*====================================+
 * __titl -- Find first title of person
 *   usage: title(INDI) -> STRING
 *===================================*/
PVALUE
__titl (PNODE node, SYMTAB stab, BOOLEAN  *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE titl, indi = eval_indi(arg, stab, eflg, NULL);
	STRING titlstr = "";
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, _(nonind1), "title");
		return NULL;
	}
	if (indi) {
		titl = find_tag(nchild(indi), "TITL");
		if (titl)
			titlstr = nval(titl);
	}
	return create_pvalue_from_string(titlstr);
}
/*=======================================================
 * rpt_shrt_format_date -- short form of date for reports
 *  This is used by the report "short" function.
 * Created: 2001/10/29 (Perry Rapp)
 *=====================================================*/
static STRING
rpt_shrt_format_date (STRING date)
{
	/* TO DO - customizing options */
	/*static unsigned char buffer[MAXLINELEN+1];*/
	if (!date) return NULL;
	return shorten_date(date);
}
/*========================================================
 * rpt_shrt_format_plac -- short form of place for reports
 *  This is used by the report "short" function.
 * Created: 2001/10/29 (Perry Rapp)
 *======================================================*/
static STRING
rpt_shrt_format_plac (STRING plac)
{
	/* TO DO - add customization */
	if (!plac) return NULL;
	return shorten_plac(plac);
}
/*==============================================================
 * init_rpt_reformat -- set up formatting structures for reports
 * Created: 2001/10/29 (Perry Rapp)
 *============================================================*/
static void
init_rpt_reformat (void)
{
	/* reformats are transforms applied to strings (date or place)
	before they are finally output */

	/* Set up long reformats */
	memset(&rpt_long_rfmt, 0, sizeof(rpt_long_rfmt));
	rpt_long_rfmt.rfmt_date = 0; /* use date as is */
	rpt_long_rfmt.rfmt_plac = 0; /* use place as is */
	rpt_long_rfmt.combopic = "%1, %2";
	/* Set up short reformats */
	memset(&rpt_shrt_rfmt, 0, sizeof(rpt_shrt_rfmt));
	rpt_shrt_rfmt.rfmt_date = &rpt_shrt_format_date;
	rpt_shrt_rfmt.rfmt_plac = &rpt_shrt_format_plac;
	rpt_shrt_rfmt.combopic = "%1, %2";
}
/*===================================+
 * __long -- Return long form of event
 *   usage: long(EVENT) -> STRING
 *==================================*/
PVALUE
__long (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	NODE even;
	STRING str;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "long");
		return NULL;
	}
	even = pvalue_to_node(val);
	delete_pvalue(val);

	/* if we were cleverer, we wouldn't call this every time */
	init_rpt_reformat();

	str = event_to_string(even, &rpt_long_rfmt);
	return create_pvalue_from_string(str);
}
/*=====================================+
 * __short -- Return short form of event
 *   usage: short(EVENT) -> STRING
 *====================================*/
PVALUE
__short (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	NODE even;
	/* RFMT rfmt = NULL; */ /* currently no reformatting for reports */
	STRING str;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "short");
		return NULL;
	}
	even = pvalue_to_node(val);
	delete_pvalue(val);

	/* if we were cleverer, we wouldn't call this every time */
	init_rpt_reformat();

	str = event_to_string(even, &rpt_shrt_rfmt);
	return create_pvalue_from_string(str);
}
/*===============================+
 * __fath -- Find father of person
 *   usage: father(INDI) -> INDI
 *==============================*/
PVALUE
__fath (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	NODE fath = NULL;
	if (*eflg) {
		prog_error(node, _(nonind1), "father");
		return NULL;
	}
	if (indi)
		fath = indi_to_fath(indi);
	return create_pvalue_from_indi(fath);
}
/*===============================+
 * __moth -- Find mother of person
 *   usage: mother(INDI) -> INDI
 *==============================*/
PVALUE
__moth (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, _(nonind1), "mother");
		return NULL;
	}
	if (!indi) return create_pvalue_from_indi(NULL);
	return create_pvalue_from_indi(indi_to_moth(indi));
}
/*===========================================+
 * __parents -- Find parents' family of person
 *   usage: parents(INDI) -> FAM
 *==========================================*/
PVALUE
__parents (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, "arg to parents must be a person");
		return NULL;
	}
	if (!indi) return create_pvalue_from_fam(NULL);
	return create_pvalue_from_fam(indi_to_famc(indi));
}
/*==========================================+
 * __nextsib -- Find person's younger sibling
 *   usage: nextsib(INDI) -> INDI
 *=========================================*/
PVALUE
__nextsib (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, _(nonind1), "nextsib");
		return NULL;
	}
	if (!indi) return create_pvalue_from_indi(NULL);
	return create_pvalue_from_indi(indi_to_next_sib_old(indi));
}
/*========================================+
 * __prevsib -- Find person's older sibling
 *   usage: prevsib(INDI) -> INDI
 *=======================================*/
PVALUE
__prevsib (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, _(nonind1), "prevsib");
		return NULL;
	}
	if (!indi) return create_pvalue_from_indi(NULL);
	return create_pvalue_from_indi(indi_to_prev_sib_old(indi));
}
/*========================================+
 * __d -- Return cardinal integer as string
 *   usage: d(INT) -> STRING
 *=======================================*/
PVALUE
__d (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	static char scratch[20];
	PVALUE val;
	val = eval_and_coerce(PINT, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, nonint1, "d");
		return NULL;
	}
	sprintf(scratch, "%d", pvalue_to_int(val));
	set_pvalue_string(val, scratch);
	return val;
}
/*=============================================+
 * __f -- Return floating point number as string
 *   usage: f(FLOAT[,INT]) -> STRING
 *============================================*/
PVALUE
__f (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	char scratch[20];
	char format[10];
	UNION u;
	INT prec = 2;
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PFLOAT, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonflox, "f", "1");
		return NULL;
	}
	u.w = pvalue(val);
	arg = inext(arg);
	if (arg) {
		val = eval_and_coerce(PINT, arg, stab, eflg);
		if (*eflg) {
			prog_var_error(node, stab, arg, val, nonintx, "f", "2");
			return NULL;
		}
		prec = pvalue_to_int(val);
		if (prec < 0) prec = 0;
		if (prec > 10) prec = 10;
	}
	sprintf(format, "%%.%df", prec);

	sprintf(scratch, format, u.f);
	set_pvalue_string(val, scratch);
	return val;
}
/*==========================================+
 * ___alpha -- Convert small integer to letter
 *   usage: alpha(INT) -> STRING
 *=========================================*/
PVALUE
___alpha (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	static char scratch[2];
	INT i;
	PVALUE val = evaluate(iargs(node), stab, eflg);
	if (*eflg) return NULL;
	TYPE_CHECK(PINT, val);
	i = pvalue_to_int(val);
	delete_pvalue(val);
	if (i < 1 || i > 26)
		sprintf(scratch, "XX");
	else
		sprintf(scratch, "%c", 'a' + i - 1);
	return create_pvalue_from_string(scratch);
}
/*================================================+
 * __ord -- Convert small integer to ordinal string
 *   usage: ord(INT) -> STRING
 *===============================================*/
static char *ordinals[] = {
	N_("first"), N_("second"), N_("third"), N_("fourth"), N_("fifth"),
	N_("sixth"), N_("seventh"), N_("eighth"), N_("ninth"), N_("tenth"),
	N_("eleventh"), N_("twelfth")
};
PVALUE
__ord (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	static char scratch[12];
	PVALUE val = evaluate(iargs(node), stab, eflg);
	INT i;
	TYPE_CHECK(PINT, val);
	i = pvalue_to_int(val);
	delete_pvalue(val);
	if (*eflg || i < 1) return NULL;
	if (i > 12)
		sprintf(scratch, _("%dth"), i);
	else
		sprintf(scratch, _(ordinals[i - 1]));
	return create_pvalue_from_string(scratch);
}
/*==================================================+
 * __card -- Convert small integer to cardinal string
 *   usage: card(INT) -> STRING
 *=================================================*/
static char *cardinals[] = {
	N_("zero"), N_("one"), N_("two"), N_("three"), N_("four"), N_("five"),
	N_("six"), N_("seven"), N_("eight"), N_("nine"), N_("ten"),
	N_("eleven"), N_("twelve")
};
PVALUE
__card (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	static char scratch[8];
	PVALUE val = evaluate(iargs(node), stab, eflg);
	INT i;
	if (*eflg) return NULL;
	TYPE_CHECK(PINT, val);
	i = pvalue_to_int(val);
	delete_pvalue(val);
	if (i < 0 || i > 12)
		sprintf(scratch, "%d", i);
	else
		sprintf(scratch, _(cardinals[i]));
	return create_pvalue_from_string(scratch);
}
/*==========================================+
 * __roman -- Convert integer to Roman numeral
 *   usage: roman(INT) -> STRING
 *=========================================*/
static char *rodigits[] = {
	"", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix"
};
static char *rotens[] = {
	"", "x", "xx", "xxx", "xl", "l", "lx", "lxx", "lxxx", "xc"
};
PVALUE
__roman (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	static char scratch[10];
	PVALUE val = evaluate(iargs(node), stab, eflg);
	INT i;
	if (*eflg) return NULL;
	TYPE_CHECK(PINT, val);
	i = pvalue_to_int(val);
	delete_pvalue(val);
	if (i < 1 || i >= 99)
		sprintf(scratch, "%d", i);
	else
		sprintf(scratch, "%s%s", rotens[i/10], rodigits[i%10]);
	return create_pvalue_from_string(scratch);
}
/*================================================+
 * __nchildren -- Find number of children in family
 *   usage: nchildren(FAM) -> INT
 *===============================================*/
PVALUE
__nchildren (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE fam = eval_fam(arg, stab, eflg, NULL);
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonfam1, "nchildren");
		return NULL;
	}
	if (!fam) return create_pvalue_from_int(0);
	return create_pvalue_from_int(length_nodes(CHIL(fam)));
}
/*===================================================+
 * __nfamilies -- Find number of families person is in
 *   usage: nfamilies(INDI) -> INT
 *==================================================*/
PVALUE
__nfamilies (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE indi = eval_indi(arg, stab, eflg, NULL);
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonind1, "nfamilies");
		return NULL;
	}
	if (!indi) return create_pvalue_from_int(0);
	return create_pvalue_from_int(length_nodes(FAMS(indi)));
}
/*===============================================+
 * __nspouses -- Find number of spouses person has
 *   usage: nspouses(INDI) -> INT
 *==============================================*/
PVALUE
__nspouses (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT nspouses;
	PNODE arg = (PNODE) iargs(node);
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonind1, "nspouses");
		return NULL;
	}
	if (!indi) return create_pvalue_from_int(0);
	FORSPOUSES(indi,spouse,fam,nspouses) ENDSPOUSES
	return create_pvalue_from_int(nspouses);
}
/*=============================+
 * __eq -- Equal operation
 *   usage: eq(ANY, ANY) -> BOOL
 *============================*/
PVALUE
__eq (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val2, val1 = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, badargx, "eq", "1");
		return NULL;
	}
	val2 = evaluate(arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val2, badargx, "eq", "2");
		return NULL;
	}
	eq_pvalues(val1, val2, eflg, &zerr); /* result in val1, val2 deleted */
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	return val1;
}
/*=============================+
 * __ne -- Not equal operation
 *   usage: ne(ANY, ANY) -> BOOL
 *============================*/
PVALUE
__ne (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val2, val1 = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, badargx, "ne", "1");
		return NULL;
	}
	val2 = evaluate(arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val2, badargx, "ne", "2");
		return NULL;
	}
	ne_pvalues(val1, val2, eflg, &zerr); /* result in val1, val2 deleted */
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	return val1;
}
/*===============================+
 * __le -- Less or equal operation
 *   usage: le(ANY, ANY) -> BOOL
 *==============================*/
PVALUE
__le (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val2, val1 = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, badargx, "le", "1");
		return NULL;
	}
	val2 = evaluate(arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val2, badargx, "le", "2");
		return NULL;
	}
	le_pvalues(val1, val2, eflg, &zerr); /* result in val1, val2 deleted */
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	return val1;
}
/*==================================+
 * __ge -- Greater or equal operation
 *   usage: ge(ANY, ANY) -> BOOL
 *=================================*/
PVALUE
__ge (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val2, val1 = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, badargx, "ge", "1");
		return NULL;
	}
	val2 = evaluate(arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val2, badargx, "le", "2");
		return NULL;
	}
	ge_pvalues(val1, val2, eflg, &zerr); /* result in val1, val2 deleted */
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	return val1;
}
/*============================+
 * __lt -- Less than operation
 *   usage: lt(ANY,ANY) -> BOOL
 *===========================*/
PVALUE
__lt (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val2, val1 = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, badargx, "lt", "1");
		return NULL;
	}
	val2 = evaluate(arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val2, badargx, "lt", "2");
		return NULL;
	}
	lt_pvalues(val1, val2, eflg, &zerr); /* result in val1, val2 deleted */
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	return val1;
}
/*==============================+
 * __gt -- Greater than operation
 *   usage: gt(ANY, ANY) -> BOOL
 *=============================*/
PVALUE
__gt (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val2, val1 = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, badargx, "gt", "1");
		return NULL;
	}
	val2 = evaluate(inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val2, badargx, "gt", "2");
		return NULL;
	}
	gt_pvalues(val1, val2, eflg, &zerr); /* result in val1, val2 deleted */
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	return val1;
}
/*=================================+
 * __and -- And operation
 *   usage: and(ANY [,ANY]+) -> BOOL
 *================================*/
PVALUE
__and (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val2, val1 = eval_and_coerce(PBOOL, arg, stab, eflg);
	BOOLEAN rc = TRUE;
	if (*eflg) {
		prog_error(node, "an arg to and is not boolean");
		return NULL;
	}
	rc = rc && pvalue_to_bool(val1);
	delete_pvalue(val1);
	while ((arg = inext(arg))) {
		if (rc) {
			val2 = eval_and_coerce(PBOOL, arg, stab, eflg);
			if (*eflg) {
				prog_error(node, "an arg to and is not boolean");
				return NULL;
			}
			rc = rc && pvalue_to_bool(val2);
			delete_pvalue(val2);
		}
	}
	return create_pvalue_from_bool(rc);
}
/*================================+
 * __or -- Or operation
 *   usage: or(ANY [,ANY]+) -> BOOL
 *===============================*/
PVALUE
__or (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	BOOLEAN rc = FALSE;
	PVALUE val2, val1 = eval_and_coerce(PBOOL, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "an arg to or is not boolean");
		return NULL;
	}
	rc = rc || pvalue_to_bool(val1);
	delete_pvalue(val1);
	while ((arg = inext(arg))) {
		if (!rc) {
			val2 = eval_and_coerce(PBOOL, arg, stab, eflg);
			if (*eflg) {
				prog_error(node, "an arg to or is not boolean");
				return NULL;
			}
			rc = rc || pvalue_to_bool(val2);
			delete_pvalue(val2);
		}
	}
	return create_pvalue_from_bool(rc);
}
/*================================+
 * __add -- Add operation
 *   usage: add(INT [,INT]+) -> INT
 *===============================*/
PVALUE
__add (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val2, val1 = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	INT iarg=1;
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, badargx, "add", "1");
		return NULL;
	}
	while ((arg = inext(arg))) {
		++iarg;
		val2 = evaluate(arg, stab, eflg);
		if (*eflg) {
			char numstr[33];
			snprintf(numstr, sizeof(numstr), "%d", iarg);
			prog_var_error(node, stab, arg, val2, badargx, "add", numstr);
			return NULL;
		}
		add_pvalues(val1, val2, eflg, &zerr); /* result in val1, val2 deleted */
		if (*eflg) {
			prog_error(node, zs_str(zerr));
			zs_free(&zerr);
			return NULL;
		}
	}
	return val1;
}
/*=============================+
 * __sub -- Subtract operation
 *   usage: sub(INT, INT) -> INT
 *============================*/
PVALUE
__sub (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val2, val1 = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, badargx, "sub", "1");
		return NULL;
	}
	val2 = evaluate(arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val2, badargx, "sub", 2);
		return NULL;
	}
	sub_pvalues(val1, val2, eflg, &zerr); /* result in val1, val2 deleted */
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	return val1;
}
/*================================+
 * __mul -- Multiply operation
 *   usage: mul(INT [,INT]+) -> INT
 *===============================*/
PVALUE
__mul (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val2, val1 = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	INT iarg=1;
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, badargx, "mul", "1");
		return NULL;
	}
	while ((arg = inext(arg))) {
		++iarg;
		val2 = evaluate(arg, stab, eflg);
		if (*eflg) {
			char numstr[33];
			snprintf(numstr, sizeof(numstr), "%d", iarg);
			prog_var_error(node, stab, arg, val2, badargx, "mul", numstr);
			return NULL;
		}
		mul_pvalues(val1, val2, eflg, &zerr); /* result in val1, val2 deleted */
		if (*eflg) {
			prog_error(node, zs_str(zerr));
			zs_free(&zerr);
			return NULL;
		}
	}
	return val1;
}
/*=============================+
 * __div -- Divide operation
 *   usage: div(INT, INT) -> INT
 *============================*/
PVALUE
__div (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val2, val1 = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, badargx, "div", "1");
		return NULL;
	}
	val2 = evaluate(arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val2, badargx, "div", 2);
		return NULL;
	}
	div_pvalues(val1, val2, eflg, &zerr); /* result in val1, val2 deleted */
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	return val1;
}
/*=============================+
 * __mod -- Modulus operation
 *   usage: mod(INT, INT) -> INT
 *============================*/
PVALUE
__mod (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val2, val1 = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, badargx, "mod", "1");
		return NULL;
	}
	val2 = evaluate(arg=inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val2, badargx, "mod", 2);
		return NULL;
	}
	mod_pvalues(val1, val2, eflg, &zerr); /* result in val1, val2 deleted */
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	return val1;
}
/*=================================+
 * __exp -- Exponentiation operation
 *   usage: exp(INT, INT) -> INT
 *================================*/
PVALUE
__exp (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE val2, val1 = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, badargx, "sub", "1");
		return NULL;
	}
	val2 = evaluate(inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val2, badargx, "sub", 2);
		return NULL;
	}
	exp_pvalues(val1, val2, eflg, &zerr); /* result in val1, val2 deleted */
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	return val1;
}
/*===========================+
 * __neg -- Negation operation
 *   usage: neg(INT) -> INT
 *==========================*/
PVALUE
__neg (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE val = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, badarg1, "neg");
		return NULL;
	}
	if (*eflg) return NULL;
	neg_pvalue(val, eflg, &zerr); /* result in val */
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	return val;
}
/*===========================+
 * __incr -- Increment variable
 *   usage: incr(VARB) -> VOID
 *==========================*/
PVALUE
__incr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE vararg = (PNODE) iargs(node);
	PVALUE val;
	ZSTR zerr=0;
	if (!iistype(vararg, IIDENT)) {
		*eflg = TRUE;
		prog_error(node, "arg to incr must be a variable");
		return NULL;
	}
	val = evaluate(vararg, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_var_error(node, stab, vararg, val, badarg1, "incr");
		return NULL;
	}
	incr_pvalue(val, eflg, &zerr);
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	assign_iden(stab, iident(vararg), val);
	return NULL;
}
/*============================+
 * __decr -- Decrement variable
 *   usage: decr(VARB) -> VOID
 *===========================*/
PVALUE
__decr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE vararg = (PNODE) iargs(node);
	PVALUE val;
	ZSTR zerr=0;
	if (!iistype(vararg, IIDENT)) {
		*eflg = TRUE;
		prog_error(node, "arg to decr must be a variable");
		return NULL;
	}
	val = evaluate(vararg, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_var_error(node, stab, vararg, val, badarg1, "decr");
		return NULL;
	}
	decr_pvalue(val, eflg, &zerr);
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	assign_iden(stab, iident(vararg), val);
	return NULL;
}
/*======================================+
 * __strcmp -- Compare two strings
 *   usage: strcmp(STRING, STRING) -> INT
 *=====================================*/
PVALUE
__strcmp (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	STRING str1, str2, emp = (STRING) "";
	PVALUE val2, val1 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to strcmp is not a string");
		return NULL;
	}
	val2 = eval_and_coerce(PSTRING, inext(arg), stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to strcmp is not a string");
		return NULL;
	}
	str1 = pvalue_to_string(val1);
	str2 = pvalue_to_string(val2);
	if (!str1) str1 = emp;
	if (!str2) str2 = emp;

	set_pvalue_int(val1, cmpstrloc(str1, str2));
	delete_pvalue(val2);
	return val1;
}
/*=========================================+
 * __nestr -- Compare two strings
 *   usage: nestr(STRING, STRING) -> BOOLEAN
 *========================================*/
PVALUE
__nestr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	STRING str1, str2, emp = (STRING) "";
	PVALUE val2, val1 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to nestr is not a string");
		return NULL;
	}
	val2 = eval_and_coerce(PSTRING, inext(arg), stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to nestr is not a string");
		return NULL;
	}
	str1 = pvalue_to_string(val1);
	str2 = pvalue_to_string(val2);
	if (!str1) str1 = emp;
	if (!str2) str2 = emp;
	set_pvalue(val1, PBOOL, (VPTR)(nestr(str1, str2) != 0));
	delete_pvalue(val2);
	return val1;
}
/*=========================================+
 * __eqstr -- Compare two strings
 *   usage: eqstr(STRING, STRING) -> BOOLEAN
 *========================================*/
PVALUE
__eqstr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	STRING str1, str2, emp = (STRING) "";
	PVALUE val2, val1 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to eqstr is not a string");
		return NULL;
	}
	val2 = eval_and_coerce(PSTRING, inext(arg), stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to eqstr is not a string");
		return NULL;
	}
	str1 = pvalue_to_string(val1);
	str2 = pvalue_to_string(val2);
	if (!str1) str1 = emp;
	if (!str2) str2 = emp;
	set_pvalue_bool(val1, (eqstr(str1, str2) != 0));
	delete_pvalue(val2);
	return val1;
}
/*=======================================+
 * __strtoint -- Convert string to integer
 *  usage: strtoint(STRING) -> INT
 *======================================*/
PVALUE
__strtoint (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE val=NULL;
	STRING str=0;
	val = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "strtoint");
		delete_pvalue(val);
		return NULL;
	}
	str = pvalue_to_string(val);
	set_pvalue_int(val, str ? atoi(pvalue(val)): 0);
	return val;
}
/*============================+
 * __list -- Create list
 *   usage: list(IDENT) -> VOID
 *===========================*/
PVALUE
__list (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list;
	PNODE arg = (PNODE) iargs(node);
	if (!iistype(arg, IIDENT)) {
		prog_var_error(node, stab, arg, NULL, nonvar1, "list");
		*eflg = TRUE;
		return NULL;
	}
	*eflg = FALSE;
	list = create_list();
	assign_iden(stab, iident(arg), create_pvalue(PLIST, (VPTR) list));
	return NULL;
}
/*=======================================+
 * __push -- Push element on front of list
 *   usage: push(LIST, ANY) -> VOID
 *======================================*/
PVALUE
__push (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	LIST list;
	PVALUE el;
	PVALUE val = eval_and_coerce(PLIST, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonlst1, "1");
		delete_pvalue(val);
		return NULL;
	}
	el = evaluate(inext(arg), stab, eflg);
	if (*eflg || !el) {
		*eflg = TRUE;
		prog_error(node, "2nd arg to push is in error");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	push_list(list, el);
	return NULL;
}
/*======================================+
 * __inlist -- see if element is in list
 *   usage: inlist(LIST, STRING) -> BOOL
 *=====================================*/
PVALUE
__inlist (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	LIST list;
	PVALUE el;
	BOOLEAN bFound;
	PVALUE val = eval_and_coerce(PLIST, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonlstx, "inlist", "1");
		delete_pvalue(val);
		return NULL;
	}
	el = evaluate(inext(arg), stab, eflg);
	if (*eflg || !el) {
		*eflg = TRUE;
		prog_error(node, "2nd arg to inlist is in error");
		return NULL;
	}
	list = pvalue_to_list(val);
	bFound = in_list(list, el, eqv_pvalues) >= 0;
	set_pvalue(val, PBOOL, (VPTR)bFound);
	delete_pvalue(el);
	return val;
}
/*====================================+
 * __enqueue -- Enqueue element on list
 *   usage: enqueue(LIST, ANY) -> VOID
 *===================================*/
PVALUE
__enqueue (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	LIST list=NULL;
	PVALUE el=NULL;
	PVALUE val = eval_and_coerce(PLIST, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonlstx, "enqueue", "1");
		delete_pvalue(val);
		return NULL;
	}
	el = evaluate(inext(arg), stab, eflg);
	if (*eflg || !el) {
		*eflg = TRUE;
		prog_error(node, "2nd arg to enqueue is in error");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	enqueue_list(list, el);
	return NULL;
}
/*========================================+
 * __requeue -- Add element to back of list
 *   usage: requeue(LIST, ANY) -> VOID
 *=======================================*/
PVALUE
__requeue (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	LIST list=NULL;
	PVALUE el=NULL;
	PVALUE val = eval_and_coerce(PLIST, arg, stab, eflg);
	if (*eflg || !val || ptype(val) != PLIST) {
		*eflg = TRUE;
		prog_error(node, "1st arg to requeue is not a list");
		return NULL;
	}
	el = evaluate(inext(arg), stab, eflg);
	if (*eflg || !el) {
		*eflg = TRUE;
		prog_error(node, "2nd arg to requeue is in error");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	back_list(list, el);
	return NULL;
}
/*=======================================+
 * __pop -- Pop element from front of list
 *   usage: pop(LIST) -> ANY
 *======================================*/
PVALUE
__pop (PNODE node, SYMTAB stab, BOOLEAN  *eflg)
{
	LIST list;
	PVALUE val = eval_and_coerce(PLIST, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to pop is not a list");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	if (is_empty_list(list)) return create_pvalue_any();
	return (PVALUE) pop_list(list);
}
/*=============================================+
 * __dequeue -- Remove element from back of list
 *   usage dequeue(LIST) -> ANY
 *============================================*/
PVALUE
__dequeue (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list;
	PVALUE val = eval_and_coerce(PLIST, iargs(node), stab, eflg);
	if (*eflg || !val || ptype(val) != PLIST) {
		*eflg = TRUE;
		prog_error(node, nonlst1, "dequeue");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	val = (PVALUE) dequeue_list(list);
	if (!val) return create_pvalue_any();
	return val;
}
/*=================================+
 * __empty -- Check if list is empty
 *   usage: empty(LIST) -> BOOL
 *================================*/
PVALUE
__empty (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list;
	BOOLEAN bEmpty;
	PVALUE val = eval_and_coerce(PLIST, iargs(node), stab, eflg);
	if (*eflg || !val || ptype(val) != PLIST) {
		*eflg = TRUE;
		prog_error(node, "the arg to empty is not a list");
		return NULL;
	}
	list = pvalue_to_list(val);
	bEmpty = is_empty_list(list);
	set_pvalue(val, PBOOL, (VPTR)bEmpty);
	return val;
}
/*===================================
 * create_list_value_pvalue -- 
 *  Create filler element
 *  Used when accessing list as an array
 *  Created: 2002/12/29 (Perry Rapp)
 *=================================*/
static VPTR
create_list_value_pvalue (LIST list)
{
	list=list; /* unused */
	return create_pvalue_any();
}
/*==================================+
 * __getel -- Get nth value from list
 *   usage: getel(LIST, INT) -> ANY
 *=================================*/
PVALUE
__getel (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list;
	INT ind;
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PLIST, arg, stab, eflg);
	if (*eflg || !val || ptype(val) != PLIST) {
		*eflg = TRUE;
		prog_error(node, nonlstx, "getel", "1");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	val = eval_and_coerce(PINT, inext(arg), stab, eflg);
	if (*eflg || !val || ptype(val) != PINT) {
		*eflg = TRUE;
		prog_error(node, nonintx, "getel", "2");
		return NULL;
	}
	ind = pvalue_to_int(val);
	delete_pvalue(val);
	if (!(val = (PVALUE) get_list_element(list, ind, &create_list_value_pvalue)))
		return create_pvalue_any();
	return copy_pvalue(val);
}
/*=======================================+
 * __setel -- Set nth value in list
 *   usage: setel(LIST, INT, ANY) -> VOID
 *======================================*/
PVALUE
__setel (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list;
	INT ind;
	PNODE arg = (PNODE) iargs(node);
	PVALUE old, val = eval_and_coerce(PLIST, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to setel is not a list");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	arg = inext(arg);
	val = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to setel is not an integer");
		return NULL;
	}
	ind = pvalue_to_int(val);
	delete_pvalue(val);
	val = evaluate(inext(arg), stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_error(node, "3rd arg to setel is in error");
		return NULL;
	}
	old = (PVALUE) get_list_element(list, ind, &create_list_value_pvalue);
	if(old) delete_pvalue(old);
	set_list_element(list, ind, val, &create_list_value_pvalue);
	return NULL;
}
/*===============================+
 * __length -- Find length of list
 *   usage: length(LIST) -> INT
 *==============================*/
PVALUE
__length (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list;
	PVALUE val = eval_and_coerce(PLIST, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to length is not a list");
		return NULL;
	}
	list = pvalue_to_list(val);
	set_pvalue_int(val, length_list(list));
	return val;
}
/*==========================+
 * __not -- Not operation
 *   usage: not(BOOL) -> BOOL
 *=========================*/
PVALUE
__not (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE val = eval_and_coerce(PBOOL, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to not is not boolean");
		return NULL;
	}
	set_pvalue(val, PBOOL, (VPTR)!pvalue_to_bool(val));
	return val;
}
/*===============================+
 * __save -- Copy string
 *   usage: save(STRING) -> STRING
 *==============================*/
PVALUE
__save (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "save");
		delete_pvalue(val);
		return NULL;
	}
	return val;
}
/*=================================+
 * __strlen -- Find length of string
 *   usage: strlen(STRING) -> INT
 *================================*/
PVALUE
__strlen (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	INT len=0;
	STRING str;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "save");
		delete_pvalue(val);
		return NULL;
	}
	str = pvalue_to_string(val);
	len = str ? strlen(str) : 0;
	set_pvalue_int(val, len);
	return val;
}
/*=============================================+
 * __concat -- Catenate strings
 *   usage: concat(STRING [, STRING]+) -> STRING
 *============================================*/
PVALUE
__concat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	INT argcnt = 0;
	STRING str;
	PVALUE val;
	ZSTR zstr = zs_new();

	while (arg) {
		val = eval_and_coerce(PSTRING, arg, stab, eflg);
		if (*eflg) {
			char argnum[8];
			sprintf(argnum, "%d", argcnt+1);
			prog_var_error(node, stab, arg, val, nonstrx, "concat", argnum);
			return NULL;
		}
		str = pvalue_to_string(val);
		zs_apps(zstr, str);
		arg = inext(arg);
		++argcnt;
		delete_pvalue(val);
	}
	val = create_pvalue_from_string(zs_str(zstr));
	zs_free(&zstr);
	return val;
}
/*=======================================+
 * __lower -- Convert string to lower case
 *   usage: lower(STRING) -> STRING
 *======================================*/
PVALUE
__lower (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	STRING str;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "lower");
		delete_pvalue(val);
		return NULL;
	}
	str = pvalue_to_string(val);
	if (str)
		str = lower(str);
	set_pvalue_string(val, str);
	return val;
}
/*=======================================+
 * __upper -- Convert string to upper case
 *   usage: upper(STRING) -> STRING
 *======================================*/
PVALUE
__upper (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	STRING str;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "upper");
		delete_pvalue(val);
		return NULL;
	}
	str = pvalue_to_string(val);
	if (str)
		str = upper(str);
	set_pvalue_string(val, str);
	return val;
}
/*=====================================+
 * __capitalize -- Capitalize string
 *   usage: capitalize(STRING) -> STRING
 *====================================*/
PVALUE
__capitalize (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	STRING str;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "capitalize");
		delete_pvalue(val);
		return NULL;
	}
	str = pvalue_to_string(val);
	if (str)
		str = capitalize(str);
	set_pvalue_string(val, str);
	return val;
}
/*=====================================+
 * __titlcase -- Titlecase string
 *   usage: capitalize(STRING) -> STRING
 * Created: 2001/12/30 (Perry Rapp)
 *====================================*/
PVALUE
__titlcase (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	STRING str;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "titlecase");
		delete_pvalue(val);
		return NULL;
	}
	str = pvalue_to_string(val);
	if (str)
		str = titlecase(str);
	set_pvalue_string(val, str);
	return val;
}
/*================================+
 * __pn -- Generate pronoun
 *   usage: pn(INDI, INT) -> STRING
 *===============================*/
static char *mpns[] = {  N_("He"),  N_("he"), N_("His"), N_("his"), N_("him") };
/* "her_" = object form (Doug hit her) (do not include underscore in translation) */
static char *fpns[] = { N_("She"), N_("she"), N_("Her"), N_("her"), N_("her_") };
PVALUE
__pn (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT typ;
	PVALUE val;
	PNODE arg = (PNODE) iargs(node);
	NODE indi = eval_indi(arg, stab, eflg, NULL);
	STRING str;
	if (*eflg || !indi) {
		*eflg = TRUE;
		prog_error(node, "1st arg to pn must be a person");
		return NULL;
	}
	val = eval_and_coerce(PINT, inext(arg), stab, eflg);
	typ = pvalue_to_int(val);
	if (*eflg || typ < 0 || typ > 4) {
		*eflg = TRUE;
		prog_error(node, "2nd arg to pn must be between 0 and 4");
		return NULL;
	}
	if (SEX(indi) == SEX_FEMALE) {
		STRING str = _(fpns[typ]);
		if (eqstr(str, "her_"))
			str = "her";
	} else {
		str = _(mpns[typ]);
	}
	set_pvalue_string(val, str);
	return val;
}
/*==================================+
 * __print -- Print to stdout window
 *   usage: print([STRING]+,) -> VOID
 *=================================*/
PVALUE
__print (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val;
	while (arg) {
		STRING str;
		val = evaluate(arg, stab, eflg);
		if (*eflg || !val || ptype(val) != PSTRING) {
			*eflg = TRUE;
			prog_error(node, "all args to print must be strings");
			return NULL;
		}
		str = pvalue_to_string(val);
		if (str) {
			uilocale();
			rpt_print(str);
			rptlocale();
		}
		delete_pvalue(val);
		arg = inext(arg);
	}
	return NULL;
}
/*=================================================+
 * __sex -- Find sex, as string M, F or U, of person
 *   usage: sex(INDI) -> STRING
 *================================================*/
PVALUE
__sex (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str = (STRING) "U";
	INT sex;
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, "the arg to sex is not a person");
		return NULL;
	}
	if (!indi) return create_pvalue_from_string(str);
	if ((sex = SEX(indi)) == SEX_MALE) str = (STRING) "M";
	else if (sex == SEX_FEMALE) str = (STRING) "F";
	return create_pvalue_from_string(str);
}
/*=================================+
 * __male -- Check if person is male
 *   usage: male(INDI) -> BOOL
 *================================*/
PVALUE
__male (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, "the arg to male is not a person");
		return NULL;
	}
	if (!indi) return create_pvalue(PBOOL, FALSE);
	return create_pvalue(PBOOL, (VPTR)(SEX(indi) == SEX_MALE));
}
/*=====================================+
 * __female -- Check if person is female
 *   usage: female(INDI) -> BOOL
 *====================================*/
PVALUE
__female (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, "the arg to female is not a person");
		return NULL;
	}
	if (!indi) return create_pvalue(PBOOL, FALSE);
	return create_pvalue(PBOOL, (VPTR)(SEX(indi) == SEX_FEMALE));
}
/*========================================+
 * __key -- Return person or family key
 *   usage: key(INDI|FAM [,BOOL]) -> STRING
 *=======================================*/
PVALUE
__key (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = evaluate(arg, stab, eflg);
	CACHEEL cel;
	BOOLEAN strip = FALSE;
	STRING key;
	if (*eflg || !val || !is_record_pvalue(val)) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val, nonrecx, "key", "1");
		return NULL;
	}
	cel = pvalue_to_cel(val); /* may return NULL */
	delete_pvalue(val);
	if (!cel) return create_pvalue_from_string("");
	if (inext(arg)) {
		val = eval_and_coerce(PBOOL, inext(arg), stab, eflg);
		if (*eflg) {
			prog_error(node, "2nd arg to key is not boolean");
			return NULL;
		}
		strip = pvalue_to_bool(val);
		delete_pvalue(val);
	}
	key = (STRING) ckey(cel);
	return create_pvalue_from_string(strip ? key + 1 : key);
}
/*==============================================+
 * __root -- Return root of cached record
 *   usage: root(INDI|FAM|EVEN|SOUR|OTHR) -> NODE
 *=============================================*/
PVALUE
__rot (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
 	PVALUE val = evaluate(iargs(node), stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_error(node, "error in the arg to root");
		return NULL;
	}
	if (!is_record_pvalue(val)) {
		*eflg = TRUE;
		prog_error(node, "the arg to root must be a record");
		return NULL;
	}
	if (!record_to_node(val)) {
		*eflg = TRUE;
		prog_error(node, "record passed to root missing from database");
		return NULL;
	}
	return val;
}
/*==============================================+
 * record_to_node -- Extract root node from record
 *  used by root & for implicit conversion
 * Created: 2002/02/16, Perry Rapp (pulled out of root)
 *=============================================*/
BOOLEAN
record_to_node (PVALUE val)
{
	RECORD rec = pvalue_to_rec(val); /* may be NULL */
	NODE gnode=0;

	if (!rec) return FALSE;

	/* pvalue_to_rec loads the record into direct cache */

	gnode = nztop(rec);
	set_pvalue_node(val, gnode); 
	ASSERT(gnode);
	return TRUE;
}
/*================================+
 * __inode -- Return root of person
 *   usage: inode(INDI) -> NODE
 *==============================*/
PVALUE
__inode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	if (*eflg || !indi) {
		*eflg = TRUE;
		prog_error(node, _(nonind1), "inode");
		return NULL;
	}
	return create_pvalue(PGNODE, (VPTR)indi);
}
/*================================+
 * __fnode -- Return root of family
 *   usage: fnode(FAM) -> NODE
 *===============================*/
PVALUE
__fnode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE fam = eval_fam(iargs(node), stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, nonfam1, "fnode");
		return NULL;
	}
	if (!fam) return create_pvalue(PGNODE, NULL);
	return create_pvalue(PGNODE, (VPTR)fam);
}
/*=============================+
 * __table -- Create table
 *   usage: table(IDENT) -> VOID
 *============================*/
PVALUE
__table (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	TABLE tab;
	PVALUE val;
	PNODE var = (PNODE) iargs(node);
	if (!iistype(var, IIDENT)) {
		*eflg = TRUE;
		prog_var_error(node, stab, var, NULL, nonvar1, "table");
		return NULL;
	}
	tab = create_table();
	val = create_pvalue(PTABLE, (VPTR)tab);

	assign_iden(stab, iident(var), val);
	return NULL;
}
/*=========================================+
 * __insert -- Add element to table
 *   usage: insert(TAB, STRING, ANY) -> VOID
 *========================================*/
PVALUE
__insert (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	BOOLEAN there;
	PNODE arg = (PNODE) iargs(node);
	PVALUE old=NULL, val=NULL;
	PVALUE valtab = eval_and_coerce(PTABLE, arg, stab, eflg);
	TABLE tab;
	STRING str;

	if (*eflg || (pvalue(valtab) == NULL)) {
        *eflg = TRUE;
		prog_var_error(node, stab, arg, valtab, nontabx, "insert", "1");
		return NULL;
	}
	tab = pvalue_to_table(valtab);

	arg = inext(arg);
	val = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg || !val || !pvalue(val)) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val, nonstrx, "insert", "2");
		return NULL;
	}
	str = strsave(pvalue_to_string(val));
	delete_pvalue(val);

	val = evaluate(inext(arg), stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_error(node, "3rd arg to insert is in error");
		return NULL;
	}

	old = valueofbool_ptr(tab, str, &there);
	if (there && old) delete_pvalue(old);
	insert_table_ptr(tab, str, val);
	if (there) stdfree(str);	/* key is already in table. free this one */
	delete_pvalue(valtab); /* finished with our copy of table */
	return NULL;
}
/*====================================+
 * prot -- protect string (replace if NULL)
 *===================================*/
STRING
prot (STRING str)
{
	return str ? str : "<NULL>";
}
/*====================================+
 * __lookup -- Look up element in table
 *   usage: lookup(TAB, STRING) -> ANY
 *===================================*/
PVALUE
__lookup (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg;
	PVALUE newv, val;
	TABLE tab;
	STRING str;

	arg = (PNODE) iargs(node);
	val = eval_and_coerce(PTABLE, arg, stab, eflg);
	if (*eflg || (pvalue(val) == NULL)) {
	    	*eflg = TRUE;
		prog_error(node, "1st arg to lookup is not a table");
		return NULL;
	}
	tab = pvalue_to_table(val);
	delete_pvalue(val);
	val = eval_and_coerce(PSTRING, inext(arg), stab, eflg);
	if (*eflg) {
		prog_error(node, nonstrx, "lookup", "2");
		return NULL;
	}
	str = pvalue_to_string(val);
	newv = valueof_ptr(tab, str);
	delete_pvalue(val);
	newv = (newv ? copy_pvalue(newv) : create_pvalue_any());
	if (prog_trace) {
		trace_out("lookup(,%s)->", prot(str));
		trace_pvalue(newv);
		trace_endl();
	}
	return newv;
}
/*====================================+
 * __trim -- Trim string if too long
 *   usage: trim(STRING, INT) -> STRING
 *===================================*/
PVALUE
__trim (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	STRING str;
	PVALUE val1, val2;
	INT len;
        val1 = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, nonstrx, "trim", "1");
		return NULL;
	}
	val2 = eval_and_coerce(PINT, inext(arg), stab, eflg);
	if (*eflg) {
		prog_error(node, nonintx, "trim", "2");
		return NULL;
	}
	str = pvalue_to_string(val1);
	len = pvalue_to_int(val2);
	set_pvalue(val2, PSTRING, (VPTR)trim(str, len));
	delete_pvalue(val1);
	return val2;
}
/*======================================+
 * __trimname -- Trim name if too long
 *   usage: trimname(INDI, INT) -> STRING
 *=====================================*/
PVALUE
__trimname (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	INT len;
	PVALUE val;
	NODE indi = eval_indi(arg, stab, eflg, (CACHEEL *) NULL);
	STRING str;
	if (*eflg) {
		prog_error(node, nonindx, "trimname", "1");
		return NULL;
	}
	if (!indi) return create_pvalue_from_string("");
	if (!(indi = NAME(indi)) || !nval(indi)) {
		if (getoptint("RequireNames", 0)) {
			*eflg = TRUE;
			prog_error(node, _("(trimname) person does not have a name"));
			return NULL;
		}
		return create_pvalue_from_string(0);
	}
	*eflg = FALSE;
	val = eval_and_coerce(PINT, inext(arg), stab, eflg);
	if (*eflg) {
		prog_error(node, nonintx, "trimname", "2");
		return NULL;
	}
	len = pvalue_to_int(val);
	str = name_string(trim_name(nval(indi), len));
	set_pvalue(val, PSTRING, (VPTR)str);
	return val;
}
/*==============================+
 * __date -- Return date of event
 *   usage: date(EVENT) -> STRING
 *=============================*/
PVALUE
__date (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE line;
	STRING str;
	PVALUE val = eval_and_coerce(PGNODE, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, nonnod1, "date");
		return NULL;
	}
	line = pvalue_to_node(val);
	str = event_to_date(line, FALSE);
	delete_pvalue(val);
	return create_pvalue_from_string(str);
}
/*=====================================================+
 * normalize_year -- Modify year before returning to report
 * historical behavior is that 0 is the return for unknown year
 *====================================================*/
static INT
normalize_year (struct dnum_s yr)
{
	if (yr.val == BAD_YEAR)
		return 0;
	else
		return yr.val;
}
/*=====================================================+
 * __extractdate -- Extract date from EVENT or DATE NODE
 *   usage: extractdate(NODE, VARB, VARB, VARB) -> VOID
 *====================================================*/
PVALUE
__extractdate (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str;
	NODE line;
	INT mod, da = 0, mo = 0, yr = 0;
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	PNODE dvar = inext(arg);
	PNODE mvar = inext(dvar);
	PNODE yvar = inext(mvar);
	GDATEVAL gdv = 0;
	if (*eflg) {
		prog_error(node, nonnodx, "extractdate", "1");
		return NULL;
	}
	line = pvalue_to_node(val);
	*eflg = TRUE;
	if (!iistype(dvar, IIDENT)) {
		prog_error(node, nonvarx, "extractdate", "2");
		return NULL;
	}
	if (!iistype(mvar, IIDENT)) {
		prog_error(node, nonvarx, "extractdate", "3");
		return NULL;
	}
	if (!iistype(yvar, IIDENT)) {
		prog_error(node, nonvarx, "extractdate", "4");
		return NULL;
	}
	if (nestr("DATE", ntag(line)))
		str = event_to_date(line, FALSE);
	else
		str = nval(line);
	delete_pvalue(val);
	gdv = extract_date(str);
	/* TODO: deal with date information */
	mod = gdv->date1.mod;
	da = gdv->date1.day.val;
	mo = gdv->date1.month.val;
	yr = normalize_year(gdv->date1.year);
	assign_iden(stab, iident(dvar), create_pvalue_from_int(da));
	assign_iden(stab, iident(mvar), create_pvalue_from_int(mo));
	assign_iden(stab, iident(yvar), create_pvalue_from_int(yr));
	free_gdateval(gdv);
	*eflg = FALSE;
	return NULL;
}
/*==================================================================+
 * __extractdatestr -- Extract date from STRING
 *   usage: extractdatestr(VARB, VARB, VARB, VARB, VARB[, STRING]) -> VOID
 *==================================================================*/
PVALUE
__extractdatestr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str = NULL, yrstr;
	INT mod, da, mo, yr;
	PVALUE val;
	PNODE date;
	PNODE modvar = (PNODE) iargs(node);
	PNODE dvar = inext(modvar);
	PNODE mvar = inext(dvar);
	PNODE yvar = inext(mvar);
	PNODE ystvar = inext(yvar);
	GDATEVAL gdv = 0;
	*eflg = TRUE;
	if (!iistype(modvar, IIDENT)) {
		prog_error(node, nonvarx, "extractdatestr", "1");
		return NULL;
	}
	if (!iistype(dvar, IIDENT)) {
		prog_error(node, nonvarx, "extractdatestr", "2");
		return NULL;
	}
	if (!iistype(mvar, IIDENT)) {
		prog_error(node, nonvarx, "extractdatestr", "3");
		return NULL;
	}
	if (!iistype(yvar, IIDENT)) {
		prog_error(node, nonvarx, "extractdatestr", "4");
		return NULL;
	}
	if (!iistype(ystvar, IIDENT)) {
		prog_error(node, nonvarx, "extractdatestr", "5");
		return NULL;
	}
	if ((date = inext(ystvar))) {
		val = evaluate(date, stab, eflg);
		if (*eflg) return NULL;
		if (ptype(val) != PSTRING) {
			*eflg = TRUE;
			prog_error(node, nonstrx, "extractdatestr", "6");
			delete_pvalue(val);
			return NULL;
		}
		str = pvalue_to_string(val);
	}
	gdv = extract_date(str);
	/* TODO: deal with date information */
	mod = gdv->date1.mod;
	da = gdv->date1.day.val;
	mo = gdv->date1.month.val;
	yr = normalize_year(gdv->date1.year);
	yrstr = gdv->date1.year.str;
	if (!yrstr) yrstr="";
	assign_iden(stab, iident(modvar), create_pvalue_from_int(mod));
	assign_iden(stab, iident(dvar), create_pvalue_from_int(da));
	assign_iden(stab, iident(mvar), create_pvalue_from_int(mo));
	assign_iden(stab, iident(yvar), create_pvalue_from_int(yr));
	assign_iden(stab, iident(ystvar), create_pvalue_from_string(yrstr));
	free_gdateval(gdv);
	*eflg = FALSE;
	return NULL;
}
/*=================================================+
 * __stddate -- Return standard date format of event
 *   usage: stddate(EVENT) -> STRING
 *      or  stddate(STRING) -> STRING
 *================================================*/
static INT daycode = 0;
static INT monthcode = 3;
static INT yearcode = 0;
static INT datecode = 0;
static INT eratimecode = 0;
static INT cmplxcode = 1;
PVALUE
__stddate (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str=0;
	PVALUE val = eval_without_coerce(iargs(node), stab, eflg);
	if (ptype(val) == PSTRING) {
		str = pvalue_to_string(val);
	} else {
		NODE evnt;
		coerce_pvalue(PGNODE, val, eflg);
		if (*eflg) {
			prog_error(node, nonnodstr1, "stddate");
			return NULL;
		}
		evnt = pvalue_to_node(val);
		str = event_to_date(evnt, FALSE);
	}
	set_pvalue(val, PSTRING, do_format_date(str,
	    daycode, monthcode, yearcode, datecode, eratimecode, FALSE));
	return val;
}
/*========================================================================+
 * __complexdate -- Return standard date format of event, including modifiers
 *   usage: complexdate(EVENT) -> STRING
 *      or  complexdate(STRING) -> STRING
 *=======================================================================*/
PVALUE
__complexdate (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str=0;
	PVALUE val = eval_without_coerce(iargs(node), stab, eflg);
	if (ptype(val) == PSTRING) {
		str = pvalue_to_string(val);
	} else {
		NODE evnt;
		coerce_pvalue(PGNODE, val, eflg);
		if (*eflg) {
			prog_error(node, nonnodstr1, "complexdate");
			return NULL;
		}
		evnt = pvalue_to_node(val);
		str = event_to_date(evnt, FALSE);
	}
	set_pvalue(val, PSTRING, do_format_date(str,
	    daycode, monthcode, yearcode, datecode, eratimecode, cmplxcode));
	return val;
}
/*===============================================+
 * __dayformat -- Set day format
 *   usage: dayformat(INT) -> NULL
 *==============================================*/
PVALUE
__dayformat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	INT value;
	PVALUE val = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, nonint1, "dayformat");
		return NULL;
	}
	value = pvalue_to_int(val);
	delete_pvalue(val);
	if (value < 0) value = 0;
	if (value > 2) value = 2;
	daycode = value;
	return NULL;
}
/*===============================================+
 * __monthformat -- Set month format
 *   usage: monthformat(INT) -> NULL
 *==============================================*/
PVALUE
__monthformat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	INT value;
	PVALUE val = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, nonint1, "monthformat");
		return NULL;
	}
	value = pvalue_to_int(val);
	delete_pvalue(val);
	if (value < 0) value = 0;
	if (value > 11) value = 8;
	monthcode = value;
	return NULL;
}
/*===============================================+
 * __yearformat -- Set month format
 *   usage: yearformat(INT) -> NULL
 * Created: 2001/12/24, Perry Rapp
 *==============================================*/
PVALUE
__yearformat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	INT value;
	PVALUE val = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, nonint1, "yearformat");
		return NULL;
	}
	value = pvalue_to_int(val);
	delete_pvalue(val);
	if (value < 0) value = 0;
	yearcode = value;
	return NULL;
}
/*=================================================+
 * __dateformat -- Set date format
 *   usage: dateformat(INT) -> NULL
 *================================================*/
PVALUE
__dateformat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	INT value;
	PVALUE val =  eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, nonint1, "dateformat");
		return NULL;
	}
	value = pvalue_to_int(val);
	delete_pvalue(val);
	if (value <  0) value = 0;
	if (value > 14) value = 14;
	datecode = value;
	return NULL;
}
/*===============================================+
 * __eraformat -- Set format for AD/BC trailer
 *   usage: eraformat(INT) -> NULL
 * Created: 2001/12/28, Perry Rapp
 *==============================================*/
PVALUE
__eraformat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	INT value;
	PVALUE val = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, nonint1, "eraformat");
		return NULL;
	}
	value = pvalue_to_int(val);
	delete_pvalue(val);
	if (value < 0) value = 0;
	eratimecode = value;
	return NULL;
}
/*===============================================+
 * __complexformat -- Set complex format
 *   usage: complexformat(INT) -> NULL
 * Created: 2001/12/24, Perry Rapp
 *==============================================*/
PVALUE
__complexformat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	INT value;
	PVALUE val = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, nonint1, "complexformat");
		return NULL;
	}
	value = pvalue_to_int(val);
	delete_pvalue(val);
	if (value < 0) value = 0;
	cmplxcode = value;
	return NULL;
}
/*===============================================+
 * __datepic -- Set custom ymd date picture string
 *   usage: datepic(STRING) -> NULL
 * Created: 2001/12/30, Perry Rapp
 *==============================================*/
PVALUE
__datepic (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	STRING str;
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, nonstrx, "datepic", "1");
		return NULL;
	}
	str = pvalue_to_string(val);
	set_date_pic(str);
	delete_pvalue(val);
	return NULL;
}
/*===============================================+
 * __complexpic -- Set custom picture string for
 *  a complex date
 *   usage: complexpic(INT, STRING) -> NULL
 * Created: 2001/12/30, Perry Rapp
 * TODO: We could add a 3rd argument giving language specifier
 *  when we are localizing
 *==============================================*/
PVALUE
__complexpic (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	INT ecmplx;
	STRING str;
	BOOLEAN ok;
	PVALUE val = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_error(node, nonintx, "complexpic", "1");
		return NULL;
	}
	ecmplx = pvalue_to_int(val);
	delete_pvalue(val);
	val = eval_and_coerce(PSTRING, arg = inext(arg), stab, eflg);
	if (*eflg) {
		prog_error(node, nonstrx, "complexpic", "2");
		return NULL;
	}
	str = pvalue_to_string(val);
	ok = set_cmplx_pic(ecmplx, str);
	delete_pvalue(val);
	if (!ok) {
		*eflg = TRUE;
		prog_error(node, badargs, "complexpic");
		return NULL;
	}
	return NULL;
}
/*==============================+
 * __year -- Return year of event
 *   usage: year(EVENT) -> STRING
 *      or  year(STRING) -> STRING
 *=============================*/
PVALUE
__year (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str=0;
	char buff[20];
	GDATEVAL gdv;
	PVALUE val = eval_without_coerce(iargs(node), stab, eflg);
	if (ptype(val) == PSTRING) {
		str = pvalue_to_string(val);
	} else {
		NODE evnt;
		coerce_pvalue(PGNODE, val, eflg);
		if (*eflg) {
			prog_error(node, nonnodstr1, "year");
			return NULL;
		}
		evnt = pvalue_to_node(val);
		str = event_to_date(evnt, FALSE);
	}
	gdv = extract_date(str);
	/* prefer year's string if it has one */
	if (gdv->date1.year.str && gdv->date1.year.str[0]) {
		str = gdv->date1.year.str;
	} else if (gdv->date1.year.val != BAD_YEAR) {
		/* no year string, so must have been a simple number */
		snprintf(buff, sizeof(buff), "%d", gdv->date1.year.val);
		str = buff;
	} else
		str = 0;
	set_pvalue(val, PSTRING, str);
	free_gdateval(gdv);
	return val;
}
/*================================+
 * __place -- Return place of event
 *   usage: place(EVENT) -> STRING
 *===============================*/
PVALUE
__place (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE evnt;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);

	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "place");
		return NULL;
	}
	evnt = pvalue_to_node(val);
	set_pvalue(val, PSTRING, (VPTR)event_to_plac(evnt, FALSE));
	return val;
}
/*============================+
 * __tag -- Return tag of node
 *   usage: tag(NODE) -> STRING
 *===========================*/
PVALUE
__tag (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE ged;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	STRING str=NULL;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "tag");
		return NULL;
	}
	ged = pvalue_to_node(val);
	if (ged)
		str=ntag(ged);
	set_pvalue(val, PSTRING, (VPTR)str);
	return val;
}
/*===============================+
 * __value -- Return value of node
 *   usage: value(NODE) -> STRING
 *==============================*/
PVALUE
__value (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE ged;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "value");
		return NULL;
	}
	ged = pvalue_to_node(val);
	if (!ged) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val, nullarg1, "value");
		return NULL;
	}
	set_pvalue(val, PSTRING, (VPTR)nval(ged));
	return val;
}
/*=============================+
 * __xref -- Return xref of node
 *   usage: xref(NODE) -> STRING
 *============================*/
PVALUE
__xref (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE ged;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "xref");
		return NULL;
	}
	ged = pvalue_to_node(val);
	if (!ged) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val, nullarg1, "xref");
		return NULL;
	}
	set_pvalue(val, PSTRING, (VPTR)nxref(ged));
	return val;
}
/*===============================+
 * __child -- Return child of node
 *   usage: child(NODE) -> NODE
 *==============================*/
PVALUE
__child (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE ged;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "child");
		return NULL;
	}
	ged = pvalue_to_node(val);
	if (!ged) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val, nullarg1, "child");
		return NULL;
	}
	set_pvalue_node(val, nchild(ged));
	return val;
}
/*=================================+
 * __parent -- Return parent of node
 *   usage: parent(NODE) -> NODE
 *================================*/
PVALUE
__parent (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE ged;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "parent");
		return NULL;
	}
	ged = pvalue_to_node(val);
	if (!ged) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val, nullarg1, "parent");
		return NULL;
	}
	set_pvalue(val, PGNODE, nparent(ged));
	return val;
}
/*========================================+
 * __sibling -- Return next sibling of node
 *   usage: sibling(NODE) -> NODE
 *=======================================*/
PVALUE
__sibling (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE ged;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "sibling");
		return NULL;
	}
	ged = pvalue_to_node(val);
	if (!ged) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val, nullarg1, "sibling");
		return NULL;
	}
	set_pvalue(val, PGNODE, nsibling(ged));
	return val;
}
/*===============================+
 * __level -- Return level of node
 *   usage: level(NODE) -> INT
 *==============================*/
PVALUE
__level (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE ged;
	INT lev = -1;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "level");
		return NULL;
	}
	ged = pvalue_to_node(val);
	while (ged) {
		lev++;
		ged = nparent(ged);
	}
	set_pvalue_int(val, lev);
	return val;
}
/*=================================+
 * __copyfile -- Copy file to output
 *   usage: copyfile(STRING) -> VOID
 *================================*/
PVALUE
__copyfile (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	FILE *cfp;
	STRING fname;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	char buffer[1024];
	STRING programsdir = getoptstr("LLPROGRAMS", ".");
	if (*eflg)  {
		prog_error(node, nonstr1, "copyfile");
		return NULL;
	}
	fname = pvalue_to_string(val);
	if (!(cfp = fopenpath(fname, LLREADTEXT, programsdir
		, (STRING)NULL, uu8, (STRING *)NULL))) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val, nonfname1, "copyfile");
		return NULL;
	}
	delete_pvalue(val);
	while (fgets(buffer, sizeof(buffer), cfp)) {
		poutput(buffer, eflg);
		if (*eflg)
			return NULL;
	}
	fclose(cfp);
	return NULL;
}
/*========================+
 * __nl -- Newline function
 *   usage: nl() -> STRING
 *=======================*/
PVALUE
__nl (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_string("\n");
}
/*=========================+
 * __space -- Space function
 *   usage: sp() -> STRING
 *========================*/
PVALUE
__space (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_string(" ");
}
/*=============================+
 * __qt -- Double quote function
 *   usage: qt() -> STRING
 *============================*/
PVALUE
__qt (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_string("\"");
}
/*=============================+
 * __indi -- Convert key to INDI
 *   usage: indi(STRING) -> INDI
 *============================*/
PVALUE
__indi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str;
	char scratch[200], *p, *q = scratch;
	INT c;
	PVALUE val = eval_and_coerce(PSTRING, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, nonstr1, "indi");
		return NULL;
	}
	p = str = pvalue_to_string(val);
	while ((c = (uchar)*p++) && chartype(c) != DIGIT)
		;
	if (c == 0) {
		delete_pvalue(val);
		return NULL;
	}
	*q++ = 'I';
	*q++ = c;
	while (chartype(c = (uchar)*p++) == DIGIT)
		*q++ = c;
	*q = 0;
	delete_pvalue(val);
	if (strlen(scratch) == 1) return NULL;
/*
 *	rawrec = (STRING) retrieve_raw_record(scratch, &len);
 *	if (rawrec && len > 6)
 *		val = create_pvalue(PINDI, (VPTR)key_to_indi_cacheel(scratch));
 *	else
 *		val = create_pvalue(PINDI, NULL);
 *	if (rawrec) stdfree(rawrec);
 */
	val = create_pvalue_from_indi_key(scratch);
/* 	val = create_pvalue(PINDI, (VPTR)qkey_to_indi_cacheel(scratch)); */
	return val;
}
/*===========================+
 * __fam -- Convert key to FAM
 *   usage: fam(STRING) -> FAM
 *==========================*/
PVALUE
__fam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str, rawrec;
	char scratch[200], *p, *q = scratch;
	INT c, len;
	PVALUE val = eval_and_coerce(PSTRING, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, nonstr1, "fam");
		return NULL;
	}
	p = str = pvalue_to_string(val);
	while ((c = (uchar)*p++) && chartype(c) != DIGIT)
		;
	if (c == 0) {
		delete_pvalue(val);
		return NULL;
	}
	*q++ = 'F';
	*q++ = c;
	while (chartype(c = (uchar)*p++) == DIGIT)
		*q++ = c;
	*q = 0;
	delete_pvalue(val);
	if (strlen(scratch) == 1) return NULL;
	/* TODO - use gedlib layer code as in __indi above */
	rawrec = retrieve_raw_record(scratch, &len);
	if (rawrec && len > 6)
		val = create_pvalue(PFAM, key_to_fam_cacheel(scratch));
	else
		val = create_pvalue(PFAM, NULL);
	if (rawrec) stdfree(rawrec);
	return val;
}
/*=======================================+
 * eval_indi -- Evaluate person expression
 *======================================*/
NODE
eval_indi (PNODE expr, SYMTAB stab, BOOLEAN *eflg, CACHEEL *pcel)
{
	NODE indi;
	CACHEEL cel;
	PVALUE val = eval_and_coerce(PINDI, expr, stab, eflg);

	if (*eflg || !val) {
		if (val) {
			delete_pvalue(val);
			val=NULL;
		}
		return NULL;
	}
	cel = pvalue_to_cel(val);
	delete_pvalue(val);
	if (!cel) return NULL;
	indi = cnode(cel);
	if (nestr("INDI", ntag(indi))) {
		*eflg = TRUE;
		return NULL;
	}
	if (pcel) *pcel = cel;
	return indi;
}
/*======================================+
 * eval_fam -- Evaluate family expression
 *=====================================*/
NODE
eval_fam (PNODE expr, SYMTAB stab, BOOLEAN *eflg, CACHEEL *pcel)
{
	NODE fam;
	CACHEEL cel;
	PVALUE val = eval_and_coerce(PFAM, expr, stab, eflg);
	if (*eflg || !val) return NULL;
	cel = pvalue_to_cel(val);
	delete_pvalue(val);
	if (!cel) return NULL;
	fam = cnode(cel);
	if (nestr("FAM", ntag(fam))) {
		*eflg = TRUE;
		return NULL;
	}
	if (pcel) *pcel = cel;
	return fam;
}
/*=================================================+
 * __free -- free up data associated with a variable 
 *   usage: free(IDEN]) --> VOID
 *=======================================*/
PVALUE
__free (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
/*	extern LIST keysets;*/
	PNODE arg = (PNODE) iargs(node);
	BOOLEAN there;
	PVALUE val;
	if (!iistype(arg, IIDENT)) {
		prog_error(node, "arg to free must be a variable");
		*eflg = TRUE;
		return NULL;
	}
	val = symtab_valueofbool(stab, iident(arg), &there);
	if (!there) {
	    val = symtab_valueofbool(globtab, iident(arg), &there);
	}
	if (there && val) {
		set_pvalue(val, PANY, NULL);
	}
	return NULL;
}
