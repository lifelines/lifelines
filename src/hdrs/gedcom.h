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
/*
 A NODE is a struct ntag which holds the in-memory
 representation of one line of a GEDCOM record.
 For example, a NODE may hold the in-memory representation
 of the GEDCOM line "2 DATE ABT 1900". The NODE actually contains
 the "DATE" part of the string in its n_tag field, and
 the "ABT 1900" portion of the string in its n_val field.
 The level is not explicitly given, but is implicit in the
 NODEs location in the NODE tree in which it lives (it has
 fields n_parent, n_child, n_sibling which connect it into
 its NODE tree). (E.g., its parent might be a NODE representing
 "1 BIRT".)
*/
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

/*=====================================
 * RECORD -- Internal form of GEDCOM record
 *===================================*/
/*
 A RECORD is a struct nrec0 which holds the in-memory representation
 of an entire GEDCOM record, such as an INDI. It has a pointer to the
 root NODE of the INDI (which is of course a NODE representing a line
 such as "0 @I43@ INDI"), and it also contains some additional data.
 A RECORD will in the future contain a pointer to its cache element.
 LifeLines is very RECORD-oriented.
*/
typedef struct nrec0 *RECORD;
struct nrec0 {
	NODE top;
	NKEY nkey;
	WAREHOUSE mdwh; /* metadata */
};
NODE nztop(RECORD); /* function so it can handle NULL input */
#define nzkey(n)    ((n)->nkey.key)
#define nzkeynum(n) ((n)->nkey.keynum)
#define nztype(n)   ((n)->nkey.ntype)

