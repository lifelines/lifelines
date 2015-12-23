RECORD rptui_ask_for_fam(STRING s1, STRING s2);
INDISEQ rptui_ask_for_indi_list(STRING ttl, BOOLEAN reask);
STRING rptui_ask_for_indi_key(STRING ttl, ASK1Q ask1);
BOOLEAN rptui_ask_for_int(STRING, INT *);
FILE * rptui_ask_for_output_file(STRING mode, STRING ttl, STRING *pfname
	, STRING *pfullpath, STRING path, STRING ext);
BOOLEAN rptui_ask_for_program(STRING mode, STRING ttl, STRING *pfname
	, STRING *pfullpath, STRING path, STRING ext, BOOLEAN picklist);
INT rptui_choose_from_array(STRING ttl, INT no, STRING *pstrngs);
int rptui_elapsed(void);
void rptui_init(void);
INT rptui_prompt_stdout(STRING prompt);
void rptui_view_array(STRING ttl, INT no, STRING *pstrngs);
