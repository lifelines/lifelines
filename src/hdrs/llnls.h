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
 * llnls.h -- Handle NLS macros & declarations
 *   Created: 2002/06 by Perry Rapp
 *==============================================================*/

#ifndef _LLNLS_H
#define _LLNLS_H

/* Need config.h for ENABLE_NLS */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if ENABLE_NLS
/*** NLS (National Language Support) ***/

/*#include <libintl.h>*/
#include "libgnuintl.h"

/* _() is used for normally translated strings */
#define _(String) gettext(String)

/* TODO: add keyword argument for _pl:2,3 & start using _pl macro */
/* #define _pl(Singular, Plural, Num) ngettext(Singular, Plural, Num) */

/* Old gettext didn't have ngettext, so provide null version */
#if !HAVE_NGETTEXT
#define ngettext(sing, pl, n) ngettext_null(sing, pl, n)
#define dngettext(dom, sing, pl, n) dngettext_null(dom, sing, pl, n)
#define dcngettext(dom, sing, pl, n, cat) dcngettext_null(dom, sing, pl, n, cat)
char * ngettext_null(const char *, const char *, unsigned long int);
char * dngettext_null(const char *, const char *, const char *, unsigned long int);
char * dcngettext_null(const char *, const char *, const char *, unsigned long int, int);
#endif /* !HAVE_NGETTEXT */

/* Old gettext didn't have bind_textdomain_codeset, so provide null version */
#if !HAVE_BIND_TEXTDOMAIN_CODESET
#define bind_textdomain_codeset(td, cs) bind_textdomain_codeset_null(td, cs)
char * bind_textdomain_codeset_null(const char *, const char *);
#endif /* !HAVE_BIND_TEXTDOMAIN_CODESET */


/* We can't use _N() for nonstranslated strings (eg "%d") -- TODO */

/* N_() is used for strings needing translation elsewhere, eg static inits */
#define N_(String) (String)

#else
/*** No NLS (National Language Support) ***/

#define _(String) String
#define N_(String) (String)
#define textdomain(Domain)
#define bindtextdomain(Package, Directory)
#define ngettext(Singular, Plural, Num) Plural
#endif


#endif /* ENABLE_NLS */
