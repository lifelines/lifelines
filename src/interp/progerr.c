/* 
   Copyright (c) 2006 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * progerr.c -- Display lifelines report errors
 *  and implement the report debugger
 *==============================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interpi.h"
#include "indiseq.h"
#include "rptui.h"
#include "feedback.h"
#include "arch.h"
#include "lloptions.h"
#include "zstr.h"
#include "icvt.h"
#include "date.h"
#include "xlat.h"

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif

/*********************************************
 * external/imported variables
 *********************************************/

extern INT progerror;
extern BOOLEAN progparsing;
extern INT rpt_cancelled;

/*********************************************
 * local types
 *********************************************/

struct dbgsymtab_s
{
	PVALUE * values;
	STRING * displays;
	INT count;
	INT current;
};

/*********************************************
 * local function prototypes
 *********************************************/

static INT count_symtab_ancestors(SYMTAB stab);
static void disp_dbgsymtab(CNSTRING title, struct dbgsymtab_s * sdata);
static void disp_list(LIST list);
static void disp_pvalue(PVALUE val);
static void disp_seq(INDISEQ seq);
static void disp_symtab(STRING title, SYMTAB stab);
static void disp_table(TABLE tab);
static void format_dbgsymtab_val(STRING key, PVALUE val, struct dbgsymtab_s * sdata);
static void format_dbgsymtab_str(STRING key, STRING valstr, PVALUE val, struct dbgsymtab_s * sdata);
static void free_dbgsymtab_arrays(struct dbgsymtab_s * sdata);
static SYMTAB get_symtab_ancestor(SYMTAB stab, INT index);
static void init_dbgsymtab_arrays(struct dbgsymtab_s * sdata, INT nels);
static PVALUE * make_empty_pvalue_table(INT nels);
static STRING * make_empty_string_table(INT nels);
static void prog_var_error_zstr(PNODE node, SYMTAB stab, PNODE arg, PVALUE val, ZSTR zstr);
static STRING vprog_error(PNODE node, STRING fmt, va_list args);

/*********************************************
 * local variables
 *********************************************/

static INT dbg_mode = 0;
static char vprog_prevfile[MAXPATHLEN]="";
static INT vprog_prevline=-1;

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

void
init_debugger (void)
{
	dbg_mode = 0;
	/* clear previous information */
	vprog_prevfile[0] = 0;
	vprog_prevline=-1;
}
/*=============================================+
 * prog_var_error_zstr -- Report a run time program error
 *  node:  [IN]  current parse node
 *  stab:  [IN]  current symbol table (lexical scope)
 *  arg:   [IN]  if non-null, parse node of troublesome argument
 *  val:   [IN]  if non-null, PVALUE of troublesome argument
 *  zstr:  [IN]  message
 *
 * Inline debugger is implemented here
 * See vprog_error
 * Created: 2002/02/17, Perry Rapp
 *============================================*/
