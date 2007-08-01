/*
 * Curses Stub - Choose between system curses or Win32 curses
 */

#include "config.h"

#if defined(HAVE_LIBNCURSESW)
    #if defined(HAVE_NCURSESW_CURSES_H)
    #include <ncursesw/curses.h>
    #elif defined(HAVE_NCURSES_CURSES_H)
    #include <ncurses/curses.h>
    #else
    #include <curses.h>
    #endif
#elif defined(HAVE_LIBNCURSES)
    #if defined(HAVE_NCURSES_CURSES_H)
    #include <ncurses/curses.h>
    #else
    #include <curses.h>
    #endif
#elif defined(HAVE_LIBCURSES)
#include <curses.h>
#elif defined(HAVE_WINDOWS_H)
#include "win32/curses.h"
#else
#error Unsupported curses configuration!
#endif
