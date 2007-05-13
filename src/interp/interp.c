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
 * interp.c -- Interpret program statements
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 16 Aug 93
 *   3.0.0 - 26 Jul 94    3.0.2 - 25 Mar 95
 *   3.0.3 - 22 Sep 95
 *===========================================================*/

#include <time.h>
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
 * global/exported variables
 *********************************************/

/*
 TODO: Move most of these into parseinfo structure
 Perry 2002.07.14
 One problem -- finishrassa closes foutfp, so it may need longer lifetime
 that parseinfo currently has
*/
TABLE gproctab=0, gfunctab=0;
SYMTAB globtab = NULL;
PATHINFO cur_pathinfo = 0;	/* program currently being parsed or run */
FILE *Poutfp = NULL;		/* file to write program output to */
STRING Poutstr = NULL;		/* string to write program output to */
INT Perrors = 0;
LIST Plist = NULL;		/* list of program files still to read */
PNODE Pnode = NULL;		/* node being interpreted */
BOOLEAN explicitvars = FALSE;	/* all vars must be declared */
BOOLEAN rpt_cancelled = FALSE;

/*********************************************
 * external/imported variables
 *********************************************/

extern INT progerror;
extern BOOLEAN progrunning, progparsing;
extern STRING qSwhatrpt;
extern STRING nonint1, nonintx, nonstr1, nonstrx, nullarg1, nonfname1;
extern STRING nonnodstr1, nonind1, nonindx, nonfam1, nonfamx;
extern STRING nonrecx, nonnod1;
extern STRING nonnodx, nonvar1, nonvarx, nonboox, nonlst1, nonlstx;
extern STRING badargs, badargx, nonrecx;
extern STRING qSunsupuniv;

/*********************************************
 * local function prototypes
 *********************************************/

static STRING check_rpt_requires(PACTX pactx, STRING fname);
static void clean_orphaned_rptlocks(void);
static void enqueue_parse_error(const char * fmt, ...);
static BOOLEAN find_program(STRING fname, STRING localdir, STRING *pfull,BOOLEAN include);
static void init_pactx(PACTX pactx);
static BOOLEAN interpret_prog(PNODE begin, SYMTAB stab);
static void parse_file(PACTX pactx, STRING fname, STRING fullpath);
static void print_report_duration(INT duration, INT uiduration);
static void progmessage(MSG_LEVEL level, STRING);
static void remove_tables(PACTX pactx);
static void wipe_pactx(PACTX pactx);

/*********************************************
 * local variables
 *********************************************/

static LIST outstanding_parse_errors = 0;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/


/*====================================+
 * initinterp -- Initialize interpreter
 *===================================*/
void
initinterp (void)
{
	initrassa();
	Perrors = 0;
	rpt_cancelled = FALSE;
	explicitvars = FALSE;
}
/*==================================+
 * finishinterp -- Finish interpreter
 *=================================*/
void
finishinterp (void)
{
	finishrassa();

	/* clean up any orphaned report locks on records */
	clean_orphaned_rptlocks();

	if (progerror) {
		refresh_stdout();
		/* we used to sleep for 5 seconds here */
	}

}
/*================================================================+
 * progmessage -- Display a status message about the report program
 *  level: [IN]  error, info, status (use enum MSG_LEVEL)
 *  msg:   [IN]  string to display (progname is added if there is room)
 *===============================================================*/
static void
progmessage (MSG_LEVEL level, STRING msg)
{
	char buf[120];
	char *ptr=buf;
	INT mylen=sizeof(buf);
	INT maxwidth = msg_width();
	INT msglen = strlen(msg);
	if (maxwidth >= 0 && maxwidth < mylen)
		mylen = maxwidth;
	if (cur_pathinfo && *cur_pathinfo->fullpath && msglen+20 < maxwidth) {
		INT len = 99999;
		if (msg_width() >= 0) {
			len = sizeof(buf)-3-msglen;
		}
		llstrcatn(&ptr, msg, &mylen);
		llstrcatn(&ptr, " [", &mylen);
		llstrcatn(&ptr, compress_path(cur_pathinfo->fullpath, len), &mylen);
		llstrcatn(&ptr, "] ", &mylen);
	} else {
		llstrcatn(&ptr, msg, &mylen);
	}
	msg_output(level, buf);
}
/*=============================================+
 * new_pathinfo -- Return new, filled-out pathinfo object
 *  all memory is newly heap-allocated
 *============================================*/
static PATHINFO
new_pathinfo (STRING fname, STRING fullpath)
{
	PATHINFO pathinfo = (PATHINFO)stdalloc(sizeof(*pathinfo));
	memset(pathinfo, 0, sizeof(*pathinfo));
	pathinfo->fname = strdup(fname);
	pathinfo->fullpath = strdup(fullpath);
	return pathinfo;
}
/*=============================================+
 * new_pathinfo -- Return new, filled-out pathinfo object
 *  all memory is newly heap-allocated
 *============================================*/
static void
delete_pathinfo (PATHINFO * pathinfo)
{
	if (pathinfo && *pathinfo) {
		strfree(&(*pathinfo)->fname);
		strfree(&(*pathinfo)->fullpath);
		stdfree(*pathinfo);
		*pathinfo = 0;
	}
}
/*=============================================+
 * interp_program_list -- Interpret LifeLines program
 *  proc:     [IN]  proc to call
 *  nargs:    [IN]  number of arguments
 *  args:     [IN]  arguments
 *  ifiles:   [IN]  program files
 *  ofile:    [IN]  output file - can be NULL
 *  picklist: [IN]  show user list of existing reports ?
 * returns 0 if it didn't actually run (eg, not found, or no report picked)
 *============================================*/
