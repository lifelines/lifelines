#ifndef _LIFLINES_H
#define _LIFLINES_H

#include "standard.h"
#include "gedcom.h"
#include "indiseq.h"

/* Function Prototypes */
/* add.c */
void add_child_to_fam(NODE child, NODE fam, INT i);
NODE add_family_to_db(NODE spouse1, NODE spouse2, NODE child);
void add_spouse_to_fam(NODE spouse, NODE fam, INT sex);
RECORD add_unlinked_indi(RECORD indi0);
INT ask_child_order(NODE fam, PROMPTQ promptq, RFMT rfmt);

/* from ask.c */
NODE ask_for_fam(STRING, STRING);
NODE ask_for_fam_by_key(STRING fttl, STRING pttl, STRING sttl);
FILE *ask_for_input_file (STRING mode, STRING ttl, STRING *pfname, STRING path, STRING ext);
FILE *ask_for_output_file (STRING mode, STRING ttl, STRING *pfname, STRING path, STRING ext);
STRING ask_for_indi_key(STRING, CONFIRMQ, ASK1Q);
INDISEQ ask_for_indi_list(STRING, BOOLEAN);
INT ask_for_int(STRING);
RECORD choose_from_indiseq(INDISEQ, ASK1Q ask1, STRING titl1, STRING titln);

/* screen.c */
INT choose_from_array(STRING, INT, STRING*);
INT choose_list_from_indiseq(STRING, INDISEQ);
INT choose_one_from_indiseq(STRING, INDISEQ);
void view_array(STRING ttl, INT no, STRING *pstrngs);

/* from askprogram.c */
FILE *ask_for_program(STRING, STRING, STRING*, STRING, STRING, BOOLEAN picklist);

#endif /* _LIFLINES_H */
