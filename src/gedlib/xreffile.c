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

#include "sys_inc.h"
#include "llstdlib.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"

extern BTREE BTR;

static BOOLEAN readxrefs(void);

/*===================================================================
 * First five words in xrefs file are number of INDI, FAM, EVEN, SOUR
 *   and other keys in file; remaining words are keys, in respective
 *   order, for the records; first in each group is next unused key;
 *   rest are keys of deleted records
 * nixrefs==1 means there are no deleted INDI keys
 * nixrefs==2 means there is one deleted INDI key (ixrefs[1])
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
static FILE *xreffp=0;	/* open xref file pointer */
static BOOLEAN xrefopen = FALSE;

/*============================== 
 * initxref -- Create xrefs file
 *============================*/
void
initxref (void)
{
	char scratch[100];
	INT i = 1, j;
	ASSERT(!xrefopen);
	sprintf(scratch, "%s/xrefs", BTR->b_basedir);
	ASSERT(xreffp = fopen(scratch, LLWRITEBINARY));
	for (j = 0; j < 10; j++) {
		ASSERT(fwrite(&i, sizeof(INT), 1, xreffp) == 1);
	}
	fclose(xreffp); xreffp=0;
}
/*============================
 * openxref -- Open xrefs file
 *==========================*/
BOOLEAN
openxref (void)
{
	char scratch[100];
	ASSERT(!xrefopen);
	sprintf(scratch, "%s/xrefs", BTR->b_basedir);
	if (!(xreffp = fopen(scratch, LLREADBINARYUPDATE))) return FALSE;
	xrefopen = TRUE;
	return readxrefs();
}
/*==============================
 * closexref -- Close xrefs file
 *============================*/
void
closexref (void)
{
	if (xreffp) {
		fclose(xreffp); xreffp = 0;
	}
	xrefopen = FALSE;
}
/*=========================================
 * getxref -- Return new keynum for type
 *  from deleted list if available, or else
 *  a new highnumber
 *  generic for all 5 types
 *=======================================*/
STRING
getxref (char ctype, INT *nxrefs, INT *xrefs)
{
	INT n;
	static unsigned char scratch[12];
	ASSERT(xrefopen && *nxrefs >= 1);
	n = (*nxrefs == 1) ? xrefs[0]++ : xrefs[--(*nxrefs)];
	ASSERT(writexrefs());
	sprintf(scratch, "@%c%d@", ctype, n);
	return scratch;
}
/*===================================================
 * get?xref -- Wrappers for each type to getxref (qv)
 *  5 symmetric versions
 *=================================================*/
STRING getixref (void)
{
	return getxref('I', &nixrefs, ixrefs);
}
STRING getfxref (void)
{
	return getxref('F', &nfxrefs, fxrefs);
}
STRING getexref (void)
{
	return getxref('E', &nexrefs, exrefs);
}
STRING getsxref (void)
{
	return getxref('S', &nsxrefs, sxrefs);
}
STRING getxxref (void)
{
	return getxref('X', &nxxrefs, xxrefs);
}
/*======================================
 * sortxref -- Sort xrefs after reading
 *====================================*/
static void
sortxref (INT nxrefs, INT * xrefs)
{
	/* they should normally already be sorted, 
	so a bubble-sort will be O(n) to verify */
	INT i,j, temp;
	for (i=1; i<nxrefs; i++) {
		for (j=i+1; j<nxrefs; j++) {
			if (xrefs[i] > xrefs[j]) {
				temp = xrefs[j];
				xrefs[j] = xrefs[i];
				xrefs[i] = temp;
			}
		}
	}
}
/*======================================
 * sortxrefs -- Sort xrefs after reading
 *====================================*/
static void
sortxrefs (void)
{
	sortxref(nixrefs, ixrefs);
	sortxref(nfxrefs, fxrefs);
	sortxref(nexrefs, exrefs);
	sortxref(nsxrefs, sxrefs);
	sortxref(nxxrefs, xxrefs);
}
/*=============================
 * readxrefs -- Read xrefs file
 *===========================*/
