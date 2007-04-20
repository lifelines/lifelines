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

#include "sys_inc.h"
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interpi.h"
#include "indiseq.h"
#include "rptui.h"
#include "feedback.h"
#include "lloptions.h"
#include "date.h"
#include "zstr.h"
#include "codesets.h"
#include "arch.h"
#include "pvalue.h"

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
static ZSTR decode(STRING str, INT * offset);
static FLOAT julianday(GDATEVAL gdv);
static INT normalize_year(INT yr);

/*********************************************
 * local variables
 *********************************************/

static struct tag_rfmt rpt_long_rfmt; /* short form report format */
static struct tag_rfmt rpt_shrt_rfmt; /* long form report format */

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*========================================+
 * llrpt_getint -- Have user provide integer
 * usage: getint(IDEN [,STRING]) --> VOID
 *=======================================*/
PVALUE
llrpt_getint (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	if (!rptui_ask_for_int(msg, &num)) {
		*eflg = TRUE;
		return NULL;
	}
	assign_iden(stab, iident(arg), create_pvalue_from_int(num));
	delete_pvalue(val);
	return NULL;
}
/*========================================+
 * llrpt_getstr -- Have user provide string
 * usage: getstr(IDEN [,STRING]) --> VOID
 *=======================================*/
PVALUE
llrpt_getstr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_getindi -- Have user identify person
 * usage: getindi(IDEN [,STRING]) --> VOID
 *========================================*/
PVALUE
llrpt_getindi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	key = rptui_ask_for_indi_key(msg, DOASK1);
	if (key) {
		assign_iden(stab, iident(arg)
			, create_pvalue_from_indi_key(key));
	}
	if (val) delete_pvalue(val);
	return NULL;
}
/*=====================================+
 * llrpt_getfam -- Have user identify family
 * usage: getfam(IDEN) --> VOID
 *====================================*/
PVALUE
llrpt_getfam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE fam;
	if (!iistype(arg, IIDENT)) {
		prog_var_error(node, stab, arg, NULL, nonvar1, "getfam");
		*eflg = TRUE;
		return NULL;
	}
	assign_iden(stab, iident(arg), NULL);
	fam = nztop(rptui_ask_for_fam(_("Enter a spouse from family."),
	    _("Enter a sibling from family.")));
	assign_iden(stab, iident(arg), create_pvalue_from_fam(fam));
	return NULL;
}
/*=================================================+
 * llrpt_getindiset -- Have user identify set of persons
 * usage: getindiset(IDEN [,STRING]) --> VOID
 * This introduces both null value indiseqs and null
 * indiseqs into reports so report code must handle them
 *================================================*/
PVALUE
llrpt_getindiset (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	seq = rptui_ask_for_indi_list(msg, TRUE);
	if (seq)
		namesort_indiseq(seq); /* in case uilocale != rptlocale */
	delete_pvalue(val);
	assign_iden(stab, iident(arg), create_pvalue_from_seq(seq));
	return NULL;
}
/*==================================+
 * llrpt_gettext -- translate to ambient locale
 * usage: gettext(STRING) --> STRING
 *=================================*/
PVALUE
llrpt_gettext (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str=0;
#ifdef ENABLE_NLS
	STRING str2=0,textdomain=0,localepath=0;
#endif /* ENABLE_NLS */
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	PVALUE newval=0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "gettext");
		return NULL;
	}
	str = pvalue_to_string(val);
#ifdef ENABLE_NLS
	textdomain = zs_str(irptinfo(node)->textdomain);
	localepath = zs_str(irptinfo(node)->localepath);
	ll_bindtextdomain(textdomain, localepath);
	set_gettext_codeset(textdomain, "work_around_set_gettext_codeset_cache");
	set_gettext_codeset(textdomain, int_codeset);
	str2 = irptinfo(node)->fullpath;
	str2 = _(str);
	ll_bindtextdomain(PACKAGE, localepath);
	str = str2;
#endif
	newval = create_pvalue_from_string(str);
	delete_pvalue(val);
	return newval;
}
/*==================================+
 * llrpt_gettoday -- Create today's event
 * usage: gettoday() --> EVENT
 *=================================*/
PVALUE
llrpt_gettoday (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_name -- Find person's name
 * usage: name(INDI[,BOOL]) -> STRING
 *===================================*/
PVALUE
llrpt_name (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PNODE arg2;
	NODE name, indi = eval_indi(arg, stab, eflg, NULL);
	SURCAPTYPE captype = DOSURCAP;
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
		captype = pvalue_to_bool(val) ? DOSURCAP : NOSURCAP;
		delete_pvalue(val);
	}
	if (!(name = find_tag(nchild(indi), "NAME"))) {
		if (getlloptint("RequireNames", 0)) {
			*eflg = TRUE;
			prog_var_error(node, stab, arg, NULL, _("name: person does not have a name"));
			return NULL;
		}
		return create_pvalue_from_string(0);
	}
	outname = manip_name(nval(name), captype, REGORDER, 68);
	return create_pvalue_from_string(outname);
}
/*==================================================+
 * llrpt_fullname -- Process person's name
 * usage: fullname(INDI, BOOL, BOOL, INT) -> STRING
 *=================================================*/
PVALUE
llrpt_fullname (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE name, indi;
	PVALUE val;
	SURCAPTYPE caps = DOSURCAP;
	SURORDER regorder = REGORDER;
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
	caps = pvalue_to_bool(val) ? DOSURCAP : NOSURCAP;
	delete_pvalue(val);
	val = eval_and_coerce(PBOOL, arg = inext(arg), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonboox, "fullname", "3");
		delete_pvalue(val);
		return NULL;
	}
	regorder = pvalue_to_bool(val) ? REGORDER : SURFIRST;
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
		if (getlloptint("RequireNames", 0)) {
			*eflg = TRUE;
			prog_var_error(node, stab, NULL, NULL, _("fullname: person does not have a name"));
			return NULL;
		}
		return create_pvalue_from_string(0);
	}
	outname = manip_name(nval(name), caps, regorder, len);
	return create_pvalue_from_string(outname);
}
/*==================================+
 * llrpt_surname -- Find person's surname using new getasurname() routine.
 * usage: surname(INDI) -> STRING
 *=================================*/
PVALUE
llrpt_surname (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE name, indi = eval_indi(arg, stab, eflg, NULL);
	CNSTRING str;

	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonvar1, "surname");
		return NULL;
	}
	if (!indi) return create_pvalue_from_string("");
	if (!(name = NAME(indi)) || !nval(name)) {
		if (getlloptint("RequireNames", 0)) {
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
 * llrpt_soundex -- SOUNDEX function on persons
 * usage: soundex(INDI) -> STRING
 *=======================================*/
PVALUE
llrpt_soundex (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE name, indi = eval_indi(arg, stab, eflg, NULL);
	if (*eflg || !indi) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, NULL, nonvar1, "soundex");
		return NULL;
	}
	if (!(name = NAME(indi)) || !nval(name)) {
		if (getlloptint("RequireNames", 0)) {
			*eflg = TRUE;
			prog_var_error(node, stab, arg, NULL, _("soundex: person does not have a name"));
			return NULL;
		}
		return create_pvalue_from_string(0);
	}
	return create_pvalue_from_string(trad_soundex(getsxsurname(nval(name))));
}
/*===========================================+
 * llrpt_strsoundex -- SOUNDEX function on strings
 * usage: strsoundex(STRING) -> STRING
 *==========================================*/
PVALUE
llrpt_strsoundex (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE newval, val = NULL;
	CNSTRING str;
	val = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "strsoundex");
		delete_pvalue(val);
		return NULL;
	}
	str = trad_soundex(pvalue_to_string(val));
	newval = create_pvalue_from_string(str);
	delete_pvalue(val);
	return newval;
}
/*===========================================+
 * llrpt_bytecode -- Input string with escape codes
 *  and optionally specified codeset
 *  eg, bytecode("I$C3$B1$C3$A1rritu", "UTF-8")
 * usage: bytecode(STRING, [STRING]) -> STRING
 *==========================================*/
PVALUE
llrpt_bytecode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	PVALUE newval=0;
	STRING codeset=0;
	INT offset;
	ZSTR zstr=0;
	STRING str;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstrx, "bytecode", "1");
		goto bytecode_exit;
	}
	if (arg) {
		PVALUE val2 = eval_and_coerce(PSTRING, arg = inext(arg), stab, eflg);
		if (*eflg) {
			prog_var_error(node, stab, arg, NULL, nonstrx, "bytecode", "2");
			goto bytecode_exit;
		}
		codeset = strsave(pvalue_to_string(val2));
		delete_pvalue(val2);
	} else {
		codeset = strsave(report_codeset_in);
	}
	str = pvalue_to_string(val);
	zstr = decode(str, &offset);
	if (offset >= 0) {
		prog_var_error(node, stab, arg, val
			, _("Bad escape code at offset %d in bytecode string <%s>")
			, offset+1, str);
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
 * llrpt_convertcode -- Convert string to another codeset
 *  eg, convertcode(str, "UTF-8//html")
 *  or for use in self-tests, convertcode(bytecode("$C3$B1$C3$A1"), "UTF-8", "ISO-8859-1")
 *  (which should come out "ñá"
 * usage: convertcode(STRING, STRING, [STRING]) -> STRING
 *==========================================*/
PVALUE
llrpt_convertcode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_setlocale -- Set current locale
 * usage: setlocale(STRING) -> STRING
 *==========================================*/
PVALUE
llrpt_setlocale (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE newval, val = eval_and_coerce(PSTRING, arg, stab, eflg);
	STRING str;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonstr1, "setlocale");
		delete_pvalue(val);
		return NULL;
	}
	str = pvalue_to_string(val);
	str = rpt_setlocale(str);
	str = str ? str : "C";
	newval = create_pvalue_from_string(str);
	delete_pvalue(val);
	return newval;
}
/*===============================+
 * llrpt_givens -- Find given names
 * usage: givens(INDI) -> STRING
 *==============================*/
