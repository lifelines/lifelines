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

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "feedback.h"

/*=================================================
 * choose_child -- Choose child of person or family
 *  irec: [IN] parent (may be null if fam provided)
 *  frec:  [IN] family (may be null if indi provided)
 *  msg0: [IN] message to display if no children
 *  msgn: [IN] title for choosing child from list
 *  ask1: [IN] whether to prompt if only one child
 *===============================================*/
RECORD
choose_child (RECORD irec, RECORD frec, STRING msg0, STRING msgn, ASK1Q ask1)
{
	RECORD rec=0;
	INDISEQ seq=0;

	if (irec) seq = indi_to_children(nztop(irec));
	if (!irec && frec) seq = fam_to_children(nztop(frec));
	if (!seq) {
		msg_error(msg0);
		return NULL;
	}
	rec = choose_from_indiseq(seq, ask1, msgn, msgn);
	remove_indiseq(seq);
	return rec;
}
/*========================================
 * choose_spouse -- Choose person's spouse
 *  irec: [IN]  known person (gives up if this is null)
 *  msg0: [IN] message to display if no spouses
 *  msgn: [IN] title for choosing spouse from list
 *  asks if multiple
 *======================================*/
RECORD
choose_spouse (RECORD irec, STRING msg0, STRING msgn)
{
	RECORD rec=0;
	INDISEQ seq=0;

	if (!irec) return NULL;
	if (!(seq = indi_to_spouses(nztop(irec)))) {
		message(msg0);
		return NULL;
	}
	rec = choose_from_indiseq(seq, NOASK1, NULL, msgn);
	remove_indiseq(seq);
	return rec;
}
/*========================================
 * choose_source -- Choose any referenced source from some,
 *  presumably top level, node
 *  always asks
 *======================================*/
RECORD
choose_source (RECORD current, STRING msg0, STRING msgn)
{
	INDISEQ seq;
	RECORD rec;
	if (!current) return NULL;
	if (!(seq = node_to_sources(nztop(current)))) {
		message(msg0);
		return NULL;
	}
	rec = choose_from_indiseq(seq, DOASK1, msgn, msgn);
	remove_indiseq(seq);
	return rec;
}
/*========================================
 * choose_note -- Choose any referenced note from some,
 *  presumably top level, node
 *  always asks
 * Created: 2001/02/11, Perry Rapp
 *======================================*/
RECORD
choose_note (RECORD current, STRING msg0, STRING msgn)
{
	INDISEQ seq;
	RECORD rec;
	if (!current) return NULL;
	if (!(seq = node_to_notes(nztop(current)))) {
		message(msg0);
		return NULL;
	}
	rec = choose_from_indiseq(seq, DOASK1, msgn, msgn);
	remove_indiseq(seq);
	return rec;
}
/*========================================
 * choose_pointer -- Choose any reference (pointer) from some,
 *  presumably top level, node
 *  always asks
 * Created: 2001/02/24, Perry Rapp
 * Returns addref'd record
 *======================================*/
RECORD
choose_pointer (RECORD current, STRING msg0, STRING msgn)
{
	INDISEQ seq;
	RECORD rec;
	if (!current) return NULL;
	if (!(seq = node_to_pointers(nztop(current)))) {
		message(msg0);
		return NULL;
	}
	rec = choose_from_indiseq(seq, DOASK1, msgn, msgn);
	remove_indiseq(seq);
	return rec;
}
/*==========================================================
 * choose_family -- Choose family from person's FAMS/C lines
 *  asks if multiple
 * irec: [IN]  person of interest
 * msg0: [IN]  message to display if no families
 * msgn: [IN]  title if need to choose which family
 * fams: [IN]  want spousal families of indi ? (or families indi is child in)
 *========================================================*/
RECORD
choose_family (RECORD irec, STRING msg0, STRING msgn, BOOLEAN fams)
{
	RECORD rec=0;
	INDISEQ seq = indi_to_families(nztop(irec), fams);
	if (!seq) {
		if (msg0)
			msg_error(msg0);
		return NULL;
	}
	rec = choose_from_indiseq(seq, NOASK1, NULL, msgn);
	remove_indiseq(seq);
	return rec;
}
/*===================================================
 * choose_father -- Choose father of person or family
 * irec: [IN]  person of interest if non-null
 * frec: [IN]  family of interest if non-null
 * msg0: [IN]  message to display if no fathers
 * msgn: [IN]  title if need to choose which father
 * ask1: [IN]  whether or not to prompt if only one father found
 *=================================================*/
RECORD
choose_father (RECORD irec, RECORD frec, STRING msg0, STRING msgn, ASK1Q ask1)
{
	RECORD rec=0;
	INDISEQ seq=0;

	if (irec) seq = indi_to_fathers(nztop(irec));
	if (!irec && frec) seq = fam_to_fathers(nztop(frec));
	if (!seq) {
		msg_error(msg0);
		return NULL;
	}
	rec = choose_from_indiseq(seq, ask1, msgn, msgn);
	remove_indiseq(seq);
	return rec;
}
/*===================================================
 * choose_mother -- Choose mother of person or family
 * irec: [IN]  person of interest if non-null
 * frec: [IN]  family of interest if non-null
 * msg0: [IN]  message to display if no mothers
 * msgn: [IN]  title if need to choose which mother
 * ask1: [IN]  whether or not to prompt if only one mother found
 *=================================================*/
RECORD
choose_mother (RECORD irec, RECORD frec, STRING msg0, STRING msgn, ASK1Q ask1)
{
	RECORD rec=0;
	INDISEQ seq=0;

	if (irec) seq = indi_to_mothers(nztop(irec));
	if (!irec && frec) seq = fam_to_mothers(nztop(frec));
	if (!seq) {
		msg_error(msg0);
		return NULL;
	}
	rec = choose_from_indiseq(seq, ask1, msgn, msgn);
	remove_indiseq(seq);
	return rec;
}
