
/* Function Prototypes */

STRING ask_for_indi_key(STRING, BOOLEAN, BOOLEAN);
NODE ask_for_fam(STRING, STRING);
FILE *ask_for_file(STRING, STRING, STRING*, STRING, STRING);
NODE ask_for_indi(STRING, BOOLEAN, BOOLEAN);
NODE ask_for_indi_once(STRING, BOOLEAN, INT*);
STRING ask_for_string(STRING, STRING);

NODE add_family(NODE, NODE, NODE);
NODE add_unlinked_indi(NODE);
NODE add_indi_by_edit(void);

void delete_indi(NODE, BOOLEAN);
NODE edit_family(NODE);
NODE edit_indi(NODE);

NODE format_and_choose_indi(INDISEQ, BOOLEAN, BOOLEAN, BOOLEAN, STRING, STRING);

NODE sort_children(NODE, NODE);
NODE remove_dupes(NODE, NODE);
NODE merge_two_indis(NODE, NODE, BOOLEAN);
NODE merge_two_fams(NODE, NODE);

