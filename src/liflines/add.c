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
 * add.c -- Add new person or family to database; add child to
 *   family; add spouse to family
 * Copyright(c) 1992-95 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   2.3.6 - 29 Oct 93    3.0.0 - 23 Sep 94
 *   3.0.2 - 12 Dec 94    3.0.3 - 20 Jan 96
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "feedback.h"
#include "lloptions.h"

#include "llinesi.h"

extern STRING idcfam, fredit, cffadd, idprnt, unksex, idsbln, mklast;
extern STRING idsadd, idsinf, kchild, iscinf, notopp, idsps1, idsps2;
extern STRING nosex,  hashsb, haswif, idchld, gdfadd, cfcadd, iredit;
extern STRING cfpadd, cfsadd, gdpadd, gdcadd, gdsadd, ronlya, ronlye;

extern TRANTABLE tran_tables[];


/*==========================================================
 * add_indi_by_edit -- Add new person to database by editing
 * (with user interaction)
 *========================================================*/
RECORD
add_indi_by_edit (void)
{
	FILE *fp;
	RECORD indi0=0;
	NODE indi=0;
	STRING str, msg;
	BOOLEAN emp;
	TRANTABLE tti = tran_tables[MEDIN];

	if (readonly) {
		message(_(ronlya));
		return NULL;
	}

/* Create person template for user to edit */

	if (!(fp = fopen(editfile, LLWRITETEXT))) return NULL;
	/* prefer useroption in this db */
	if ((str = getoptstr("INDIREC", NULL)))
		fprintf(fp, "%s\n", str);
	else { /* default */
		fprintf(fp, "0 INDI\n1 NAME Fname/Surname\n1 SEX MF\n");
		fprintf(fp, "1 BIRT\n  2 DATE\n  2 PLAC\n");
		fprintf(fp, "1 DEAT\n  2 DATE\n  2 PLAC\n1 SOUR\n");
	}

/* Have user edit new person record */

	fclose(fp);
	do_edit();
	while (TRUE) {
		indi0 = file_to_record(editfile, tti, &msg, &emp);
		if (!indi0) {
			if (ask_yes_or_no_msg(msg, iredit)) {
				do_edit();
				continue;
			} 
			break;
		}
		indi = nztop(indi0);
		if (!valid_indi_old(indi, &msg, NULL)) {
			if (ask_yes_or_no_msg(msg, iredit)) {
				do_edit();
				continue;
			}
			free_rec(indi0);
			indi0 = NULL;
			break;
		}
		break;
	}
	if (!indi0 || !ask_yes_or_no(cfpadd)) {
		if (indi0) free_rec(indi0);
		return NULL;
	}
	return add_unlinked_indi(indi0);
}
/*==========================================================
 * add_unlinked_indi -- Add person with no links to database
 * (no user interaction)
 * creates record & adds to cache
 *========================================================*/
RECORD
add_unlinked_indi (RECORD indi0)
{
	NODE name, refn, sex, body, dumb, node;
	STRING key;
	INT keynum;
	TRANTABLE ttd = tran_tables[MINDS];
	NODE indi = nztop(indi0);

	split_indi_old(indi, &name, &refn, &sex, &body, &dumb, &dumb);
	keynum = getixrefnum();
	init_new_record(indi0, 'I', keynum);
	key = nzkey(indi0);
	for (node = name; node; node = nsibling(node))
		add_name(nval(node), key);
	for (node = refn; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);
	join_indi(indi, name, refn, sex, body, NULL, NULL);
	resolve_links(indi);
	indi_to_dbase(indi);
	indi_to_cache(indi0);
	msg_status(gdpadd, indi_to_name(indi, ttd, 35));
	return indi0;
}
/*================================================================
 * add_linked_indi -- Add linked person to database; links assumed
 *   correct
 * (no user interaction)
 *==============================================================*/
BOOLEAN
add_linked_indi (NODE indi)
{
	NODE node, name, refn, sex, body, famc, fams;
	STRING str, key;

	split_indi_old(indi, &name, &refn, &sex, &body, &famc, &fams);
	key = rmvat(nxref(indi));
	for (node = name; node; node = nsibling(node))
		add_name(nval(node), key);
	for (node = refn; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);
	join_indi(indi, name, refn, sex, body, famc, fams);
	resolve_links(indi);
	str = node_to_string(indi);
	store_record(key, str, strlen(str));
	stdfree(str);
	return TRUE;
}
/*========================================================
 * ask_child_order --  ask user in what order to put child
 * (with user interaction)
 *  fam:     [in] children of this family
 *  promptq: [in] what question to ask
 *  rfmt:    [in] reformatting info
 *======================================================*/