static void
prog_var_error_zstr (PNODE node, SYMTAB stab, PNODE arg, PVALUE val, ZSTR zstr)
{
	STRING choices[5];
	INT rtn=0;
	SYMTAB curstab = stab; /* currently displayed symbol table */
	INT nlevels=0; /* number frames on callstack */
	INT curlevel=0; /* 0 is lowest */

	ASSERT(zstr);

	if (val) {
		ZSTR zval = describe_pvalue(val);
		zs_appf(zstr, " (value: %s)", zs_str(zval));
	} else if (arg) {
		INT max=40 + zs_len(zstr); /* not too much argument description */
		/* arg isn't evaluated, but describe will at least give its type */
		zs_apps(zstr, " (arg: ");
		describe_pnode(arg, zstr, max);
		zs_apps(zstr, ")");
	}
	prog_error(node, zs_str(zstr));
	zs_free(&zstr);

	if (dbg_mode != -99 && dbg_mode != 3) {
		INT ch = 0;
		while (!(ch=='d' || ch=='q'))
			ch = rptui_prompt_stdout(_("Enter d for debugger, q to quit"));
		if (ch == 'q')
			dbg_mode = -99;
	}

	/* call stack size & location */
	nlevels = count_symtab_ancestors(stab) + 1;
	curlevel = 0;

	/* report debugger loop */
	while (dbg_mode != -99) {
		ZSTR zstr=zs_new();
		INT n=0;
		/* 0: display local variable(s) */
		n = (curstab->tab ? get_table_count(curstab->tab) : 0);
		zs_setf(zstr, _pl("Display local (%d var)",
			"Display locals (%d vars)", n), n);
		zs_appf(zstr, " [%s]", curstab->title);
		choices[0] = strsave(zs_str(zstr));
		/* 1: display global variables */
		n = (globtab->tab ? get_table_count(globtab->tab) : 0);
		zs_setf(zstr, _pl("Display global (%d var)",
			"Display globals (%d vars)", n), n);
		choices[1] = strsave(zs_str(zstr));
		/* 2: up call stack */
		n = nlevels - curlevel - 1;
		zs_setf(zstr, _pl("Call stack has %d higher level", "Call stack has %d higher levels", n), n);
		zs_apps(zstr, ". ");
		if (n > 0) {
			zs_apps(zstr, _(" Go up one level"));
			zs_appf(zstr, "(%s)", curstab->parent->title);
		}
		choices[2] = strsave(zs_str(zstr));
		/* 3: down call stack */
		n = curlevel;
		zs_setf(zstr, _pl("Call stack has %d lower level", "Call stack has %d lower levels", n), n);
		zs_apps(zstr, ". ");
		if (n > 0) {
			CNSTRING title = get_symtab_ancestor(stab, n-1)->title;
			zs_apps(zstr, _(" Go down one level"));
			zs_appf(zstr, "(%s)", title);
		}
		choices[3] = strsave(zs_str(zstr));
		/* quit */
		choices[4] = strsave(_("Quit debugger"));
dbgloop:
		rtn = rptui_choose_from_array(_("Report debugger"), ARRSIZE(choices), choices);
		if (rtn == 4 || rtn == -1) {
			dbg_mode = -99;
		} else if (rtn == 0) {
			disp_symtab(_("Local variables"), curstab);
			goto dbgloop;
		} else if (rtn == 1) {
			disp_symtab(_("Global variables"), globtab);
			goto dbgloop;
		} else if (rtn == 2) {
			if (curlevel+1 < nlevels) {
				curstab = curstab->parent;
				++curlevel;
				ASSERT(curstab);
			}
		} else if (rtn == 3) {
			if (curlevel > 0) {
				curstab = get_symtab_ancestor(stab, curlevel-1);
				--curlevel;
				ASSERT(curstab);
			}
		}
		zs_free(&zstr);
		free_array_strings(ARRSIZE(choices), choices);
	}
}
/*=============================================+
 * count_symtab_ancestors -- count number of symbol tables
 *  in call stack above specified one
 *============================================*/
static INT
count_symtab_ancestors (SYMTAB stab)
{
	INT ct=0;
	SYMTAB curstab = stab;
	while (curstab->parent) {
		++ct;
		curstab = curstab->parent;
	}
	return ct;
}
/*=============================================+
 * get_symtab_ancestor -- return nth ancestor
 *   where n is the index passed by caller
 *   (0 is valid, and returns same one)
 *============================================*/
static SYMTAB
get_symtab_ancestor (SYMTAB stab, INT index)
{
	SYMTAB curstab = stab;
	while (index > 0) {
		ASSERT(curstab);
		curstab = curstab->parent;
		--index;
	}
	ASSERT(curstab);
	return curstab;
}
/*=============================================+
 * prog_var_error -- Report a run time program error
 *  due to mistyping of a particular variable
 *  node:  [IN]  current parse node
 *  stab:  [IN]  current symbol table (lexical scope)
 *  arg:   [IN]  if non-null, parse node of troublesome argument
 *  val:   [IN]  if non-null, PVALUE of troublesome argument
 *  fmt... [IN]  message
 *
 * Inline debugger is implemented here
 * See vprog_error
 * Created: 2005-06-01, Perry Rapp
 *============================================*/
