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

/*********************************************
 * local types
 *********************************************/

/* This will go into translat.c when new system is working */
struct xlat_s {
	/* All members either NULL or heap-alloc'd */
	STRING name;
	STRING src;
	STRING dest;
	LIST steps;
	BOOLEAN adhoc;
	BOOLEAN valid;
};
/* dynamically loadable translation table, entry in dyntt list */
typedef struct dyntt_s {
	STRING path;
	TRANTABLE tt; /* when loaded */
	BOOLEAN loadfailure;
} *DYNTT;

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
static INT check_tt_name(CNSTRING filename, ZSTR * pzsrc, ZSTR * pzdest);
static XLSTEP create_iconv_step(CNSTRING src, CNSTRING dest);
static XLSTEP create_dyntt_step(DYNTT dyntt);
static XLAT create_null_xlat(void);
static XLAT create_xlat(CNSTRING src, CNSTRING dest, BOOLEAN adhoc);
static DYNTT create_dyntt(TRANTABLE tt, STRING path);
static void free_dyntts(void);
static void free_xlat(XLAT xlat);
static DYNTT get_conversion_dyntt(CNSTRING src, CNSTRING dest);
static DYNTT get_subcoding_dyntt(CNSTRING codeset, CNSTRING subcoding);
static void load_dyntt_if_needed(DYNTT dyntt);
static void load_dynttlist_from_dir(STRING dir);
static int select_tts(const struct dirent *entry);
static void zero_dyntt(DYNTT dyntt);

/*********************************************
 * local variables
 *********************************************/

static LIST f_xlats=0; /* cache of conversions */
static TABLE f_dyntts=0; /* cache of dynamic translation tables */
static char f_ttext[] = ".tt";

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==========================================================
 * create_xlat -- Create a new translation
 * (also adds to cache)
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
static XLAT
create_null_xlat (void)
{
	/* create & initialize new xlat */
	XLAT xlat = (XLAT)malloc(sizeof(*xlat));
	memset(xlat, 0, sizeof(*xlat));
	xlat->steps = create_list();
	xlat->valid = TRUE;
	set_list_type(xlat->steps, LISTDOFREE);
	if (!f_xlats) {
		f_xlats = create_list();
	}
	enqueue_list(f_xlats, xlat);
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
	XLAT xlat = create_null_xlat();
	xlat->src = strsave(src);
	xlat->dest = strsave(dest);
	xlat->adhoc = adhoc;
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
	make_list_empty(xlat->steps);
	remove_list(xlat->steps, 0);
}
/*==========================================================
 * create_iconv_step -- Create an iconv step of a translation chain
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
static XLSTEP
create_iconv_step (CNSTRING src, CNSTRING dest)
{
	XLSTEP xstep;
	xstep = (XLSTEP )malloc(sizeof(*xstep));
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
	xstep = (XLSTEP )malloc(sizeof(*xstep));
	memset(xstep, 0, sizeof(*xstep));
	xstep->dyntt = dyntt;
	return xstep;
}
/*==========================================================
 * create_dyntt -- Create record for dynamcially loading TRANTABLE
 * Created: 2002/12/10 (Perry Rapp)
 *========================================================*/
