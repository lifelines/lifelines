#ifndef listui_h_included
#define listui_h_included

INT array_interact (STRING ttl, INT len, STRING *strings
	, BOOLEAN selectable, DETAILFNC detfnc, void * param);

INT choose_one_or_list_from_indiseq (STRING ttl, INDISEQ seq, BOOLEAN multi);

void listui_init_windows(INT extralines);
void listui_placecursor_main(INT * prow, INT * pcol);
void paint_list_screen(void);

#endif /* listui_h_included */
