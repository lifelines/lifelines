/*
   Copyright (c) 2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * intlshim.c -- Shim to connect to gettext dll if available
 * Windows specific
 *   Created: 2002/06 by Perry Rapp
 *   Edited:  2002/11/20 (Perry Rapp)
 *==============================================================*/

#include "intlshim.h"
#include <windows.h>
#include <stdio.h>

#ifndef INTLDECL
#define INTLDECL
#endif

#define INTLSHIM_VERSION "1.1.1"


static FARPROC MyGetProcAddress(HMODULE hModule, LPCSTR lpProcName);
static int ishim_get_dll_name(char * filepath, int pathlen);
static int ishim_get_file_version(const char * filepath, char * verout, int veroutlen);
static int ishim_set_dll_name(const char *filepath);
static int load_dll(void);
static void unload_dll(void);

static HINSTANCE f_hinstDll=0;
static int f_failed=0;
static char f_dllpath[MAX_PATH]="";
static char * f_defaults[] = { "gettext.dll", "libintl.dll", "intl.dll" };


typedef char * (*gettext_type)(const char *m);
typedef char * (*dgettext_type)(const char *dom, const char *m);
typedef char * (*dcgettext_type)(const char *dom, const char *m, int cat);
typedef char * (*ngettext_type)(const char *__msgid1, const char *__msgid2, unsigned long int __n);
typedef char * (*dngettext_type)(const char *dom, const char *m1, const char *m2, unsigned long int n);
typedef char * (*dcngettext_type)(const char *__domainname, const char *m1, const char *m2, unsigned long int n, int cat);
typedef char * (*textdomain_type)(const char *dom);
typedef char * (*bindtextdomain_type)(const char *dom, const char *dir);
typedef char * (*bind_textdomain_codeset_type)(const char *dom, const char *cs);
typedef int (*gt_notify_language_change_type)(void);
typedef int (*gt_get_property_type)(const char *name, char *value, int valuelen);
typedef int (*gt_set_property_type)(const char *name, const char *value);

static struct gettext_fncs_s
{
	gettext_type gettext_x;
	dgettext_type dgettext_x;
	dcgettext_type dcgettext_x;
	ngettext_type ngettext_x;
	dngettext_type dngettext_x;
	dcngettext_type dcngettext_x;
	textdomain_type textdomain_x;
	bindtextdomain_type bindtextdomain_x;
	bind_textdomain_codeset_type bind_textdomain_codeset_x;
	gt_notify_language_change_type gt_notify_language_change_x;
	gt_get_property_type gt_get_property_x;
	gt_set_property_type gt_set_property_x;
} f_gettext_fncs;


/* Look up MSGID in the current default message catalog for the current
   LC_MESSAGES locale.  If not found, returns MSGID itself (the default
   text).  */
INTLDECL char *
gettext(const char *__msgid)
{
	if (!load_dll() || !f_gettext_fncs.gettext_x)
		return (char *)__msgid;
	return (*f_gettext_fncs.gettext_x)(__msgid);
}

/* Look up MSGID in the DOMAINNAME message catalog for the current
   LC_MESSAGES locale.  */
INTLDECL char *
dgettext(const char *__domainname, 
				     const char *__msgid)
{
	if (!load_dll() || !f_gettext_fncs.dgettext_x)
		return (char *)__msgid;
	return (*f_gettext_fncs.dgettext_x)(__domainname, __msgid);
}

/* Look up MSGID in the DOMAINNAME message catalog for the current CATEGORY
   locale.  */
INTLDECL char *
dcgettext(const char *__domainname,
				      const char *__msgid,
				      int __category)
{
	if (!load_dll() || !f_gettext_fncs.dcgettext_x)
		return (char *)__msgid;
	return (*f_gettext_fncs.dcgettext_x)(__domainname, __msgid, __category);
}

/* Similar to `gettext' but select the plural form corresponding to the
   number N.  */
INTLDECL char *
ngettext(const char *__msgid1,
				     const char *__msgid2,
				     unsigned long int __n)
{
	if (!load_dll() || !f_gettext_fncs.ngettext_x)
		return __n>1 ? (char *)__msgid2 : (char *)__msgid1;
	return (*f_gettext_fncs.ngettext_x)(__msgid1, __msgid2, __n);
}

/* Similar to `dgettext' but select the plural form corresponding to the
   number N.  */
INTLDECL char *
dngettext(const char *__domainname,
				      const char *__msgid1,
				      const char *__msgid2,
				      unsigned long int __n)
{
	if (!load_dll() || !f_gettext_fncs.dngettext_x)
		return __n>1 ? (char *)__msgid2 : (char *)__msgid1;
	return (*f_gettext_fncs.dngettext_x)(__domainname, __msgid1, __msgid2, __n);
}

