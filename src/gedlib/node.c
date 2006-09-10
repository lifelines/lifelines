/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * node.c -- Standard GEDCOM NODE operations
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 04 Sep 93
 *   3.0.0 - 29 Aug 94    3.0.2 - 23 Dec 94
 *   3.0.3 - 16 Jan 96
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "feedback.h"
#include "metadata.h"
#include "lloptions.h"
#include "date.h"
#include "vtable.h"

/*********************************************
 * global/exported variables
 *********************************************/


/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSfileof, qSreremp, qSrerlng, qSrernlv, qSrerinc;
extern STRING qSrerbln, qSrernwt, qSrerilv, qSrerwlv, qSunsupunix, qSunsupuniv;

/*********************************************
 * local types
 *********************************************/

/* node allocator's freelist */
typedef struct blck *NDALLOC;
struct blck { NDALLOC next; };

/*********************************************
 * local enums & defines
 *********************************************/

enum { NEW_RECORD, EXISTING_LACKING_WH_RECORD };

/*********************************************
 * local function prototypes, alphabetical
 *********************************************/

static NODE alloc_node(void);
static STRING fixup(STRING str);
static STRING fixtag (STRING tag);
static RECORD indi_to_prev_sib_impl(NODE indi);
static void node_destructor(VTABLE *obj);
static INT node_strlen(INT levl, NODE node);

/*********************************************
 * unused local function prototypes
 *********************************************/

#ifdef UNUSED_CODE
static BOOLEAN all_digits (STRING);
NODE children_nodes(NODE faml);
NODE father_nodes(NODE faml);
NODE mother_nodes(NODE faml);
NODE parents_nodes(NODE faml);
#endif /* UNUSED_CODE */

/*********************************************
 * local variables
 *********************************************/

/* node allocator's free list */
static NDALLOC first_blck = (NDALLOC) 0;
static INT live_count = 0;

static struct tag_vtable vtable_for_node = {
	VTABLE_MAGIC
	, "node"
	, &node_destructor
	, &refcountable_isref
	, &refcountable_addref
	, &refcountable_release
	, 0 /* copy_fnc */
	, &generic_get_type_name
};

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==============================
 * fixup -- Save non-tag strings
 *============================*/
static STRING
fixup (STRING str)
{
	if (!str || *str == 0) return NULL;
	return strsave(str);
}
/*=============================
 * fixtag -- Keep tags in table
 * returns pointer to table's memory
 *===========================*/
static STRING
fixtag (STRING tag)
{
	STRING str = valueof_str(tagtable, tag);
	if (!str) {
		insert_table_str(tagtable, tag, tag);
		str = valueof_str(tagtable, tag);
	}
	return str;
}
/*=====================================
 * change_node_tag -- Give new tag to node
 *===================================*/
void
change_node_tag (NODE node, STRING newtag)
{
	/* tag belongs to tagtable, so don't free old one */
	ntag(node) = fixtag(newtag);
}
/*=====================================
 * alloc_node -- Special node allocator
 *===================================*/
static NODE
alloc_node (void)
{
	NODE node;
	NDALLOC blck;
	int i;
	if (first_blck == (NDALLOC) 0) {
		node = (NODE) stdalloc(100*sizeof(*node));
		first_blck = (NDALLOC) node;
		for (i = 1; i <= 99; i++) {
			blck = (NDALLOC) node;
			blck->next = (NDALLOC) (node + 1);
			node++;
		}
		((NDALLOC) node)->next = (NDALLOC) 0;
	}
	node = (NODE) first_blck;
	first_blck = first_blck->next;
	++live_count;
	return node;
}
/*======================================
 * free_node -- Special node deallocator
 *====================================*/
void
free_node (NODE node)
{
	if (nxref(node)) stdfree(nxref(node));
	if (nval(node)) stdfree(nval(node));

	/*
	tag is pointer into shared tagtable
	which we cannot delete until all nodes are freed
	*/
	((NDALLOC) node)->next = first_blck;
	first_blck = (NDALLOC) node;
	--live_count;
}
/*===========================
 * create_node -- Create NODE
 *
 * STRING xref  [in] xref
 * STRING tag   [in] tag
 * STRING val:  [in] value
 * NODE prnt:   [in] parent
 *=========================*/
