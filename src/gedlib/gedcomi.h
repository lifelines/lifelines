#ifndef _GEDCOM_PRIV_H
#define _GEDCOM_PRIV_H

/* from charmaps.c */
BOOLEAN init_map_from_file(STRING, INT, TRANTABLE*);

/* from names.c */
RECORD id_by_key(STRING name, char ctype);

#endif /* _GEDCOM_PRIV_H */
