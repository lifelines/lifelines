/* 
   Copyright (c) 1991-2005 Thomas T. Wetmore IV

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
 * gedcom_macros.h -- Macros used by or implemented by gedcom module
 *===========================================================*/

#ifndef GEDCOM_MACROS_H_INCLUDED
#define GEDCOM_MACROS_H_INCLUDED

/*******************
 various Macros
 *******************/


#define NAME(indi)  find_tag(nchild(indi),"NAME")
#define REFN(indi)  find_tag(nchild(indi),"REFN")
#define SEX(indi)   val_to_sex(find_tag(nchild(indi),"SEX"))
#define BIRT(indi)  find_tag(nchild(indi),"BIRT")
#define DEAT(indi)  find_tag(nchild(indi),"DEAT")
#define BAPT(indi)  find_tag(nchild(indi),"CHR")
#define BURI(indi)  find_tag(nchild(indi),"BURI")
#define FAMC(indi)  find_tag(nchild(indi),"FAMC")
#define FAMS(indi)  find_tag(nchild(indi),"FAMS")

#define HUSB(fam)   find_tag(nchild(fam),"HUSB")
#define WIFE(fam)   find_tag(nchild(fam),"WIFE")
#define MARR(fam)   find_tag(nchild(fam),"MARR")
#define CHIL(fam)   find_tag(nchild(fam),"CHIL")

#define DATE(evnt)   find_tag(nchild(evnt),"DATE")
#define PLAC(evnt)   find_tag(nchild(evnt),"PLAC")

/*=============================================
 * indi_to_key, fam_to_key - return key of node
 *  eg, "I21"
 *  returns static buffer
 *===========================================*/
#define indi_to_key(indi)  (rmvat(nxref(indi)))
#define fam_to_key(fam)    (rmvat(nxref(fam)))
#define node_to_key(node)  (rmvat(nxref(node)))

/*=============================================
 * indi_to_keynum, fam_to_keynum, etc - 
 *  return keynum of node, eg, 21
 *===========================================*/
#define indi_to_keynum(indi) (node_to_keynum('I', indi))
#define fam_to_keynum(fam)  (node_to_keynum('F', fam))
#define sour_to_keynum(sour) (node_to_keynum('S', sour))

/*=============================================
 * num_families - count spouses of indi
 *===========================================*/
#define num_families(indi) (length_nodes(FAMS(indi)))

 /*=============================================
 * num_children - count children of indi
 *===========================================*/
#define num_children(fam)  (length_nodes(CHIL(fam)))

/*
  Possibly all of these should lock their main argument in the cache
  Instead of having some of the client code do it, and some not
  Now that NODE has a pointer to its parent RECORD,
  and RECORD has a pointer to the cacheel, it is possible 
*/
/*
 FORCHILDRENx takes a node as its first arg
 FORCHILDREN takes a record as its first arg
*/

#define FORCHILDRENx(fam,child,num) \
	{\
	NODE __node = find_tag(nchild(fam), "CHIL");\
	RECORD irec=0;\
	NODE child=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		if (!eqstr(ntag(__node), "CHIL")) break;\
		__key = rmvat(nval(__node));\
		__node = nsibling(__node);\
		++num;\
		if (!__key || !(irec=qkey_to_irecord(__key)) || !(child=nztop(irec))) {\
			continue;\
		}\
		{

#define ENDCHILDRENx \
		}\
		release_record(irec);\
	}}

#define FORCHILDREN(fam,child,num) \
	{\
	NODE __node = find_tag(nchild(fam), "CHIL");\
	RECORD child=0, irec=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		if (!eqstr(ntag(__node), "CHIL")) break;\
		__key = rmvat(nval(__node));\
		__node = nsibling(__node);\
		++num;\
		if (!__key || !(child = key_to_irecord(__key))) {\
			continue;\
		}\
		irec=child;\
		{

#define ENDCHILDREN \
		}\
		release_record(irec);\
	}}

/* FORSPOUSES iterate over all FAMS nodes & all spouses of indi
 * if there are multiple spouses, user will see same fam with multiple spouses
 * if there are no spouses for a particular family the family is not returned.
 * the counter, num, is only incremented for each spouse returned
 *     i.e. it doesn't count indi as a spouse.
 */
#define FORSPOUSES(indi,spouse,fam,num) \
	{\
	NODE __node = find_tag(nchild(indi),"FAMS");\
	NODE __node1=0, fam=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		if (!eqstr(ntag(__node), "FAMS")) break;\
	    __key = rmvat(nval(__node));\
	    __node = nsibling(__node);\
	    if (!__key || !(fam = qkey_to_fam(__key))) {\
			continue;\
		}\
		__node1 = nchild(fam);\
		/* Now loop through fam node tree looking for spouses */\
		while (__node1) {\
			NODE spouse=0;\
			INT __hits=0;\
			if (eqstr(ntag(__node1), "HUSB")||eqstr(ntag(__node1), "WIFE")) ++__hits;\
			else if (__hits)\
				/* Its not HUSB or WIFE, and we've seen a HUSB or WIFE before */ \
				/* So we must be out of the HUSB & WIFE section of the node tree */ \
				break;\
			__key = rmvat(nval(__node1));\
			__node1 = nsibling(__node1);\
			if (!__hits || !__key || !(spouse = qkey_to_indi(__key))||spouse==indi){\
				continue;\
			}\
			/* It is a valid HUSB or WIFE, not self, has key, and in database */\
			++num;\
			{

#define ENDSPOUSES \
			}\
	    }\
	}}

/* FORFAMS iterate over all FAMS nodes of indi
 *    this is an optimization of FORFAMSS for cases where spouse is not used
 *    or computed in other ways.
 */
