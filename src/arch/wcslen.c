/*
  Replacement wcslen for systems lacking such.
*/

/* need config.h for HAVE_WCHAR_H */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#include <stdlib.h>
#include "arch.h"

size_t wcslen (const wchar_t *s)
{
	const wchar_t *p;
	for (p=s; *p; ++p)
		;
	return p-s;
}