PVALUE
llrpt_givens (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	NODE name, indi = eval_indi(arg, stab, eflg, NULL);
	CNSTRING str;
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, _(nonindx), "givens", "1");
		return NULL;
	}
	if (!indi) return create_pvalue_from_string("");
	if (!(name = NAME(indi)) || !nval(name)) {
		if (getlloptint("RequireNames", 0)) {
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
 * llrpt_set -- Assignment operation
 * usage: set(IDEN, ANY) -> VOID
 *==============================*/
PVALUE
llrpt_set (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
		if (!(*eflg) && !val) {
			*eflg = TRUE;
			prog_var_error(node, stab, argexpr, val, _("set(%s, <Null>) is invalid"), iident(argvar));
		} else {
			*eflg = TRUE;
			prog_var_error(node, stab, argexpr, val, badargx, "set", "2");
		}
		return NULL;
	}
	assign_iden(stab, iident(argvar), val);
	return NULL;
}
/*===========================================+
 * llrpt_setdate -- Date assignment operation
 * usage: setdate(IDEN, STRING) -> VOID
 * creation: Patrick Texier 2005/05/22
 * Added to cvs: 2006/06/10
 *===========================================*/
PVALUE
llrpt_setdate (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE argvar = iargs(node);
	PNODE arg2;
	STRING str = 0;
	PVALUE val = NULL;
	NODE prnt, chil;

	if (!iistype(argvar, IIDENT)) {
		prog_var_error(node, stab, argvar, NULL, nonvarx, "setdate", "1");
		*eflg = TRUE;
		return NULL;
	}
	if ((arg2 = inext(argvar)) != NULL) {
		val = eval_and_coerce(PSTRING, arg2, stab, eflg);
		if (*eflg) {
			prog_var_error(node, stab, arg2, val, nonstrx, "setdate", "2");
			delete_pvalue(val);
			return NULL;
		}
		str = pvalue_to_string(val);
	}
	/* Create an EVEN node with subordinate DATE node */
	prnt = create_temp_node(NULL, "EVEN", NULL, NULL);
	chil = create_temp_node(NULL, "DATE", str, prnt);
	nchild(prnt) = chil;
	/* Assign new EVEN node to new pvalue, and assign that to specified identifier */
	assign_iden(stab, iident(argvar), create_pvalue_from_node(prnt));
	return NULL;
}
/*===============================+
 * llrpt_dup -- Dup operation
 * usage: dup(LIST) -> LIST
 *==============================*/
PVALUE
llrpt_dup (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE argexpr = iargs(node);
	LIST list, newlist;
	PVALUE val, newval;
	INT i;

	val = evaluate(argexpr, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_var_error(node, stab, argexpr, val, badargx, "dup", "1");
		return NULL;
	}
	/* traverse and copy */
	list = pvalue_to_list(val);
	delete_pvalue(val);
	newlist = create_list3(delete_vptr_pvalue);
	for (i=0; i<length_list(list); i++) {
		newval = (PVALUE) get_list_element(list, i+1, NULL);
		enqueue_list(newlist, copy_pvalue(newval));
	}
	/* assign new list */
	newval = create_pvalue_from_list(newlist);
	release_list(newlist); /* release our ref to newlist */
	return newval;
}
/*=========================================+
 * llrpt_husband -- Find first husband of family
 * usage: husband(FAM) -> INDI
 *========================================*/
PVALUE
llrpt_husband (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_wife -- Find first wife of family
 * usage: wife(FAM) -> INDI
 *==================================*/
PVALUE
llrpt_wife (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_firstchild -- Find first child of family
 * usage: firstchild(FAM) -> INDI
 *=========================================*/
PVALUE
llrpt_firstchild (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_lastchild -- Find last child of family
 * usage: lastchild(FAM) -> INDI
 *=======================================*/
PVALUE
llrpt_lastchild (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_marr -- Find marriage of family
 * usage: marriage(FAM) -> EVENT
 *================================*/
PVALUE
llrpt_marr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_birt -- Find first birth event of person
 * usage: birth(INDI) -> EVENT
 *=========================================*/
PVALUE
llrpt_birt (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_deat -- Find first death event of person
 * usage: death(INDI) -> EVENT
 *=========================================*/
PVALUE
llrpt_deat (PNODE node, SYMTAB stab, BOOLEAN  *eflg)
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
 * llrpt_bapt -- Find first baptism event of person
 * usage: baptism(INDI) -> EVENT
 *===========================================*/
PVALUE
llrpt_bapt (PNODE node, SYMTAB stab, BOOLEAN  *eflg)
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
 * llrpt_buri -- Find first burial event of person
 * usage: burial(INDI) -> EVENT
 *==========================================*/
PVALUE
llrpt_buri (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_titl -- Find first title of person
 * usage: title(INDI) -> STRING
 *===================================*/
PVALUE
llrpt_titl (PNODE node, SYMTAB stab, BOOLEAN  *eflg)
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
 * llrpt_long -- Return long form of event
 * usage: long(EVENT) -> STRING
 *==================================*/
PVALUE
llrpt_long (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_short -- Return short form of event
 * usage: short(EVENT) -> STRING
 *====================================*/
PVALUE
llrpt_short (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_fath -- Find father of person
 * usage: father(INDI) -> INDI
 *==============================*/
PVALUE
llrpt_fath (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE indival=0;
	NODE indi = eval_indi2(arg, stab, eflg, NULL, &indival);
	NODE fath = NULL;
	if (*eflg) {
		prog_var_error(node, stab, arg, indival, _(nonind1), "father");
		delete_pvalue(indival);
		return NULL;
	}
	delete_pvalue(indival);
	if (indi)
		fath = indi_to_fath(indi);
	return create_pvalue_from_indi(fath);
}
/*===============================+
 * llrpt_moth -- Find mother of person
 * usage: mother(INDI) -> INDI
 *==============================*/
PVALUE
llrpt_moth (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE indival=0;
	NODE indi = eval_indi2(arg, stab, eflg, NULL, &indival);
	if (*eflg) {
		prog_var_error(node, stab, arg, indival, _(nonind1), "mother");
		return NULL;
	}
	if (!indi) return create_pvalue_from_indi(NULL);
	return create_pvalue_from_indi(indi_to_moth(indi));
}
/*===========================================+
 * llrpt_parents -- Find parents' family of person
 * usage: parents(INDI) -> FAM
 *==========================================*/
PVALUE
llrpt_parents (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE indival=0;
	NODE indi = eval_indi2(arg, stab, eflg, NULL, &indival);
	if (*eflg) {
		prog_var_error(node, stab, arg, indival, _(nonind1), "parents");
		return NULL;
	}
	if (!indi) return create_pvalue_from_fam(NULL);
	return create_pvalue_from_fam(indi_to_famc(indi));
}
/*==========================================+
 * llrpt_nextsib -- Find person's younger sibling
 * usage: nextsib(INDI) -> INDI
 *=========================================*/
PVALUE
llrpt_nextsib (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE indival=0;
	NODE indi = eval_indi2(arg, stab, eflg, NULL, &indival);
	if (*eflg) {
		prog_var_error(node, stab, arg, indival, _(nonind1), "nextsib");
		return NULL;
	}
	if (!indi) return create_pvalue_from_indi(NULL);
	return create_pvalue_from_indi(indi_to_next_sib_old(indi));
}
/*========================================+
 * llrpt_prevsib -- Find person's older sibling
 * usage: prevsib(INDI) -> INDI
 *=======================================*/
PVALUE
llrpt_prevsib (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = iargs(node);
	PVALUE indival=0;
	NODE indi = eval_indi2(arg, stab, eflg, NULL, &indival);
	if (*eflg) {
		prog_var_error(node, stab, arg, indival, _(nonind1), "prevsib");
		return NULL;
	}
	if (!indi) return create_pvalue_from_indi(NULL);
	return create_pvalue_from_indi(indi_to_prev_sib_old(indi));
}
/*========================================+
 * llrpt_d -- Return cardinal integer as string
 * usage: d(INT) -> STRING
 *=======================================*/
PVALUE
llrpt_d (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	static char scratch[20] = "";
	PNODE arg = iargs(node);
	PVALUE val;
	INT i;
	val = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonint1, "d", "1");
		return NULL;
	}
	i = pvalue_to_int(val);
	sprintf(scratch, "%ld", i);
	set_pvalue_string(val, scratch);
	return val;
}
/*=============================================+
 * llrpt_f -- Return floating point number as string
 * usage: f(FLOAT[,INT]) -> STRING
 *============================================*/
PVALUE
llrpt_f (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	char scratch[20];
	char format[10];
	INT prec = 2;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PFLOAT, arg, stab, eflg);
	float fval;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonflox, "f", "1");
		return NULL;
	}
	fval = pvalue_to_float(val);
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
	sprintf(format, "%%.%ldf", prec);

	sprintf(scratch, format, fval);
	set_pvalue_string(val, scratch);
	return val;
}
/*==========================================+
 * llrpt_alpha -- Convert small integer to letter
 * usage: alpha(INT) -> STRING
 *=========================================*/
PVALUE
llrpt_alpha (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	static char scratch[2];
	INT i;
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonint1, "alpha");
		return NULL;
	}
	i = pvalue_to_int(val);
	delete_pvalue(val);
	if (i < 1 || i > 26)
		sprintf(scratch, "XX");
	else
		sprintf(scratch, "%c", 'a' + i - 1);
	return create_pvalue_from_string(scratch);
}
/*================================================+
 * llrpt_ord -- Convert small integer to ordinal string
 * usage: ord(INT) -> STRING
 *===============================================*/
static char *ordinals[] = {
	N_("first"), N_("second"), N_("third"), N_("fourth"), N_("fifth"),
	N_("sixth"), N_("seventh"), N_("eighth"), N_("ninth"), N_("tenth"),
	N_("eleventh"), N_("twelfth")
};
PVALUE
llrpt_ord (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	static char scratch[12];
	INT i;
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonint1, "ord");
		return NULL;
	}
	i = pvalue_to_int(val);
	delete_pvalue(val);
	if (*eflg || i < 1) return NULL;
	if (i > 12)
		sprintf(scratch, _("%ldth"), i);
	else
		sprintf(scratch, _(ordinals[i - 1]));
	return create_pvalue_from_string(scratch);
}
/*==================================================+
 * llrpt_card -- Convert small integer to cardinal string
 * usage: card(INT) -> STRING
 *=================================================*/