#define FORFAMS(indi,fam,num) \
	{\
	RECORD frec=0; \
	NODE __node = find_tag(nchild(indi),"FAMS");\
	NODE fam=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		if (!eqstr(ntag(__node), "FAMS")) break;\
		__key = rmvat(nval(__node));\
		__node = nsibling(__node);\
		++num;\
	    if (!__key || !(frec=qkey_to_frecord(__key)) || !(fam=nztop(frec))) {\
			continue;\
		}\
		{

#define ENDFAMS \
		}\
		release_record(frec); \
	}}

/* FORFAMSS iterate over all FAMS nodes & all spouses of indi
 * if there are multiple spouses, user will see same fam with multiple spouses
 * if there are no spouses for a particular family NULL is returned for spouse
 */
#define FORFAMSS(indi,fam,spouse,num) \
	{\
	INT first_sp=0; /* have reported spouse in current family? */\
	RECORD frec=0; \
	NODE __node = find_tag(nchild(indi),"FAMS");\
	NODE __node1=0, fam=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		if (!eqstr(ntag(__node), "FAMS")) break;\
	    __key = rmvat(nval(__node));\
	    __node = nsibling(__node);\
	    if (__key && (frec=qkey_to_frecord(__key)) && (fam=nztop(frec))) {\
			first_sp=0; /* not yet reported this family */\
			__node1 = nchild(fam);\
			while (__node1 || !first_sp) {\
				NODE spouse=0;\
				if (__node1) { \
				    if (eqstr("HUSB", ntag(__node1)) || eqstr("WIFE", ntag(__node1))) { \
					__key = rmvat(nval(__node1));\
					__node1 = nsibling(__node1);\
					if (!__key || !(spouse = qkey_to_indi(__key))||spouse==indi) {\
						    /* invalid spouse, or self */\
						    continue;\
					}\
				    } else {\
					    /* Not a spouse (not HUSB or WIFE) */\
					    if (first_sp)\
						    break; /* finished HUSB/WIFE section & reported already */\
						__node1 = nsibling(__node1);\
					    if (__node1) continue;\
						/* fall through here for end of family not yet reported */\
					}\
				/* } else { \
				 * !__node1 && !first_sp fall thru to report * fam*/ \
				}\
				++num; \
				++first_sp; /* reporting this family */\
				{

#define ENDFAMSS \
				}\
			}\
			release_record(frec); \
		}\
	}}

/* FORFAMCS iterate over all parent families of indi
 * Up to one father and mother are given for each
 * (This ignores non-traditional parents)
 */
#define FORFAMCS(indi,fam,fath,moth,num) \
	{\
	RECORD frec=0; \
	NODE __node = find_tag(nchild(indi),"FAMC");\
	NODE fam, fath, moth;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		if (!eqstr(ntag(__node), "FAMC")) break;\
		__key = rmvat(nval(__node));\
		 __node = nsibling(__node);\
		 ++num;\
	    if (!__key || !(frec=qkey_to_frecord(__key)) || !(fam=nztop(frec))) {\
			 continue;\
		}\
		fath = fam_to_husb_node(fam);\
		moth = fam_to_wife_node(fam);\
		{

#define ENDFAMCS \
		}\
		release_record(frec); \
	}}

/* FORHUSBS iterate over all husbands in one family
 * (This handles more than one husband in a family)
 */
#define FORHUSBS(fam,husb,num) \
	{\
	NODE __node = find_tag(nchild(fam), "HUSB");\
	NODE husb=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		__key = rmvat(nval(__node));\
		if (!__key || !(husb = key_to_indi(__key))) {\
			++num;\
			__node = nsibling(__node);\
			continue;\
		}\
		++num;\
		{

#define ENDHUSBS \
		}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "HUSB")) __node = NULL;\
	}}

/* FORWIFES iterate over all wives in one family
 * (This handles more than one wife in a family)
 */
#define FORWIFES(fam,wife,num) \
	{\
	NODE __node = find_tag(nchild(fam), "WIFE");\
	NODE wife=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		__key = rmvat(nval(__node));\
		if (!__key || !(wife = qkey_to_indi(__key))) {\
			++num;\
			__node = nsibling(__node);\
			if (__node && nestr(ntag(__node), "WIFE")) __node = NULL;\
			continue;\
		}\
		ASSERT(wife);\
		num++;\
		{

#define ENDWIFES \
		}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "WIFE")) __node = NULL;\
	}}

/* FORFAMSPOUSES iterate over all spouses in one family
 * (All husbands and wives)
 */
#define FORFAMSPOUSES(fam,spouse,num) \
	{\
	NODE __node = nchild(fam);\
	NODE spouse=0;\
	STRING __key=0;\
	num = 0;\
	while (__node) {\
		if (!eqstr(ntag(__node), "HUSB") && !eqstr(ntag(__node), "WIFE")) {\
			__node = nsibling(__node);\
			continue;\
		}\
		__key = rmvat(nval(__node));\
		if (!__key || !(spouse = qkey_to_indi(__key))) {\
			++num;\
			__node = nsibling(__node);\
			continue;\
		}\
		++num;\
		{

#define ENDFAMSPOUSES \
		}\
		__node = nsibling(__node);\
	}}

#define FORTAGVALUES(root,tag,node,value)\
	{\
	NODE node, __node = nchild(root);\
	STRING value, __value;\
	while (__node) {\
		while (__node && nestr(tag, ntag(__node)))\
			__node = nsibling(__node);\
		if (__node == NULL) break;\
		__value = value = full_value(__node, "\n");/*OBLIGATION*/\
		node = __node;\
		{
#define ENDTAGVALUES \
		}\
		if (__value) stdfree(__value);/*RELEASE*/\
		 __node = nsibling(__node);\
	}}


#endif /* GEDCOM_MACROS_H_INCLUDED */
