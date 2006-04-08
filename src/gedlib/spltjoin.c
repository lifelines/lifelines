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
 * spltjoin.c -- Splits and joins persons and families
 * Copyright(c) 1993-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   3.0.0 - 16 May 94    3.0.2 - 21 Nov 94
 *   3.0.3 - 17 Jan 96
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void fix_cel(NODE root);
static void fix_children(NODE root);

/*======================================
 * split_indi -- Split person into parts
 *  all unrecognized (level 1) nodes go into body
 *====================================*/

/*
 2002.06.02, Perry
 I'm not sure if I need to replace split_indi_old with
 one that takes a RECORD as an argument
 */

/*======================================
 * split_indi_old -- Split person into parts
 *  all unrecognized (level 1) nodes go into body
 *  should be replaced by split_indi
 *====================================*/
void
split_indi_old (NODE indi,
            NODE *pname,
            NODE *prefn,
            NODE *psex,
            NODE *pbody,
            NODE *pfamc,
            NODE *pfams)
{
	NODE name, lnam, refn, sex, body, famc, fams, last;
	NODE lfmc, lfms, lref, prev, node;
	ASSERT(eqstr("INDI", ntag(indi)));
	name = sex = body = famc = fams = last = lfms = lfmc = lnam = NULL;
	refn = lref = NULL;
	node = nchild(indi);
	nchild(indi) = nsibling(indi) = NULL;
	while (node) {
		STRING tag = ntag(node);
		if (eqstr("NAME", tag)) {
			if (!name)
				name = lnam = node;
			else
				lnam = nsibling(lnam) = node;
		} else if (!sex && eqstr("SEX", tag)) {
			sex = node;
		} else if (eqstr("FAMC", tag)) {
			if (!famc)
				famc = lfmc = node;
			else
				lfmc = nsibling(lfmc) = node;
 		} else if (eqstr("FAMS", tag)) {
			if (!fams)
				fams = lfms = node;
			else
				lfms = nsibling(lfms) = node;
 		} else if (eqstr("REFN", tag)) {
			if (!refn)
				refn = lref = node;
			else
				lref = nsibling(lref) = node;
		} else {
			if (!body)
				body = last = node;
			else
				last = nsibling(last) = node;
		}
		prev = node;
		node = nsibling(node);
		nsibling(prev) = NULL;
	}
	*pname = name;
	*prefn = refn;
	*psex = sex;
	*pbody = body;
	*pfamc = famc;
	*pfams = fams;
}
/*====================================
 * join_indi -- Join person from parts
 *==================================*/
void
join_indi (NODE indi,
           NODE name,
           NODE refn,
           NODE sex,
           NODE body,
           NODE famc,
           NODE fams)
{
	NODE node = NULL;
	ASSERT(indi);
	ASSERT(eqstr("INDI", ntag(indi)));

	nchild(indi) = NULL;
	if (name) {
		nchild(indi) = node = name;
		while (nsibling(node))
			node = nsibling(node);
	}
	if (refn) {
		if (node)
			node = nsibling(node) = refn;
		else
			nchild(indi) = node = refn;
		while (nsibling(node))
			node = nsibling(node);
	}
	if (sex) {
		if (node)
			node = nsibling(node) = sex;
		else
			nchild(indi) = node = sex;
	}
	if (body) {
		if (node)
			node = nsibling(node) = body;
		else
			nchild(indi) = node = body;
		while (nsibling(node))
			node = nsibling(node);
	}
	if (famc) {
		if (node)
			node = nsibling(node) = famc;
		else
			nchild(indi) = node = famc;
		while (nsibling(node))
			node = nsibling(node);
	}
	if (fams) {
		if (node)
			nsibling(node) = fams;
		else
			nchild(indi) = fams;
	}
	/* fix parenthood of all children of indi */
	fix_children(indi);
	/* fix cache pointers of entire node tree */
	fix_cel(indi);
	/* validate entire node tree (if nodechecking on) */
	nodechk(indi, "join_indi");
}
/*=======================================
 * split_fam -- Split a family into parts
 *  all unrecognized (level 1) nodes go into rest
 *  fam:   [I/O] node root of family
 *  prefn: [OUT] first REFN (with others as sibs)
 *  phusb: [OUT] first HUSB (with others as sibs)
 *  pwife: [OUT] first WIFE (with others as sibs)
 *  pchil: [OUT] first CHIL (with others as sibs)
 *  prest: [OUT] first of rest (with others as sibs)
 * family is broken apart, so each section (eg, husbands)
 *  is not connected to other sections
 *=====================================*/
void
split_fam (NODE fam, NODE *prefn, NODE *phusb, NODE *pwife, NODE *pchil
	, NODE *prest)
{
	NODE node, rest, last, husb, lhsb, wife, lwfe, chil, lchl;
	NODE prev, refn, lref;
	STRING tag;

	rest = last = husb = wife = chil = lchl = lhsb = lwfe = NULL;
	prev = refn = lref = NULL;
	node = nchild(fam);
	nchild(fam) = nsibling(fam) = NULL;
	while (node) {
		tag = ntag(node);
		if (eqstr("HUSB", tag)) {
			if (husb)
				lhsb = nsibling(lhsb) = node;
			else
				husb = lhsb = node;
		} else if (eqstr("WIFE", tag)) {
			if (wife)
				lwfe = nsibling(lwfe) = node;
			else
				wife = lwfe = node;
		} else if (eqstr("CHIL", tag)) {
			if (chil)
				lchl = nsibling(lchl) = node;
			else
				chil = lchl = node;
		} else if (eqstr("REFN", tag)) {
			if (refn)
				lref = nsibling(lref) = node;
			else
				refn = lref = node;
		} else {
			if (rest)
				last = nsibling(last) = node;
			else
				last = rest = node;
		}
		prev = node;
		node = nsibling(node);
		nsibling(prev) = NULL;
	}
	*prefn = refn;
	*phusb = husb;
	*pwife = wife;
	*pchil = chil;
	*prest = rest;
}
/*===================================
 * join_fam -- Join family from parts
 *  fam:  [I/O] FAM root node
 *  refn: [I/O] REFN node(s)
 *  husb: [I/O] HUSB node(s)
 *  wife: [I/O] WIFE node(s)
 *  chil: [I/O] CHILD node(s)
 *  rest: [I/O] other node(s)
 * Only call on pieces previously split by split_fam.
 * This reassembles pieces, linking them back together
 *=================================*/