static BOOLEAN
readxrefs (void)
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
	ASSERT((INT)fread(ixrefs, sizeof(INT), nixrefs, xreffp) == nixrefs);
	ASSERT((INT)fread(fxrefs, sizeof(INT), nfxrefs, xreffp) == nfxrefs);
	ASSERT((INT)fread(exrefs, sizeof(INT), nexrefs, xreffp) == nexrefs);
	ASSERT((INT)fread(sxrefs, sizeof(INT), nsxrefs, xreffp) == nsxrefs);
	ASSERT((INT)fread(xxrefs, sizeof(INT), nxxrefs, xreffp) == nxxrefs);
	sortxrefs();
	return TRUE;
}
/*================================
 * writexrefs -- Write xrefs file.
 *==============================*/
BOOLEAN
writexrefs (void)
{
	ASSERT(xrefopen);
	rewind(xreffp);
	ASSERT(fwrite(&nixrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fwrite(&nfxrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fwrite(&nexrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fwrite(&nsxrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fwrite(&nxxrefs, sizeof(INT), 1, xreffp) == 1);
	ASSERT((INT)fwrite(ixrefs, sizeof(INT), nixrefs, xreffp) == nixrefs);
	ASSERT((INT)fwrite(fxrefs, sizeof(INT), nfxrefs, xreffp) == nfxrefs);
	ASSERT((INT)fwrite(exrefs, sizeof(INT), nexrefs, xreffp) == nexrefs);
	ASSERT((INT)fwrite(sxrefs, sizeof(INT), nsxrefs, xreffp) == nsxrefs);
	ASSERT((INT)fwrite(xxrefs, sizeof(INT), nxxrefs, xreffp) == nxxrefs);
	fflush(xreffp);
	return TRUE;
}
/*=====================================
 * addxref -- Add deleted key to xrefs.
 *  generic for all types
 *===================================*/
void
addxref (INT key, INT *nxrefs, INT maxxrefs, INT *xrefs, void (*growfnc)(void))
{
	INT lo,hi,md, i;
	if (key <= 0 || !xrefopen || (*nxrefs) < 1) FATAL();
	if (*nxrefs >= maxxrefs)
		(*growfnc)();
	ASSERT(*nxrefs < maxxrefs);
	lo=1;
	hi=(*nxrefs)-1;
	/* binary search to find where to insert key */
	while (lo<=hi) {
		md = (lo + hi)/2;
		if (key<xrefs[md])
			hi=--md;
		else if (key>xrefs[md])
			lo=++md;
		else
			FATAL(); /* deleting a deleted record! */
	}
	/* key replaces xrefs[lo] - push lo+ up */
	for (i=lo; i<*nxrefs; i++)
		xrefs[i+1] = xrefs[i];
	xrefs[lo] = key;
	(*nxrefs)++;
	ASSERT(writexrefs());
}
/*===================================================
 * add?xref -- Wrappers for each type to addxref (qv)
 *  5 symmetric versions
 *=================================================*/
void addixref (INT key)
{
	addxref(key, &nixrefs, maxixrefs, ixrefs, &growixrefs);
}
void addfxref (INT key)
{
	addxref(key, &nfxrefs, maxfxrefs, fxrefs, &growfxrefs);
}
void addexref (INT key)
{
	addxref(key, &nexrefs, maxexrefs, exrefs, &growexrefs);
}
void addsxref (INT key)
{
	addxref(key, &nsxrefs, maxsxrefs, sxrefs, &growsxrefs);
}
void addxxref (INT key)
{
	addxref(key, &nxxrefs, maxxxrefs, xxrefs, &growxxrefs);
}
/*==========================================
 * growxrefs -- Grow memory for xrefs array.
 *  generic for all types
 *========================================*/
static void
growxrefs (INT nxrefs, INT *maxxrefs, INT **xrefs)
{
	INT i, m = *maxxrefs, *newp;
	*maxxrefs = nixrefs + 10;
	newp = (INT *) stdalloc((*maxxrefs)*sizeof(INT));
	if (m) {
		for (i = 0; i < nxrefs; i++)
			newp[i] = (*xrefs)[i];
		stdfree(*xrefs);
	}
	(*xrefs) = newp;
}
/*======================================================
 * grow?xrefs -- Wrappers for each type to growxrefs (qv)
 *  5 symmetric versions
 *=====================================================*/
void growixrefs (void)
{
	growxrefs(nixrefs, &maxixrefs, &ixrefs);
}
void growfxrefs (void)
{
	growxrefs(nfxrefs, &maxfxrefs, &fxrefs);
}
void growexrefs (void)
{
	growxrefs(nexrefs, &maxexrefs, &exrefs);
}
void growsxrefs (void)
{
	growxrefs(nsxrefs, &maxsxrefs, &sxrefs);
}
void growxxrefs (void)
{
	growxrefs(nxxrefs, &maxxxrefs, &xxrefs);
}
/*==========================================================
 * num_????s -- Return number of type of things in database.
 *  5 symmetric versions
 *========================================================*/
INT num_indis (void)
{
	return ixrefs[0] - nixrefs;
}
INT num_fams (void)
{
	return fxrefs[0] - nfxrefs;
}
INT num_evens (void)
{
	return exrefs[0] - nexrefs;
}
INT num_sours (void)
{
	return sxrefs[0] - nsxrefs;
}
INT num_othrs (void)
{
	return xxrefs[0] - nxxrefs;
}
/*================================================
 * newixref -- Return original or next ixref value
 *==============================================*/
STRING
newixref (STRING xrefp, /* key of the individual */
          BOOLEAN flag) /* use the current key */
{
	INT n;
	BOOLEAN changed;
	static unsigned char scratch[12];
	if(flag) {
		n = atoi(xrefp+1);
		changed = ((nixrefs != 1) || (n >= ixrefs[0]));
		if(nixrefs != 1) nixrefs = 1;	/* forget about deleted entries */
		if(n >= ixrefs[0]) ixrefs[0] = n+1;	/* next available */
		if(changed) ASSERT(writexrefs());
		sprintf(scratch, "@%s@", xrefp);
		return(scratch);
	}
	return(getixref());
}
/*================================================
 * newfxref -- Return original or next fxref value
 *==============================================*/
STRING
newfxref (STRING xrefp, /* key of the individual */
          BOOLEAN flag) /* use the current key */
{
	INT n;
	BOOLEAN changed;
	static unsigned char scratch[12];
	if(flag) {
		n = atoi(xrefp+1);
		changed = ((nfxrefs != 1) || (n >= fxrefs[0]));
		nfxrefs = 1;	/* forget about deleted entries */
		if(n >= fxrefs[0]) fxrefs[0] = n+1;	/* next available */
		if(changed) ASSERT(writexrefs());
		sprintf(scratch, "@%s@", xrefp);
		return(scratch);
	}
	return(getfxref());
}
/*================================================
 * newsxref -- Return original or next sxref value
 *==============================================*/
STRING
newsxref (STRING xrefp, /* key of the individual */
          BOOLEAN flag) /* use the current key */
{
	INT n;
	BOOLEAN changed;
	static unsigned char scratch[12];
	if(flag) {
		n = atoi(xrefp+1);
		changed = ((nsxrefs != 1) || (n >= sxrefs[0]));
		nsxrefs = 1;	/* forget about deleted entries */
		if(n >= sxrefs[0]) sxrefs[0] = n+1;	/* next available */
		if(changed) ASSERT(writexrefs());
		sprintf(scratch, "@%s@", xrefp);
		return(scratch);
	}
	return(getsxref());
}
/*================================================
 * newexref -- Return original or next exref value
 *==============================================*/
STRING
newexref (STRING xrefp, /* key of the individual */
          BOOLEAN flag) /* use the current key */
{
	INT n;
	BOOLEAN changed;
	static unsigned char scratch[12];
	if(flag) {
		n = atoi(xrefp+1);
		changed = ((nexrefs != 1) || (n >= exrefs[0]));
		nexrefs = 1;	/* forget about deleted entries */
		if(n >= exrefs[0]) exrefs[0] = n+1;	/* next available */
		if(changed) ASSERT(writexrefs());
		sprintf(scratch, "@%s@", xrefp);
		return(scratch);
	}
	return(getexref());
}
/*================================================
 * newxxref -- Return original or next xxref value
 *==============================================*/
STRING
newxxref (STRING xrefp, /* key of the individual */
          BOOLEAN flag) /* use the current key */
{
	INT n;
	BOOLEAN changed;
	static unsigned char scratch[12];
	if(flag) {
		n = atoi(xrefp+1);
		changed = ((nxxrefs != 1) || (n >= xxrefs[0]));
		nxxrefs = 1;	/* forget about deleted entries */
		if(n >= xxrefs[0]) xxrefs[0] = n+1;	/* next available */
		if(changed) ASSERT(writexrefs());
		sprintf(scratch, "@%s@", xrefp);
		return(scratch);
	}
	return(getxxref());
}
/*================================================
 * xref_isvalid_impl -- is this a valid whatever ?
 *  generic for all 5 types
 * (internal use)
 *==============================================*/
static INT
xref_isvalid_impl(INT nxrefs, INT * xrefs, INT i)
{
	int j;
	if (nxrefs == xrefs[0]) return 0; /* no valids */
	/*
	TO DO: 
	Change this to a binary sort, when xrefs sorting
	has been tested.
	Perry, 2001/01/01
	*/
	for (j=1; j< nxrefs; j++)
		if (i == xrefs[j])
			return 0;
	return 1;
}
/*====================================================
 * xref_next -- Return next valid of some type after i
 *  returns 0 if none found
 *  generic for all 5 types
 *==================================================*/
static INT
xref_next(INT nxrefs, INT * xrefs, INT i)
{
	if (nxrefs == xrefs[0]) return 0; /* no valids */
	while (++i < xrefs[0])
	{
		if (xref_isvalid_impl(nxrefs, xrefs, i)) return i;
	}
	return 0;
}
/*=====================================================
 * xref_prev -- Return prev valid of some type before i
 *  returns 0 if none found
 *  generic for all 5 types
 *===================================================*/
static INT
xref_prev(INT nxrefs, INT * xrefs, INT i)
{
	if (nxrefs == xrefs[0]) return 0; /* no valids */
	while (--i)
	{
		if (xref_isvalid_impl(nxrefs, xrefs, i)) return i;
	}
	return 0;
}
/*===============================================
 * xref_next? -- Return next valid indi/? after i
 *  returns 0 if none found
 *  5 symmetric versions
 *=============================================*/
INT xref_nexti (INT i) 
{
	return xref_next(nixrefs, ixrefs, i); 
}
INT xref_nextf (INT i)
{
	return xref_next(nfxrefs, fxrefs, i);
}
INT xref_nexte (INT i)
{
	return xref_next(nexrefs, exrefs, i);
}
INT xref_nexts (INT i)
{
	return xref_next(nsxrefs, sxrefs, i);
}
INT xref_nextx (INT i)
{
	return xref_next(nxxrefs, xxrefs, i);
}
/*================================================
 * xref_prev? -- Return prev valid indi/? before i
 *  returns 0 if none found
 *  5 symmetric versions
 *==============================================*/
INT xref_previ(INT i)
{
	return xref_prev(nixrefs, ixrefs, i);
}
INT xref_prevf (INT i)
{
	return xref_prev(nfxrefs, fxrefs, i);
}
INT xref_preve (INT i)
{
	return xref_prev(nexrefs, exrefs, i);
}
INT xref_prevs (INT i)
{
	return xref_prev(nsxrefs, sxrefs, i);
}
INT xref_prevx (INT i)
{
	return xref_prev(nxxrefs, xxrefs, i);
}
/*=========================================
 * xref_first? -- Return first valid indi/?
 *  returns 0 if none found
 *  5 symmetric versions
 *=======================================*/
INT xref_firsti (void)
{
	return xref_nexti(0);
}
INT xref_firstf (void)
{
	return xref_nextf(0);
}
INT xref_firste (void)
{
	return xref_nexte(0);
}
INT xref_firsts (void)
{
	return xref_nexts(0);
}
INT xref_firstx (void)
{
	return xref_nextx(0);
}
/*=======================================
 * xref_last? -- Return last valid indi/?
 *  returns 0 if none found
 *  5 symmetric versions
 *=====================================*/
INT xref_lasti (void)
{
	return xref_previ(ixrefs[0]);
}
INT xref_lastf (void)
{
	return xref_prevf(fxrefs[0]);
}
INT xref_laste (void)
{
	return xref_prevf(exrefs[0]);
}
INT xref_lasts (void)
{
	return xref_prevf(sxrefs[0]);
}
INT xref_lastx (void)
{
	return xref_prevf(xxrefs[0]);
}