NODE
create_node (STRING xref, STRING tag, STRING val, NODE prnt)
{
	NODE node = alloc_node();
	memset(node, 0, sizeof(*node));
	nxref(node) = fixup(xref);
	ntag(node) = fixtag(tag);
	nval(node) = fixup(val);
	nparent(node) = prnt;
	if (prnt)
		node->n_cel = prnt->n_cel;
	return node;
}
/*===========================
 * create_temp_node -- Create NODE for temporary use
 *  (not to be connected to a record)
 * [All arguments are duplicated, so caller doesn't have to]
 * STRING xref  [in] xref
 * STRING tag   [in] tag
 * STRING val:  [in] value
 * NODE prnt:   [in] parent
 * Created: 2003-02-01 (Perry Rapp)
 *=========================*/
NODE
create_temp_node (STRING xref, STRING tag, STRING val, NODE prnt)
{
	NODE node = create_node(xref, tag, val, prnt);
	nflag(node) = ND_TEMP;
	return node;
}
/*===========================
 * free_temp_node_tree -- Free a node created by create_temp_node
 * Created: 2003-02-01 (Perry Rapp)
 *=========================*/
void
free_temp_node_tree (NODE node)
{
	NODE n2;
	if ((n2 = nchild(node))) {
		free_temp_node_tree(n2);
		nchild(node) = 0;
	}
	if ((n2 = nsibling(node))) {
		free_temp_node_tree(n2);
		nsibling(node) = 0;
	}
	free_node(node);
}
/*===================================
 * is_temp_node -- Return whether node is a temp
 * Created: 2003-02-04 (Perry Rapp)
 *=================================*/
BOOLEAN
is_temp_node (NODE node)
{
	return !!(nflag(node) & ND_TEMP);
}
/*===================================
 * set_temp_node -- make node temp (or not)
 * Created: 2003-02-04 (Perry Rapp)
 *=================================*/
void
set_temp_node (NODE node, BOOLEAN temp)
{
	if (is_temp_node(node) ^ temp)
		nflag(node) ^= ND_TEMP;
}
/*=====================================
 * free_nodes -- Free all NODEs in tree
 *===================================*/
void
free_nodes (NODE node)
{
	NODE sib;
	while (node) {
		if (nchild(node)) free_nodes(nchild(node));
		sib = nsibling(node);
		free_node(node);
		node = sib;
	}
}
/*==============================================================
 * tree_strlen -- Compute string length of tree -- don't count 0
 *============================================================*/
INT
tree_strlen (INT levl,       /* level */
             NODE node)      /* root */
{
	INT len = 0;
	while (node) {
		len += node_strlen(levl, node);
		if (nchild(node))
			len += tree_strlen(levl + 1, nchild(node));
		node = nsibling(node);
	}
	return len;
}
/*================================================================
 * node_strlen -- Compute NODE string length -- count \n but not 0
 *==============================================================*/
static INT
node_strlen (INT levl,       /* level */
             NODE node)      /* node */
{
	INT len;
	char scratch[10];
	sprintf(scratch, "%ld", levl);
	len = strlen(scratch) + 1;
	if (nxref(node)) len += strlen(nxref(node)) + 1;
	len += strlen(ntag(node));
	if (nval(node)) len += strlen(nval(node)) + 1;
	return len + 1;
}
/*==========================================
 * unknown_node_to_dbase -- Store node of unknown type
 *  in database
 * Created: 2001/04/06, Perry Rapp
 *========================================*/
void
unknown_node_to_dbase (NODE node)
{
	/* skip tag validation */
	node_to_dbase(node, NULL);
}
/*==========================================
 * indi_to_dbase -- Store person in database
 *========================================*/
void
indi_to_dbase (NODE node)
{
	/*
	To start storing metadata, we need the RECORD here
	If we were using RECORD everywhere, we'd pass it in here
	We could look it up in the cache, but it might not be there
	(new records)
	(and this applies to fam_to_dbase, sour_to_dbase, etc)
	Perry, 2001/01/15
	*/
	node_to_dbase(node, "INDI");
}
/*=========================================
 * fam_to_dbase -- Store family in database
 *=======================================*/
void
fam_to_dbase (NODE node)
{
	node_to_dbase(node, "FAM");
}
/*=========================================
 * even_to_dbase -- Store event in database
 *=======================================*/
void
even_to_dbase (NODE node)
{
	node_to_dbase(node, "EVEN");
}
/*==========================================
 * sour_to_dbase -- Store source in database
 *========================================*/
