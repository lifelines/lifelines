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
 * xreffile.c -- Handle the xref file
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 02 Sep 93
 *   3.0.0 - 02 May 94    3.0.2 - 10 Nov 94
 *===========================================================*/

#include "standard.h"
#include "btree.h"
#include "table.h"
#include "gedcom.h"

extern BTREE BTR;

/*===================================================================
 * First five words in xrefs file are number of INDI, FAM, EVEN, SOUR
 *   and other keys in file; remaining words are keys, in respective
 *   order, for the records; first in each group is next unused key;
 *   rest are keys of deleted records
 *=================================================================*/

static INT nixrefs;	/* num of INDI keys */
static INT nfxrefs;	/* num of FAM keys */
static INT nexrefs;	/* num of EVEN keys */
static INT nsxrefs;	/* num of SOUR keys */
static INT nxxrefs;	/* num of other keys */
static INT *ixrefs;	/* list of INDI keys */
static INT *fxrefs;	/* list of FAM keys */
static INT *exrefs;	/* list of EVEN keys */
static INT *sxrefs;	/* list of SOUR keys */
static INT *xxrefs;	/* list of other keys */
static INT maxixrefs = 0;
static INT maxfxrefs = 0;
static INT maxexrefs = 0;
static INT maxsxrefs = 0;
static INT maxxxrefs = 0;
static FILE *xreffp;	/* open xref file pointer */
static BOOLEAN xrefopen = FALSE;

/*============================== 
 * initxref -- Create xrefs file
 *============================*/
initxref ()
{
	char scratch[100];
	INT i = 1, j;
	ASSERT(!xrefopen);
	sprintf(scratch, "%s/xrefs", BTR->b_basedir);
	ASSERT(xreffp = fopen(scratch, "w"));
	for (j = 0; j < 10; j++) {
		ASSERT(fwrite(&i, sizeof(INT), 1, xreffp) == 1);
	}
	fclose(xreffp);
}
/*============================
 * openxref -- Open xrefs file
 *==========================*/
BOOLEAN openxref ()
{
	char scratch[100];
	ASSERT(!xrefopen);
	sprintf(scratch, "%s/xrefs", BTR->b_basedir);
	if (!(xreffp = fopen(scratch, "r+"))) return FALSE;
	xrefopen = TRUE;
	return readxrefs();
}
/*==============================
 * closexref -- Close xrefs file
 *============================*/
closexref ()
{
	fclose(xreffp);
	xrefopen = FALSE;
}
/*====================================
 * getixref -- Return next ixref value
 *==================================*/
STRING getixref ()
{
	INT n;
	static unsigned char scratch[10];
	ASSERT(xrefopen && nixrefs >= 1);
	n = (nixrefs == 1) ? ixrefs[0]++ : ixrefs[--nixrefs];
	ASSERT(writexrefs());
	sprintf(scratch, "@I%d@", n);
	return scratch;
}
/*====================================
 * getfxref -- Return next fxref value
 *==================================*/
STRING getfxref ()
{
	INT n;
	static unsigned char scratch[10];
	ASSERT(xrefopen && nfxrefs >= 1);
	n = (nfxrefs == 1) ? fxrefs[0]++ : fxrefs[--nfxrefs];
	ASSERT(writexrefs());
	sprintf(scratch, "@F%d@", n);
	return scratch;
}
/*====================================
 * getexref -- Return next exref value
 *==================================*/
STRING getexref ()
{
	INT n;
	static unsigned char scratch[10];
	ASSERT(xrefopen && nexrefs >= 1);
	n = (nexrefs == 1) ? exrefs[0]++ : exrefs[--nexrefs];
	ASSERT(writexrefs());
	sprintf(scratch, "@E%d@", n);
	return scratch;
}
/*====================================
 * getsxref -- Return next sxref value
 *==================================*/
