#ifndef _LIFLINES_PRIV_H
#define _LIFLINES_PRIV_H

typedef struct llrect_s {
	INT top;
	INT bottom;
	INT left;
	INT right;
} *LLRECT;

struct import_feedback;

/* add.c */
NODE add_family(NODE, NODE, NODE);
RECORD add_indi_by_edit(void);
BOOLEAN add_indi_no_cache(NODE);
STRING get_unresolved_ref_error_string(INT count);
NODE prompt_add_child(NODE, NODE);
BOOLEAN prompt_add_spouse(NODE, NODE, BOOLEAN);

/* advedit.c */
void advanced_person_edit(NODE);
void advanced_family_edit(NODE);

/* ask.c */
RECORD ask_for_any(STRING ttl, CONFIRMQ, ASK1Q);
NODE ask_for_any_old(STRING ttl, CONFIRMQ, ASK1Q);
RECORD ask_for_indi(STRING ttl, CONFIRMQ, ASK1Q);
NODE ask_for_indi_old(STRING ttl, CONFIRMQ, ASK1Q);
INDISEQ ask_for_indiseq(STRING ttl, char ctype, INT *prc);
BOOLEAN ask_yes_or_no(STRING);

/* browse.c */
void browse(NODE, INT code);
RECORD choose_any_event(void);
RECORD choose_any_other(void);
RECORD choose_any_source(void);
BOOLEAN handle_fam_mode_cmds(INT c, INT * mode);
BOOLEAN handle_indi_mode_cmds(INT c, INT * mode);
BOOLEAN handle_menu_cmds(INT c, BOOLEAN * reuse);
BOOLEAN handle_scroll_cmds(INT c, BOOLEAN * reuse);
void init_browse_module(void);
void term_browse_module(void);

/* delete.c */
void delete_indi(NODE, BOOLEAN);
void choose_and_delete_family(void);
BOOLEAN choose_and_remove_child(NODE indi, NODE fam, BOOLEAN nolast);
BOOLEAN choose_and_remove_spouse(NODE indi, NODE fam, BOOLEAN nolast);

/* edit.c */
NODE edit_family(NODE);
NODE edit_indi(NODE);

/* export.c */
BOOLEAN archive_in_file (void);

/* import.c */
BOOLEAN import_from_gedcom_file(struct import_feedback * ifeed, FILE *fp);

/* lbrowse.c */
INT browse_list(NODE*, NODE*, NODE*, NODE*, INDISEQ*);

/* loadsave.c */
void load_gedcom(void);

/* merge.c */
NODE merge_two_indis(NODE, NODE, BOOLEAN);
NODE merge_two_fams(NODE, NODE);

/* miscutls.c */
void key_util(void);
void show_database_stats(void);
void who_is_he_she(void);

/* newrecs.c */
RECORD ask_for_record(STRING, INT);
NODE edit_add_event(void);
NODE edit_add_other(void);
NODE edit_add_source(void);
void edit_event(NODE);
void edit_other(NODE);
void edit_source(NODE);

/* pedigree.c */
	/* gedcom view mode */
enum { GDVW_NORMAL, GDVW_EXPANDED, GDVW_TEXT };
	/* data for output canvas */
	/* NB: pedigree will adjust scroll if out of limits */
	struct canvasdata_s;
		/* callback to output a line */
	typedef void (*PEDLINE)(struct canvasdata_s * canvas, INT x, INT y
		, STRING string, INT overflow);
		/* collection of data needed by pedigree */
	typedef struct canvasdata_s { LLRECT rect; INT scroll; void * param;
		PEDLINE line; } *CANVASDATA;
	/* functions */
void pedigree_draw_ancestors(NODE indi, CANVASDATA canvasdata, BOOLEAN reuse);
void pedigree_draw_descendants(NODE indi, CANVASDATA canvasdata, BOOLEAN reuse);
void pedigree_draw_gedcom(NODE node, INT gdvw, CANVASDATA canvasdata, BOOLEAN reuse);
void pedigree_increase_generations(INT delta);
void pedigree_toggle_mode(void);

/* scan.c */
RECORD full_name_scan(STRING sts);
RECORD name_fragment_scan(STRING sts);
RECORD refn_scan(STRING sts);

/* screen.c */
void adjust_menu_cols(INT delta);
void adjust_menu_height(INT delta);
INT aux_browse(NODE, INT mode, BOOLEAN reuse);
void cycle_menu(void);
void display_2fam(NODE fam1, NODE fam2, INT mode);
void display_2indi(NODE indi1, NODE indi2, INT mode);
void display_fam(NODE, INT mode, BOOLEAN reuse);
void display_indi(NODE, INT mode, BOOLEAN reuse);
INT interact_2fam(void);
INT interact_2indi(void);
INT interact_fam(void);
INT interact_indi(void);
INT list_browse(INDISEQ seq, INT top, INT *cur, INT mark, NODE * pindi);
void lock_status_msg(BOOLEAN lock);
void toggle_menu(void);

/* show.c */
extern struct rfmt_s disp_long_rfmt, disp_shrt_rfmt;
void display_cache_stats(void);
void init_show_module(void);
void show_big_list(INDISEQ, INT, INT, INT);
void show_childnumbers(void);
void show_reset_scroll(void);
void show_sour_display(NODE, INT, INT);
void show_scroll(INT delta);
void show_scroll2(INT delta);
void switch_scrolls(void);
void term_show_module(void);

/* swap.c */
BOOLEAN swap_children(NODE prnt, NODE fam);
BOOLEAN reorder_child(NODE prnt, NODE fam);
BOOLEAN swap_families(NODE);

/* tandem.c */
INT browse_tandem(NODE*, NODE*, NODE*, NODE*, INDISEQ*);
INT browse_2fam(NODE*, NODE*, NODE*, NODE*, INDISEQ*);

/* valgdcom.c */
int check_stdkeys (void);
void addmissingkeys (INT);
INT xref_to_index (STRING);
BOOLEAN validate_gedcom (struct import_feedback * ifeed, FILE*);

#endif /* _LIFLINES_PRIV_H */
