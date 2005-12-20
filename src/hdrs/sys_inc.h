/* sys_inc.h */
/* Includes system header files based on platform and compiler */

/* COMMON INCLUDES */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32

/* WIN32 - COMMON INCLUDES */
#include <io.h>
#include <math.h>
#ifndef _POSIX_
#define MAXPATHLEN _MAX_PATH
#endif

/* WIN32 - BORLAND INCLUDES */
#ifdef __BORLANDC__
#include <dir.h>
#include <dos.h>
#define int32 INT32
#endif

/* WIN32 - MSVC INCLUDES */
#ifdef _MSC_VER
#include <direct.h>
#include <fcntl.h> /* _O_BINARY */
#include "msvc.h" /* pragmas to suppress warnings */
#define snprintf _snprintf
#define __int32 INT32
#endif

/* WIN32 - NATIVE FUNCTION IMPLEMENTATIONS */
extern int w32system(const char *cp);

#else

/* UNIX INCLUDES */
#include <inttypes.h>
#include <unistd.h>
#include <math.h>
#include <sys/param.h>

#endif
