/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
/*======================================================
 * path.c -- Handle files with environment variables
 * Copyright (c) by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 12 Aug 93
 *   3.0.0 - 05 May 94    3.0.2 - 01 Dec 94
 *   3.0.3 - 06 Sep 95
 *=====================================================*/

#ifndef WIN32
#include <unistd.h>
#endif
#include "standard.h"

/*===========================================
 * filepath -- Find file in sequence of paths
 *=========================================*/
STRING filepath (name, mode, path)
STRING name, mode, path;
{
	unsigned char buf1[MAXLINELEN], buf2[MAXLINELEN];
	STRING p, q;
	INT c;

	if (!name || *name == 0) return NULL;
	if (!path || *path == 0) return name;
	if (*name == LLCHRDIRSEPARATOR || *name == '.') return name;
#ifdef WIN32
	if ((*name == '/') || ((name[1] == ':') && isalpha(*name))) return name;
#endif
	if (strlen(name) + strlen(path) >= MAXLINELEN) return NULL;
	strcpy(buf1, path);
	p = buf1;
	while (c = *p) {
		if (c == LLCHRPATHSEPARATOR
#ifdef WIN32
	    	    || c == '/'
#endif
			) *p = 0;
		p++;
	}
	*(++p) = 0;
	p = buf1;
	while (*p) {
		q = buf2;
		strcpy(q, p);
		q += strlen(q);
		strcpy(q, LLSTRDIRSEPARATOR);
		q++;
		strcpy(q, name);
		if (access(buf2, 0) == 0) return strsave(buf2);
		p += strlen(p);
		p++;
	}
	if (mode[0] == 'r') return NULL;
	p = buf1;
	q = buf2;
	strcpy(q, p);
	q += strlen(q);
	strcpy(q, LLSTRDIRSEPARATOR);
	q++;
	strcpy(q, name);
	return strsave(buf2);
}
/*===========================================
 * fopenpath -- Open file using path variable
 *=========================================*/
FILE *fopenpath (name, mode, path)
STRING name, mode, path;
{
	STRING str;
	if (!(str = filepath(name, mode, path))) return NULL;
	return fopen(str, mode);
}
/*=================================================
 * lastpathname -- Return last componenet of a path
 *===============================================*/
STRING lastpathname (path)
STRING path;
{
	static unsigned char scratch[MAXLINELEN+1];
	INT len, c;
	STRING p = scratch, q;
	if (!path || *path == 0) return NULL;
	len = strlen(path);
	strcpy(p, path);
	if (p[len-1] == LLCHRDIRSEPARATOR
#ifdef WIN32
	    || p[len-1] == '/'
#endif
		) {
		len--;
		p[len] = 0;
	}
	q = p;
	while (c = *p++) {
		if (c == LLCHRDIRSEPARATOR
#ifdef WIN32
	    	    || c == '/'
#endif
			) q = p;
	}
	return q;
}
