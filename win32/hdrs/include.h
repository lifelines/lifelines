/* include.h */

#ifndef LL_INCLUDEH
#define LL_INCLUDEH

#include <../../hdrs/standard.h>

/* #define NOMACROS 1 /* used for real curses to find what macros are used */

#include <mycurses.h>

#include <../../hdrs/screen.h>
#include <../../hdrs/btree.h>
#include <../../hdrs/table.h>
#include <../../hdrs/gedcom.h>
#include <../../hdrs/interp.h>
#include <../../hdrs/sequence.h>
#include <../../hdrs/gedcheck.h>
#include <../../hdrs/indiseq.h>
#include <../../hdrs/translat.h>
#include <../../hdrs/cache.h>
#include <ll_getopt.h>
#include <proto.h>
#include <string.h>
#include <fcntl.h>

/* WARNING: Borland C++ 5.02 declares wprintf() in stdio.h. Redefine LifeLines
 * wprintf to llwprintf. [added to standard.h]
 * #define wprintf llwprintf
 */

#endif