INT
ask_child_order (NODE fam, PROMPTQ promptq, RFMT rfmt)
{
	INT i, nchildren;
	STRING *childstrings, *childkeys;
/* If first child in family, confirm and add */

	childstrings = get_child_strings(fam, rfmt, &nchildren, &childkeys);
	if (nchildren == 0) {
		if (promptq == ALWAYS_PROMPT && !ask_yes_or_no(cfcadd))
				return -1;
		i=0;
/* If not first, find where child belongs */
	} else {
		childstrings[nchildren] = mklast;
		i = choose_from_array(idcfam, nchildren+1, childstrings);
	}
	return i;
}
/*==================================
 * add_child --  Add child to family
 * (with user interaction)
 *  child: [in] new child to add
 *  fam:   [in] family to which to add
 *================================*/
NODE
add_child (NODE child, NODE fam)
{
	INT i;
	TRANTABLE ttd = tran_tables[MINDS];

	if (readonly) {
		message(_(ronlye));
		return NULL;
	}

/* Identify child if caller did not */

	if (!child) child = ask_for_indi_old(idchld, NOCONFIRM, DOASK1);
	if (!child) return NULL;

/* Identify family if caller did not */

	if (!fam) fam = ask_for_fam(idprnt, idsbln);
	if (!fam) return NULL;

	i = ask_child_order(fam, ALWAYS_PROMPT, &disp_shrt_rfmt);
	if (i == -1) return NULL;

/* Add FAMC node to child */

	add_child_to_fam(child, fam, i);
	msg_status(gdcadd, indi_to_name(child, ttd, 35));
	return fam;
}

/*========================================
 * add_child_to_fam -- Add child to family
 * (no user interaction)
 *======================================*/
void
add_child_to_fam (NODE child, NODE fam, INT i)
{
	NODE node, new, name, sex, body, famc, fams;
	INT j;
	NODE husb, wife, chil, rest, refn, fref;
	NODE nfmc, this, prev;

	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
	prev = NULL;
	node = chil;
	j = 0;
	if (i == -1) { /* add last */
		if (node) {
			while (node) {
				prev = node;
				node = nsibling(node);
			}
		}
	}
	else {
		while (j++ < i) {
			prev = node;
			node = nsibling(node);
		}
	}
	new = create_node(NULL, "CHIL", nxref(child), fam);
	nsibling(new) = node;
	if (prev)
		nsibling(prev) = new;
	else
		chil = new;
	join_fam(fam, fref, husb, wife, chil, rest);

/* Add FAMC node to child */

	split_indi_old(child, &name, &refn, &sex, &body, &famc, &fams);
	nfmc = create_node(NULL, "FAMC", nxref(fam), child);
	prev = NULL;
	this = famc;
	while (this) {
		prev = this;
		this = nsibling(this);
	}
	if (!prev)
		famc = nfmc;
	else
		nsibling(prev) = nfmc;
	join_indi(child, name, refn, sex, body, famc, fams);

/* Write updated records to database */

	resolve_links(child);
	resolve_links(fam);
	fam_to_dbase(fam);
	indi_to_dbase(child);
}
/*===================================
 * add_spouse -- Add spouse to family
 * prompt for family & confirm if needed
 * (with user interaction)
 *=================================*/
BOOLEAN
add_spouse (NODE spouse,
            NODE fam,
            BOOLEAN conf)
{
	INT sex;
	NODE husb, wife, chil, rest, fref;

	if (readonly) {
		message(_(ronlye));
		return FALSE;
	}

/* Identify spouse to add to family */

	if (!spouse) spouse = ask_for_indi_old(idsadd, NOCONFIRM, DOASK1);
	if (!spouse) return FALSE;
	if ((sex = SEX(spouse)) == SEX_UNKNOWN) {
		message(nosex);
		return FALSE;
	}

/* Identify family to add spouse to */

	if (!fam) fam = ask_for_fam(idsinf, kchild);
	if (!fam) return FALSE;

/* Check that spouse can be added */

	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
	join_fam(fam, fref, husb, wife, chil, rest);
#if 0
	if (sex == SEX_MALE && husb) {
		message(hashsb);
		return FALSE;
	}
	if (sex == SEX_FEMALE && wife) {
		message(haswif);
		return FALSE;
	}
#endif
	if (conf && !ask_yes_or_no(cfsadd)) return FALSE;

	add_spouse_to_fam(spouse, fam, sex);
	return TRUE;
}
/*===================================
 * add_spouse_to_fam -- Add spouse to family
 * after all user input
 * (no user interaction)
 *=================================*/