static char *cardinals[] = {
	N_("zero"), N_("one"), N_("two"), N_("three"), N_("four"), N_("five"),
	N_("six"), N_("seven"), N_("eight"), N_("nine"), N_("ten"),
	N_("eleven"), N_("twelve")
};
PVALUE
llrpt_card (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	static char scratch[8];
	INT i;
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonint1, "card");
		return NULL;
	}
	i = pvalue_to_int(val);
	delete_pvalue(val);
	if (i < 0 || i > 12)
		sprintf(scratch, "%ld", i);
	else
		sprintf(scratch, _(cardinals[i]));
	return create_pvalue_from_string(scratch);
}
/*==========================================+
 * llrpt_roman -- Convert integer to Roman numeral
 * usage: roman(INT) -> STRING
 * The roman system only expressed positive numbers (>0).
 * Numbers larger than 3000 were expressed by adding a bar
 * above a symbol to indicate multiply by 1000.  This usage 
 * no longer current, as the largest numbers usually expressed
 * are dates.  So this code handles 1 thru 3999.
 *=========================================*/
static char *rodigits[] = {
	"", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix"
};
static char *rotens[] = {
	"", "x", "xx", "xxx", "xl", "l", "lx", "lxx", "lxxx", "xc"
};
static char *rohuns[] = {
	"", "c", "cc", "ccc", "cd", "d", "dc", "dcc", "dccc", "cm"
};
static char *rothou[] = {
	"", "m", "mm", "mmm"
};
PVALUE
llrpt_roman (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	static char scratch[20];
	INT i;
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonint1, "roman");
		return NULL;
	}
	i = pvalue_to_int(val);
	delete_pvalue(val);
	if (i < 1 || i > 3999)
		sprintf(scratch, "%ld", i);
	else {
		int t;
		int m = i/1000;
		i =  i%1000;
		t = i/100;
		i = i%100;

		sprintf(scratch, "%s%s%s%s", rothou[m], rohuns[t],
		                             rotens[i/10], rodigits[i%10]);
	}
	return create_pvalue_from_string(scratch);
}
/*================================================+
 * llrpt_nchildren -- Find number of children in family
 * usage: nchildren(FAM) -> INT
 *===============================================*/
