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
 * edit.c -- Edit person or family record
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 23 Sep 94    3.0.2 - 12 Dec 94
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "translat.h"

extern STRING iredit, fredit, cfpupt, cffupt, idpedt, idspse, idfbys;
extern STRING ntprnt, gdpmod, gdfmod, ronlye;

/*=====================================
 * edit_indi -- Edit person in database
 *===================================*/
NODE edit_indi (indi1)
NODE indi1;	/* may be NULL */
{
	NODE name1, refn1, sex1, body1, famc1, fams1, name2, refn2;
	NODE sex2, body2, indi2, oldn, newn, node;
	FILE *fp;
	BOOLEAN emp;
	STRING msg, key, oldr, newr;
	TRANTABLE tti = tran_tables[MEDIN], tto = tran_tables[MINED];
	TRANTABLE ttd = tran_tables[MINDS];

	if (!indi1 && !(indi1 = ask_for_indi(idpedt, FALSE, FALSE)))
		return NULL;

/* Prepare file for user to edit */
	ASSERT(fp = fopen(editfile, "w"));
	split_indi(indi1, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	write_nodes(0, fp, tto, indi1, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, name1, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, refn1, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, sex1,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, body1, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, famc1, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, fams1, TRUE, TRUE, TRUE);
	fclose(fp);
	join_indi(indi1, name1, refn1, sex1, body1, famc1, fams1);

/* Have user edit file */
	do_edit();
	if (readonly) {
		indi2 = file_to_node(editfile, tti, &msg, &emp);
		if (!equal_tree(indi1, indi2))
			message(ronlye);
		free_nodes(indi2);
		return indi1;
	}
	while (TRUE) {
		indi2 = file_to_node(editfile, tti, &msg, &emp);
		if (!indi2) {
			if (ask_yes_or_no_msg(msg, iredit)) {
				do_edit();
				continue;
			} 
			break;
		}
		if (!valid_indi_tree(indi2, &msg, indi1)) {
			if (ask_yes_or_no_msg(msg, iredit)) {
				do_edit();
				continue;
			}
			free_nodes(indi2);
			indi2 = NULL;
			break;
		}
		break;
	}

/* Editing done; see if database changes */
	if (!indi2) return indi1;
	if (equal_tree(indi1, indi2) || !ask_yes_or_no(cfpupt)) {
		free_nodes(indi2);
		return indi1;
	}

/* Change database */
	split_indi(indi1, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	free_nodes(sex1);
	free_nodes(body1);
	free_nodes(famc1);
	free_nodes(fams1);
	oldn = name1;
	oldr = refn1 ? nval(refn1) : NULL;
	split_indi(indi2, &name2, &refn2, &sex2, &body2, &famc1, &fams1);
	free_nodes(indi2);
	newn = copy_nodes(name2, TRUE, TRUE);
	newr = refn2 ? nval(refn2) : NULL;
	join_indi(indi1, name2, copy_nodes(refn2, TRUE, TRUE), sex2,
	    body2, famc1, fams1);
	remove_duplicate_names(&oldn, &newn);

/* Write changed person to database */
	resolve_links(indi1);
	indi_to_dbase(indi1);
	key = rmvat(nxref(indi1));
	for (node = oldn; node; node = nsibling(node))
		remove_name(nval(node), key);
	rename_from_browse_lists(key);
	for (node = newn; node; node = nsibling(node))
		add_name(nval(node), key);
	if (newr && oldr && eqstr(newr, oldr))
		newr = oldr = NULL;
	if (oldr) remove_refn(oldr, key);
	if (newr) add_refn(newr, key);
	free_nodes(oldn);
	free_nodes(newn);
	free_nodes(refn1);
	free_nodes(refn2);
	mprintf(gdpmod, indi_to_name(indi1, ttd, 35));
	return indi1;
}
/*====================================
 * edit_fam -- Edit family in database
 *==================================*/
NODE edit_family (fam1)
NODE fam1;
{
	NODE indi, husb1, wife1, chil1, rest1, fref1;
	NODE fam2, husb2, wife2, chil2, rest2, fref2;
	TRANTABLE tti = tran_tables[MEDIN], tto = tran_tables[MINED];
	INT i;
	STRING msg;
	FILE *fp;
	BOOLEAN emp;

/* Identify family if need be */
	if (!fam1) {
		indi = ask_for_indi(idspse, FALSE, FALSE);
		if (!indi) return NULL;
		if (!FAMS(indi)) {
			message(ntprnt);
			return NULL;
		}
		fam1 = choose_family(indi, "e", idfbys, TRUE);
		if (!fam1) return FALSE;
	}

/* Prepare file for user to edit */
	split_fam(fam1, &fref1, &husb1, &wife1, &chil1, &rest1);
	ASSERT(fp = fopen(editfile, "w"));
	write_nodes(0, fp, tto, fam1,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, fref1, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, husb1, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, wife1, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, rest1, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, chil1, TRUE, TRUE, TRUE);
	fclose(fp);

/* Have user edit record */
	do_edit();
	if (readonly) {
		fam2 = file_to_node(editfile, tti, &msg, &emp);
		join_fam(fam1, fref1, husb1, wife1, chil1, rest1);
		if (!equal_tree(fam1, fam2))
			message(ronlye);
		free_nodes(fam2);
		return fam1;
	}
	while (TRUE) {
		fam2 = file_to_node(editfile, tti, &msg, &emp);
		if (!fam2) {
			if (ask_yes_or_no_msg(msg, fredit)) {
				do_edit();
				continue;
			}
			break;
		}
		if (!valid_fam_tree(fam2, &msg, fam1, husb1, wife1, chil1)) {
			if (ask_yes_or_no_msg(msg, fredit)) {
				do_edit();
				continue;
			}
			free_nodes(fam2);
			fam2 = NULL;
			break;
		}
		break;
	}

/* If error or user backs out return */
	join_fam(fam1, fref1, husb1, wife1, chil1, rest1);
	if (!fam2) return fam1;
	if (equal_tree(fam1, fam2) || !ask_yes_or_no(cffupt)) {
		free_nodes(fam2);
		return fam1;
	}

/* Change database */
	split_fam(fam1, &fref1, &husb1, &wife1, &chil1, &rest1);
	free_nodes(husb1);
	free_nodes(wife1);
	free_nodes(chil1);
	free_nodes(rest1);
	split_fam(fam2, &fref2, &husb2, &wife2, &chil2, &rest2);
	free_nodes(fam2);
	join_fam(fam1, fref2, husb2, wife2, chil2, rest2);
	resolve_links(fam1);
	fam_to_dbase(fam1);
	mprintf(gdfmod);
	return fam1;
}
/*============================================================
 * remove_duplicate_names -- Remove duplicate names from index
 *==========================================================*/
remove_duplicate_names (pone, ptwo)
NODE *pone, *ptwo;
{
	NODE copy;
	remove_duplicates(pone);
	remove_duplicates(ptwo);
	copy = copy_nodes(*ptwo, TRUE, TRUE);
	remove_one_way(pone, ptwo);
	remove_one_way(&copy, pone);
	free_nodes(copy);
}
/*================================================================
 * remove_one_way -- Remove names in second list that are in first
 *==============================================================*/
remove_one_way (pone, ptwo)
NODE *pone, *ptwo;
{
	NODE one = *pone, two = *ptwo;
	NODE this1, this2, prev1, prev2, next;
	prev1 = NULL;  this1 = one;
	while (this1) {
		prev2 = NULL;  this2 = two;
		while (this2) {
			if (eqstr(nval(this1), nval(this2))) {
				next = nsibling(this2);
				if (prev2)
					nsibling(prev2) = next;
				else
					two = next;
				nsibling(this2) = NULL;
				free_nodes(this2);
				this2 = next;
			} else {
				prev2 = this2;
				this2 = nsibling(this2);
			}
		}
		prev1 = this1;
		this1 = nsibling(this1);
	}
	*ptwo = two;
}
/*==========================================================
 * remove_duplicates -- Remove duplicates from list of names
 *   NOTE: does not take into account lower level lines
 *========================================================*/
remove_duplicates (pnames)
NODE *pnames;
{
	NODE base, prev, this, next;
	base = *pnames;
	while (base) {
		prev = base;
		this = nsibling(base);
		while (this) {
			if (eqstr(nval(base), nval(this))) {
				nsibling(prev) = next = nsibling(this);
				nsibling(this) = NULL;
				free_nodes(this);
				this = next;
			} else {
				prev = this;
				this = nsibling(this);
			}
		}
		base = nsibling(base);
	}
}
/*=========================================
 * equal_tree -- See if two trees are equal
 *=======================================*/
BOOLEAN equal_tree (root1, root2)
NODE root1, root2;
{
	STRING str1, str2;
	if (!root1 && !root2) return TRUE;
	if (!root1 || !root2) return FALSE;
	if (node_list_length(root1) != node_list_length(root2)) return FALSE;
	while (root1) {
		if (nestr(ntag(root1), ntag(root2))) return FALSE;
		str1 = nval(root1);
		str2 = nval(root2);
		if (str1 && !str2) return FALSE;
		if (str2 && !str1) return FALSE;
		if (str1 && str2 && nestr(str1, str2)) return FALSE;
		if (!equal_tree(nchild(root1), nchild(root2))) return FALSE;
		root1 = nsibling(root1);
		root2 = nsibling(root2);
	}
	return TRUE;
}
/*=========================================
 * equal_node -- See if two nodes are equal
 *=======================================*/
BOOLEAN equal_node (node1, node2)
NODE node1, node2;
{
	STRING str1, str2;
	if (!node1 && !node2) return TRUE;
	if (!node1 || !node2) return FALSE;
	if (nestr(ntag(node1), ntag(node2))) return FALSE;
	str1 = nval(node1);
	str2 = nval(node2);
	if (str1 && !str2) return FALSE;
	if (str2 && !str1) return FALSE;
	if (str1 && str2 && nestr(str1, str2)) return FALSE;
	return TRUE;
}
