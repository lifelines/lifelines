#ifndef _VERSION_H
#define _VERSION_H

/* This is the public build version, from configure */
#define LIFELINES_VERSION PACKAGE_VERSION

/* This is the private build version, appended to the public version */
#define LIFELINES_VERSION_EXTRA "(alpha)"

/* Function prototypes */
STRING get_lifelines_version (INT maxlen);
void print_version (CNSTRING program);

#endif /* _VERSION_H */
