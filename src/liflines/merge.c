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
 * merge.c -- Merge persons and families
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 23 Sep 94    3.0.2 - 13 Dec 94
 *===========================================================*/

#include "standard.h"
#include "btree.h"
#include "table.h"
#include "gedcom.h"
#include "translat.h"

extern STRING iredit, cfpmrg, nopmrg, noqmrg, noxmrg, nofmrg;
extern STRING dhusb,  dwife,  cffmrg, fredit, badata, ronlym;

/*================================================================
 * merge_two_indis -- Merge first person to second; data from both
 *   are put in file that user edits; first person removed
 *==============================================================*/
NODE merge_two_indis (indi1, indi2)
NODE indi1, indi2;
{
	NODE name1, refn1, sex1, body1, famc1, fams1;
	NODE name2, refn2, sex2, body2, famc2, fams2;
	NODE indi3, name3, refn3, sex3, body3, famc3, fams3;
	NODE indi4, name4, refn4, sex4, body4, famc4, fams4;
	NODE fam, husb, wife, chil, rest, fref;
	NODE curs1, curs2, node, this, prev, next, old, new;
	TRANTABLE tti = tran_tables[MEDIN], tto = tran_tables[MINED];
	FILE *fp;
	char name[100];
	INT sx2;
	STRING msg, key;
	BOOLEAN emp, iso_tree();

	if (readonly) {
		message(ronlym);
		return NULL;
	}

/* Perform sanity checks */
	ASSERT(indi1 && indi2);
	ASSERT(eqstr("INDI", ntag(indi1)));
	ASSERT(eqstr("INDI", ntag(indi2)));
	if (indi1 == indi2) {
		message(nopmrg);
		return NULL;
	}

/* Check restrictions on persons */
	famc1 = FAMC(indi1);
	famc2 = FAMC(indi2);
#if 0
	if (famc1 && famc2 && nestr(nval(famc1), nval(famc2))) {
		message(noqmrg);
		return NULL;
	}
#endif
	fams1 = FAMS(indi1);
	fams2 = FAMS(indi2);
	if (fams1 && fams2 && SEX(indi1) != SEX(indi2)) {
		message(noxmrg);
		return NULL;
	}

/* Split persons */
	split_indi(indi1, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	split_indi(indi2, &name2, &refn2, &sex2, &body2, &famc2, &fams2);
	ASSERT(name1 && name2);
	sx2 = SEX_UNKNOWN;
	if (fams1) sx2 = val_to_sex(sex1);
	if (fams2) sx2 = val_to_sex(sex2);

/*CONDITION 1 -- 1 & 2 split */

/* Construct first version of merged person */
	ASSERT(fp = fopen(editfile, "w"));
	indi3 = copy_nodes(indi2, TRUE, TRUE);
	name3 = unique_nodes(name1, name2);
	refn3 = unique_nodes(refn1, refn2);
	sex3  = unique_nodes(sex1, sex2);
	body3 = unique_nodes(body1, body2);
	famc3 = unique_nodes(famc1, famc2);
	fams3 = unique_nodes(fams1, fams2);
	write_nodes(0, fp, tto, indi3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, name3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, refn3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, sex3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, body3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, famc3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, fams3, TRUE, TRUE, TRUE);
	fclose(fp);
	join_indi(indi3, name3, refn3, sex3, body3, famc3, fams3);
/*CONDITION 2 -- 3 (init combined) created and joined*/

/* Have user edit merged person */
	do_edit();
	while (TRUE) {
		indi4 = file_to_node(editfile, tti, &msg, &emp);
		if (!indi4 && !emp) {
			if (ask_yes_or_no_msg(msg, iredit)) {
				do_edit();
				continue;
			} 
			break;
		}
		if (!valid_indi_tree(indi4, &msg, indi3)) {
			if (ask_yes_or_no_msg(msg, iredit)) {
				do_edit();
				continue;
			}
			free_nodes(indi4);
			indi4 = NULL;
			break;
		}
		break;
	}
	free_nodes(indi3);

/* Have user confirm changes */
	if (!indi4 || !ask_yes_or_no(cfpmrg)) {
		if (indi4) free_nodes(indi4);
		join_indi(indi1, name1, refn1, sex1, body1, famc1, fams1);
		join_indi(indi2, name2, refn2, sex2, body2, famc2, fams2);
		return NULL;
	}

/* Modify families that have persons as children */
	split_indi(indi4, &name4, &refn4, &sex4, &body4, &famc4, &fams4);
	curs1 = famc1;
	while (curs1) {
		curs2 = famc2;
		while (curs2 && nestr(nval(curs1), nval(curs2)))
			curs2 = nsibling(curs2);
		fam = key_to_fam(rmvat(nval(curs1)));
		split_fam(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		this = chil;

/* Both were children in same family; remove first as child */
		if (curs2) {
			while (this) {
				if (eqstr(nval(this), nxref(indi1))) {
					next = nsibling(this);
					nsibling(this) = NULL;
					free_nodes(this);
					if (!prev)
						chil = next;
					else
						nsibling(prev) = next;
					this = next;
				} else {
					prev = this;
					this = nsibling(this);
				}
			}
			join_fam(fam, fref, husb, wife, chil, rest);
			fam_to_dbase(fam);

/* Only first was child; make family refer to second */
		} else {
			while (this) {
				if (eqstr(nval(this), nxref(indi1))) {
					stdfree(nval(this));
					nval(this) = strsave(nxref(indi2));
				}
				prev = this;
				this = nsibling(this);
			}
			join_fam(fam, fref, husb, wife, chil, rest);
			fam_to_dbase(fam);
		}
		curs1 = nsibling(curs1);
	}

/* Modify families that had persons as spouse */
	this = fams1;
	while (this) {
		fam = key_to_fam(rmvat(nval(this)));
		split_fam(fam, &fref, &husb, &wife, &chil, &rest);
		if (sx2 == SEX_MALE) {
			ASSERT(husb);
			stdfree(nval(husb));
			nval(husb) = (STRING) strsave(nxref(indi2));
		} else {
			ASSERT(wife);
			stdfree(nval(wife));
			nval(wife) = (STRING) strsave(nxref(indi2));
		}
		join_fam(fam, fref, husb, wife, chil, rest);
		fam_to_dbase(fam);
		this = nsibling(this);
	}

/* Remove unneeded nodes and delete first person */
	join_indi(indi1, name1, refn1, sex1, body1, NULL, NULL);
	delete_indi(indi1, FALSE);
	free_nodes(famc1);
	free_nodes(fams1);
	free_nodes(sex2);
	free_nodes(body2);
	free_nodes(famc2);
	free_nodes(fams2);

/* Put merged person in database */
	old = name2;
	new = copy_nodes(name4, TRUE, TRUE);
	join_indi(indi2, name4, refn4, sex4, body4, famc4, fams4);
	remove_duplicate_names(&old, &new);
	resolve_links(indi2);
	indi_to_dbase(indi2);
	key = rmvat(nxref(indi2));
	for (node = old; node; node = nsibling(node))
		remove_name(nval(node), key);
	rename_from_browse_lists(key);
	for (node = new; node; node = nsibling(node))
		add_name(nval(node), key);
	free_nodes(old);
	free_nodes(new);
	return indi2;
}
/*=================================================================
 * merge_two_fams -- Merge first family into second; data from both
 *   are put in file that user edits; first family removed
 *===============================================================*/
NODE merge_two_fams (fam1, fam2)
NODE fam1, fam2;
{
	NODE husb1, wife1, chil1, rest1, husb2, wife2, chil2, rest2;
	NODE fref1, fref2;
	NODE fam3, husb3, wife3, chil3, rest3, fref3;
	NODE fam4, husb4, wife4, chil4, rest4, fref4;
	TRANTABLE tti = tran_tables[MEDIN], tto = tran_tables[MINED];
	FILE *fp;
	STRING msg;
	BOOLEAN emp;

	if (readonly) {
		message(ronlym);
		return NULL;
	}
	ASSERT(fam1 && fam2);
	ASSERT(eqstr("FAM", ntag(fam1)));
	ASSERT(eqstr("FAM", ntag(fam2)));
	if (fam1 == fam2) {
		message(nofmrg);
		return NULL;
	}

/* Check restrictions on families */
	split_fam(fam1, &fref1, &husb1, &wife1, &chil1, &rest1);
	split_fam(fam2, &fref2, &husb2, &wife2, &chil2, &rest2);
#if 0
	if (husb1 && husb2 && nestr(nval(husb1), nval(husb2))) {
		message(dhusb);
		join_fam(fam1, fref1, husb1, wife1, chil1, rest1);
		join_fam(fam2, fref2, husb2, wife2, chil2, rest2);
		return NULL;
	}
	if (wife1 && wife2 && nestr(nval(wife1), nval(wife2))) {
		message(dwife);
		join_fam(fam1, fref1, husb1, wife1, chil1, rest1);
		join_fam(fam2, fref2, husb2, wife2, chil2, rest2);
		return NULL;
	}
#endif

/* Create merged file with both families together */
	ASSERT(fp = fopen(editfile, "w"));
	fam3 = copy_nodes(fam2, TRUE, TRUE);
	fref3 = unique_nodes(fref1, fref2);
	husb3 = unique_nodes(husb1, husb2);
	wife3 = unique_nodes(wife1, wife2);
	rest3 = unique_nodes(rest1, rest2);
	chil3 = sort_children(chil1, chil2);
	write_nodes(0, fp, tto, fam3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, fref3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, husb3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, wife3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, rest3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, chil3, TRUE, TRUE, TRUE);
	fclose(fp);

/* Have user edit merged family */
	do_edit();
	while (TRUE) {
		fam4 = file_to_node(editfile, tti, &msg, &emp);
		if (!fam4 && !emp) {
			if (ask_yes_or_no_msg(msg, fredit)) {
				do_edit();
				continue;
			} 
			break;
		}
		if (!valid_fam_tree(fam4, &msg, fam3, husb3, wife3, chil3)) {
			if (ask_yes_or_no_msg(badata, iredit)) {
				do_edit();
				continue;
			}
			free_nodes(fam4);
			fam4 = NULL;
			break;
		}
		break;
	}
	join_fam(fam3, fref3, husb3, wife3, rest3, chil3);
	free_nodes(fam3);

/* Have user confirm changes */
	if (!fam4 || !ask_yes_or_no(cffmrg)) {
		if (fam4) free_nodes(fam4);
		join_fam(fam1, fref1, husb1, wife1, chil1, rest1);
		join_fam(fam2, fref2, husb2, wife2, chil2, rest2);
		return NULL;
	}
	split_fam(fam4, &fref4, &husb4, &wife4, &chil4, &rest4);

 /* Modify links between persons and families */
#define CHUSB 1
#define CWIFE 2
#define CCHIL 3
	merge_fam_links (fam1, fam2, husb1, husb2, CHUSB);
	merge_fam_links (fam1, fam2, wife1, wife2, CWIFE);
	merge_fam_links (fam1, fam2, chil1, chil2, CCHIL);

/* Update database with second family; remove first */
	join_fam(fam4, fref2, husb2, wife2, chil2, rest2);
	free_nodes(fam4);
	nchild(fam1) = NULL;
	delete_fam(fam1);
	free_nodes(husb1);
	free_nodes(wife1);
	free_nodes(chil1);
	free_nodes(rest1);
	join_fam(fam2, fref4, husb4, wife4, chil4, rest4);
	resolve_links(fam2);
	fam_to_dbase(fam2);
	return fam2;
}
/*============================================
 * iso_tree -- See if two trees are isomorphic
 *==========================================*/
BOOLEAN iso_tree (root1, root2)
NODE root1, root2;
{
	STRING str1, str2;
	INT nchil1, nchil2;
	NODE chil1, chil2;
	if (!root1 && !root2) return TRUE;
	if (!root1 || !root2) return FALSE;
	if (nestr(ntag(root1), ntag(root2))) return FALSE;
	str1 = nval(root1);
	str2 = nval(root2);
	if (str1 && !str2) return FALSE;
	if (str2 && !str1) return FALSE;
	if (str1 && str2 && nestr(str1, str2)) return FALSE;
	nchil1 = node_list_length(nchild(root1));
	nchil2 = node_list_length(nchild(root2));
	if (nchil1 != nchil2) return FALSE;
	if (nchil1 == 0) return TRUE;
	chil1 = nchild(root1);
	while (chil1) {
		chil2 = nchild(root2);
		while (chil2) {
			if (iso_tree(chil1, chil2))
				break;
			chil2 = nsibling(chil2);
		}
		if (!chil2) return FALSE;
		chil1 = nsibling(chil1);
	}
	return TRUE;
}
/*======================================================
 * iso_tree_list -- See if two tree lists are isomorphic
 *====================================================*/
BOOLEAN iso_tree_list (root1, root2)
NODE root1, root2;
{
	INT len1, len2;
	NODE node1, node2;
	if (!root1 || !root2) return FALSE;
	len1 = node_list_length(root1);
	len2 = node_list_length(root2);
	if (len1 != len2) return FALSE;
	if (len1 == 0) return TRUE;
	node1 = root1;
	while (node1) {
		node2 = root2;
		while (node2) {
			if (iso_tree(node1, node2)) break;
			node2 = nsibling(node2);
		}
		if (!node2) return FALSE;
		node1 = nsibling(node1);
	}
	return TRUE;
}
/*===============================================
 * unique_nodes -- Return union of two node trees
 *=============================================*/
NODE unique_nodes (node1, node2)
NODE node1, node2;
{
	NODE copy1 = copy_nodes(node1, TRUE, TRUE);
	NODE copy2 = copy_nodes(node2, TRUE, TRUE);
	NODE curs1, next1, prev1, curs2, prev2;
	prev2 = NULL;
	curs2 = copy2;
	while (curs2) {
		prev1 = NULL;
		curs1 = copy1;
		while (curs1 && !iso_tree(curs1, curs2)) {
			prev1 = curs1;
			curs1 = nsibling(curs1);
		}
		if (curs1) {
			next1 = nsibling(curs1);
			nsibling(curs1) = NULL;
			free_nodes(curs1);
			if (prev1)
				nsibling(prev1) = next1;
			else
				copy1 = next1;
		}
		prev2 = curs2;
		curs2 = nsibling(curs2);
	}
	if (prev2) {
		nsibling(prev2) = copy1;
		return copy2;
	}
	return copy1;
}
#if 0
/*==============================================================
 * unique_to_file -- Copy unique parts of node1 and node2, up to
 *   isomorphism, to file.
 *============================================================*/
unique_to_file (node1, node2, tt, fp)
NODE node1, node2;
TRANTABLE tt;
FILE *fp;
{
	NODE prev, this, next, copy1 = copy_nodes(node1, TRUE, TRUE);
	while (node2) {
		write_nodes(1, fp, tt, node2, TRUE, TRUE, FALSE);
		prev = NULL;
		this = copy1;
		while (this && !iso_tree(this, node2)) {
			prev = this;
			this = nsibling(this);
		}
		if (this) {
			next = nsibling(this);
			nsibling(this) = NULL;
			free_nodes(this);
			if (prev)
				nsibling(prev) = next;
			else
				copy1 = next;
		}
		node2 = nsibling(node2);
	}
	write_nodes(1, fp, tt, copy1, TRUE, TRUE, TRUE);
	free_nodes(copy1);
}
#endif
/*================================================================
 * merge_fam_links -- Shift links of persons in list1 from fam1 to
 *   fam2.  List1 holds the persons that refer to fam1, and list2
 *   holds the persons who refer to fam2.  If a person is on both
 *   lists, the reference in the person to the fam1 is removed from
 *   the person.  If a person is only on list1, the reference to fam1
 *   is changed to refer to fam2.  No changes are made for persons
 *   only on list2.  No changes are made to the references from the
 *   families to the persons.
 *================================================================*/
merge_fam_links (fam1, fam2, list1, list2, code)
NODE fam1, fam2, list1, list2;
INT code;
{
	NODE curs1, curs2, prev, this, next, first;
	NODE indi, name, refn, sex, body, famc, fams;
	curs1 = list1;
	while (curs1) {
		curs2 = list2;
		while (curs2 && nestr(nval(curs1), nval(curs2)))
			curs2 = nsibling(curs2);
		indi = key_to_indi(rmvat(nval(curs1)));
		split_indi(indi, &name, &refn, &sex, &body, &famc, &fams);
		prev = NULL;
		if (code == CHUSB || code == CWIFE)
			first = this = fams;
		else
			first = this = famc;

/* Both fams linked to this indi; remove link in indi to first */
		if (curs2) {
			while (this) {
				if (eqstr(nval(this), nxref(fam1))) {
					next = nsibling(this);
					nsibling(this) = NULL;
					free_nodes(this);
					if (!prev)
						first = next;
					else
						nsibling(prev) = next;
					this = next;
				} else {
					prev = this;
					this = nsibling(this);
				}
			}

/* Only first fam linked with this indi; move link to second */
		} else {
			while (this) {
				if (eqstr(nval(this), nxref(fam1))) {
					stdfree(nval(this));
					nval(this) = strsave(nxref(fam2));
				}
				prev = this;
				this = nsibling(this);
			}
		}
		if (code == CHUSB || code == CWIFE)
			fams = first;
		else
			famc = first;
		join_indi(indi, name, refn, sex, body, famc, fams);
		indi_to_dbase(indi);
		curs1 = nsibling(curs1);
	}
}
/*================================================
 * sort_children -- Return sorted list of children
 *==============================================*/
NODE sort_children (chil1, chil2)
NODE chil1, chil2;
{
	NODE copy1, copy2, chil3, prev, kid1, kid2;
	STRING year1, year2;
	INT int1, int2;
	copy1 = remove_dupes(chil1, chil2);
	copy2 = copy_nodes(chil2, TRUE, TRUE);
	int1 = int2 = 1;
	prev = chil3 = NULL;
	while (copy1 && copy2) {
		if (int1 == 1) {
			kid1 = key_to_indi(rmvat(nval(copy1)));
			year1 = event_to_date(BIRT(kid1), NULL, TRUE);
			if (!year1)
				year1 = event_to_date(BAPT(kid1), NULL, TRUE);
			int1 = year1 ? atoi(year1) : 0;
		}
		if (int2 == 1) {
			kid2 = key_to_indi(rmvat(nval(copy2)));
			year2 = event_to_date(BIRT(kid2), NULL, TRUE);
			if (!year2)
				year2 = event_to_date(BAPT(kid2), NULL, TRUE);
			int2 = year2 ? atoi(year2) : 0;
		}
		if (int1 < int2) {
			if (!prev)
				prev = chil3 = copy1;
			else
				prev = nsibling(prev) = copy1;
			copy1 = nsibling(copy1);
			int1 = 1;
		} else {
			if (!prev)
				prev = chil3 = copy2;
			else
				prev = nsibling(prev) = copy2;
			copy2 = nsibling(copy2);
			int2 = 1;
		}
	}
	if (copy1) {
		if (!prev)
			chil3 = copy1;
		else
			nsibling(prev) = copy1;
	}
	if (copy2) {
		if (!prev)
			chil3 = copy2;
		else
			nsibling(prev) = copy2;
	}
	return chil3;
}
/*=================================================
 * remove_dupes -- Return all in list1 not in list2
 *===============================================*/
NODE remove_dupes (list1, list2)
NODE list1, list2;
{
	NODE copy1 = copy_nodes(list1, TRUE, TRUE);
	NODE prev1, next1, curs1, curs2;
	prev1 = NULL;
	curs1 = copy1;
	while (curs1) {
		curs2 = list2;
		while (curs2 && nestr(nval(curs1), nval(curs2)))
			curs2 = nsibling(curs2);
		if (curs2) {
			next1 = nsibling(curs1);
			nsibling(curs1) = NULL;
			free_nodes(curs1);
			if (!prev1)
				copy1 = next1;
			else
				nsibling(prev1) = next1;
			curs1 = next1;
		} else {
			prev1 = curs1;
			curs1 = nsibling(curs1);
		}
	}
	return copy1;
}
/*=================================================
 * iso_list -- See if two node lists are isomorphic
 *===============================================*/
BOOLEAN iso_list (root1, root2)
NODE root1, root2;
{
	INT len1, len2;
	NODE node1, node2;
	if (!root1 && !root2) return TRUE;
	if (!root1 || !root2) return FALSE;
	len1 = node_list_length(root1);
	len2 = node_list_length(root2);
	if (len1 != len2) return FALSE;
	if (len1 == 0) return TRUE;
	node1 = root1;
	while (node1) {
		node2 = root2;
		while (node2) {
			if (equal_node(node1, node2))
				break;
			node2 = nsibling(node2);
		}
		if (!node2) return FALSE;
		node1 = nsibling(node1);
	}
	return TRUE;
}
