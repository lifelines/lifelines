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
#include "parse.h"
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
	STRING * locals;
	INT count;
	INT current;
};

/*********************************************
 * local function prototypes
 *********************************************/

static INT count_symtab_ancestors(SYMTAB stab);
static void disp_symtab(STRING title, SYMTAB stab);
static BOOLEAN disp_symtab_cb(STRING key, PVALUE val, VPTR param);
static SYMTAB get_symtab_ancestor(SYMTAB stab, INT index);
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
	INT n = (stab->tab ? get_table_count(stab->tab) : 0);
	struct dbgsymtab_s sdata;
	INT bytes = n * sizeof(STRING);
	if (!n) return;
	memset(&sdata, 0, sizeof(sdata));
	sdata.count = n;
	sdata.locals = (STRING *)stdalloc(bytes);
	memset(sdata.locals, 0, bytes);
	/* Now traverse & print the actual entries via disp_symtab_cb() */
	symtabit = begin_symtab_iter(stab);
	if (symtabit) {
		STRING key=0;
		PVALUE pval=0;
		while (next_symtab_entry(symtabit, (CNSTRING *)&key, &pval)) {
			disp_symtab_cb(key, pval, &sdata.locals);
		}
		end_symtab_iter(&symtabit);
	}
	/* Title of report debugger's list of local symbols */
	/* TODO: 2003-01-19, we could allow drilldown on lists, tables & sets here */
	rptui_view_array(title, n, sdata.locals);
	free_array_strings(n, sdata.locals);
}
/*====================================================
 * disp_symtab_cb -- Display one entry in symbol table
 *  This is part of the report language debugger
 *  key:   [IN]  name of current symbol
 *  val:   [IN]  value of current symbol
 *  param: [I/O] points to dbgsymtab_s, where list is being printed
 *==================================================*/
static BOOLEAN
disp_symtab_cb (STRING key, PVALUE val, VPTR param)
{
	struct dbgsymtab_s * sdata = (struct dbgsymtab_s *)param;
	ZSTR zline = zs_newn(80), zval;
	ASSERT(sdata->current < sdata->count);
	zval = describe_pvalue(val);
	zs_setf(zline, "%s: %s", key, zs_str(zval));
	zs_free(&zval);
	sdata->locals[sdata->current++] = strsave(zs_str(zline));
	zs_free(&zline);
	return TRUE; /* continue */
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
			fprintf(fp, zs_str(zstr));
			fprintf(fp, "\n");
			fclose(fp);
		}
	}
	if ((num = getlloptint("PerErrorDelay", 0)))
		sleep(num);
	zs_free(&zstr);
	return msgbuff;
}
