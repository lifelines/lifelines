/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * locales.c -- functions dealing with locales
 * TODO: this is a mess of ifdefs -- please clean up at some point
 *==============================================================*/

#include "llstdlib.h"
#ifdef HAVE_LOCALE_H
# include <locale.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
# include "isolangs.h"
#endif
#endif
#ifdef HAVE_LANGINFO_CODESET
# include <langinfo.h>
#else
# include "langinfz.h"
#endif
#include "translat.h"
#include "liflines.h"
#include "feedback.h"
#include "icvt.h"
#include "lloptions.h"
#include "date.h"
#include "gedcomi.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void customlocale(STRING prefix);
static STRING get_current_locale(INT category);
#ifdef ENABLE_NLS
static BOOLEAN is_msgcategory(int category);
#if ! ( defined(HAVE_SETLOCALE) && defined(HAVE_LC_MESSAGES) )
static STRING llsetenv(STRING name, STRING value);
#endif /* ! (defined(HAVE_SETLOCALE) && defined(HAVE_LC_MESSAGES) ) */
#endif /* ENABLE_NLS */
static void notify_gettext_language_changed(void);
static void send_uilang_callbacks(void);
#ifdef ENABLE_NLS
static STRING setmsgs(STRING localename);
#endif /* ENABLE_NLS */
static char * win32_setlocale(int category, char * locale);

/*********************************************
 * local variables
 *********************************************/

static STRING  deflocale_coll = NULL;
static STRING  deflocale_msgs = NULL;
static BOOLEAN customized_loc = FALSE;
#ifdef ENABLE_NLS
static BOOLEAN customized_msgs = FALSE;
#endif /* ENABLE_NLS */
static STRING current_coll = NULL; /* most recent */
static STRING current_msgs = NULL; /* most recent */
static STRING rptlocalestr = NULL; /* if set by report program */
static LIST f_uicodeset_callbacks = NULL; /* collection of callbacks for UI codeset changes */
static LIST f_uilang_callbacks = NULL; /* collection of callbacks for UI language changes */



/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==========================================
 * get_current_locale -- return current locale
 *  returns "C" in case setlocale(x, 0) returns 0
 * Created: 2002/02/24 (Perry Rapp)
 *========================================*/
#ifdef HAVE_SETLOCALE
static STRING
get_current_locale (INT category)
{
	STRING str = 0;
	str = setlocale(category, NULL);
	return str ? str : "C";
}
#endif /* HAVE_SETLOCALE */
/*==========================================
 * save_original_locales -- grab current locales for later default
 *  We need these for an obscure problem. If user sets only
 *  locales for report, then when we switch back to GUI mode,
 *  we shouldn't stay in the customized report locale.
 * Created: 2002/02/24 (Perry Rapp)
 *========================================*/
void
save_original_locales (void)
{
	/* get collation locale, if available */
#ifdef HAVE_SETLOCALE
	deflocale_coll = strsave(get_current_locale(LC_COLLATE));
	current_coll = strsave(deflocale_coll);
#endif /* HAVE_SETLOCALE */

	/* get messages locale (via locale or via environ.) */
#ifdef HAVE_SETLOCALE
#ifdef LC_MESSAGES
	if (LC_MESSAGES >= 0 && LC_MESSAGES != 1729) {
		/* 1729 is the gettext code when there wasn't any LC_MESSAGES */
		deflocale_msgs = strsave(get_current_locale(LC_MESSAGES));
		current_msgs = strsave(deflocale_msgs);
	}
#endif /* LC_MESSAGES */
#endif /* HAVE_SETLOCALE */
	/* fallback to the environment (see setmsgs) */
	if (!deflocale_msgs) {
		STRING msgs = getenv("LC_MESSAGES");
		deflocale_msgs = strsave(msgs ? msgs : "");
		current_msgs = strsave(deflocale_msgs);
	}

}
/*==========================================
 * get_original_locale_collate -- Get collation locale captured at startup
 * caller may not alter string
 * Created: 2002/06/15 (Perry Rapp)
 *========================================*/
STRING
get_original_locale_collate (void)
{
#ifdef HAVE_SETLOCALE
	return deflocale_coll;
#endif /* HAVE_SETLOCALE */
	return "";
}
/*==========================================
 * get_current_locale_collate -- Get collation locale (as last set)
 * caller may not alter string
 * Created: 2002/06/15 (Perry Rapp)
 *========================================*/
