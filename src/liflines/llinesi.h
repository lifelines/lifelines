/*
  Functions used inside the liflines module
  The high-level but not curses-specific portion of the program
*/

#ifndef _LIFLINES_PRIV_H
#define _LIFLINES_PRIV_H

typedef struct tag_llrect {
	INT top;
	INT bottom;
	INT left;
	INT right;
} *LLRECT;

struct tag_import_feedback;
#ifndef IMPORT_FEEDBACK_type_defined
typedef struct tag_import_feedback *IMPORT_FEEDBACK;
#define IMPORT_FEEDBACK_type_defined
#endif

struct tag_export_feedback;

/* add.c */
RECORD add_family(RECORD spouse1, RECORD spouse2, RECORD child);
RECORD add_indi_by_edit(void);
BOOLEAN add_indi_no_cache(NODE);
STRING get_unresolved_ref_error_string(INT count);
NODE prompt_add_child(NODE child, NODE fam, RFMT rfmt);
BOOLEAN prompt_add_spouse(RECORD spouse, RECORD fam, BOOLEAN conf);

/* advedit.c */
void advanced_person_edit(NODE);
void advanced_family_edit(NODE);

/* ask.c */
RECORD ask_for_any(STRING ttl, CONFIRMQ, ASK1Q);
INDISEQ ask_for_indiseq(STRING ttl, char ctype, INT *prc);

/* browse.c */
RECORD choose_any_event(void);
RECORD choose_any_other(void);
RECORD choose_any_source(void);
RECORD disp_vhistory_list(void);
RECORD disp_chistory_list(void);
INT get_vhist_len(void);
INT get_chist_len(void);
BOOLEAN handle_fam_mode_cmds(INT c, INT * mode);
BOOLEAN handle_indi_mode_cmds(INT c, INT * mode);
BOOLEAN handle_menu_cmds(INT c, BOOLEAN * reuse);
BOOLEAN handle_scroll_cmds(INT c, BOOLEAN * reuse);
void history_record_change(RECORD);
void init_browse_module(void);
void main_browse(RECORD, INT code);
NODE my_prompt_add_child(NODE child, NODE fam);
void term_browse_module(void);

/* delete.c */
void delete_indi(NODE, BOOLEAN);
void choose_and_delete_family(void);
BOOLEAN choose_and_remove_child(RECORD irec, RECORD frec, BOOLEAN nolast);
BOOLEAN choose_and_remove_spouse(RECORD irec, RECORD frec, BOOLEAN nolast);

/* edit.c */
BOOLEAN edit_family(RECORD);
BOOLEAN edit_indi(RECORD);

/* export.c */
BOOLEAN archive_in_file (struct tag_export_feedback * efeed, FILE *fp);

/* import.c */
BOOLEAN import_from_gedcom_file(IMPORT_FEEDBACK ifeed, FILE *fp);

/* lbrowse.c */
INT browse_list(RECORD *prec1, RECORD *prec2, INDISEQ *pseq);


/* merge.c */
RECORD merge_two_indis(NODE, NODE, BOOLEAN);
RECORD merge_two_fams(NODE, NODE);

/* miscutls.c */
void key_util(void);
void show_database_stats(void);
void who_is_he_she(void);

/* newrecs.c */
RECORD edit_add_event(void);
RECORD edit_add_other(void);
RECORD edit_add_source(void);
BOOLEAN edit_any_record(RECORD rec);
BOOLEAN edit_event(RECORD);
BOOLEAN edit_other(RECORD);
BOOLEAN edit_source(RECORD);

/* pedigree.c */
	/* gedcom view mode */
enum { GDVW_NORMAL, GDVW_EXPANDED, GDVW_TEXT };
	/* data for output canvas */
	/* NB: pedigree will adjust scroll if out of limits */
	struct tag_canvasdata;
		/* callback to output a line */
	typedef void (*PEDLINE)(struct tag_canvasdata * canvas, INT x, INT y
		, STRING string, INT overflow);
		/* collection of data needed by pedigree */
	typedef struct tag_canvasdata { LLRECT rect; INT scroll; void * param;
		PEDLINE line; } *CANVASDATA;
	/* functions */
void pedigree_draw_ancestors(RECORD rec, CANVASDATA canvasdata, BOOLEAN reuse);
void pedigree_draw_descendants(RECORD rec, CANVASDATA canvasdata, BOOLEAN reuse);
void pedigree_draw_gedcom(RECORD rec, INT gdvw, CANVASDATA canvasdata, BOOLEAN reuse);
void pedigree_increase_generations(INT delta);
void pedigree_toggle_mode(void);

/* scan.c */
RECORD full_name_scan(STRING sts);
RECORD name_fragment_scan(STRING sts);
RECORD refn_scan(STRING sts);

/* swap.c */
BOOLEAN swap_children(RECORD prnt, RECORD frec);
BOOLEAN reorder_child(RECORD prnt, RECORD frec, RFMT rfmt);
BOOLEAN swap_families(RECORD);

/* tandem.c */
INT browse_tandem(RECORD *prec1, RECORD *prec2, INDISEQ *pseq);
INT browse_2fam(RECORD *prec1, RECORD *prec2, INDISEQ *pseq);

/* valgdcom.c */
void addmissingkeys (INT);
int check_stdkeys (void);
BOOLEAN scan_header(FILE * fp, TABLE metadatatab, ZSTR * zerr);
BOOLEAN validate_gedcom (IMPORT_FEEDBACK ifeed, FILE*);
INT xref_to_index (STRING);


#endif /* _LIFLINES_PRIV_H */
