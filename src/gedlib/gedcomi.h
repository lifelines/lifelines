#ifndef _GEDCOM_PRIV_H
#define _GEDCOM_PRIV_H

/* from charmaps.c */
void custom_translate(ZSTR * pzstr, TRANTABLE tt);
BOOLEAN init_map_from_file(CNSTRING file, CNSTRING mapname, TRANTABLE*, ZSTR *pzerr);

/* from names.c */
RECORD id_by_key(STRING name, char ctype);

#endif /* _GEDCOM_PRIV_H */
