#ifndef _GEDCOM_PRIV_H
#define _GEDCOM_PRIV_H

/* charmaps.c */
void custom_translate(ZSTR zstr, TRANTABLE tt);
BOOLEAN init_map_from_file(CNSTRING file, CNSTRING mapname, TRANTABLE*, ZSTR zerr);

/* names.c */
RECORD id_by_key(STRING name, char ctype);

/* node.c */
RECORD alloc_new_record(void);
void init_new_record_and_just_read_node (RECORD rec, NODE node, CNSTRING key);

#endif /* _GEDCOM_PRIV_H */
