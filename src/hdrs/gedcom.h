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
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 02 Sep 93
 *   3.0.0 - 23 Sep 94    3.0.2 - 09 Dec 94
 *   3.0.3 - 17 Jan 96
 *===========================================================*/

#ifndef _GEDCOM_H
#define _GEDCOM_H

#include "table.h"
#include "translat.h"

#define NAMESEP '/'
#define MAXGEDNAMELEN 512
/* keys run up to 9,999,999, eg I9999999 */
#define MAXKEYWIDTH 8

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

struct nkeytag { char ntype; INT keynum; STRING key; };
typedef struct nkeytag NKEY;

typedef struct ntag0 *NOD0;
struct ntag0 {
	NODE top;
	NKEY nkey;
	WAREHOUSE mdwh; /* metadata */
};
NODE nztop(NOD0); /* function so it can handle NULL input */
#define nzkey(n)    ((n)->nkey.key)
#define nzkeynum(n) ((n)->nkey.keynum)
#define nztype(n)   ((n)->nkey.ntype)

/* reformating functions - format context provided by client */
struct rfmt_s {
	STRING (*rfmt_date)(STRING); /* returns static buffer */
};
typedef struct rfmt_s *RFMT;

/*==============================================
 * Option type enumerations (but we use defines)
 *============================================*/

/* ask to ensure user got to see the indi */
typedef INT CONFIRMQ;
#define DOCONFIRM 1
#define NOCONFIRM 0

/* whether to prompt for new child if none existing */
typedef INT PROMPTQ;
#define ALWAYS_PROMPT 0
#define PROMPT_IF_CHILDREN 1

/* ask if only one match */
typedef INT ASK1Q;
#define DOASK1 1
#define NOASK1 0

#define SEX_MALE    1
#define SEX_FEMALE  2
#define SEX_UNKNOWN 3

enum {
	BROWSE_INDI
	, BROWSE_FAM
	, BROWSE_PED
	, BROWSE_TAND
	, BROWSE_QUIT
	, BROWSE_2FAM
	, BROWSE_LIST
	, BROWSE_AUX
	, BROWSE_EVEN
	, BROWSE_SOUR
	, BROWSE_UNK
};

#define MEDIN 0
#define MINED 1
#define MGDIN 2
#define MINGD 3
#define MDSIN 4
#define MINDS 5
#define MINRP 6
#define MSORT 7
#define MCHAR 8
#define MLCAS 9
#define MUCAS 10
#define MPREF 11

/*========
 * Globals
 *======*/

extern INT flineno;
extern INT travlineno;
extern BOOLEAN inited;
extern BOOLEAN keyflag;
extern BOOLEAN readonly;
extern BOOLEAN cursesio;
extern STRING editstr;
extern STRING editfile;
extern TABLE tagtable;		/* table for GEDCOM tags */
extern TABLE placabbvs;		/* table for place abbrvs */
extern TABLE useropts;		/* table for user options */
extern BOOLEAN add_metadata;
extern INT int_codeset; /* numeric coding of internal code set, ref get_codset_desc */



/*=====================
 * Function definitions
 *===================*/

