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

#endif
