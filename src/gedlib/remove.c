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
 * remove.c -- Remove child or spouse from family
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 23 Sep 94    3.0.2 - 05 Dec 94
 *   3.0.3 - 15 Aug 95
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "feedback.h"

extern STRING haslnk;


/*================================================================
 * remove_indi -- Delete person and links; if this leaves families
 *   with no links, remove them.
 *   This should not fail.
 *  indi:  [in]  person to remove - (must be valid)
 * Created: 2001/11/08, Perry Rapp
 *==============================================================*/
void
remove_indi (NODE indi)
{
	STRING key;
	NODE name, refn, sex, body, famc, fams, this;
	NODE node, husb, wife, chil, rest, fam, prev, next, fref;
	INT isex, keyint;
	BOOLEAN found;

	split_indi_old(indi, &name, &refn, &sex, &body, &famc, &fams);
	if (!fams) goto checkfamc;
	isex = val_to_sex(sex);
	ASSERT(isex != SEX_UNKNOWN);

/* Remove person from families he/she is in as a parent */

	for (node = fams; node; node = nsibling(node)) {
		fam = key_to_fam(rmvat(nval(node)));
		split_fam(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		if (isex == SEX_MALE) this = husb;
		else this = wife;
		found = FALSE;
		while (this) {
			if (eqstr(nxref(indi), nval(this))) {
				found = TRUE;
				break;
			}
			prev = this;
			this = nsibling(this);
		}
		if(found) {
			next = nsibling(this);
			if (prev)
				nsibling(prev) = next;
			else if (isex == SEX_MALE) husb = next;
			else wife = next;
			nsibling(this) = NULL;
			free_nodes(this);
		}
		join_fam(fam, fref, husb, wife, chil, rest);
		if (husb || wife || chil)
			fam_to_dbase(fam);
		else
			remove_empty_fam(fam);
	}

/* Remove person from families he/she is in as a child */

checkfamc:
	for (node = famc; node; node = nsibling(node)) { 
		fam = key_to_fam(rmvat(nval(node)));
		split_fam(fam, &fref, &husb, &wife, &chil, &rest);
		found = FALSE;
		prev = NULL;
		this = chil;
		while (this) {
			if (eqstr(nxref(indi), nval(this))) {
				found = TRUE;
				break;
			}
			prev = this;
			this = nsibling(this);
		}
		if(found) {
			next = nsibling(this);
			if (prev)
				nsibling(prev) = next;
			else
				chil = next;
			nsibling(this) = NULL;
			free_nodes(this);
		}
		join_fam(fam, fref, husb, wife, chil, rest);
		if (husb || wife || chil)
			fam_to_dbase(fam);
		else
			remove_empty_fam(fam);
	}
	key = rmvat(nxref(indi));
	keyint = atoi(key + 1);
	addixref(keyint);
	remove_indi_cache(key);
	for (node = name; node; node = nsibling(node))
		{
		remove_name(nval(node), key);
		}
	for (node = refn; node; node = nsibling(node))
		{
		if (nval(node))
			{
			remove_refn(nval(node), key);
			}
		}
	remove_from_browse_lists(key);
	del_in_dbase(key);
	join_indi(indi, name, refn, sex, body, famc, fams);
	free_nodes(indi);
}
/*==========================================
 * remove_empty_fam -- Delete family from database
 *  This will call message & fail if there are any
 *  people in the family.
 *========================================*/
BOOLEAN
remove_empty_fam (NODE fam)
{
	STRING key;
	NODE node, husb, wife, chil, rest, refn;
	INT keyint;
	if (!fam) return TRUE;
	split_fam(fam, &refn, &husb, &wife, &chil, &rest);
	if (husb || wife || chil) {
		/* TO DO - This probably should never happen, and maybe could be
		changed to an assertion, 2001/11/08, Perry, but I've not checked
		merge code's call */
		message(haslnk);
		join_fam(fam, refn, husb, wife, chil, rest);
		return FALSE;
	}
	key = rmvat(nxref(fam));
	keyint = atoi(key + 1);
	addfxref(keyint);
	for (node = refn; node; node = nsibling(node))
		if (nval(node)) remove_refn(nval(node), key);
	remove_fam_cache(key);
	del_in_dbase(key);
	join_fam(fam, refn, husb, wife, chil, rest);
	free_nodes(fam);
	return TRUE;
}
/*=========================================
 * remove_child -- Remove child from family
 *  silent function
 *=======================================*/
BOOLEAN
remove_child (NODE indi, NODE fam)
{
	NODE node, last;

/* Make sure child is in family and remove his/her CHIL line */
	if (!(node = find_node(fam, "CHIL", nxref(indi), &last))) {
		return FALSE;
	}
	if (last)
		nsibling(last) = nsibling(node);
	else
		nchild(fam) = nsibling(node);
	free_node(node);

/* Remove FAMC line from child */
	node = find_node(indi, "FAMC", nxref(fam), &last);
	ASSERT(node && last);
	nsibling(last) = nsibling(node);
	free_node(node);

/* Update database with changed records */
	indi_to_dbase(indi);
	if (num_fam_xrefs(fam) == 0)
		remove_empty_fam(fam);
	else
		fam_to_dbase(fam);
	return TRUE;
}
/*===========================================
 * remove_spouse -- Remove spouse from family
 *  both arguments required
 *  silent function
 *=========================================*/
BOOLEAN
remove_spouse (NODE indi, NODE fam)
{
	NODE node, last;
	INT sex;
	STRING stag;

	sex = SEX(indi);
	ASSERT(sex == SEX_MALE || sex == SEX_FEMALE);
/* Make sure spouse is in family and remove his/her HUSB/WIFE line */
	stag = (STRING) ((sex == SEX_MALE) ? "HUSB" : "WIFE");
	if (!(node = find_node(fam, stag, nxref(indi), &last)))
		return FALSE;
	if (last)
		nsibling(last) = nsibling(node);
	else
		nchild(fam) = nsibling(node);
	free_node(node);

/* Remove FAMS line from spouse */
	node = find_node(indi, "FAMS", nxref(fam), &last);
	ASSERT(node && last);
	nsibling(last) = nsibling(node);
	free_node(node);

/* Update database with change records */
	indi_to_dbase(indi);
	if (num_fam_xrefs(fam) == 0)
		remove_empty_fam(fam);
	else
		fam_to_dbase(fam);
	return TRUE;
}
/*=======================================================
 * num_fam_xrefs -- Find number of person links in family
 *   LOOSEEND -- How about other links in the future???
 *=====================================================*/
INT
num_fam_xrefs (NODE fam)
{
	INT num;
	NODE fref, husb, wife, chil, rest;

	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
	num = length_nodes(husb) + length_nodes(wife) + length_nodes(chil);
	join_fam(fam, fref, husb, wife, chil, rest);
	return num;
}