/* Similar to `dcgettext' but select the plural form corresponding to the
   number N.  */
INTLDECL char *
dcngettext(const char *__domainname,
				       const char *__msgid1,
				       const char *__msgid2,
				       unsigned long int __n,
				       int __category)
{
	if (!load_dll() || !f_gettext_fncs.dcngettext_x)
		return __n>1 ? (char *)__msgid2 : (char *)__msgid1;
	return (*f_gettext_fncs.dcngettext_x)(__domainname, __msgid1, __msgid2, __n, __category);
}

/* Set the current default message catalog to DOMAINNAME.
   If DOMAINNAME is null, return the current default.
   If DOMAINNAME is "", reset to the default of "messages".  */
INTLDECL char *
textdomain(const char *__domainname)
{
	if (!load_dll() || !f_gettext_fncs.textdomain_x)
		return 0;
	return (*f_gettext_fncs.textdomain_x)(__domainname);
}

/* Specify that the DOMAINNAME message catalog will be found
   in DIRNAME rather than in the system locale data base.  */
INTLDECL char *
bindtextdomain(const char *__domainname,
					   const char *__dirname)
{
	if (!load_dll() || !f_gettext_fncs.bindtextdomain_x)
		return 0;
	return (*f_gettext_fncs.bindtextdomain_x)(__domainname, __dirname);
}

/* Specify the character encoding in which the messages from the
   DOMAINNAME message catalog will be returned.  */
INTLDECL char *
bind_textdomain_codeset(const char *__domainname,
						    const char *__codeset)
{
	if (!load_dll() || !f_gettext_fncs.bind_textdomain_codeset_x)
		return 0;
	return (*f_gettext_fncs.bind_textdomain_codeset_x)(__domainname, __codeset);
}

/* increment nl_msg_cat_cntr in gettext */
INTLDECL void
gt_notify_language_change (void)
{
	/*
	TODO: Need to find out how to directly bump counter
	in case this is someone else's gettext.dll
	*/
	if (!load_dll() || !f_gettext_fncs.gt_notify_language_change_x)
		return;
	(*f_gettext_fncs.gt_notify_language_change_x)();
}

/* pass some get_property call to gettext */
INTLDECL int
gt_get_property (const char *name, char *value, int valuelen)
{
	if (!load_dll() || !f_gettext_fncs.gt_get_property_x)
		return 0;
	return (*f_gettext_fncs.gt_get_property_x)(name, value, valuelen);
}

/* pass some set_property call to gettext */
INTLDECL int
gt_set_property (const char *name, const char *value)
{
	if (!load_dll() || !f_gettext_fncs.gt_set_property_x)
		return 0;
	return (*f_gettext_fncs.gt_set_property_x)(name, value);
}

static void
unload_dll (void)
{
	if (!f_hinstDll)
		return;
	memset(&f_gettext_fncs, 0, sizeof(f_gettext_fncs));
	FreeLibrary(f_hinstDll);
	f_hinstDll = 0;
}

static FARPROC
MyGetProcAddress (HMODULE hModule, LPCSTR lpProcName)
{
	/* TODO: Add property for client to set prefix */
	const char * prefix ="lib";
	char buffer[256];
	FARPROC proc = GetProcAddress(hModule, lpProcName);
	if (proc)
		return proc;
	if (lstrlen(lpProcName)+lstrlen(prefix)+1>sizeof(buffer))
		return 0;
	lstrcpy(buffer, prefix);
	lstrcat(buffer, lpProcName);
	proc = GetProcAddress(hModule, lpProcName);
	if (proc)
		return proc;
	return 0;
}