void
sour_to_dbase (NODE node)
{
	node_to_dbase(node, "SOUR");
}
/*================================================
 * othr_to_dbase -- Store other record in database
 *==============================================*/
void
othr_to_dbase (NODE node)
{
	node_to_dbase(node, NULL);
}
/*===============================================
 * node_to_dbase -- Store GEDCOM tree in database
 *=============================================*/
void
node_to_dbase (NODE node,
               STRING tag)
{
	STRING str;
	ASSERT(node);
	if (tag) { ASSERT(eqstr(tag, ntag(node))); }
	str = node_to_string(node);
	ASSERT(store_record(rmvat(nxref(node)), str, strlen(str)));
	stdfree(str);
}
/*==================================================
 * indi_to_famc -- Return family-as-child for person
 *================================================*/
NODE
indi_to_famc (NODE node)
{
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), "FAMC"))) return NULL;
	return key_to_fam(rmvat(nval(node)));
}
/*========================================
 * fam_to_husb_node -- Return husband of family
 *======================================*/
NODE
fam_to_husb_node (NODE node)
{
	CNSTRING key=0;
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), "HUSB"))) return NULL;
	key = rmvat(nval(node));
	if (!key) return NULL;
	return qkey_to_indi(key);
}
/*========================================
 * fam_to_husb -- Get husband of family
 *  returns 1 if found, 0 if not found, -1 if bad pointer
 *======================================*/
INT
fam_to_husb (RECORD frec, RECORD * prec)
{
	NODE fam = nztop(frec), husb;
	CNSTRING key=0;
	*prec = NULL;
	if (!fam) return 0;
	if (!(husb = find_tag(nchild(fam), "HUSB"))) return 0;
	key = rmvat(nval(husb));
	if (!key) return -1;
	*prec = key_to_irecord(key); /* ASSERT if fail */
	return 1;
}
/*=====================================
 * fam_to_wife_node -- Return wife of family
 *===================================*/
NODE
fam_to_wife_node (NODE node)
{
	CNSTRING key=0;
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), "WIFE"))) return NULL;
	key = rmvat(nval(node));
	if (!key) return NULL;
	return qkey_to_indi(key);
}
/*========================================
 * fam_to_wife -- Return husband of family
 *  returns 1 if found, 0 if not found, -1 if bad pointer
 *======================================*/
INT
fam_to_wife (RECORD frec, RECORD * prec)
{
	NODE fam = nztop(frec), husb;
	CNSTRING key=0;
	*prec = NULL;
	if (!fam) return 0;
	if (!(husb = find_tag(nchild(fam), "WIFE"))) return 0;
	key = rmvat(nval(husb));
	if (!key) return -1;
	*prec = key_to_irecord(key); /* ASSERT if fail */
	return 1;
}
/*===============================================
 * fam_to_spouse -- Return other spouse of family
 *=============================================*/
NODE
fam_to_spouse (NODE fam, NODE indi)
{
	INT num;
	if (!fam) return NULL;
	FORHUSBS(fam, husb, num)
		if(husb != indi) return(husb);
	ENDHUSBS
	FORWIFES(fam, wife, num)
	  if(wife != indi) return(wife);
	ENDWIFES
	return NULL;
}
/*===============================================
 * next_spouse -- Return next spouse of family
 * NODE *node     [in/out] pass in nchild(fam) to start
 *                or nsibling(previous node returned from this routine) to continue
 * RECORD *spouse [out]     next spouse in family
 * returns 1 for success, -1 if next HUSB/WIFE record is invalid
 *         0 no more spouses found
 * returns addref'd record in *spouse
 *=============================================*/
int
next_spouse (NODE *node, RECORD *spouse)
{
	CNSTRING key=0;
	if (!node || !spouse) return 0;
	while (*node) {
	    if (eqstr(ntag(*node),"HUSB") || eqstr(ntag(*node),"WIFE")) {
		key = rmvat(nval(*node));
		if (!key) return -1;
		*spouse = qkey_to_irecord(key);
		if (!*spouse) return -1;
		return 1;
	    }
	    *node = nsibling(*node);
	}
	return 0;
}
/*==================================================
 * fam_to_first_chil -- Return first child of family
 *================================================*/
