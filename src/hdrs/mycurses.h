/*
 * Curses Stub - Choose between system curses or Win32 curses
 */

#include "config.h"

#if defined(HAVE_LIBNCURSES) || defined(HAVE_LIBCURSES)
#include <curses.h>
#elif defined(HAVE_WINDOWS_H)
#include "win32/curses.h"
#else
#error Unsupported curses configuration!
#endif
