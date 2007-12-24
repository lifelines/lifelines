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
 * intlshim.h -- Shim to connect to gettext dll if available
 *   Created: 2002/06 by Perry Rapp
 *==============================================================*/

#ifndef INTLSHIM_H_INCLUDED
#define INTLSHIM_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif

/* package may define INTLDECL to be __declspec(dllexport) to reexport these shim functions */
#ifndef INTLDECL
#define INTLDECL
#endif

/* Look up MSGID in the current default message catalog for the current
   LC_MESSAGES locale.  If not found, returns MSGID itself (the default
   text).  */
INTLDECL char *gettext(const char *__msgid);

/* Look up MSGID in the DOMAINNAME message catalog for the current
   LC_MESSAGES locale.  */
INTLDECL char *dgettext(const char *__domainname, 
				     const char *__msgid);

/* Look up MSGID in the DOMAINNAME message catalog for the current CATEGORY
   locale.  */
INTLDECL char *dcgettext(const char *__domainname,
				      const char *__msgid,
				      int __category);


/* Similar to `gettext' but select the plural form corresponding to the
   number N.  */
INTLDECL char *ngettext(const char *__msgid1,
				     const char *__msgid2,
				     unsigned long int __n);

/* Similar to `dgettext' but select the plural form corresponding to the
   number N.  */
INTLDECL char *dngettext(const char *__domainname,
				      const char *__msgid1,
				      const char *__msgid2,
				      unsigned long int __n);

/* Similar to `dcgettext' but select the plural form corresponding to the
   number N.  */
INTLDECL char *dcngettext(const char *__domainname,
				       const char *__msgid1,
				       const char *__msgid2,
				       unsigned long int __n,
				       int __category);


/* Set the current default message catalog to DOMAINNAME.
   If DOMAINNAME is null, return the current default.
   If DOMAINNAME is "", reset to the default of "messages".  */
INTLDECL char *textdomain(const char *__domainname);

/* Specify that the DOMAINNAME message catalog will be found
   in DIRNAME rather than in the system locale data base.  */
INTLDECL char *bindtextdomain(const char *__domainname,
					   const char *__dirname);

/* Specify the character encoding in which the messages from the
   DOMAINNAME message catalog will be returned.  */
INTLDECL char *bind_textdomain_codeset(const char *__domainname,
						    const char *__codeset);

/* increment nl_msg_cat_cntr in gettext */
INTLDECL void gt_notify_language_change(void);

/* pass some get_property call to gettext */
INTLDECL int gt_get_property(const char *name, char *value, int valuelen);

/* pass some set_property call to gettext */
INTLDECL int gt_set_property(const char *name, const char *value);

/* tell gettext dll where to find iconv.dll */
INTLDECL int gt_set_property(const char *name, const char *value);

/* intlshim specific API */
INTLDECL int intlshim_get_property(const char *name, char * value, int valuelen);
INTLDECL int intlshim_set_property(const char *name, const char *value);

#ifdef __cplusplus
}
#endif

#endif /* INTLSHIM_H_INCLUDED */

