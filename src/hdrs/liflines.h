
#ifndef _STANDARD_H
#include "standard.h"
#endif

#ifndef _GEDCOM_H
#include "gedcom.h"
#endif

#ifndef _INDISEQ_H
#include "indiseq.h"
#endif

/* Function Prototypes */

INT ask_for_char(STRING, STRING, STRING);
INT ask_for_char_msg(STRING, STRING, STRING, STRING);
STRING ask_for_indi_key(STRING, BOOLEAN, BOOLEAN);
INT ask_for_int(STRING);
NODE ask_for_fam(STRING, STRING);
FILE *ask_for_file(STRING, STRING, STRING*, STRING, STRING);
NODE ask_for_indi(STRING, BOOLEAN, BOOLEAN);
INDISEQ ask_for_indiseq(STRING, INT*);
STRING ask_for_indi_key(STRING, BOOLEAN, BOOLEAN);
INDISEQ ask_for_indi_list(STRING, BOOLEAN);
INDISEQ ask_for_indi_list_once(STRING, INT*);
NODE ask_for_indi_once(STRING, BOOLEAN, INT*);
STRING ask_for_string(STRING, STRING);
BOOLEAN ask_yes_or_no(STRING);
BOOLEAN ask_yes_or_no_msg(STRING, STRING);

NODE add_child(NODE, NODE);
NODE add_family(NODE, NODE, NODE);
BOOLEAN add_linked_indi(NODE);
BOOLEAN add_spouse(NODE, NODE, BOOLEAN);
NODE add_unlinked_indi(NODE);
NODE add_indi_by_edit(void);

BOOLEAN remove_child(NODE, NODE, BOOLEAN);
BOOLEAN remove_spouse(NODE, NODE, BOOLEAN);
INT num_fam_xrefs(NODE);

void delete_indi(NODE, BOOLEAN);
void delete_fam(NODE);

NODE edit_family(NODE);
NODE edit_indi(NODE);

NODE format_and_choose_indi(INDISEQ, BOOLEAN, BOOLEAN, BOOLEAN, STRING, STRING);
INT choose_from_list(STRING, INT, STRING*);
INDISEQ choose_list_from_indiseq(STRING, INDISEQ);
INT choose_one_from_indiseq(STRING, INDISEQ);

void browse(NODE);
INT browse_indi(NODE*, NODE*, NODE*, NODE*, INDISEQ*);
INT browse_fam(NODE*, NODE*, NODE*, NODE*, INDISEQ*);
INT browse_pedigree(NODE*, NODE*, NODE*, NODE*, INDISEQ*);
INT browse_tandem(NODE*, NODE*, NODE*, NODE*, INDISEQ*);
INT browse_2fam(NODE*, NODE*, NODE*, NODE*, INDISEQ*);

NODE sort_children(NODE, NODE);
NODE remove_dupes(NODE, NODE);
NODE merge_two_indis(NODE, NODE, BOOLEAN);
NODE merge_two_fams(NODE, NODE);

void message(STRING);
void llwprintf(STRING, ...);
void mprintf(STRING, ...);
void final_cleanup(void);
void do_edit(void);

BOOLEAN pointer_value(STRING);
