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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/* modified 2000-08-22 J.F.Chandler */
/*=============================================================
 * valgdcom.c -- Validate GEDCOM file
 * Copyright(c) 1993-96 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93     2.3.5 - 02 Sep 93
 *   3.0.0 - 06 Jul 94     3.0.2 - 18 Feb 95
 *   3.0.3 - 29 Jun 96
 *===========================================================*/

#include <stdlib.h>
#include <stdarg.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <dir.h>
#include <io.h>
#endif
#include <ctype.h>
#include "standard.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcheck.h"
#include "liflines.h"

/* external data set by check_stdkeys() , used by addmissingkeys() */

INT gd_itot = 0;        /* total number of individuals */
INT gd_ftot = 0;        /* total number of families */
INT gd_stot = 0;        /* total number of sources */
INT gd_etot = 0;        /* total number of events */
INT gd_xtot = 0;        /* total number of others */
INT gd_imax = 0;        /* maximum individual key number */
INT gd_fmax = 0;        /* maximum family key number */
INT gd_smax = 0;        /* maximum source key number */
INT gd_emax = 0;        /* maximum event key number */
INT gd_xmax = 0;        /* maximum other key number */

static TABLE convtab = NULL;
static INT rec_type;
static BOOLEAN named = FALSE;
static INT person = -1;
static INT family = -1;
static INT event  = -1;
static INT source = -1;
static INT other  = -1;
static INT struct_len = 0;
static INT struct_max = 0;
static INT num_errors;
static INT num_warns;
static INT defline;
static BOOLEAN logopen;
static FILE *flog;

ELMNT *index_data = NULL;

#define SS (STRING)

STRING misixr = SS "Line %d: The person defined here has no key: skipped.";
STRING misfxr = SS "Line %d: The family defined here has no key.";
STRING misexr = SS "Line %d: The event defined here has no key.";
STRING missxr = SS "Line %d: The source defined here has no key.";
STRING misrxr = SS "Line %d: The record defined here has no key.";
STRING mulper = SS "Lines %d and %d: Person %s is multiply defined: skipped.";
STRING mulfam = SS "Lines %d and %d: Family %s is multiply defined.";
STRING mulsrc = SS "Lines %d and %d: Source %s is multiply defined.";
STRING mulevn = SS "Lines %d and %d: Event %s is multiply defined.";
STRING muloth = SS "Lines %d and %d: Record %s is multiply defined.";
STRING matper = SS "Line %d: Person %s has an incorrect key: skipped.";
STRING matfam = SS "Line %d: Family %s has an incorrect key.";
STRING matsrc = SS "Line %d: Source %s has an incorrect key.";
STRING matevn = SS "Line %d: Event %s has an incorrect key.";
STRING matoth = SS "Line %d: Record %s has an incorrect key.";
STRING misval = SS "Line %d: This %s line is missing a value field.";
STRING undper = SS "Person %s is referred to but not defined.";
STRING undfam = SS "Family %s is referred to but not defined.";
STRING undrec = SS "Record %s is referred to but not defined.";
STRING undsrc = SS "Source %s is referred to but not defined.";
STRING undevn = SS "Event %s is referred to but not defined.";
STRING badlev = SS "Line %d: This line has a level number that is too large.";
STRING noname = SS "Line %d: Person defined here has no name.";
STRING mulfth = SS "Line %d: Person %s has multiple father links.";
STRING mulmth = SS "Line %d: Person %s has multiple mother links.";
STRING mulhsb = SS "Line %d: Family %s has multiple husband links.";
STRING mulwif = SS "Line %d: Family %s has multiple wife links.";
STRING twosex = SS "Line %d: Person %s is both male and female.";
STRING mlefml = SS "Line %d: Person %s is male but must be female.";
STRING fmlmle = SS "Line %d: Person %s is female but must be male.";
STRING impsex = SS "Line %d: Person %s is implied to be both male and female.";
#if 0
STRING noxref = SS "Line %d: This record has no cross reference value.";
#endif