STRING
get_current_locale_collate (void)
{
	return current_coll;
}
/*==========================================
 * get_original_locale_msgs -- Get LC_MESSAGES locale captured at startup
 * caller may not alter string
 * Created: 2002/06/15 (Perry Rapp)
 *========================================*/
STRING
get_original_locale_msgs (void)
{
	return deflocale_msgs;
}
/*==========================================
 * get_current_locale_msgs -- Get LC_MESSAGES locale (as last set)
 * caller may not alter string
 * Created: 2002/06/15 (Perry Rapp)
 *========================================*/
STRING
get_current_locale_msgs (void)
{
	return current_msgs;
}
/*==========================================
 * are_locales_supported -- locale support compiled in ?
 * Created: 2002/06/15 (Perry Rapp)
 *========================================*/
BOOLEAN
are_locales_supported (void)
{
#ifdef HAVE_SETLOCALE
	return TRUE;
#endif /* HAVE_SETLOCALE */
	return FALSE;
}
/*==========================================
 * is_nls_supported -- is NLS (National Language Support) compiled in ?
 * Created: 2002/06/15 (Perry Rapp)
 *========================================*/
BOOLEAN
is_nls_supported (void)
{
#ifdef ENABLE_NLS
	return TRUE;
#endif /* ENABLE_NLS */
	return FALSE;
}
/*==========================================
 * is_iconv_supported -- is iconv (codeset conversion library) compiled in ?
 * Created: 2002/06/15 (Perry Rapp)
 *========================================*/
BOOLEAN
is_iconv_supported (void)
{
#ifdef HAVE_ICONV
	return TRUE;
#endif /* HAVE_ICONV */
	return FALSE;
}
/*==========================================
 * ll_langinfo -- wrapper for nl_langinfo
 *  in case not provided (eg, MS-Windows)
 *========================================*/
STRING
ll_langinfo (void)
{
	STRING str = nl_langinfo(CODESET);
	/* TODO: Should we apply norm_charmap.c ?
	http://www.cl.cam.ac.uk/~mgk25/ucs/norm_charmap.c
	*/
	/* TODO: In any case tho, Markus' nice replacement nl_langinfo gives the
	wrong default codepages for MS-Windows I think -- eg, should be 1252 for 
	generic default instead of 8859-1 */

	/* TODO: Check out libcharset (in the libiconv distribution)
	It probably has the Win32 code in it */

	return str ? str : ""; /* I don't know if nl_langinfo ever returns NULL */
}
/*==========================================
 * termlocale -- free locale related variables
 * Created: 2002/02/24 (Perry Rapp)
 *========================================*/
void
termlocale (void)
{
	/* free & zero out globals */
	strfree(&deflocale_coll);
	strfree(&current_coll);
	strfree(&deflocale_msgs);
	strfree(&current_msgs);
}
/*==========================================
 * uilocale -- set locale to GUI locale
 *  (eg, for displaying a sorted list of people)
 * Created: 2001/08/02 (Perry Rapp)
 *========================================*/
void
uilocale (void)
{
	update_textdomain_localedir(PACKAGE, "Ui");

	customlocale("UiLocale");
}
/*==========================================
 * rptlocale -- set locale to report locale
 *  (eg, for _namesort)
 * Created: 2001/08/02 (Perry Rapp)
 *========================================*/
void
rptlocale (void)
{
	/* 2007-04-19, Perry:
	 I'm not sure what textdomain to use here
	 rptinfo has a textdomain (see llrpt_gettext)
	 but that is per-rptinfo
	*/

	customlocale("RptLocale");
	if (rptlocalestr) /* report has specified locale */
		llsetlocale(LC_ALL, rptlocalestr);
}
/*==========================================
 * rpt_setlocale -- set report locale to custom locale
 *  used by report language
 * Created: 2002/06/27 (Perry Rapp)
 *========================================*/
STRING
rpt_setlocale (STRING str)
{
	strfree(&rptlocalestr);
	rptlocalestr = llsetlocale(LC_ALL, str);
	if (rptlocalestr)
		rptlocalestr = strsave(rptlocalestr);
	return rptlocalestr;
}
/*==========================================
 * setmsgs -- set locale for LC_MESSAGES
 * Returns non-null string if succeeds
 *========================================*/
