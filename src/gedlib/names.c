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
/*=============================================================
 * names.c -- Handle name values and name indexing
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 02 Sep 93
 *   3.0.0 - 13 Sep 94    3.0.2 - 31 Dec 94
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "btree.h"
#include "translat.h"
#include "gedcom.h"

extern BOOLEAN opt_finnish;
extern BTREE BTR;

RKEY name2rkey();
char *getasurname();

static int old = 0;
static INT codeof(int);
static STRING parts_to_name(STRING*);
static void name_to_parts(STRING, STRING*);
static void squeeze(STRING, STRING);
static STRING nextpiece(STRING);
static void cmpsqueeze(STRING, STRING);

/*===================================================================
 * name records -- Name indexing information is kept in the database
 *   in name records; all persons with the same SOUNDEX code and the
 *   same first letter in their first given name, are indexed
 *   together
 *===================================================================
 * database record format -- The first INT of the record holds the
 *   number of names indexed in the record
 *-------------------------------------------------------------------
 *        1 INT  nnames  - number of names indexed in this record
 *   nnames RKEY rkeys   - RKEYs of the INDI records with the names
 *   nnames INT  noffs   - offsets into following strings where names
 *			   begin
 *   nnames STRING names - char buffer where the names are stored
 *			   based on char offsets
 *-------------------------------------------------------------------
 * internal format -- At any time there can be only one name record
 *   stored internally; the data is stored in global data structures
 *-------------------------------------------------------------------
 *   RKEY    NRkey   - RKEY of the current name record
 *   STRING  NRrec   - current name record
 *   INT     NRsize  - size of current name record
 *   INT     NRcount - number of entries in current name record
 *   INT    *NRoffs  - char offsets to names in current name
 *			  record
 *   RKEY   *NRkeys  - RKEYs of the INDI records with the names
 *   STRING *NRnames - name values from INDI records that the
 *			  index is based upon
 *   INT     NRmax   - max allocation size of internal arrays
 *-------------------------------------------------------------------
 * When a name record is used to match a search name, the internal
 *   structures are modified to remove all entries that don't match
 *   the name; in addition, other global data structures are used
 *-------------------------------------------------------------------
 *   STRING *LMkeys  - keys (strings) of all INDI records that match
 *   INT     LMcount - number of entries in modified record arrays
 *   INT     LMmax   - max allocation size of LMkeys array
 *=================================================================*/

static RKEY    NRkey;
static STRING  NRrec = NULL;
static INT     NRsize;
static INT     NRcount;
static INT    *NRoffs;
static RKEY   *NRkeys;
static STRING *NRnames;
static INT     NRmax = 0;

static STRING *LMkeys = NULL;
static INT     LMcount = 0;
static INT     LMmax = 0;

/*====================================================
 * getnamerec -- Read name record and store in globals
 *==================================================*/
BOOLEAN getnamerec (name)
STRING name;
{
	STRING p;
	INT i;

/* Convert name to key and read name record */
	NRkey = name2rkey(name);
	if (NRrec) stdfree(NRrec);
	p = NRrec = (STRING) getrecord(BTR, NRkey, &NRsize);
	if (!NRrec) {
		NRcount = 0;
		if (NRmax == 0) {
			NRmax = 10;
			NRkeys = (RKEY *) stdalloc(10*sizeof(RKEY));
			NRoffs = (INT *) stdalloc(10*sizeof(INT));
			NRnames = (STRING *) stdalloc(10*sizeof(STRING));
		}
		return FALSE;
	}

/* Store name record in data structures */
	memcpy (&NRcount, p, sizeof(INT));
	p += sizeof(INT);
	if (NRcount >= NRmax - 1) {
		if (NRmax != 0) {
			stdfree(NRkeys);
			stdfree(NRoffs);
			stdfree(NRnames);
		}
		NRmax = NRcount + 10;
		NRkeys = (RKEY *) stdalloc((NRmax)*sizeof(RKEY));
		NRoffs = (INT *) stdalloc((NRmax)*sizeof(INT));
		NRnames = (STRING *) stdalloc((NRmax)*sizeof(STRING));
	}
	for (i = 0; i < NRcount; i++) {
		memcpy(&NRkeys[i], p, sizeof(RKEY));
		p += sizeof(RKEY);
	}
	for (i = 0; i < NRcount; i++) {
		memcpy(&NRoffs[i], p, sizeof(INT));
		p += sizeof(INT);
	}
	for (i = 0; i < NRcount; i++)
		NRnames[i] = p + NRoffs[i];
	return TRUE;
}
/*============================================
 * name2rkey - Convert name to name record key
 *==========================================*/