static INT add_even_defn(STRING, INT);
static INT add_fam_defn(STRING, INT);
static INT add_indi_defn(STRING, INT, ELMNT*);
static INT add_othr_defn(STRING, INT);
static INT add_sour_defn(STRING, INT);
static INT add_to_structures(STRING, ELMNT);
static void check_even_links(ELMNT);
static void check_fam_links(ELMNT);
static void check_indi_links(ELMNT);
static void check_othr_links(ELMNT);
static void check_references(void);
static void check_sour_links(ELMNT);
static void handle_fam_lev1(STRING, STRING, INT);
static void handle_indi_lev1(STRING, STRING, INT);
static void handle_value(STRING, INT);
static void clear_structures(void);
static void handle_warn(STRING, ...);
static void handle_err(STRING, ...);

/*===================================================
 * validate_gedcom -- Validate GEDCOM records in file
 *=================================================*/
BOOLEAN validate_gedcom (FILE *fp)
{
	INT lev, rc, curlev = 0;
	INT nindi, nfam, nsour, neven, nothr;
	extern INT lineno;
	ELMNT el;
	TRANTABLE tt = tran_tables[MGDIN];
	STRING xref, tag, val, msg;

	nindi = nfam = nsour = neven = nothr = 0;
	num_errors = num_warns = 0;
	logopen = FALSE;
	lineno = 0;
	defline = 0;
	curlev = 0;
	clear_structures();
	convtab = create_table();

	wfield(1, 1, "     0 Persons");
	wfield(2, 1, "     0 Families");
	wfield(3, 1, "     0 Events");
	wfield(4, 1, "     0 Sources");
	wfield(5, 1, "     0 Others");
	wfield(6, 1, "     0 Errors");
	wfield(7, 1, "     0 Warnings");

	rc = file_to_line(fp, tt, &lev, &xref, &tag, &val, &msg);
	xref = xref ? rmvat(xref) : NULL;
	rec_type = OTHR_REC;
	while (rc != DONE)  {
		if (lev > curlev + 1 || rc == ERROR) {
			if (rc == ERROR)
				handle_err(msg);
			else
				handle_err(badlev, lineno);
			handle_value(val, lineno);
			curlev = lev;
			rc = file_to_line(fp, tt, &lev, &xref, &tag, &val,
			    &msg);
			xref = xref ? rmvat(xref) : NULL;
			continue;
		}
		if (lev > 1) {
			handle_value(val, lineno);
			curlev = lev;
			rc = file_to_line(fp, tt, &lev, &xref, &tag, &val,
			    &msg);
			xref = xref ? rmvat(xref) : NULL;
			continue;
		}
		if (lev == 0) {
			char str[10];
			if (rec_type == INDI_REC && !named)
				handle_err(noname, defline);
			defline = lineno;
			if (eqstr("HEAD", tag) || eqstr("TRLR", tag))
				rec_type = IGNR_REC;
			else if (eqstr("INDI", tag)) {
				sprintf(str, "%6d", ++nindi);
				wfield(1, 1, str);
				rec_type = INDI_REC;
				named = FALSE;
				person = add_indi_defn(xref, lineno, &el);
				/* pretend a NAME if the record is skipped */
				if (person == -2) named = TRUE;
			} else if (eqstr("FAM", tag)) {
				sprintf(str, "%6d", ++nfam);
				wfield(2, 1, str);
				rec_type = FAM_REC;
				family = add_fam_defn(xref, lineno);
			} else if (eqstr("EVEN", tag)) {
				sprintf(str, "%6d", ++neven);
				wfield(3, 1, str);
				rec_type = EVEN_REC;
				event = add_even_defn(xref, lineno);
			} else if (eqstr("SOUR", tag)) {
				sprintf(str, "%6d", ++nsour);
				wfield(4, 1, str);
				rec_type = SOUR_REC;
				source = add_sour_defn(xref, lineno);
			} else {
				sprintf(str, "%6d", ++nothr);
				wfield(5, 1, str);
				rec_type = OTHR_REC;
				other = add_othr_defn(xref, lineno);
			}
		} else {
			if (rec_type == INDI_REC)
				handle_indi_lev1(tag, val, lineno);
			else if (rec_type == FAM_REC)
				handle_fam_lev1(tag, val, lineno);
			else
				handle_value(val, lineno);
		}
		curlev = lev;
		rc = file_to_line(fp, tt, &lev, &xref, &tag, &val, &msg);
		xref = xref ? rmvat(xref) : NULL;
	}
	if (rec_type == INDI_REC && !named)
		handle_err(noname, defline);
	check_references();
	if (logopen) {
		fclose(flog);
		logopen = FALSE;
	}
	return num_errors == 0;
}
/*=======================================
 * add_indi_defn -- Add person definition
 *  return index of person structure or
 *         -1 if unexplained error or
 *         -2 if explained error
 *=====================================*/