STRING getsxref ()
{
	INT n;
	static unsigned char scratch[10];
	ASSERT(xrefopen && nsxrefs >= 1);
	n = (nsxrefs == 1) ? sxrefs[0]++ : sxrefs[--nsxrefs];
	ASSERT(writexrefs());
	sprintf(scratch, "@S%d@", n);
	return scratch;
}
/*====================================
 * getxxref -- Return next xxref value
 *==================================*/
STRING getxxref ()
{
	INT n;
	static unsigned char scratch[10];
	ASSERT(xrefopen && nxxrefs >= 1);
	n = (nxxrefs == 1) ? xxrefs[0]++ : xxrefs[--nxxrefs];
	ASSERT(writexrefs());
	sprintf(scratch, "@X%d@", n);
	return scratch;
}
/*=============================
 * readxrefs -- Read xrefs file
 *===========================*/
BOOLEAN readxrefs ()
{
	ASSERT(xrefopen);
	ASSERT(fread(&nixrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fread(&nfxrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fread(&nexrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fread(&nsxrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fread(&nxxrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(nixrefs > 0);
	ASSERT(nfxrefs > 0);
	ASSERT(nexrefs > 0);
	ASSERT(nsxrefs > 0);
	ASSERT(nxxrefs > 0);
	if (nixrefs > maxixrefs) growixrefs();
	if (nfxrefs > maxfxrefs) growfxrefs();
	if (nexrefs > maxexrefs) growexrefs();
	if (nsxrefs > maxsxrefs) growsxrefs();
	if (nxxrefs > maxxxrefs) growxxrefs();
	ASSERT(fread(ixrefs, sizeof(INT), nixrefs, xreffp) == nixrefs);
	ASSERT(fread(fxrefs, sizeof(INT), nfxrefs, xreffp) == nfxrefs);
	ASSERT(fread(exrefs, sizeof(INT), nexrefs, xreffp) == nexrefs);
	ASSERT(fread(sxrefs, sizeof(INT), nsxrefs, xreffp) == nsxrefs);
	ASSERT(fread(xxrefs, sizeof(INT), nxxrefs, xreffp) == nxxrefs);
	return TRUE;
}
/*================================
 * writexrefs -- Write xrefs file.
 *==============================*/
writexrefs ()
{
	ASSERT(xrefopen);
	rewind(xreffp);
	ASSERT(fwrite(&nixrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fwrite(&nfxrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fwrite(&nexrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fwrite(&nsxrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fwrite(&nxxrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fwrite(ixrefs, sizeof(INT), nixrefs, xreffp) == nixrefs);
	ASSERT(fwrite(fxrefs, sizeof(INT), nfxrefs, xreffp) == nfxrefs);
	ASSERT(fwrite(exrefs, sizeof(INT), nexrefs, xreffp) == nexrefs);
	ASSERT(fwrite(sxrefs, sizeof(INT), nsxrefs, xreffp) == nsxrefs);
	ASSERT(fwrite(xxrefs, sizeof(INT), nxxrefs, xreffp) == nxxrefs);
	fflush(xreffp);
	return TRUE;
}
/*============================================
 * addixref -- Add deleted INDI key to ixrefs.
 *==========================================*/
addixref (key)
INT key;
{
	if (key <= 0 || !xrefopen || nixrefs < 1) FATAL();
	if (nixrefs >= maxixrefs) growixrefs();
	ixrefs[nixrefs++] = key;
	ASSERT(writexrefs());
}
/*===========================================
 * addfxref -- Add deleted FAM key to fxrefs.
 *=========================================*/
addfxref (key)
INT key;
{
	if (key <= 0 || !xrefopen || nfxrefs < 1) FATAL();
	if (nfxrefs >= maxfxrefs) growfxrefs();
	fxrefs[nfxrefs++] = key;
	ASSERT(writexrefs());
}
/*============================================
 * addexref -- Add deleted EVEN key to exrefs.
 *==========================================*/
addexref (key)
INT key;
{
	if (key <= 0 || !xrefopen || nexrefs < 1) FATAL();
	if (nexrefs >= maxexrefs) growexrefs();
	exrefs[nexrefs++] = key;
	ASSERT(writexrefs());
}
/*============================================
 * addsxref -- Add deleted SOUR key to sxrefs.
 *==========================================*/
addsxref (key)
INT key;
{
	if (key <= 0 || !xrefopen || nsxrefs < 1) FATAL();
	if (nsxrefs >= maxsxrefs) growsxrefs();
	sxrefs[nsxrefs++] = key;
	ASSERT(writexrefs());
}
/*=============================================
 * addfxref -- Add other deleted key to xxrefs.
 *===========================================*/
addxxref (key)
INT key;
{
	if (key <= 0 || !xrefopen || nxxrefs < 1) FATAL();
	if (nxxrefs >= maxxxrefs) growxxrefs();
	xxrefs[nxxrefs++] = key;
	ASSERT(writexrefs());
}
/*============================================
 * growixrefs -- Grow memory for ixrefs array.
 *==========================================*/
growixrefs ()
{
	INT i, m = maxixrefs, *newp;
	maxixrefs = nixrefs + 10;
	newp = (INT *) stdalloc(maxixrefs*sizeof(INT));
	if (m) {
		for (i = 0; i < nixrefs; i++)
			newp[i] = ixrefs[i];
		stdfree(ixrefs);
	}
	ixrefs = newp;
}
/*============================================
 * growfxrefs -- Grow memory for fxrefs array.
 *==========================================*/
growfxrefs ()
{
	INT i, m = maxfxrefs, *newp;
	maxfxrefs = nfxrefs + 10;
	newp = (INT *) stdalloc(maxfxrefs*sizeof(INT));
	if (m) {
		for (i = 0; i < nfxrefs; i++)
			newp[i] = fxrefs[i];
		stdfree(fxrefs);
	}
	fxrefs = newp;
}
/*============================================
 * growexrefs -- Grow memory for exrefs array.
 *==========================================*/
growexrefs ()
{
	INT i, m = maxexrefs, *newp;
	maxexrefs = nexrefs + 10;
	newp = (INT *) stdalloc(maxexrefs*sizeof(INT));
	if (m) {
		for (i = 0; i < nexrefs; i++)
			newp[i] = exrefs[i];
		stdfree(exrefs);
	}
	exrefs = newp;
}
/*============================================
 * growsxrefs -- Grow memory for sxrefs array.
 *==========================================*/
growsxrefs ()
{
	INT i, m = maxsxrefs, *newp;
	maxsxrefs = nsxrefs + 10;
	newp = (INT *) stdalloc(maxsxrefs*sizeof(INT));
	if (m) {
		for (i = 0; i < nsxrefs; i++)
			newp[i] = sxrefs[i];
		stdfree(sxrefs);
	}
	sxrefs = newp;
}
/*============================================
 * growxxrefs -- Grow memory for xxrefs array.
 *==========================================*/
growxxrefs ()
{
	INT i, m = maxxxrefs, *newp;
	maxxxrefs = nxxrefs + 10;
	newp = (INT *) stdalloc(maxxxrefs*sizeof(INT));
	if (m) {
		for (i = 0; i < nxxrefs; i++)
			newp[i] = xxrefs[i];
		stdfree(xxrefs);
	}
	xxrefs = newp;
}
/*===================================================
 * num_indis -- Return number of persons in database.
 *=================================================*/
INT num_indis ()
{
	return ixrefs[0] - nixrefs;
}
/*===================================================
 * num_fams -- Return number of families in database.
 *=================================================*/
INT num_fams ()
{
	return fxrefs[0] - nfxrefs;
}
/*==================================================
 * num_evens -- Return number of events in database.
 *================================================*/
INT num_evens ()
{
	return exrefs[0] - nexrefs;
}
/*===================================================
 * num_sours -- Return number of sources in database.
 *=================================================*/
INT num_sours ()
{
	return sxrefs[0] - nsxrefs;
}
/*=========================================================
 * num_othrs -- Return number of other records in database.
 *=======================================================*/
INT num_othrs ()
{
	return xxrefs[0] - nxxrefs;
}
