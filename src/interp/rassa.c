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
#include "zstr.h"

#include "interpi.h"


/*********************************************
 * external/imported variables
 *********************************************/

extern STRING nonintx;

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
extern STRING qSwhtout;
extern INT rpt_cancelled;

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
 * __pagemode -- Switch output to page mode
 *   usage: pagemode(INT, INT) -> VOID
 *======================================*/
PVALUE
__pagemode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT cols, rows;
	PVALUE val = eval_and_coerce(PINT, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to pagemode must be an integer.");
		return NULL;
	}
	rows = pvalue_to_int(val);
	delete_pvalue(val);
	val = eval_and_coerce(PINT, inext((PNODE)iargs(node)), stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to pagemode must be an integer.");
		return NULL;
	}
	cols = pvalue_to_int(val);
	delete_pvalue(val);
	*eflg = TRUE;
	if (cols < 1 || cols > MAXCOLS || rows < 1 || rows > MAXROWS) {
		prog_error(node, "illegal page size.");
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
 * __linemode -- Switch output to line mode
 *   usage: linemode() -> VOID
 *=======================================*/
PVALUE
__linemode (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	outputmode = BUFFERED;
	linebuflen = 0;
	bufptr = (STRING)linebuffer;
	curcol = 1;
	*eflg = FALSE;
	return NULL;
}
/*======================================+
 * __newfile -- Switch output to new file
 *   usage: newfile(STRING, BOOL) -> VOID
 *=====================================*/
PVALUE
__newfile (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	BOOLEAN aflag;
	STRING name;
	PVALUE val = eval_and_coerce(PSTRING, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to newfile must be a string.");
		return NULL;
	}
	name = pvalue_to_string(val);
	if (!name || *name == 0) {
		*eflg = TRUE;
		prog_error(node, "1st arg to newfile must be a string.");
		return NULL;
	}
	if (outfilename)
		stdfree(outfilename);
	outfilename = strsave(name);
	delete_pvalue(val);
	val = eval_and_coerce(PBOOL, inext((PNODE) iargs(node)), stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to newfile must be boolean.");
		return NULL;
	}
	aflag = pvalue_to_bool(val);
	delete_pvalue(val);
	set_output_file(outfilename, aflag);
	return NULL;
}
BOOLEAN
set_output_file (STRING outfname, BOOLEAN append)
{
	STRING modestr = append ? LLAPPENDTEXT:LLWRITETEXT;
	STRING rptdir;
	if (Poutfp) {
		finishrassa();
		fclose(Poutfp);
		Poutfp = NULL;
	}
	rptdir = getoptstr("LLREPORTS", ".");
	Poutfp = fopenpath(outfname, modestr, rptdir, NULL, NULL);
	if (!Poutfp) {
		msg_error("Could not open file %s", outfname);
		return FALSE;
	}
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
	STRING rptdir = getoptstr("LLREPORTS", ".");
	STRING fname=0, fullpath=0;
	Poutfp = ask_for_output_file(LLWRITETEXT, _(qSwhtout), &fname, &fullpath
		, rptdir, NULL);
	if (!Poutfp || !fname || !fname[0])  {
		if (fname)
			prog_error(0, "Report stopping due to error opening output file");
		else
			prog_error(0, "Report stopping due to lack of output file");
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
	return TRUE;
}
/*====================================+
 * __outfile -- Return output file name
 *   usage: outfile() -> STRING
 *===================================*/
PVALUE
__outfile (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	if (!Poutfp) {
		if (!request_file(eflg))
			return NULL;
		setbuf(Poutfp, NULL);
	}
	*eflg = FALSE;
	return create_pvalue_from_string(outfilename);
}
/*===============================================+
 * __pos -- Position page output to row and column
 *   usage: pos(INT, INT) -> VOID
 *==============================================*/
PVALUE
__pos (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT col, row;
	PVALUE val = eval_and_coerce(PINT, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, nonintx, "pos", "1");
		return NULL;
	}
	row = pvalue_to_int(val);
	delete_pvalue(val);
	val = eval_and_coerce(PINT, inext((PNODE) iargs(node)), stab, eflg);
	if (*eflg) {
		prog_error(node, nonintx, "pos", "1");
		return NULL;
	}
	col = pvalue_to_int(val);
	delete_pvalue(val);
	if (outputmode != PAGEMODE || row < 1 || row > __rows ||
	    col < 1 || col > __cols) {
		*eflg = TRUE;
		prog_error(node, "illegal call to pos.");
		return NULL;
	}
	currow = row;
	curcol = col;
	return NULL;
}
/*========================================+
 * __row -- Position output to start of row
 *   usage: row(INT) -> VOID
 *=======================================*/
PVALUE
__row (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT row;
	PVALUE val = eval_and_coerce(PINT, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to row must be an integer.");
		return NULL;
	}
	*eflg = TRUE;
	row = pvalue_to_int(val);
	delete_pvalue(val);
	if (outputmode != PAGEMODE || row < 1 || row > __rows) {
		prog_error(node, "the arg to row is in error.");
		return NULL;
	}
	*eflg = FALSE;
	currow = row;
	curcol = 1;
	return NULL;
}
/*==================================+
 * __col -- Position output to column
 *   usage: col(INT) -> VOID
 *=================================*/
PVALUE
__col (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	INT newcol;
	PVALUE val = eval_and_coerce(PINT, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to col must be an integer.");
		return NULL;
	}
	newcol = pvalue_to_int(val);
	delete_pvalue(val);
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
 * __getcol -- Return current column
 *   usage: getcol() -> INT
 *================================*/
PVALUE
__getcol (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	node=node; /* unused */
	stab=stab; /* unused */
	*eflg = FALSE;
	return create_pvalue_from_int(curcol);
}
/*======================================================
 * __pageout -- Output current page and clear page buffer
 *   usage: pageout() -> VOID
 *====================================================*/
PVALUE
__pageout (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	char scratch[MAXCOLS+2];
	STRING p;
	INT row, i;
	node=node; /* unused */
	stab=stab; /* unused */
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
	TRANMAPPING ttmr = get_tranmapping(MINRP);
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
	zs_free(zstr);
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