static INT add_indi_defn (xref, line, pel)
STRING xref;	/* ref value */
INT line;	/* line num */
ELMNT *pel;
{
	ELMNT el;
	INT dex;

	*pel = NULL;
	if (!xref || *xref == 0) {
		handle_err(misixr, line);
		return -2;
	}
	if ((dex = xref_to_index(xref)) == -1) {
		*pel = el = (ELMNT) stdalloc(sizeof(*el));
		Type(el) = INDI_REC;
		Key(el) = xref = strsave(xref);
		Line(el) = 0;
		New(el) = NULL;
		Sex(el) = 0;
		Male(el) = 0;
		Fmle(el) = 0;
		dex = add_to_structures(xref, el);
	} else
		*pel = el = index_data[dex];
	if (KNOWNTYPE(el) && Type(el) != INDI_REC) {
		handle_err(matper, line, xref);
		return -2;
	}
	if (Type(el) == INDI_REC) {
		if (Line(el) && line) {
			handle_err(mulper, Line(el), line, xref);
			return -2;
		}
		if (line) Line(el) = line;
		return dex;
	}
	Type(el) = INDI_REC;
	Line(el) = line;
	Sex(el) = 0;
	Male(el) = 0;
	Fmle(el) = 0;
	return dex;
}
/*======================================
 * add_fam_defn -- Add family definition
 *====================================*/
static INT add_fam_defn (xref, line)
STRING xref;	/* cross ref value */
INT line;	/* line num */
{
	ELMNT el;
	INT dex;
	if (!xref || *xref == 0) {
		handle_err(misfxr, line);
		return -1;
	}
	if ((dex = xref_to_index(xref)) == -1) {
		el = (ELMNT) stdalloc(sizeof(*el));
		Type(el) = FAM_REC;
		Key(el) = xref = strsave(xref);
		Line(el) = 0;
		New(el) = NULL;
		Male(el) = 0;
		Fmle(el) = 0;
		dex = add_to_structures(xref, el);
	} else
		el = index_data[dex];
	if (KNOWNTYPE(el) && Type(el) != FAM_REC) {
		handle_err(matfam, line, xref);
		return -1;
	}
	if (Type(el) == FAM_REC) {
		if (Line(el) && line) {
			handle_err(mulfam, Line(el), line, xref);
			return -1;
		}
		if (line) Line(el) = line;
		return dex;
	}
	Type(el) = FAM_REC;
	Line(el) = line;
	Male(el) = 0;
	Fmle(el) = 0;
	return dex;
}
/*=======================================
 * add_sour_defn -- Add source definition
 *=====================================*/
