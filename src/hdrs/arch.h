/* -*- Mode: C; c-file-style:"gnu"; indent-tabs-mode:nil -*-
   arch.h -- wrapper API for architecture dependant behaviour
*/

#ifndef ARCH_H
#define ARCH_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SLEEP
#include <unistd.h>
#else
extern int sleep(int seconds);
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#else
struct dirent /* Simple replacement for the less fortunate platforms */
{
	char d_name[256];
};
#endif

#ifndef HAVE_SCANDIR
extern int scandir(const char *dir, struct dirent ***namelist,
                   int (*select)(const struct dirent *),
                   int (*compar)(const struct dirent **,
                                 const struct dirent **));
#endif

#ifndef HAVE_ALPHASORT
extern int alphasort(const struct dirent **a, const struct dirent **b);
#endif

#endif