static DYNTT
create_dyntt (TRANTABLE tt, STRING path)
{
	DYNTT dyntt = (DYNTT)malloc(sizeof(*dyntt));
	memset(dyntt, 0, sizeof(*dyntt));
	dyntt->tt = tt;
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
	ZSTR zsrc=0, zdest=0;
	LIST srcsubs=0, destsubs=0;
	STRING subcoding=0;
	
	if (!src || !src[0] || !dest || !dest[0]) {
		return create_null_xlat();
	}

	/* first check existing cache */
	if (f_xlats) {
		XLAT xlattemp;
		FORLIST(f_xlats, el)
			xlattemp = (XLAT)el;
			if (eqstr_ex(xlattemp->src, src) && eqstr_ex(xlattemp->dest, dest)) {
				return xlattemp;
			}
		ENDLIST
	}
	/* check if identity */
	if (eqstr(src, dest)) {
		/* new empty xlat will work for identity */
		return create_xlat(src, dest, adhoc);
	}

	/* create new xlat & fill it out */
	xlat = create_xlat(src, dest, adhoc);

	/* parse out codesets & subcodings */
	xl_parse_codeset(src, &zsrc, &srcsubs);
	xl_parse_codeset(dest, &zdest, &destsubs);

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
		XLSTEP xstep = create_iconv_step(src, dest);
		enqueue_list(xlat->steps, xstep);
	} else {
		DYNTT dyntt = get_conversion_dyntt(zs_str(zsrc), zs_str(zdest));
		if (dyntt)
			add_dyntt_step(xlat, dyntt);
		else
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
	return create_null_xlat();
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
	zs_appc(&zttname, '_');
	zs_apps(&zttname, dest);
	dyntt = (DYNTT)valueof_ptr(f_dyntts, zs_str(zttname));
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
	zs_apps(&zttname, "__");
	zs_apps(&zttname, subcoding);
	dyntt = (DYNTT)valueof_ptr(f_dyntts, zs_str(zttname));
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
	ZSTR zerr=0;
	if (dyntt->tt || dyntt->loadfailure)
		return;
	if (!init_map_from_file(dyntt->path, dyntt->path, &dyntt->tt, &zerr)) {
		dyntt->loadfailure = TRUE;
	}
}
/*==========================================================
 * xl_do_xlat -- Perform a translation on a string
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
BOOLEAN
xl_do_xlat (XLAT xlat, ZSTR * pzstr)
{
	BOOLEAN cvtd=FALSE;
	XLSTEP xstep=0;
	if (!xlat || !xlat->valid) return cvtd;
	/* simply cycle through & perform each step */
	FORLIST(xlat->steps, el)
		xstep = (XLSTEP)el;
		if (xstep->iconv_src) {
			/* an iconv step */
			ZSTR ztemp=0;
			if (iconv_trans(xstep->iconv_src, xstep->iconv_dest, zs_str(*pzstr), &ztemp, "?")) {
				cvtd=TRUE;
				zs_free(pzstr);
				*pzstr = ztemp;
			} else {
				/* iconv failed, anything to do ? */
			}
		} else if (xstep->dyntt) {
			/* a custom translation table step */
			if (xstep->dyntt->tt)
				custom_translate(pzstr, xstep->dyntt->tt);
		}
	ENDLIST
	return cvtd;
}
/*==========================================================
 * xl_load_all_tts -- Load internal list of available translation
 *  tables (based on *.tt files in TTPATH)
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
void
xl_load_all_dyntts (CNSTRING ttpath)
{
	STRING dirs,p;
	free_dyntts();
	if (!ttpath ||  !ttpath[0])
		return;
	f_dyntts = create_table();
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
	for (i=0; i<n; ++i) {
		CNSTRING ttfile = programs[i]->d_name;
		/* filename without extension */
		ZSTR zfile = zs_newsubs(ttfile, strlen(ttfile)-(sizeof(f_ttext)-1));
		ZSTR zsrc=0, zdest=0;
		INT ntype = check_tt_name(zs_str(zfile), &zsrc, &zdest);
		/*
		Valid names are like so:
			UTF-8_ISO-8859-1 (type 1; code conversion)
			UTF-8__HTML (type 2; subcoding)
		*/
		if (ntype==1 || ntype==2) {
			if (!valueof_ptr(f_dyntts, zs_str(zfile))) {
				TRANTABLE tt=0; /* will be loaded when needed */
				STRING path = concat_path_alloc(dir, ttfile);
				DYNTT dyntt = create_dyntt(tt, path);
				strfree(&path);
				insert_table_ptr(f_dyntts, strsave(zs_str(zfile)), dyntt);
			}
		}
		zs_free(&zfile);
		zs_free(&zsrc);
		zs_free(&zdest);
	}
	if (n>0)
		stdfree(programs);
}
/*===========================================================
 * check_tt_name -- Parse tt filename to see what codesets it does
 *  returns 1 if codeset-codeset conversion
 *  returns 2 if codeset subcoding conversion
 *  returns 0 if not valid name for tt
 *==========================================================*/
