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
#include "llstdlib.h" /* includes standard.h, sys_inc.h, llnls.h, config.h */
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#include "bfs.h"

extern int opt_finnish;

static usersortfnc usersort = 0;

extern bfptr makewide(const char *str);

static BOOLEAN widecmp(char *str1, char *str2, INT *rtn);

/*===================================================
 * ll_strcmp -- Compare two strings
 * currently handles Finnish build (hard-coded compare)
 * locale build
 * and simple strcmp
 * Perry 2001/07/21 moved all custom sort code to
 * llstrcmploc, which is only used via cmpstrloc
 * Most callers of this had been using eqstr, and did NOT
 * need custom sort (eg, eqstr(tag,"HEAD"))
 * TODO: Why isn't this used ? index must not use locale sort
 *=================================================*/
#if UNUSED_CODE
int
ll_strcmp (char *str1, char *str2)
{
	return strcmp(str1, str2);
}
#endif
/*===================================================
 * ll_strcmploc -- Compare two strings with locale
 * which is to say, either Finnish build (hard-coded compare)
 * or user-specified custom sort
 * or locale info if available (HAVE_STRCOLL)
 * Created: 2001/07/21 (Perry Rapp)
 *=================================================*/
int
ll_strcmploc (char *str1, char *str2)
{
	INT rtn;

	/* special Finnish version */
	if (opt_finnish)
		return(MY_STRCMP(str1, str2));

	/* user-defined collation */
	if (usersort && (*usersort)(str1, str2, &rtn))
		return rtn;

	/* regular wchar.h implementation */
	if (widecmp(str1, str2, &rtn))
		return rtn;

#ifdef HAVE_STRCOLL
	errno = 0;
	rtn = strcoll(str1, str2); /* sets errno if fails */
	return !errno ? rtn : strcmp(str1, str2);
#else
	return(strcmp(str1, str2));
#endif
}
/*===================================================
 * ll_strncmp -- Compare two strings
 *  this is just a wrapper for strncmp
 *  except in the Finnish build, it does Finnish
 *  localized comparison
 * TO DO - review this for localization
 *=================================================*/
int
ll_strncmp (char *str1, char *str2, int len)
{
	if(opt_finnish) return(MY_STRNCMP(str1, str2, len));
	/* TO DO - scan thru letters with custom sort order */
	return(strncmp(str1, str2, len));
}
/*===================================================
 * set_usersort -- Install custom sort
 * this will be used by llstrcmp
 *=================================================*/
void
set_usersort (usersortfnc fnc)
{
	usersort = fnc;
}
/*===================================================
 * widecmp -- Perform unicode string comparison, if available
 *=================================================*/
static BOOLEAN
widecmp (char *str1, char *str2, INT *rtn)
{
	bfptr bfs1=0, bfs2=0;
	BOOLEAN success = FALSE;
#ifdef HAVE_WCSCOLL
	if (uu8) {
		/* convert to wchar_t & use wide compare (wcscoll) */
#ifdef _WIN32
		/* MS-Windows really only handles UCS-2 */
		CNSTRING dest = "UCS-2-INTERNAL";
#else
		CNSTRING dest = "UCS-4-INTERNAL";
#endif
		bfs1 = makewide(str1);
		if (bfs1) {
			bfs2 = makewide(str2);
		}

		if (bfs1 && bfs2) {
			const wchar_t * wfs1 = (const wchar_t *)bfs1->str;
			const wchar_t * wfs2 = (const wchar_t *)bfs2->str;
			*rtn = wcscoll(wfs1, wfs2);
			success = TRUE;
		}
	}
#else
	str1=str1; /* unused */
	str2=str2; /* unused */
	rtn=rtn; /* unused */
#endif /* HAVE_WCSCOLL */

	if (bfs1)
		bfDelete(bfs1);
	if (bfs2)
		bfDelete(bfs2);
	return success;
}