static INT add_sour_defn (xref, line)
STRING xref;	/* cross ref value */
INT line;	/* line num */
{
	ELMNT el;
	INT dex;
	if (!xref || *xref == 0) {
		handle_err(missxr, line);
		return -1;
	}
	if ((dex = xref_to_index(xref)) == -1) {
		el = (ELMNT) stdalloc(sizeof(*el));
		Type(el) = SOUR_REC;
		Key(el) = xref = strsave(xref);
		Line(el) = 0;
		New(el) = NULL;
		dex = add_to_structures(xref, el);
	} else
		el = index_data[dex];
	if (KNOWNTYPE(el) && Type(el) != SOUR_REC) {
		handle_err(matsrc, line, xref);
		return -1;
	}
	if (Type(el) == SOUR_REC) {
		if (Line(el) && line) {
			handle_err(mulsrc, Line(el), line, xref);
			return -1;
		}
		if (line) Line(el) = line;
		return dex;
	}
	Type(el) = SOUR_REC;
	Line(el) = line;
	return dex;
}
/*======================================
 * add_even_defn -- Add event definition
 *====================================*/
static INT add_even_defn (xref, line)
STRING xref;	/* cross ref value */
INT line;	/* line num */
{
	ELMNT el;
	INT dex;
	if (!xref || *xref == 0) {
		handle_err(misexr, line);
		return -1;
	}
	if ((dex = xref_to_index(xref)) == -1) {
		el = (ELMNT) stdalloc(sizeof(*el));
		Type(el) = EVEN_REC;
		Key(el) = xref = strsave(xref);
		Line(el) = 0;
		New(el) = NULL;
		dex = add_to_structures(xref, el);
	} else
		el = index_data[dex];
	if (KNOWNTYPE(el) && Type(el) != EVEN_REC) {
		handle_err(matevn, line, xref);
		return -1;
	}
	if (Type(el) == EVEN_REC) {
		if (Line(el) && line) {
			handle_err(mulevn, Line(el), line, xref);
			return -1;
		}
		if (line) Line(el) = line;
		return dex;
	}
	Type(el) = EVEN_REC;
	Line(el) = line;
	return dex;
}
/*==================================================
 * add_othr_defn -- Add other record type definition
 *================================================*/
static INT add_othr_defn (xref, line)
STRING xref;	/* cross ref value */
INT line;	/* line num */
{
	ELMNT el;
	INT dex;
	if (!xref || *xref == 0) {
		handle_err(misrxr, line);
		return -1;
	}
	if ((dex = xref_to_index(xref)) == -1) {
		el = (ELMNT) stdalloc(sizeof(*el));
		Type(el) = OTHR_REC;
		Key(el) = xref = strsave(xref);
		Line(el) = 0;
		New(el) = NULL;
		dex = add_to_structures(xref, el);
	} else
		el = index_data[dex];
	if (KNOWNTYPE(el) && Type(el) != OTHR_REC) {
		handle_err(matoth, line, xref);
		return -1;
	}
	if (Type(el) == OTHR_REC) {
		if (Line(el) && line) {
			handle_err(muloth, Line(el), line, xref);
			return -1;
		}
		if (line) Line(el) = line;
		return dex;
	}
	Type(el) = OTHR_REC;
	Line(el) = line;
	return dex;
}
/*===========================================================
 * handle_indi_lev1 -- Handle level 1 lines in person records
 *=========================================================*/