void
prog_var_error (PNODE node, SYMTAB stab, PNODE arg, PVALUE val, STRING fmt, ...)
{
	va_list args;
	ZSTR zstr;

	va_start(args, fmt);
	zstr = zs_newvf(fmt, args);
	va_end(args);
	prog_var_error_zstr(node, stab, arg, val, zstr);
}
/*====================================================
 * disp_symtab -- Display contents of a symbol table
 *  This is part of the report language debugger
 *==================================================*/
static void
disp_symtab (STRING title, SYMTAB stab)
{
	SYMTAB_ITER symtabit=0;
	INT nels = (stab->tab ? get_table_count(stab->tab) : 0);
	struct dbgsymtab_s sdata;
	if (!nels) return;
	init_dbgsymtab_arrays(&sdata, nels);
	/* Now traverse & print the actual entries into string array
	(sdata.locals) via format_dbgsymtab_val() */
	symtabit = begin_symtab_iter(stab);
	if (symtabit) {
		STRING key=0;
		PVALUE val=0;
		while (next_symtab_entry(symtabit, (CNSTRING *)&key, &val)) {
			format_dbgsymtab_val(key, val, &sdata);
		}
		end_symtab_iter(&symtabit);
	}
	disp_dbgsymtab(title, &sdata);
	free_dbgsymtab_arrays(&sdata);
}
/*====================================================
 * display_dbgsymtab -- Display values in dbgsymtab in list
 *  (allows user to drill into container values)
 * This is used for symbol tables, lists, etc.
 *  title: [IN]  list title
 *  sdata: [IN]  values to display
 *==================================================*/
static void
disp_dbgsymtab (CNSTRING title, struct dbgsymtab_s * sdata)
{
	while (TRUE) {
		PVALUE val = 0;
		INT choice = 0;
		choice = rptui_choose_from_array((STRING)title, sdata->count, sdata->displays);
		if (choice == -1)
			break;
		val = sdata->values[choice];
		if (val)
			disp_pvalue(val);
	}
}
/*====================================================
 * format_dbgsymtab_val -- Display one entry in symbol table
 *  This is part of the report language debugger
 *  key:   [IN]  name of current symbol
 *  val:   [IN]  value of current symbol
 *  sdata: [I/O] structure holding array of values & strings to display
 *==================================================*/
static void
format_dbgsymtab_val (STRING key, PVALUE val, struct dbgsymtab_s * sdata)
{
	ZSTR zval = describe_pvalue(val);
	STRING valstr = zs_str(zval);
	format_dbgsymtab_str(key, valstr, val, sdata);
	zs_free(&zval);
}
/*====================================================
 * format_dbgsymtab_val -- Display one entry in symbol table
 *  This is part of the report language debugger
 *  key:   [IN]  name of current symbol
 *  valstr:   [IN]  string to display for current value
 *  val:   [IN]  value of current symbol (or NULL if none applicable)
 *  sdata: [I/O] structure holding array of values & strings to display
 *==================================================*/
static void
format_dbgsymtab_str (STRING key, STRING valstr, PVALUE val, struct dbgsymtab_s * sdata)
{
	ZSTR zline = zs_newn(80);
	ASSERT(sdata->current < sdata->count);
	zs_setf(zline, "%s: %s", key, valstr);
	sdata->displays[sdata->current] = strsave(zs_str(zline));
	sdata->values[sdata->current] = val;
	++sdata->current;
	zs_free(&zline);
}
/*====================================================
 * disp_pvalue -- Display details of specified pvalue
 *  Drilldown in variable debugger
 *  This is primarily to display contents of container values
 *  val:   [IN]  value to display
 *==================================================*/
