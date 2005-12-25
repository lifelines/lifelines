/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*======================================================
 * path.c -- Handle files with environment variables
 * Copyright (c) by T.T. Wetmore IV; all rights reserved
 *=====================================================*/

#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#ifndef WIN32
#include <pwd.h>
#endif

/*********************************************
 * local function prototypes
 *********************************************/
static STRING get_user_homedir(STRING username);
static INT zero_separate_path(STRING path);

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
 * is_path -- Is this a path (not a bare filename) ?
 *  ie, does this have any slashes in it?
 * (does not handle escaped slashes)
 *=============================================*/
BOOLEAN
is_path (CNSTRING dir)
{

	if (strchr(dir,LLCHRDIRSEPARATOR)) return TRUE;
#ifdef WIN32
	/* windows \ or / is path separator. */
	if (strchr(dir,'/')) return TRUE;
	if (dir[0] && dir[1]==':' && isasciiletter(dir[0])) {
		return TRUE;
	}
#endif
	return FALSE;
}
#ifdef NOTUSED
/*===============================================
 * is_absolute_path -- Is this an absolute path ?
 *  handle WIN32 characters, and ~ homedir references
 *=============================================*/
BOOLEAN
is_absolute_path (CNSTRING dir)
{
	/* if it starts with a slash, it's absolute */
	if (is_dir_sep(dir[0])) return TRUE;
	/* if it starts with a dot, it's relative, but not relative
	to search path, only to current directory -- as in shell logic 
	so we say it is absolute */
	if (dir[0] == '.') return TRUE;
	/* if it starts with ~, it is a reference to an absolute home dir */
	if (dir[0] == '~') return TRUE;
#ifdef WIN32
	/* if it starts with a drive letter, it's absolute */
	if (dir[0] && dir[1]==':' && isasciiletter(dir[0])) {
		return TRUE;
	}
#endif
	return FALSE;
}	
#endif
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
 * path_cmp -- compare two paths as appropriate
 *  handle WIN32 filename case insensitivity
 *===============================*/
