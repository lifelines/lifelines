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
/*================================================================
 * gedcom.c -- Read and convert GEDCOM records to LifeLines format
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 08 May 94    3.0.2 - 01 Dec 94
 *==============================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "sequence.h"
#include "gedcheck.h"
#include "indiseq.h"
#include "liflines.h"
#include "feedback.h"
#include "lloptions.h"

static NODE indi_to_indi(NODE);
static NODE fam_to_fam(NODE);
static NODE even_to_even(NODE);
static NODE sour_to_sour(NODE);
static NODE othr_to_othr(NODE);

STRING noiref = (STRING) "FAM record has no INDI references; record ignored.\n";

/*========================================================
 * node_to_node -- Convert GEDCOM record to LifeLines form
 *======================================================*/
NODE
node_to_node (NODE node,
              INT *ptype)
{
	*ptype = 0;
	if (eqstr("HEAD", ntag(node)) ||
	    eqstr("TRLR", ntag(node))) return NULL;
	if (eqstr("INDI", ntag(node)))      *ptype = INDI_REC;
	else if (eqstr("FAM",  ntag(node))) *ptype = FAM_REC;
	else if (eqstr("EVEN", ntag(node))) *ptype = EVEN_REC;
	else if (eqstr("SOUR", ntag(node))) *ptype = SOUR_REC;
	else                                *ptype = OTHR_REC;
	switch (*ptype) {
	case INDI_REC: return indi_to_indi(node);
	case FAM_REC:  return fam_to_fam(node);
	case EVEN_REC: return even_to_even(node);
	case SOUR_REC: return sour_to_sour(node);
	case OTHR_REC: return othr_to_othr(node);
	default: FATAL();
	}
	return NULL;		/* keep compiler happy */
}
/*=======================================================
 * indi_to_indi - Convert person record to LifeLines form
 *=====================================================*/
static NODE
indi_to_indi (NODE indi)
{
	NODE node, last, name, famc, lfmc, fams, lfms, frst, prev = 0;
	last = name = famc = fams = frst = lfmc =lfms = NULL;
	node = nchild(indi);
	while (node) {
		STRING tag = ntag(node);
		if (eqstr("NAME", tag) && !name)
			name = node;
		else if (eqstr("FAMS", tag)) {
			if (!fams)
				lfms = fams = node;
			else
				lfms = nsibling(lfms) = node;
		} else if (eqstr("FAMC", tag)) {
			if (!famc)
				lfmc = famc = node;
			else
				lfmc = nsibling(lfmc) = node;
		} else {
			if (!frst)
				frst = last = node;
			else
				last = nsibling(last) = node;
		}
		prev = node;
		node = nsibling(node);
		nsibling(prev) = NULL;
	}
	if (name) {
		nchild(indi) = node = name;
	} else {
		if (getlloptint("RequireNames", 0)) {
			llwprintf(_("Missing NAME line in INDI record; record ignored.\n"));
			return NULL;
		}
		nchild(indi) = node = 0;
	}
	if (frst) {
		if (node) {
			nsibling(node) = frst;
		} else {
			nchild(indi) = frst;
		}
		node = last;
	}
	if (famc) {
		if (node) {
			nsibling(node) = famc;
		} else {
			nchild(indi) = famc;
		}
		node = lfmc;
	}
	if (fams) {
		if (node) {
			nsibling(node) = fams;
		} else {
			nchild(indi) = fams;
		}
	}
	return indi;
}
/*======================================================
 * fam_to_fam -- Convert family record to LifeLines form
 *====================================================*/
