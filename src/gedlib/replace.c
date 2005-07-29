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
 * replace.c -- Replace persons and families
 * Copyright(c) 1995-96 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   3.0.3 - 20 Jan 96
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"

/*===================================================================
 * replace_indi -- Replace a person in database with modified version
 *  indi1 = current record (copy from database, may or may not be in cache)
 *  indi2 = new data
 *  replaces all children nodes of indi1 with children nodes of indi2
 *  consumes indi2 (calls free_node on it)
 *=================================================================*/
void
replace_indi (NODE indi1, NODE indi2)
{
	NODE name1, name2, refn1, refn2, sex, body, famc, fams;
	NODE node, namen, refnn, name1n, refn1n, indi0;
	STRING key;


	/* Move indi1 data into indi0 & delete it (saving names & refns */
	split_indi_old(indi1, &name1, &refn1, &sex, &body, &famc, &fams);
	indi0 = copy_node(indi1);
	join_indi(indi0, NULL, NULL, sex, body, famc, fams);
	free_nodes(indi0);
	/* Move indi2 data into indi1, also copy out lists of names & refns */
	split_indi_old(indi2, &name2, &refn2, &sex, &body, &famc, &fams);
	namen = copy_nodes(name2, TRUE, TRUE);
	refnn = copy_nodes(refn2, TRUE, TRUE);
	join_indi(indi1, name2, refn2, sex, body, famc, fams);
	free_node(indi2);
	nodechk(indi1, "replace_indi");

	/* Write data to database */

	indi_to_dbase(indi1);
	key = rmvat(nxref(indi1));
	/* update name & refn info */
	/* classify does a diff on its first two arguments, repopulating all three
	arguments -- first is left-only, second is right-only, third is shared */
	/* Note: classify eliminates duplicates */
	classify_nodes(&name1, &namen, &name1n);
	classify_nodes(&refn1, &refnn, &refn1n);
	for (node = name1; node; node = nsibling(node))
		remove_name(nval(node), key);
	for (node = namen; node; node = nsibling(node))
		add_name(nval(node), key);
	rename_from_browse_lists(key);
	for (node = refn1; node; node = nsibling(node))
		if (nval(node)) remove_refn(nval(node), key);
	for (node = refnn; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);

/* now cleanup (indi1 tree is now composed of indi2 data) */
	free_nodes(name1);
	free_nodes(namen);
	free_nodes(name1n);
	free_nodes(refn1);
	free_nodes(refnn);
	free_nodes(refn1n);
}
/*==================================================================
 * replace_fam -- Replace a family in database with modified version
 *  fam1 = current record (copy from database, may or may not be in cache)
 *  fam2 = new data
 *  replaces all children nodes of fam1 with children nodes of fam2
 *  consumes fam2 (calls free_node on it)
 *================================================================*/
void
replace_fam (NODE fam1, NODE fam2)
{
	NODE refn1, refn2, husb, wife, chil, body;
	NODE refnn, refn1n, node, fam0;
	STRING key;


	/* Move fam1 data into fam0 & delete it (saving refns) */
	split_fam(fam1, &refn1, &husb, &wife, &chil, &body);
	fam0 = copy_node(fam1);
	join_fam(fam0, NULL, husb, wife, chil, body);
	free_nodes(fam0);
	/* Move fam2 data into fam1, also copy out list of refns */
	split_fam(fam2, &refn2, &husb, &wife, &chil, &body);
	refnn = copy_nodes(refn2, TRUE, TRUE);
	join_fam(fam1, refn2, husb, wife, chil, body);
	free_node(fam2);

	/* Write data to database */
	

	fam_to_dbase(fam1);
	key = rmvat(nxref(fam1));
	/* remove deleted refns & add new ones */
	classify_nodes(&refn1, &refnn, &refn1n);
	for (node = refn1; node; node = nsibling(node))
		if (nval(node)) remove_refn(nval(node), key);
	for (node = refnn; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);
	free_nodes(refn1);
	free_nodes(refnn);
	free_nodes(refn1n);
}
