/*
   Copyright (c) 2002 Perry Rapp

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
/*=============================================================
 * llnls.c -- Wrappers for running with old gettext
 *   Created: 2002 by Perry Rapp
 *==============================================================*/

#include "llstdlib.h"


/*********************************************
 * local function definitions
 * body of module
 *********************************************/

#if ENABLE_NLS
/*
  null replacements in case they're using an old gettext
  by pulling these in, we can avoid ifdefs in the main code
  and avoid implementing these with macros
*/
#if !HAVE_NGETTEXT
char * ngettext (const char * msgid, const char * msgid_plural,
                  unsigned long int n)
{
	return n>1 ? (char *)msgid_plural : (char *)msgid;
}

char * dngettext (const char * domainname,
                   const char * msgid, const char * msgid_plural,
                   unsigned long int n)
{
	domainname=domainname; /* unused */
	return n>1 ? (char *)msgid_plural : (char *)msgid;
}
char * dcngettext (const char * domainname,
                    const char * msgid, const char * msgid_plural,
                    unsigned long int n, int category)
{
	domainname=domainname; /* unused */
	category=category; /* unused */
	return n>1 ? (char *)msgid_plural : (char *)msgid;
}
#endif /* !HAVE_NGETTEXT */
#if !HAVE_BIND_TEXTDOMAIN_CODESET
char * bind_textdomain_codeset (const char * domainname,
                                const char * codeset)
{
	domainname=domainname; /* unused */
	codeset=codeset; /* unused */
	return "";
}
#endif /* !HAVE_BIND_TEXTDOMAIN_CODESET */
#endif /* ENABLE_NSL */

