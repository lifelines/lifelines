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
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 02 Sep 93
 *   3.0.0 - 11 Sep 94    3.0.2 - 13 Dec 94
 *   3.0.3 - 23 Jul 96
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "liflines.h"

#define SS (STRING)

extern STRING badind,badfmc,badfms,badfam,badhsb,badwif,badchl;
extern STRING bademp,badin0,badfm0,badsr0,badev0,badothr0;
extern STRING badmul,badnnm,badenm,badpsx,badirf;

/*===================================
 * valid_indi -- Validate person tree
 *  indi1:  [IN]  person to validate
 *  pmsg:   [OUT] error message, if any
 *  orig:   [IN]  person to match - may be NULL
 * rtn: FALSE for bad
 *=================================*/
BOOLEAN
valid_indi (NODE indi1, STRING *pmsg, NODE orig)
{
	NODE name1, refn1, sex1, body1, famc1, fams1, node;
	NODE name0, refn0, sex0, body0, famc0, fams0;
	INT isex, num;
	STRING *keys, ukey;

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
	if (orig)
		split_indi(orig, &name0, &refn0, &sex0, &body0, &famc0,
		    &fams0);
	if (orig && !iso_nodes(indi1, orig, FALSE, FALSE)) {
		*pmsg = badind; 
		goto bad1;
	}
	if (!iso_nodes(famc1, famc0, FALSE, TRUE)) {
		*pmsg = badfmc;
		goto bad1;
	}
	if (!iso_nodes(fams1, fams0, FALSE, TRUE)) {
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
	if (num > 1 || (num == 1 && (!orig ||
	    nestr(keys[0], rmvat(nxref(indi1)))))) {
		*pmsg = badirf;
		goto bad1;
	}
	if (orig)
		join_indi(orig, name0, refn0, sex0, body0, famc0, fams0);
	join_indi(indi1, name1, refn1, sex1, body1, famc1, fams1);
	return TRUE;
bad1:
	if (orig)
		join_indi(orig, name0, refn0, sex0, body0, famc0, fams0);
bad2:
	join_indi(indi1, name1, refn1, sex1, body1, famc1, fams1);
	return FALSE;
}
/*===============================
 * valid_fam -- Validate FAM tree
 *  fam1,  [IN]  family to validate
 *  pmsg:  [OUT] error message, if any
 *  fam0:  [IN]  family to match - may be NULL
 *=============================*/
BOOLEAN
valid_fam (NODE fam1, STRING *pmsg, NODE fam0)
{
	NODE refn0, husb0, wife0, chil0, body0;
	NODE refn1, husb1, wife1, chil1, body1;

	if (!fam1) {
		*pmsg = bademp;
  		return FALSE;
	}
	if (nestr("FAM", ntag(fam1))) {
		*pmsg = badfm0;
		return FALSE;
	}
	if (nsibling(fam1)) {
		*pmsg = badmul;
		return FALSE;
	}

	refn0 = husb0 = wife0 = chil0 = body0 = NULL;
	if (fam0)
		split_fam(fam0, &refn0, &husb0, &wife0, &chil0, &body0);
	split_fam(fam1, &refn1, &husb1, &wife1, &chil1, &body1);
	
	if (fam0 && !iso_nodes(fam1, fam0, FALSE, TRUE)) {
		*pmsg = badfam; 
		goto bad3;
	}
	if (!iso_nodes(husb1, husb0, FALSE, TRUE)) {
		*pmsg = badhsb;
		goto bad3;
	}
	if (!iso_nodes(wife1, wife0, FALSE, TRUE)) {
		*pmsg = badwif;
		goto bad3;
	}
	if (!iso_nodes(chil1, chil0, FALSE, TRUE)) {
		*pmsg = badchl;
		goto bad3;
	}
	if (fam0)
		join_fam(fam0, refn0, husb0, wife0, chil0, body0);
	join_fam(fam1, refn1, husb1, wife1, chil1, body1);
	return TRUE;
bad3:
	if (fam0)
		join_fam(fam0, refn0, husb0, wife0, chil0, body0);
	join_fam(fam1, refn1, husb1, wife1, chil1, body1);
	return FALSE;
}
/*============================
 * valid_name -- Validate name
 *==========================*/
BOOLEAN
valid_name (STRING name)
{
	INT c, n = 0;
	if (!name) return FALSE;
	if (pointer_value(name)) return FALSE;
	while ((c = *name++)) {
		if (c == NAMESEP) n++;
	}
	return n <= 2;
}
/*======================================
 * valid_node_type -- Validate top-level node tree
 *  node:   [IN]  node to validate
 *  ntype:  [IN]  I/F/S/E/X
 *  pmsg,   [OUT] error message, if any
 *  orig:   [IN]  node to match (may be null)
 *====================================*/
BOOLEAN
valid_node_type (NODE node, char ntype, STRING *pmsg, NODE node0)
{
	switch(ntype) {
	case 'I': return valid_indi(node, pmsg, node0);
	case 'F': return valid_fam(node, pmsg, node0);
	case 'S': return valid_sour_tree(node, pmsg, node0);
	case 'E': return valid_even_tree(node, pmsg, node0);
	default: return valid_othr_tree(node, pmsg, node0);
	}
}
/*======================================
 * valid_sour_tree -- Validate SOUR tree
 *  node:  [IN]  source to validate 
 *  pmsg:  [OUT] error message, if any 
 *  orig:  [IN]  SOUR node to match 
 *====================================*/
BOOLEAN
valid_sour_tree (NODE node, STRING *pmsg, NODE orig)
{
	orig = NULL;         /* keep compiler happy */
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
 *  node:  [IN]  source to validate
 *  pmsg,  [OUT] error message, if any
 *  orig:  [IN]  EVEN node to match
 *====================================*/
BOOLEAN
valid_even_tree (NODE node, STRING *pmsg, NODE orig)
{
	orig = NULL;         /* keep compiler happy */
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
 *  node:  [IN]  source to validate
 *  pmsg,  [OUT] error message, if any
 *  orig:  [IN]  OTHR node to match
 *====================================*/
BOOLEAN
valid_othr_tree (NODE node, STRING *pmsg, NODE orig)
{
	orig = NULL;         /* keep compiler happy */
	*pmsg = NULL;
	if (!node) {
		*pmsg = bademp;
  		return FALSE;
	}
	if (eqstr("INDI", ntag(node)) || eqstr("FAM", ntag(node))
		|| eqstr("EVEN", ntag(node)) || eqstr("SOUR", ntag(node))) {
		*pmsg = badothr0;
		return FALSE;
	}
	return TRUE;
}
/*=========================================
 * pointer_value -- See if value is pointer
 *=======================================*/
BOOLEAN
pointer_value (STRING val)
{
	if (!val || *val != '@' || strlen(val) < 3) return FALSE;
	return val[strlen(val)-1] == '@';
}
