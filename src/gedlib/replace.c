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
 *   3.0.3 - 20 Jan 96
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "gedcom.h"

/*===================================================================
 * replace_indi -- Replace a person in database with modified version
 *=================================================================*/
BOOLEAN replace_indi (indi1, indi2, pmsg)
NODE indi1;	/* original person - as now in database */
NODE indi2;	/* as person should now be */
STRING *pmsg;
{
	NODE name1, name2, refn1, refn2, sex, body, famc, fams;
	NODE node, namen, refnn, name1n, refn1n, indi0;
	STRING key;

	*pmsg = NULL;
	if (!valid_indi(indi2, pmsg, indi1))  return FALSE;
	if (equal_tree(indi1, indi2)) return TRUE;
	if (readonly) {
		*pmsg = (STRING) "Database is read only -- can't change person.";
		return FALSE;
	}

	split_indi(indi1, &name1, &refn1, &sex, &body, &famc, &fams);
	indi0 = copy_node(indi1);
	join_indi(indi0, NULL, NULL, sex, body, famc, fams);
	free_nodes(indi0);
	split_indi(indi2, &name2, &refn2, &sex, &body, &famc, &fams);
	namen = copy_nodes(name2, TRUE, TRUE);
	refnn = copy_nodes(refn2, TRUE, TRUE);
	join_indi(indi1, name2, refn2, sex, body, famc, fams);
	free_node(indi2);
	classify_nodes(&name1, &namen, &name1n);
	classify_nodes(&refn1, &refnn, &refn1n);

	resolve_links(indi1);
	indi_to_dbase(indi1);
	key = rmvat(nxref(indi1));
	for (node = name1; node; node = nsibling(node))
		remove_name(nval(node), key);
	for (node = namen; node; node = nsibling(node))
		add_name(nval(node), key);
	rename_from_browse_lists(key);
	for (node = refn1; node; node = nsibling(node))
		if (nval(node)) remove_refn(nval(node), key);
	for (node = refnn; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);

	free_nodes(name1);
	free_nodes(namen);
	free_nodes(name1n);
	free_nodes(refn1);
	free_nodes(refnn);
	free_nodes(refn1n);
	*pmsg = (STRING) "Person modified okay.";
	return TRUE;
}
/*==================================================================
 * replace_fam -- Replace a family in database with modified version
 *================================================================*/
BOOLEAN replace_fam (fam1, fam2, pmsg)
NODE fam1;	/* original family - now in database */
NODE fam2;	/* as family should now be */
STRING *pmsg;
{
	NODE refn1, refn2, husb, wife, chil, body;
	NODE refnn, refn1n, node, fam0;
	STRING key;

	*pmsg = NULL;
	if (!valid_fam(fam2, pmsg, fam1)) return FALSE;
	if (equal_tree(fam1, fam2)) return TRUE;
	if (readonly) {
		*pmsg = (STRING) "Database is read only -- can't change family.";
		return FALSE;
	}

	split_fam(fam1, &refn1, &husb, &wife, &chil, &body);
	fam0 = copy_node(fam1);
	join_fam(fam0, NULL, husb, wife, chil, body);
	free_nodes(fam0);
	split_fam(fam2, &refn2, &husb, &wife, &chil, &body);
	refnn = copy_nodes(refn2, TRUE, TRUE);
	join_fam(fam1, refn2, husb, wife, chil, body);
	free_node(fam2);
	classify_nodes(&refn1, &refnn, &refn1n);

	resolve_links(fam1);
	fam_to_dbase(fam1);
	key = rmvat(nxref(fam1));
	for (node = refn1; node; node = nsibling(node))
		if (nval(node)) remove_refn(nval(node), key);
	for (node = refnn; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);
	free_nodes(refn1);
	free_nodes(refnn);
	free_nodes(refn1n);
	*pmsg = (STRING) "Family modified okay.";
	return TRUE;
}