static INT
interp_program_list (STRING proc, INT nargs, VPTR *args, LIST lifiles
	, STRING ofile, BOOLEAN picklist)
{
	LIST plist=0, donelist=0;
	SYMTAB stab = NULL;
	INT i;
	INT nfiles = length_list(lifiles);
	PNODE first, parm;
	struct tag_pactx pact;
	PACTX pactx = &pact;
	STRING rootfilepath=0;
	INT ranit=0;

	init_pactx(pactx);

	pvalues_begin();

   /* Get the initial list of program files */
	plist = create_list();
	/* list of pathinfos finished */
	donelist = create_list();

	if (nfiles > 0) {
		for (i = 1; i < nfiles+1; i++) {
			STRING fullpath = 0;
			STRING progfile = get_list_element(lifiles, i, NULL);
			if (find_program(progfile, 0, &fullpath,FALSE)) {
				PATHINFO pathinfo = new_pathinfo(progfile, fullpath);
				strfree(&fullpath);
				enqueue_list(plist, pathinfo);
				if (i==1)
					strupdate(&rootfilepath, pathinfo->fullpath);
			} else {
				enqueue_parse_error(_("Report not found: %s "), progfile);
			}
		}
	} else {
		PATHINFO pathinfo = 0;
		STRING fname=0, fullpath=0;
		STRING programsdir = getlloptstr("LLPROGRAMS", ".");
		if (!rptui_ask_for_program(LLREADTEXT, _(qSwhatrpt), &fname, &fullpath
			, programsdir, ".ll", picklist)) {
			if (fname)  {
				/* tried & failed to open report program */
				llwprintf(_("Error: file <%s> not found"), fname);
			}
			strfree(&fname);
			strfree(&fullpath);
			goto interp_program_notfound;
		}
		pathinfo = new_pathinfo(fname, fullpath);
		strfree(&fname);
		strfree(&fullpath);

		strupdate(&rootfilepath, pathinfo->fullpath);
		enqueue_list(plist, pathinfo);
	}

	progparsing = TRUE;

	/* Parse each file in the list -- don't reparse any file */
	/* (paths are resolved before files are enqueued, & stored in pathinfo) */

	gproctab = create_table_obj();
	globtab = create_symtab_global();
	gfunctab = create_table_obj();
	initinterp();

	while (!is_empty_list(plist)) {
		cur_pathinfo = (PATHINFO) dequeue_list(plist);
		if (!in_table(pactx->filetab, cur_pathinfo->fullpath)) {
			STRING str;
			insert_table_obj(pactx->filetab, cur_pathinfo->fullpath, 0);
			Plist = plist;
			parse_file(pactx, cur_pathinfo->fname, cur_pathinfo->fullpath);
			if ((str = check_rpt_requires(pactx, cur_pathinfo->fullpath)) != 0) {
				progmessage(MSG_ERROR, str);
				goto interp_program_exit;
			}
			enqueue_list(donelist, cur_pathinfo);
		} else {
			/* skip references to files we've already parsed */
			delete_pathinfo(&cur_pathinfo);
		}
		cur_pathinfo = 0;
	}
	destroy_list(plist);
	plist=NULL;


	if (outstanding_parse_errors) {
		STRING str;
		FORLIST(outstanding_parse_errors, el)
			str = (STRING)el;
			prog_error(NULL, str);
			++Perrors;
		ENDLIST
		destroy_list(outstanding_parse_errors);
		outstanding_parse_errors=0;
	}

	if (Perrors) {
		progmessage(MSG_ERROR, _("Program contains errors.\n"));
		goto interp_program_exit;
	}

   /* Find top procedure */

	if (!(first = (PNODE) valueof_ptr(get_rptinfo(rootfilepath)->proctab, proc))) {
		progmessage(MSG_ERROR, _("Program needs a starting procedure.\n"));
		goto interp_program_exit;
	}

   /* Open output file if name is provided */

	if (ofile) {
		if (!start_output_file(ofile)) {
			goto interp_program_exit;
		}
	}
	if (Poutfp) setbuf(Poutfp, NULL);

   /* Link arguments to parameters in symbol table */

	parm = (PNODE) iargs(first);
	if (nargs != num_params(parm)) {
		msg_error(_("Proc %s must be called with %d (not %d) parameters."),
			proc, num_params(parm), nargs);
		goto interp_program_exit;
	}
	stab = create_symtab_proc(proc, NULL);
	for (i = 0; i < nargs; i++) {
		insert_symtab(stab, iident(parm), args[0]);
		parm = inext(parm);
	}

   /* Interpret top procedure */
	ranit = 1;
	progparsing = FALSE;
	progrunning = TRUE;
	progerror = 0;
	progmessage(MSG_STATUS, _("Program is running..."));
	ranit = interpret_prog((PNODE) ibody(first), stab);

   /* Clean up and return */

	progrunning = FALSE;
	finishinterp(); /* includes 5 sec delay if errors on-screen */
	if (Poutfp) fclose(Poutfp);
	Poutfp = NULL;

interp_program_exit:

	remove_tables(pactx);
	if (stab) {
		remove_symtab(stab);
		stab = NULL;
	}

interp_program_notfound:

	symbol_tables_end();
	pvalues_end();
	wipe_pactx(pactx);
	xl_free_adhoc_xlats();

	/* kill any orphaned pathinfos */
	while (!is_empty_list(plist)) {
		PATHINFO pathinfo = (PATHINFO)dequeue_list(plist);
		delete_pathinfo(&pathinfo);
	}
	/* Assumption -- pactx->fullpath stays live longer than all pnodes */
	while (!is_empty_list(donelist)) {
		PATHINFO pathinfo = (PATHINFO)dequeue_list(donelist);
		delete_pathinfo(&pathinfo);
	}
	strfree(&rootfilepath);
	destroy_list(donelist);
	destroy_list(plist);
	return ranit;
}
/*===============================================
 * interpret_prog -- execute a report program
 *=============================================*/
static BOOLEAN
interpret_prog (PNODE begin, SYMTAB stab)
{
	PVALUE dummy=0;
	INT rtn = interpret(begin, stab, &dummy);

	delete_pvalue(dummy);
	dummy=0;

	switch(rtn) {
	case INTOKAY:
	case INTRETURN:
		progmessage(MSG_INFO, _("Program was run successfully.\n"));
		return TRUE;
	default:
		if (rpt_cancelled) {
			progmessage(MSG_STATUS, _("Program was cancelled.\n"));
		} else
			progmessage(MSG_STATUS, _("Program was not run because of errors.\n"));
		return FALSE;
	}
}
/*===============================================
 * init_pactx -- initialize global parsing context
 *=============================================*/
static void
init_pactx (PACTX pactx)
{
	memset(pactx, 0, sizeof(*pactx));
	pactx->filetab = create_table_obj(); /* table of tables */
}
/*===============================================
 * wipe_pactx -- destroy global parsing context
 *=============================================*/
static void
wipe_pactx (PACTX pactx)
{
	destroy_table(pactx->filetab);
	pactx->filetab=NULL;
	memset(pactx, 0, sizeof(*pactx));
}
/*===========================================+
 * remove_tables -- Remove interpreter's tables
 *==========================================*/
static void
remove_tables (PACTX pactx)
{
	pactx=pactx; /* unused */
	destroy_table(gproctab);
	gproctab=NULL;
	remove_symtab(globtab);
	globtab = NULL;
	destroy_table(gfunctab);
	gfunctab=NULL;
}
/*======================================+
 * find_program -- search for program file
 *  fname: [IN]  filename desired
 *  pfull: [OUT] full path found (stdalloc'd)
 * Returns TRUE if found
 *=====================================*/
static BOOLEAN
find_program (STRING fname, STRING localdir, STRING *pfull,BOOLEAN include)
{
	STRING programsdir = getlloptstr("LLPROGRAMS", ".");
	FILE * fp = 0;
	ZSTR zstr=zs_new();
	BOOLEAN rtn=FALSE;
	if (!fname || *fname == 0) goto end_find_program;
	/* prefer local dir, so prefix path with localdir */
	if (localdir && localdir[0]) {
		zs_sets(zstr, localdir);
		zs_apps(zstr, LLSTRPATHSEPARATOR);
	}
	zs_apps(zstr, programsdir);
	if (include) {
	    fp = fopenpath(fname, LLREADTEXT, zs_str(zstr), ".li", uu8, pfull);
	    if (fp) {
		    fclose(fp);
		    rtn = TRUE;
		    goto end_find_program;
	    }
	}
	fp = fopenpath(fname, LLREADTEXT, zs_str(zstr), ".ll", uu8, pfull);
	if (fp) {
		fclose(fp);
		rtn = TRUE;
	}

end_find_program:
	zs_free(&zstr);
	return rtn;
}
/*======================================+
 * parse_file -- Parse single program file
 *  pactx: [I/O] pointer to global parsing context
 *  ifile: [IN]  file to parse
 * Parse file (yyparse may wind up adding entries to plist, via include statements)
 *=====================================*/
static void
parse_file (PACTX pactx, STRING fname, STRING fullpath)
{
	STRING unistr=0;

	ASSERT(!pactx->Pinfp);
	if (!fullpath || !fullpath[0]) return;
	pactx->Pinfp = fopen(fullpath, LLREADTEXT);
	if (!pactx->Pinfp) {
		llwprintf(_("Error: file <%s> not found: %s\n"), fname, fullpath);
		Perrors++;
		return;
	}

	if ((unistr=check_file_for_unicode(pactx->Pinfp)) && !eqstr(unistr, "UTF-8")) {
		msg_error(_(qSunsupuniv), unistr);
		Perrors++;
		return;
	}

	/* Assumption -- pactx->fullpath stays live longer than all pnodes */
	pactx->ifile = fname;
	pactx->fullpath = fullpath;
	pactx->lineno = 0;
	pactx->charpos = 0;

	yyparse(pactx);
	
	closefp(&pactx->Pinfp);
	pactx->ifile = 0;
	pactx->fullpath = 0;
	pactx->lineno = 0;
	pactx->charpos = 0;
}
/*====================================+
 * report_duration -- print report duration
 *===================================*/