NODE
fam_to_first_chil (NODE node)
{
	CNSTRING key=0;
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), "CHIL"))) return NULL;
	key = rmvat(nval(node));
	return qkey_to_indi(key);
}
/*=================================================
 * fam_to_last_chil -- Return last child of family
 *===============================================*/
NODE
fam_to_last_chil (NODE node)
{
	CNSTRING key=0;
	NODE prev = NULL;
	if (!node) return NULL;
	/* find first CHIL in fam */
	if (!(node = find_tag(nchild(node), "CHIL"))) return NULL;
	/* cycle thru all remaining nodes, keeping most recent CHIL node */
	while (node) {
		if (eqstr(ntag(node),"CHIL"))
			prev = node;
		node = nsibling(node);
	}
	key = rmvat(nval(prev));
	return qkey_to_indi(key);
}
/*========================================
 * indi_to_fath -- Return father of person
 *======================================*/
NODE
indi_to_fath (NODE node)
{
	return fam_to_husb_node(indi_to_famc(node));
}
/*========================================
 * indi_to_moth -- Return mother of person
 *======================================*/
NODE
indi_to_moth (NODE node)
{
	return fam_to_wife_node(indi_to_famc(node));
}
/*==================================================
 * indi_to_prev_sib -- Return previous sib of person
 *  returns addref'd record
 *================================================*/
static RECORD
indi_to_prev_sib_impl (NODE indi)
{
	NODE fam, prev, node;
	if (!indi) return NULL;
	if (!(fam = indi_to_famc(indi))) return NULL;
	prev = NULL;
	node = CHIL(fam);
	/* loop thru all nodes following first child, keeping most recent CHIL */
	while (node) {
		if (eqstr(nxref(indi), nval(node))) {
			if (!prev) return NULL;
			return key_to_record(rmvat(nval(prev)));
		}
		if (eqstr(ntag(node),"CHIL"))
			prev = node;
		node = nsibling(node);
	}
	return NULL;
}
NODE
indi_to_prev_sib_old (NODE indi)
{
	return nztop(indi_to_prev_sib_impl(indi));
}
RECORD
indi_to_prev_sib (RECORD irec)
{
	return indi_to_prev_sib_impl(nztop(irec));
}
/*==============================================
 * indi_to_next_sib -- Return next sib of person
 *  returns addref'd record
 *============================================*/
static RECORD
indi_to_next_sib_impl (NODE indi)
{
	NODE fam, node;
	BOOLEAN found;
	if (!indi) return NULL;
	if (!(fam = indi_to_famc(indi))) return NULL;
	node = CHIL(fam);
	found = FALSE;  /* until we find indi */
	while (node) {
		if (!found) {
			if (eqstr(nxref(indi), nval(node)))
				found = TRUE;
		} else {
			if (eqstr(ntag(node),"CHIL"))
				return key_to_record(rmvat(nval(node)));
		}
		node = nsibling(node);
	}
	return NULL;
}
NODE
indi_to_next_sib_old (NODE indi)
{
	RECORD rec = indi_to_next_sib_impl(indi);
	NODE node = nztop(rec);
	release_record(rec); /* release ref from indi_to_next_sib_impl */
	return node;
}
RECORD
indi_to_next_sib (RECORD irec)
{
	return indi_to_next_sib_impl(nztop(irec));
}
/*======================================
 * indi_to_name -- Return name of person
 *====================================*/
STRING
indi_to_name (NODE node, INT len)
{
	SURCAPTYPE surcaptype = DOSURCAP;
	if (node)
		node = find_tag(nchild(node), "NAME");
	if (!node)
		return _("NO NAME");
	if (!getlloptint("UppercaseSurnames", 1))
		surcaptype = NOSURCAP;
	return manip_name(nval(node), surcaptype, REGORDER, len);
}
/*======================================
 * indi_to_title -- Return title of person
 *====================================*/
STRING
indi_to_title (NODE node, INT len)
{
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), "TITL"))) return NULL;
	return manip_name(nval(node), NOSURCAP, REGORDER, len);
}
/*======================================
 * node_to_tag -- Return a subtag of a node
 * (presumably top level, but not necessarily)
 *====================================*/
STRING node_to_tag (NODE node, STRING tag, INT len)
{
	static char scratch[MAXGEDNAMELEN+1];
	STRING refn;
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), tag)))
		return NULL;
	refn = nval(node);
	if (len > (INT)sizeof(scratch)-1)
		len = sizeof(scratch)-1;
	llstrsets(scratch, len, uu8, refn);
	return scratch;
}
/*==============================================
 * fam_to_event -- Convert event tree to string
 *============================================*/
