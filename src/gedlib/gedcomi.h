#ifndef _GEDCOM_PRIV_H
#define _GEDCOM_PRIV_H

/* charmaps.c */
void custom_translate(ZSTR zstr, TRANTABLE tt);
BOOLEAN init_map_from_file(CNSTRING file, CNSTRING mapname, TRANTABLE*, ZSTR zerr);

/* init.c */
void dbnotify_close(void);

/* names.c */
RECORD id_by_key(CNSTRING name, char ctype);

/* node.c */
RECORD alloc_new_record(void);
void init_new_record_and_just_read_node (RECORD rec, NODE node, CNSTRING key);

/* xreffile.c */
BOOLEAN xrefs_get_counts_from_unopened_db(CNSTRING path, INT *nindis, INT *nfams
	, INT *nsours, INT *nevens, INT *nothrs);

#endif /* _GEDCOM_PRIV_H */
