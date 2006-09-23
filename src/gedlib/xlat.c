/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*===========================================================
 * xlat.c -- Implementation of translation module
 * xlat module converts between named codesets
 * It uses iconv & charmaps as steps in a chain, as needed
 *=========================================================*/

#include "llstdlib.h"
#include "translat.h"
#include "gedcom.h"
#include "zstr.h"
#include "icvt.h"
#include "gedcomi.h"
#include "xlat.h" 
#include "arch.h"
#include "lloptions.h"
#include "log.h"
#include "vtable.h"


/*********************************************
 * local types
 *********************************************/

/* This will go into translat.c when new system is working */
struct tag_xlat {
	/* All members either NULL or heap-alloc'd */
	STRING src;
	STRING dest;
	LIST steps;
	BOOLEAN adhoc;
	BOOLEAN valid;
	INT uparam; /* opaque number used by client */
};
/* dynamically loadable translation table, entry in dyntt list */
struct tag_dyntt {
	struct tag_vtable *vtable; /* generic object */
	INT refcnt; /* ref-countable object */
	STRING name;
	STRING path;
	TRANTABLE tt; /* when loaded */
	BOOLEAN loadfailure;
};
typedef struct tag_dyntt *DYNTT;

/* step of a translation, either iconv_src or trantble is NULL */
typedef struct xlat_step_s {
	STRING iconv_src;
	STRING iconv_dest;
	DYNTT dyntt;
} *XLSTEP;


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void add_dyntt_step(XLAT xlat, DYNTT dyntt);
static INT check_tt_name(CNSTRING filename, ZSTR zsrc, ZSTR zdest);
static XLSTEP create_iconv_step(CNSTRING src, CNSTRING dest);
static XLSTEP create_dyntt_step(DYNTT dyntt);
static XLAT create_null_xlat(BOOLEAN adhoc);
static XLAT create_xlat(CNSTRING src, CNSTRING dest, BOOLEAN adhoc);
static DYNTT create_dyntt(TRANTABLE tt, CNSTRING name, CNSTRING path);
static void destroy_dyntt(DYNTT dyntt);
static void dyntt_destructor(VTABLE *obj);
static void free_dyntts(void);
static void free_xlat(XLAT xlat);
static DYNTT get_conversion_dyntt(CNSTRING src, CNSTRING dest);
static DYNTT get_subcoding_dyntt(CNSTRING codeset, CNSTRING subcoding);
static void load_dyntt_if_needed(DYNTT dyntt);
static void load_dynttlist_from_dir(STRING dir);
static int select_tts(const struct dirent *entry);

/*********************************************
 * local variables
 *********************************************/

static LIST f_xlats=0; /* cache of conversions */
static TABLE f_dyntts=0; /* cache of dynamic translation tables */
static char f_ttext[] = ".tt";

static struct tag_vtable vtable_for_dyntt = {
	VTABLE_MAGIC
	, "dyntt"
	, &dyntt_destructor
	, &refcountable_isref
	, &refcountable_addref
	, &refcountable_release
	, 0 /* copy_fnc */
	, &generic_get_type_name
};

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==========================================================
 * create_null_xlat -- Create a new translation
 * (also adds to cache)
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
static XLAT
create_null_xlat (BOOLEAN adhoc)
{
	/* create & initialize new xlat */
	XLAT xlat = (XLAT)stdalloc(sizeof(*xlat));
	memset(xlat, 0, sizeof(*xlat));
	xlat->steps = create_list2(LISTDOFREE);
	xlat->valid = TRUE;
	if (!f_xlats) {
		f_xlats = create_list();
	}
	enqueue_list(f_xlats, xlat);
	xlat->adhoc = adhoc;
	return xlat;
}
/*==========================================================
 * create_xlat -- Create a new translation
 * (also adds to cache)
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
static XLAT
create_xlat (CNSTRING src, CNSTRING dest, BOOLEAN adhoc)
{
	/* create & initialize new xlat */
	XLAT xlat = create_null_xlat(adhoc);
	xlat->src = strsave(src);
	xlat->dest = strsave(dest);
	return xlat;
}
/*==========================================================
 * free_xlat -- Free a translation
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
static void
free_xlat (XLAT xlat)
{
	XLSTEP xstep=0;
	/* free each step */
	FORLIST(xlat->steps, el)
		xstep = (XLSTEP)el;
		strfree(&xstep->iconv_src);
		strfree(&xstep->iconv_dest);
		xstep->dyntt = 0; /* f_dyntts owns dyntt memory */
	ENDLIST
	destroy_list(xlat->steps);
	strfree(&xlat->src);
	strfree(&xlat->dest);
	stdfree(xlat);
}
/*==========================================================
 * create_iconv_step -- Create an iconv step of a translation chain
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
static XLSTEP
create_iconv_step (CNSTRING src, CNSTRING dest)
{
	XLSTEP xstep;
	xstep = (XLSTEP) stdalloc(sizeof(*xstep));
	memset(xstep, 0, sizeof(*xstep));
	xstep->iconv_dest = strsave(dest);
	xstep->iconv_src = strsave(src);
	return xstep;
}
/*==========================================================
 * create_dyntt_step -- Create a tt step of a translation chain
 * Created: 2002/12/01 (Perry Rapp)
 *========================================================*/