static int
load_dll (void)
{
	if (f_hinstDll)
		return 1;
	if (f_failed)
		return 0;
	f_failed=1;

	memset(&f_gettext_fncs, 0, sizeof(f_gettext_fncs));

	if (!f_dllpath[0]) 
	{
		/* no requested path, try defaults */
		int i;
		for (i=0; i<sizeof(f_defaults)/sizeof(f_defaults[0]); ++i) 
		{
			if ((f_hinstDll = LoadLibrary(f_defaults[i]))!=NULL)
			{
				strncpy(f_dllpath, f_defaults[i], sizeof(f_dllpath));
				break;
			}
		}
	} 
	else 
	{
		f_hinstDll = LoadLibrary(f_dllpath);
	}
	if (!f_hinstDll)
		return 0;

	f_gettext_fncs.gettext_x = (gettext_type)MyGetProcAddress(f_hinstDll, "gettext");
	f_gettext_fncs.dgettext_x = (dgettext_type)MyGetProcAddress(f_hinstDll, "dgettext");
	f_gettext_fncs.dcgettext_x = (dcgettext_type)MyGetProcAddress(f_hinstDll, "dcgettext");
	f_gettext_fncs.ngettext_x = (ngettext_type)MyGetProcAddress(f_hinstDll, "ngettext");
	f_gettext_fncs.dngettext_x = (dngettext_type)MyGetProcAddress(f_hinstDll, "dngettext");
	f_gettext_fncs.dcngettext_x = (dcngettext_type)MyGetProcAddress(f_hinstDll, "dcngettext");
	f_gettext_fncs.textdomain_x = (textdomain_type)MyGetProcAddress(f_hinstDll, "textdomain");
	f_gettext_fncs.bindtextdomain_x = (bindtextdomain_type)MyGetProcAddress(f_hinstDll, "bindtextdomain");
	f_gettext_fncs.bind_textdomain_codeset_x = (bind_textdomain_codeset_type)MyGetProcAddress(f_hinstDll, "bind_textdomain_codeset");
	f_gettext_fncs.gt_notify_language_change_x = (gt_notify_language_change_type)MyGetProcAddress(f_hinstDll, "gt_notify_language_change");
	f_gettext_fncs.gt_get_property_x = (gt_get_property_type)MyGetProcAddress(f_hinstDll, "gt_get_property");
	f_gettext_fncs.gt_set_property_x = (gt_set_property_type)MyGetProcAddress(f_hinstDll, "gt_set_property");
	
	f_failed=0;
	return 1;
}

INTLDECL int
intlshim_get_property (const char *name, char * value, int valuelen)
{
	static char file_version[] = "file_version:";
	if (!lstrcmp(name, "dll_path"))
		return ishim_get_dll_name(value, valuelen);
	if (!lstrcmp(name, "dll_version"))
	{
		char path[_MAX_PATH];
		if (!ishim_get_dll_name(path, sizeof(path)))
			return 0;
		return ishim_get_file_version(path, value, valuelen);
	}
	if (!lstrcmp(name, "shim_version"))
	{
		lstrcpyn(value, INTLSHIM_VERSION, valuelen);
		return lstrlen(INTLSHIM_VERSION);
	}
	if (!strncmp(name, file_version, strlen(file_version)))
	{
		return ishim_get_file_version(name+strlen(file_version), value, valuelen);
	}
	return 0;
}

INTLDECL int
intlshim_set_property (const char *name, const char *value)
{
	if (!strcmp(name, "dll_path"))
	{
		return ishim_set_dll_name(value);
	}
	return 0;
}


static int
ishim_get_dll_name (char * filepath, int pathlen)
{
	if (!f_hinstDll)
		return 0;
	if (!GetModuleFileName(f_hinstDll, filepath, pathlen))
		return 0;
	return 1;
}

static int
ishim_get_file_version (const char * filepath, char * verout, int veroutlen)
{
	DWORD dwDummyHandle, len;
	BYTE * buf = 0;
	unsigned int verlen;
	LPVOID lpvi;
	VS_FIXEDFILEINFO fileInfo;

	if (!filepath || !filepath[0]) return 0;

	len = GetFileVersionInfoSize((char *)filepath, &dwDummyHandle);
	if (!len) return 0;
	buf = (BYTE *)malloc(len * sizeof(BYTE));
	if (!buf) return 0;
	GetFileVersionInfo((char *)filepath, 0, len, buf);
	VerQueryValue(buf, "\\", &lpvi, &verlen);
	fileInfo = *(VS_FIXEDFILEINFO*)lpvi;
	_snprintf(verout, veroutlen, "FV:%d.%d.%d.%d, PV:%d.%d.%d.%d"
		, HIWORD(fileInfo.dwFileVersionMS)
		, LOWORD(fileInfo.dwFileVersionMS)
		, HIWORD(fileInfo.dwFileVersionLS)
		, LOWORD(fileInfo.dwFileVersionLS)
		, HIWORD(fileInfo.dwProductVersionMS)
		, LOWORD(fileInfo.dwProductVersionMS)
		, HIWORD(fileInfo.dwProductVersionLS)
		, LOWORD(fileInfo.dwProductVersionLS));
	free(buf);
	return len;
}

static int
ishim_set_dll_name (const char *filepath)
{
	if (!filepath)
		filepath = "";
	lstrcpyn(f_dllpath, filepath, sizeof(f_dllpath));

	unload_dll();
	f_failed=0; /* force attempt to load */
	return load_dll();
}
