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
/*============================================================
 * choose.c -- Implements the choose operations
 * Copyright(c) 1992-4 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 06 Dec 94    3.0.3 - 08 May 95
 *==========================================================*/

#include "standard.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"

/*=================================================
 * choose_child -- Choose child of person or family
 *===============================================*/
NODE choose_child (indi, fam, msg0, msgn, ask1)
NODE indi, fam;
STRING msg0, msgn;
BOOLEAN ask1;
{
	INDISEQ seq = NULL;
	NODE node;
	INT i = 0;
	if (indi) seq = indi_to_children(indi);
	if (!indi && fam) seq = fam_to_children(fam);
	if (!seq) {
		message(msg0);
		return NULL;
	}
	node = format_and_choose_indi(seq, FALSE, FALSE, ask1, msgn, msgn);
	remove_indiseq(seq, FALSE);
	return node;
}
/*========================================
 * choose_spouse -- Choose person's spouse
 *======================================*/
NODE choose_spouse (indi, msg0, msgn)
NODE indi;
STRING msg0, msgn;
{
	INDISEQ seq;
	NODE node;
	INT i = 0, len;
	if (!indi) return NULL;
	if (!(seq = indi_to_spouses(indi))) {
		message(msg0);
		return NULL;
	}
	node = format_and_choose_indi(seq, FALSE, TRUE, FALSE, NULL, msgn);
	remove_indiseq(seq, FALSE);
	return node;
}
/*==========================================================
 * choose_family -- Choose family from person's FAMS/C lines
 *========================================================*/
NODE choose_family (indi, msg0, msgn, fams)
NODE indi;
STRING msg0, msgn;
BOOLEAN fams;
{
	NODE node, fam, spouse;
	INT i = 0, len;
	char scratch[12];
	INDISEQ seq = indi_to_families(indi, fams);
	if (!seq) {
		message(msg0);
		return NULL;
	}
	node = format_and_choose_indi(seq, TRUE, FALSE, FALSE, NULL, msgn);
	remove_indiseq(seq, FALSE);
	return node;
}
/*===================================================
 * choose_father -- Choose father of person or family
 *=================================================*/
NODE choose_father (indi, fam, msg0, msgn, ask1)
NODE indi, fam;
STRING msg0, msgn;
BOOLEAN ask1;
{
	INDISEQ seq = NULL;
	NODE node;
	INT i = 0;
	if (indi) seq = indi_to_fathers(indi);
	if (!indi && fam) seq = fam_to_fathers(fam);
	if (!seq) {
		message(msg0);
		return NULL;
	}
	node = format_and_choose_indi(seq, FALSE, FALSE, ask1, msgn, msgn);
	remove_indiseq(seq, FALSE);
	return node;
}
/*===================================================
 * choose_mother -- Choose mother of person or family
 *=================================================*/
NODE choose_mother (indi, fam, msg0, msgn, ask1)
NODE indi, fam;
STRING msg0, msgn;
BOOLEAN ask1;
{
	INDISEQ seq = NULL;
	NODE node;
	INT i = 0;
	if (indi) seq = indi_to_mothers(indi);
	if (!indi && fam) seq = fam_to_mothers(fam);
	if (!seq) {
		message(msg0);
		return NULL;
	}
	node = format_and_choose_indi(seq, FALSE, FALSE, ask1, msgn, msgn);
	remove_indiseq(seq, FALSE);
	return node;
}