RKEY name2rkey (name)
STRING name;
{
	RKEY rkey;
	STRING sdex = soundex(getsurname(name));
	char finitial = getfinitial(name);
	rkey.r_rkey[0] = rkey.r_rkey[1] = ' ';
	rkey.r_rkey[2] = 'N';
	rkey.r_rkey[3] = finitial;
	rkey.r_rkey[4] = *sdex++;
	rkey.r_rkey[5] = *sdex++;
	rkey.r_rkey[6] = *sdex++;
	rkey.r_rkey[7] = *sdex;
	return rkey;
}
/*=============================
 * getsurname -- Return surname
 *===========================*/
STRING getsurname (name)
STRING name;	/* GEDCOM name */
{
	INT c;
	static unsigned char buffer[3][MAXLINELEN+1];
	static INT dex = 0;
	STRING p, surname;
	if (++dex > 2) dex = 0;
	p = surname = buffer[dex];
	while ((c = *name++) && c != '/')
		;
	if (c == 0) return (STRING) "____";
	while (iswhite(c = *name++))
		;
	if (c == 0 || c == '/' || !isletter(c)) return (STRING) "____";
	*p++ = c;
	while ((c = *name++) && c != '/')
		*p++ = c;
	*p = 0;
	return surname;
}
/*============================================
 * getfinitial -- Return first initial of name
 *==========================================*/
INT getfinitial (name)
STRING name;	/* GEDCOM name */
{
	INT c;
	while (TRUE) {
		while (iswhite(c = *name++))
			;
		if (isletter(c)) return ll_toupper(c);
		if (c == 0) return '$';
		if (c != '/') return '$';
		while ((c = *name++) && c != '/')
			;
		if (c == 0) return '$';
	}
}
/*==================================================================
 * soundex -- Return SOUNDEX code of name; any case; return Z999 for
 *   problem names
 *================================================================*/
STRING soundex (name)
STRING name;	/* surname */
{
	static unsigned char scratch[MAXNAMELEN+2];
	STRING p = name, q = scratch;
	INT c, i, j;
	if (!name || strlen(name) > MAXNAMELEN || eqstr(name, "____"))
		return (STRING) "Z999";
	p = name;
	q = scratch;
	while ((c = *p++))
		*q++ = ll_toupper(c);
	*q = 0;
	p = q = &scratch[1];
	i = 1;
	old = 0;
	while ((c = *p++) && i < 4) {
		if ((j = codeof(c)) == 0) continue;
		*q++ = j;
		i++;
	}
	while (i < 4) {
		*q++ = '0';
		i++;
	}
	*q = 0;
	return scratch;
}
/*========================================
 * codeof -- Return letter's SOUNDEX code.
 *======================================*/
static INT codeof (letter)
int letter;
{
	int new = 0;

    if(opt_finnish) {
	/* Finnish Language */
	switch (letter) {
	case 'B': case 'P': case 'F': case 'V': case 'W':
		new = '1'; break;
	case 'C': case 'S': case 'K': case 'G': case '\337':
	case 'J': case 'Q': case 'X': case 'Z': case '\307':
		new = '2'; break;
	case 'D': case 'T': case '\320': case '\336':
		new = '3'; break;
	case 'L':
		new = '4'; break;
	case 'M': case 'N': case '\321':
		new = '5'; break;
	case 'R':
		new = '6'; break;
	default:	/* new stays zero */
		break;
	}
    } else {
	/* English Language (Default) */
	switch (letter) {
	case 'B': case 'P': case 'F': case 'V':
		new = '1'; break;
	case 'C': case 'S': case 'K': case 'G':
	case 'J': case 'Q': case 'X': case 'Z':
		new = '2'; break;
	case 'D': case 'T':
		new = '3'; break;
	case 'L':
		new = '4'; break;
	case 'M': case 'N':
		new = '5'; break;
	case 'R':
		new = '6'; break;
	default:	/* new stays zero */
		break;
	}
    }
  
	if (new == 0) {
		old = 0;
		return 0;
	}
	if (new == old) return 0;
	old = new;
	return new;
}
/*=========================================
 * add_name -- Add new entry to name record
 *=======================================*/