static XLSTEP
create_dyntt_step (DYNTT dyntt)
{
	XLSTEP xstep;
	xstep = (XLSTEP) stdalloc(sizeof(*xstep));
	memset(xstep, 0, sizeof(*xstep));
	xstep->dyntt = dyntt;
	return xstep;
}
/*==========================================================
 * create_dyntt -- Create record for dynamcially loading TRANTABLE
 * Created: 2002/12/10 (Perry Rapp)
 *========================================================*/
static DYNTT
create_dyntt (TRANTABLE tt, CNSTRING name, CNSTRING path)
{
	DYNTT dyntt = (DYNTT)stdalloc(sizeof(*dyntt));
	memset(dyntt, 0, sizeof(*dyntt));
	dyntt->vtable = &vtable_for_dyntt;
	dyntt->refcnt = 1;
	dyntt->tt = tt;
	dyntt->name = strsave(name);
	dyntt->path = strsave(path);
	return dyntt;
}
/*==========================================================
 * xl_get_xlat -- Find translation between specified codesets
 *  returns NULL if fails
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
XLAT
xl_get_xlat (CNSTRING src, CNSTRING dest, BOOLEAN adhoc)
{
	XLAT xlat=0;
	ZSTR zsrc=zs_new(), zdest=zs_new();
	ZSTR zsrc_u=ll_toupperz(src,0),zdest_u=ll_toupperz(dest,0);
	LIST srcsubs=0, destsubs=0;
	STRING subcoding=0;
	
	if (!src || !src[0] || !dest || !dest[0]) {
		xlat = create_null_xlat(adhoc);
		goto end_get_xlat;
	}

	/* first check existing cache */
	/* (only adhoc xlats can use the cache) */
	if (adhoc && f_xlats) {
		XLAT xlattemp;
		FORLIST(f_xlats, el)
			xlattemp = (XLAT)el;
			if (xlattemp->adhoc 
				&& eqstr_ex(xlattemp->src, zs_str(zsrc_u))
				&& eqstr_ex(xlattemp->dest, zs_str(zdest_u))
				) {
				xlat = xlattemp;
				STOPLIST
				goto end_get_xlat;
			}
		ENDLIST
	}
	/* create new xlat & fill it out */
	xlat = create_xlat(zs_str(zsrc_u), zs_str(zdest_u), adhoc);

	/* check if identity */
	if (eqstr(zs_str(zsrc_u), zs_str(zdest_u))) {
		/* new empty xlat will work for identity */
		goto end_get_xlat;
	}

	/* parse out codesets & subcodings */
	xl_parse_codeset(zs_str(zsrc_u), zsrc, &srcsubs);
	xl_parse_codeset(zs_str(zdest_u), zdest, &destsubs);

	/* in source codeset, do subcodings requested by destination */
	/* eg, if going from UTF-8 to GUI, transliterations done in UTF-8 first */
	if (destsubs) {
		FORLIST(destsubs, el)
			subcoding = (STRING)el;
			add_dyntt_step(xlat
				, get_subcoding_dyntt(zs_str(zsrc), subcoding));
		ENDLIST
	}

	/* do main codeset conversion, prefering iconv if available */
	if (eqstr(zs_str(zsrc), zs_str(zdest))) {
		/* main conversion is identity */
	} else if (iconv_can_trans(zs_str(zsrc), zs_str(zdest))) {
		XLSTEP xstep = create_iconv_step(zs_str(zsrc), zs_str(zdest));
		enqueue_list(xlat->steps, xstep);
	} else {
		STRING src = zs_str(zsrc), dest = zs_str(zdest);
		BOOLEAN foundit = FALSE;
		DYNTT dyntt = get_conversion_dyntt(src, dest);
		if (dyntt) {
			add_dyntt_step(xlat, dyntt);
			foundit = TRUE;
		}
		else if (!eqstr(src, "UTF-8") && !eqstr(dest, "UTF-8")) {
			/* try going through UTF-8 as intermediate step */
			DYNTT dyntt1 = get_conversion_dyntt(src, "UTF-8");
			DYNTT dyntt2 = get_conversion_dyntt("UTF-8", dest);
			if (dyntt1 && dyntt2) {
				add_dyntt_step(xlat, dyntt1);
				add_dyntt_step(xlat, dyntt2);
				foundit = TRUE;
			}
		}
		if (!foundit)
			xlat->valid = FALSE; /* missing main conversion */
	}

	/* in destination codeset, do subcodings requested by source */
	/* eg, if going from GUI into UTF-8, transliterations undone in UTF-8 last */
	if (srcsubs) {
		FORLIST(srcsubs, el)
			subcoding = (STRING)el;
			add_dyntt_step(xlat
				, get_subcoding_dyntt(zs_str(zdest), subcoding));
		ENDLIST
	}