static void
print_report_duration (INT duration, INT uiduration)
{
	ZSTR zt1=approx_time(duration-uiduration), zt2=approx_time(uiduration);
	llwprintf(_("\nReport duration %s (ui duration %s)\n")
		, zs_str(zt1), zs_str(zt2));
	zs_free(&zt1);
	zs_free(&zt2);
}
/*====================================+
 * interp_main -- Interpreter main proc
 *  picklist: [IN]  
 *  ifiles:   [IN]  program files
 *  ofile:    [IN]  output file - can be NULL
 *  picklist: [IN]  show user list of existing reports ?
 *  timing:   [IN]  show report elapsed time info ?
 *===================================*/
void
interp_main (LIST lifiles, STRING ofile, BOOLEAN picklist, BOOLEAN timing)
{
	time_t begin = time(NULL);
	int elapsed, uitime;
	int ranit=0;
	/* whilst still in uilocale, check if we need to reload report strings
	(in case first time or uilocale changed) */
	interp_load_lang();
	prog_trace = FALSE; /* clear report debug flag */
	init_debugger();
	rptui_init(); /* clear ui time counter */

	rptlocale();
	ranit = interp_program_list("main", 0, NULL, lifiles, ofile, picklist);
	uilocale();
	elapsed = time(NULL) - begin;
	uitime = rptui_elapsed();

	if (ranit && timing)
		print_report_duration(elapsed, uitime);
	
	/*
	TO DO: unlock all cache elements (2001/03/17, Perry)
	in case any were left locked by report
	*/
}
/*======================================
 * interpret -- Interpret statement list
 * PNODE node:   first node to interpret
 * TABLE stab:   current symbol table
 * PVALUE *pval: possible return value
 *====================================*/