static void
disp_pvalue (PVALUE val)
{
	switch (which_pvalue_type(val)) {
		case PGNODE:
			{
				NODE node = pvalue_to_node(val);
				char buffer[256] = "";
				size_t len = sizeof(buffer);
				STRING str = buffer;
				if (ntag(node)) {
					llstrappf(str, len, uu8, "%s: ", ntag(node));
				}
				if (nval(node)) {
					llstrapps(str, len, uu8, nval(node));
				}
				msg_info(str);
			}
			return;
		case PINDI:
		case PFAM:
		case PSOUR:
		case PEVEN:
		case POTHR:
			{
				RECORD rec = pvalue_to_record(val);
				NODE node = nztop(rec);
				size_t len = 128;
				STRING txt = generic_to_list_string(node, NULL, len, " ", NULL, TRUE);
				msg_info(txt);
			}
			return;
		case PLIST:
			{
				LIST list = pvalue_to_list(val);
				disp_list(list);
			}
			return;
		case PTABLE:
			{
				TABLE tab = pvalue_to_table(val);
				disp_table(tab);
			}
			return;
		case PSET:
			{
				INDISEQ seq = pvalue_to_seq(val);
				disp_seq(seq);
			}
			return;
	}
}
/*=============================================+
 * disp_list -- Display list contents
 *  used for drilldown in variable debugger
 *  list:  list to display
 *============================================*/
static void
disp_list (LIST list)
{
	struct dbgsymtab_s sdata;
	INT nels = length_list(list);
	if (!nels) {
		msg_info(_("list is empty"));
		return;
	}
	init_dbgsymtab_arrays(&sdata, nels);

	/* loop thru list building display array */
	{
		LIST_ITER listit = begin_list(list);
		VPTR ptr = 0;
		while (next_list_ptr(listit, &ptr)) {
			PVALUE val = ptr;
			char key[FMT_INT_LEN+1];
			snprintf(key, sizeof(key), FMT_INT, sdata.current+1);
			format_dbgsymtab_val(key, val, &sdata);
		}
		end_list_iter(&listit);
	}

	disp_dbgsymtab(_("LIST contents"), &sdata);

	free_dbgsymtab_arrays(&sdata);
}
/*=============================================+
 * disp_table -- Display table contents
 *  used for drilldown in variable debugger
 *  tab:  table to display
 *============================================*/
static void
disp_table (TABLE tab)
{
	struct dbgsymtab_s sdata;
	INT nels = get_table_count(tab);
	if (!nels) {
		msg_info(_("table is empty"));
		return;
	}
	init_dbgsymtab_arrays(&sdata, nels);

	/* loop thru table building display array */
	{
		TABLE_ITER tabit = begin_table_iter(tab);
		STRING key=0;
		VPTR ptr = 0;
		while (next_table_ptr(tabit, (CNSTRING *)&key, &ptr)) {
			PVALUE val = ptr;
			format_dbgsymtab_val(key, val, &sdata);
		}
		end_table_iter(&tabit);
	}
	

	disp_dbgsymtab(_("TABLE contents"), &sdata);

	free_dbgsymtab_arrays(&sdata);
}
/*=============================================+
 * disp_seq -- Display sequence contents
 *  used for drilldown in variable debugger
 *  seq:  sequence to display
 *============================================*/
static void
disp_seq (INDISEQ seq)
{
	struct dbgsymtab_s sdata;
	INT nels = length_indiseq(seq);
	INT i=0;
	if (!nels) {
		msg_info(_("sequence is empty"));
		return;
	}
	init_dbgsymtab_arrays(&sdata, nels);

	/* loop thru seq building display array */
	for (i=0; i<nels; ++i)
	{
		STRING key=0, name=0;
		PVALUE val = NULL;
		element_indiseq(seq, i, &key, &name);
		format_dbgsymtab_str(key, name, val, &sdata);
	}
	
	disp_dbgsymtab(_("SEQ contents"), &sdata);

	free_dbgsymtab_arrays(&sdata);
}
/*=============================================+
 * prog_error -- Report a run time program error
 *  node:   current parsed node
 *  fmt:    printf style format string
 *  ...:    printf style varargs
 *  See vprog_error
 *============================================*/
void
prog_error (PNODE node, STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprog_error(node, fmt, args);
	va_end(args);
}
/*=============================================+
 * vprog_error -- Report a run time program error
 *  node:        current parsed node
 *  fmt, args:   printf style message
 *  ...:    printf style varargs
 * Prints error to the stdout-style curses window
 * and to the report log (if one was specified in config file)
 * Always includes line number of node, if available
 * Only includes file name if not same as previous error
 * Returns static buffer with one-line description
 *============================================*/
