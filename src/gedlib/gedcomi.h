#ifndef _GEDCOM_PRIV_H
#define _GEDCOM_PRIV_H

/* from charmaps.c */
TRANTABLE init_map_from_file(STRING, INT, BOOLEAN*);

/* from names.c */
STRING* id_by_key(STRING, STRING**);

#endif /* _GEDCOM_PRIV_H */