void
add_spouse_to_fam (NODE spouse, NODE fam, INT sex)
{
/* Add HUSB or WIFE node to family */
	NODE husb, wife, chil, rest, fams, prev, fref, this, new;
	TRANTABLE ttd = tran_tables[MINDS];

	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
	if (sex == SEX_MALE) {
		prev = NULL;
		this = husb;
		while (this) {
			prev = this;
			this = nsibling(this);
		}
		new = create_node(NULL, "HUSB", nxref(spouse), fam);
		if (prev)
			nsibling(prev) = new;
		else
			husb = new;
	} else {
		prev = NULL;
		this = wife;
		while (this) {
			prev = this;
			this = nsibling(this);
		}
		new = create_node(NULL, "WIFE", nxref(spouse), fam);
		if (prev)
			nsibling(prev) = new;
		else
			wife = new;
	}
	join_fam(fam, fref, husb, wife, chil, rest);

/* Add FAMS node to spouse */

	fams = create_node(NULL, "FAMS", nxref(fam), spouse);
	prev = NULL;
	this = nchild(spouse);
	while (this) {
		prev = this;
		this = nsibling(this);
	}
	ASSERT(prev);
	nsibling(prev) = fams;

/* Write updated records to database */

	resolve_links(spouse);
	resolve_links(fam);
	indi_to_dbase(spouse);
	fam_to_dbase(fam);
	msg_status(gdsadd, indi_to_name(spouse, ttd, 35));
}
/*=========================================
 * add_members_to_family -- Add members to new family
 * (no user interaction)
 *=======================================*/
static void
add_members_to_family (STRING xref, NODE spouse1, NODE spouse2, NODE child)
{
	NODE refn, body;
	NODE name, sex, famc, fams, node, prev, new, this;
	if (spouse1) {
		new = create_node(NULL, "FAMS", xref, spouse1);
		prev = NULL;
		node = nchild(spouse1);
		while (node) {
			prev = node;
			node = nsibling(node);
		}
		if (prev)
			nsibling(prev) = new;
		else
			nchild(spouse1) = new;
	}
	if (spouse2) {
		new = create_node(NULL, "FAMS", xref, spouse2);
		prev = NULL;
		node = nchild(spouse2);
		while (node) {
			prev = node;
			node = nsibling(node);
		}
		if (prev)
			nsibling(prev) = new;
		else
			nchild(spouse2) = new;
	}
	if (child) {
		split_indi_old(child, &name, &refn, &sex, &body, &famc, &fams);
		new = create_node(NULL, "FAMC", xref, child);
		prev = NULL;
		this = famc;
		while (this) {
			prev = this;
			this = nsibling(this);
		}
		if (prev)
			nsibling(prev) = new;
		else
			famc = new;
		join_indi(child, name, refn, sex, body, famc, fams);
	}
}
/*=========================================
 * add_family -- Add new family to database
 * (with user interaction)
 *=======================================*/