BOOLEAN add_name (name, key)
STRING name;	/* person's name */
STRING key;	/* person's INDI key */
{
	STRING rec, p;
	INT i, len, off;
	RKEY rkey;
	rkey = str2rkey(key);
	(void) getnamerec(name);
	for (i = 0; i < NRcount; i++) {
		if (!ll_strncmp(rkey.r_rkey, NRkeys[i].r_rkey, 8) &&
		    eqstr(name, NRnames[i]))
			return TRUE;
	}
	NRkeys[NRcount] = rkey;
	NRnames[NRcount] = name;
	NRcount++;
	p = rec = (STRING) stdalloc(NRsize + sizeof(RKEY) +
	    sizeof(INT) + strlen(name) + 10);
	len = 0;
	memcpy(p, &NRcount, sizeof(INT));
	p += sizeof(INT);
	len += sizeof(INT);
	for (i = 0; i < NRcount; i++) {
		memcpy(p, &NRkeys[i], sizeof(RKEY));
		p += sizeof(RKEY);
		len += sizeof(RKEY);
	}
	off = 0;
	for (i = 0; i < NRcount; i++) {
		memcpy(p, &off, sizeof(INT));
		p += sizeof(INT);
		len += sizeof(INT);
		off += strlen(NRnames[i]) + 1;
	}
	for (i = 0; i < NRcount; i++) {
		memcpy(p, NRnames[i], strlen(NRnames[i]) + 1);
		p += strlen(NRnames[i]) + 1;
		len += strlen(NRnames[i]) + 1;
	}
	addrecord(BTR, NRkey, rec, len);
	stdfree(rec);
	return TRUE;
}
/*=============================================
 * remove_name -- Remove entry from name record
 *===========================================*/
BOOLEAN remove_name (name, key)
STRING name;	/* preson's name */
STRING key;	/* person's INDI key */
{
	STRING rec, p;
	INT i, len, off;
	BOOLEAN found;
	RKEY rkey;
	rkey = str2rkey(key);
	(void) getnamerec(name);
	found = FALSE;
	for (i = 0; i < NRcount; i++) {
		if (!ll_strncmp(rkey.r_rkey, NRkeys[i].r_rkey, 8) &&
		    eqstr(name, NRnames[i])) {
			found = TRUE;
			break;
		}
	}
	if (!found) return FALSE;
	NRcount--;
	for ( ; i < NRcount; i++) {
		NRkeys[i] = NRkeys[i+1];
		NRnames[i] = NRnames[i+1];
	}
	p = rec = (STRING) stdalloc(NRsize);
	len = 0;
	memcpy(p, &NRcount, sizeof(INT));
	p += sizeof(INT);
	len += sizeof(INT);
	for (i = 0; i < NRcount; i++) {
		memcpy(p, &NRkeys[i], sizeof(RKEY));
		p += sizeof(RKEY);
		len += sizeof(RKEY);
	}
	off = 0;
	for (i = 0; i < NRcount; i++) {
		memcpy(p, &off, sizeof(INT));
		p += sizeof(INT);
		len += sizeof(INT);
		off += strlen(NRnames[i]) + 1;
	}
	for (i = 0; i < NRcount; i++) {
		memcpy(p, NRnames[i], strlen(NRnames[i]) + 1);
		p += strlen(NRnames[i]) + 1;
		len += strlen(NRnames[i]) + 1;
	}
	addrecord(BTR, NRkey, rec, len);
	stdfree(rec);
	return TRUE;
}
#if 0
/*===============================================
 * replace_name -- Replace entry in name records.
 *=============================================*/
BOOLEAN replace_name (old, new, key)
STRING old;	/* person's old name */
STRING new;	/* person's new name */
STRING key;	/* person's INDI key */
{
	remove_name(old, key);
	add_name(new, key);
	return TRUE;
}
#endif
/*=========================================================
 * exactmatch -- Check if first name is contained in second
 *=======================================================*/
BOOLEAN exactmatch (partial, complete)
STRING partial;		/* name from user */
STRING complete;	/* GEDCOM name */
{
	char part[MAXNAMELEN+2], comp[MAXNAMELEN+2], *p, *q;
	BOOLEAN okay;

	if (strlen(partial) > MAXNAMELEN || strlen(complete) > MAXNAMELEN)
		return FALSE;
	squeeze(partial, part);
	squeeze(complete, comp);
	q = comp;
	for (p = part; *p; p += strlen(p) + 1) {
		okay = FALSE;
		for (; !okay && *q; q += strlen(q) + 1) {
			if (piecematch(p, q)) okay = TRUE;
		}
		if (!okay) return FALSE;
	}
	return TRUE;
}
/*================================================================
 * piecematch -- Match partial word with complete; must begin with
 *   same letter; letters in partial must be in same order as in
 *   complete; case insensitive
 *==============================================================*/