STRING
fam_to_event (NODE node, STRING tag, STRING head
	, INT len, RFMT rfmt)
{
	return indi_to_event(node, tag, head, len, rfmt);
}
/*==============================================
 * record_to_date_place -- Find event info
 *  record:  [IN]  record to search
 *  tag:     [IN]  desired tag (eg, "BIRT")
 *  date:    [OUT] date found (optional)
 *  plac:    [OUT] place found (option)
 * Created: 2003-01-12 (Perry Rapp)
 *============================================*/
void
record_to_date_place (RECORD record, STRING tag, STRING * date, STRING * plac)
{
	NODE node;
	for (node = record_to_first_event(record, tag)
		; node
		; node = node_to_next_event(node, tag)) {
		event_to_date_place(node, date, plac);
		if (date && *date && plac && *plac)
			return;
	}
}
/*==============================================
 * record_to_first_event -- Find requested event subtree
 *  record:  [IN]  record to search
 *  tag:     [IN]  desired tag (eg, "BIRT")
 * Created: 2003-01-12 (Perry Rapp)
 *============================================*/
NODE
record_to_first_event (RECORD record, CNSTRING tag)
{
	NODE node = nztop(record);
	if (!node) return NULL;
	return find_tag(nchild(node), tag);
}
/*==============================================
 * node_to_next_event -- Find next event after node
 *  node:  [IN]  start search after node
 *  tag:   [IN]  desired tag (eg, "BIRT")
 * Created: 2003-01-12 (Perry Rapp)
 *============================================*/
NODE
node_to_next_event (NODE node, CNSTRING tag)
{
	return find_tag(nsibling(node), tag);
}
/*==============================================
 * indi_to_event -- Convert event tree to string
 *  node: [IN]  event subtree to search
 *  ttm:  [IN]  translation table to apply to event strings
 *  tag:  [IN]  desired tag (eg, "BIRT")
 *  head: [IN]  header to print in output (eg, "born: ")
 *  len:  [IN]  max length output desired
 * Searches node substree for desired tag
 *  returns formatted string (event_to_string) if found,
 *  else NULL
 *============================================*/
STRING
indi_to_event (NODE node, STRING tag, STRING head
	, INT len, RFMT rfmt)
{
	static char scratch[200];
	STRING p = scratch;
	INT mylen = sizeof(scratch)/sizeof(scratch[0]);
	STRING event;
	STRING omit;
	if (mylen > len+1) mylen = len+1; /* incl. trailing 0 */
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), tag))) return NULL;
	event = event_to_string(node, rfmt);
	if (!event) return NULL;
	/* need at least room for head + 1 character + "..." or no point */
	if ((INT)strlen(head)+4>len) return NULL;
	p[0] = 0;
	/* TODO: break out following code into a subroutine for disp_person_birth to call */
	llstrcatn(&p, head, &mylen);
	if (mylen<(INT)strlen(event)+1) {
		omit = getlloptstr("ShortOmitString", NULL);
		if (omit) {
			mylen -= strlen(omit)+1; /* plus trailing 0 */
			llstrcatn(&p, event, &mylen);
			mylen += strlen(omit)+1;
			llstrcatn(&p, omit, &mylen);
		} else {
			llstrcatn(&p, event, &mylen);
		}
	} else {
		llstrcatn(&p, event, &mylen);
		if (mylen && p[-1]!='.')
		llstrcatn(&p, ".", &mylen);
	}
	return scratch;
}
/*===========================================
 * event_to_date_place  -- Find date & place
 *  node:  [IN]  node tree of event to describe
 *  date:  [OUT] value of first DATE line (optional)
 *  plac:  [OUT] value of first PLACE line (optional)
 *=========================================*/
void
event_to_date_place (NODE node, STRING * date, STRING * plac)
{
	INT count=0;
	if (date) {
		*date = NULL;
	} else {
		++count;
	}
	if (plac) {
		*plac = NULL;
	} else {
		++count;
	}
	if (!node) return;
	node = nchild(node);
	while (node && count<2) {
		if (eqstr("DATE", ntag(node)) && date && !*date) {
			*date = nval(node);
			++count;
		}
		if (eqstr("PLAC", ntag(node)) && plac && !*plac) {
			*plac = nval(node);
			++count;
		}
		node = nsibling(node);
	}
}
/*===========================================
 * event_to_string -- Convert event to string
 * Finds DATE & PLACE nodes, and prints a string
 * representation of them.
 *  node:  [IN]  node tree of event to describe
 *  ttm:   [IN]  translation table to use
 *  rfmt:  [IN]  reformatting info (may be NULL)
 *=========================================*/
