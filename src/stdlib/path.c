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
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 12 Aug 93
 *   3.0.0 - 05 May 94    3.0.2 - 01 Dec 94
 *   3.0.3 - 06 Sep 95
 *=====================================================*/

#include "sys_inc.h"
#include "llstdlib.h"

/*===========================================
 * is_path_sep -- Is path separator character
 *  handle WIN32 characters
 *=========================================*/
BOOLEAN
is_path_sep (char c)
{
	/* : on UNIX and ; on Windows */
	return c==LLCHRPATHSEPARATOR;
}
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
is_absolute_path (CNSTRING dir)
{
	if (is_dir_sep(*dir) || *dir == '.') return TRUE;
	if (*dir == '~') return TRUE;
#ifdef WIN32
	if (is_dir_sep(*dir) || (*dir && dir[1] == ':' && isalpha((uchar)*dir))) return TRUE;
#endif
	return FALSE;
}
/*=========================================
 * path_match -- are paths the same ?
 *  handle WIN32 filename case insensitivity
 *========================================*/
BOOLEAN
path_match (CNSTRING path1, CNSTRING path2)
{
#ifdef WIN32
	return !stricmp((STRING)path1, (STRING)path2);
#else
	return !strcmp((STRING)path1, (STRING)path2);
#endif
}
/*=================================
 * path_cmp -- compare two paths
 * handle WIN32 filename case insensitivity
 *===============================*/
int
path_cmp (CNSTRING path1, CNSTRING path2)
{
#ifdef WIN32
	return _stricoll(path1, path2);
#else
#ifdef HAVE_STRCOLL_H
	return strcoll(path1, path2);
#else
	return strcmp(path1, path2);
#endif
#endif
}
/*=============================================
 * test_concat_path -- test code for concat_path
 *===========================================*/
