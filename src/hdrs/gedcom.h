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
 * gedcom.h -- Main header file of LifeLines system
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 02 Sep 93
 *   3.0.0 - 23 Sep 94    3.0.2 - 09 Dec 94
 *   3.0.3 - 17 Jan 96
 *===========================================================*/

#ifndef _GEDCOM_H
#define _GEDCOM_H

#define MAXNAMELEN 512

#define OKAY  1
#define ERROR 0
#define DONE -1

/*=====================================
 * NODE -- Internal form of GEDCOM line
 *===================================*/
typedef struct ntag *NODE, NODE_struct;
struct ntag {
	STRING n_xref;		/* cross ref */
	STRING n_tag;		/* tag */
	STRING n_val;		/* value */
	NODE   n_parent;	/* parent */
	NODE   n_child;		/* first child */
	NODE   n_sibling;	/* sibling */
};
#define nxref(n)    ((n)->n_xref)
#define ntag(n)     ((n)->n_tag)
#define nval(n)     ((n)->n_val)
#define nparent(n)  ((n)->n_parent)
#define nchild(n)   ((n)->n_child)
#define nsibling(n) ((n)->n_sibling)

#define SEX_MALE    1
#define SEX_FEMALE  2
#define SEX_UNKNOWN 3

#define BROWSE_INDI 1
#define BROWSE_FAM  2
#define BROWSE_PED  3
#define BROWSE_TAND 4
#define BROWSE_QUIT 5
#define BROWSE_2FAM 6
#define BROWSE_LIST 7

#define MEDIN 0
#define MINED 1
#define MGDIN 2
#define MINGD 3
#define MINDS 4
#define MINRP 5

extern INT lineno;
extern INT tlineno;
extern BOOLEAN inited;
extern BOOLEAN keyflag;
extern BOOLEAN readonly;
extern BOOLEAN cursesio;
extern STRING editstr;
extern STRING editfile;
extern STRING llreports;
extern TABLE tagtable;		/* table for GEDCOM tags */
extern TABLE placabbvs;		/* table for place abbrvs */
extern TABLE useropts;		/* table for user options */

/* function definitions */
STRING addat(STRING);
NODE choose_child(NODE, NODE, STRING, STRING, BOOLEAN);
NODE choose_father(NODE, NODE, STRING, STRING, BOOLEAN);
NODE choose_family(NODE, STRING, STRING, BOOLEAN);
NODE choose_mother(NODE, NODE, STRING, STRING, BOOLEAN);
NODE choose_spouse(NODE, STRING, STRING);
NODE copy_node(NODE);
NODE copy_nodes(NODE, BOOLEAN, BOOLEAN);
NODE create_node(STRING, STRING, STRING, NODE);
void even_to_cache(NODE);
void even_to_dbase(NODE);
STRING event_to_date(NODE, TRANTABLE, BOOLEAN);
STRING event_to_plac(NODE, BOOLEAN);
STRING event_to_string(NODE, TRANTABLE, BOOLEAN);
NODE fam_to_first_chil(NODE);
NODE fam_to_last_chil(NODE);
NODE fam_to_husb(NODE);
NODE fam_to_wife(NODE);
NODE fam_to_spouse(NODE, NODE);
NODE file_to_node(STRING, TRANTABLE, STRING*, BOOLEAN*);
NODE find_node(NODE, STRING, STRING, NODE*);
NODE find_tag(NODE, STRING);
NODE first_fp_to_node(FILE*, BOOLEAN, TRANTABLE, STRING*, BOOLEAN*);
STRING full_value(NODE);
STRING *get_child_strings(NODE, INT*, STRING**);
STRING *get_names(STRING, INT*, STRING**, BOOLEAN);
INT getfinitial(STRING);
STRING getexref(void);
STRING getfxref(void);
STRING getixref(void);
STRING getsxref(void);
STRING getxxref(void);
STRING getsurname(STRING);
STRING indi_to_event(NODE, TRANTABLE, STRING, STRING, INT, BOOLEAN);
NODE indi_to_famc(NODE);
NODE indi_to_fath(NODE);
STRING indi_to_list_string(NODE, NODE, INT);
NODE indi_to_moth(NODE);
STRING indi_to_name(NODE, TRANTABLE, INT);
NODE indi_to_next_sib(NODE);
NODE indi_to_prev_sib(NODE);
NODE key_to_even(STRING);
NODE key_to_fam(STRING);
NODE key_to_indi(STRING);
NODE key_to_othr(STRING);
NODE key_to_record(STRING,INT);
NODE key_to_sour(STRING);
STRING manip_name(STRING, TRANTABLE, BOOLEAN, BOOLEAN, INT);
STRING name_string(STRING);
STRING name_surfirst(STRING);
NODE next_fp_to_node(FILE*, BOOLEAN, TRANTABLE, STRING*, BOOLEAN*);
void node_to_dbase(NODE, STRING);
BOOLEAN node_to_file(INT, NODE, STRING, BOOLEAN, TRANTABLE);
NODE node_to_node(NODE, INT*);
STRING node_to_string(NODE);
INT num_spouses(NODE);
void othr_to_cache(NODE);
void othr_to_dbase(NODE);
NODE refn_to_record(STRING, INT);
BOOLEAN replace_indi(NODE, NODE, STRING*);
BOOLEAN retrieve_file(STRING, STRING);
STRING retrieve_record(STRING, INT*);
STRING rmvat(STRING);
STRING shorten_date(STRING);
STRING shorten_plac(STRING);
STRING soundex(STRING);
void sour_to_cache(NODE);
void sour_to_dbase(NODE);
BOOLEAN store_file(STRING, STRING);
BOOLEAN store_record(STRING, STRING, INT);
NODE string_to_node(STRING);
BOOLEAN traverse_nodes(NODE, BOOLEAN(*func)());
STRING trim_name(STRING, INT);
NODE union_nodes(NODE, NODE, BOOLEAN, BOOLEAN);
NODE unique_nodes(NODE, BOOLEAN);
BOOLEAN valid_indi(NODE, STRING*, NODE);
BOOLEAN valid_fam(NODE, STRING*, NODE);
INT val_to_sex(NODE);
STRING value_to_xref(STRING);

