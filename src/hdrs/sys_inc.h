/* sys_inc.h */
/* Includes system header files based on platform and compiler */

/* COMMON INCLUDES */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32

/* WIN32 - COMMON INCLUDES */
#include <io.h>

/* WIN32 - BORLAND INCLUDES */
#ifdef __BORLANDC__
#include <dir.h>
#include <dos.h>
#endif

/* WIN32 - MSVC INCLUDES */
#ifdef _MSC_VER
#include <dirent.h>
#endif

#else

/* UNIX INCLUDES */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#endif