BOOLEAN piecematch (part, comp)
STRING part, comp;
{
	if(opt_finnish) {
	  if(my_chrcmp(*part++, *comp++) != 0) return FALSE;
	} else {
	  if (*part++ != *comp++) return FALSE;
	}
	while (*part && *comp) {
	  if(opt_finnish) {
		if (my_chrcmp(*part, *comp++) == 0) part++;
	  } else {
		if (*part == *comp++) part++;
	  }
	}
	return *part == 0;
}
/*===============================================================
 * squeeze -- Squeeze string to superstring, string of uppercase,
 *   0-terminated words, ending with another 0; non-letters not
 *   copied; eg., `Anna /Van Cott/' maps to `ANNA\0VANCOTT\0\0'.
 *=============================================================*/
static void squeeze (in, out)
STRING in;	/* string of words */
STRING out;	/* superstring of words */
{
	INT c;
	while ((c = *in++) && chartype(c) != LETTER)
		;
	if (c == 0) {
		*out++ = 0; *out = 0;
		return;
	}
	while (TRUE) {
		*out++ = ll_toupper(c);
		while ((c = *in++) && c != '/' && chartype(c) != WHITE) {
			if (chartype(c) == LETTER) *out++ = ll_toupper(c);
		}
		if (c == 0) {
			*out++ = 0; *out = 0;
			return;
		}
		*out++ = 0;
		while ((c = *in++) && chartype(c) != LETTER)
			;
		if (c == 0) {
			*out++ = 0; *out = 0;
			return;
		}
	}
}

/*====================================================
 * get_names -- Find all persons who match name or key
 *==================================================*/
STRING *get_names (name, pnum, pkeys, exact)
STRING name;
INT *pnum;
STRING **pkeys;
BOOLEAN exact;	/* unused! */
{
	INT i, n;
	STRING *strs, *id_by_key();

   /* See if user is asking for person by key instead of name */
	if ((strs = id_by_key(name, pkeys))) {
		*pnum = 1;
		return strs;
	}

   /* Clean up allocated memory from last call */
	if (LMcount) {
		for (i = 0; i < LMcount; i++)
			stdfree(LMkeys[i]);
	}

   /* Load up static name buffers; return if no match */
	LMcount = 0;
	if (!getnamerec(name)) {
		*pnum = 0;
		return NULL;
	}

   /* Compare user's name against all names in name record; the name
      record data structures are modified */
	n = 0;
	for (i = 0; i < NRcount; i++) {
		if (exactmatch(name, NRnames[i])) {
			if (i != n) {
				NRnames[n] = NRnames[i];
				NRkeys[n] = NRkeys[i];
			}
			n++;
		}
	}
	*pnum = NRcount = n;
	if (NRcount > LMmax) {
		if (LMmax) stdfree(LMkeys);
		LMkeys = (STRING *) stdalloc(NRcount*sizeof(STRING));
		LMmax = NRcount;
	}
	for (i = 0; i < NRcount; i++)
		LMkeys[i] = strsave(rkey2str(NRkeys[i]));
	*pkeys = LMkeys;
	return NRnames;
}
/*====================================
 * namecmp -- Compare two GEDCOM names
 *==================================*/
int namecmp (name1, name2)
STRING name1, name2;
{
	unsigned char sqz1[MAXNAMELEN], sqz2[MAXNAMELEN];
	STRING p1 = sqz1, p2 = sqz2;
	INT r = nestr(getsurname(name1), getsurname(name2));
	if (r) return r;
	if(opt_finnish) {
	  r = my_chrcmp(getfinitial(name1),  getfinitial(name2));
	} else {
	  r = getfinitial(name1) - getfinitial(name2);
	}
	if (r) return r;
	cmpsqueeze(name1, p1);
	cmpsqueeze(name2, p2);
	while (*p1 && *p2) {
		r = nestr(p1, p2);
		if (r) return r;
		p1 += strlen(p1) + 1;
		p2 += strlen(p2) + 1;
	}
	if (*p1) return 1;
	if (*p2) return -1;
	return 0;
}
/*===========================================================
 * cmpsqueeze -- Squeeze GEDCOM name to superstring of givens
 *=========================================================*/