#ifdef ENABLE_NLS
static STRING
setmsgs (STRING localename)
{
	STRING str;
	if (eqstr_ex(current_msgs, localename))
		return localename; /* skip it if already current */
#if defined(HAVE_SETLOCALE) && defined(HAVE_LC_MESSAGES)
	str = setlocale(LC_MESSAGES, localename);
	if (str) {
		strfree(&current_msgs);
		current_msgs = strsave(str);
	}
#else
	str = llsetenv("LC_MESSAGES", localename);
	if (str) {
		strfree(&current_msgs);
		current_msgs = strsave(str);
	}
#endif
	if (str) {
		notify_gettext_language_changed();
		send_uilang_callbacks();
		date_update_lang();
	}
	return str;
}
#endif /* ENABLE_NLS */
#ifdef ENABLE_NLS
#if ! ( defined(HAVE_SETLOCALE) && defined(HAVE_LC_MESSAGES) )
/*==========================================
 * llsetenv -- assign a value to an environment variable
 * Workaround for systems without HAVE_SETLOCALE && HAVE_LC_MESSAGES
 * Returns value if it succeeded
 *========================================*/
static STRING
llsetenv (STRING name, STRING value)
{
	char buffer[128];
	STRING str = 0;

	buffer[0] = 0;
	llstrappf(buffer, sizeof(buffer), uu8, "%s=%s", name, value);

#ifdef HAVE_SETENV
	if (setenv(name, value, 1) != -1)
		str = value;
#else
#ifdef HAVE_PUTENV
	if (putenv(buffer) != -1)
		str = value;
#else
#ifdef HAVE__PUTENV
	if (_putenv(buffer) != -1)
		str = value;
#endif /* HAVE__PUTENV */
#endif /* HAVE_PUTENV */
#endif /* HAVE_SETENV */
	return str;
}
#endif /* !defined(HAVE_SETLOCALE) && !defined(HAVE_LC_MESSAGES) */
#endif /* ENABLE_NLS */
/*==========================================
 * customlocale -- set locale to custom setting
 *  depending on user options
 *  prefix:  [IN]  option prefix (eg, "UiLocale")
 * Created: 2002/02/24 (Perry Rapp)
 *========================================*/
static void
customlocale (STRING prefix)
{
	char option[64];
	STRING str;
	INT prefixlen = strlen(prefix);
	
	if (prefixlen > 30) return;

	strcpy(option, prefix);

#ifdef HAVE_SETLOCALE
	/* did user set, eg, UiLocaleCollate option ? */
	strcpy(option+prefixlen, "Collate");
	str = getlloptstr(option, 0);
	if (str) {
		customized_loc = TRUE;
		str = llsetlocale(LC_COLLATE, str);
	}
	if (!str) {
		/* did user set, eg, UiLocale option ? */
		option[prefixlen] = 0;
		str = getlloptstr(option, 0);
		if (str) {
			customized_loc = TRUE;
			str = llsetlocale(LC_COLLATE, str);
		}
		/* nothing set, so try to revert to startup value */
		if (!str && customized_loc)
			llsetlocale(LC_COLLATE, deflocale_coll);
	}
#endif /* HAVE_SETLOCALE */

#ifdef ENABLE_NLS
	/* did user set, eg, UiLocaleMessages option ? */
	strcpy(option+prefixlen, "Messages");
	str = getlloptstr(option, 0);
	if (str) {
		customized_msgs = TRUE;
		str = setmsgs(str);
	} else {
		/* did user set, eg, UiLocale option ? */
		option[prefixlen] = 0;
		str = getlloptstr(option, 0);
		if (str) {
			customized_msgs = TRUE;
			str = setmsgs(str);
		}
		if (!str && customized_msgs)
			setmsgs(deflocale_msgs ? deflocale_msgs : "");
	}
#endif /* ENABLE_NLS */
}
/*==========================================
 * notify_gettext_language_changed --
 *  signal gettext that desired language has changed
 * Created: 2002/06/15 (Perry Rapp)
 *========================================*/
