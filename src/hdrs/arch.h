/* -*- Mode: C; c-file-style:"gnu"; indent-tabs-mode:nil -*-
   arch.h -- wrapper API for architecture dependant behaviour
*/

#ifndef ARCH_H
#define ARCH_H

/*
 * .c file must have already included config.h if available
 * Normally this is done by including llstdlib.h first
 */

/* *****************************************************************
 * sleep()           
 * ***************************************************************** */

/* 
 * We need to use the external definiton of sleep in arch/sleep.c if:
 * - we're on Windows
 * - there is no sleep() function available
 */

#if defined(HAVE_WINDOWS_H) || !defined(HAVE_SLEEP)
extern int sleep(int seconds);
#else
#include <unistd.h>
#endif /* HAVE_WINDOWS_H or not HAVE_SLEEP */

/* *****************************************************************
 * scandir()           
 * ***************************************************************** */

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#else
struct dirent /* Simple replacement for the less fortunate platforms */
{
	char d_name[256];
};
#endif /* HAVE_DIRENT_H */

#ifndef HAVE_SCANDIR
extern int scandir(const char *dir, struct dirent ***namelist,
                   int (*select)(const struct dirent *),
                   int (*compar)(const struct dirent **,
                                 const struct dirent **));
#endif /* HAVE_SCANDIR */

/* *****************************************************************
 * alphasort()           
 * ***************************************************************** */

#ifndef HAVE_ALPHASORT
extern int alphasort(const struct dirent **a, const struct dirent **b);
#endif /* HAVE_ALPHASORT */

/* *****************************************************************
 * strcmpi()           
 * ***************************************************************** */

#ifndef HAVE_STRCMPI
#ifdef HAVE_STRCASECMP
#define strcmpi strcasecmp
#endif /* HAVE_STRCASECMP */
#endif /* HAVE_STRCMPI */

/* *****************************************************************
 * getopt()           
 * ***************************************************************** */

#ifndef HAVE_GETOPT
#ifndef HAVE_GETOPT_H

/* Don't use prototypes in case getopt() is defined in some header 
 * that we're not checking during configure, and would cause conficts
 * here. */

extern int getopt();
extern char *optarg;
extern int optind;
extern int opterr;

#endif /* HAVE_GETOPT */
#endif /* HAVE_GETOPT_H */

/* *****************************************************************
 * vsnprintf()
 * ***************************************************************** */

#ifndef HAVE_VSNPRINTF
int vsnprintf(char *buffer, size_t count, const char *fmt, va_list args);
#endif /* HAVE_VSNPRINTF */

/* *****************************************************************
 * WIN32/MSVC hacks
 * ***************************************************************** */
#ifdef WIN32
#ifdef _MSC_VER
/* MS-Windows/Microsoft Visual C++ */
#define S_ISREG(qq) ((qq) & S_IFREG)
#define S_ISDIR(qq) ((qq) & S_IFDIR)
#endif
#endif


#endif /* ARCH_H */
