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
 * rassa.c -- Handle output to the product file
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
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
#include "screen.h"

#define MAXPAGESIZE 65536
#define MAXROWS 512
#define MAXCOLS 512
INT __cols = 0, __rows = 0;
INT curcol = 1, currow = 1;
INT outputmode = BUFFERED;

static STRING pagebuffer = NULL;
static unsigned char linebuffer[1024];
static INT linebuflen = 0;
static STRING bufptr = linebuffer;

static void adjust_cols(STRING);

STRING outfilename;
STRING noreport = (STRING) "No report was generated.";
STRING whtout = (STRING) "What is the name of the output file?";

static void adjust_cols (STRING str);

/*======================================+
 * initrassa -- Initialize program output
 *=====================================*/
void
initrassa (void)
{
	outputmode = BUFFERED;
	linebuflen = 0;
	bufptr = linebuffer;
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
		bufptr = linebuffer;
		curcol = 1;
	}
}
/*========================================+
 * __pagemode -- Switch output to page mode
 *   usage: pagemode(INT, INT) -> VOID
 *======================================*/
PVALUE
__pagemode (PNODE node,
            TABLE stab,
            BOOLEAN *eflg)
{
	INT cols, rows;
	PVALUE val = eval_and_coerce(PINT, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to pagemode must be an integer.");
		return NULL;
	}
	rows = (INT) pvalue(val);
	delete_pvalue(val);
	val = eval_and_coerce(PINT, inext((PNODE)iargs(node)), stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to pagemode must be an integer.");
		return NULL;
	}
	cols = (INT) pvalue(val);
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
__linemode (PNODE node,
            TABLE stab,
            BOOLEAN *eflg)
{
	outputmode = BUFFERED;
	linebuflen = 0;
	bufptr = linebuffer;
	curcol = 1;
	*eflg = FALSE;
	return NULL;
}
/*======================================+
 * __newfile -- Switch output to new file
 *   usage: newfile(STRING, BOOL) -> VOID
 *=====================================*/
PVALUE
__newfile (PNODE node,
           TABLE stab,
           BOOLEAN *eflg)
{
	BOOLEAN aflag;
	STRING name;
	PVALUE val = eval_and_coerce(PSTRING, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to newfile must be a string.");
		return NULL;
	}
	name = (STRING) pvalue(val);
	if (!name || *name == 0) {
		*eflg = TRUE;
		prog_error(node, "1st arg to newfile must be a string.");
		return NULL;
	}
	outfilename = strsave(name);
	delete_pvalue(val);
	val = eval_and_coerce(PBOOL, inext((PNODE) iargs(node)), stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to newfile must be boolean.");
		return NULL;
	}
	aflag = (BOOLEAN) pvalue(val);
	delete_pvalue(val);
	if (Poutfp) {
		finishrassa();
		fclose(Poutfp);
		Poutfp = NULL;
	}
	if (!(Poutfp = fopenpath(outfilename,
			 aflag?LLAPPENDTEXT:LLWRITETEXT, llreports, NULL, (STRING *)NULL))) {
		mprintf("Could not open file %s", outfilename);
		return NULL;
	}
	return NULL;
}
/*====================================+
 * __outfile -- Return output file name
 *   usage: outfile() -> STRING
 *===================================*/
PVALUE
__outfile (PNODE node,
           TABLE stab,
           BOOLEAN *eflg)
{
	if (!Poutfp) {
		Poutfp = ask_for_file(LLWRITETEXT, whtout, &outfilename, llreports, NULL);
		if (!Poutfp)  {
			*eflg = TRUE;
			message(noreport);
			return NULL;
		}
		setbuf(Poutfp, NULL);
	}
	*eflg = FALSE;
	return create_pvalue(PSTRING, outfilename);
}
/*===============================================+
 * __pos -- Position page output to row and column
 *   usage: pos(INT, INT) -> VOID
 *==============================================*/
PVALUE
__pos (PNODE node,
       TABLE stab,
       BOOLEAN *eflg)
{
	INT col, row;
	PVALUE val = eval_and_coerce(PINT, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "1st arg to pos must be an integer.");
		return NULL;
	}
	row = (INT) pvalue(val);
	delete_pvalue(val);
	val = eval_and_coerce(PINT, inext((PNODE) iargs(node)), stab, eflg);
	if (*eflg) {
		prog_error(node, "2nd arg to pos must be an integer.");
		return NULL;
	}
	col = (INT) pvalue(val);
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
__row (PNODE node,
       TABLE stab,
       BOOLEAN *eflg)
{
	INT row;
	PVALUE val = eval_and_coerce(PINT, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to row must be an integer.");
		return NULL;
	}
	*eflg = TRUE;
	row = (INT) pvalue(val);
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
__col (PNODE node,
       TABLE stab,
       BOOLEAN *eflg)
{
	INT newcol;
	PVALUE val = eval_and_coerce(PINT, iargs(node), stab, eflg);
	if (*eflg) {
		prog_error(node, "the arg to col must be an integer.");
		return NULL;
	}
	newcol = (INT) pvalue(val);
	delete_pvalue(val);
	if (newcol < 1) newcol = 1;
	if (newcol > MAXCOLS) newcol = MAXCOLS;
	if (newcol == curcol) return NULL;
	if (newcol < curcol) poutput("\n");
	while (curcol < newcol) poutput(" ");
	return NULL;
}
/*=================================+
 * __getcol -- Return current column
 *   usage: getcol() -> INT
 *================================*/
PVALUE
__getcol (PNODE node,
          TABLE stab,
          BOOLEAN *eflg)
{
	*eflg = FALSE;
	return create_pvalue(PINT, (WORD)curcol);
}
/*======================================================
 * __pageout -- Output current page and clear page buffer
 *   usage: pageout() -> VOID
 *====================================================*/
PVALUE
__pageout (PNODE node,
           TABLE stab,
           BOOLEAN *eflg)
{
	char scratch[MAXCOLS+2];
	STRING p;
	INT row, i;
	*eflg = TRUE;
	if (outputmode != PAGEMODE) return NULL;
	if (!Poutfp) {
		Poutfp = ask_for_file(LLWRITETEXT, whtout, &outfilename, llreports, NULL);
		if (!Poutfp)  {
			message(noreport);
			return NULL;
		}
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
poutput (STRING str)
{
	STRING p, name;
	INT c, len;
	if (!str || *str == 0 || (len = strlen(str)) <= 0) return;
	if (!Poutfp) {
		Poutfp = ask_for_file(LLWRITETEXT, whtout, &name, llreports, NULL);
		if (!Poutfp)  {
			message(noreport);
			return;
		}
		setbuf(Poutfp, NULL);
		outfilename = strsave(name);
	}
	switch (outputmode) {
	case UNBUFFERED:
		fwrite(str, len, 1, Poutfp);
		adjust_cols(str);
		return;
	case BUFFERED:
		if (len >= 1024) {
			fwrite(linebuffer, linebuflen, 1, Poutfp);
			fwrite(str, len, 1, Poutfp);
			linebuflen = 0;
			bufptr = linebuffer;
			adjust_cols(str);
			return;
		}
		if (len + linebuflen >= 1024) {
			fwrite(linebuffer, linebuflen, 1, Poutfp);
			linebuflen = 0;
			bufptr = linebuffer;
		}
		linebuflen += len;
		while ((c = *bufptr++ = *str++)) {
			if (c == '\n')
				curcol = 1;
			else
				curcol++;
		}
		--bufptr;
		return;
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
		return;
	default:
		FATAL();
	}
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