static STRING
vprog_error (PNODE node, STRING fmt, va_list args)
{
	INT num;
	STRING rptfile;
	ZSTR zstr=zs_newn(256);
	static char msgbuff[100];
	if (rpt_cancelled)
		return _("Report cancelled");
	rptfile = getlloptstr("ReportLog", NULL);
	if (node) {
		STRING fname = irptinfo(node)->fullpath;
		INT lineno = iline(node)+1;
		/* Display filename if not same as last error */
		if (!eqstr(vprog_prevfile, fname)) {
			llstrsets(vprog_prevfile, sizeof(vprog_prevfile), uu8, fname);
			zs_apps(zstr, _("Report file: "));
			zs_apps(zstr, fname);
			zs_appc(zstr, '\n');
			vprog_prevline = -1; /* force line number display */
		}
		/* Display line number if not same as last error */
		if (vprog_prevline != lineno) {
			vprog_prevline = lineno;
			if (progparsing)
				zs_appf(zstr, _("Parsing Error at line %d: "), lineno);
			else
				zs_appf(zstr, _("Runtime Error at line %d: "), lineno);
		}
	} else {
		zs_apps(zstr, _("Aborting: "));
	}
	zs_appvf(zstr, fmt, args);
	llwprintf("\n");
	llwprintf(zs_str(zstr));
	++progerror;
	/* if user specified a report error log (in config file) */
	if (rptfile && rptfile[0]) {
		FILE * fp = fopen(rptfile, LLAPPENDTEXT);
		if (fp) {
			if (progerror == 1) {
				LLDATE creation;
				get_current_lldate(&creation);
				fprintf(fp, "\n%s\n", creation.datestr);
			}
			fprintf(fp, "%s", zs_str(zstr));
			fprintf(fp, "\n");
			fclose(fp);
		}
	}
	if ((num = getlloptint("PerErrorDelay", 0)))
		sleep(num);
	zs_free(&zstr);
	return msgbuff;
}
/*=============================================+
 * init_dbgsymtab_arrays -- Initialize contents of dbgsymtab
 *  and create its arrays
 *  sdata: [I/O] dbgsymtab to be initialized
 *  nels:  [IN] number of elements
 *============================================*/
static void
init_dbgsymtab_arrays (struct dbgsymtab_s * sdata, INT nels)
{
	memset(sdata, 0, sizeof(*sdata));
	sdata->count = nels;
	sdata->displays = make_empty_string_table(nels);
	sdata->values = make_empty_pvalue_table(nels);
}
/*=============================================+
 * free_dbgsymtab_arrays -- Clear dbgsymtab after use
 *  frees its arrays and dynamic strings
 *  values are assumed to be borrowed, not to free
 *  sdata: [I/O] dbgsymtab to be cleared
 *============================================*/
static void
free_dbgsymtab_arrays (struct dbgsymtab_s * sdata)
{
	free_array_strings(sdata->count, sdata->displays);
	stdfree(sdata->displays);
	sdata->displays = 0;
	/* do not free sdata.values pointers, they're borrowed */
	stdfree(sdata->values);
	sdata->values = 0;
	sdata->count = 0;
	memset(sdata, 0, sizeof(*sdata));
}
/*=============================================+
 * make_empty_string_table -- Create table of strings
 *  allocate table and initialize all strings to NULL
 *  nels:  [IN] number of elements in table
 * Returns dynamically allocated array
 *============================================*/
static STRING *
make_empty_string_table (INT nels)
{
	STRING * arrd = (STRING *)stdalloc(nels * sizeof(arrd[0]));
	memset(arrd, 0, nels * sizeof(arrd[0]));
	return arrd;
}
/*=============================================+
 * make_empty_pvalue_table -- Create table of pvalue pointers
 *  allocate table and initialize all pointers to NULL
 *  nels:  [IN] number of elements in table
 * Returns dynamically allocated array
 *============================================*/
static PVALUE *
make_empty_pvalue_table (INT nels)
{
	PVALUE * arrd = (PVALUE*)stdalloc(nels * sizeof(arrd[0]));
	memset(arrd, 0, nels * sizeof(arrd[0]));
	return arrd;
}
