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

#include "sys_inc.h"
#include "standard.h"

/*===========================================
 * filepath -- Find file in sequence of paths
 *=========================================*/
STRING filepath (name, mode, path, ext)
STRING name, mode, path, ext;
{
	unsigned char buf1[MAXLINELEN], buf2[MAXLINELEN];
	STRING p, q;
	INT c;
	int nlen, elen;

	if (!name || *name == 0) return NULL;
	if (!path || *path == 0) return name;
	if (*name == LLCHRDIRSEPARATOR || *name == '.') return name;
#ifdef WIN32
	if ((*name == '/') || ((name[1] == ':') && isalpha(*name))) return name;
#endif
	nlen = strlen(name);
	if(ext && *ext) {
	    elen = strlen(ext);
	    if((elen > nlen)
#ifdef WIN32
		&& (stricmp(name+nlen-elen, ext) == 0)
#else
		&& (strcmp(name+nlen-elen, ext) == 0)
#endif
		) {
		/*  name has an explicit extension the same as this one */
		ext = NULL;
		elen = 0;
	    }
	}
	else { ext = NULL; elen = 0; }
	if (nlen + strlen(path) + elen >= MAXLINELEN) return NULL;
	strcpy(buf1, path);
	p = buf1;
	while ((c = *p)) {
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
		if(ext) {
		    strcat(buf2, ext);
		    if(access(buf2, 0) == 0) return strsave(buf2);
		    nlen = strlen(buf2);
		    buf2[nlen-elen] = '\0'; /* remove extension */
		}
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
	if(ext) strcat(q, ext);
	return strsave(buf2);
}
/*===========================================
 * fopenpath -- Open file using path variable
 *=========================================*/
FILE *fopenpath (name, mode, path, ext, pfname)
STRING name, mode, path, ext;
STRING *pfname;
{
	STRING str;
	if(pfname) *pfname = NULL;
	if (!(str = filepath(name, mode, path, ext))) return NULL;
	if(pfname) *pfname = str;
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
	while ((c = *p++)) {
		if (c == LLCHRDIRSEPARATOR
#ifdef WIN32
	    	    || c == '/'
#endif
			) q = p;
	}
	return q;
}
