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
#include "llstdlib.h"

/*===========================================
 * IS_PATH_SEP -- Is path separator character
 *  handle WIN32 characters
 *=========================================*/
#ifdef WIN32
#define IS_PATH_SEP(qq) ((qq) == LLCHRPATHSEPARATOR || (qq) == '/')
#else
#define IS_PATH_SEP(qq) ((qq) == LLCHRPATHSEPARATOR)
#endif
/*=================================================
 * is_dir_sep -- Is directory separator character ?
 *  handle WIN32 characters
 *===============================================*/
BOOLEAN
is_dir_sep (char c)
{
#ifdef WIN32
	return c==LLCHRDIRSEPARATOR || c=='/';
#else
	return c==LLCHRDIRSEPARATOR;
#endif
}
/*===============================================
 * is_absolute_path -- Is this an absolute path ?
 *  ie, does this begin with directory info
 *  handle WIN32 characters
 *=============================================*/
static BOOLEAN
is_absolute_path (STRING dir)
{
	if (is_dir_sep(*dir) || *dir == '.') return TRUE;
#ifdef WIN32
	if (is_dir_sep(*dir) || (*dir && dir[1] == ':' && isalpha(*dir))) return TRUE;
#endif
	return FALSE;
}
/*=========================================
 * path_match -- are paths the same ?
 *  handle WIN32 filename case insensitivity
 *========================================*/
static BOOLEAN
path_match (STRING path1, STRING path2)
{
#ifdef WIN32
	return !stricmp(path1, path2);
#else
	return !strcmp(path1, path2);
#endif
}
/*=============================================
 * test_concat_path -- test code for concat_path
 *===========================================*/
#if TEST_CODE
static void
test_concat_path (void)
{
	STRING readpath;
	readpath = concat_path("hey", "jude");
	readpath = concat_path("hey", "/jude");
	readpath = concat_path("hey/", "jude");
	readpath = concat_path("hey/", "/jude");
	readpath = concat_path("hey", "jude");
	readpath = concat_path("hey", "\\jude");
	readpath = concat_path("hey/", "jude");
	readpath = concat_path("hey\\", "\\jude");
	readpath = concat_path(NULL, "\\jude");
	readpath = concat_path("hey", NULL);
}
#endif
/*=============================================
 * concat_path -- add file & directory together
 *  returns static buffer
 *  handles NULL in either argument
 *  handles trailing / in dir and/or leading / in file
 *  (see test_concat_path above)
 *  returns no trailing / if file is NULL
 *  returns static buffer
 *===========================================*/
STRING
concat_path (STRING dir, STRING file)
{
	static char buffer[MAXPATHLEN];
	STRING ptr = buffer;
	INT len=sizeof(buffer);
	ptr[0]=0;
	if (dir)
		llstrcatn(&ptr, dir, &len);
	if (is_dir_sep(buffer[strlen(buffer)-1])) {
		if (!file) {
			buffer[strlen(buffer)-1] = 0;
		} else {
			/* dir ends in sep */
			if (is_dir_sep(file[0])) {
				/* file starts in sep */
				llstrcatn(&ptr, &file[1], &len);
			} else {
				/* file doesn't start in sep */
				llstrcatn(&ptr, file, &len);
			}
		}
	} else {
		if (!file) {
		} else {
			if (is_dir_sep(file[0])) {
				/* file starts in sep */
				llstrcatn(&ptr, file, &len);
			} else {
				/* file doesn't start in sep */
				llstrcatn(&ptr, "/", &len);
				llstrcatn(&ptr, file, &len);
			}
		}
	}
	return buffer;
}
/*===========================================
 * filepath -- Find file in sequence of paths
 *  handles NULL in either argument
 *  returns alloc'd buffer
 *=========================================*/
STRING
filepath (STRING name,
          STRING mode,
          STRING path,
          STRING  ext)
{
	char buf1[MAXPATHLEN], buf2[MAXPATHLEN];
	STRING p, q;
	INT c;
	INT nlen, elen, dirs;

	if (ISNULL(name)) return NULL;
	if (ISNULL(path)) return name;
	if (is_absolute_path(name)) return name;
	nlen = strlen(name);
	if(ext && *ext) {
		elen = strlen(ext);
		if((elen > nlen) && path_match(name+nlen-elen, ext)) {
		/*  name has an explicit extension the same as this one */
			ext = NULL;
			elen = 0;
		}
	}
	else { ext = NULL; elen = 0; }
	if (nlen + strlen(path) + elen >= MAXLINELEN) return NULL;
	strcpy(buf1, path);
	p = buf1;
	dirs = 1; /* count dirs in path */
	while ((c = *p)) {
		if (IS_PATH_SEP(c)) {
			*p = 0;
			dirs++;
		}
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
FILE *
fopenpath (STRING name,
           STRING mode,
           STRING path,
           STRING ext,
           STRING *pfname)
{
	STRING str;
	if(pfname) *pfname = NULL;
	if (!(str = filepath(name, mode, path, ext))) return NULL;
	if(pfname) *pfname = str;
	return fopen(str, mode);
}
/*=================================================
 * lastpathname -- Return last component of a path
 *===============================================*/
STRING
lastpathname (STRING path)
{
	static unsigned char scratch[MAXLINELEN+1];
	INT len, c;
	STRING p = scratch, q;
	if (ISNULL(path)) return NULL;
	len = strlen(path);
	strcpy(p, path);
	if (is_dir_sep(p[len-1])) {
		len--;
		p[len] = 0;
	}
	q = p;
	while ((c = *p++)) {
		if (is_dir_sep((char)c))
			q = p;
	}
	return q;
}
