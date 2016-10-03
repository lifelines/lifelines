/*
 * Curses Stub - Choose between system curses or Win32 curses
 */

#include "config.h"

/*
 * Note that we do not use the HAVE_LIB*CURSES* #defines here, since the
 * presence of libraries does not imply the presence of headers. This
 * is because headers and libraries are packaged separately on many
 * distros.
 */

#if defined(HAVE_NCURSESW_CURSES_H)
  #include <ncursesw/curses.h>
#elif defined(HAVE_NCURSES_CURSES_H)
  #include <ncurses/curses.h>
#elif defined(HAVE_CURSES_H)
  #include <curses.h>
#elif defined(HAVE_WINDOWS_H)
    #include "mswin/curses.h"
#else
    #error Unsupported curses configuration!
#endif
