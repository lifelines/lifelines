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
 * rassa.c -- Handle program report output to the output file
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 17 Dec 93    3.0.2 - 15 Dec 94
 *   3.0.3 - 19 Nov 95
 *===========================================================*/

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
#include "zstr.h"

#include "interpi.h"


/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSwhtout;
extern INT rpt_cancelled;

/*********************************************
 * local enums & defines
 *********************************************/

#define MAXPAGESIZE 65536
#define MAXROWS 512
#define MAXCOLS 512

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void adjust_cols(STRING);
static BOOLEAN request_file(BOOLEAN *eflg);
static BOOLEAN set_output_file(STRING outfilename, BOOLEAN append);

/*********************************************
 * local variables
 *********************************************/

static INT __cols = 0, __rows = 0;
static INT curcol = 1, currow = 1;
static INT outputmode = BUFFERED;

static STRING pagebuffer = NULL;
static char linebuffer[1024];
static INT linebuflen = 0;
static STRING bufptr = (STRING)linebuffer;

static STRING outfilename;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*======================================+
 * initrassa -- Initialize program output
 *=====================================*/
void
initrassa (void)
{
	outputmode = BUFFERED;
	linebuflen = 0;
	bufptr = (STRING)linebuffer;
	curcol = 1;
}
/*======================================+
 * finishrassa -- Finalize program output
 *=====================================*/
void
finishrassa (void)
{
	if (outputmode == BUFFERED && linebuflen > 0 && Poutfp) {
		fwrite(linebuffer, linebuflen, 1, Poutfp);
		linebuflen = 0;
		bufptr = (STRING)linebuffer;
		curcol = 1;
	}
}
/*========================================+
 * llrpt_pagemode -- Switch output to page mode
 * usage: pagemode(INT, INT) -> VOID
 *======================================*/
PVALUE
llrpt_pagemode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT cols=0, rows=0;
	PNODE argvar = builtin_args(node);
	PVALUE val = eval_and_coerce(PINT, argvar, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, argvar, val, nonintx, "pagemode", "1");
		delete_pvalue_ptr(&val);
		return NULL;
	}
	rows = pvalue_to_int(val);
	delete_pvalue_ptr(&val);
	val = eval_and_coerce(PINT, argvar=inext(argvar), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, argvar, val, nonintx, "pagemode", "2");
		delete_pvalue_ptr(&val);
		return NULL;
	}
	cols = pvalue_to_int(val);
	delete_pvalue_ptr(&val);
	*eflg = TRUE;
	if (!(cols >= 1 && cols <= MAXCOLS)) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, val, badargx, "pagemode", "1");
		return NULL;
	}
	if (!(rows >= 1 && rows <= MAXROWS)) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, val, badargx, "pagemode", "2");
		return NULL;
	}
	*eflg = FALSE;
	outputmode = PAGEMODE;
	__rows = rows;
	__cols = cols;
	if (pagebuffer) stdfree(pagebuffer);
	pagebuffer = (STRING) stdalloc(__rows*__cols);
	memset(pagebuffer, ' ', __rows*__cols);
	return NULL;
}
/*========================================+
 * llrpt_linemode -- Switch output to line mode
 * usage: linemode() -> VOID
 *=======================================*/
PVALUE
llrpt_linemode (HINT_PARAM_UNUSED PNODE node, HINT_PARAM_UNUSED SYMTAB stab, BOOLEAN *eflg)
{
	outputmode = BUFFERED;
	linebuflen = 0;
	bufptr = (STRING)linebuffer;
	curcol = 1;
	*eflg = FALSE;
	return NULL;
}
/*======================================+
 * llrpt_newfile -- Switch output to new file
 * usage: newfile(STRING, BOOL) -> VOID
 *=====================================*/
PVALUE
llrpt_newfile (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	PNODE argvar = builtin_args(node);
	BOOLEAN aflag=FALSE;
	STRING name=0;
	PVALUE val = eval_and_coerce(PSTRING, argvar, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, argvar, val, nonstrx, "newfile", "1");
		delete_pvalue_ptr(&val);
		return NULL;
	}
	name = pvalue_to_string(val);
	if (!name || !name[0]) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, val, "1st arg to newfile must be a nonempty string.");
		delete_pvalue_ptr(&val);
		return NULL;
	}
	strupdate(&outfilename, name);
	delete_pvalue_ptr(&val);
	val = eval_and_coerce(PBOOL, argvar=inext(argvar), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, argvar, val, nonboox, "newfile", "2");
		delete_pvalue_ptr(&val);
		return NULL;
	}
	aflag = pvalue_to_bool(val);
	delete_pvalue_ptr(&val);
	if (!set_output_file(outfilename, aflag)) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, NULL, "Failed to open output file: %s", outfilename);
	}
	return NULL;
}
/*======================================+
 * start_output_file -- Start outputing to specified file
 *  (Closes any previously open output file)
 * Calls msg_error & returns FALSE, if problem
 *=====================================*/