static void handle_indi_lev1 (tag, val, line)
STRING tag, val;
INT line;
{
	ELMNT indi, pers;
	ASSERT(person != -1);
	if (person == -2) {
		return;
	}
	indi = index_data[person];
	if (eqstr(tag, "FAMC")) {
		if (!pointer_value(val)) {
			handle_err(misval, line, "FAMC");
			return;
		}
		(void) add_fam_defn(rmvat(val), 0);
	} else if (eqstr(tag, "FAMS")) {
		if (!pointer_value(val)) {
			handle_err(misval, line, "FAMS");
			return;
		}
		(void) add_fam_defn(rmvat(val), 0);
	} else if (eqstr(tag, "FATH")) {
		if (!pointer_value(val)) {
			handle_warn(misval, line, "FATH");
			return;
		}
		Male(indi) += 1;
		if (add_indi_defn(rmvat(val), 0, &pers) >= 0)
			Sex(pers) |= BE_MALE;
	} else if (eqstr(tag, "MOTH")) {
		if (!pointer_value(val)) {
			handle_warn(misval, line, "MOTH");
			return;
		}
		Fmle(indi) += 1;
		if (add_indi_defn(rmvat(val), 0, &pers) >= 0)
			Sex(pers) |= BE_FEMALE;
	} else if (eqstr(tag, "SEX")) {
		if (val && (*val == 'M'))
			Sex(indi) |= IS_MALE;
		else if (val && (*val == 'F'))
			Sex(indi) |= IS_FEMALE;
	} else if (eqstr(tag, "NAME")) {
		named = TRUE;
		if (!val || *val == 0 || !valid_name(val)) {
			handle_err("Line %d: Bad NAME syntax.\n", line);
			return;
		}
	} else
		handle_value(val, line);
}
/*==========================================================
 * handle_fam_lev1 -- Handle level 1 lines in family records
 *========================================================*/
static void handle_fam_lev1 (tag, val, line)
STRING tag, val;
INT line;
{
	ELMNT fam, pers;
#ifdef DEBUG
	llwprintf("handle_fam_lev1: %s, %s, %d\n",tag,val,line);
	llwprintf("handle_fam_lev1: family == %d\n", family);
#endif
	fam = (family != -1) ? index_data[family] : NULL;
	if (eqstr(tag, "HUSB")) {
		if (!pointer_value(val)) {
			handle_err(misval, line, "HUSB");
			return;
		}
		if (fam) Male(fam) += 1;
		if (add_indi_defn(rmvat(val), 0, &pers) >= 0)
			Sex(pers) |= BE_MALE;
	} else if (eqstr(tag, "WIFE")) {
		if (!pointer_value(val)) {
			handle_err(misval, line, "WIFE");
			return;
		}
		if (fam) Fmle(fam) += 1;
		if (add_indi_defn(rmvat(val), 0, &pers) >= 0)
			Sex(pers) |= BE_FEMALE;
	} else if (eqstr(tag, "CHIL")) {
		if (!pointer_value(val)) {
			handle_err(misval, line, "CHIL");
			return;
		}
		(void) add_indi_defn(rmvat(val), 0, &pers);
	} else
		handle_value(val, line);
}
/*=================================================
 * check_akey -- Check for a standard format key
 *===============================================*/
int check_akey (firstchar, keyp, maxp)
    int firstchar;
    STRING keyp;
    INT *maxp;
{
    INT val;
    if(keyp && (*keyp == firstchar)) {
	keyp++;
	if(*keyp && isdigit(*keyp)) {
	    val = atoi(keyp);
	    if(val > *maxp) *maxp = val;
	    while(*keyp && isdigit(*keyp)) keyp++;
	    if(*keyp == '\0') return TRUE;
	}
    }
    return FALSE;
}
/*=================================================
 * check_stdkeys -- Check for standard format keys
 *===============================================*/
int check_stdkeys (void)
{
	INT i;
	int retval = TRUE;
	gd_imax = 0; gd_itot = 0;
	gd_fmax = 0; gd_ftot = 0;
	gd_emax = 0; gd_etot = 0;
	gd_smax = 0; gd_stot = 0;
	gd_xmax = 0; gd_xtot = 0;
	for (i = 0; retval && (i < struct_len); i++) {
		ELMNT el = index_data[i];
		switch (Type(el)) {
		case INDI_REC:
		    gd_itot++;
		    retval = check_akey('I', Key(el), &gd_imax);
		    break;
		case FAM_REC:
		    gd_ftot++;
		    retval = check_akey('F', Key(el), &gd_fmax);
		    break;
		case EVEN_REC:
		    gd_etot++;
		    retval = check_akey('E', Key(el), &gd_emax);
		    break;
		case SOUR_REC:
		    gd_stot++;
		    retval = check_akey('S', Key(el), &gd_smax);
		    break;
		case OTHR_REC:
		    gd_xtot++;
		    retval = check_akey('X', Key(el), &gd_xmax);
		    break;
		default: retval = FALSE; break;
		}
	}
	return retval;
}
/*================================================
 * addmissingkeys -- add keys which are not in use
 *==============================================*/