#if TEST_CODE
static void
test_concat_path (void)
{
	STRING testpath;
	testpath = concat_path("hey", "jude");
	testpath = concat_path("hey", "/jude");
	testpath = concat_path("hey/", "jude");
	testpath = concat_path("hey/", "/jude");
	testpath = concat_path("hey", "jude");
	testpath = concat_path("hey", "\\jude");
	testpath = concat_path("hey/", "jude");
	testpath = concat_path("hey\\", "\\jude");
	testpath = concat_path(NULL, "\\jude");
	testpath = concat_path("hey", NULL);
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
concat_path (CNSTRING dir, CNSTRING file)
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
				llstrcatn(&ptr, LLSTRDIRSEPARATOR, &len);
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
filepath (CNSTRING name,
          CNSTRING mode,
          CNSTRING path,
          CNSTRING  ext)
{
	char buf1[MAXPATHLEN], buf2[MAXPATHLEN];
	STRING p, q;
	INT c;
	INT nlen, elen, dirs;

	if (ISNULL(name)) return NULL;
	if (ISNULL(path)) return strsave(name);
	if (is_absolute_path(name)) return strsave(name);
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
		if (is_path_sep((unsigned char)c)) {
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
		expand_special_fname_chars(buf2, sizeof(buf2));
		q += strlen(q);
		if (q>buf2 && !is_dir_sep(q[-1])) {
			strcpy(q, LLSTRDIRSEPARATOR);
			q++;
		}
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
	expand_special_fname_chars(buf2, sizeof(buf2));
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
 * returns static buffer
 * (This returns a copy because if input was "xx/yy/"
 *  return is "yy")
 *===============================================*/
STRING
lastpathname (CNSTRING path)
{
	static char scratch[MAXLINELEN+1];
	INT len, c;
	STRING p = scratch, q;
	if (ISNULL(path)) return NULL;
	len = strlen(path);
	ASSERT(len < (INT)sizeof(scratch));
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
/*========================================
 * compress_path -- return path truncated
 *  returns static buffer in all cases
 *  truncates path from the front if needed
 * Created: 2001/12/22 (Perry Rapp)
 *======================================*/
STRING
compress_path (CNSTRING path, INT len)
{
	static char buf[120];
	INT pathlen = strlen(path);
	if (len > (INT)sizeof(buf)-1)
		len = (INT)sizeof(buf)-1;
	/* TODO: be nice to expand "."  */
	if (pathlen > len) {
		STRING dotdotdot = "...";
		INT delta = pathlen - len - strlen(dotdotdot);
		strcpy(buf, dotdotdot);
		strcpy(buf+strlen(dotdotdot), path+delta);
	} else {
		strcpy(buf, path);
	}
	return buf;
}
/*========================================================================
 * check_file_for_unicode -- Check for BOM (byte order mark) bytes
 *  return descriptive string if found, or 0 if not
 *  eg, "UTF-8", or "UTF-16LE or UTF-32LE"
 *  return "?" if invalid BOM 
 * If found, advance file pointer past BOM.
 * It would be nice if someone clever made this less messy.
 *======================================================================*/
STRING
check_file_for_unicode (FILE * fp)
{
	INT c1 = (uchar)fgetc(fp);
	if (c1 == 0xEF) {
		INT c2 = (uchar)fgetc(fp);
		if (c2 == 0xBB) {
			/* EF BB BF is BOM for UTF-8 */
			INT c3 = (uchar)fgetc(fp);
			if (c3 == 0xBF)
				return "UTF-8";
			ungetc(c3, fp);
		}
		/* No other BOMs start with EF */
		ungetc(c2, fp);
		ungetc(c1, fp);
		return 0;
	} else if (c1 == 0xFF) {
		INT c2 = (uchar)fgetc(fp);
		if (c2 == 0xFE) {
			/* FF FE is UTF-16LE BOM (or start of UTF-32LE BOM) -- we reject */
			ungetc(c2, fp);
			ungetc(c1, fp);
			return "UTF-16LE or UTF-32LE";
		}
		/* No other BOMs start with FF */
		ungetc(c2, fp);
		ungetc(c1, fp);
		return 0;
	} else if (c1 == 0xFE) {
		INT c2 = (uchar)fgetc(fp);
		if (c2 == 0xFF) {
			/* FE FF is UTF-16BE BOM -- we reject */
			ungetc(c2, fp);
			ungetc(c1, fp);
			return "UTF-16BE";
		}
		/* No other BOMs start with FE */
		ungetc(c2, fp);
		ungetc(c1, fp);
		return 0;
	} else if (c1 == 0) {
		/* Could be 00 00 FE FF for UTF-32BE, 
		but we don't want it no matter what it is */
		ungetc(c1, fp);
		return "Possibly UTF-32BE?";
	}
	ungetc(c1, fp);
	return 0;
}
/*==================================================
 * chop_path -- copy path into buff, & zero-separate all dirs
 *  path:  [IN]  path list to copy
 *  buff:  [OUT] output buffer
 * NB: buff should be one byte larger than path
 *================================================*/
INT
chop_path (STRING path, STRING dirs)
{
	INT ndirs;
	STRING p, q;
	char c=0;
	ndirs=0;
	p = dirs;;
	q = path;
	while ((c = *q)) {
		if (is_path_sep(c)) {
			if (p == dirs || p[-1] == 0) {
				q++;
			} else {
				*p++ = 0;
				q++;
				++ndirs;
			}
		} else {
			*p++ = *q++;
		}
	}
	if (!(p == dirs || p[-1] == 0)) {
		*p++ = 0;
		++ndirs;
	}
	*(++p) = 0;
	return ndirs;
}
/*============================================
 * get_home -- Find user's home
 *==========================================*/
static STRING
get_home (void)
{
	STRING home;
#ifdef WIN32
	/* replace ~ with user's home directory, if present */
	/* TODO: Or HOMEPATH, HOMESHARE, or USERPROFILE ? */
	home = (STRING)getenv("APPDATA");
#else
	home = (STRING)getenv("HOME");
#endif
	return home;
}
/*============================================
 * expand_special_fname_chars -- Replace ~ with home
 *==========================================*/
BOOLEAN
expand_special_fname_chars (STRING buffer, INT buflen)
{
	if (buffer[0]=='~') {
		if (is_dir_sep(buffer[1])) {
			STRING home = get_home();
			if (home && home[0]) {
				STRING tmp;
				if ((INT)strlen(buffer)+(INT)strlen(home)+1 > buflen) {
					return FALSE;
				}
				tmp = strsave(buffer);
				buffer[0] = 0;
				llstrapp(buffer, buflen, home);
				llstrapp(buffer, buflen, tmp+1);
				strfree(&tmp);
				return TRUE;
			}
		}
		/* TODO: handle other homes (eg, ~someone/) ? */
	}
	return TRUE;
}
