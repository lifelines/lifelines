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
 * delete.c -- Removes person and family records from database 
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 15 Aug 93
 *   3.0.0 - 30 Jun 94    3.0.2 - 10 Dec 94
 *   3.0.3 - 21 Jan 96
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "screen.h"

#include "llinesi.h"

extern STRING idpdel, cfpdel, haslnk;

/*================================================================
 * delete_indi -- Delete person and links; if this leaves families
 *   with no links, remove them
 *==============================================================*/
void
delete_indi (NODE indi,      /* person to remove - may be null */
             BOOLEAN conf)   /* have user confirm */
{
	STRING key;
	NODE name, refn, sex, body, famc, fams, this;
	NODE node, husb, wife, chil, rest, fam, prev, next, fref;
	INT isex, keyint;
	BOOLEAN found;

	if (!indi && !(indi = ask_for_indi(idpdel, NOCONFIRM, TRUE)))
		return;
	if (conf && !ask_yes_or_no(cfpdel)) return;

	split_indi(indi, &name, &refn, &sex, &body, &famc, &fams);
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
			delete_fam(fam);
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
			delete_fam(fam);
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
 * delete_fam -- Delete family from database
 *========================================*/
void
delete_fam (NODE fam)
{
	STRING key;
	NODE node, husb, wife, chil, rest, refn;
	INT keyint;
	if (!fam) return;
	split_fam(fam, &refn, &husb, &wife, &chil, &rest);
	if (husb || wife || chil) {
		message(haslnk);
		join_fam(fam, refn, husb, wife, chil, rest);
		return;
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
}