static NODE
fam_to_fam (NODE fam)
{
	NODE node, frst, last, husb, wife, chil, lchl, prev;
	frst = last = husb = wife = chil = lchl = NULL;
	node = nchild(fam);
	while (node) {
		STRING tag = ntag(node);
		if (eqstr("HUSB", tag) && !husb)
			husb = node;
		else if (eqstr("WIFE", tag) && !wife)
			wife = node;
		else if (eqstr("CHIL", tag)) {
			if (!chil)
				chil = lchl = node;
			else
				lchl = nsibling(lchl) = node;
		} else {
			if (!frst)
				last = frst = node;
			else
				last = nsibling(last) = node;
		}
		prev = node;
		node = nsibling(node);
		nsibling(prev) = NULL;
	}
	node = NULL;
	/* We do not skip over empty families (!husb && !wife && !chil),
	lest we create dangling pointers from individuals to empty families */
	if (husb) nchild(fam) = node = husb;
	if (wife) {
		if (!node)
			nchild(fam) = node = wife;
		else
			node = nsibling(node) = wife;
	}
	if (frst) {
		if (!node)
			nchild(fam) = frst;
		else
			nsibling(node) = frst;
		node = last;
	}
	if (chil) {
		if (!node)
			nchild(fam) = chil;
		else
			nsibling(node) = chil;
	}
	return fam;
}
/*=======================================================
 * even_to_even -- Convert event record to LifeLines form
 *=====================================================*/
static NODE
even_to_even (NODE evn)
{
	return evn;
}
/*========================================================
 * sour_to_sour -- Convert source record to LifeLines form
 *======================================================*/
static NODE
sour_to_sour (NODE src)
{
	return src;
}
/*=======================================================
 * othr_to_othr -- Convert other record to LifeLines form
 *=====================================================*/
static NODE
othr_to_othr (NODE oth)
{
	return oth;
}
/*==================================================
 * node_to_nkey -- get NKEY from NODE (if exists)
 *================================================*/
BOOLEAN
node_to_nkey (NODE node, NKEY * nkey)
{
	char keynum[MAXKEYWIDTH];
	nkey_clear(nkey);
	if (!node) {
		*nkey = nkey_zero();
		return FALSE;
	}
	strcpy(nkey->key, node_to_key(node));
	strcpy(keynum,&nkey->key[1]);
	nkey->ntype = nkey->key[0];
	nkey->keynum = atoi(keynum);
	return TRUE;
}
/*==================================================
 * nkey_to_node -- get NODE from nkey (if exists)
 * TODO: delete this when no longer used, Perry, 2002/06/24
 *================================================*/
BOOLEAN
nkey_to_node (NKEY * nkey, NODE * node)
{
	RECORD rec=0;
	if (!nkey_to_record(nkey, &rec))
		return FALSE;
	*node = nztop(rec);
	return TRUE;
}
/*==================================================
 * nkey_to_record -- get RECORD from nkey (if exists)
 *  else return NULL
 *================================================*/
BOOLEAN
nkey_to_record (NKEY * nkey, RECORD * prec)
{
	if (!nkey->keynum) {
		*prec = NULL;
		return FALSE;
	}
	nkey_load_key(nkey);
	*prec = qkey_to_record(nkey->key);
	return *prec != NULL;
}
/*==================================================
 * nkey_load_key -- load key field of NKEY (if not loaded)
 *================================================*/
void
nkey_load_key (NKEY * nkey)
{
	char key[MAXKEYWIDTH+1];
	if (nkey->key)
		return;
	sprintf(key, "%c%ld", nkey->ntype, nkey->keynum);
	strcpy(nkey->key, key);
}
/*==================================================
 * nkey_eq -- compare two NKEYs
 *================================================*/
BOOLEAN
nkey_eq (NKEY * nkey1, NKEY * nkey2)
{
	if (nkey1->ntype != nkey2->ntype) return FALSE;
	if (nkey1->keynum != nkey2->keynum) return FALSE;
	return TRUE;
}
/*==================================================
 * nkey_copy -- copy NKEY
 *================================================*/
void
nkey_copy (NKEY * src, NKEY * dest)
{
	nkey_clear(dest);
	strcpy(dest->key, src->key);
	dest->keynum = src->keynum;
	dest->ntype = src->ntype;
}
/*==================================================
 * nkey_clear -- clear NKEY & free memory
 *================================================*/
void
nkey_clear (NKEY * nkey)
{
	strcpy(nkey->key, "");
	nkey->keynum = 0;
}
/*==================================================
 * nkey_zero -- return a null NKEY
 *================================================*/
NKEY
nkey_zero (void)
{
	static NKEY nkey; /* all zeros */
	return nkey;
}
