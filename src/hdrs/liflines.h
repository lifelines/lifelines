#ifndef _LIFLINES_H
#define _LIFLINES_H

#include "standard.h"
#include "gedcom.h"
#include "indiseq.h"

/* Function Prototypes */

/* from ask.c */
typedef INT CONFIRMQ;
#define DOCONFIRM 1
#define NOCONFIRM 0

NODE ask_for_fam(STRING, STRING);
FILE *ask_for_input_file (STRING mode, STRING ttl, STRING *pfname, STRING path, STRING ext);
FILE *ask_for_output_file (STRING mode, STRING ttl, STRING *pfname, STRING path, STRING ext);
STRING ask_for_indi_key(STRING, CONFIRMQ, BOOLEAN);
INDISEQ ask_for_indi_list(STRING, BOOLEAN);
INT ask_for_int(STRING);
BOOLEAN ask_yes_or_no_msg(STRING, STRING);

INT choose_from_list(STRING, INT, STRING*);
INDISEQ choose_list_from_indiseq(STRING, INDISEQ);
INT choose_one_from_indiseq(STRING, INDISEQ);
NODE choose_from_indiseq(INDISEQ, BOOLEAN, STRING, STRING);

/* from askprogram.c */
FILE *ask_for_program(STRING, STRING, STRING*, STRING, STRING, BOOLEAN picklist);

/* from valgdcom.c */
BOOLEAN pointer_value(STRING);

#endif /* _LIFLINES_H */