end_get_xlat:
	zs_free(&zsrc);
	zs_free(&zdest);
	zs_free(&zsrc_u);
	zs_free(&zdest_u);
	destroy_list(srcsubs);
	destroy_list(destsubs);
	return xlat;
}
/*==========================================================
 * xl_get_null_xlat -- Get placeholder translation
 *  (These are used for special purposes like custom sort table)
 * Created: 2002/12/13 (Perry Rapp)
 *========================================================*/
XLAT
xl_get_null_xlat (void)
{
	return create_null_xlat(FALSE);
}
/*==========================================================
 * add_dyntt_step -- Add dynamic translation table step
 *  to end of translation chain (xlat)
 * Created: 2002/12/10 (Perry Rapp)
 *========================================================*/
static void
add_dyntt_step (XLAT xlat, DYNTT dyntt)
{
	if (!dyntt) return;
	load_dyntt_if_needed(dyntt);
	if (!dyntt->loadfailure) {
		XLSTEP xstep = create_dyntt_step(dyntt);
		enqueue_list(xlat->steps, xstep);
	}
}
/*==========================================================
 * get_conversion_dyntt -- get dyntt entry for code conversion, if in dyntt list
 * Created: 2002/12/01 (Perry Rapp)
 *========================================================*/
static DYNTT
get_conversion_dyntt (CNSTRING src, CNSTRING dest)
{
	DYNTT dyntt;
	ZSTR zttname = zs_news(src);
	zs_appc(zttname, '_');
	zs_apps(zttname, dest);
	dyntt = (DYNTT)valueof_obj(f_dyntts, zs_str(zttname));
	if (getlloptint("TTPATH.debug", 0)) {
		log_outf("ttpath.dbg",
			_("ttpath get_conversion_dyntt:from <%s> to <%s>: %s"),
			src, dest,
			dyntt ? _("succeeded") : _("failed")
			);
	}
	zs_free(&zttname);
	return dyntt;
}
/*==========================================================
 * get_subcoding_dyntt -- get dyntt entry for subcoding, if in dyntt list
 * Created: 2002/12/01 (Perry Rapp)
 *========================================================*/
static DYNTT
get_subcoding_dyntt (CNSTRING codeset, CNSTRING subcoding)
{
	DYNTT dyntt;
	ZSTR zttname = zs_news(codeset);
	zs_apps(zttname, "__");
	zs_apps(zttname, subcoding);
	dyntt = (DYNTT)valueof_obj(f_dyntts, zs_str(zttname));
	if (getlloptint("TTPATH.debug", 0)) {
		log_outf("ttpath.dbg",
			_("ttpath get_subcoding_dyntt from <%s> to subcode <%s>: %s"),
			codeset,subcoding,
			dyntt ? _("succeeded") : _("failed")
			);
	}
	zs_free(&zttname);
	return dyntt;
}
/*==========================================================
 * load_dyntt_if_needed -- load dyntt from disk (unless already loaded)
 * Created: 2002/12/01 (Perry Rapp)
 *========================================================*/