/*
 reformating functions - format context provided by client/
 either or both may be null, meaning use date or place exactly as
 occurs in data
*/
struct rfmt_s {
	STRING (*rfmt_date)(STRING); /* returns static buffer */
	STRING (*rfmt_plac)(STRING); /* returns static buffer */
	STRING combopic; /* stdalloc'd buffer, eg, "%1, %2" */
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

/* custom translation tables */
	/* MEDIN: translate editor characters to internal */
#define MEDIN 0
	/* MINED: translate internal characters to editor */
#define MINED 1
	/* MGDIN: translate gedcom file characters to internal */
#define MGDIN 2
	/* MGDIN: translate internal characters to gedcom file */
#define MINGD 3
	/* MDSIN: translate display characters to internal */
#define MDSIN 4
	/* MINDS: translate internal characters to display */
#define MINDS 5
	/* MINRP: translate internal characters to report */
#define MINRP 6
	/* MSORT: custom sort table, characters to numeric order */
#define MSORT 7
	/* MCHAR: character table (translation result unused) */
#define MCHAR 8
	/* MLCAS: translate character to lower-case (UNIMPLEMENTED) */
#define MLCAS 9
	/* MUCAS: translate character to upper-case (UNIMPLEMENTED) */
#define MUCAS 10
	/* MPREF: prefix to skip for sorting (UNIMPLEMENTED) */
#define MPREF 11
	/* number of maps listed above */
#define NUM_TT_MAPS 12

/*========
 * Globals
 *======*/

extern INT flineno;
extern BOOLEAN inited;
extern BOOLEAN keyflag;
extern BOOLEAN readonly;
extern BOOLEAN immutable;
extern BOOLEAN cursesio;
extern STRING editstr;
extern STRING editfile;
extern TABLE tagtable;		/* table for GEDCOM tags */
extern TABLE placabbvs;		/* table for place abbrvs */
extern BOOLEAN add_metadata;



/*=====================
 * Function definitions
 *===================*/

STRING addat(STRING);
void addixref(INT);
void addexref(INT);
void addfxref(INT);
void addsxref(INT);
void addxxref(INT);
BOOLEAN add_name(STRING, STRING);
BOOLEAN add_refn(STRING, STRING);
BOOLEAN are_locales_supported(void);
RECORD choose_child(RECORD irec, RECORD frec, STRING msg0, STRING msgn, ASK1Q ask1);
void choose_and_remove_family(void);
RECORD choose_father(RECORD irec, RECORD frec, STRING msg0, STRING msgn, ASK1Q ask1);
RECORD choose_family(RECORD irec, STRING msg0, STRING msgn, BOOLEAN fams);
RECORD choose_mother(RECORD indi, RECORD fam, STRING msg0, STRING msgn, ASK1Q ask1);
RECORD choose_note(RECORD current, STRING msg0, STRING msgn);
RECORD choose_pointer(RECORD current, STRING msg0, STRING msgn);
RECORD choose_source(RECORD current, STRING msg0, STRING msgn);
RECORD choose_spouse(RECORD irec, STRING msg0, STRING msgn);
void classify_nodes(NODE*, NODE*, NODE*);
void closexref(void);
void close_lifelines(void);
void close_lldb(void);
NODE copy_node(NODE);
NODE copy_nodes(NODE, BOOLEAN, BOOLEAN);
RECORD create_record(NODE node);
NODE create_node(STRING, STRING, STRING, NODE);
void del_in_dbase(STRING key);
void delete_metarec(STRING key);
BOOLEAN edit_mapping(INT);
BOOLEAN edit_valtab_from_db(STRING, TABLE*, INT sep, STRING, STRING (*validator)(TABLE tab));
BOOLEAN equal_tree(NODE, NODE);
BOOLEAN equal_node(NODE, NODE);
BOOLEAN equal_nodes(NODE, NODE, BOOLEAN, BOOLEAN);
void even_to_cache(NODE);
void even_to_dbase(NODE);
STRING even_to_list_string(NODE even, INT len, STRING delim);
STRING event_to_date(NODE, TRANMAPPING, BOOLEAN);
STRING event_to_plac(NODE, BOOLEAN);
STRING event_to_string(NODE, TRANMAPPING, RFMT rfmt);
INT expand_refn_links(NODE node);
void fam_to_cache(NODE);
void fam_to_dbase(NODE);
STRING fam_to_event(NODE, TRANMAPPING, STRING tag, STRING head, INT len, RFMT);
NODE fam_to_first_chil(NODE);
RECORD fam_to_husb(RECORD);
NODE fam_to_husb_node(NODE);
NODE fam_to_last_chil(NODE);
STRING fam_to_list_string(NODE fam, INT len, STRING delim);
NODE fam_to_spouse(NODE, NODE);
NODE fam_to_wife(NODE);
RECORD file_to_record(STRING fname, TRANMAPPING ttm, STRING *pmsg, BOOLEAN *pemp);
NODE file_to_node(STRING, TRANMAPPING, STRING*, BOOLEAN*);
INT file_to_line(FILE*, TRANMAPPING, INT*, STRING*, STRING*, STRING*, STRING*);
NODE find_node(NODE, STRING, STRING, NODE*);
NODE find_tag(NODE, STRING);
NODE convert_first_fp_to_node(FILE*, BOOLEAN, TRANMAPPING, STRING*, BOOLEAN*);
void free_name_list(LIST list);
void free_rec(RECORD);
void free_node(NODE);
void free_nodes(NODE);
void free_string_list(LIST list);
STRING full_value(NODE);
STRING generic_to_list_string(NODE node, STRING key, INT len, STRING delim, RFMT rfmt);
STRING get_cache_stats(void);
STRING *get_child_strings(NODE, RFMT, INT*, STRING**);
STRING get_current_locale_collate(void);
STRING get_current_locale_msgs(void);
INT get_dblist(STRING path, LIST * dblist, LIST * dbdesclist);
INT get_decimal(STRING);
INT get_hexidecimal(STRING);
STRING get_lifelines_version(INT maxlen);
CNSTRING get_map_name(INT ttnum);
STRING get_original_locale_collate(void);
STRING get_original_locale_msgs(void);
void get_names(STRING name, INT *pnum, STRING **pkeys, BOOLEAN exact);
STRING get_property(STRING opt);
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
BOOLEAN in_string(INT, STRING);
void index_by_refn(NODE, STRING);
void indi_to_cache(RECORD rec);
void indi_to_cache_old(NODE);
void indi_to_dbase(NODE);
STRING indi_to_event(NODE, TRANMAPPING, STRING tag, STRING head, INT len, RFMT);
NODE indi_to_famc(NODE);
NODE indi_to_fath(NODE);
STRING indi_to_list_string(NODE, NODE, INT, RFMT);
NODE indi_to_moth(NODE);
STRING indi_to_name(NODE, TRANMAPPING, INT);
RECORD indi_to_next_sib(RECORD);
NODE indi_to_next_sib_old(NODE);
STRING indi_to_ped_fix(NODE indi, INT len);
RECORD indi_to_prev_sib(RECORD);
NODE indi_to_prev_sib_old(NODE);
STRING indi_to_title(NODE, TRANMAPPING, INT);
void initxref(void);
void init_browse_lists(void);
void init_caches(void);
BOOLEAN init_lifelines_db(void);
BOOLEAN init_lifelines_global(STRING configfile, STRING * pmsg, void (*notify)(STRING db, BOOLEAN opening));
CNSTRING init_get_config_file(void);
void init_new_record(RECORD rec, char ntype, INT keynum);
BOOLEAN init_valtab_from_file(STRING, TABLE, TRANMAPPING, INT sep, STRING*);
BOOLEAN init_valtab_from_rec(CNSTRING, TABLE, INT sep, STRING*);
BOOLEAN init_valtab_from_string(CNSTRING, TABLE, INT sep, STRING*);
BOOLEAN is_db_open(void);
BOOLEAN is_codeset_utf8(STRING codeset);
BOOLEAN is_iconv_supported(void);
BOOLEAN is_nls_supported(void);
BOOLEAN iso_list(NODE, NODE);
BOOLEAN iso_nodes(NODE, NODE, BOOLEAN, BOOLEAN);
void join_fam(NODE, NODE, NODE, NODE, NODE, NODE);
void join_indi(NODE, NODE, NODE, NODE, NODE, NODE, NODE);
void join_othr(NODE root, NODE refn, NODE rest);
STRING key_of_record(NODE, TRANMAPPING tt);
RECORD key_possible_to_record(STRING, INT let);
NODE key_to_even(STRING);
RECORD key_to_erecord(STRING);
NODE key_to_fam(STRING);
RECORD key_to_frecord(STRING);
NODE key_to_indi(STRING);
RECORD key_to_irecord(STRING);
RECORD key_to_orecord(STRING);
NODE key_to_othr(STRING);
RECORD key_to_record(STRING key);
NODE key_to_sour(STRING);
RECORD key_to_srecord(STRING);
NODE key_to_type(STRING key, INT reportmode);
NODE keynum_to_fam(int keynum);
RECORD keynum_to_erecord(int keynum);
RECORD keynum_to_frecord(int keynum);
NODE keynum_to_indi(int keynum);
RECORD keynum_to_irecord(int keynum);
NODE keynum_to_node(char ntype, int keynum);
RECORD keynum_to_orecord(int keynum);
RECORD keynum_to_record(char ntype, int keynum);
RECORD keynum_to_srecord(int keynum);
NODE keynum_to_sour(int keynum);
NODE keynum_to_even(int keynum);
NODE keynum_to_othr(int keynum);
INT length_nodes(NODE);
STRING ll_langinfo(void);
void load_char_mappings(void);
BOOLEAN load_new_tt(CNSTRING filepath, INT ttnum);
STRING manip_name(STRING, TRANMAPPING, BOOLEAN, BOOLEAN, INT);
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
RECORD next_fp_to_record(FILE*, BOOLEAN, TRANMAPPING, STRING*, BOOLEAN*);
NODE next_fp_to_node(FILE*, BOOLEAN, TRANMAPPING, STRING*, BOOLEAN*);
void nkey_copy(NKEY * src, NKEY * dest);
BOOLEAN nkey_eq(NKEY * nkey1, NKEY * nkey2);
void nkey_clear(NKEY * nkey);
void nkey_load_key(NKEY * nkey);
BOOLEAN nkey_to_node(NKEY * nkey, NODE * node);
BOOLEAN nkey_to_record(NKEY * nkey, RECORD * prec);
NKEY nkey_zero(void);
void node_to_dbase(NODE, STRING);
BOOLEAN node_to_file(INT, NODE, STRING, BOOLEAN, TRANTABLE);
INT node_to_keynum(char ntype, NODE nod);
BOOLEAN node_to_nkey(NODE node, NKEY * nkey);
NODE node_to_node(NODE, INT*);
RECORD node_to_record(NODE node);
STRING node_to_string(NODE);
STRING node_to_tag(NODE node, STRING tag, TRANMAPPING tt, INT len);
INT num_evens(void);
INT num_fam_xrefs(NODE fam);
INT num_fams(void);
INT num_indis(void);
INT num_spouses_of_indi(NODE);
INT num_sours(void);
INT num_othrs(void);
BOOLEAN openxref(BOOLEAN readonly);
STRING other_to_list_string(NODE node, INT len, STRING delim);
void othr_to_cache(NODE);
void othr_to_dbase(NODE);
BOOLEAN piecematch(STRING, STRING);
BOOLEAN place_to_list(STRING, LIST, INT*);
BOOLEAN pointer_value(STRING);
NODE qkey_to_even(STRING);
RECORD qkey_to_erecord(STRING);
NODE qkey_to_fam(STRING);
RECORD qkey_to_frecord(STRING);
NODE qkey_to_indi(STRING);
RECORD qkey_to_irecord(STRING);
RECORD qkey_to_orecord(STRING);
NODE qkey_to_othr(STRING);
RECORD qkey_to_record(STRING key);
NODE qkey_to_sour(STRING);
RECORD qkey_to_srecord(STRING);
NODE qkey_to_type(STRING key);
RECORD qkeynum_to_frecord(int keynum);
NODE qkeynum_to_indi(int keynum);
INT record_letter(STRING);
NODE refn_to_record(STRING, INT);
void release_dblist(LIST dblist);
BOOLEAN remove_child(NODE indi, NODE fam);
BOOLEAN remove_empty_fam(NODE);
void remove_indi(NODE);
void remove_indi_cache(STRING);
void remove_fam_cache(STRING);
void remove_from_browse_lists(STRING);
BOOLEAN remove_name(STRING, STRING);
BOOLEAN remove_refn(STRING, STRING);
void rename_from_browse_lists(STRING);
BOOLEAN remove_spouse(NODE indi, NODE fam);
BOOLEAN replace_fam(NODE, NODE, STRING*);
BOOLEAN replace_indi(NODE, NODE, STRING*);
INT resolve_refn_links(NODE);
RECORD_STATUS retrieve_to_file(STRING key, STRING file);
RECORD_STATUS retrieve_to_textfile(STRING key, STRING file, TRANSLFNC);
STRING retrieve_raw_record(CNSTRING, INT*);
STRING rmvat(STRING);
STRING rmvbrackets(STRING str);
void rptlocale(void);
void save_original_locales(void);
BOOLEAN save_tt_to_file(INT ttnum, STRING filename);
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
void split_indi_old(NODE, NODE*, NODE*, NODE*, NODE*, NODE*, NODE*);
void split_othr(NODE node, NODE *prefn, NODE *prest);
BOOLEAN store_file_to_db(STRING key, STRING file);
BOOLEAN store_record(STRING key, STRING rec, INT len);
BOOLEAN store_text_file_to_db(STRING key, CNSTRING file, TRANSLFNC);
RECORD string_to_record(STRING str, STRING key, INT len);
NODE string_to_node(STRING);
BOOLEAN symbolic_link(STRING);
void termlocale(void);
void traverse_db_key_recs(BOOLEAN(*func)(STRING key, RECORD, void *param), void *param);
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
BOOLEAN valid_indi_tree(NODE, STRING*, NODE);
BOOLEAN valid_fam_tree(NODE, STRING*, NODE);
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
void write_nodes(INT, FILE*, TRANMAPPING, NODE, BOOLEAN, BOOLEAN, BOOLEAN);
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
INT xref_max_any(void);
INT xref_max_indis(void);
INT xref_max_evens (void);
INT xref_max_fams (void);
INT xref_max_othrs (void);
INT xref_max_sours (void);
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
TODO: Change all FORCHILDRENx to FORCHILDREN loops
This means changing use of NODE to RECORD
Perry, 2002.06.24
*/
#define FORCHILDRENx(fam,child,num) \
	{\
	NODE __node = find_tag(nchild(fam), "CHIL");\
	NODE child;\
	num = 0;\
	while (__node) {\
		if((child = key_to_indi(rmvat(nval(__node))))) {\
		  ASSERT(child);\
		  num++;\
		  {

#define ENDCHILDRENx \
		  }}\
		__node = nsibling(__node);\
		if (__node && nestr(ntag(__node), "CHIL")) __node = NULL;\
	}}

#define FORCHILDREN(fam,child,num) \
	{\
	NODE __node = find_tag(nchild(fam), "CHIL");\
	RECORD child;\
	num = 0;\
	while (__node) {\
		if((child = key_to_irecord(rmvat(nval(__node))))) {\
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
			spouse = fam_to_husb_node(fam);\
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
			spouse = fam_to_husb_node(fam);\
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
		fath = fam_to_husb_node(fam);\
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
