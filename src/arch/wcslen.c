/*
  Replacement wcslen for systems lacking such.
*/

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#include <stdlib.h>

size_t wcslen (const wchar_t *s)
{
	const wchar_t *p;
	for (p=s; *p; ++p)
		;
	return p-s;
}