STRING
event_to_string (NODE node, RFMT rfmt)
{
	static char scratch1[MAXLINELEN+1];
	STRING date, plac;
	event_to_date_place(node, &date, &plac);
	if (!date && !plac) return NULL;
	/* Apply optional, caller-specified date & place reformatting */
	if (rfmt && date && rfmt->rfmt_date)
		date = (*rfmt->rfmt_date)(date);
	if (rfmt && plac && rfmt->rfmt_plac)
		plac = (*rfmt->rfmt_plac)(plac);
	if (rfmt && rfmt->combopic && date && date[0] && plac && plac[0]) {
		sprintpic2(scratch1, sizeof(scratch1), uu8, rfmt->combopic, date, plac);
	} else if (date && date[0]) {
		llstrncpy(scratch1, date, sizeof(scratch1), uu8);
	} else if (plac && plac[0]) {
		llstrncpy(scratch1, plac, sizeof(scratch1), uu8);
	} else {
		return NULL;
	}
	return scratch1;
}
/*=======================================
 * event_to_date -- Convert event to date
 *  node: [IN]  event node
 *  ttm:  [IN]  translation table to apply
 *  shrt: [IN]  flag - use short form if set
 *=====================================*/
STRING
event_to_date (NODE node, BOOLEAN shrt)
{
	static char scratch[MAXLINELEN+1];
	if (!node) return NULL;
	if (!(node = DATE(node))) return NULL;
	llstrsets(scratch, sizeof(scratch),uu8, nval(node));
	if (shrt) return shorten_date(scratch);
	return scratch;
}
/*========================================
 * event_to_plac -- Convert event to place
 *======================================*/
STRING
event_to_plac (NODE node, BOOLEAN shrt)
{
	if (!node) return NULL;
	node = PLAC(node);
	if (!node) return NULL;
	if (shrt) return shorten_plac(nval(node));
	return nval(node);
}
/*================================
 * show_node -- Show tree -- DEBUG
 *==============================*/
void
show_node (NODE node)
{
	if (!node) llwprintf("(NIL)");
	show_node_rec(0, node);
}
/*================================================
 * show_node_rec -- Recursive version of show_node
 *==============================================*/
void
show_node_rec (INT levl,
               NODE node)
{
	INT i;
	if (!node) return;
	for (i = 1;  i < levl;  i++)
		llwprintf("  ");
	llwprintf("%d", levl);
	if (nxref(node)) llwprintf(" %s", nxref(node));
	llwprintf(" %s", ntag(node));
	if (nval(node)) llwprintf(" %s", nval(node));
	llwprintf("\n");
	show_node_rec(levl + 1, nchild(node));
	show_node_rec(levl    , nsibling(node));
}
/*===========================================
 * length_nodes -- Return length of NODE list
 *=========================================*/
INT
length_nodes (NODE node)
{
	INT len = 0;
	while (node) {
		len++;
		node = nsibling(node);
	}
	return len;
}
/*=================================================
 * shorten_plac -- Return short form of place value
 * Returns modified input string, or value from placabbr table
 *===============================================*/
STRING
shorten_plac (STRING plac)
{
	STRING plac0 = plac, comma, val;
	if (!plac) return NULL;
	comma = (STRING) strrchr(plac, ',');
	if (comma) plac = comma + 1;
	while (*plac++ == ' ')
		;
	plac--;
	if (*plac == 0) return plac0;
	if ((val = valueof_str(placabbvs, plac))) return val;
	return plac;
}

#ifdef UNUSED_CODE
/*============================================
 * all_digits -- Check if string is all digits
 * UNUSED CODE
 *==========================================*/
static BOOLEAN
all_digits (STRING s)
{
	INT c;
	while ((c = *s++)) {
		if (c < '0' || c > '9') return FALSE;
	}
	return TRUE;
}
#endif /* UNUSED_CODE */
/*=======================
 * copy_node -- Copy node
 *=====================*/