INTERPTYPE
interpret (PNODE node, SYMTAB stab, PVALUE *pval)
{
	STRING str;
	BOOLEAN eflg = FALSE;
	INTERPTYPE irc;
	PVALUE val;

	*pval = NULL;

	while (node) {
		Pnode = node;
		if (prog_trace) {
			trace_out("d%d: ", iline(node)+1);
			trace_pnode(node);
			trace_endl();
		}
		switch (itype(node)) {
		case ISCONS:
			poutput(pvalue_to_string(ivalue(node)), &eflg);
			if (eflg)
				goto interp_fail;
			break;
		case IIDENT:
			val = eval_and_coerce(PSTRING, node, stab, &eflg);
			if (eflg) {
				prog_error(node, _("identifier: %s should be a string\n"),
				    iident(node));
				goto interp_fail;
			}
			str = pvalue_to_string(val);
			if (str) {
				poutput(str, &eflg);
				if (eflg) {
					goto interp_fail;
				}
			}
			delete_pvalue(val);
			break;
		case IBCALL:
			val = evaluate_func(node, stab, &eflg);
			if (eflg) {
				goto interp_fail;
			}
			if (!val) break;
			if (which_pvalue_type(val) == PSTRING) {
				str = pvalue_to_string(val);
				if (str) {
					poutput(str, &eflg);
					if (eflg)
						goto interp_fail;
				}
			}
			delete_pvalue(val);
			break;
		case IFCALL:
			val = evaluate_ufunc(node, stab, &eflg);
			if (eflg) {
				goto interp_fail;
			}
			if (!val) break;
			if (which_pvalue_type(val) == PSTRING) {
				str = pvalue_to_string(val);
				if (str) {
					poutput(str, &eflg);
					if (eflg)
						goto interp_fail;
				}
			}
			delete_pvalue(val);
			break;
		case IPDEFN:
			FATAL();
		case ICHILDREN:
			switch (irc = interp_children(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case IFAMILYSPOUSES:
			switch (irc = interp_familyspouses(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case ISPOUSES:
			switch (irc = interp_spouses(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case IFAMILIES:
			switch (irc = interp_families(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case IFATHS:
			switch (irc = interp_fathers(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case IMOTHS:
			switch (irc = interp_mothers(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case IFAMCS:
			switch (irc = interp_parents(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case ISET:
			switch (irc = interp_indisetloop(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case IINDI:
			switch (irc = interp_forindi(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case IFAM:
			switch (irc = interp_forfam(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case ISOUR:
			switch (irc = interp_forsour(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case IEVEN:
			switch (irc = interp_foreven(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case IOTHR:
			switch (irc = interp_forothr(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case ILIST:
			switch (irc = interp_forlist(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case INOTES:
			switch (irc = interp_fornotes(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case INODES:
			switch (irc = interp_fornodes(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case ITRAV:
			switch (irc = interp_traverse(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case IIF:
			switch (irc = interp_if(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case IWHILE:
			switch (irc = interp_while(node, stab, pval)) {
			case INTOKAY:
			case INTBREAK:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case IPCALL:
			switch (irc = interp_call(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
				goto interp_fail;
			default:
				return irc;
			}
			break;
		case IBREAK:
			return INTBREAK;
		case ICONTINUE:
			return INTCONTINUE;
		case IRETURN:
			if (iargs(node))
				*pval = evaluate(iargs(node), stab, &eflg);
			if (eflg && getlloptint("FullReportCallStack", 0) > 0)
				prog_error(node, "in return statement");
			return INTRETURN;
		default:
			llwprintf("itype(node) is %d\n", itype(node));
			llwprintf("HUH, HUH, HUH, HUNH!\n");
			goto interp_fail;
		}
		node = inext(node);
	}
	return TRUE;

interp_fail:
	if (getlloptint("FullReportCallStack", 0) > 0) {
		llwprintf("e%d: ", iline(node)+1);
		debug_show_one_pnode(node);
		llwprintf("\n");
	}
	return INTERROR;
}
/*========================================+
 * interp_children -- Interpret child loop
 *  usage: children(INDI,INDI_V,INT_V) {...}
 *=======================================*/
INTERPTYPE
interp_children (PNODE node, SYMTAB stab, PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INT nchil;
	CACHEEL fcel, cel;
	INTERPTYPE irc;
	PVALUE val;
	NODE fam = (NODE) eval_fam(iloopexp(node), stab, &eflg, &fcel);
	if (eflg) {
		prog_error(node, nonfamx, "children", "1");
		return INTERROR;
	}
	if (fam && nestr(ntag(fam), "FAM")) {
		prog_error(node, badargx, "children", "1");
		return INTERROR;
	}
	if (!fam) return INTOKAY;
	lock_cache(fcel);
	FORCHILDRENx(fam, chil, nchil)
		val = create_pvalue_from_indi(chil);
		insert_symtab(stab, ichild(node), val);
		insert_symtab(stab, inum(node), create_pvalue_from_int(nchil));
		/* val should be real person, because it came from FORCHILDREN */
		cel = pvalue_to_cel(val);
		lock_cache(cel);
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(cel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto aloop;
		default:
			goto aleave;
		}
aloop:	;
	ENDCHILDRENx
	irc = INTOKAY;
aleave:
	delete_symtab_element(stab, ichild(node));
	delete_symtab_element(stab, inum(node));
	unlock_cache(fcel);
	return irc;
}
/*========================================+
 * interp_familyspouses -- Interpret familyspouses loop
 *  usage: familyspouses(FAM,INDI_V,INT_V) {...}
 *=======================================*/
INTERPTYPE
interp_familyspouses (PNODE node, SYMTAB stab, PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INT nspouse;
	CACHEEL fcel, cel;
	INTERPTYPE irc;
	PVALUE val;
	NODE fam = (NODE) eval_fam(iloopexp(node), stab, &eflg, &fcel);
	if (eflg) {
		prog_error(node, nonfamx, "family spouses", "1");
		return INTERROR;
	}
	if (fam && nestr(ntag(fam), "FAM")) {
		prog_error(node, badargx, "family spouses", "1");
		return INTERROR;
	}
	if (!fam) return INTOKAY;
	lock_cache(fcel);
	FORFAMSPOUSES(fam, spouse, nspouse)
		val = create_pvalue_from_indi(spouse);
		insert_symtab(stab, ichild(node), val);
		insert_symtab(stab, inum(node), create_pvalue_from_int(nspouse));
		/* val should be real person, because it came from FORFAMSPOUSES */
		cel = pvalue_to_cel(val);
		lock_cache(cel);
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(cel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto aloop;
		default:
			goto aleave;
		}
aloop:	;
	ENDFAMSPOUSES
	irc = INTOKAY;
aleave:
	delete_symtab_element(stab, ichild(node));
	delete_symtab_element(stab, inum(node));
	unlock_cache(fcel);
	return irc;
}
/*==============================================+
 * interp_spouses -- Interpret spouse loop
 *  usage: spouses(INDI,INDI_V,FAM_V,INT_V) {...}
 *=============================================*/
INTERPTYPE
interp_spouses (PNODE node, SYMTAB stab, PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INT nspouses;
	CACHEEL icel, scel, fcel;
	INTERPTYPE irc;
	PVALUE sval, fval, nval;
	NODE indi = (NODE) eval_indi(iloopexp(node), stab, &eflg, &icel);
	if (eflg) {
		prog_error(node, nonindx, "spouses", "1");
		return INTERROR;
	}
	if (indi && nestr(ntag(indi), "INDI")) {
		prog_error(node, badargx, "spouses", "1");
		return INTERROR;
	}
	if (!indi) return TRUE;
	lock_cache(icel);
	FORSPOUSES(indi, spouse, fam, nspouses)
		sval = create_pvalue_from_indi(spouse);
		insert_symtab(stab, ispouse(node), sval);
		fval = create_pvalue_from_fam(fam);
		insert_symtab(stab, ifamily(node), fval);
		nval = create_pvalue_from_int(nspouses);
		insert_symtab(stab, inum(node), nval);
		/* sval should be real person, because it came from FORSPOUSES */
		scel = pvalue_to_cel(sval);
		/* fval should be real person, because it came from FORSPOUSES */
		fcel = pvalue_to_cel(fval);
		lock_cache(scel);
		lock_cache(fcel);
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(scel);
		unlock_cache(fcel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto bloop;
		default:
			goto bleave;
		}
bloop:	;
	ENDSPOUSES
	irc = INTOKAY;
bleave:
	delete_symtab_element(stab, ispouse(node));
	delete_symtab_element(stab, ifamily(node));
	delete_symtab_element(stab, inum(node));
	unlock_cache(icel);
	return irc;
}
/*===============================================+
 * interp_families -- Interpret family loop
 *  usage: families(INDI,FAM_V,INDI_V,INT_V) {...}
 * 2001/03/17 Revised by Perry Rapp
 *  to call insert_symtab_pvalue, get_cel_from_pvalue
 *  to delete its loop pvalues when finished
 *==============================================*/
INTERPTYPE
interp_families (PNODE node, SYMTAB stab, PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INT nfams;
	CACHEEL icel, fcel, scel;
	INTERPTYPE irc;
	PVALUE fval, sval, nval;
	NODE indi = (NODE) eval_indi(iloopexp(node), stab, &eflg, &icel);
	if (eflg) {
		prog_error(node, nonindx, "families", "1");
		return INTERROR;
	}
	if (indi && nestr(ntag(indi), "INDI")) {
		prog_error(node, badargx, "families", "1");
		return INTERROR;
	}
	if (!indi) return INTOKAY;
	lock_cache(icel);
	FORFAMSS(indi, fam, spouse, nfams)
		fval = create_pvalue_from_fam(fam);
		insert_symtab(stab, ifamily(node), fval);
		sval = create_pvalue_from_indi(spouse);
		insert_symtab(stab, ispouse(node), sval);
		nval = create_pvalue_from_int(nfams);
		insert_symtab(stab, inum(node), nval);
		/* fval should be real person, because it came from FORFAMSS */
		fcel = pvalue_to_cel(fval);
		/* sval may not be a person -- so scel may be NULL */
		scel = pvalue_to_cel(sval);
		lock_cache(fcel);
		if (scel) lock_cache(scel);
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(fcel);
		if (scel) unlock_cache(scel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto cloop;
		default:
			goto cleave;
		}
cloop:	;
	ENDFAMSS
	irc = INTOKAY;
cleave:
	delete_symtab_element(stab, ifamily(node));
	delete_symtab_element(stab, ispouse(node));
	delete_symtab_element(stab, inum(node));
	unlock_cache(icel);
	return irc;
}
/*========================================+
 * interp_fathers -- Interpret fathers loop
 * 2001/03/17 Revised by Perry Rapp
 *  to call insert_symtab_pvalue, get_cel_from_pvalue
 *  to delete its loop pvalues when finished
 *=======================================*/
INTERPTYPE
interp_fathers (PNODE node, SYMTAB stab, PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INT nfams;
	INT ncount = 1;
	CACHEEL icel, fcel, scel;
	INTERPTYPE irc;
	PVALUE sval, fval;
	NODE indi = (NODE) eval_indi(iloopexp(node), stab, &eflg, &icel);
	if (eflg) {
		prog_error(node, nonindx, "fathers", "1");
		return INTERROR;
	}
	if (indi && nestr(ntag(indi), "INDI")) {
		prog_error(node, badargx, "fathers", "1");
		return INTERROR;
	}
	if (!indi) return TRUE;
	lock_cache(icel);
	insert_symtab(stab, inum(node), create_pvalue_from_int(0));
	FORFAMCS(indi, fam, husb, wife, nfams)
		sval = create_pvalue_from_indi(husb);
		scel = pvalue_to_cel(sval);
		if (!scel) goto dloop;
		fval = create_pvalue_from_fam(fam);
		fcel = pvalue_to_cel(fval);
		insert_symtab(stab, ifamily(node), fval);
		insert_symtab(stab, iiparent(node), create_pvalue_from_cel(PINDI, scel));
		insert_symtab(stab, inum(node), create_pvalue_from_int(ncount++));
		lock_cache(fcel);
		lock_cache(scel);
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(fcel);
		unlock_cache(scel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			irc = INTOKAY;
			goto dloop;
		default:
			goto dleave;
		}
dloop:	;
	ENDFAMCS
	irc = INTOKAY;
dleave:
	delete_symtab_element(stab, ifamily(node));
	delete_symtab_element(stab, iiparent(node));
	delete_symtab_element(stab, inum(node));
	unlock_cache(icel);
	return irc;
}
/*========================================+
 * interp_mothers -- Interpret mothers loop
 * 2001/03/18 Revised by Perry Rapp
 *  to call insert_symtab_pvalue, get_cel_from_pvalue
 *  to delete its loop pvalues when finished
 *=======================================*/
INTERPTYPE
interp_mothers (PNODE node, SYMTAB stab, PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INT nfams;
	INT ncount = 1;
	CACHEEL icel, fcel, scel;
	INTERPTYPE irc;
	PVALUE sval, fval;
	NODE indi = (NODE) eval_indi(iloopexp(node), stab, &eflg, &icel);
	if (eflg) {
		prog_error(node, nonindx, "mothers", "1");
		return INTERROR;
	}
	if (indi && nestr(ntag(indi), "INDI")) {
		prog_error(node, badargx, "mothers", "1");
		return INTERROR;
	}
	if (!indi) return TRUE;
	lock_cache(icel);
	insert_symtab(stab, inum(node), create_pvalue_from_int(0));
	FORFAMCS(indi, fam, husb, wife, nfams)
		sval = create_pvalue_from_indi(wife);
		scel = pvalue_to_cel(sval);
		if (!scel) goto eloop;
		fval = create_pvalue_from_fam(fam);
		fcel = pvalue_to_cel(fval);
		insert_symtab(stab, ifamily(node), fval);
		insert_symtab(stab, iiparent(node), create_pvalue_from_cel(PINDI, scel));
		insert_symtab(stab, inum(node), create_pvalue_from_int(ncount++));
		lock_cache(fcel);
		lock_cache(scel);
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(fcel);
		unlock_cache(scel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto eloop;
		default:
			goto eleave;
		}
eloop:	;
	ENDFAMCS
	irc = INTOKAY;
eleave:
	delete_symtab_element(stab, ifamily(node));
	delete_symtab_element(stab, iiparent(node));
	delete_symtab_element(stab, inum(node));
	unlock_cache(icel);
	return irc;
}
/*========================================+
 * interp_parents -- Interpret parents loop
 * 2001/03/18 Revised by Perry Rapp
 *  to call insert_symtab_pvalue, get_cel_from_pvalue
 *  to delete its loop pvalues when finished
 *=======================================*/
INTERPTYPE
interp_parents (PNODE node, SYMTAB stab, PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INT nfams;
	CACHEEL icel, fcel;
	INTERPTYPE irc;
	PVALUE fval;
	NODE indi = (NODE) eval_indi(iloopexp(node), stab, &eflg, &icel);
	if (eflg) {
		prog_error(node, nonindx, "parents", "1");
		return INTERROR;
	}
	if (indi && nestr(ntag(indi), "INDI")) {
		prog_error(node, badargx, "parents", "1");
		return INTERROR;
	}
	if (!indi) return TRUE;
	lock_cache(icel);
	FORFAMCS(indi, fam, husb, wife, nfams)
		fval = create_pvalue_from_fam(fam);
		insert_symtab(stab, ifamily(node), fval);
		insert_symtab(stab, inum(node), create_pvalue_from_int(nfams));
		fcel = pvalue_to_cel(fval);
		lock_cache(fcel);
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(fcel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto floop;
		default:
			goto fleave;
		}
floop:	;
	ENDFAMCS
	irc = INTOKAY;
fleave:
	delete_symtab_element(stab, ifamily(node));
	delete_symtab_element(stab, inum(node));
	unlock_cache(icel);
	return INTOKAY;
}
/*=======================================
 * interp_fornotes -- Interpret NOTE loop
 *=====================================*/
INTERPTYPE
interp_fornotes (PNODE node, SYMTAB stab, PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INTERPTYPE irc;
	NODE root;
	PVALUE val = eval_and_coerce(PGNODE, iloopexp(node), stab, &eflg);
	if (eflg) {
		prog_error(node, nonrecx, "fornotes", "1");
		return INTERROR;
	}
	root = pvalue_to_node(val);
	delete_pvalue(val);
	if (!root) return INTOKAY;
	FORTAGVALUES(root, "NOTE", sub, vstring)
		insert_symtab(stab, ielement(node), create_pvalue_from_string(vstring));
		irc = interpret((PNODE) ibody(node), stab, pval);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto gloop;
		default:
			goto gleave;
		}
gloop:      ;
	ENDTAGVALUES
	irc = INTOKAY;
gleave:
	delete_symtab_element(stab, ielement(node));
	return irc;
}
/*==========================================+
 * interp_fornodes -- Interpret fornodes loop
 *  usage: fornodes(NODE,NODE_V) {...}
 * 2001/03/19 Revised by Perry Rapp
 *  to delete its loop pvalue when finished
 *=========================================*/
INTERPTYPE
interp_fornodes (PNODE node, SYMTAB stab, PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INTERPTYPE irc;
	NODE sub, root=NULL;
	PVALUE val = eval_and_coerce(PGNODE, iloopexp(node), stab, &eflg);
	if (eflg) {
		prog_error(node, nonrecx, "fornodes", "1");
		return INTERROR;
	}
	root = pvalue_to_node(val);
	delete_pvalue(val);
	if (!root) return INTOKAY;
	sub = nchild(root);
	while (sub) {
		insert_symtab(stab, ielement(node), create_pvalue_from_node(sub));
		irc = interpret((PNODE) ibody(node), stab, pval);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			sub = nsibling(sub);
			goto hloop;
		default:
			goto hleave;
		}
hloop: ;
	}
	irc = INTOKAY;
hleave:
	delete_symtab_element(stab, ielement(node));
	return irc;
}
/*========================================+
 * printkey -- Make key from keynum
 *=======================================*/
#ifdef UNUSED_CODE
static void
printkey (STRING key, char type, INT keynum)
{
	if (keynum>9999999 || keynum<0)
		keynum=0;
	sprintf(key, "%c%d", type, keynum);
}
#endif
/*========================================+
 * interp_forindi -- Interpret forindi loop
 *  usage: forindi(INDI_V,INT_V) {...}
 * 2001/03/18 Revised by Perry Rapp
 *  to call create_pvalue_from_indi_keynum, get_cel_from_pvalue
 *  to delete its loop pvalues when finished
 *=======================================*/
INTERPTYPE
interp_forindi (PNODE node, SYMTAB stab, PVALUE *pval)
{
	CACHEEL icel=NULL;
	INTERPTYPE irc;
	PVALUE ival=NULL;
	INT count = 0;
	INT icount = 0;
	insert_symtab(stab, inum(node), create_pvalue_from_int(0));
	while (TRUE) {
		count = xref_nexti(count);
		if (!count) {
			irc = INTOKAY;
			goto ileave;
		}
		ival = create_pvalue_from_indi_keynum(count);
		icel = pvalue_to_cel(ival);
		if (!icel) { /* apparently missing record */
			delete_pvalue(ival);
			continue;
		}
		icount++;
		lock_cache(icel); /* keep current indi in cache during loop body */
		/* set loop variables */
		insert_symtab(stab, ielement(node), ival);
		insert_symtab(stab, inum(node), create_pvalue_from_int(icount));
		/* execute loop body */
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(icel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			continue;
		default:
			goto ileave;
		}
	}
ileave:
	delete_symtab_element(stab, ielement(node));
	delete_symtab_element(stab, inum(node));
	return irc;
}
/*========================================+
 * interp_forsour -- Interpret forsour loop
 *  usage: forsour(SOUR_V,INT_V) {...}
 * 2001/03/20 Revised by Perry Rapp
 *  to call create_pvalue_from_indi_keynum, get_cel_from_pvalue
 *  to delete its loop pvalues when finished
 *=======================================*/
INTERPTYPE
interp_forsour (PNODE node, SYMTAB stab, PVALUE *pval)
{
	CACHEEL scel=NULL;
	INTERPTYPE irc;
	PVALUE sval=NULL;
	INT count = 0;
	INT scount = 0;
	insert_symtab(stab, inum(node), create_pvalue_from_int(0));
	while (TRUE) {
		count = xref_nexts(count);
		if (!count) {
			irc = INTOKAY;
			goto sourleave;
		}
		sval = create_pvalue_from_sour_keynum(count);
		scel = pvalue_to_cel(sval);
		if (!scel) { /* apparently missing record */
			delete_pvalue(sval);
			continue;
		}
		scount++;
		lock_cache(scel); /* keep current source in cache during loop body */
		/* set loop variables */
		insert_symtab(stab, ielement(node), sval);
		insert_symtab(stab, inum(node), create_pvalue_from_int(scount));
		/* execute loop body */
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(scel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			continue;
		default:
			goto sourleave;
		}
	}
sourleave:
	/* remove loop variables from symbol table */
	delete_symtab_element(stab, ielement(node));
	delete_symtab_element(stab, inum(node));
	return irc;
}
/*========================================+
 * interp_foreven -- Interpret foreven loop
 *  usage: foreven(EVEN_V,INT_V) {...}
 *=======================================*/
INTERPTYPE
interp_foreven (PNODE node, SYMTAB stab, PVALUE *pval)
{
	CACHEEL ecel=NULL;
	INTERPTYPE irc;
	PVALUE eval=NULL;
	INT count = 0;
	INT ecount = 0;
	insert_symtab(stab, inum(node), create_pvalue_from_int(count));
	while (TRUE) {
		count = xref_nexte(count);
		if (!count) {
			irc = INTOKAY;
			goto evenleave;
		}
		eval = create_pvalue_from_even_keynum(count);
		ecel = pvalue_to_cel(eval);
		if (!ecel) { /* apparently missing record */
			delete_pvalue(eval);
			continue;
		}
		ecount++;
		lock_cache(ecel); /* keep current event in cache during loop body */
		/* set loop variables */
		insert_symtab(stab, ielement(node), eval);
		insert_symtab(stab, inum(node), create_pvalue_from_int(ecount));
		/* execute loop body */
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(ecel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			continue;
		default:
			goto evenleave;
		}
	}
evenleave:
	/* remove loop variables from symbol table */
	delete_symtab_element(stab, ielement(node));
	delete_symtab_element(stab, inum(node));
	return irc;
}
/*========================================+
 * interp_forothr -- Interpret forothr loop
 *  usage: forothr(OTHR_V,INT_V) {...}
 *=======================================*/
INTERPTYPE
interp_forothr (PNODE node, SYMTAB stab, PVALUE *pval)
{
	CACHEEL xcel;
	INTERPTYPE irc;
	PVALUE xval;
	INT count = 0;
	INT xcount = 0;
	insert_symtab(stab, inum(node), create_pvalue_from_int(0));
	while (TRUE) {
		count = xref_nextx(count);
		if (!count) {
			irc = INTOKAY;
			goto othrleave;
		}
		xval = create_pvalue_from_othr_keynum(count);
		xcel = pvalue_to_cel(xval);
		if (!xcel) { /* apparently missing record */
			delete_pvalue(xval);
			continue;
		}
		xcount++;
		lock_cache(xcel); /* keep current source in cache during loop body */
		/* set loop variables */
		insert_symtab(stab, ielement(node), xval);
		insert_symtab(stab, inum(node), create_pvalue_from_int(xcount));
		/* execute loop body */
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(xcel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			continue;
		default:
			goto othrleave;
		}
	}
othrleave:
	delete_symtab_element(stab, ielement(node));
	delete_symtab_element(stab, inum(node));
	return irc;
}
/*======================================+
 * interp_forfam -- Interpret forfam loop
 *  usage: forfam(FAM_V,INT_V) {...}
 *=====================================*/
INTERPTYPE
interp_forfam (PNODE node, SYMTAB stab, PVALUE *pval)
{
	CACHEEL fcel=NULL;
	INTERPTYPE irc;
	PVALUE fval=NULL;
	INT count = 0;
	INT fcount = 0;
	insert_symtab(stab, inum(node), create_pvalue_from_int(0));
	while (TRUE) {
		count = xref_nextf(count);
		if (!count) {
			irc = INTOKAY;
			goto mleave;
		}
		fval = create_pvalue_from_fam_keynum(count);
		fcel = pvalue_to_cel(fval);
		if (!fcel) { /* apparently missing record */
			delete_pvalue(fval);
			continue;
		}
		fcount++;
		lock_cache(fcel);
		insert_symtab(stab, ielement(node), fval);
		insert_symtab(stab, inum(node), create_pvalue_from_int(fcount));
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(fcel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			continue;
		default:
			goto mleave;
		}
	}
mleave:
	delete_symtab_element(stab, ielement(node));
	delete_symtab_element(stab, inum(node));
	return irc;
}
/*============================================+
 * interp_indisetloop -- Interpret indiset loop
 * 2001/03/21 Revised by Perry Rapp
 *  to delete its loop pvalues when finished
 *===========================================*/
INTERPTYPE
interp_indisetloop (PNODE node, SYMTAB stab, PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INTERPTYPE irc;
	PVALUE indival=0, loopval=0;
	INDISEQ seq = NULL;
	PVALUE val = evaluate(iloopexp(node), stab, &eflg);
	if (eflg || !val || ptype(val) != PSET) {
		prog_error(node, "1st arg to forindiset must be set expr");
		return INTERROR;
	}
	seq = pvalue_to_seq(val);
	if (!seq) {
		delete_pvalue(val); /* delete temp evaluated val - may destruct seq */
		return INTOKAY;
	}
	/* can't delete val until we're done with seq */
	/* initialize counter */
	insert_symtab(stab, inum(node), create_pvalue_from_int(0));
	FORINDISEQ(seq, el, ncount)
		/* put current indi in symbol table */
		indival = create_pvalue_from_indi_key(element_skey(el));
		insert_symtab(stab, ielement(node), indival);
		/* put current indi's value in symbol table */
		loopval = element_pval(el);
		if (loopval)
			loopval = copy_pvalue(loopval);
		else
			loopval = create_pvalue_any();
		insert_symtab(stab, ivalvar(node), loopval);
		/* put counter in symbol table */
		insert_symtab(stab, inum(node), create_pvalue_from_int(ncount + 1));
		switch (irc = interpret((PNODE) ibody(node), stab, pval)) {
		case INTCONTINUE:
		case INTOKAY:
			goto hloop;
		default:
			goto hleave;
		}
hloop:	;
	ENDINDISEQ
	irc = INTOKAY;
hleave:
	delete_pvalue(val); /* delete temp evaluated val - may destruct seq */
	delete_symtab_element(stab, ielement(node)); /* remove indi */
	delete_symtab_element(stab, ivalvar(node)); /* remove indi's value */
	delete_symtab_element(stab, inum(node)); /* remove counter */
	return irc;
}
/*=====================================+
 * interp_forlist -- Interpret list loop
 * 2001/03/21 Revised by Perry Rapp
 *  to delete its loop pvalues when finished
 *====================================*/
INTERPTYPE
interp_forlist (PNODE node, SYMTAB stab, PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INTERPTYPE irc;
	INT ncount = 1;
	LIST list;
	PVALUE val = eval_and_coerce(PLIST, iloopexp(node), stab, &eflg);
	if (eflg || !val || ptype(val) != PLIST) {
		prog_error(node, "1st arg to forlist is not a list");
		return INTERROR;
	}
	list = pvalue_to_list(val);
	/* can't delete val until we're done with list */
	if (!list) {
		delete_pvalue(val); /* delete temp evaluated val - may destruct list */
		prog_error(node, "1st arg to forlist is in error");
		return INTERROR;
	}
	insert_symtab(stab, inum(node), create_pvalue_from_int(0));
	FORLIST(list, el)
		/* insert/update current element in symbol table */
		insert_symtab(stab, ielement(node), copy_pvalue(el));
		/* insert/update counter in symbol table */
		insert_symtab(stab, inum(node), create_pvalue_from_int(ncount++));
		switch (irc = interpret((PNODE) ibody(node), stab, pval)) {
		case INTCONTINUE:
		case INTOKAY:
			goto iloop;
		default:
			STOPLIST
			goto ileave;
		}
iloop:	;
	ENDLIST
	irc = INTOKAY;
ileave:
	delete_pvalue(val); /* delete temp evaluated val - may destruct list */
	/* remove element & counter from symbol table */
	delete_symtab_element(stab, ielement(node));
	delete_symtab_element(stab, inum(node));
	return irc;
}
/*===================================+
 * interp_if -- Interpret if structure
 *==================================*/
INTERPTYPE
interp_if (PNODE node, SYMTAB stab, PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	BOOLEAN cond = evaluate_cond(icond(node), stab, &eflg);
	if (eflg) return INTERROR;
	if (cond) return interpret((PNODE) ithen(node), stab, pval);
	if (ielse(node)) return interpret((PNODE) ielse(node), stab, pval);
	return INTOKAY;
}
/*=========================================+
 * interp_while -- Interpret while structure
 *========================================*/
INTERPTYPE
interp_while (PNODE node, SYMTAB stab, PVALUE *pval)
{
	BOOLEAN eflg = FALSE, cond;
	INTERPTYPE irc;
	while (TRUE) {
		cond = evaluate_cond(icond(node), stab, &eflg);
		if (eflg) return INTERROR;
		if (!cond) return INTOKAY;
		switch (irc = interpret((PNODE) ibody(node), stab, pval)) {
		case INTCONTINUE:
		case INTOKAY:
			continue;
		default:
			return irc;
		}
	}
}
/*=======================================+
 * get_proc_node -- Find proc (or func) in local or global table
 *  returns NULL if error, and sets *count to how many global listing
 *  (ie, 0 or more than 1)
 * Created: 2002/11/30 (Perry Rapp)
 *======================================*/
PNODE
get_proc_node (CNSTRING procname, TABLE loctab, TABLE gtab, INT * count)
{
	LIST list=0;
	PNODE proc = valueof_ptr(loctab, procname);
	if (proc) return proc;
	/* now look for global proc, and must be unique */
	list = valueof_obj(gtab, procname);
	if (!list) {
		*count = 0;
		return NULL;
	}
	if (length_list(list)>1) {
		*count = length_list(list);
		return NULL;
	}
	proc = peek_list_head(list);
	ASSERT(proc);
	return proc;
}
/*=======================================+
 * interp_call -- Interpret call structure
 *======================================*/
INTERPTYPE
interp_call (PNODE node, SYMTAB stab, PVALUE *pval)
{
	SYMTAB newstab = NULL;
	INTERPTYPE irc=INTERROR;
	PNODE arg=NULL, parm=NULL, proc;
	STRING procname = iname(node);
	INT count=0;
	/* find proc in local or global table */
	proc = get_proc_node(procname, irptinfo(node)->proctab, gproctab, &count);
	if (!proc) {
		if (!count)
			prog_error(node, _("Undefined proc: %s"), procname);
		else
			prog_error(node, _("Ambiguous call to proc: %s"), procname);
		irc = INTERROR;
		goto call_leave;
	}
	newstab = create_symtab_proc(procname, stab);
	arg = (PNODE) iargs(node);
	parm = (PNODE) iargs(proc);
	while (arg && parm) {
		BOOLEAN eflg = FALSE;
		PVALUE value = evaluate(arg, stab, &eflg);
		if (eflg) {
			irc = INTERROR;
			goto call_leave;
		}
		insert_symtab(newstab, iident(parm), value);
		arg = inext(arg);
		parm = inext(parm);
	}
	if (arg || parm) {
		prog_error(node, "``%s'': mismatched args and params\n", iname(node));
		irc = INTERROR;
		goto call_leave;
	}
	irc = interpret((PNODE) ibody(proc), newstab, pval);
	switch (irc) {
	case INTRETURN:
	case INTOKAY:
		irc = INTOKAY;
		break;
	case INTBREAK:
	case INTCONTINUE:
	case INTERROR:
	default:
		irc = INTERROR;
		break;
	}

call_leave:
	if (newstab) {
		remove_symtab(newstab);
		newstab = NULL;
	}
	return irc;
}
/*==============================================+
 * interp_traverse -- Interpret traverse iterator
 *  usage: traverse(NODE,NODE_V,INT_V) {...}
 * TO DO - doesn't clean up its symtab entries (2001/03/24)
 *=============================================*/
INTERPTYPE
interp_traverse (PNODE node, SYMTAB stab, PVALUE *pval)
{
	NODE snode, stack[100];
	BOOLEAN eflg = FALSE;
	INTERPTYPE irc;
	INT lev = -1;
	NODE root;
	PVALUE val = eval_and_coerce(PGNODE, iloopexp(node), stab, &eflg);
	if (eflg) {
		prog_var_error(node, stab, iloopexp(node), val, nonrecx,  "traverse", "1");
		irc = INTERROR;
		goto traverse_leave;
	}
	root = pvalue_to_node(val);
	if (!root) {
		irc = INTOKAY;
		goto traverse_leave;
	}
	stack[++lev] = snode = root;
	while (TRUE) {
		insert_symtab(stab, ielement(node), create_pvalue_from_node(snode));
		insert_symtab(stab, ilev(node), create_pvalue_from_int(lev));
		switch (irc = interpret((PNODE) ibody(node), stab, pval)) {
		case INTCONTINUE:
		case INTOKAY:
			break;
		case INTBREAK:
			irc = INTOKAY;
			goto traverse_leave;
		default:
			goto traverse_leave;
		}
		if (nchild(snode)) {
			snode = stack[++lev] = nchild(snode);
			continue;
		}
		if (lev>0 && nsibling(snode)) {
			snode = stack[lev] = nsibling(snode);
			continue;
		}
		while (--lev >= 1 && !nsibling(stack[lev]))
			;
		if (lev <= 0) break;
		snode = stack[lev] = nsibling(stack[lev]);
	}
	irc = INTOKAY;
traverse_leave:
	delete_symtab_element(stab, ielement(node));
	delete_symtab_element(stab, ilev(node));
	delete_pvalue(val);
	val=NULL;
	return irc;
}
/*=============================================+
 * pa_handle_global -- declare global variable
 * Called directly from generated parser code (ie, from code in yacc.y)
 *=============================================*/
void
pa_handle_global (STRING iden)
{
	insert_symtab(globtab, iden, create_pvalue_any());
}
/*=============================================+
 * pa_handle_option -- process option specified in report
 * Called directly from generated parser code (ie, from code in yacc.y)
 *=============================================*/
void
pa_handle_option (PVALUE optval)
{
	STRING optstr;
	ASSERT(ptype(optval)==PSTRING); /* grammar only allows strings */
	optstr = pvalue_to_string(optval);
	if (eqstr(optstr,"explicitvars")) {
		explicitvars = 1;
	} else {
		/* TO DO - figure out how to set the error flag & report error */
	}
}
/*=============================================+
 * pa_handle_char_encoding -- report command char_encoding("...")
 *  parse-time handling of report command
 *  node:   [IN]  current parse node
 *  vpinfo: [I/O] pointer to parseinfo structure (parse globals)
 * Called directly from generated parser code (ie, from code in yacc.y)
 *=============================================*/
void
pa_handle_char_encoding (PACTX pactx, PNODE node)
{
	PVALUE pval = ivalue(node);
	STRING codeset;
	ASSERT(ptype(pval)==PSTRING); /* grammar only allows strings */
	pactx=pactx; /* unused */
	codeset = pvalue_to_string(pval);
	strupdate(&irptinfo(node)->codeset, codeset);
}
/*=============================================+
 * make_internal_string_node -- make string node
 *  do any needed codeset conversion here
 *  lexical analysis time (same as parse-time) handling of string constants
 *=============================================*/
PNODE
make_internal_string_node (PACTX pactx, STRING str)
{
	PNODE node = 0;
	ZSTR zstr = zs_news(str);
	if (str && str[0]) {
		STRING fname = pactx->fullpath;
		STRING rptcodeset = get_rptinfo(fname)->codeset;
		XLAT xlat = transl_get_xlat_to_int(rptcodeset);
		transl_xlat(xlat, zstr);
	}
	node = string_node(pactx, zs_str(zstr));
	zs_free(&zstr);
	return node;
}
/*=============================================+
 * pa_handle_include -- report command include("...")
 *  parse-time handling of report command
 *=============================================*/
void
pa_handle_include (PACTX pactx, PNODE node)
{
	/*STRING fname = ifname(node); */ /* current file */
	PVALUE pval = ivalue(node);
	STRING newfname;
	STRING fullpath=0, localpath=0;
	ZSTR zstr=0;
	PATHINFO pathinfo = 0;
	pactx=pactx; /* unused */

	ASSERT(ptype(pval)==PSTRING); /* grammar only allows strings */
	newfname = pvalue_to_string(pval);
	
	/* if it is relative, get local path to give to find_program */
	if (!is_path(newfname)) {
		localpath = zs_str(irptinfo(node)->localpath);
	}

	if (find_program(newfname, localpath, &fullpath,TRUE)) {
		pathinfo = new_pathinfo(newfname, fullpath);
		strfree(&fullpath);
		enqueue_list(Plist, pathinfo);
	} else {
		prog_error(node, "included file not found: %s\n", newfname);
		++Perrors;
	}
	zs_free(&zstr);
}
/*=============================================+
 * pa_handle_require -- report command require("...")
 *  pactx: [I/O] pointer to parseinfo structure (parse globals)
 *  node:  [IN]  current parse node
 *=============================================*/
void
pa_handle_require (PACTX pactx, PNODE node)
{
	PVALUE pval = ivalue(node);
	STRING str;
	STRING propstr = "requires_lifelines-reports.version:";
	TABLE tab;
	ASSERT(ptype(pval)==PSTRING);
	pactx=pactx; /* unused */

	tab = (TABLE)valueof_obj(pactx->filetab, pactx->fullpath);
	if (!tab) {
		tab = create_table_str();
		insert_table_obj(pactx->filetab, cur_pathinfo->fullpath, tab);
		release_table(tab); /* release our reference, pactx->filetab owns now */
	}

	str = pvalue_to_string(pval);
	insert_table_str(tab, propstr, str);
}
/*=============================================+
 * pa_handle_proc -- proc declaration (parse time)
 * Created: 2002/11/30 (Perry Rapp)
 *=============================================*/
void
pa_handle_proc (PACTX pactx, CNSTRING procname, PNODE nd_args, PNODE nd_body)
{
	RPTINFO rptinfo = get_rptinfo(pactx->fullpath);
	PNODE procnode;
	LIST list;

	/* check for local duplicates, else add to local proc table */
	procnode = (PNODE)valueof_ptr(rptinfo->proctab, procname);
	if (procnode) {
		enqueue_parse_error(_("Duplicate proc %s (lines %d and %d) in report: %s")
			, procname, iline(procnode)+1, iline(nd_body)+1, pactx->fullpath);
	}
	/* consumes procname */
	procnode = proc_node(pactx, procname, nd_args, nd_body);
	insert_table_ptr(rptinfo->proctab, procname, procnode);

	/* add to global proc table */
	list = (LIST)valueof_obj(gproctab, procname);
	if (!list) {
		list = create_list2(LISTNOFREE);
		insert_table_obj(gproctab, procname, list);
		release_list(list); /* now table owns list */
	}
	enqueue_list(list, procnode);
}
/*=============================================+
 * pa_handle_func -- func declaration (parse time)
 * Created: 2002/11/30 (Perry Rapp)
 *=============================================*/
void
pa_handle_func (PACTX pactx, CNSTRING procname, PNODE nd_args, PNODE nd_body)
{
	RPTINFO rptinfo = get_rptinfo(pactx->fullpath);
	PNODE procnode=0;
	LIST list=0;

	/* check for local duplicates, else add to local proc table */
	procnode = (PNODE)valueof_ptr(rptinfo->functab, procname);
	if (procnode) {
		enqueue_parse_error(_("Duplicate func %s (lines %d and %d) in report: %s")
			, procname, iline(procnode)+1, iline(nd_body)+1, pactx->fullpath);
	}
	/* consumes procname */
	procnode = fdef_node(pactx, procname, nd_args, nd_body);
	insert_table_ptr(rptinfo->functab, procname, procnode);

	/* add to global proc table */
	list = (LIST)valueof_obj(gfunctab, procname);
	if (!list) {
		list = create_list2(LISTNOFREE);
		insert_table_obj(gfunctab, procname, list);
		release_list(list); /* now table owns list */
	}
	enqueue_list(list, procnode);
}
/*=============================================+
 * parse_error -- handle bison parse error
 *  pactx: [I/O] pointer to parseinfo structure (parse globals)
 *  ploc:  [IN]  token location
 *  node:  [IN]  current parse node
 *=============================================*/
void
parse_error (PACTX pactx, STRING str)
{
	/* TO DO - how to pass current pnode ? */
	prog_error(NULL, "Syntax Error (%s): %s: line %d, char %d\n"
		, str, pactx->fullpath, pactx->lineno+1, pactx->charpos+1);
	Perrors++;
}
/*=============================================+
 * check_rpt_requires -- check any prerequisites for
 *  this report file -- return desc. string if fail
 *=============================================*/
static STRING
check_rpt_requires (PACTX pactx, STRING fullpath)
{
	TABLE tab = (TABLE)valueof_obj(pactx->filetab, fullpath);
	STRING str;
	STRING propstr = "requires_lifelines-reports.version:";
	INT ours=0, desired=0;
	STRING optr=LIFELINES_REPORTS_VERSION;
	STRING dptr=0;
	if (!tab)
		return 0;
	str = valueof_str(tab, propstr);
	if (str) {
		dptr=str+strlen(propstr)-strlen("requires_");
		while (1) {
			ours=0;
			desired=0;
			while (optr[0] && !isdigit(optr[0]))
				++optr;
			while (dptr[0] && !isdigit(dptr[0]))
				++dptr;
			if (!optr[0] && !dptr[0])
				return 0;
			/* no UTF-8 allowed here, only regular digits */
			while (isdigit(optr[0]))
				ours = ours*10 + (*optr++ -'0');
			while (isdigit(dptr[0]))
				desired = desired*10 + (*dptr++ - '0');
			if (desired > ours)
				return _("This report requires a newer program to run\n");
			if (desired < ours)
				return 0;
			/* else equal, continue to minor version blocks */
		}
	}
	return 0;
}
/*=============================================+
 * enqueue_parse_error -- Queue up a parsing error
 *  for display after parsing complete
 *  (this is fatal; it will prevent report execution)
 * Created: 2002/11/30 (Perry Rapp)
 *=============================================*/
static void
enqueue_parse_error (const char * fmt, ...)
{
	char buffer[512];
	va_list args;
	va_start(args, fmt);

	if (!outstanding_parse_errors) {
		outstanding_parse_errors = create_list2(LISTDOFREE);
	}
	llstrsetvf(buffer, sizeof(buffer), 0, fmt, args);
	va_end(args);
	enqueue_list(outstanding_parse_errors, strsave(buffer));
}

/*=============================================+
 * get_report_error_message - Return error message
 *  for display during signal processing
 * Created: 2003/07/01 (Matt Emmerton)
 *=============================================*/
ZSTR
get_report_error_msg (STRING msg)
{
	ZSTR zstr=0;

	if (progrunning) {
		char line[20];
		snprintf(line, sizeof(line), "%ld", iline(Pnode)+1);
		zstr = zprintpic2(_(msg), irptinfo(Pnode)->fullpath, line);
        }
	return zstr;
}
/*=============================================+
 * clean_orphaned_rptlocks - Clean up any locks on
 * records that report didn't unlock
 *=============================================*/
static
void clean_orphaned_rptlocks (void)
{
	int ct = free_all_rprtlocks();
	if (ct) {
		char msg[256];
		sprintf(msg, _pl("Program forgot to unlock %d record",
			"Program forgot to unlock %d records", ct), ct);
		progmessage(MSG_ERROR, msg);

	}
}