NODE
add_family (NODE spouse1,
            NODE spouse2,
            NODE child)
{
	INT sex1 = 0;
	INT sex2 = 0;
	NODE fam1, fam2=0, refn, husb, wife, chil, body;
	NODE node;
	TRANTABLE tti = tran_tables[MEDIN], tto = tran_tables[MINED];
	STRING xref, msg, key, str;
	BOOLEAN emp;
	FILE *fp;

	if (readonly) {
		message(_(ronlya));
		return NULL;
	}

/* Handle case where child is known */

	if (child)  {
		spouse1 = spouse2 = NULL;
		goto editfam;
	}

/* Identify first spouse */

	if (!spouse1) spouse1 = ask_for_indi_old(idsps1, NOCONFIRM, NOASK1);
	if (!spouse1) return NULL;
	if ((sex1 = SEX(spouse1)) == SEX_UNKNOWN) {
		message(unksex);
		return NULL;
	}

/* Identify optional spouse */

	if (!spouse2) spouse2 = ask_for_indi_old(idsps2, NOCONFIRM, DOASK1);
	if (spouse2) {
		if ((sex2 = SEX(spouse2)) == SEX_UNKNOWN || sex1 == sex2) {
			message(notopp);
			return NULL;
		}
	}

/* Create new family */

editfam:
	fam1 = create_node(NULL, "FAM", NULL, NULL);
	husb = wife = chil = NULL;
	if (spouse1) {
		if (sex1 == SEX_MALE)
			husb = create_node(NULL, "HUSB", nxref(spouse1), fam1);
		else
			wife = create_node(NULL, "WIFE", nxref(spouse1), fam1);
	}
	if (spouse2) {
		if (sex2 == SEX_MALE)
			husb = create_node(NULL, "HUSB", nxref(spouse2), fam1);
		else
			wife = create_node(NULL, "WIFE", nxref(spouse2), fam1);
	}
	if (child)
		chil = create_node(NULL, "CHIL", nxref(child), fam1);

/* Prepare file for user to edit */

	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	write_nodes(0, fp, tto, fam1, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, husb, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, wife, TRUE, TRUE, TRUE);
	/* prefer user option in db */
	if ((str = getoptstr("FAMRECBODY", NULL)))
		fprintf(fp, "%s\n", str);
	else /* default */
		fprintf(fp, "1 MARR\n  2 DATE\n  2 PLAC\n  2 SOUR\n");
	write_nodes(1, fp, tto, chil, TRUE, TRUE, TRUE);
	fclose(fp);
	join_fam(fam1, NULL, husb, wife, chil, NULL);

/* Have user edit family info */

	do_edit();
	while (TRUE) {
		fam2 = file_to_node(editfile, tti, &msg, &emp);
		if (!fam2) {
			if (ask_yes_or_no_msg(msg, fredit)) {
				do_edit();
				continue;
			}
			break;
		}
		if (!valid_fam_old(fam2, &msg, fam1)) {
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

/* Confirm family add operation */

	free_nodes(fam1);
	if (!fam2 || !ask_yes_or_no(cffadd)) {
		free_nodes(fam2);
		return NULL;
	}
	nxref(fam2) = strsave(xref = getfxref());

/* Modify spouse/s and/or child */

	add_members_to_family(xref, spouse1, spouse2, child);

/* Write updated records to database */

	split_fam(fam2, &refn, &husb, &wife, &chil, &body);
	key = rmvat(nxref(fam2));
	for (node = refn; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);
	join_fam(fam2, refn, husb, wife, chil, body);
	resolve_links(fam2);
	resolve_links(spouse1);
	resolve_links(spouse2);
	resolve_links(child);
	fam_to_dbase(fam2);
	fam_to_cache(fam2);
	if (spouse1) indi_to_dbase(spouse1);
	if (spouse2) indi_to_dbase(spouse2);
	if (child) indi_to_dbase(child);
	message(gdfadd);
	return fam2;
}
#ifdef ETHEL
/*=========================================
 * add_family_to_db -- Add new family to database
 * (no user interaction)
 * This is stolen from add_family
 * and may not be terribly efficient - Perry
 *=======================================*/
NODE
add_family_to_db (NODE spouse1, NODE spouse2, NODE child)
{
	INT sex1 = spouse1 ? SEX(spouse1) : SEX_UNKNOWN;
	INT sex2 = spouse1 ? SEX(spouse2) : SEX_UNKNOWN;
	NODE fam1, fam2, refn, husb, wife, chil, body;
	NODE node;
	TRANTABLE tti = tran_tables[MEDIN], tto = tran_tables[MINED];
	STRING xref, msg, key;
	BOOLEAN emp;
	FILE *fp;

	fam1 = create_node(NULL, "FAM", NULL, NULL);
	husb = wife = chil = NULL;
	if (spouse1) {
		if (sex1 == SEX_MALE)
			husb = create_node(NULL, "HUSB", nxref(spouse1), fam1);
		else
			wife = create_node(NULL, "WIFE", nxref(spouse1), fam1);
	}
	if (spouse2) {
		if (sex2 == SEX_MALE)
			husb = create_node(NULL, "HUSB", nxref(spouse2), fam1);
		else
			wife = create_node(NULL, "WIFE", nxref(spouse2), fam1);
	}
	if (child)
		chil = create_node(NULL, "CHIL", nxref(child), fam1);

/* Create file */

	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	write_nodes(0, fp, tto, fam1, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, husb, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, wife, TRUE, TRUE, TRUE);
	fprintf(fp, "1 MARR\n  2 DATE\n  2 PLAC\n  2 SOUR\n");
	write_nodes(1, fp, tto, chil, TRUE, TRUE, TRUE);
	fclose(fp);
	join_fam(fam1, NULL, husb, wife, chil, NULL);

	fam2 = file_to_node(editfile, tti, &msg, &emp);

	free_nodes(fam1);

	nxref(fam2) = strsave(xref = getfxref());

/* Modify spouse/s and/or child */

	add_members_to_family(xref, spouse1, spouse2, child);

	split_fam(fam2, &refn, &husb, &wife, &chil, &body);
	key = rmvat(nxref(fam2));
	for (node = refn; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);
	join_fam(fam2, refn, husb, wife, chil, body);
	resolve_links(fam2);
	resolve_links(spouse1);
	resolve_links(spouse2);
	resolve_links(child);
	fam_to_dbase(fam2);
	fam_to_cache(fam2);
	if (spouse1) indi_to_dbase(spouse1);
	if (spouse2) indi_to_dbase(spouse2);
	if (child) indi_to_dbase(child);

	return fam2;
}
#endif /* ETHEL */