void cmpsqueeze (in, out)
STRING in, out;
{
	INT c;
	while ((in = nextpiece(in))) {
		while (TRUE) {
			c = *in++;
			if (iswhite(c) || c == '/' || c == 0) {
				*out++ = 0;
				--in;
				break;
			}
			*out++ = c;
		}
	}
	*out = 0;
}
/*=====================================
 * givens -- Return given names of name
 *===================================*/
STRING givens (name)
STRING name;
{
	INT c;
	static unsigned char scratch[MAXNAMELEN+1];
	STRING out = scratch;
	while ((name = nextpiece(name))) {
		while (TRUE) {
			if ((c = *name++) == 0) {
				if (*(out-1) == ' ') --out;
				*out = 0;
				return scratch;
			}
			if (iswhite(c) || c == '/') {
				*out++ = ' ';
				--name;
				break;
			}
			*out++ = c;
		}
	}
	if (*(out-1) == ' ') --out;
	*out = 0;
	return scratch;
}
/*========================================
 * nextpiece -- Return next word in string
 *======================================*/
static STRING nextpiece (in)
STRING in;
{
	int c;
	while (TRUE) {
		while (iswhite(c = *in++))
			;
		if (c == 0) return NULL;
		if (c != '/') return --in;
		while ((c = *in++) && c != '/')
			;
		if (c == 0) return NULL;
	}
}
/*===================================================================
 * trim_name -- Trim GEDCOM name to less or equal to given length but
 *   not shorter than first initial and surname
 *=================================================================*/
#define MAXPARTS 100
STRING trim_name (name, len)
STRING name;
INT len;
{
	STRING parts[MAXPARTS];
	INT i, sdex = -1, nparts;
	name_to_parts(name, parts);
	name = parts_to_name(parts);
	if (strlen(name) <= len + 2) return name;
	for (i = 0; i < MAXPARTS; i++) {
		if (!parts[i]) break;
		if (*parts[i] == '/') sdex = i;
	}
	nparts = i;
	/* WARNING: this will cause a program termination if there
	 * is no surname delimited by "/" 
	 * ASSERT(sdex != -1);
	 */
	if(sdex == -1) sdex = nparts;
	for (i = sdex-1; i >= 0; --i) {
		*(parts[i] + 1) = 0;
		name = parts_to_name(parts);
		if (strlen(name) <= len + 2) return name;
	}
	for (i = sdex-1; i >= 1; --i) {
		parts[i] = NULL;
		name = parts_to_name(parts);
		if (strlen(name) <= len + 2) return name;
	}
	for (i = nparts-1; i > sdex; --i) {
		parts[i] = NULL;
		name = parts_to_name(parts);
		if (strlen(name) <= len + 2) return name;
	}
	return name;
}
/*============================================================
 * name_to_parts -- Convert GEDCOM name to parts; keep slashes
 *==========================================================*/
static void name_to_parts (name, parts)
STRING name;	/* GEDCOM name */
STRING *parts;
{
	static unsigned char scratch[MAXNAMELEN+1];
	STRING p = scratch;
	INT c, i = 0;
	ASSERT(strlen(name) <= MAXNAMELEN);
	for (i = 0; i < MAXPARTS; i++)
		parts[i] = NULL;
	i = 0;
	while (TRUE) {
		while (iswhite(c = *name++))
			;
		if (c == 0) return;
		ASSERT(i < MAXPARTS);
		parts[i++] = p;
		*p++ = c;
		if (c == '/') {
			while ((c = *p++ = *name++) && c != '/')
				;
			if (c == 0) return;
			*p++ = 0;
		} else {
			while ((c = *name++) && !iswhite(c) && c != '/')
				*p++ = c;
			*p++ = 0;
			if (c == 0) return;
			if (c == '/') name--;
		}
	}
}
/*======================================================
 * parts_to_name -- Convert list of parts back to string
 *====================================================*/
static STRING parts_to_name (parts)
STRING *parts;
{
	INT i;
	static unsigned char scratch[MAXNAMELEN+1];
	STRING p = scratch;
	for (i = 0; i < MAXPARTS; i++) {
		if (!parts[i]) continue;
		strcpy(p, parts[i]);
		p += strlen(parts[i]);
		*p++ = ' ';
	}
	if (*(p - 1) == ' ')
		*(p - 1) = 0;
	else
		*p = 0;
	return scratch;
}
/*=======================================================
 * upsurname -- Convert GEDCOM surname name to upper case
 *=====================================================*/