INT
path_cmp (CNSTRING path1, CNSTRING path2)
{
/* Special case for Win32, which needs case-insensitive */
#ifdef WIN32
	return _stricoll(path1, path2);
#else
/* We use direct RTL calls, because paths are not in
internal codeset, they are in system codeset */
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
#ifdef TEST_CODE
static void
test_concat_path (void)
{
	char buffer[MAXPATHLEN];
	STRING testpath;
	testpath = concat_path("hey", "jude", buffer, sizeof(buffer));
	testpath = concat_path("hey", "/jude", buffer, sizeof(buffer));
	testpath = concat_path("hey/", "jude", buffer, sizeof(buffer));
	testpath = concat_path("hey/", "/jude", buffer, sizeof(buffer));
	testpath = concat_path("hey", "jude", buffer, sizeof(buffer));
	testpath = concat_path("hey", "\\jude", buffer, sizeof(buffer));
	testpath = concat_path("hey/", "jude", buffer, sizeof(buffer));
	testpath = concat_path("hey\\", "\\jude", buffer, sizeof(buffer));
	testpath = concat_path(NULL, "\\jude", buffer, sizeof(buffer));
	testpath = concat_path(NULL, "jude", buffer, sizeof(buffer));
	testpath = concat_path("hey", NULL, buffer, sizeof(buffer));
	testpath = concat_path("hey/", NULL, buffer, sizeof(buffer));
	testpath = concat_path(NULL, NULL, buffer, sizeof(buffer));
	testpath = concat_path("/", NULL, buffer, sizeof(buffer));
}
#endif
/*=============================================
 * concat_path_alloc -- add file & directory together into newly alloc'd string & return
 *  dir:  [IN]  directory (may be NULL)
 *  file: [IN]  file (may be NULL)
 * See concat_path
 *===========================================*/
STRING
concat_path_alloc (CNSTRING dir, CNSTRING file)
{
	INT len = (dir ? strlen(dir) : 0) + (file ? strlen(file) : 0) +2;
	STRING buffer = malloc(len);
	INT myutf8=0; /* buffer is big enough, so won't matter */
	return concat_path(dir, file, myutf8, buffer, len);
}
/*=============================================
 * concat_path -- add file & directory together
 *  dir:  [IN]  directory (may be NULL)
 *  file: [IN]  file (may be NULL)
 *  handles trailing / in dir and/or leading / in file
 *  (see test_concat_path above)
 *  returns no trailing / if file is NULL
 *===========================================*/
STRING
concat_path (CNSTRING dir, CNSTRING file, INT utf8, STRING buffer, INT buflen)
{
	ASSERT(buflen);
	buffer[0] = 0;
	if (dir && dir[0]) {
		llstrapps(buffer, buflen, utf8, dir);
		if (is_dir_sep(buffer[strlen(buffer)-1])) {
			/* dir ends in sep */
			if (!file || !file[0]) {
				/* dir but no file, we don't include trailing slash */
				buffer[strlen(buffer)-1] = 0;
			} else {
				if (is_dir_sep(file[0])) {
					/* file starts in sep */
					llstrapps(buffer, buflen, utf8, &file[1]);
				} else {
					/* file doesn't start in sep */
					llstrapps(buffer, buflen, utf8, file);
				}
			}
		} else {
			/* dir doesn't end in sep */
			if (!file || !file[0]) {
				/* dir but no file, we don't include trailing slash */
			} else {
				if (is_dir_sep(file[0])) {
					/* file starts in sep */
					llstrapps(buffer, buflen, utf8, file);
				} else {
					/* file doesn't start in sep */
					llstrapps(buffer, buflen, utf8, LLSTRDIRSEPARATOR);
					llstrapps(buffer, buflen, utf8, file);
				}
			}
		}
	} else {
		/* no dir, include file exactly as it is */
		if (file && file[0])
			llstrapps(buffer, buflen, utf8, file);
	}

	return buffer;
}
/*===========================================
 * filepath -- Find file in sequence of paths
 *  handles NULL in either argument
 *  returns alloc'd buffer
 * Warning: if called with mode other than "r" this may not 
 *         do what you want, because 1) if file is not found, it appends ext.
 *         and 2) file always goes in 1st specified directory of path unless
 *         name is absolute or ./something
 *=========================================*/
STRING
filepath (CNSTRING name, CNSTRING mode, CNSTRING path, CNSTRING  ext, INT utf8)
{
	char buf1[MAXPATHLEN], buf2[MAXPATHLEN];
	STRING p, q;
	INT nlen, elen;

	if (ISNULL(name)) return NULL;
	if (ISNULL(path)) return strsave(name);
	nlen = strlen(name);
	if (ext && *ext) {
		elen = strlen(ext);
		if ((nlen > elen) && path_match(name+nlen-elen, ext)) {
			ext = NULL;
			elen = 0;
		}
	}
	else { ext = NULL; elen = 0; }
	/*  at this point ext is non-null only if name doesn't 
	 *  end in ext.  I.E. we need to check for name + ext and name
	 */

	if (nlen + strlen(path) + elen >= MAXLINELEN) return NULL;

	/* for absolute and relative path names we first check the
	 * pathname for validity relative to the current directory
	 */
	if (is_path(name)) {
		/* If this is a path, i.e. it has multiple path elements
		 * use the name as is first.  So absolute paths don't
		 * get resolved by appending search paths.
		 */
		if (ext) {
			strcpy(buf1,name);
			nlen = strlen(buf1);
			strcat(buf1, ext);
			if (access(buf1, 0) == 0) return strsave(buf1);
			buf1[nlen] = '\0'; /* remove extension */
			if (access(buf1, 0) == 0) return strsave(buf1);
		}
		if (access(name, 0) == 0) return strsave(name);
		/* fail if get here and name begins with a / or ./
		 * as we didn't find the file.
		 * however let foo/bar sneak thru, so we allow access
		 * to files in subdirectories of the search path if 
		 * named in the filename.
		 */
		if (is_dir_sep(name[0]) || (name[0] == '.' && is_dir_sep(name[1])))
			return strsave(name);
#ifdef WIN32
		if (name[0] && name[1]==':' && isasciiletter(name[0])) {
			return strsave(name);
		}
#endif
	}

	/* it is a relative path, so search for it in search path */
	strcpy(buf1, path);
	zero_separate_path(buf1);
	p = buf1;
	while (*p) {
		q = buf2;
		strcpy(q, p);
		expand_special_fname_chars(buf2, sizeof(buf2), utf8);
		q += strlen(q);
		if (q>buf2 && !is_dir_sep(q[-1])) {
			strcpy(q, LLSTRDIRSEPARATOR);
			q++;
		}
		strcpy(q, name);
		if (ext) {
			nlen = strlen(buf2);
			strcat(buf2, ext);
			if (access(buf2, 0) == 0) return strsave(buf2);
			buf2[nlen] = '\0'; /* remove extension */
		}
		if (access(buf2, 0) == 0) return strsave(buf2);
		p += strlen(p);
		p++;
	}
	if (mode[0] == 'r') return NULL;
	p = buf1;
	q = buf2;
	strcpy(q, p);
	expand_special_fname_chars(buf2, sizeof(buf2), utf8);
	q += strlen(q);
	if (q>buf2 && !is_dir_sep(q[-1])) {
		strcpy(q, LLSTRDIRSEPARATOR);
		q++;
	}
	strcpy(q, name);
	if (ext) strcat(q, ext);
	return strsave(buf2);
}
/*===========================================
 * zero_separate_path -- Zero separate dirs in path
 * (Also appends extra zero at end)
 * Assumes there are no empty directory entries.
 * Returns count of directories.
 *=========================================*/
static INT
zero_separate_path (STRING path)
{
	INT c=0, dirs=0;
	if (!path[0]) {
		path[1] = 0;
		return 0;
	}
	++dirs;
	while ((c = (uchar)*path)) {
		if (is_path_sep((uchar)c)) {
			*path = 0;
			++dirs;
		}
		++path;
	}
	path[1] = 0;
	return dirs;
}
/*===========================================
 * get_first_path_entry -- Return first directory
 *  on specified path
 *  returns static buffer
 *=========================================*/
CNSTRING
get_first_path_entry (CNSTRING path)
{
	static char buf1[MAXPATHLEN];

	if (!path) return NULL;
	if (!path[0]) return path;
	if (strlen(path)+2>sizeof(buf1)) return "";
	strcpy(buf1, path);
	zero_separate_path(buf1);
	return buf1;
}
/*===========================================
 * fopenpath -- Open file using path variable
 *  pfname: [OUT]  stdalloc'd copy of full path found
 *=========================================*/
FILE *
fopenpath (STRING name, STRING mode, STRING path, STRING ext, INT utf8
	, STRING *pfname)
{
	FILE * fp;
	STRING str;
	if(pfname) *pfname = NULL;
	if (!(str = filepath(name, mode, path, ext, utf8))) return NULL;
	if(pfname) {
		*pfname = str;
	}
	fp = fopen(str, mode);
	if (!pfname) stdfree(str);
	return fp;
}
/*===========================================
 * closefp -- Close file pointer & set pointer to zero
 *=========================================*/
void
closefp (FILE **pfp)
{
	if (pfp && *pfp) {
		fclose(*pfp);
		*pfp = 0;
	}
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
 *  dirs:  [OUT] output buffer
 * NB: dirs should be one byte larger than path
 *     ignore zero length paths
 *================================================*/
INT
chop_path (CNSTRING path, STRING dirs)
{
	INT ndirs;
	STRING p;
	CNSTRING q;
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
	*p = 0; /* ends with extra trailing zero after last one */
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
expand_special_fname_chars (STRING buffer, INT buflen, INT utf8)
{
	char * sep=0;
	if (buffer[0]=='~') {
		if (is_dir_sep(buffer[1])) {
			STRING home = get_home();
			if (home && home[0]) {
				STRING tmp;
				if ((INT)strlen(home) + 1 + (INT)strlen(buffer) > buflen) {
					return FALSE;
				}
				tmp = strsave(buffer);
				buffer[0] = 0;
				llstrapps(buffer, buflen, utf8, home);
				llstrapps(buffer, buflen, utf8, tmp+1);
				strfree(&tmp);
				return TRUE;
			}
		}
		/* check for ~name/... and resolve the ~name */
		if ((sep = strchr(buffer,LLCHRDIRSEPARATOR))) {
			STRING username = strsave(buffer+1);
			STRING homedir;
			username[sep-buffer+1] = 0;
			homedir = get_user_homedir(username);
			strfree(&username);
			if (homedir) {
				STRING tmp=0;
				if ((INT)strlen(homedir) + 1 + (INT)strlen(sep+1) > buflen) {
					return FALSE;
				}
				tmp = strsave(sep+1);
				buffer[0] = 0;
				llstrapps(buffer, buflen, utf8, homedir);
				llstrapps(buffer, buflen, utf8, tmp+(sep-buffer+1));
				strfree(&tmp);
				return TRUE;
			}
		}
	}
	return TRUE;
}
/*============================================
 * get_user_homedir -- Return home directory of specified user
 *  returns 0 if unknown or error
 *  returns alloc'd value
 *==========================================*/
static STRING
get_user_homedir (STRING username)
{
	struct passwd *pw=0;
	if (!username) return 0;
#ifdef WIN32
	/*
	This could be implemented for NT+ class	using NetUserGetInfo,
	but I doubt it's worth the trouble. Perry, 2005-11-25.
	*/
#else /* not WIN32 */
	setpwent();
	/* loop through the password file/database
	 * to see if the string following ~ matches
	 * a login name - 
	 */
	while ((pw = getpwent())) {
		if (eqstr(pw->pw_name,username)) {
			/* found user in passwd file */
			STRING homedir = strsave(pw->pw_dir);
			endpwent();
			return homedir;
		}
	}
	endpwent();
#endif
	return 0;
}