static void
load_dyntt_if_needed (DYNTT dyntt)
{
	ZSTR zerr=zs_new();
	if (dyntt->tt || dyntt->loadfailure)
		return;
	if (!init_map_from_file(dyntt->path, dyntt->name, &dyntt->tt, zerr)) {
		dyntt->loadfailure = TRUE;
	}
	zs_free(&zerr);
}
/*==========================================================
 * xl_do_xlat -- Perform a translation on a string
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
BOOLEAN
xl_do_xlat (XLAT xlat, ZSTR zstr)
{
	BOOLEAN cvtd=FALSE;
	XLSTEP xstep=0;
	if (!xlat || !xlat->valid) return cvtd;
	/* simply cycle through & perform each step */
	FORLIST(xlat->steps, el)
		xstep = (XLSTEP)el;
		if (xstep->iconv_src) {
			/* an iconv step */
			ZSTR ztemp=zs_new();
			if (iconv_trans(xstep->iconv_src, xstep->iconv_dest, zs_str(zstr), ztemp, '?')) {
				cvtd=TRUE;
				zs_move(zstr, &ztemp);
			} else {
				zs_free(&ztemp);
				/* iconv failed, anything to do ? */
			}
		} else if (xstep->dyntt) {
			/* a custom translation table step */
			if (xstep->dyntt->tt)
				custom_translatez(zstr, xstep->dyntt->tt);
		}
	ENDLIST
	return cvtd;
}
/*==========================================================
 * xl_load_all_dyntts -- Load internal list of available translation
 *  tables (based on *.tt files in TTPATH)
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
void
xl_load_all_dyntts (CNSTRING ttpath)
{
	STRING dirs,p;
	free_dyntts();
	if (getlloptint("TTPATH.debug", 0)) {
		if (!ttpath ||  !ttpath[0])
			log_outf("ttpath.dbg", _("No TTPATH config variable"));
		else
			log_outf("ttpath.dbg", "ttpath: %s", ttpath);
	}
	if (!ttpath ||  !ttpath[0])
		return;
	f_dyntts = create_table_obj();
	dirs = (STRING)stdalloc(strlen(ttpath)+2);
	/* find directories in dirs & delimit with zeros */
	chop_path(ttpath, dirs);
	/* now process each directory */
	for (p=dirs; *p; p+=strlen(p)+1) {
		load_dynttlist_from_dir(p);
	}
	strfree(&dirs);

}
/*==========================================================
 * load_dynttlist_from_dir -- Get list of all translation tables in 
 *  specified directory
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
static void
load_dynttlist_from_dir (STRING dir)
{
	struct dirent **programs;
	INT n = scandir(dir, &programs, select_tts, alphasort);
	INT i;
	if (getlloptint("TTPATH.debug", 0)) {
		log_outf("ttpath.dbg", _("ttpath checking dir <%s>"), dir);
	}
	for (i=0; i<n; ++i) {
		CNSTRING ttfile = programs[i]->d_name;
		/* filename without extension */
		ZSTR zfile = zs_newsubs(ttfile, strlen(ttfile)-(sizeof(f_ttext)-1));
		ZSTR zsrc=zs_new(), zdest=zs_new();
		INT ntype = check_tt_name(zs_str(zfile), zsrc, zdest);
		/*
		Valid names are like so:
			UTF-8_ISO-8859-1 (type 1; code conversion)
			UTF-8__HTML (type 2; subcoding)
		*/
		if (getlloptint("TTPATH.debug", 0)) {
			log_outf("ttpath.dbg", _("ttpath file <%s> typed as %d"), ttfile, ntype);
		}
		if (ntype==1 || ntype==2) {
			ZSTR zfile_u = ll_toupperz(zs_str(zfile),0);
			if (!valueof_obj(f_dyntts, zs_str(zfile_u))) {
				TRANTABLE tt=0; /* will be loaded when needed */
				STRING path = concat_path_alloc(dir, ttfile);
				DYNTT dyntt = create_dyntt(tt, ttfile, path);
				strfree(&path);
				insert_table_obj(f_dyntts, zs_str(zfile_u), dyntt);
				zs_free(&zfile_u);
				--dyntt->refcnt; /* leave table as sole owner */
			}
		}
		zs_free(&zfile);
		zs_free(&zsrc);
		zs_free(&zdest);
	}
	if (n>0) {
		for (i=0; i<n; ++i) {
			stdfree(programs[i]);
		}
		stdfree(programs);
	}
}
/*===========================================================
 * check_tt_name -- Parse tt filename to see what codesets it does
 *  returns 1 if codeset-codeset conversion
 *  returns 2 if codeset subcoding conversion
 *  returns 0 if not valid name for tt
 *  we leave case of filename alone, but zsrc and zdest are made uppercase
 *==========================================================*/
