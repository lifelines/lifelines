/* 
   Copyright (c) 2001 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*======================================================================
 * parse.c -- Handle parse global variables
 * Copyright(c) 2002 by Perry Rapp; all rights reserved
 *====================================================================*/

#include <stdarg.h>
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
#include "arch.h"
#include "lloptions.h"
#include "parse.h"


/*********************************************
 * local types
 *********************************************/

/* parse global context */
struct pactx_s {
	FILE *Pinfp;     /* file to read program from */
	STRING Pinstr;   /* string to read program from */
	TABLE filetab;   /* table of files called by current report (incl. itself) */
	STRING ifile;    /* user's requested program path (current report) */
	STRING fullpath; /* actual path of current program */
	INT lineno;      /* current line number */
};

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*===============================================
 * create_pactx -- create new global parsing context
 *=============================================*/
void *
create_pactx (void)
{
	struct pactx_s *pctx = (struct pactx_s *)stdalloc(sizeof(*pctx));
	memset(pctx, 0, sizeof(*pctx));
	pctx->filetab = create_table();
	return pctx;
}
/*===============================================
 * delete_pactx -- destroy global parsing context
 *=============================================*/
void
delete_pactx (void *pactx)
{
	struct pactx_s *pctx = (struct pactx_s *)stdalloc(sizeof(*pctx));
/* we can't free the keys in filetab, because the
parser put those pointers into pvalues for files
named in include statements */
	remove_table(pctx->filetab, DONTFREE);
	pctx->filetab=NULL;
	
	stdfree(pactx);
}
/*===============================================
 * get_filetab -- return filetab out of global parsing context
 *=============================================*/
TABLE
get_filetab (void *pactx)
{
	struct pactx_s *pctx = (struct pactx_s *)pactx;
	ASSERT(pctx);
	return pctx->filetab;
}
/*===============================================
 * get_filetab -- return filetab out of global parsing context
 *=============================================*/
FILE *
get_infp (void *pactx)
{
	struct pactx_s *pctx = (struct pactx_s *)pactx;
	ASSERT(pctx);
	return pctx->Pinfp;
}
/*===============================================
 * get_infp_info -- get path info on current report
 * NB: client may not change or free these.
 *=============================================*/
void
get_infp_info (void *pactx, STRING *pifile, STRING *pfullpath)
{
	struct pactx_s *pctx = (struct pactx_s *)pactx;
	ASSERT(pctx);
	*pifile = pctx->ifile;
	*pfullpath = pctx->fullpath;
}
/*===============================================
 * set_infp -- store file handle to current program file
 *=============================================*/
void
set_infp (void *pactx, FILE *fp, STRING file, STRING fullpath)
{
	struct pactx_s *pctx = (struct pactx_s *)pactx;
	ASSERT(pctx && !pctx->Pinfp);
	pctx->Pinfp = fp;
	pctx->ifile = strdup(file);
	pctx->fullpath = strdup(fullpath);
}
/*===============================================
 * close_infp -- close current input file (ie, current program source)
 *=============================================*/
void
close_infp (void *pactx)
{
	struct pactx_s *pctx = (struct pactx_s *)pactx;
	ASSERT(pctx && pctx->Pinfp);
	closefp(&pctx->Pinfp);
	strfree(&pctx->ifile);
	strfree(&pctx->fullpath);
}
/*===============================================
 * get_lineno -- return current line number
 *=============================================*/
INT
get_lineno (void *pactx)
{
	struct pactx_s *pctx = (struct pactx_s *)pactx;
	ASSERT(pctx);
	return pctx->lineno;
}
/*===============================================
 * adj_lineno -- change current line number
 *=============================================*/
void
adj_lineno (void *pactx, int delta)
{
	struct pactx_s *pctx = (struct pactx_s *)pactx;
	ASSERT(pctx);
	pctx->lineno += delta;
}