NODE
copy_node (NODE node)
{
	return create_node(nxref(node), ntag(node), nval(node), NULL);
}
/*========================
 * copy_node_subtree -- Copy tree
 *======================*/
NODE
copy_node_subtree (NODE node)
{
	return copy_nodes(node, TRUE, FALSE);
}
/*========================
 * copy_nodes -- Copy tree
 *======================*/
NODE
copy_nodes (NODE node, BOOLEAN kids, BOOLEAN sibs)
{
	NODE newn, kin;
	if (!node) return NULL;
	newn = copy_node(node);
	if (kids && nchild(node)) {
		kin = copy_nodes(nchild(node), TRUE, TRUE);
		ASSERT(kin);
		nchild(newn) = kin;
		while (kin) {
			nparent(kin) = newn;
			kin = nsibling(kin);
		}
	}
	if (sibs && nsibling(node)) {
		kin = copy_nodes(nsibling(node), kids, TRUE);
		ASSERT(kin);
		nsibling(newn) = kin;
	}
	return newn;
}
/*===============================================================
 * traverse_nodes -- Traverse nodes in tree while doing something
 * NODE node:    root of tree to traverse
 * func:         function to call at each node (returns FALSE to stop traversal)
 * param:        opaque pointer for client use, passed thru to callback
 *=============================================================*/
BOOLEAN
traverse_nodes (NODE node, BOOLEAN (*func)(NODE, VPTR), VPTR param)
{
	while (node) {
		if (!(*func)(node, param)) return FALSE;
		if (nchild(node)) {
			if (!traverse_nodes(nchild(node), func, param))
				return FALSE;
		}
		node = nsibling(node);
	}
	return TRUE;
}
/*==================================================
 * begin_node_it -- Being a node iteration
 *================================================*/
void
begin_node_it (NODE node, NODE_ITER nodeit)
{
	nodeit->start = node;
	/* first node to return is node */
	nodeit->next = node;
}
/*==================================================
 * find_next -- Find next node in an ongoing iteration
 *================================================*/
static NODE
find_next (NODE_ITER nodeit)
{
	NODE curr = nodeit->next;
	/* goto child if there is one */
	NODE next = nchild(curr);
	if (next)
		return next;
	/* otherwise try for sibling, going up to ancestors until
	we find one, or we hit the start & give up */
	while (1) {
		if (next == nodeit->start)
			return NULL;
		if (nsibling(curr))
			return nsibling(curr);
		curr = nparent(curr);
		if (!curr)
			return NULL;
	}

}
/*==================================================
 * next_node_it_ptr -- Return next node in an ongoing iteration
 *================================================*/
NODE
next_node_it_ptr (NODE_ITER nodeit)
{
	NODE current = nodeit->next;
	if (current)
		nodeit->next = find_next(nodeit);
	return current;
}
/*==================================================
 * num_spouses_of_indi -- Returns number of spouses of person
 *================================================*/
INT
num_spouses_of_indi (NODE indi)
{
	INT nsp;
	if (!indi) return 0;
	FORSPOUSES(indi, spouse, fam, nsp) ENDSPOUSES
	return nsp;  /* don't include self*/
}
/*===================================================
 * find_node -- Find node with specific tag and value
 *
 * NODE prnt:   [in] parent node
 * STRING tag:  [in] tag, may be NULL
 * STRING val:  [in] value, may be NULL
 * NODE *plast: [out] previous node, may be NULL
 *=================================================*/
NODE
find_node (NODE prnt, STRING tag, STRING val, NODE *plast)
{
	NODE last, node;

	if (plast) *plast = NULL;
	if (!prnt || (!tag && !val)) return NULL;
	for (last = NULL, node = nchild(prnt); node;
	     last = node, node = nsibling(node)) {
		if (tag && nestr(tag, ntag(node))) continue;
		if (val && nestr(val, nval(node))) continue;
		if (plast) *plast = last;
		return node;
	}
	return NULL;
}

#ifdef UNUSED_CODE
/*=======================================================================
 * father_nodes -- Given list of FAMS or FAMC nodes, returns list of HUSB
 *   lines they contain
 * UNUSED CODE
 *=====================================================================*/
