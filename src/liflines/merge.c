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
 * merge.c -- Merge persons and families
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 23 Sep 94    3.0.2 - 13 Dec 94
 *   3.0.3 - 21 Jan 96
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "llinesi.h"
#include "feedback.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN traditional;
extern STRING qSiredit, qScfpmrg, qSnopmrg, qSnoqmrg, qSnoxmrg, qSnofmrg;
extern STRING qSdhusb,  qSdwife,  qScffmrg, qSfredit, qSbadata, qSronlym;
extern STRING qSmgsfam,qSmgconf;

static void merge_fam_links(NODE, NODE, NODE, NODE, INT);
static NODE remove_dupes(NODE, NODE);
static NODE sort_children(NODE, NODE);


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void check_indi_lineage_links(NODE indi);
static void check_fam_lineage_links(NODE fam);


/*================================================================
 * merge_two_indis -- Merge first person to second; data from both
 *   are put in file that user edits; first person removed
 *  indi1: [IN]  person who will be deleted (non-null)
 *  indi2: [IN]  person who will receive new (combined & edited) data (non-null)
 *  conf:  [IN]  should we prompt user to confirm change ?
 *---------------------------------------------------------------
 *  These are the four main variables
 *   indi1 - person in database -- indi1 is merged into indi2
 *   indi2 - person in database -- indi1 is merged into this person
 *   indi3 - merged version of the two persons before editing
 *   indi4 - merged version of the two persons after editing
 *==============================================================*/
RECORD
merge_two_indis (NODE indi1, NODE indi2, BOOLEAN conf)
{
	NODE indi01, indi02;	/* original arguments */
	NODE name1, refn1, sex1, body1, famc1, fams1;
	NODE name2, refn2, sex2, body2, famc2, fams2;
	NODE indi3, name3, refn3, sex3, body3, famc3, fams3;
	NODE indi4=0;
	NODE fam, husb, wife, chil, rest, fref, keep=NULL;
	NODE this, that, prev, next, node, head;
	NODE fam12;
	NODE name24, refn24;
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);
	XLAT ttmo = transl_get_predefined_xlat(MINED);
	FILE *fp;
	INT sx2;
	STRING msg, key;
 	BOOLEAN emp;

/* Do start up checks */

	ASSERT(indi1);
	ASSERT(indi2);
	ASSERT(eqstr("INDI", ntag(indi1)));
	ASSERT(eqstr("INDI", ntag(indi2)));
	if (readonly) {
		message(_(qSronlym));
		return NULL;
	}
	if (indi1 == indi2) {
		message(_(qSnopmrg));
		return NULL;
	}

/* Check restrictions on persons */

	famc1 = FAMC(indi1);
	famc2 = FAMC(indi2);
/*LOOSEEND -- THIS CHECK IS NOT GOOD ENOUGH */
/* comment from merge.c 1.2 2000-01-03 nozell */
	if (traditional) {
		if (famc1 && famc2 && nestr(nval(famc1), nval(famc2))) {
			if (!ask_yes_or_no_msg(_(qSmgsfam), _(qSmgconf))) {
				message(_(qSnoqmrg));
				return NULL;
			}
		}
	}
	fams1 = FAMS(indi1);
	fams2 = FAMS(indi2);
	if (fams1 && fams2 && SEX(indi1) != SEX(indi2)) {
		message(_(qSnoxmrg));
		return NULL;
	}


/* sanity check lineage links */
	check_indi_lineage_links(indi1);
	check_indi_lineage_links(indi2);

/* Split original persons */

	/* If we are successful, the original indi1 will be deleted
 	 * and the indi2 will be updated with the new info.
   	 * However, if we do not merge then we must
	 * insure that indi1 and indi2 are in their original state.
	 * It seems safer to do this by only working with copies
    	 * of the originals.
	 */
	indi01 = indi1;	/* keep original indi1 for later delete */
	indi1 = copy_nodes(indi1, TRUE, TRUE);
	indi02 = indi2;	/* keep original indi2 for later update and return */
	indi2 = copy_nodes(indi2, TRUE, TRUE);