#define fam_to_event indi_to_event

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

#define indi_to_key(indi)  (rmvat(nxref(indi)))
#define fam_to_key(fam)    (rmvat(nxref(fam)))
#define num_families(indi) (length_nodes(FAMS(indi)))
#define num_children(fam)  (length_nodes(CHIL(fam)))

#define FORCHILDREN(fam,child,num) \
	{\
	NODE __node = find_tag(nchild(fam), "CHIL");\
	NODE child;\
	num = 0;\
	while (__node) {\
		if(child = key_to_indi(rmvat(nval(__node)))) {\
		  ASSERT(child);\
		  num++;\
		  {

#define ENDCHILDREN \
		  }}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "CHIL")) __node = NULL;\
	}}

#define FORSPOUSES(indi,spouse,fam,num) \
	{\
	NODE __node = FAMS(indi);\
	INT __sex = SEX(indi);\
	NODE spouse;\
	NODE fam;\
	num = 0;\
	while (__node) {\
		fam = key_to_fam(rmvat(nval(__node)));\
		ASSERT(fam);\
		if (__sex == SEX_MALE)\
			spouse = fam_to_wife(fam);\
		else if (__sex == SEX_FEMALE)\
			spouse = fam_to_husb(fam);\
		else    spouse = fam_to_spouse(fam, indi);\
		if (spouse != NULL) {\
			num++;\
		{

#define ENDSPOUSES \
		}}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "FAMS")) __node = NULL;\
	}}

#define FORFAMSS(indi,fam,spouse,num) \
	{\
	NODE __node = FAMS(indi);\
	INT __sex = SEX(indi);\
	NODE fam, spouse;\
	num = 0;\
	while (__node) {\
		fam = key_to_fam(rmvat(nval(__node)));\
		ASSERT(fam);\
		if (__sex == SEX_MALE)\
			spouse = fam_to_wife(fam);\
		else if (__sex == SEX_FEMALE)\
			spouse = fam_to_husb(fam);\
		else    spouse = fam_to_spouse(fam, indi);\
		num++;\
		{

#define ENDFAMSS \
		}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "FAMS")) __node = NULL;\
	}}

#define FORFAMCS(indi,fam,fath,moth,num) \
	{\
	NODE __node = FAMC(indi);\
	NODE fam, fath, moth;\
	num = 0;\
	while (__node) {\
		fam = key_to_fam(rmvat(nval(__node)));\
		ASSERT(fam);\
		fath = fam_to_husb(fam);\
		moth = fam_to_wife(fam);\
		num++;\
		{

#define ENDFAMCS \
		}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "FAMC")) __node = NULL;\
	}}

#define FORHUSBS(fam,husb,num) \
	{\
	NODE __node = find_tag(nchild(fam), "HUSB");\
	NODE husb;\
	num = 0;\
	while (__node) {\
		husb = key_to_indi(rmvat(nval(__node)));\
		ASSERT(husb);\
		num++;\
		{

#define ENDHUSBS \
		}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "HUSB")) __node = NULL;\
	}}

#define FORWIFES(fam,wife,num) \
	{\
	NODE __node = find_tag(nchild(fam), "WIFE");\
	NODE wife;\
	num = 0;\
	while (__node) {\
		wife = key_to_indi(rmvat(nval(__node)));\
		ASSERT(wife);\
		num++;\
		{

#define ENDWIFES \
		}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "WIFE")) __node = NULL;\
	}}

#define FORTAGVALUES(root,tag,node,value)\
	{\
	NODE node, __node = nchild(root);\
	STRING value, __value;\
	while (__node) {\
		while (__node && ll_strcmp(tag, ntag(__node)))\
			__node = nsibling(__node);\
		if (__node == NULL) break;\
		__value = value = full_value(__node);/*OBLIGATION*/\
		node = __node;\
		{
#define ENDTAGVALUES \
		}\
		if (__value) stdfree(__value);/*RELEASE*/\
		 __node = nsibling(__node);\
	}}

#endif /* _GEDCOM_H */