STRING upsurname (name)
STRING name;
{
	static unsigned char scratch[MAXNAMELEN+1];
	STRING p = scratch;
	INT c;
	while ((c = *p++ = *name++) && c != '/')
		;
	if (c == 0) return scratch;
	while ((c = *name++) && c != '/')
		*p++ = ll_toupper(c);
	*p++ = c;
	if (c == 0) return scratch;
	while ((c = *p++ = *name++))
		;
	return scratch;
}
/*==================================================
 * manip_name - Convert GEDCOM name to various forms
 *================================================*/
STRING manip_name (name, tt, caps, regorder, len)
STRING name;	/* name */
TRANTABLE tt;	/* translation table */
BOOLEAN caps;	/* surname in caps? */
BOOLEAN regorder;	/* regular order? (not surname first) */
INT len;	/* max name length */
{
	static unsigned char scratch[MAXNAMELEN+1];
	if (!name || *name == 0) return NULL;
	translate_string(tt, name, scratch, MAXNAMELEN+1);
	name = scratch;
	if (caps) name = upsurname(name);
	name = trim_name(name, regorder ? len: len-1);
	if (regorder) return trim(name_string(name), len);
	return trim(name_surfirst(name), len);
}
/*===============================================
 * name_string -- Remove slashes from GEDCOM name
 *=============================================*/
STRING name_string (name)
STRING name;
{
	static unsigned char scratch[MAXNAMELEN+1];
	STRING p = scratch;
	ASSERT(strlen(name) <= MAXNAMELEN);
	while (*name) {
		if (*name != '/') *p++ = *name;
		name++;
	}
	*p-- = 0;
	striptrail(scratch);
	return scratch;
}
/*==========================================================
 * name_surfirst - Convert GEDCOM name to surname first form
 *========================================================*/
STRING name_surfirst (name)
STRING name;
{
	static unsigned char scratch[MAXNAMELEN+1];
	STRING p = scratch;
	ASSERT(strlen(name) <= MAXNAMELEN);
	strcpy(p, getasurname(name));
	p += strlen(p);
	strcpy(p, ", ");
	p += strlen(p);
	strcpy(p, givens(name));
	return scratch;
}
/*================================
 * id_by_key -- Find name from key
 *==============================*/
STRING *id_by_key (name, pkeys)
STRING name;
STRING **pkeys;
{
	STRING rec, str, p = name;
	static unsigned char kbuf[MAXNAMELEN];
	static unsigned char nbuf[MAXNAMELEN];
	static STRING kaddr, naddr;
	INT i = 0, c, len;
	NODE indi;
	while ((c = *p++) && chartype(c) == WHITE)
		;
	if (c == 0) return NULL;
	if (c != 'I' && c != 'i' && chartype(c) != DIGIT) return NULL;
	if (chartype(c) != DIGIT) c = *p++;
	if (chartype(c) != DIGIT) return NULL;
	kbuf[i++] = 'I';
	kbuf[i++] = c;
	while ((c = *p++) && chartype(c) == DIGIT)
		kbuf[i++] = c;
	if (c != 0) return NULL;
	kbuf[i] = 0;
	kaddr = kbuf;
	*pkeys = &kaddr;
	if (!(rec = (STRING) getrecord(BTR, str2rkey(kbuf), &len)))
		return NULL;
	if (!(indi = string_to_node(rec))) {
		stdfree(rec);
		return NULL;
	}
	if (!(str = nval(NAME(indi))) || *str == 0) {
		stdfree(rec);
		return NULL;
	}
	strcpy(nbuf, str);
	naddr = nbuf;
	stdfree(rec);
	return &naddr;
}
/*============================================
 * name_to_list -- Convert name to string list
 *==========================================*/
BOOLEAN name_to_list (name, list, plen, psind)
STRING name;	/* GEDCOM name */
LIST list;	/* list (must exist) */
INT *plen;	/* returned length */
INT *psind;	/* index (rel 1) of surname in list */
{
	INT i;
	STRING str;
	STRING parts[MAXPARTS];
	if (!name || *name == 0 || !list) return FALSE;
	make_list_empty(list);
	set_list_type(list, LISTDOFREE);
	*psind = 0;
	name_to_parts(name, parts);
	for (i = 0; i < MAXPARTS; i++) {
		if (!parts[i]) break;
		if (*parts[i] == '/') {
			*psind = i + 1;
			str = strsave(parts[i] + 1);
			if (str[strlen(str) - 1] == '/')
				str[strlen(str) - 1] = 0;
		} else
			str = strsave(parts[i]);
		set_list_element(list, i + 1, str);
	}
	*plen = i;
	return TRUE;
}