STRING addat(STRING);
void addixref(INT);
void addexref(INT);
void addfxref(INT);
void addsxref(INT);
void addxxref(INT);
void assign_nod0(NOD0 nod0, char ntype, INT keynum);
BOOLEAN add_name(STRING, STRING);
BOOLEAN add_refn(STRING, STRING);
STRING get_cache_stats(void);
STRING get_property(STRING opt);
NODE choose_child(NODE, NODE, STRING, STRING, ASK1Q);
void choose_and_delete_family(void);
NODE choose_father(NODE, NODE, STRING, STRING, ASK1Q);
NODE choose_family(NODE, STRING, STRING, BOOLEAN);
NODE choose_mother(NODE, NODE, STRING, STRING, ASK1Q);
NODE choose_note(NODE what, STRING msg0, STRING msgn);
NODE choose_pointer(NODE what, STRING msg0, STRING msgn);
NODE choose_source(NODE what, STRING msg0, STRING msgn);
NODE choose_spouse(NODE, STRING, STRING);
void classify_nodes(NODE*, NODE*, NODE*);
void closexref(void);
void close_lifelines(void);
void close_lldb(void);
NODE copy_node(NODE);
NODE copy_nodes(NODE, BOOLEAN, BOOLEAN);
NOD0 create_nod0(NODE node);
NODE create_node(STRING, STRING, STRING, NODE);
void del_in_dbase (STRING key);
void delete_metarec(STRING key);
BOOLEAN edit_mapping(INT);
BOOLEAN edit_valtab(STRING, TABLE*, INT, STRING);
BOOLEAN equal_tree(NODE, NODE);
BOOLEAN equal_node(NODE, NODE);
BOOLEAN equal_nodes(NODE, NODE, BOOLEAN, BOOLEAN);
void even_to_cache(NODE);
void even_to_dbase(NODE);
STRING event_to_date(NODE, TRANTABLE, BOOLEAN);
STRING event_to_plac(NODE, BOOLEAN);
STRING event_to_string(NODE, TRANTABLE, BOOLEAN shrt, RFMT rfmt);
void fam_to_cache(NODE);
void fam_to_dbase(NODE);
NODE fam_to_first_chil(NODE);
NODE fam_to_last_chil(NODE);
NODE fam_to_husb(NODE);
NODE fam_to_wife(NODE);
NODE fam_to_spouse(NODE, NODE);
NOD0 file_to_nod0(STRING fname, TRANTABLE tt, STRING *pmsg, BOOLEAN *pemp);
NODE file_to_node(STRING, TRANTABLE, STRING*, BOOLEAN*);
INT file_to_line(FILE*, TRANTABLE, INT*, STRING*, STRING*, STRING*, STRING*);
NODE find_node(NODE, STRING, STRING, NODE*);
NODE find_tag(NODE, STRING);
NODE first_fp_to_node(FILE*, BOOLEAN, TRANTABLE, STRING*, BOOLEAN*);
void free_name_list (LIST list);
void free_nod0(NOD0);
void free_node(NODE);
void free_nodes(NODE);
STRING full_value(NODE);
STRING generic_to_list_string(NODE node, STRING key, INT len, STRING delim, RFMT rfmt);
STRING *get_child_strings(NODE, RFMT, INT*, STRING**);
INT get_decimal(STRING);
INT get_hexidecimal(STRING);
STRING get_lifelines_version(INT maxlen);
STRING *get_names(STRING, INT*, STRING**, BOOLEAN);
void get_refns(STRING, INT*, STRING**, INT);
STRING getasurname(STRING);
STRING getexref(void);
INT getfinitial(STRING);
STRING getfxref(void);
INT getixrefnum(void);
STRING getsxref(void);
STRING getsxsurname(STRING);
STRING getxxref(void);
BOOLEAN getrefnrec(STRING);
STRING givens(STRING);
void growexrefs(void);
void growfxrefs(void);
void growixrefs(void);
void growsxrefs(void);
void growxxrefs(void);
INT hexvalue(INT);
void index_by_refn(NODE, STRING);
void indi0_to_cache(NOD0 nod0);
void indi_to_cache(NODE);
void indi_to_dbase(NODE);
STRING indi_to_event(NODE, TRANTABLE, STRING, STRING, INT, BOOLEAN, RFMT);
NODE indi_to_famc(NODE);
NODE indi_to_fath(NODE);
STRING indi_to_list_string(NODE, NODE, INT, RFMT);
NODE indi_to_moth(NODE);
STRING indi_to_name(NODE, TRANTABLE, INT);
NODE indi_to_next_sib(NODE);
STRING indi_to_ped_fix(NODE indi, INT len);
NODE indi_to_prev_sib(NODE);
STRING indi_to_title(NODE, TRANTABLE, INT);
void initxref(void);
void init_browse_lists(void);
void init_caches(void);
void init_lifelines_db(void);
BOOLEAN init_lifelines_global(STRING * pmsg);
void init_mapping(void);
void init_new_nod0(NOD0 nod0, char ntype, INT keynum);
void init_show_module(void);
BOOLEAN init_valtab_from_file(STRING, TABLE, TRANTABLE, INT, STRING*);
BOOLEAN init_valtab_from_rec(STRING, TABLE, INT, STRING*);
BOOLEAN in_string(INT, STRING);
BOOLEAN iso_list(NODE, NODE);
BOOLEAN iso_nodes(NODE, NODE, BOOLEAN, BOOLEAN);
void join_fam(NODE, NODE, NODE, NODE, NODE, NODE);
void join_indi(NODE, NODE, NODE, NODE, NODE, NODE, NODE);
STRING key_of_record(NODE);
NODE key_to_even(STRING);
NOD0 key_to_even0(STRING);
NODE key_to_fam(STRING);
NOD0 key_to_fam0(STRING);
NODE key_to_indi(STRING);
NOD0 key_to_indi0(STRING);
NODE key_to_othr(STRING);
NOD0 key_to_othr0(STRING);
NOD0 key_to_record(STRING,INT);
NODE key_to_sour(STRING);
NOD0 key_to_sour0(STRING);
NODE key_to_type(STRING key, INT reportmode);
NOD0 key_to_typ0(STRING key, INT reportmode);
NODE keynum_to_fam(int keynum);
NODE keynum_to_indi(int keynum);
NODE keynum_to_node(char ntype, int keynum);
NODE keynum_to_sour(int keynum);
NODE keynum_to_even(int keynum);
NODE keynum_to_othr(int keynum);
INT length_nodes(NODE);
STRING manip_name(STRING, TRANTABLE, BOOLEAN, BOOLEAN, INT);
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
NOD0 next_fp_to_nod0(FILE*, BOOLEAN, TRANTABLE, STRING*, BOOLEAN*);
NODE next_fp_to_node(FILE*, BOOLEAN, TRANTABLE, STRING*, BOOLEAN*);
void nkey_copy(NKEY * src, NKEY * dest);
BOOLEAN nkey_eq(NKEY * nkey1, NKEY * nkey2);
void nkey_clear(NKEY * nkey);
void nkey_load_key(NKEY * nkey);
BOOLEAN nkey_to_node(NKEY * nkey, NODE * node);
NKEY nkey_zero(void);
void node_to_dbase(NODE, STRING);
BOOLEAN node_to_file(INT, NODE, STRING, BOOLEAN, TRANTABLE);
INT node_to_keynum(char ntype, NODE nod);
BOOLEAN node_to_nkey(NODE node, NKEY * nkey);
NODE node_to_node(NODE, INT*);
STRING node_to_string(NODE);
STRING node_to_tag (NODE node, STRING tag, TRANTABLE tt, INT len);
INT num_evens(void);
INT num_fams(void);
INT num_indis(void);
INT num_spouses_of_indi(NODE);
INT num_sours(void);
INT num_othrs(void);
BOOLEAN openxref(void);
STRING other_to_list_string(NODE node, INT len, STRING delim);
void othr_to_cache(NODE);
void othr_to_dbase(NODE);
BOOLEAN piecematch(STRING, STRING);
BOOLEAN place_to_list(STRING, LIST, INT*);
BOOLEAN pointer_value(STRING);
NODE qkey_to_even(STRING);
NOD0 qkey_to_even0(STRING);
NODE qkey_to_fam(STRING);
NOD0 qkey_to_fam0(STRING);
NODE qkey_to_indi(STRING);
NOD0 qkey_to_indi0(STRING);
NODE qkey_to_othr(STRING);
NOD0 qkey_to_othr0(STRING);
NODE qkey_to_sour(STRING);
NOD0 qkey_to_sour0(STRING);
NODE qkey_to_type(STRING key);
NODE qkeynum_to_fam(int keynum);
NODE qkeynum_to_indi(int keynum);
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
RECORD_STATUS retrieve_to_file(STRING key, STRING file);
RECORD_STATUS retrieve_to_textfile(STRING key, STRING file, TRANSLFNC);
STRING retrieve_record(STRING, INT*);
STRING rmvat(STRING);
STRING rmvbrackets (STRING str);
void rptlocale(void);
void set_displaykeys(BOOLEAN);
STRING shorten_date(STRING);
STRING shorten_plac(STRING);
void show_node(NODE node);
void show_node_rec(INT, NODE);
STRING soundex(STRING);
void sour_to_cache(NODE);
void sour_to_dbase(NODE);
STRING sour_to_list_string(NODE sour, INT len, STRING delim);
void split_fam(NODE, NODE*, NODE*, NODE*, NODE*, NODE*);
void split_indi(NODE, NODE*, NODE*, NODE*, NODE*, NODE*, NODE*);
BOOLEAN store_file_to_db(STRING key, STRING file);
BOOLEAN store_record(STRING key, STRING rec, INT len);
BOOLEAN store_text_file_to_db(STRING key, STRING file, TRANSLFNC);
NOD0 string_to_nod0(STRING str, STRING key, INT len);
NODE string_to_node(STRING);
BOOLEAN symbolic_link(STRING);
void traverse_db_key_nod0s (BOOLEAN(*func)(STRING key, NOD0, void *param), void *param);
void traverse_db_rec_keys(STRING lo, STRING hi, BOOLEAN(*func)(STRING key, STRING, INT, void *param), void * param);
void traverse_names(BOOLEAN(*func)(STRING key, STRING name, BOOLEAN newset, void *param), void *param);
BOOLEAN traverse_nodes(NODE node, BOOLEAN (*func)(NODE, VPTR), VPTR param);
void traverse_refns(BOOLEAN(*func)(STRING key, STRING refn, BOOLEAN newset, void *param), void *param);
INT tree_strlen(INT, NODE);
STRING trim_name(STRING, INT);
void uilocale(void);
NODE union_nodes(NODE, NODE, BOOLEAN, BOOLEAN);
NODE unique_nodes(NODE, BOOLEAN);
void unknown_node_to_dbase(NODE node);
void update_useropts(void);
BOOLEAN valid_indi(NODE, STRING*, NODE);
BOOLEAN valid_fam(NODE, STRING*, NODE);
BOOLEAN valid_name(STRING);
BOOLEAN valid_node_type(NODE node, char ntype, STRING *pmsg, NODE node0);
BOOLEAN valid_sour_tree(NODE, STRING*, NODE);
BOOLEAN valid_to_list(STRING, LIST, INT*, STRING);
BOOLEAN valid_even_tree(NODE, STRING*, NODE);
BOOLEAN valid_othr_tree(NODE, STRING*, NODE);
INT val_to_sex(NODE);
BOOLEAN value_to_list(STRING, LIST, INT*, STRING);
STRING value_to_xref(STRING);
BOOLEAN writexrefs(void);
void write_indi_to_editfile(NODE indi);
void write_fam_to_editfile(NODE fam);
void write_node_to_editfile(NODE); /* used by Ethel */
void write_nodes(INT, FILE*, TRANTABLE, NODE, BOOLEAN, BOOLEAN, BOOLEAN);
INT xref_firste(void);
INT xref_firstf(void);
INT xref_firsti(void);
INT xref_firsts(void);
INT xref_firstx(void);
INT xref_laste(void);
INT xref_lastf(void);
INT xref_lasti(void);
INT xref_lasts(void);
INT xref_lastx(void);
INT xref_next(char ntype, INT i);
INT xref_nexte(INT);
INT xref_nextf(INT);
INT xref_nexti(INT);
INT xref_nexts(INT);
INT xref_nextx(INT);
INT xref_prev(char ntype, INT i);
INT xref_preve(INT);
INT xref_prevf(INT);
INT xref_previ(INT);
INT xref_prevs(INT);
INT xref_prevx(INT);
INT xrefval(char ntype, STRING str);

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
		while (__node && nestr(tag, ntag(__node)))\
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
