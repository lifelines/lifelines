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
STRING ask_for_lldb (STRING ttl, STRING prmpt, STRING basedir);
FILE *ask_for_program(STRING, STRING, STRING*, STRING, STRING);
STRING ask_for_indi_key(STRING, CONFIRMQ, BOOLEAN);
INDISEQ ask_for_indi_list(STRING, BOOLEAN);
INT ask_for_int(STRING);
BOOLEAN ask_yes_or_no_msg(STRING, STRING);
INT ask_for_char(STRING, STRING, STRING);
INT ask_for_char_msg(STRING, STRING, STRING, STRING);
STRING ask_for_string(STRING, STRING);
INT choose_from_list(STRING, INT, STRING*);
INDISEQ choose_list_from_indiseq(STRING, INDISEQ);
INT choose_one_from_indiseq(STRING, INDISEQ);
NODE format_and_choose_indi(INDISEQ, BOOLEAN, BOOLEAN, BOOLEAN, STRING, STRING);

/* from main.c */
void final_cleanup(void);

/* from valgdcom.c */
BOOLEAN pointer_value(STRING);

#endif /* _LIFLINES_H */
