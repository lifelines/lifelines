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

/*======================================
 * split_indi -- Split person into parts
 *  all unrecognized (level 1) nodes go into body
 *====================================*/
void
split_indi (NODE indi,
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
	ASSERT(indi && eqstr("INDI", ntag(indi)));

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
}
/*=======================================
 * split_fam -- Split a family into parts
 *  all unrecognized (level 1) nodes go into rest
 *=====================================*/
void
split_fam (NODE fam,
           NODE *prefn,
           NODE *phusb,
           NODE *pwife,
           NODE *pchil,
           NODE *prest)
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
		} else if (rest)
			last = nsibling(last) = node;
		else
			last = rest = node;
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
 *=================================*/
void
join_fam (NODE fam,
          NODE refn,
          NODE husb,
          NODE wife,
          NODE chil,
          NODE rest)
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
}