static INT
check_tt_name (CNSTRING filename, ZSTR zsrc, ZSTR zdest)
{
	CNSTRING ptr;
	CNSTRING underbar=0;
	ZSTR ztemp;
	for (ptr=filename; *ptr; ++ptr) {
		if (*ptr=='_') {
			if (!underbar) {
				underbar = ptr;
			} else {
				if (underbar+1 != ptr)
					return 0;
			}
		} else if (*ptr==' ') {
			return 0;
		}
	}
	if (!underbar || underbar==filename ||
		!underbar[1] || (underbar[1]=='_' && !underbar[2])) {
		return 0;
	}
	ztemp = zs_newsubs(filename, underbar-filename);
	zsrc = ll_toupperz(zs_str(ztemp),0);
	zs_free(&ztemp);
	if (underbar[1]=='_') {
		zdest = ll_toupperz(underbar+2,0);
		return 2;
	} else {
		zdest = ll_toupperz(underbar+1,0);
		return 1;
	}
}
/*===========================================================
 * select_tts -- choose translation tables
 *==========================================================*/
static int
select_tts (const struct dirent *entry)
{
	INT tlen = strlen(entry->d_name);
	CNSTRING entext;
	if (tlen < (INT)sizeof(f_ttext)) return 0;

	/* examine end of entry->d_name */
	entext = entry->d_name + tlen - (sizeof(f_ttext)-1);

	/* is it what we want ? use platform correct comparison, from path.c */
	if (!path_match(f_ttext, entext))
		return 0;

	return 1;
}
/*==========================================================
 * free_adhoc_xlats -- Free all the adhoc translation chains
 *  (ie, the ones used during report processing)
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
void
xl_free_adhoc_xlats (void)
{
    XLAT xlattemp=0;
	LIST newlist=0;
	if (!f_xlats)
		return;
	/* we don't have a way to delete items from a list,
	so just copy the ones we want to a new list */
	newlist = create_list();
	FORLIST(f_xlats, el)
		xlattemp = (XLAT)el;
		if (xlattemp->adhoc) {
			free_xlat(xlattemp);
		} else {
			back_list(newlist, xlattemp);
		}
	ENDLIST
	destroy_list(f_xlats);
	f_xlats = newlist;
}
/*==========================================================
 * free_xlats -- Free all the translation chains
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
void
xl_free_xlats (void)
{
	XLAT xlattemp=0;
	if (!f_xlats)
		return;
	FORLIST(f_xlats, el)
		xlattemp = (XLAT)el;
		free_xlat(xlattemp);
	ENDLIST
	destroy_list(f_xlats);
	f_xlats = 0;
}
/*==========================================================
 * free_dyntts -- Free table of dynamic translation tables
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
static void
free_dyntts (void)
{
	if (f_dyntts) {
		destroy_table(f_dyntts);
		f_dyntts = 0;
	}
}
/*==========================================================
 * xlat_shutdown -- Clear and free allocated objects
 *========================================================*/
void
xlat_shutdown (void)
{
	xl_free_xlats();
	free_dyntts();
}
/*=================================================
 * destroy_table -- destroy all element & memory for table
 *===============================================*/
static void
destroy_dyntt (DYNTT dyntt)
{
	if (!dyntt) return;
	ASSERT(dyntt->vtable == &vtable_for_dyntt);
	strfree(&dyntt->name);
	strfree(&dyntt->path);
	remove_trantable(dyntt->tt);
	dyntt->tt = 0;
	stdfree(dyntt);
}
/*==========================================================
 * xl_parse_codeset -- Parse out subcode suffixes of a codeset
 *  eg, "CP437//TrGreekAscii//TrCyrillicAscii"
 *  will recognize CP437 as the codeset name, and list
 *  "TrGreekAscii" and "TrCyrillicAscii"  as subcodes
 * Created: 2002/12/01 (Perry Rapp)
 *========================================================*/