/* we split indi1 & indi2 and leave them split until near the end */
	split_indi_old(indi1, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	split_indi_old(indi2, &name2, &refn2, &sex2, &body2, &famc2, &fams2);
	indi3 = indi2; 
	indi2 = copy_nodes(indi2, TRUE, TRUE);
	sx2 = SEX_UNKNOWN;
	if (fams1) sx2 = val_to_sex(sex1);
	if (fams2) sx2 = val_to_sex(sex2);

/*CONDITION: 1s, 2s - build first version of merged person */

	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	name3 = union_nodes(name1, name2, TRUE, TRUE);
	refn3 = union_nodes(refn1, refn2, TRUE, TRUE);
	sex3  = union_nodes(sex1,  sex2,  TRUE, TRUE);
	body3 = union_nodes(body1, body2, TRUE, TRUE);
	famc3 = union_nodes(famc1, famc2, TRUE, TRUE);
	fams3 = union_nodes(fams1, fams2, TRUE, TRUE);
	write_nodes(0, fp, ttmo, indi3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, name3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, refn3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, sex3,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, body3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, famc3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, fams3, TRUE, TRUE, TRUE);
	fclose(fp);
	join_indi(indi3, name3, refn3, sex3, body3, famc3, fams3);

/*CONDITION 2 -- 3 (init combined) created and joined*/
/* 
	indi1 & indi2 are originals
	indi3 is combined version
	indi4 will be what user creates editing indi3
	(and then we'll throw away indi3)
*/
/* Have user edit merged person */
	do_edit();
	while (TRUE) {
		indi4 = file_to_node(editfile, ttmi, &msg, &emp);
		if (!indi4 && !emp) {
			if (ask_yes_or_no_msg(msg, _(qSiredit))) {
				do_edit();
				continue;
			} 
			break;
		}
		if (!valid_indi_tree(indi4, &msg, indi3)) {
			if (ask_yes_or_no_msg(msg, _(qSiredit))) {
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
	if (!indi4 || (conf && !ask_yes_or_no(_(qScfpmrg)))) {
		if (indi4) free_nodes(indi4);
		join_indi(indi1, name1, refn1, sex1, body1, famc1, fams1);
		free_nodes(indi1);
		join_indi(indi2, name2, refn2, sex2, body2, famc2, fams2);
		free_nodes(indi2);
		/* originals (indi01 and indi02) have not been modified */
		return NULL;
	}

/* Modify families that have persons as children */

	classify_nodes(&famc1, &famc2, &fam12);

/*
 process fam12 - the list of FAMC in both original nodes
 Both were children in same family; remove first as child
 FAMCs in indi4 are union of indi1 & indi2 because we don't
 allow the user to edit these 
 */

	this = fam12;
	while (this) {
		fam = key_to_fam(rmvat(nval(this)));
		split_fam(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		that = chil;
		while (that) {
			if (eqstr(nval(that), nxref(indi1))) {
				next = nsibling(that);
				nsibling(that) = NULL;
				keep = nchild(that);
				free_node(that);
				if (!prev)
					chil = next;
				else
					nsibling(prev) = next;
				that = next;
			} else {
				prev = that;
				that = nsibling(that);
			}
		}
		that = chil;
		while (keep && that) {
			if (eqstr(nval(that), nxref(indi2))) {
				nchild(that) = union_nodes(nchild(that),
					keep, TRUE, FALSE);
			}
			that = nsibling(that);
		}
		join_fam(fam, fref, husb, wife, chil, rest);
		fam_to_dbase(fam);
		this = nsibling(this);
	}

/* process famc1 - the list of FAMC only in first original node */
/* Only first was child; make family refer to second */

	this = famc1;
	while (this) {
		fam = key_to_fam(rmvat(nval(this)));
		split_fam(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		that = chil;
		while (that) {
			if (eqstr(nval(that), nxref(indi1))) {
				stdfree(nval(that));
				nval(that) = strsave(nxref(indi2));
			}
			prev = that;
			that = nsibling(that);
		}
		join_fam(fam, fref, husb, wife, chil, rest);
		fam_to_dbase(fam);
		this = nsibling(this);
	}
	free_nodes(fam12);

/*HERE*/
/* Modify families that had persons as spouse */

	classify_nodes(&fams1, &fams2, &fam12);

/*
 process fam12 - the list of FAMS in both original nodes
 Both were parents in same family; remove first as parent
 FAMSs in indi4 are union of indi1 & indi2 because we don't
 allow the user to edit these 
 */

	this = fam12;
	while (this) {
		fam = key_to_fam(rmvat(nval(this)));
		split_fam(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		if (sx2 == SEX_MALE)
			head = that = husb;
		else
			head = that = wife;
		while (that) {
			if (eqstr(nval(that), nxref(indi1))) {
				next = nsibling(that);
				nsibling(that) = NULL;
				free_nodes(that);
				if (!prev)
					prev = head = next;
				else
					nsibling(prev) = next;
				that = next;
			} else {
				prev = that;
				that = nsibling(that);
			}
		}
		if (sx2 == SEX_MALE)
			husb = head;
		else
			wife = head;
		join_fam(fam, fref, husb, wife, chil, rest);
		fam_to_dbase(fam);
		this = nsibling(this);
	}

/* process fams1 - the list of FAMS only in first original node */
/* Only first was parent; make family refer to second */

	this = fams1;
	while (this) {
		fam = key_to_fam(rmvat(nval(this)));
		split_fam(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		that = (sx2 == SEX_MALE) ? husb : wife;
		while (that) {
			if (eqstr(nval(that), nxref(indi1))) {
				stdfree(nval(that));
				nval(that) = strsave(nxref(indi2));
			}
			prev = that;
			that = nsibling(that);
		}
		join_fam(fam, fref, husb, wife, chil, rest);
		fam_to_dbase(fam);
		this = nsibling(this);
	}
	free_nodes(fam12);

/*
 name1 holds original names of #1/
 name2 holds original names of #2
 indi4 holds new/edited names

 The NAMEs & REFNs in original #1 (indi01) will get deleted
  when indi01 is deleted below
 We just need to take care of any changes from indi02 to indi4
  diff name2 vs name4 & delete any dropped ones, & add new ones
 But instead of messing with indi4, which is the new record
  we'll make a scratch copy (in indi3, which is not used now)
*/

	indi3 = copy_nodes(indi4, TRUE, TRUE);

	split_indi_old(indi3, &name3, &refn3, &sex3, &body3, &famc3, &fams3);
	classify_nodes(&name2, &name3, &name24);
	classify_nodes(&refn2, &refn3, &refn24);

	key = rmvat(nxref(indi4));
	for (node = name2; node; node = nsibling(node))
		remove_name(nval(node), key);
	for (node = name3; node; node = nsibling(node))
		add_name(nval(node), key);
	rename_from_browse_lists(key);
	for (node = refn2; node; node = nsibling(node))
		if (nval(node)) remove_refn(nval(node), key);
	for (node = refn3; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);
	join_indi(indi3, name3, refn3, sex3, body3, famc3, fams3);
	free_nodes(indi3);
	free_nodes(name24);
	free_nodes(refn24);

/* done with changes, save new record to db */

	resolve_refn_links(indi4);
	indi_to_dbase(indi4);

/* finally we're done with indi1 & indi2 */

	join_indi(indi1, name1, refn1, sex1, body1, famc1, fams1);
	free_nodes(indi1);
	join_indi(indi2, name2, refn2, sex2, body2, famc2, fams2);
	free_nodes(indi2);

/* update indi02 to contain info from new merged record in indi4 */
/* Note - we could probably just save indi4 and delete indi02 
	- Perry, 2000/12/06 */

	split_indi_old(indi4, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	split_indi_old(indi02, &name2, &refn2, &sex2, &body2, &famc2, &fams2);
	join_indi(indi4, name2, refn2, sex2, body2, famc2, fams2);
	join_indi(indi02, name1, refn1, sex1, body1, famc1, fams1);
	free_nodes(indi4);

	remove_indi_by_root(indi01);	/* this is the original indi1 */

/* sanity check lineage links */
	check_indi_lineage_links(indi02);

	return node_to_record(indi02);   /* this is the updated indi2 */
}
/*=================================================================
 * merge_two_fams -- Merge first family into second; data from both
 *   are put in file that user edits; first family removed
 *---------------------------------------------------------------
 *  These are the four main variables
 *   fam1 - upper family in database -- fam1 is merged into fam2
 *   fam2 - lower family in database -- fam1 is merged into this person
 *   fam3 - temporary which is the merged version for the user to edit
 *   fam4 - merged version of the two persons after editing
 *     the nodes inside fam4 are stored into fam2 & to dbase at the very end
 *===============================================================*/
RECORD
merge_two_fams (NODE fam1, NODE fam2)
{
	NODE husb1, wife1, chil1, rest1, husb2, wife2, chil2, rest2;
	NODE fref1, fref2;
	NODE fam3, husb3, wife3, chil3, rest3, fref3;
	NODE fam4=0, husb4, wife4, chil4, rest4, fref4;
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);
	XLAT ttmo = transl_get_predefined_xlat(MINED);
	FILE *fp;
	STRING msg;
	BOOLEAN emp;

	if (readonly) {
		message(_(qSronlym));
		return NULL;
	}
	ASSERT(fam1);
	ASSERT(fam2);
	ASSERT(eqstr("FAM", ntag(fam1)));
	ASSERT(eqstr("FAM", ntag(fam2)));
	if (fam1 == fam2) {
		message(_(qSnofmrg));
		return NULL;
	}

/* sanity check lineage links */
	check_fam_lineage_links(fam1);
	check_fam_lineage_links(fam2);

/* Check restrictions on families */
	split_fam(fam1, &fref1, &husb1, &wife1, &chil1, &rest1);
	split_fam(fam2, &fref2, &husb2, &wife2, &chil2, &rest2);
	if (traditional) {
		BOOLEAN ok = TRUE;
		if (husb1 && husb2 && nestr(nval(husb1), nval(husb2))) {
			message(_(qSdhusb));
			ok = FALSE;
		}
		if (ok && wife1 && wife2 && nestr(nval(wife1), nval(wife2))) {
			message(_(qSdwife));
			ok = FALSE;
		}
		if (!ok) {
			join_fam(fam1, fref1, husb1, wife1, chil1, rest1);
			join_fam(fam2, fref2, husb2, wife2, chil2, rest2);
			return NULL;
		}
	}

/* Create merged file with both families together */
	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	fam3 = copy_nodes(fam2, TRUE, TRUE);
	fref3 = union_nodes(fref1, fref2, TRUE, TRUE);
	husb3 = union_nodes(husb1, husb2, TRUE, TRUE);
	wife3 = union_nodes(wife1, wife2, TRUE, TRUE);
	rest3 = union_nodes(rest1, rest2, TRUE, TRUE);
	chil3 = sort_children(chil1, chil2);
	write_nodes(0, fp, ttmo, fam3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, fref3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, husb3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, wife3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, rest3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, chil3, TRUE, TRUE, TRUE);
	fclose(fp);

/* Have user edit merged family */
	join_fam(fam3, fref3, husb3, wife3, rest3, chil3);
	do_edit();
	while (TRUE) {
		fam4 = file_to_node(editfile, ttmi, &msg, &emp);
		if (!fam4 && !emp) {
			if (ask_yes_or_no_msg(msg, _(qSfredit))) {
				do_edit();
				continue;
			} 
			break;
		}
		/* check validation & allow user to reedit if invalid */
		/* this is a showstopper, so alternative is to abort */
		if (!valid_fam_tree(fam4, &msg, fam3)) {
			if (ask_yes_or_no_msg(_(qSbadata), _(qSiredit))) {
				do_edit();
				continue;
			}
			free_nodes(fam4);
			fam4 = NULL;
			break;
		}
		break;
	}
	free_nodes(fam3);

/* Have user confirm changes */

	if (!fam4 || !ask_yes_or_no(_(qScffmrg))) {
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
	merge_fam_links(fam1, fam2, husb1, husb2, CHUSB);
	merge_fam_links(fam1, fam2, wife1, wife2, CWIFE);
	merge_fam_links(fam1, fam2, chil1, chil2, CCHIL);

/* Update database with second family; remove first */
	join_fam(fam4, fref2, husb2, wife2, chil2, rest2);
	free_nodes(fam4);
	nchild(fam1) = NULL;
	remove_empty_fam(fam1); /* TO DO - can this fail ? 2001/11/08, Perry */
	free_nodes(husb1);
	free_nodes(wife1);
	free_nodes(chil1);
	free_nodes(rest1);
	join_fam(fam2, fref4, husb4, wife4, chil4, rest4);
	resolve_refn_links(fam2);
	fam_to_dbase(fam2);


/* sanity check lineage links */
	check_fam_lineage_links(fam2);
	
	return node_to_record(fam2);
}
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
void
merge_fam_links (NODE fam1, NODE fam2, NODE list1, NODE list2, INT code)
{
	NODE curs1, curs2, prev, this, next, first, keep=NULL;
	NODE indi, name, refn, sex, body, famc, fams;

	curs1 = list1;
	while (curs1) {
		curs2 = list2;
		while (curs2 && nestr(nval(curs1), nval(curs2)))
			curs2 = nsibling(curs2);
		indi = key_to_indi(rmvat(nval(curs1)));
		split_indi_old(indi, &name, &refn, &sex, &body, &famc, &fams);
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
					keep = nchild(this);
					free_node(this);
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
			this = first;
			while (keep && this) {
				if (eqstr(nval(this), nxref(fam2))) {
					nchild(this) =
					    union_nodes(nchild(this), keep,
					    TRUE, FALSE);
/*HERE*/
				}
				this = nsibling(this);
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
static NODE
sort_children (NODE chil1,
               NODE chil2)
{
	NODE copy1, copy2, chil3, prev, kid1, kid2;
	STRING year1, year2;
	INT int1, int2;
	/* copy1 contains all children in chil1 not in chil2 */
	copy1 = remove_dupes(chil1, chil2);
	copy2 = copy_nodes(chil2, TRUE, TRUE);
	int1 = int2 = 1;
	prev = chil3 = NULL;
	while (copy1 && copy2) {
		if (int1 == 1) {
			kid1 = key_to_indi(rmvat(nval(copy1)));
			year1 = event_to_date(BIRT(kid1), TRUE);
			if (!year1)
				year1 = event_to_date(BAPT(kid1), TRUE);
			int1 = year1 ? atoi(year1) : 0;
		}
		if (int2 == 1) {
			kid2 = key_to_indi(rmvat(nval(copy2)));
			year2 = event_to_date(BIRT(kid2), TRUE);
			if (!year2)
				year2 = event_to_date(BAPT(kid2), TRUE);
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
 *  Creates a copy of list1 then traverses it
 *   removing any items which are in list2
 *  Then returns this new trimmed list.
 * This is pretty inefficient algorithm, as every item
 *  in list1 is compared against every item in list2
 * It would be more efficient to make a table of items
 *  on list2 first, and then use that to check each item
 *  in list1.
 *===============================================*/
static NODE
remove_dupes (NODE list1, NODE list2)
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
 * check_indi_lineage_links -- Check all families of
 *  this person to make sure they point back to this person
 *===============================================*/
static void
check_indi_lineage_links (NODE indi)
{
	NODE name=0, refn=0, sex=0, body=0, famc=0, fams=0;
	NODE curs=0; /* for travesing node lists */
	TABLE memtab = memtab = create_table_int();
	TABLE_ITER tabit=0;
	CNSTRING famkey=0; /* used inside traversal loops */
	INT count=0;
	CNSTRING ikey = nxref(indi);

	/* sanity check record is not deleted */
	ASSERT(is_key_in_use(ikey));

/* Now validate lineage links of this person */
	split_indi_old(indi, &name, &refn, &sex, &body, &famc, &fams);

	/*
	Make table listing all families this person is spouse in
	(& how many times each)
	*/
	for (curs = fams; curs; curs = nsibling(curs)) {
		famkey = rmvat(nval(curs));
		if (!eqstr(ntag(curs), "FAMS")) {
			char msg[512];
			snprintf(msg, sizeof(msg)/sizeof(msg[0]), _("Bad spouse tag: %s"), ntag(curs));
			FATAL2(msg);
		}
		increment_table_int(memtab, famkey);
	}

	/*
	Check that all listed families contain person as spouse as many times
	as expected
	*/
	tabit = begin_table_iter(memtab);
	while (next_table_int(tabit, &famkey, &count)) {
		NODE fam = key_to_fam(famkey);
		/*
		count how many times our main person (ikey)
		occurs in this family (fam) as a spouse (HUSB or WIFE)
		*/
		INT occur = 0;
		for (curs = nchild(fam); curs; curs = nsibling(curs)) {
			if (eqstr(ntag(curs), "HUSB") || eqstr(ntag(curs), "WIFE")) {
				if (eqstr(nval(curs), ikey)) {
					++occur;
				}
			}
		}
		if (count != occur) {
			char msg[512];
			snprintf(msg, sizeof(msg)/sizeof(msg[0])
				, _("Mismatched lineage spouse links between %s and %s: %ld and %ld")
				, ikey, famkey, count, occur);
			FATAL2(msg);
		}
	}
	destroy_table(memtab);
	memtab = create_table_int();

	/*
	Make table listing all families this person is child in
	(& how many times each)
	*/
	for (curs = famc; curs; curs = nsibling(curs)) {
		famkey = rmvat(nval(curs));
		if (!eqstr(ntag(curs), "FAMC")) {
			char msg[512];
			snprintf(msg, sizeof(msg)/sizeof(msg[0]), _("Bad child tag: %s"), ntag(curs));
			FATAL2(msg);
		}
		increment_table_int(memtab, famkey);
	}

	/*
	Check that all listed families contain person as child (CHIL) as many times
	as expected
	*/
	tabit = begin_table_iter(memtab);
	while (next_table_int(tabit, &famkey, &count)) {
		NODE fam = key_to_fam(famkey);
		/*
		count how many times our main person (ikey)
		occurs in this family (fam) as a child (CHIL)
		*/
		INT occur = 0;
		for (curs = nchild(fam); curs; curs = nsibling(curs)) {
			if (eqstr(ntag(curs), "CHIL")) {
				if (eqstr(nval(curs), ikey)) {
					++occur;
				}
			}
		}
		if (count != occur) {
			char msg[512];
			snprintf(msg, sizeof(msg)/sizeof(msg[0])
				, _("Mismatched lineage child links between %s and %s: %ld and %ld")
				, ikey, famkey, count, occur);
			FATAL2(msg);
		}
	}

	join_indi(indi, name, refn, sex, body, famc, fams);
	destroy_table(memtab);
}
/*=================================================
 * check_fam_lineage_links -- Check all persons of
 *  this family to make sure they point back to this family
 *===============================================*/
static void
check_fam_lineage_links (NODE fam)
{
	NODE fref=0, husb=0, wife=0, chil=0, rest=0;
	NODE curs=0; /* for travesing node lists */
	TABLE memtab = memtab = create_table_int();
	TABLE_ITER tabit=0;
	CNSTRING indikey=0; /* used inside traversal loops */
	INT count=0;
	CNSTRING fkey = nxref(fam);

	/* sanity check record is not deleted */
	ASSERT(is_key_in_use(fkey));
	
/* Now validate lineage links of this family */
	split_fam(fam, &fref, &husb, &wife, &chil, &rest);

	/*
	Make table listing all spouses in this family
	(& how many times each)
	*/
	for (curs = husb; curs; curs = nsibling(curs)) {
		indikey = rmvat(nval(curs));
		if (!eqstr(ntag(curs), "HUSB")) {
			char msg[512];
			snprintf(msg, sizeof(msg)/sizeof(msg[0]), _("Bad HUSB tag: %s"), ntag(curs));
			FATAL2(msg);
		}
		increment_table_int(memtab, indikey);
	}
	for (curs = wife; curs; curs = nsibling(curs)) {
		indikey = rmvat(nval(curs));
		if (!eqstr(ntag(curs), "WIFE")) {
			char msg[512];
			snprintf(msg, sizeof(msg)/sizeof(msg[0]), _("Bad HUSB tag: %s"), ntag(curs));
			FATAL2(msg);
		}
		increment_table_int(memtab, indikey);
	}

	/*
	Check that all listed persons contain family as FAMS as many times
	as expected
	*/
	tabit = begin_table_iter(memtab);
	while (next_table_int(tabit, &indikey, &count)) {
		NODE indi = key_to_indi(indikey);
		/*
		count how many times our main family (fkey)
		occurs in this person (indi) as a spousal family (FAMS)
		*/
		INT occur = 0;
		for (curs = nchild(indi); curs; curs = nsibling(curs)) {
			if (eqstr(ntag(curs), "FAMS")) {
				if (eqstr(nval(curs), fkey)) {
					++occur;
				}
			}
		}
		if (count != occur) {
			char msg[512];
			snprintf(msg, sizeof(msg)/sizeof(msg[0])
				, _("Mismatched lineage spouse links between %s and %s: %ld and %ld")
				, fkey, indikey, count, occur);
			FATAL2(msg);
		}
	}
	destroy_table(memtab);
	memtab = create_table_int();

	/*
	Make table listing all families this person is child in
	(& how many times each)
	*/
	for (curs = chil; curs; curs = nsibling(curs)) {
		indikey = rmvat(nval(curs));
		if (!eqstr(ntag(curs), "CHIL")) {
			char msg[512];
			snprintf(msg, sizeof(msg)/sizeof(msg[0]), _("Bad child tag: %s"), ntag(curs));
			FATAL2(msg);
		}
		increment_table_int(memtab, indikey);
	}

	/*
	Check that all listed families contain person as FAMC as many times
	as expected
	*/
	tabit = begin_table_iter(memtab);
	while (next_table_int(tabit, &indikey, &count)) {
		NODE indi = key_to_indi(indikey);
		/*
		count how many times our main family (fkey)
		occurs in this person (indi) as a parental family (FAMC)
		*/
		INT occur = 0;
		for (curs = nchild(indi); curs; curs = nsibling(curs)) {
			if (eqstr(ntag(curs), "FAMC")) {
				if (eqstr(nval(curs), fkey)) {
					++occur;
				}
			}
		}
		if (count != occur) {
			char msg[512];
			snprintf(msg, sizeof(msg)/sizeof(msg[0])
				, _("Mismatched lineage child links between %s and %s: %ld and %ld")
				, fkey, indikey, count, occur);
			FATAL2(msg);
		}
	}
	
	
	join_fam(fam, fref, husb, wife, chil, rest);
}
