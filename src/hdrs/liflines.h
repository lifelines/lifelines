#ifndef _LIFLINES_H
#define _LIFLINES_H

/*
 UI functions required for doing work with records
 TODO: ARRAYDETAILS probably doesn't belong here (seems curses dependent)
 TODO: rearrange functions 
    -- (they are currently arranged by curses implementation file
	  and they ought to be arranged in some way not related to curses impl.)
*/


#include "standard.h"
#include "gedcom.h"

#ifndef _INDISEQ_H
#include "indiseq.h"
#endif

#ifndef INCLUDED_UIPROMPTS_H
#include "uiprompts.h"
#endif

/* Types */

/* screen.c types */
/* data used in choose_from_array_x */
typedef struct tag_array_details {
  STRING * list; /* original array of choices */
  INT cur; /* currently selected choice */
  STRING * lines; /* lines of details */
  INT count; /* how many lines */
  INT maxlen; /* size of each line */
  INT scroll; /* scroll offset in details */
} *ARRAY_DETAILS;
typedef void (*DETAILFNC)(ARRAY_DETAILS, void *);

/* Function Prototypes */
/* add.c */
void add_child_to_fam(NODE child, NODE fam, INT i);
NODE add_family_to_db(NODE spouse1, NODE spouse2, NODE child);
void add_spouse_to_fam(NODE spouse, NODE fam, INT sex);
INT ask_child_order(NODE fam, PROMPTQ promptq, RFMT rfmt);
STRING ask_for_indi_key(STRING, ASK1Q ask1);
RECORD ask_for_indi(STRING ttl, ASK1Q ask1);

/* ask.c */
RECORD ask_for_fam(STRING, STRING);
RECORD ask_for_fam_by_key(STRING fttl, STRING pttl, STRING sttl);
FILE *ask_for_input_file (STRING mode, STRING ttl, STRING *pfname, STRING *pfullpath, STRING path, STRING ext);
FILE *ask_for_output_file (STRING mode, STRING ttl, STRING *pfname, STRING *pfullpath, STRING path, STRING ext);
INDISEQ ask_for_indi_list(STRING, BOOLEAN);
BOOLEAN ask_for_int(STRING, INT *);
RECORD ask_for_record(STRING, INT);
STRING ask_for_record_key(STRING title, STRING prompt);
RECORD choose_from_indiseq(INDISEQ, ASK1Q ask1, STRING titl1, STRING titln);

/* askgedc.c */
BOOLEAN ask_for_gedcom(STRING mode, STRING ttl, STRING *pfname, STRING *pfullpath
	, STRING path, STRING ext, BOOLEAN picklist);

/* askprogram.c */
BOOLEAN ask_for_program(STRING mode, STRING ttl, STRING *pfname, STRING *pfullpath
	, STRING path, STRING ext, BOOLEAN picklist);
void proparrdetails(ARRAY_DETAILS arrdets, void * param);

/* screen.c functions */
INT choose_from_array(STRING ttl, INT no, STRING *pstrngs);
INT choose_from_list(STRING ttl, LIST list);
INT choose_from_array_x(STRING ttl, INT count, STRING* list, DETAILFNC, void *);
INT choose_list_from_indiseq(STRING, INDISEQ);
INT choose_one_from_indiseq(STRING, INDISEQ);
INT display_list(STRING ttl, LIST list);
INT prompt_stdout(STRING prompt);
void view_array(STRING ttl, INT no, STRING *pstrngs);

/* selectdb.c */
BOOLEAN open_or_create_database(INT alteration, STRING *dbused);
BOOLEAN select_database(STRING dbrequested, INT alteration, STRING * perrmsg);


#endif /* _LIFLINES_H */