NODE
father_nodes (NODE faml)      /* list of FAMC and/or FAMS nodes */
{
	NODE fam, refn, husb, wife, chil, rest;
	NODE old = NULL, new = NULL;
	while (faml) {
		ASSERT(eqstr("FAMC", ntag(faml)) || eqstr("FAMS", ntag(faml)));
		ASSERT(fam = key_to_fam(rmvat(nval(faml))));
		split_fam(fam, &refn, &husb, &wife, &chil, &rest);
		new = union_nodes(old, husb, FALSE, TRUE);
		free_nodes(old);
		old = new;
		join_fam(fam, refn, husb, wife, chil, rest);
		faml = nsibling(faml);
	}
	return new;
}
/*=======================================================================
 * mother_nodes -- Given list of FAMS or FAMC nodes, returns list of WIFE
 *   lines they contain
 *=====================================================================*/
NODE
mother_nodes (NODE faml)      /* list of FAMC and/or FAMS nodes */
{
	NODE fam, refn, husb, wife, chil, rest;
	NODE old = NULL, new = NULL;
	while (faml) {
		ASSERT(eqstr("FAMC", ntag(faml)) || eqstr("FAMS", ntag(faml)));
		ASSERT(fam = key_to_fam(rmvat(nval(faml))));
		split_fam(fam, &refn, &husb, &wife, &chil, &rest);
		new = union_nodes(old, wife, FALSE, TRUE);
		free_nodes(old);
		old = new;
		join_fam(fam, refn, husb, wife, chil, rest);
		faml = nsibling(faml);
	}
	return new;
}
/*=========================================================================
 * children_nodes -- Given list of FAMS or FAMC nodes, returns list of CHIL
 *   lines they contain
 *=======================================================================*/
NODE
children_nodes (NODE faml)      /* list of FAMC and/or FAMS nodes */
{
	NODE fam, refn, husb, wife, chil, rest;
	NODE old = NULL, new = NULL;
	while (faml) {
		ASSERT(eqstr("FAMC", ntag(faml)) || eqstr("FAMS", ntag(faml)));
		ASSERT(fam = key_to_fam(rmvat(nval(faml))));
		split_fam(fam, &refn, &husb, &wife, &chil, &rest);
		new = union_nodes(old, chil, FALSE, TRUE);
		free_nodes(old);
		old = new;
		join_fam(fam, refn, husb, wife, chil, rest);
		faml = nsibling(faml);
	}
	return new;
}
/*========================================================================
 * parents_nodes -- Given list of FAMS or FAMC nodes, returns list of HUSB
 *   and WIFE lines they contain
 *======================================================================*/
NODE
parents_nodes (NODE faml)      /* list of FAMC and/or FAMS nodes */
{
	NODE fam, refn, husb, wife, chil, rest;
	NODE old = NULL, new = NULL;
	while (faml) {
		ASSERT(eqstr("FAMC", ntag(faml)) || eqstr("FAMS", ntag(faml)));
		ASSERT(fam = key_to_fam(rmvat(nval(faml))));
		split_fam(fam, &refn, &husb, &wife, &chil, &rest);
		new = union_nodes(old, husb, FALSE, TRUE);
		free_nodes(old);
		old = new;
		new = union_nodes(old, wife, FALSE, TRUE);
		free_nodes(old);
		old = new;
		join_fam(fam, refn, husb, wife, chil, rest);
		faml = nsibling(faml);
	}
	return new;
}
#endif /* UNUSED_CODE */
/*=================================================
 * node_destructor -- destructor for node
 *  (destructor entry in vtable)
 *===============================================*/
static void
node_destructor (VTABLE *obj)
{
	NODE node = (NODE)obj;
	ASSERT((*obj) == &vtable_for_node);
	free_node(node);
}
/*=================================================
 * check_node_leaks -- Called when database closing
 *  for debugging
 *===============================================*/
void
check_node_leaks (void)
{
	if (live_count) {
		STRING report_leak_path = getlloptstr("ReportLeakLog", NULL);
		FILE * fp=0;
		if (report_leak_path)
			fp = fopen(report_leak_path, LLAPPENDTEXT);
		if (fp) {
			LLDATE date;
			get_current_lldate(&date);
			fprintf(fp, _("Node memory leaks:"));
			fprintf(fp, " %s", date.datestr);
			fprintf(fp, "\n  ");
			fprintf(fp, _pl("%d item leaked", "%d items leaked", live_count), live_count);
			fprintf(fp, "\n");
			fclose(fp);
			fp = 0;
		}
	}
}
