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

#include "table.h"
#include "translat.h"

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
void addixref(INT);
void addexref(INT);
void addfxref(INT);
void addsxref(INT);
void addxxref(INT);
void add_child_to_fam(NODE child, NODE fam, INT i);
void add_spouse_to_fam(NODE spouse, NODE fam, INT sex);
BOOLEAN add_name(STRING, STRING);
BOOLEAN add_refn(STRING, STRING);
NODE add_unlinked_indi (NODE indi);
BOOLEAN allwhite(STRING);
enum {ALWAYS_PROMPT, PROMPT_IF_CHILDREN};
INT ask_child_order(NODE fam, int prompt);
void cache_stats(void);
NODE choose_child(NODE, NODE, STRING, STRING, BOOLEAN);
NODE choose_father(NODE, NODE, STRING, STRING, BOOLEAN);
NODE choose_family(NODE, STRING, STRING, BOOLEAN);
NODE choose_mother(NODE, NODE, STRING, STRING, BOOLEAN);
NODE choose_spouse(NODE, STRING, STRING);
void classify_nodes(NODE*, NODE*, NODE*);
void closexref(void);
void close_lifelines(void);
NODE copy_node(NODE);
NODE copy_nodes(NODE, BOOLEAN, BOOLEAN);
NODE create_node(STRING, STRING, STRING, NODE);
BOOLEAN edit_mapping(INT);
BOOLEAN edit_valtab(STRING, TABLE*, INT, STRING);
BOOLEAN equal_tree(NODE, NODE);
BOOLEAN equal_node(NODE, NODE);
BOOLEAN equal_nodes(NODE, NODE, BOOLEAN, BOOLEAN);
void even_to_cache(NODE);
void even_to_dbase(NODE);
STRING event_to_date(NODE, TRANTABLE, BOOLEAN);
STRING event_to_plac(NODE, BOOLEAN);
STRING event_to_string(NODE, TRANTABLE, BOOLEAN);
void fam_to_cache(NODE);
void fam_to_dbase(NODE);
NODE fam_to_first_chil(NODE);
NODE fam_to_last_chil(NODE);
NODE fam_to_husb(NODE);
NODE fam_to_wife(NODE);
NODE fam_to_spouse(NODE, NODE);
NODE file_to_node(STRING, TRANTABLE, STRING*, BOOLEAN*);
INT file_to_line(FILE*, TRANTABLE, INT*, STRING*, STRING*, STRING*, STRING*);
NODE find_node(NODE, STRING, STRING, NODE*);
NODE find_tag(NODE, STRING);
NODE first_fp_to_node(FILE*, BOOLEAN, TRANTABLE, STRING*, BOOLEAN*);
void free_node(NODE);
void free_nodes(NODE);
STRING full_value(NODE);
STRING *get_child_strings(NODE, INT*, STRING**);
INT get_decimal(STRING);
INT get_hexidecimal(STRING);
STRING *get_names(STRING, INT*, STRING**, BOOLEAN);
INT getfinitial(STRING);
STRING getexref(void);
STRING getfxref(void);
STRING getixref(void);
STRING getsxref(void);
STRING getxxref(void);
STRING getsurname(STRING);
void get_refns(STRING, INT*, STRING**, INT);
BOOLEAN getrefnrec(STRING);
STRING givens(STRING);
void growexrefs(void);
void growfxrefs(void);
void growixrefs(void);
void growsxrefs(void);
void growxxrefs(void);
INT hexvalue(INT);
void index_by_refn(NODE, STRING);
void indi_to_cache(NODE);
void indi_to_dbase(NODE);
STRING indi_to_event(NODE, TRANTABLE, STRING, STRING, INT, BOOLEAN);
NODE indi_to_famc(NODE);
NODE indi_to_fath(NODE);
STRING indi_to_list_string(NODE, NODE, INT);
NODE indi_to_moth(NODE);
STRING indi_to_name(NODE, TRANTABLE, INT);
NODE indi_to_next_sib(NODE);
NODE indi_to_prev_sib(NODE);
STRING indi_to_title(NODE, TRANTABLE, INT);
void initxref(void);
void init_browse_lists(void);
void init_caches(void);
void init_lifelines(void);
void init_mapping(void);
BOOLEAN init_valtab_from_file(STRING, TABLE, INT, STRING*);
BOOLEAN init_valtab_from_rec(STRING, TABLE, INT, STRING*);
BOOLEAN init_valtab_from_string(STRING, TABLE, INT, STRING*);
BOOLEAN in_string(INT, STRING);
BOOLEAN iso_list(NODE, NODE);
BOOLEAN iso_nodes(NODE, NODE, BOOLEAN, BOOLEAN);
void join_fam(NODE, NODE, NODE, NODE, NODE, NODE);
void join_indi(NODE, NODE, NODE, NODE, NODE, NODE, NODE);
STRING key_of_record(NODE);
NODE key_to_even(STRING);
NODE key_to_fam(STRING);
NODE key_to_indi(STRING);
NODE key_to_othr(STRING);
NODE key_to_record(STRING,INT);
NODE key_to_sour(STRING);
NODE key_to_type (STRING key, STRING type, INT reportmode);
NODE keynum_to_fam(int keynum);
NODE keynum_to_indi(int keynum);
NODE keynum_to_sour(int keynum);
INT length_nodes(NODE);
STRING manip_name(STRING, TRANTABLE, BOOLEAN, BOOLEAN, INT);
void maperror(INT, INT, STRING);
int namecmp(STRING, STRING);
STRING name_string(STRING);
STRING name_surfirst(STRING);
BOOLEAN name_to_list(STRING, LIST, INT*, INT*);
STRING newexref(STRING, BOOLEAN);
STRING newfxref(STRING, BOOLEAN);
STRING newixref(STRING, BOOLEAN);
STRING newsxref(STRING, BOOLEAN);
STRING newxxref(STRING, BOOLEAN);
void new_name_browse_list(STRING, STRING);
NODE next_fp_to_node(FILE*, BOOLEAN, TRANTABLE, STRING*, BOOLEAN*);
INT nod0_to_keynum(NODE nod, char ntype);
INT node_strlen(INT, NODE);
void node_to_dbase(NODE, STRING);
BOOLEAN node_to_file(INT, NODE, STRING, BOOLEAN, TRANTABLE);
NODE node_to_node(NODE, INT*);
STRING node_to_string(NODE);
INT num_evens(void);
INT num_fams(void);
INT num_indis(void);
INT num_spouses(NODE);
INT num_sours(void);
INT num_othrs(void);
BOOLEAN openxref(void);
void othr_to_cache(NODE);
void othr_to_dbase(NODE);
BOOLEAN piecematch(STRING, STRING);
BOOLEAN place_to_list(STRING, LIST, INT*);
BOOLEAN readxrefs(void);
INT record_letter(STRING);
NODE refn_to_record(STRING, INT);
void remove_indi_cache(STRING);
void remove_fam_cache(STRING);
void remove_from_browse_lists(STRING);
BOOLEAN remove_name(STRING, STRING);
BOOLEAN remove_refn(STRING, STRING);
void rename_from_browse_lists(STRING);
BOOLEAN replace_fam(NODE, NODE, STRING*);
BOOLEAN replace_indi(NODE, NODE, STRING*);
void resolve_links(NODE);
BOOLEAN resolve_traverse(NODE);
BOOLEAN retrieve_file(STRING, STRING);
STRING retrieve_record(STRING, INT*);
NODE rkey_to_even(STRING);
NODE rkey_to_fam(STRING);
NODE rkey_to_indi(STRING);
NODE rkey_to_othr(STRING);
NODE rkey_to_sour(STRING);
NODE rkeynum_to_fam(int keynum);
NODE rkeynum_to_indi(int keynum);
STRING rmvat(STRING);
STRING shorten_date(STRING);
STRING shorten_plac(STRING);
void show_node_rec(INT, NODE);
STRING soundex(STRING);
void sour_to_cache(NODE);
void sour_to_dbase(NODE);
void split_fam(NODE, NODE*, NODE*, NODE*, NODE*, NODE*);
void split_indi(NODE, NODE*, NODE*, NODE*, NODE*, NODE*, NODE*);
BOOLEAN store_file(STRING, STRING);
BOOLEAN store_record(STRING, STRING, INT);
NODE string_to_node(STRING);
void striptrail(STRING);
BOOLEAN symbolic_link(STRING);
BOOLEAN traverse_nodes(NODE, BOOLEAN(*func)(NODE));
INT tree_strlen(INT, NODE);
STRING trim_name(STRING, INT);
NODE union_nodes(NODE, NODE, BOOLEAN, BOOLEAN);
NODE unique_nodes(NODE, BOOLEAN);
BOOLEAN valid_indi(NODE, STRING*, NODE);
BOOLEAN valid_fam(NODE, STRING*, NODE);
BOOLEAN valid_name(STRING);
BOOLEAN valid_sour_tree(NODE, STRING*, NODE);
BOOLEAN valid_to_list(STRING, LIST, INT*, STRING);
BOOLEAN valid_even_tree(NODE, STRING*, NODE);
BOOLEAN valid_othr_tree(NODE, STRING*, NODE);
INT val_to_sex(NODE);
BOOLEAN value_to_list(STRING, LIST, INT*, STRING);
STRING value_to_xref(STRING);
BOOLEAN writexrefs(void);
void write_nodes(INT, FILE*, TRANTABLE, NODE, BOOLEAN, BOOLEAN, BOOLEAN);
INT xref_nexti(INT);
INT xref_previ(INT);
INT xref_nextf(INT);
INT xref_prevf(INT);
INT xref_nexts(INT);
INT xref_nexte(INT);
INT xref_nextx(INT);
INT xrefval(STRING str, char ntype);

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
#define indi_to_keynum(indi) (nod0_to_keynum(indi, 'I'))
#define fam_to_keynum(fam)  (nod0_to_keynum(fam, 'F'))
#define sour_to_keynum(sour) (nod0_to_keynum(sour, 'S'))
#define num_families(indi) (length_nodes(FAMS(indi)))
#define num_children(fam)  (length_nodes(CHIL(fam)))

#define FORCHILDREN(fam,child,num) \
	{\
	NODE __node = find_tag(nchild(fam), "CHIL");\
	NODE child;\
	num = 0;\
	while (__node) {\
		if((child = key_to_indi(rmvat(nval(__node))))) {\
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