BOOLEAN
start_output_file (STRING outfname)
{
	BOOLEAN append=FALSE;
	if (set_output_file(outfname, append)) {
		strupdate(&outfilename, outfname);
		return TRUE;
	} else {
		return FALSE;
	}
}
/*======================================+
 * set_output_file -- Open specified output file
 *  (Closes any previously open output file)
 * Calls msg_error & returns FALSE, if problem
 *=====================================*/
static BOOLEAN
set_output_file (STRING outfname, BOOLEAN append)
{
	STRING modestr = append ? LLAPPENDTEXT:LLWRITETEXT;
	STRING rptdir=0;
	if (Poutfp) {
		finishrassa();
		fclose(Poutfp);
		Poutfp = NULL;
	}
	rptdir = getlloptstr("LLREPORTS", ".");
	Poutfp = fopenpath(outfname, modestr, rptdir, NULL, uu8, NULL);
	if (!Poutfp) {
		/* TODO: need to forward this through rptui */
		msg_error(_("Could not open file %s"), outfname);
		return FALSE;
	}
	/* if appending to existing non-empty file, don't add BOM */
	if (append) {
		long offset = 0;
		fseek(Poutfp, 0, SEEK_END);
		offset = ftell(Poutfp);
		if (offset > 0)
			return TRUE;
	}
	prefix_file_for_report(Poutfp);
	return TRUE;
}
/*====================================+
 * request_file -- Prompt user for file name
 *  returns open file pointer, or NULL if error
 *  handles error message
 * Created: 2002/01/18
 *===================================*/
static BOOLEAN
request_file (BOOLEAN *eflg)
{
	STRING rptdir = getlloptstr("LLREPORTS", ".");
	STRING fname=0, fullpath=0;
	Poutfp = rptui_ask_for_output_file(LLWRITETEXT, _(qSwhtout), &fname
		, &fullpath, rptdir, NULL);
	if (!Poutfp || !fname || !fname[0])  {
		if (fname)
			prog_error(0, _("Report stopping due to error opening output file"));
		else
			prog_error(0, _("Report stopping due to lack of output file"));
		/* set error flag to stop interpreter */
		*eflg = TRUE;
		/* set cancel flag to suppress traceback */
		rpt_cancelled = TRUE;
		strfree(&fname);
		return FALSE;
	}
	if (outfilename)
		stdfree(outfilename);
	outfilename = fullpath;
	strfree(&fname);
	prefix_file_for_report(Poutfp);
	return TRUE;
}
/*====================================+
 * llrpt_outfile -- Return output file name
 * usage: outfile() -> STRING
 *===================================*/
PVALUE
llrpt_outfile (HINT_PARAM_UNUSED PNODE node, HINT_PARAM_UNUSED SYMTAB stab, BOOLEAN *eflg)
{
	if (!Poutfp) {
		if (!request_file(eflg))
			return NULL;
		setbuf(Poutfp, NULL);
	}
	*eflg = FALSE;
	return create_pvalue_from_string(outfilename);
}
/*===============================================+
 * llrpt_pos -- Position page output to row and column
 * usage: pos(INT, INT) -> VOID
 *==============================================*/
PVALUE
llrpt_pos (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT col=0, row=0;
	PNODE argvar = builtin_args(node);
	PVALUE val = eval_and_coerce(PINT, argvar, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, argvar, val, nonintx, "pos", "1");
		delete_pvalue_ptr(&val);
		return NULL;
	}
	row = pvalue_to_int(val);
	if (row < 1 || row > __rows) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, val, badargx, "pos", "1");
		return NULL;
	}
	delete_pvalue_ptr(&val);
	val = eval_and_coerce(PINT, argvar=inext(argvar), stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, argvar, val, nonintx, "pos", "2");
		delete_pvalue_ptr(&val);
		return NULL;
	}
	col = pvalue_to_int(val);
	if (col < 1 || col > __cols) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, val, badargx, "pos", "2");
		return NULL;
	}
	delete_pvalue_ptr(&val);
	if (outputmode != PAGEMODE) {
		*eflg = TRUE;
		prog_var_error(node, stab, NULL, val, "pos only valid in page mode");
		return NULL;
	}
	currow = row;
	curcol = col;
	return NULL;
}
/*========================================+
 * llrpt_row -- Position output to start of row
 * usage: row(INT) -> VOID
 *=======================================*/
PVALUE
llrpt_row (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT row=0;
	PNODE argvar = builtin_args(node);
	PVALUE val = eval_and_coerce(PINT, argvar, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, argvar, val, nonint1, "row");
		delete_pvalue_ptr(&val);
		return NULL;
	}
	*eflg = TRUE;
	row = pvalue_to_int(val);
	delete_pvalue_ptr(&val);
	if (outputmode != PAGEMODE) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, val, "row only valid in page mode");
		return NULL;
	}
	if (row < 1 || row > __rows) {
		*eflg = TRUE;
		prog_var_error(node, stab, argvar, val, badarg1, "row");
		return NULL;
	}
	*eflg = FALSE;
	currow = row;
	curcol = 1;
	return NULL;
}
/*==================================+
 * llrpt_col -- Position output to column
 * usage: col(INT) -> VOID
 *=================================*/
