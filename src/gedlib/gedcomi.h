#ifndef _GEDCOM_PRIV_H
#define _GEDCOM_PRIV_H

/* charmaps.c */
ZSTR custom_translate(CNSTRING str, TRANTABLE tt);
void custom_translatez(ZSTR zstr, TRANTABLE tt);
BOOLEAN init_map_from_file(CNSTRING file, CNSTRING mapname, TRANTABLE*, ZSTR zerr);

/* init.c */
void dbnotify_close(void);

/* keytonod.c */
void cel_remove_record(CACHEEL cel, RECORD rec);
void cel_set_record(CACHEEL cel, RECORD rec);

NODE is_cel_loaded(CACHEEL cel);

/* names.c */
RECORD id_by_key(CNSTRING name, char ctype);

/* node.c */
void set_record_key_info(RECORD rec, char ntype, INT keynum);

/* record.c */
RECORD create_record_for_keyed_node(NODE node, CNSTRING key);
RECORD create_record_for_unkeyed_node(NODE node);
NODE is_record_temp(RECORD rec);
void set_record_cache_info(RECORD rec, CACHEEL cel);


/* xreffile.c */
BOOLEAN xrefs_get_counts_from_unopened_db(CNSTRING path, INT *nindis, INT *nfams
	, INT *nsours, INT *nevens, INT *nothrs);

#endif /* _GEDCOM_PRIV_H */
