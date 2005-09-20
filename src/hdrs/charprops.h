#ifndef charprops_h_included
#define charprops_h_included

BOOLEAN charprops_load_utf8(void);
void charprops_free_all(void);
BOOLEAN charprops_load(const char * codepage);
BOOLEAN charprops_is_loaded(void);

#endif /* charprops_h_included */