void addmissingkeys (INT t)		/* type of record: INDI_REC ... */
{
    	INT tmax, ttot;
	INT i,j;
	INT keystoadd;
	char *kp;

	switch(t)
	{
	case INDI_REC: ttot = gd_itot; tmax = gd_imax; break;
	case FAM_REC:  ttot = gd_ftot; tmax = gd_fmax; break;
	case EVEN_REC: ttot = gd_etot; tmax = gd_emax; break;
	case SOUR_REC: ttot = gd_stot; tmax = gd_smax; break;
	case OTHR_REC: ttot = gd_xtot; tmax = gd_xmax; break;
	default: return;
	}

	if((keystoadd = (tmax - ttot)) > 0) {
	    ASSERT(kp = (char *)stdalloc(tmax+1));
	    for(i = 0; i < tmax; i++) kp[i] = 0;
	    for (i = 0; i < struct_len; i++) {
		ELMNT el = index_data[i];
		if(Type(el) == t) {
		    j = atoi(Key(el)+1);
		    ASSERT((j>0) && (j <= tmax));
		    kp[j] = 1;
		}
	    }
	    for(i = 1; (keystoadd > 0) && (i <= tmax); i++) {
		if(kp[i] == 0) {
		    switch(t)
		    {
		    case INDI_REC: addixref(i); break;
		    case FAM_REC:  addfxref(i); break;
		    case EVEN_REC: addexref(i); break;
		    case SOUR_REC: addsxref(i); break;
		    case OTHR_REC: addxxref(i); break;
		    }
		    keystoadd--;
		}
	    }
	    stdfree(kp);
	}
}
/*=================================================
 * check_references -- Check for undefined problems
 *===============================================*/
static void check_references (void)
{
	INT i;
	for (i = 0; i < struct_len; i++) {
		ELMNT el = index_data[i];
		switch (Type(el)) {
		case INDI_REC: check_indi_links(el); break;
		case FAM_REC:  check_fam_links(el);  break;
		case EVEN_REC: check_even_links(el); break;
		case SOUR_REC: check_sour_links(el); break;
		case OTHR_REC: check_othr_links(el); break;
		default: handle_err(undrec, Key(el));
		}
	}
}
/*=======================================
 * check_indi_links -- Check person links
 *=====================================*/
static void check_indi_links (per)
ELMNT per;
{
	BOOLEAN jm, jf, bm, bf;
	if (Male(per) > 1)  handle_warn(mulfth, Line(per), Key(per));
	if (Fmle(per) > 1)  handle_warn(mulmth, Line(per), Key(per));
	if (Line(per) == 0) {
		handle_err(undper, Key(per));
		return;
	}
	jm = Sex(per) & IS_MALE;
	jf = Sex(per) & IS_FEMALE;
	bm = Sex(per) & BE_MALE;
	bf = Sex(per) & BE_FEMALE;
	if (jm && jf) handle_err(twosex, Line(per), Key(per));
	if (jm && bf) handle_err(mlefml, Line(per), Key(per));
	if (jf && bm) handle_err(fmlmle, Line(per), Key(per));
	if (bm && bf) handle_err(impsex, Line(per), Key(per));
}
/*======================================
 * check_fam_links -- Check family links
 *====================================*/
static void check_fam_links (fam)
ELMNT fam;
{
	if (Line(fam) == 0) handle_err(undfam, Key(fam));
	if (Male(fam) > 1)  handle_warn(mulhsb, Line(fam), Key(fam));
	if (Fmle(fam) > 1)  handle_warn(mulwif, Line(fam), Key(fam));
}
/*======================================
 * check_even_links -- Check event links
 *====================================*/