void
xl_parse_codeset (CNSTRING codeset, ZSTR zcsname, LIST * subcodes)
{
	CNSTRING p=codeset, prev=codeset;
	BOOLEAN base=FALSE;
	for ( ; ; ++p) {
		if ( !p[0] || (p[0]=='/' && p[1]=='/')) {
			if (!base) {
				ZSTR ztemp = zs_newsubs(codeset, p-prev);
				zs_move(zcsname, &ztemp);
				base=TRUE;
			} else {
				if (subcodes) {
					ZSTR ztemp=0;
					if (!*subcodes) {
						*subcodes = create_list2(LISTDOFREE);
					}
					ztemp = zs_newsubs(prev, p-prev);
					enqueue_list(*subcodes, strsave(zs_str(ztemp)));
					zs_free(&ztemp);
				}
			}
			if (!p[0])
				return;
			prev = p+2;
			p = p+1; /* so we jump over both slashes */
		}
	}
}
/*==========================================================
 * xlat_get_description -- Fetch description of a translation
 *  eg, "3 steps with iconv(UTF-8, CP1252)"
 * Created: 2002/12/13 (Perry Rapp)
 *========================================================*/
ZSTR
xlat_get_description (XLAT xlat)
{
	INT count=0;
	ZSTR zrtn=zs_new();  /* final string to return */
	ZSTR zstr=zs_new(); /* string with details of iconv conversions */
	char stepcount[32];
	XLSTEP xstep=0;
	if (!xlat || !xlat->steps) {
		zs_sets(zrtn, _("(no conversion)"));
		goto end_get_desc;
	}
	/* simply cycle through & perform each step */
	FORLIST(xlat->steps, el)
		xstep = (XLSTEP)el;
		++count;
		if (xstep->iconv_src) {
			/* an iconv step */
			if (zs_len(zstr)) zs_apps(zstr, ", ");
			zs_apps(zstr, "iconv(");
			zs_apps(zstr, xstep->iconv_src);
			zs_apps(zstr, ",");
			zs_apps(zstr, xstep->iconv_dest);
			zs_apps(zstr, ")");
		} else if (xstep->dyntt) {
			if (zs_len(zstr)) zs_apps(zstr, ", ");
			zs_apps(zstr, "tt(");
			zs_apps(zstr, tt_get_name(xstep->dyntt->tt));
			zs_apps(zstr, ")");
		}
	ENDLIST
	/* TRANSLATORS: steps in a chain of codeset conversions, eg, Editor-to-Internal */
	snprintf(stepcount, sizeof(stepcount), _pl("%d step", "%d steps", count), count);
	zs_sets(zrtn, stepcount);
	zs_apps(zrtn, ": ");
	zs_appz(zrtn, zstr);

end_get_desc:
	zs_free(&zstr);
	return zrtn;
}
/*==========================================================
 * xl_is_xlat_valid -- Does it do the job ?
 * Created: 2002/12/15 (Perry Rapp)
 *========================================================*/
BOOLEAN
xl_is_xlat_valid (XLAT xlat)
{
	return xlat->valid;
}
/*==========================================================
 * xl_release_xlat -- Client finished with this
 * Created: 2002/12/15 (Perry Rapp)
 *========================================================*/
void
xl_release_xlat (XLAT xlat)
{
	xlat=xlat; /* unused */
	/*
	TODO: If it is an adhoc xlat, free it
	Have to remove it from cache list, which is slightly annoying
	(we have no way to remove an item from a list, so we have
	to recopy the list)
	*/
}
void
xl_set_uparam (XLAT xlat, INT uparam)
{
	xlat->uparam = uparam;
}
INT
xl_get_uparam (XLAT xlat)
{
	return xlat->uparam;
}
/*==========================================================
 * xl_get_dest_codeset -- return name of codeset of destination
 *========================================================*/
CNSTRING
xl_get_dest_codeset (XLAT xlat)
{
	return xlat->dest;
}
/*=================================================
 * dyntt_destructor -- destructor for dyntt
 *  (destructor entry in vtable)
 *===============================================*/
static void
dyntt_destructor (VTABLE *obj)
{
	DYNTT tab = (DYNTT)obj;
	ASSERT(tab->vtable == &vtable_for_dyntt);
	destroy_dyntt(tab);
}
