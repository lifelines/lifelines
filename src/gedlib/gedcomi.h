#ifndef _GEDCOM_PRIV_H
#define _GEDCOM_PRIV_H

/* charmaps.c */
ZSTR custom_translate(CNSTRING str, TRANTABLE tt);
void custom_translatez(ZSTR zstr, TRANTABLE tt);
BOOLEAN init_map_from_file(CNSTRING file, CNSTRING mapname, TRANTABLE*, ZSTR zerr);

/* init.c */
void dbnotify_close(void);

/* keytonod.c */

/* names.c */
RECORD id_by_key(CNSTRING name, char ctype);

/* node.c */
void assign_record(RECORD rec, char ntype, INT keynum);
RECORD create_record_for_unkeyed_node(NODE node);
RECORD create_record_for_keyed_node(NODE node, CNSTRING key);

/* xreffile.c */
BOOLEAN xrefs_get_counts_from_unopened_db(CNSTRING path, INT *nindis, INT *nfams
	, INT *nsours, INT *nevens, INT *nothrs);

#endif /* _GEDCOM_PRIV_H */