static void check_even_links (evn)
ELMNT evn;
{
	if (Line(evn) == 0) handle_err(undevn, Key(evn));
}
/*=======================================
 * check_sour_links -- Check source links
 *=====================================*/
static void check_sour_links (src)
ELMNT src;
{
	if (Line(src) == 0) handle_err(undsrc, Key(src));
}
/*======================================
 * check_othr_links -- Check other links
 *====================================*/
static void check_othr_links (otr)
ELMNT otr;
{
	if (Line(otr) == 0) handle_err(undrec, Key(otr));
}
/*=======================================
 * handle_value -- Handle arbitrary value
 *=====================================*/
static void handle_value (val, line)
STRING val;
INT line;
{
	ELMNT el;
	STRING xref;

	if (rec_type == IGNR_REC) return;
	if (!pointer_value(val)) return;
	xref = rmvat(val);
	if (xref_to_index(xref) != -1) return;
	el = (ELMNT) stdalloc(sizeof(*el));
	Type(el) = UNKN_REC;
	Key(el) = xref = strsave(xref);
	New(el) = NULL;
	Line(el) = line;
	add_to_structures(xref, el);
}
/*=========================================
 * pointer_value -- See if value is pointer
 *=======================================*/
BOOLEAN pointer_value (val)
STRING val;
{
	if (!val || *val != '@' || strlen(val) < 3) return FALSE;
	return val[strlen(val)-1] == '@';
}
/*==================================
 * handle_err -- Handle GEDCOM error
 *================================*/
void handle_err (STRING fmt, ...)
{
	va_list args;
	char str[100];

	if (!logopen) {

		unlink("err.log");

		ASSERT(flog = fopen("err.log", LLWRITETEXT));
		logopen = TRUE;
	}
	fprintf(flog, "Error: ");
	va_start(args, fmt);
	vfprintf(flog, fmt, args);
	va_end(args);
	fprintf(flog, "\n");
	sprintf(str, "%6d Errors (see log file `err.log')", ++num_errors);
	wfield(6, 1, str);
}
/*=====================================
 * handle_warn -- Handle GEDCOM warning
 *===================================*/
void handle_warn (STRING fmt, ...)
{
	va_list args;
	char str[100];
	if (!logopen) {
		unlink("err.log");
		ASSERT(flog = fopen("err.log", LLWRITETEXT));
		logopen = TRUE;
	}
	fprintf(flog, "Warning: ");
	va_start(args, fmt);
	vfprintf(flog, fmt, args);
	va_end(args);
	fprintf(flog, "\n");
	sprintf(str, "%6d Warnings (see log file `err.log')", ++num_warns);
	wfield(7, 1, str);
}
/*=========================================
 * xref_to_index - Convert pointer to index
 *=======================================*/
INT xref_to_index (xref)
STRING xref;
{
	BOOLEAN there;
	INT dex = (INT)valueofbool(convtab, xref, &there);
	return there ? dex : -1;
}
/*=========================================================
 * add_to_structures -- Add new elements to data structures
 *=======================================================*/
static INT add_to_structures(xref, el)
STRING xref;
ELMNT el;
{
	INT i, n;

	if ((n = struct_len) >= struct_max)  {
		ELMNT *newi = (ELMNT *) stdalloc(sizeof(ELMNT)*
		    (n + 10000));
		for (i = 0; i < n; i++)
			newi[i] = index_data[i];
		if (n) stdfree(index_data);
		index_data = newi;
		struct_max += 10000;
	}
	i = struct_len;
	index_data[i] = el;
	insert_table(convtab, xref, (WORD) struct_len++);
	return struct_len - 1;
}
/*========================================================
 * clear_structures -- Clear GEDCOM import data structures
 *======================================================*/
static void clear_structures (void)
{
	INT i;

	for (i = 0; i < struct_len; i++)
		stdfree(index_data[i]);
	struct_len = 0;
	if (convtab) {
		remove_table(convtab, DONTFREE);
		convtab = NULL;
	}
}