static INT
check_tt_name (CNSTRING filename, ZSTR * pzsrc, ZSTR * pzdest)
{
	CNSTRING ptr;
	CNSTRING underbar=0;
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
	*pzsrc = zs_newsubs(filename, underbar-filename);
	if (underbar[1]=='_') {
		zs_sets(pzdest, underbar+2);
		return 2;
	} else {
		zs_sets(pzdest, underbar+1);
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
    XLAT xlattemp;
	LIST newlist;
	if (!f_xlats)
		return;
	/* we don't have a way to delete items from a list,
	so just copy the ones we want to a new list */
	newlist = create_list();
	set_list_type(newlist, LISTDOFREE);
	FORLIST(f_xlats, el)
		xlattemp = (XLAT)el;
		if (xlattemp->adhoc) {
			free_xlat(xlattemp);
		} else {
			back_list(newlist, xlattemp);
		}
	ENDLIST
	make_list_empty(f_xlats);
	remove_list(f_xlats, 0);
	f_xlats = newlist;
}
/*==========================================================
 * free_xlats -- Free all the translation chains
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
void
xl_free_xlats (void)
{
	XLAT xlattemp;
	if (!f_xlats)
		return;
	FORLIST(f_xlats, el)
		xlattemp = (XLAT)el;
		free_xlat(xlattemp);
	ENDLIST
	make_list_empty(f_xlats);
	remove_list(f_xlats, 0);
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
		struct table_iter_s tabit;
		xl_free_xlats(); /* xlats point into f_dyntts */
		if (begin_table(f_dyntts, &tabit)) {
			VPTR ptr;
			STRING key;
			while (next_table_ptr(&tabit, &key, &ptr)) {
				DYNTT dyntt = (DYNTT)ptr;
				zero_dyntt((DYNTT)ptr);
			}
		}
		remove_table(f_dyntts, FREEBOTH);
		f_dyntts = 0;
	}
}
/*==========================================================
 * zero_dyntt -- Empty (free) contents of a dyntt
 *  NB: This does not free the dyntt memory itself
 *  b/c that is handled in the f_dyntts cache table
 * Created: 2002/12/13 (Perry Rapp)
 *========================================================*/
static void
zero_dyntt (DYNTT dyntt)
{
	strfree(&dyntt->path);
	remove_trantable(dyntt->tt);
	dyntt->tt = 0;
}
/*==========================================================
 * xl_parse_codeset -- Parse out subcode suffixes of a codeset
 *  eg, "CP437//TrGreekAscii//TrCyrillicAscii"
 *  will recognize CP437 as the codeset name, and list
 *  "TrGreekAscii" and "TrCyrillicAscii"  as subcodes
 * Created: 2002/12/01 (Perry Rapp)
 *========================================================*/
void
xl_parse_codeset (CNSTRING codeset, ZSTR * zcsname, LIST * subcodes)
{
	CNSTRING p=codeset, prev=codeset;
	BOOLEAN base=FALSE;
	for ( ; ; ++p) {
		if ( !p[0] || (p[0]=='/' && p[1]=='/')) {
			if (!base) {
				if (zcsname) {
					zs_free(zcsname);
					*zcsname = zs_newsubs(codeset, p-prev);
				}
				base=TRUE;
			} else {
				ZSTR ztemp=0;
				if (subcodes) {
					if (!*subcodes) {
						*subcodes = create_list();
						set_list_type(*subcodes, LISTDOFREE);
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
 * xl_set_name -- Store English name in xlat
 *  This is just for debugging convenience
 * Created: 2002/12/10 (Perry Rapp)
 *========================================================*/
void
xl_set_name (XLAT xlat, CNSTRING name)
{
	strupdate(&xlat->name, name);
}
/*==========================================================
 * xlat_get_description -- Fetch description of a translation
 *  eg, "3 steps with iconv(UTF-8, CP1252)"
 * Created: 2002/12/13 (Perry Rapp)
 *========================================================*/
ZSTR
xlat_get_description (XLAT xlat)
{
	INT count=0, iconv_count=0;
	ZSTR zrtn=0;  /* final string to return */
	ZSTR zstr=0; /* string with details of iconv conversions */
	char stepcount[32];
	XLSTEP xstep=0;
	if (!xlat || !xlat->steps) return zs_news(_("(no conversion)"));
	/* simply cycle through & perform each step */
	FORLIST(xlat->steps, el)
		xstep = (XLSTEP)el;
		++count;
		if (xstep->iconv_src) {
			/* an iconv step */
			if (zs_len(zstr)) zs_apps(&zstr, ", ");
			zs_apps(&zstr, "iconv(");
			zs_apps(&zstr, xstep->iconv_src);
			zs_apps(&zstr, ",");
			zs_apps(&zstr, xstep->iconv_dest);
			zs_apps(&zstr, ")");
		} else if (xstep->dyntt) {
			if (zs_len(zstr)) zs_apps(&zstr, ", ");
			zs_apps(&zstr, "tt(");
			zs_apps(&zstr, tt_get_name(xstep->dyntt->tt));
			zs_apps(&zstr, ")");
		}
	ENDLIST
	snprintf(stepcount, sizeof(stepcount), _pl("%d step", "%d steps", count), count);
	zs_sets(&zrtn, stepcount);
	zs_apps(&zrtn, ": ");
	zs_appz(&zrtn, zstr);
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