void
join_fam (NODE fam, NODE refn, NODE husb, NODE wife, NODE chil, NODE rest)
{
	NODE node = NULL;

	nchild(fam) = NULL;
	if (refn) {
		nchild(fam) = node = refn;
		while (nsibling(node))
			node = nsibling(node);
	}
	if (husb) {
		if (node)
			node = nsibling(node) = husb;
		else
			nchild(fam) = node = husb;
		while (nsibling(node))
			node = nsibling(node);
	}
	if (wife) {
		if (node)
			node = nsibling(node) = wife;
		else
			nchild(fam) = node = wife;
		while (nsibling(node))
			node = nsibling(node);
	}
	if (rest) {
		if (node)
			node = nsibling(node) = rest;
		else
			nchild(fam) = node = rest;
		while (nsibling(node))
			node = nsibling(node);
	}
	if (chil) {
		if (node)
			nsibling(node) = chil;
		else
			nchild(fam) = chil;
	}
	/* fix parenthood of all children of fam */
	fix_children(fam);
	/* fix cache pointers of entire node tree */
	fix_cel(fam);
	/* validate entire node tree (if nodechecking on) */
	nodechk(fam, "join_fam");
}
/*=======================================
 * split_othr -- Split a misc node tree into parts
 *  all unrecognized (level 1) nodes go into rest
 *  root:  [I/O] node root
 *  prefn: [OUT] first REFN (with others as sibs)
 *  prest: [OUT] first of rest (with others as sibs)
 * node tree is broken apart, so each section (eg, refns)
 *  is not connected to other sections
 *=====================================*/
void
split_othr (NODE root, NODE *prefn, NODE *prest)
{
	NODE node, rest, last;
	NODE prev, refn, lref;
	STRING tag;

	rest = last = NULL;
	prev = refn = lref = NULL;
	node = nchild(root);
	nchild(root) = nsibling(root) = NULL;
	while (node) {
		tag = ntag(node);
		if (eqstr("REFN", tag)) {
			if (refn)
				lref = nsibling(lref) = node;
			else
				refn = lref = node;
		} else if (rest)
			last = nsibling(last) = node;
		else
			last = rest = node;
		prev = node;
		node = nsibling(node);
		nsibling(prev) = NULL;
	}
	*prefn = refn;
	*prest = rest;
}
/*===================================
 * join_othr -- Join misc. node tree from parts
 *  root: [I/O] root node
 *  refn: [I/O] REFN node(s)
 *  rest: [I/O] other node(s)
 * Only call on pieces previously split by split_othr.
 * This reassembles pieces, linking them back together
 *=================================*/
void
join_othr (NODE root, NODE refn, NODE rest)
{
	NODE node = NULL;

	nchild(root) = NULL;
	if (refn) {
		nchild(root) = node = refn;
		while (nsibling(node))
			node = nsibling(node);
	}
	if (rest) {
		if (node)
			node = nsibling(node) = rest;
		else
			nchild(root) = node = rest;
		while (nsibling(node))
			node = nsibling(node);
	}
}
/*==================================================
 * normalize_rec -- ensure nodes are in lifelines order
 * for any record
 *================================================*/
void
normalize_rec (RECORD irec)
{
	NODE root = nztop(irec);
	if (!root) return;
	if (eqstr(ntag(root), "INDI"))
		normalize_indi(root);
	else if (eqstr(ntag(root), "FAM"))
		normalize_fam(root);
}
/*==================================================
 * normalize_irec -- ensure nodes are in lifelines order
 * for individual record
 *================================================*/
void
normalize_irec (RECORD irec)
{
	NODE indi=nztop(irec);
	if (indi)
		normalize_indi(indi);
}
/*==================================================
 * normalize_indi -- ensure nodes are in lifelines order
 * for individual root node
 *================================================*/
void
normalize_indi (NODE indi)
{
	NODE name, refn, sex, body, famc, fams;
	
	split_indi_old(indi, &name, &refn, &sex, &body, &famc, &fams);
	ASSERT(eqstr(ntag(indi), "INDI"));
	join_indi(indi, name, refn, sex, body, famc, fams);
}
/*==================================================
 * normalize_fam -- ensure nodes are in lifelines order
 *================================================*/
void
normalize_fam (NODE fam)
{
	NODE fref, husb, wife, chil, rest;

	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
	ASSERT(eqstr(ntag(fam), "FAM"));
	join_fam(fam, fref, husb, wife, chil, rest);
}
/*=======================================
 * fix_children -- Set parent pointers of all immediate children
 *=====================================*/
static void
fix_children (NODE root)
{
	NODE node=0;
	for (node = nchild(root); node; node = nsibling(node)) {
		nparent(node) = root;
	}
}
/*=======================================
 * fix_cel -- Set cel of all descendants to agree with root
 *=====================================*/
static void
fix_cel (NODE root)
{
	set_all_nodetree_to_root_cel(root);
}