static void
notify_gettext_language_changed (void)
{
#if ENABLE_NLS
#if  WIN32_INTL_SHIM
	gt_notify_language_change();
#else
	extern int _nl_msg_cat_cntr;
	++_nl_msg_cat_cntr;
#endif
#endif
}
/*==========================================
 * llsetlocale -- wrapper for setlocale
 * Handle MS-Windows annoying lack of LC_MESSAGES
 * TODO: clean up other translat.c functions by calling this
 *========================================*/
char *
llsetlocale (int category, char * locale)
{
	char * rtn = "C";
#ifdef HAVE_SETLOCALE
	rtn = setlocale(category, locale);
	if (!rtn && locale)
		rtn = win32_setlocale(category, locale);
#endif /* HAVE_SETLOCALE */
#ifdef ENABLE_NLS
	if (rtn && is_msgcategory(category)) {
		setmsgs(locale);
	}
#endif /* ENABLE_NLS */
	return rtn;
}
/*==========================================
 * is_msgcategory -- check for LC_ALL or LC_MESSAGES
 *========================================*/
#ifdef ENABLE_NLS
static BOOLEAN
is_msgcategory (int category)
{
#ifdef LC_MESSAGES
	return category==LC_ALL || category==LC_MESSAGES;
#else
	return category==LC_ALL;
#endif
}
#endif /* ENABLE_NLS */
/*==========================================
 * win32_setlocale -- handle MS-Windows goofed up locale names
 *========================================*/
static char *
win32_setlocale (int category, char * locale)
{
	char * rtn = 0;
#if defined(_WIN32) && !defined(__CYGWIN__)
	/* TODO: Obviously this needs work -- and move to win32 subdir ? */
	if (locale) {
		char w32loc[30]="";
		char * ptr;
		int i;
		for (i=0; langs[i]; i += 2) {
			if (eqstrn(locale, langs[i], 2)) {
				llstrapps(w32loc, sizeof(w32loc), uu8, langs[i+1]);
				break;
			}
		}
		if (!langs[i])
			return 0;
		ptr = locale+strlen(langs[i]);
		if (ptr[0]=='_') {
			llstrappc(w32loc, sizeof(w32loc), ptr[0]);
			for (i=0; countries[i]; i += 2) {
				if (eqstrn(ptr+1, countries[i], 2)) {
					llstrapps(w32loc, sizeof(w32loc), uu8, countries[i+1]);
					break;
				}
			}
		}
		/* TODO: strip off codeset, because we don't want user's codeset anyway,
		at least unless int_codeset == 0 */
		rtn = setlocale(category, w32loc);
	}
#else
	category=category; /* unused */
	locale=locale; /* unused */
#endif /* _WIN32 */
	return rtn;
}
/*==========================================
 * register_uilang_callback -- 
 *========================================*/
void
register_uilang_callback (CALLBACK_FNC fncptr, VPTR uparm)
{
	add_listener(&f_uilang_callbacks, fncptr, uparm);
}
/*==========================================
 * unregister_uilang_callback -- 
 *========================================*/
void
unregister_uilang_callback (CALLBACK_FNC fncptr, VPTR uparm)
{
	delete_listener(&f_uilang_callbacks, fncptr, uparm);
}
/*==========================================
 * send_uilang_callbacks -- 
 *========================================*/
#ifdef ENABLE_NLS
static void
send_uilang_callbacks (void)
{
	notify_listeners(&f_uilang_callbacks);
}
#endif /* ENABLE_NLS */
/*==========================================
 * register_uicodeset_callback -- 
 *========================================*/
void
register_uicodeset_callback (CALLBACK_FNC fncptr, VPTR uparm)
{
	add_listener(&f_uicodeset_callbacks, fncptr, uparm);
}
/*==========================================
 * unregister_uicodeset_callback -- 
 *========================================*/
void
unregister_uicodeset_callback (CALLBACK_FNC fncptr, VPTR uparm)
{
	delete_listener(&f_uicodeset_callbacks, fncptr, uparm);
}
/*==========================================
 * locales_notify_uicodeset_changes -- 
 *========================================*/
void
locales_notify_uicodeset_changes (void)
{
	notify_listeners(&f_uicodeset_callbacks);
}
/*==========================================
 * locales_notify_language_change -- notify gettext of change
 *========================================*/
void
locales_notify_language_change (void)
{
	notify_gettext_language_changed();
}

