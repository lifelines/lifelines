#ifndef _STDLIB_PRIV_H
#define _STDLIB_PRIV_H

void get_backtrace(int);
ZSTR makewide(const char * str);
ZSTR makeznarrow(ZSTR);
BOOLEAN iswletter (wchar_t wch);

#endif /* _STDLIB_PRIV_H */