PVALUE
llrpt_col (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT newcol=0;
	PNODE argvar = builtin_args(node);
	PVALUE val = eval_and_coerce(PINT, argvar, stab, eflg);
	if (*eflg) {
		prog_var_error(node, stab, argvar, val, nonint1, "col");
		delete_pvalue_ptr(&val);
		return NULL;
	}
	newcol = pvalue_to_int(val);
	delete_pvalue_ptr(&val);
	if (newcol < 1) newcol = 1;
	if (newcol > MAXCOLS) newcol = MAXCOLS;
	if (newcol == curcol) return NULL;
	if (newcol < curcol)
		poutput("\n", eflg);
	while (curcol < newcol && !(*eflg))
		poutput(" ", eflg);
	return NULL;
}
/*=================================+
 * llrpt_getcol -- Return current column
 * usage: getcol() -> INT
 *================================*/
PVALUE
llrpt_getcol (HINT_PARAM_UNUSED PNODE node, HINT_PARAM_UNUSED SYMTAB stab, BOOLEAN *eflg)
{
	*eflg = FALSE;
	return create_pvalue_from_int(curcol);
}
/*======================================================
 * llrpt_pageout -- Output current page and clear page buffer
 * usage: pageout() -> VOID
 *====================================================*/
PVALUE
llrpt_pageout (HINT_PARAM_UNUSED PNODE node, HINT_PARAM_UNUSED SYMTAB stab, BOOLEAN *eflg)
{
	char scratch[MAXCOLS+2];
	STRING p;
	INT row, i;
	*eflg = TRUE;
	if (outputmode != PAGEMODE) return NULL;
	if (!Poutfp) {
		if (!request_file(eflg))
			return NULL;
		setbuf(Poutfp, NULL);
	}
	*eflg = FALSE;
	scratch[__cols] = '\n';
	scratch[__cols+1] = 0;
	p = pagebuffer;
	for (row = 1; row <= __rows; row++) {
		memcpy(scratch, p, __cols);
		for (i = __cols - 1; i > 0 && scratch[i] == ' '; i--)
			;
		scratch[i+1] = '\n';
		scratch[i+2] = 0;
		fputs(scratch, Poutfp);
		p += __cols;
	}
	memset(pagebuffer, ' ', __rows*__cols);
	return NULL;
}
/*========================================+
 * poutput -- Output string in current mode
 *=======================================*/
void
poutput (STRING str, BOOLEAN *eflg)
{
	STRING p;
	ZSTR zstr = 0;
	INT c, len;
	XLAT ttmr = transl_get_predefined_xlat(MINRP);
	if (!str || (len = strlen(str)) <= 0) return;
	zstr = translate_string_to_zstring(ttmr, str);
	str = zs_str(zstr);
	if ((len = strlen(str)) <= 0)
		goto exit_poutput;
	if (!Poutfp) {
		if (!request_file(eflg))
			goto exit_poutput;
		setbuf(Poutfp, NULL);
	}
	switch (outputmode) {
	case UNBUFFERED:
		fwrite(str, len, 1, Poutfp);
		adjust_cols(str);
		goto exit_poutput;
	case BUFFERED:
		if (len >= 1024) {
			fwrite(linebuffer, linebuflen, 1, Poutfp);
			fwrite(str, len, 1, Poutfp);
			linebuflen = 0;
			bufptr = (STRING)linebuffer;
			adjust_cols(str);
			goto exit_poutput;
		}
		if (len + linebuflen >= 1024) {
			fwrite(linebuffer, linebuflen, 1, Poutfp);
			linebuflen = 0;
			bufptr = (STRING)linebuffer;
		}
		linebuflen += len;
		while ((c = *bufptr++ = *str++)) {
			if (c == '\n')
				curcol = 1;
			else
				curcol++;
		}
		--bufptr;
		goto exit_poutput;
	case PAGEMODE:
		p = pagebuffer + (currow - 1)*__cols + curcol - 1;
		while ((c = *str++)) {
			if (c == '\n') {
				curcol = 1;
				currow++;
				p = pagebuffer + (currow - 1)*__cols;
			} else {
				if (curcol <= __cols && currow <= __rows)
					*p++ = c;
				curcol++;
			}
		}
		goto exit_poutput;
	default:
		FATAL();
	}
exit_poutput:
	zs_free(&zstr);
}
/*==================================================+
 * adjust_cols -- Adjust column after printing string
 *=================================================*/
static void
adjust_cols (STRING str)
{
	INT c;
	while ((c = *str++)) {
		if (c == '\n')
			curcol = 1;
		else
			curcol++;
	}
}
