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
 * valid.c -- Record validation functions
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 02 Sep 93
 *   3.0.0 - 11 Sep 94    3.0.2 - 13 Dec 94
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "gedcom.h"

#define SS (STRING)

STRING badind = SS "You cannot edit the INDI line in a person record.";
STRING badfmc = SS "You cannot edit the FAMC line in a person record.";
STRING badfms = SS "You cannot edit the FAMS lines in a person record.";
STRING badfam = SS "You cannot edit the FAM line in a family record.";
STRING badhsb = SS "You cannot edit the HUSB line in a family record.";
STRING badwif = SS "You cannot edit the WIFE line in a family record.";
STRING badchl = SS "You cannot edit the CHIL lines in a family record.";
STRING bademp = SS "The record is empty.";
STRING badin0 = SS "The record does not begin with an INDI line.";
STRING badfm0 = SS "The record does not begin with a FAM line";
STRING badsr0 = SS "The record does not begin with a SOUR line";
STRING badev0 = SS "The record does not begin with an EVEN line";
STRING badmul = SS "The record contains multiple level 0 lines.";
STRING badnnm = SS "This person record does not have a name line.";
STRING badenm = SS "This person record has bad GEDCOM name syntax.";
STRING badpsx = SS "You cannot change the sex of a parent.";
STRING badirf = SS "This person's REFN key is already in use.";

/*======================================
 * valid_indi_tree -- Validate INDI tree
 *====================================*/
BOOLEAN valid_indi_tree (indi1, pmsg, indi0)
NODE indi1;	/* person to validate */
STRING *pmsg;	/* error message, if any */
NODE indi0;	/* INDI node to match */
{
	NODE name1, refn1, sex1, body1, famc1, fams1, node;
	NODE name0, refn0, sex0, body0, famc0, fams0;
	INT isex, num;
	STRING **keys, ukey;

	if (!indi1) {
		*pmsg = bademp;
  		return FALSE;
	}
	if (nestr("INDI", ntag(indi1))) {
		*pmsg = badin0;
		return FALSE;
	}
	if (nsibling(indi1)) {
		*pmsg = badmul;
		return FALSE;
	}
	split_indi(indi1, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	if (!name1) {
		*pmsg = badnnm;
		goto bad2;
	}
	for (node = name1; node; node = nsibling(node)) {
		if (!valid_name(nval(node))) {
			*pmsg = badenm;
			goto bad2;
		}
	}
	name0 = refn0 = sex0 = body0 = famc0 = fams0 = NULL;
	if (indi0)
		split_indi(indi0, &name0, &refn0, &sex0, &body0, &famc0,
		    &fams0);
	if (indi0 && !iso_list(indi1, indi0)) {
		*pmsg = badind; 
		goto bad1;
	}
	if (!iso_list(famc1, famc0)) {
		*pmsg = badfmc;
		goto bad1;
	}
	if (!iso_list(fams1, fams0)) {
		*pmsg = badfms; 
		goto bad1;
	}
	isex = val_to_sex(sex0);
	if (!fams0) isex = SEX_UNKNOWN;
	if (isex != SEX_UNKNOWN && isex != val_to_sex(sex1)) {
		*pmsg = badpsx;
		goto bad1;
	}
	ukey = (refn1 ? nval(refn1) : NULL);
	get_refns(ukey, &num, &keys, 'I');
	if (num > 1 || (num == 1 && nestr(keys[0], rmvat(nxref(indi1))))) {
		*pmsg = badirf;
		goto bad1;
	}
	if (indi0)
		join_indi(indi0, name0, refn0, sex0, body0, famc0, fams0);
	join_indi(indi1, name1, refn1, sex1, body1, famc1, fams1);
	return TRUE;
bad1:
	if (indi0)
		join_indi(indi0, name0, refn0, sex0, body0, famc0, fams0);
bad2:
	join_indi(indi1, name1, refn1, sex1, body1, famc1, fams1);
	return FALSE;
}
/*====================================
 * valid_fam_tree -- Validate FAM tree
 *==================================*/
BOOLEAN valid_fam_tree (fam, pmsg, fam0, husb0, wife0, chil0)
NODE fam;
STRING *pmsg;
NODE fam0, husb0, wife0, chil0;
{
	NODE husb, wife, chil, rest, fref;

	if (!fam) {
		*pmsg = bademp;
  		return FALSE;
	}
	if (nestr("FAM", ntag(fam))) {
		*pmsg = badfm0;
		return FALSE;
	}
	if (nsibling(fam)) {
		*pmsg = badmul;
		return FALSE;
	}
	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
	if (fam0 && !iso_list(fam, fam0)) {
		*pmsg = badfam; 
		join_fam(fam, fref, husb, wife, chil, rest);
		return FALSE;
	}
	if (!iso_list(husb, husb0)) {
		*pmsg = badhsb;
		join_fam(fam, fref, husb, wife, chil, rest);
		return FALSE;
	}
	if (!iso_list(wife, wife0)) {
		*pmsg = badwif;
		join_fam(fam, fref, husb, wife, chil, rest);
		return FALSE;
	}
	if (!iso_list(chil, chil0)) {
		*pmsg = badchl;
		join_fam(fam, fref, husb, wife, chil, rest);
		return FALSE;
	}
	join_fam(fam, fref, husb, wife, chil, rest);
	return TRUE;
}
/*============================
 * valid_name -- Validate name
 *==========================*/
BOOLEAN valid_name (name)
STRING name;
{
	INT c, n = 0;
	if (!name) return FALSE;
	while (c = *name++) {
		if (c == '/') n++;
	}
	return n <= 2;
}
/*======================================
 * valid_sour_tree -- Validate SOUR tree
 *====================================*/
BOOLEAN valid_sour_tree (node, pmsg, node0)
NODE node;	/* source to validate */
STRING *pmsg;	/* error message, if any */
NODE node0;	/* SOUR node to match */
{
	*pmsg = NULL;
	if (!node) {
		*pmsg = bademp;
  		return FALSE;
	}
	if (nestr("SOUR", ntag(node))) {
		*pmsg = badsr0;
		return FALSE;
	}
	return TRUE;
}
/*======================================
 * valid_even_tree -- Validate EVEN tree
 *====================================*/
BOOLEAN valid_even_tree (node, pmsg, node0)
NODE node;	/* source to validate */
STRING *pmsg;	/* error message, if any */
NODE node0;	/* EVEN node to match */
{
	*pmsg = NULL;
	if (!node) {
		*pmsg = bademp;
  		return FALSE;
	}
	if (nestr("EVEN", ntag(node))) {
		*pmsg = badsr0;
		return FALSE;
	}
	return TRUE;
}
/*======================================
 * valid_othr_tree -- Validate OTHR tree
 *====================================*/
BOOLEAN valid_othr_tree (node, pmsg, node0)
NODE node;	/* source to validate */
STRING *pmsg;	/* error message, if any */
NODE node0;	/* OTHR node to match */
{
	*pmsg = NULL;
	if (!node) {
		*pmsg = bademp;
  		return FALSE;
	}
	return TRUE;
}