PVALUE
llrpt_nchildren (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_nfamilies -- Find number of families person is in
 * usage: nfamilies(INDI) -> INT
 *==================================================*/
PVALUE
llrpt_nfamilies (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_nspouses -- Find number of spouses person has
 * usage: nspouses(INDI) -> INT
 *==============================================*/
PVALUE
llrpt_nspouses (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT nspouses=0, nactual=0;
	PNODE arg = (PNODE) iargs(node);
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	if (*eflg) {
		prog_var_error(node, stab, arg, NULL, nonind1, "nspouses");
		return NULL;
	}
	if (!indi) return create_pvalue_from_int(0);
	FORSPOUSES(indi,spouse,fam,nspouses)
		++nactual;
	ENDSPOUSES
	/* nspouses is number of pointers, nactual is number of valid pointers */
	return create_pvalue_from_int(nactual);
}
/*=============================+
 * llrpt_eq -- Equal operation
 * usage: eq(ANY, ANY) -> BOOL
 *============================*/
PVALUE
llrpt_eq (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_ne -- Not equal operation
 * usage: ne(ANY, ANY) -> BOOL
 *============================*/
PVALUE
llrpt_ne (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_le -- Less or equal operation
 * usage: le(ANY, ANY) -> BOOL
 *==============================*/
PVALUE
llrpt_le (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_ge -- Greater or equal operation
 * usage: ge(ANY, ANY) -> BOOL
 *=================================*/
PVALUE
llrpt_ge (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_lt -- Less than operation
 * usage: lt(ANY,ANY) -> BOOL
 *===========================*/
PVALUE
llrpt_lt (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_gt -- Greater than operation
 * usage: gt(ANY, ANY) -> BOOL
 *=============================*/
PVALUE
llrpt_gt (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_and -- And operation
 * usage: and(ANY [,ANY]+) -> BOOL
 *================================*/
PVALUE
llrpt_and (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	BOOLEAN rc = TRUE; /* result of function */
	PVALUE val2, val1 = eval_and_coerce(PBOOL, arg, stab, eflg);
	INT argix=1; /* count arguments for error message */
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, nonboox, "and", "1");
		return NULL;
	}
	rc = rc && pvalue_to_bool(val1);
	delete_pvalue(val1);
	while ((arg = inext(arg))) {
		++argix;
		if (rc) {
			val2 = eval_and_coerce(PBOOL, arg, stab, eflg);
			if (*eflg) {
				char numstr[33];
				snprintf(numstr, sizeof(numstr), "%ld", argix);
				prog_var_error(node, stab, arg, val2, nonboox, "and", numstr);
				return NULL;
			}
			rc = rc && pvalue_to_bool(val2);
			delete_pvalue(val2);
		}
	}
	return create_pvalue_from_bool(rc);
}
/*================================+
 * llrpt_or -- Or operation
 * usage: or(ANY [,ANY]+) -> BOOL
 *===============================*/
PVALUE
llrpt_or (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	BOOLEAN rc = FALSE; /* result of function */
	PVALUE val2, val1 = eval_and_coerce(PBOOL, arg, stab, eflg);
	INT argix=1; /* count arguments for error message */
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, nonboox, "or", "1");
		return NULL;
	}
	rc = rc || pvalue_to_bool(val1);
	delete_pvalue(val1);
	while ((arg = inext(arg))) {
		++argix;
		if (!rc) {
			val2 = eval_and_coerce(PBOOL, arg, stab, eflg);
			if (*eflg) {
				char numstr[33];
				snprintf(numstr, sizeof(numstr), "%ld", argix);
				prog_var_error(node, stab, arg, val2, nonboox, "or", numstr);
				return NULL;
			}
			rc = rc || pvalue_to_bool(val2);
			delete_pvalue(val2);
		}
	}
	return create_pvalue_from_bool(rc);
}
/*================================+
 * llrpt_add -- Add operation
 * usage: add(INT [,INT]+) -> INT
 *===============================*/
PVALUE
llrpt_add (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val2, val1 = evaluate(arg, stab, eflg);
	ZSTR zerr=0;
	INT argix=1; /* count arguments for error message */
	if (*eflg) {
		prog_var_error(node, stab, arg, val1, badargx, "add", "1");
		return NULL;
	}
	while ((arg = inext(arg))) {
		++argix;
		val2 = evaluate(arg, stab, eflg);
		if (*eflg) {
			char numstr[33];
			snprintf(numstr, sizeof(numstr), "%ld", argix);
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
 * llrpt_sub -- Subtract operation
 * usage: sub(INT, INT) -> INT
 *============================*/
PVALUE
llrpt_sub (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_mul -- Multiply operation
 * usage: mul(INT [,INT]+) -> INT
 *===============================*/
PVALUE
llrpt_mul (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
			snprintf(numstr, sizeof(numstr), "%ld", iarg);
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
 * llrpt_div -- Divide operation
 * usage: div(INT, INT) -> INT
 *============================*/
PVALUE
llrpt_div (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_mod -- Modulus operation
 * usage: mod(INT, INT) -> INT
 *============================*/
PVALUE
llrpt_mod (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_exp -- Exponentiation operation
 * usage: exp(INT, INT) -> INT
 *================================*/
PVALUE
llrpt_exp (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_neg -- Negation operation
 * usage: neg(INT) -> INT
 *==========================*/
PVALUE
llrpt_neg (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_incr -- Increment variable
 * usage: incr(VARB [, number]) -> VOID
 *==========================*/
PVALUE
llrpt_incr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE vararg = (PNODE) iargs(node);
	PNODE arg2=0;
	PVALUE val=0;
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

	if ((arg2 = inext(vararg))) {
		PVALUE val2 = evaluate(arg2, stab, eflg);
		if (*eflg) {
			prog_var_error(node, stab, arg2, val2, badargx, "incr", "2");
			return NULL;
		}
		add_pvalues(val, val2, eflg, &zerr); /* adds into val */
	} else {
		incr_pvalue(val, eflg, &zerr);
	}
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	assign_iden(stab, iident(vararg), val);
	return NULL;
}
/*============================+
 * llrpt_decr -- Decrement variable
 * usage: decr(VARB [, number]) -> VOID
 *===========================*/
PVALUE
llrpt_decr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE vararg = (PNODE) iargs(node);
	PNODE arg2=0;
	PVALUE val=0;
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
	if ((arg2 = inext(vararg))) {
		PVALUE val2 = evaluate(arg2, stab, eflg);
		if (*eflg) {
			prog_var_error(node, stab, arg2, val2, badargx, "decr", "2");
			return NULL;
		}
		sub_pvalues(val, val2, eflg, &zerr); /* subtracts into val */
	} else {
		decr_pvalue(val, eflg, &zerr);
	}
	if (*eflg) {
		prog_error(node, zs_str(zerr));
		zs_free(&zerr);
		return NULL;
	}
	assign_iden(stab, iident(vararg), val);
	return NULL;
}
/*======================================+
 * llrpt_strcmp -- Compare two strings
 * usage: strcmp(STRING, STRING) -> INT
 *=====================================*/
PVALUE
llrpt_strcmp (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_nestr -- Compare two strings
 * usage: nestr(STRING, STRING) -> BOOLEAN
 *  calls nestr function
 *========================================*/
PVALUE
llrpt_nestr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	set_pvalue_bool(val1,(nestr(str1, str2) != 0));
	delete_pvalue(val2);
	return val1;
}
/*=========================================+
 * llrpt_eqstr -- Compare two strings
 * usage: eqstr(STRING, STRING) -> BOOLEAN
 *  calls eqstr function
 *========================================*/
PVALUE
llrpt_eqstr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_strtoint -- Convert string to integer
 * usage: strtoint(STRING) -> INT
 *  calls atoi function
 *======================================*/
PVALUE
llrpt_strtoint (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	set_pvalue_int(val, str ? atoi(str): 0);
	return val;
}
/*============================+
 * llrpt_clear -- Clear a list, set, indiseq
 * usage: clear(LIST) -> VOID
 *===========================*/
PVALUE
llrpt_clear (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
      LIST list;
      PNODE arg = (PNODE) iargs(node);
      PVALUE val = eval_and_coerce(PLIST, arg, stab, eflg);
      if (*eflg) {
              prog_var_error(node, stab, arg, val, nonlst1, "1");
              delete_pvalue(val);
              return NULL;
      }
      list = pvalue_to_list(val);
      make_list_empty(list); /* leaking elements? 2005-02-05 Perry */
      return NULL;
}
/*============================+
 * llrpt_list -- Create list
 * usage: list(IDENT) -> VOID
 *===========================*/
PVALUE
llrpt_list (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{

	PVALUE newval=0;
	PNODE arg = (PNODE) iargs(node);
	if (!iistype(arg, IIDENT)) {
		prog_var_error(node, stab, arg, NULL, nonvar1, "list");
		*eflg = TRUE;
		return NULL;
	}
	*eflg = FALSE;

	newval = create_new_pvalue_list();

	assign_iden(stab, iident(arg), newval);
	return NULL;
}
/*=======================================+
 * llrpt_push -- Push element on front of list
 * usage: push(LIST, ANY) -> VOID
 *======================================*/
PVALUE
llrpt_push (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_inlist -- see if element is in list
 * usage: inlist(LIST, STRING) -> BOOL
 *=====================================*/
PVALUE
llrpt_inlist (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	set_pvalue_bool(val, bFound);
	delete_pvalue(el);
	return val;
}
/*====================================+
 * llrpt_enqueue -- Enqueue element on list
 * usage: enqueue(LIST, ANY) -> VOID
 *===================================*/
PVALUE
llrpt_enqueue (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_requeue -- Add element to back of list
 * usage: requeue(LIST, ANY) -> VOID
 *=======================================*/
PVALUE
llrpt_requeue (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	LIST list=NULL;
	PVALUE el=NULL;
	PVALUE val = eval_and_coerce(PLIST, arg, stab, eflg);
	if (*eflg || !val) {
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
 * llrpt_pop -- Pop element from front of list
 * usage: pop(LIST) -> ANY
 *======================================*/
PVALUE
llrpt_pop (PNODE node, SYMTAB stab, BOOLEAN  *eflg)
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
 * llrpt_dequeue -- Remove element from back of list
 * usage: dequeue(LIST) -> ANY
 *============================================*/
PVALUE
llrpt_dequeue (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list;
	PVALUE val = eval_and_coerce(PLIST, iargs(node), stab, eflg);
	if (*eflg || !val) {
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
 * llrpt_empty -- Check if list is empty
 * usage: empty(LIST/TABLE/SET) -> BOOL
 *================================*/
PVALUE
llrpt_empty (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE val = eval_without_coerce(iargs(node), stab, eflg);
	int type = which_pvalue_type(val);
	BOOLEAN bEmpty = TRUE;

	if (val && (type == PLIST))
	{
		LIST list = pvalue_to_list(val);
		set_pvalue_int(val, length_list(list));
		bEmpty = !list || !length_list(list);
	}
	else if (val && (type == PTABLE))
	{
		TABLE table = pvalue_to_table(val);
		bEmpty = !table || !get_table_count(table);
	}
	else if (val && (type == PSET))
	{
       	INDISEQ seq = pvalue_to_seq(val);
		bEmpty = !seq || !length_indiseq(seq);
	}
	else
	{
		prog_error(node, _("the arg to empty is not a list, table or set"));
		*eflg = TRUE;
		return NULL;
	}

	set_pvalue_bool(val, bEmpty);
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
 * llrpt_getel -- Get nth value from list
 * usage: getel(LIST, INT) -> ANY
 *=================================*/
PVALUE
llrpt_getel (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	LIST list;
	INT ind;
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PLIST, arg, stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_error(node, nonlstx, "getel", "1");
		return NULL;
	}
	list = pvalue_to_list(val);
	delete_pvalue(val);
	val = eval_and_coerce(PINT, inext(arg), stab, eflg);
	if (*eflg || !val) {
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
 * llrpt_setel -- Set nth value in list
 * usage: setel(LIST, INT, ANY) -> VOID
 *======================================*/
PVALUE
llrpt_setel (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
/*==================================================+
 * llrpt_length -- Find length of list, indiseq or table
 * usage: length(LIST/TABLE/SET) -> INT
 *==================================================*/
PVALUE
llrpt_length (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE val = eval_without_coerce(iargs(node), stab, eflg);
	INT len=-1;

	if (val) {
		INT type = which_pvalue_type(val);
		if (type == PLIST) {
			LIST list = pvalue_to_list(val);
			len = (list ? length_list(list) : 0);
		} else if (type == PTABLE) {
			TABLE table = pvalue_to_table(val);
			len = (table ? get_table_count(table) : 0);
		} else if (type == PSET) {
			INDISEQ seq = pvalue_to_seq(val);
			len = (seq ? length_indiseq(seq) : 0);
		}
	}
	if (len == -1) {
		prog_error(node, _("the arg to length is not a list, table or set"));
		*eflg = TRUE;
		return NULL;
	}

	set_pvalue_int(val, len);
	return val;
}
/*==========================+
 * llrpt_not -- Not operation
 * usage: not(BOOL) -> BOOL
 *=========================*/
PVALUE
llrpt_not (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE val = eval_and_coerce(PBOOL, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to not is not boolean");
		return NULL;
	}
	set_pvalue_bool(val, !pvalue_to_bool(val));
	return val;
}
/*===============================+
 * llrpt_save -- Copy string
 * usage: save(STRING) -> STRING
 *==============================*/
PVALUE
llrpt_save (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_strlen -- Find length of string
 * usage: strlen(STRING) -> INT
 *================================*/
PVALUE
llrpt_strlen (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_concat -- Catenate strings
 * usage: concat(STRING [, STRING]+) -> STRING
 *============================================*/
PVALUE
llrpt_concat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
			sprintf(argnum, "%ld", argcnt+1);
			prog_var_error(node, stab, arg, val, nonstrx, "concat", argnum);
			return NULL;
		}
		str = pvalue_to_string(val);
		zs_apps(zstr, str);
		arg = inext(arg);
		++argcnt;
		delete_pvalue(val);
	}
	val = create_pvalue_from_zstr(&zstr);
	return val;
}
/*=======================================+
 * llrpt_lower -- Convert string to lower case
 * usage: lower(STRING) -> STRING
 *======================================*/
PVALUE
llrpt_lower (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	if (str) {
		ZSTR zstr = ll_tolowerz(str, uu8);
		set_pvalue_string(val, zs_str(zstr));
		zs_free(&zstr);
	}
	return val;
}
/*=======================================+
 * llrpt_upper -- Convert string to upper case
 * usage: upper(STRING) -> STRING
 *======================================*/
PVALUE
llrpt_upper (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	if (str) {
		ZSTR zstr = ll_toupperz(str, uu8);
		set_pvalue_string(val, zs_str(zstr));
		zs_free(&zstr);
	}
	return val;
}
/*=====================================+
 * llrpt_capitalize -- Capitalize string
 * usage: capitalize(STRING) -> STRING
 *====================================*/
PVALUE
llrpt_capitalize (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	if (str) {
		ZSTR zstr = ll_tocapitalizedz(str, uu8);
		set_pvalue_string(val, zs_str(zstr));
		zs_free(&zstr);
	}
	return val;
}
/*=====================================+
 * llrpt_titlcase -- Titlecase string
 * usage: capitalize(STRING) -> STRING
 * Created: 2001/12/30 (Perry Rapp)
 *====================================*/
PVALUE
llrpt_titlcase (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	if (str) {
		ZSTR zstr = ll_totitlecasez(str, uu8);
		set_pvalue_string(val, zs_str(zstr));
		zs_free(&zstr);
	}
	return val;
}
/*================================+
 * llrpt_pn -- Generate pronoun
 * usage: pn(INDI, INT) -> STRING
 *===============================*/
static char *mpns[] = {  N_("He"),  N_("he"), N_("His"), N_("his"), N_("him") };
/* "her_" = object form (Doug hit her) (do not include underscore in translation) */
static char *fpns[] = { N_("She"), N_("she"), N_("Her"), N_("her"), N_("her_") };
PVALUE
llrpt_pn (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT typ;
	PVALUE val;
	PNODE arg = (PNODE) iargs(node);
	NODE indi = eval_indi(arg, stab, eflg, NULL);
	STRING str="";
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
		str = _(fpns[typ]);
		/* disambiguation of object & possessive for l10n */
		if (eqstr(str, "her_"))
			str = "her";
	} else {
		str = _(mpns[typ]);
	}
	set_pvalue_string(val, str);
	return val;
}
/*==================================+
 * llrpt_print -- Print to stdout window
 * usage: print([STRING]+,) -> VOID
 *=================================*/
PVALUE
llrpt_print (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val;
	INT narg=1;
	while (arg) {
		STRING str;
		val = eval_and_coerce(PSTRING, arg, stab, eflg);
		if (*eflg || !val) {
			char nargstr[33];
			sprintf(nargstr, "%d", narg);
			prog_var_error(node, stab, arg, val, nonstrx, "print", nargstr);
			delete_pvalue(val);
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
		++narg;
	}
	return NULL;
}
/*=================================================+
 * llrpt_sex -- Find sex, as string M, F or U, of person
 * usage: sex(INDI) -> STRING
 *================================================*/
PVALUE
llrpt_sex (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_male -- Check if person is male
 * usage: male(INDI) -> BOOL
 *================================*/
PVALUE
llrpt_male (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, "the arg to male is not a person");
		return NULL;
	}
	if (!indi) return create_pvalue_from_bool(FALSE);
	return create_pvalue_from_bool((SEX(indi) == SEX_MALE));
}
/*=====================================+
 * llrpt_female -- Check if person is female
 * usage: female(INDI) -> BOOL
 *====================================*/
PVALUE
llrpt_female (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, "the arg to female is not a person");
		return NULL;
	}
	if (!indi) return create_pvalue_from_bool(FALSE);
	return create_pvalue_from_bool((SEX(indi) == SEX_FEMALE));
}
/*========================================+
 * llrpt_key -- Return person or family key
 * usage: key(INDI|FAM [,BOOL]) -> STRING
 *=======================================*/
PVALUE
llrpt_key (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = evaluate(arg, stab, eflg);
	CACHEEL cel;
	BOOLEAN strip = FALSE;
	CNSTRING key=0;
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
	key = cacheel_to_key(cel);
	return create_pvalue_from_string(strip ? key + 1 : key);
}
/*==============================================+
 * llrpt_root -- Return root of cached record
 * usage: root(INDI|FAM|EVEN|SOUR|OTHR) -> NODE
 *=============================================*/
PVALUE
llrpt_rot (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	RECORD rec = pvalue_to_record(val); /* may be NULL */
	NODE gnode=0;

	if (!rec) return FALSE;

	/* pvalue_to_record loads the record into direct cache */

	gnode = nztop(rec);
	set_pvalue_node(val, gnode); 
	ASSERT(gnode);
	return TRUE;
}
/*================================+
 * llrpt_inode -- Return root of person
 * usage: inode(INDI) -> NODE
 *==============================*/
PVALUE
llrpt_inode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE indi = eval_indi(iargs(node), stab, eflg, NULL);
	if (*eflg || !indi) {
		*eflg = TRUE;
		prog_error(node, _(nonind1), "inode");
		return NULL;
	}
	return create_pvalue_from_node(indi);
}
/*================================+
 * llrpt_fnode -- Return root of family
 * usage: fnode(FAM) -> NODE
 *===============================*/
PVALUE
llrpt_fnode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE fam = eval_fam(iargs(node), stab, eflg, NULL);
	if (*eflg) {
		prog_error(node, nonfam1, "fnode");
		return NULL;
	}
	/* fam may be NULL */
	return create_pvalue_from_node(fam);
}
/*=============================+
 * llrpt_table -- Create table
 * usage: table(IDENT) -> VOID
 *============================*/
PVALUE
llrpt_table (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PVALUE newval=0;
	PNODE var = (PNODE) iargs(node);
	if (!iistype(var, IIDENT)) {
		*eflg = TRUE;
		prog_var_error(node, stab, var, NULL, nonvar1, "table");
		return NULL;
	}
	newval = create_new_pvalue_table();

	assign_iden(stab, iident(var), newval);
	return NULL;
}
/*=========================================+
 * llrpt_insert -- Add element to table
 * usage: insert(TAB, STRING, ANY) -> VOID
 *========================================*/
PVALUE
llrpt_insert (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val=NULL;
	PVALUE valtab = eval_and_coerce(PTABLE, arg, stab, eflg);
	TABLE tab=0;
	STRING str=0;

	if (*eflg || !valtab) {
        *eflg = TRUE;
		prog_var_error(node, stab, arg, valtab, nontabx, "insert", "1");
		goto exit_insert;
	}
	tab = pvalue_to_table(valtab);

	arg = inext(arg);
	val = eval_and_coerce(PSTRING, arg, stab, eflg);
	if (*eflg || !val || !pvalue_to_string(val)) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val, nonstrx, "insert", "2");
		goto exit_insert;
	}
	str = pvalue_to_string(val);
	if (str) 
	    str = strsave(str);
	delete_pvalue(val);

	val = evaluate(inext(arg), stab, eflg);
	if (*eflg || !val) {
		*eflg = TRUE;
		prog_error(node, "3rd arg to insert is in error");
		goto exit_insert;
	}

	insert_table_ptr(tab, str, val);

exit_insert: /* free memory and leave */

	delete_pvalue(valtab); /* finished with our copy of table */
	strfree(&str);
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
 * llrpt_lookup -- Look up element in table
 * usage: lookup(TAB, STRING) -> ANY
 *===================================*/
PVALUE
llrpt_lookup (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg;
	PVALUE newv, val;
	TABLE tab;
	STRING str;

	arg = (PNODE) iargs(node);
	val = eval_and_coerce(PTABLE, arg, stab, eflg);
	if (*eflg || !val) {
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
 * llrpt_trim -- Trim string if too long
 * usage: trim(STRING, INT) -> STRING
 *===================================*/
PVALUE
llrpt_trim (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	set_pvalue_string(val2, trim(str, len));
	delete_pvalue(val1);
	return val2;
}
/*======================================+
 * llrpt_trimname -- Trim name if too long
 * usage: trimname(INDI, INT) -> STRING
 *=====================================*/
PVALUE
llrpt_trimname (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	INT len;
	PVALUE val=NULL;
	NODE indi = eval_indi(arg, stab, eflg, (CACHEEL *) NULL);
	STRING str=NULL;
	if (*eflg) {
		prog_error(node, nonindx, "trimname", "1");
		return NULL;
	}
	if (!indi) return create_pvalue_from_string("");
	if (!(indi = NAME(indi)) || !nval(indi)) {
		if (getlloptint("RequireNames", 0)) {
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
	if (str)
		str = strsave(str);
	set_pvalue_string(val, str);
	strfree(&str);
	return val;
}
/*==============================+
 * llrpt_date -- Return date of event
 * usage: date(EVENT) -> STRING
 *=============================*/
PVALUE
llrpt_date (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE line=NULL;
	STRING str=NULL;
	PVALUE val = eval_and_coerce(PGNODE, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, nonnod1, "date");
		return NULL;
	}
	line = pvalue_to_node(val);
	str = event_to_date(line, FALSE);
	/* save string in case node is temp (will get deleted in create_pvalue) */
	if (str)
		str = strsave(str);
	delete_pvalue(val);
	val = create_pvalue_from_string(str);
	strfree(&str);
	return val;
}
/*==========================================+
 * llrpt_date2jd -- Return julian day of date
 * usage: date2jd(EVENT) -> FLOAT
 *        date2jd(STRING) -> FLOAT
 * creation : Patrick Texier 2006/05/22
 * Added to cvs: 2006/06/10
 * =========================================*/
PVALUE
llrpt_date2jd (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str;
	GDATEVAL gdv;
	
	FLOAT jd = 0;
	PVALUE val = eval_without_coerce(iargs(node), stab, eflg);
	/* Handle string input */
	if (val && which_pvalue_type(val) == PSTRING) {
		str = pvalue_to_string(val);
	}
	else /* handle NODE input */
	{
		NODE evnt;
		coerce_pvalue(PGNODE, val, eflg);
		if (*eflg) {
			/* Input neither string nor node, error */
			prog_error(node, nonnodstr1, "date2jd");
			return NULL;
		}
		evnt = pvalue_to_node(val);
		str = event_to_date(evnt, FALSE);
	}
	/* Parse into lifelines date structure (GDATEVAL) */
	gdv = extract_date(str);
	/* Compute julian date value as float */
	jd = julianday(gdv);

	free_gdateval(gdv);
	*eflg = FALSE;
	return create_pvalue_from_float(jd);
}
/*=============================================+
 * llrpt_dayofweek -- Return day of week
 * usage: dayofweek(EVENT) -> STRING
 *        dayofweek(STRING) -> STRING
 * creation: Patrick Texier 2006/05/22
 * Added to cvs: 2006/06/10
 *=============================================*/
static char *dofw[] = {  N_("Sunday"),  N_("Monday"), N_("Tuesday"),
	N_("Wednesday"), N_("Thursday"), N_("Friday"), N_("Saturday") };
PVALUE
llrpt_dayofweek (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str = 0, str2 = 0;
	GDATEVAL gdv;
	INT weekdaynum = 0;
	
	FLOAT jd = 0;
	PVALUE val = eval_without_coerce(iargs(node), stab, eflg);
	/* Handle string input */
	if (val && which_pvalue_type(val) == PSTRING) {
		str = pvalue_to_string(val);
	}
	else /* handle NODE input */
	{
		NODE evnt;
		coerce_pvalue(PGNODE, val, eflg);
		if (*eflg) {
			prog_error(node, nonnodstr1, "dayofweek");
			return NULL;
		}
		evnt = pvalue_to_node(val);
		str = event_to_date(evnt, FALSE);
	}
	/* Parse into lifelines date structure (GDATEVAL) */
	gdv = extract_date(str);
	/* Compute julian date value as float */
	jd = julianday(gdv);
	/* Compute which day of week */
	weekdaynum = (INT)(jd + 1.5) % 7;
	/* Convert to localized name */
	str2 = _(dofw[weekdaynum]);

	free_gdateval(gdv);
	*eflg = FALSE;
	return create_pvalue_from_string(str2);
}
/*===============================================+
 * llrpt_jd2date -- Return date from Julian Day
 * usage : jd2date(FLOAT) -> EVENT
 * creation : Patrick Texier 2006/05/22
 * Added to cvs: 2006/06/10
 * =============================================== */
static char *gedmonths[] = { "JAN", "FEB","MAR", "APR", "MAY", "JUN",
	"JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
/* TODO Use an existing .h */

PVALUE
llrpt_jd2date (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	FLOAT val2;
	FLOAT f, z, a, ab, b, c, d, e;
	INT yr, mo, dy;
	NODE prnt, chil;
	static char str[12];

	PVALUE val = eval_and_coerce(PFLOAT, arg, stab, eflg);

	if (*eflg) {
		prog_error(node, nonflox, "jd2date", "1");
		return NULL;
	}
	/* Extract julian date float value */
	val2 = pvalue_to_float(val);
	z = floor(val2 + 0.5);
	f = floor(val2 + 0.5 - z);
	/* Gregorian correction */
	if (z >= 2299159.5) {
		ab = floor((z - 1867216.25) / 36524.25);
		a = z + 1 + ab - floor(ab / 4);
	} else {
		a = z;
	}
	b = a + 1524;
	c = floor((b - 122.1) / 365.25);
	d = floor(365.25 * c);
	e = floor((b - d) / 30.6001);
	dy  = (INT)(b - d - floor(30.6001 * e) + f);
	if (e <= 13) {
		mo = (INT)(e - 1);
	} else {
		mo = (INT)(e - 13);
	}
	if (mo >= 2) {
		yr = (INT)(c - 4716);
	} else {
		yr = (INT)(c - 4715);
	}
	/* Now print GEDCOM style date string */
	sprintf(str, "%ld %s %ld", dy, gedmonths[mo - 1], yr);
	/* Create an EVEN node with subordinate DATE node */
	prnt = create_temp_node(NULL, "EVEN", NULL, NULL);
	chil = create_temp_node(NULL, "DATE", str, prnt);
	nchild(prnt) = chil;

	*eflg = FALSE;

	return create_pvalue_from_node(prnt);
}

/*======================================
 * Julian day calculation
 * Creation: Patrick Texier 2006/05/22
 * Added to cvs: 2006/06/10
 * ===================================== */
static FLOAT
julianday (GDATEVAL gdv)
{
	INT da = 0, mo = 0, yr = 0;
	INT mmo = 0, yyr = 0;
	FLOAT jd = 0.0;
	
	da = date_get_day(gdv);
	/* 1th if no day */
	if (da == 0)
		da = 1;
	mo = date_get_month(gdv);
	/* January if no month */
	if (mo == 0)
		mo = 1;
	yr = date_get_year(gdv);
	
	if (mo < 3) {
		yyr = yr - 1;
		mmo = mo + 12;
	} else {
		yyr = yr;
		mmo = mo;
	}
	jd = floor(yyr * 365.25);
	jd += floor(30.6001 * (mmo + 1));
	jd += da + 1720994.5;
	/*  gregorian correction after 1582/10/14 */
	if ( jd > 2299159.5 )
		jd = jd + 2.0 - floor(yyr/100.0) + floor(yyr / 400.0);
	return jd;
}
/*=====================================================+
 * normalize_year -- Modify year before returning to report
 * historical behavior is that 0 is the return for unknown year
 *====================================================*/
static INT
normalize_year (INT yr)
{
	return (yr == BAD_YEAR) ? 0 : yr;
}
/*=====================================================+
 * llrpt_extractdate -- Extract date from EVENT or DATE NODE
 * usage: extractdate(NODE, VARB, VARB, VARB) -> VOID
 *====================================================*/
PVALUE
llrpt_extractdate (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str;
	NODE line;
	INT da = 0, mo = 0, yr = 0;
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	PNODE dvar = inext(arg);
	PNODE mvar = inext(dvar);
	PNODE yvar = inext(mvar);
	GDATEVAL gdv = 0;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnodx, "extractdate", "1");
		return NULL;
	}
	line = pvalue_to_node(val);
	*eflg = TRUE; /* error if we don't make it all the way through */
	if (!line) {
		prog_var_error(node, stab, arg, val, nonnodx, "extractdate", "1");
		return NULL;
	}
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
	if (str)
	    str = strsave(str); /* save in case we delete line node */
	delete_pvalue(val);
	gdv = extract_date(str);
	strfree(&str);
	/* TODO: deal with date information */
	da = date_get_day(gdv);
	mo = date_get_month(gdv);
	yr = date_get_year(gdv);
	yr = normalize_year(yr);
	assign_iden(stab, iident(dvar), create_pvalue_from_int(da));
	assign_iden(stab, iident(mvar), create_pvalue_from_int(mo));
	assign_iden(stab, iident(yvar), create_pvalue_from_int(yr));
	free_gdateval(gdv);
	*eflg = FALSE;
	return NULL;
}
/*==================================================================+
 * llrpt_extractdatestr -- Extract date from STRING
 * usage: extractdatestr(VARB, VARB, VARB, VARB, VARB[, STRING]) -> VOID
 *==================================================================*/
PVALUE
llrpt_extractdatestr (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str = NULL, yrstr;
	INT mod=0, da=0, mo=0, yr=0;
	PVALUE val;
	PNODE date;
	PNODE modvar = (PNODE) iargs(node);
	PNODE dvar = inext(modvar);
	PNODE mvar = inext(dvar);
	PNODE yvar = inext(mvar);
	PNODE ystvar = inext(yvar);
	GDATEVAL gdv = 0;
	if (!iistype(modvar, IIDENT)) {
		*eflg = TRUE;
		prog_error(node, nonvarx, "extractdatestr", "1");
		return NULL;
	}
	if (!iistype(dvar, IIDENT)) {
		*eflg = TRUE;
		prog_error(node, nonvarx, "extractdatestr", "2");
		return NULL;
	}
	if (!iistype(mvar, IIDENT)) {
		*eflg = TRUE;
		prog_error(node, nonvarx, "extractdatestr", "3");
		return NULL;
	}
	if (!iistype(yvar, IIDENT)) {
		*eflg = TRUE;
		prog_error(node, nonvarx, "extractdatestr", "4");
		return NULL;
	}
	if (!iistype(ystvar, IIDENT)) {
		*eflg = TRUE;
		prog_error(node, nonvarx, "extractdatestr", "5");
		return NULL;
	}
	if ((date = inext(ystvar))) {
		val = eval_and_coerce(PSTRING, date, stab, eflg);
		if (*eflg || !val) {
			*eflg = TRUE;
			prog_error(node, nonstrx, "extractdatestr", "6");
			delete_pvalue(val);
			return NULL;
		}
		str = pvalue_to_string(val);
	}
	gdv = extract_date(str);
	/* TODO: deal with date information */
	mod = date_get_mod(gdv);
	da = date_get_day(gdv);
	mo = date_get_month(gdv);
	yr = date_get_year(gdv);
	yr = normalize_year(yr);
	yrstr = date_get_year_string(gdv);
	if (!yrstr) yrstr="";
	assign_iden(stab, iident(modvar), create_pvalue_from_int(mod));
	assign_iden(stab, iident(dvar), create_pvalue_from_int(da));
	assign_iden(stab, iident(mvar), create_pvalue_from_int(mo));
	assign_iden(stab, iident(yvar), create_pvalue_from_int(yr));
	assign_iden(stab, iident(ystvar), create_pvalue_from_string(yrstr));
	free_gdateval(gdv);
	return NULL;
}
/*=================================================+
 * llrpt_stddate -- Return standard date format of event
 * usage: stddate(EVENT) -> STRING
 *    or  stddate(STRING) -> STRING
 *================================================*/
static INT daycode = 0;
static INT monthcode = 3;
static INT yearcode = 0;
static INT datecode = 0;
static INT eratimecode = 0;
static INT cmplxcode = 1;
PVALUE
llrpt_stddate (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str=0;
	PVALUE val = eval_without_coerce(iargs(node), stab, eflg);
	if (val && which_pvalue_type(val) == PSTRING) {
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
	set_pvalue_string(val, do_format_date(str,
	    daycode, monthcode, yearcode, datecode, eratimecode, FALSE));
	return val;
}
/*========================================================================+
 * llrpt_complexdate -- Return standard date format of event, including modifiers
 * usage: complexdate(EVENT) -> STRING
 *      or  complexdate(STRING) -> STRING
 *=======================================================================*/
PVALUE
llrpt_complexdate (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str=0;
	PVALUE val = eval_without_coerce(iargs(node), stab, eflg);
	if (val && which_pvalue_type(val) == PSTRING) {
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
	set_pvalue_string(val, do_format_date(str,
	    daycode, monthcode, yearcode, datecode, eratimecode, cmplxcode));
	return val;
}
/*===============================================+
 * llrpt_dayformat -- Set day format
 * usage: dayformat(INT) -> NULL
 *==============================================*/
PVALUE
llrpt_dayformat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	if (!is_valid_dayfmt(value)) value = 2;
	daycode = value;
	return NULL;
}
/*===============================================+
 * llrpt_monthformat -- Set month format
 * usage: monthformat(INT) -> NULL
 *==============================================*/
PVALUE
llrpt_monthformat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	if (!is_valid_monthfmt(value)) value = 8;
	monthcode = value;
	return NULL;
}
/*===============================================+
 * llrpt_yearformat -- Set month format
 * usage: yearformat(INT) -> NULL
 * Created: 2001/12/24, Perry Rapp
 *==============================================*/
PVALUE
llrpt_yearformat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_dateformat -- Set date format
 * usage: dateformat(INT) -> NULL
 *================================================*/
PVALUE
llrpt_dateformat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_eraformat -- Set format for AD/BC trailer
 * usage: eraformat(INT) -> NULL
 * Created: 2001/12/28, Perry Rapp
 *==============================================*/
PVALUE
llrpt_eraformat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_complexformat -- Set complex format
 * usage: complexformat(INT) -> NULL
 * Created: 2001/12/24, Perry Rapp
 *==============================================*/
PVALUE
llrpt_complexformat (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_datepic -- Set custom ymd date picture string
 * usage: datepic(STRING) -> NULL
 * Created: 2001/12/30, Perry Rapp
 *==============================================*/
PVALUE
llrpt_datepic (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_complexpic -- Set custom picture string for
 *  a complex date
 * usage: complexpic(INT, STRING) -> NULL
 * Created: 2001/12/30, Perry Rapp
 * TODO: We could add a 3rd argument giving language specifier
 *  when we are localizing
 *==============================================*/
PVALUE
llrpt_complexpic (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_year -- Return year of event
 * usage: year(EVENT) -> STRING
 *      or  year(STRING) -> STRING
 *=============================*/
PVALUE
llrpt_year (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str=0;
	char buff[20];
	GDATEVAL gdv;
	PVALUE val = eval_without_coerce(iargs(node), stab, eflg);
	if (val && which_pvalue_type(val) == PSTRING) {
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
	str = date_get_year_string(gdv);
	if (str && str[0]) {
		/* we'll use year string, now in str */
	} else {
		INT yr = date_get_year(gdv);
		if (yr != BAD_YEAR) {
			/* no year string, so must have been a simple number */
			snprintf(buff, sizeof(buff), "%ld", yr);
			str = buff;
		} else {
			str = 0;
		}
	}
	set_pvalue_string(val, str);
	free_gdateval(gdv);
	return val;
}
/*================================+
 * llrpt_place -- Return place of event
 * usage: place(EVENT) -> STRING
 *===============================*/
PVALUE
llrpt_place (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE evnt=NULL;
	PNODE arg = iargs(node);
	STRING str=NULL;
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);

	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "place");
		return NULL;
	}
	evnt = pvalue_to_node(val);
	str = event_to_plac(evnt, FALSE);
	if (str)
		str = strsave(str);
	set_pvalue_string(val, str);
	strfree(&str);
	return val;
}
/*============================+
 * llrpt_tag -- Return tag of node
 * usage: tag(NODE) -> STRING
 *===========================*/
PVALUE
llrpt_tag (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE ged=NULL;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	STRING str=NULL;
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonnod1, "tag");
		return NULL;
	}
	ged = pvalue_to_node(val);
	if (ged)
		str = ntag(ged);
	if (str)
		str = strsave(str);
	set_pvalue_string(val, str);
	strfree(&str);
	return val;
}
/*===============================+
 * llrpt_value -- Return value of node
 * usage: value(NODE) -> STRING
 *==============================*/
PVALUE
llrpt_value (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	NODE ged=NULL;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PGNODE, arg, stab, eflg);
	STRING str = 0;
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
	/*
	save away string, so it doesn't die when val is cleared for 
	assignment below
	*/
	str = nval(ged);
	if (str)
		str = strsave(str);
	set_pvalue_string(val, str);
	strfree(&str);
	return val;
}
/*=============================+
 * llrpt_xref -- Return xref of node
 * usage: xref(NODE) -> STRING
 *============================*/
PVALUE
llrpt_xref (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	set_pvalue_string(val, nxref(ged));
	return val;
}
/*===============================+
 * llrpt_child -- Return child of node
 * usage: child(NODE) -> NODE
 *==============================*/
PVALUE
llrpt_child (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_parent -- Return parent of node
 * usage: parent(NODE) -> NODE
 *================================*/
PVALUE
llrpt_parent (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_sibling -- Return next sibling of node
 * usage: sibling(NODE) -> NODE
 *=======================================*/
PVALUE
llrpt_sibling (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
	set_pvalue_node(val, nsibling(ged));
	return val;
}
/*===============================+
 * llrpt_level -- Return level of node
 * usage: level(NODE) -> INT
 *==============================*/
PVALUE
llrpt_level (PNODE node, SYMTAB stab, BOOLEAN *eflg)
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
 * llrpt_copyfile -- Copy file to output
 * usage: copyfile(STRING) -> VOID
 *================================*/
PVALUE
llrpt_copyfile (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	FILE *cfp=NULL;
	STRING fname;
	PNODE arg = iargs(node);
	PVALUE val = eval_and_coerce(PSTRING, arg, stab, eflg);
	char buffer[1024];
	STRING programsdir = getlloptstr("LLPROGRAMS", ".");
	if (*eflg)  {
		prog_error(node, nonstr1, "copyfile");
		goto copyfile_end;
	}
	fname = pvalue_to_string(val);
	if (!(cfp = fopenpath(fname, LLREADTEXT, programsdir
		, (STRING)NULL, uu8, (STRING *)NULL))) {
		*eflg = TRUE;
		prog_var_error(node, stab, arg, val, nonfname1, "copyfile");
		goto copyfile_end;
	}
	delete_pvalue(val);
	while (fgets(buffer, sizeof(buffer), cfp)) {
		poutput(buffer, eflg);
		if (*eflg)
			goto copyfile_end;
	}
copyfile_end:
	if (cfp) fclose(cfp);
	return NULL;
}
/*========================+
 * llrpt_nl -- Newline function
 * usage: nl() -> STRING
 *=======================*/
PVALUE
llrpt_nl (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_string("\n");
}
/*=========================+
 * llrpt_space -- Space function
 * usage: sp() -> STRING
 *========================*/
PVALUE
llrpt_space (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_string(" ");
}
/*=============================+
 * llrpt_qt -- Double quote function
 * usage: qt() -> STRING
 *============================*/
PVALUE
llrpt_qt (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_string("\"");
}
/*=============================+
 * llrpt_indi -- Convert key to INDI
 * usage: indi(STRING) -> INDI
 *============================*/
PVALUE
llrpt_indi (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str;
	char scratch[200], *p, *q = scratch;
	int strip_at = 0;
	INT c;
	PVALUE val = eval_and_coerce(PSTRING, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, nonstr1, "indi");
		return NULL;
	}
	p = str = pvalue_to_string(val);
	if (p && *p == '@') {
	    strip_at = 1;
	    p++;
	}
	if (!p || *p++ != 'I' || *p == 0) {
		delete_pvalue(val);
		return create_pvalue(PINDI,0);
	}
	*q++ = 'I';
	while (chartype(c = (uchar)*p++) == DIGIT)
		*q++ = c;
	*q = 0;
	delete_pvalue(val);
	if (c != 0 && (strip_at == 0 || c != '@')) {
		return create_pvalue(PINDI,0);
	}
	if (strlen(scratch) == 1) return create_pvalue(PINDI,0);

	val = create_pvalue_from_indi_key(scratch);
	return val;
}
/*===========================+
 * llrpt_fam -- Convert key to FAM
 * usage: fam(STRING) -> FAM
 *==========================*/
PVALUE
llrpt_fam (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	STRING str;
	char scratch[200], *p, *q = scratch;
	int strip_at = 0;
	INT c;
	PVALUE val = eval_and_coerce(PSTRING, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, nonstr1, "fam");
		return NULL;
	}
	p = str = pvalue_to_string(val);
	if (p && *p == '@') {
	    strip_at = 1;
	    p++;
	}
	if (!p || *p++ != 'F' || *p == 0) {
		delete_pvalue(val);
		return create_pvalue(PFAM,0);
	}
	*q++ = 'F';
	while (chartype(c = (uchar)*p++) == DIGIT)
		*q++ = c;
	*q = 0;
	delete_pvalue(val);
	if (c != 0 && (strip_at == 0 || c != '@')) {
		return create_pvalue(PFAM,0);
	}
	if (strlen(scratch) == 1) return create_pvalue(PFAM,0);

	val = create_pvalue_from_fam_key(scratch);
	return val;
}
/*=======================================+
 * eval_indi -- Evaluate person expression
 *  if any error occurs, *eflg is set to non-null
 *  if caller wants pointer to cache element, pass in non-null pcel
 *======================================*/
NODE
eval_indi (PNODE expr, SYMTAB stab, BOOLEAN *eflg, CACHEEL *pcel)
{
	return eval_indi2(expr, stab, eflg, pcel, NULL);
}
/*=======================================+
 * eval_indi2 -- Evaluate person expression
 *  If pval is non-null, it will be used to return
 *  the evaluation PVALUE in case of error (*eflag set to non-zero)
 *  in this case, caller must delete it
 *======================================*/
NODE
eval_indi2 (PNODE expr, SYMTAB stab, BOOLEAN *eflg, CACHEEL *pcel, PVALUE *pval)
{
	NODE indi=0;
	CACHEEL cel=0;
	PVALUE val = eval_and_coerce(PINDI, expr, stab, eflg);

	if (*eflg || !val) {
		if (val) {
			if (pval) {
				*pval = val;
				/* now caller owns val */
			} else {
				delete_pvalue(val);
				val=NULL;
			}
		}
		return NULL;
	}
	cel = pvalue_to_cel(val);
	delete_pvalue(val);
	if (!cel) return NULL;
	indi = cacheel_to_node(cel);
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
	NODE fam=0;
	CACHEEL cel=0;
	PVALUE val = eval_and_coerce(PFAM, expr, stab, eflg);
	if (*eflg || !val) return NULL;
	cel = pvalue_to_cel(val);
	delete_pvalue(val);
	if (!cel) return NULL;
	fam = cacheel_to_node(cel);
	if (nestr("FAM", ntag(fam))) {
		*eflg = TRUE;
		return NULL;
	}
	if (pcel) *pcel = cel;
	return fam;
}
/*=================================================+
 * llrpt_free -- free up data associated with a variable 
 * usage: free(IDEN]) --> VOID
 *=======================================*/
PVALUE
llrpt_free (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
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
		set_pvalue(val, PNULL, NULL);
	}
	return NULL;
}
/*=============================================+
 * llrpt_float -- Converts a NUMBER to a FLOAT
 * usage: float(NUMBER) -> FLOAT
 *============================================*/
PVALUE
llrpt_float (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PFLOAT, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonflox, "f", "1");
		return NULL;
	}
	return val;
}
/*=============================================+
 * llrpt_int -- Converts a NUMBER to an INT
 * usage: int(NUMBER) -> INT
 *============================================*/
PVALUE
llrpt_int (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val = eval_and_coerce(PINT, arg, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg, val, nonflox, "f", "1");
		return NULL;
	}
	return val;
}
/*=============================================+
 * llrpt_test -- Perform tests on a file or
 *           directory parameter.
 * usage: test(STRING, STRING) -> BOOL
 *============================================*/
PVALUE
llrpt_test (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE arg1, arg2;
	PVALUE arg1val=0, arg2val=0, val = NULL;
	STRING arg1str, arg2str;
	struct stat statdata;
	int rc;

	arg1 = (PNODE) iargs(node);
	arg1val = eval_and_coerce(PSTRING, arg1, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg1, arg1val, nonstrx, "test", "1");
		goto end_test;
	}
	arg1str = pvalue_to_string(arg1val);

	arg2 = (PNODE) inext(arg1);
	arg2val = eval_and_coerce(PSTRING, arg2, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, arg2, arg2val, nonstrx, "test", "2");
		goto end_test;
	}
	arg2str = pvalue_to_string(arg2val);

	if (arg1str == 0 ) {
		prog_var_error(node, stab, arg1, arg1val, nonstrx, "test", "1");
		goto end_test;
	}
	if (arg2str == 0 ) {
		prog_var_error(node, stab, arg2, arg2val, nonstrx, "test", "2");
		goto end_test;
	}

	rc = stat(arg2str, &statdata);
	if (rc) {
		val = create_pvalue_from_bool(FALSE);
		goto end_test;
	}

	if (eqstr(arg1str,"r")) {
		if (access(arg2str,R_OK)==0) 
			val = create_pvalue_from_bool(TRUE);

	} else if (eqstr(arg1str,"w")) {
		if (access(arg2str,W_OK)==0)
			val = create_pvalue_from_bool(TRUE);

	} else if (eqstr(arg1str,"e")) {
			val = create_pvalue_from_bool(TRUE);

	} else if (eqstr(arg1str,"z")) {
		if (statdata.st_size == 0)
			val = create_pvalue_from_bool(TRUE);

	} else if (eqstr(arg1str,"s")) {
		if (statdata.st_size != 0)
			val = create_pvalue_from_bool(TRUE);

	} else if (eqstr(arg1str,"f")) {
		if ((statdata.st_mode & S_IFMT) & S_IFREG)
			val = create_pvalue_from_bool(TRUE);

	} else if (eqstr(arg1str,"d")) {
		if ((statdata.st_mode & S_IFMT) & S_IFDIR)
			val = create_pvalue_from_bool(TRUE);
	} else {
		prog_var_error(node, stab, arg1, arg1val, badargx, "test", "1");
		goto end_test;
	}

end_test:
	if (val == NULL)
	    val = create_pvalue_from_bool(FALSE);
	if (arg1val) delete_pvalue(arg1val);
	if (arg2val) delete_pvalue(arg2val);
	return val;
}
