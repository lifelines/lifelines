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
 * stdstrng.c -- Standard string routines
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   3.0.2 - 31 Dec 94
 *===========================================================*/

#include "llstdlib.h"
#include "mystring.h"

/*********************************************
 * external/imported variables
 *********************************************/
extern BOOLEAN opt_finnish;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static BOOLEAN printpic_arg(STRING *b, INT max, CNSTRING arg, INT arglen);

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*===============================
 * strsave -- Save copy of string
 * returns stdalloc'd memory
 *=============================*/
STRING
strsave (CNSTRING str)
{
	return strcpy(stdalloc(strlen(str) + 1), str);
}
/*===============================
 * strfree -- Free & clear a STRING by ref
 *  (STRING may be NULL)
 *=============================*/
void
strfree (STRING * str)
{
	if (*str) {
		stdfree(*str);
		*str = NULL;
	}
}
/*==================================
 * strconcat -- Catenate two strings
 * Either (but not both) args may be null
 * returns stdalloc'd memory
 *================================*/
STRING
strconcat (STRING s1, STRING s2)
{
	INT c, len;
	STRING s3, p;
	if (!s1) return strsave(s2);
	if (!s2) return strsave(s1);
	len = strlen(s1) + strlen(s2);
	p = s3 = (STRING) stdalloc(len+1);
	while ((c = *s1++)) *p++ = c;
	while ((c = *s2++)) *p++ = c;
	*p = '\0';
	return s3;
}
/*==================================
 * chartype -- Return character type
 *================================*/
INT
chartype (INT c)
{
	if (c<0) {
	/* This most likely means someone assigned a char to an INT
	which is very bad -- it gets sign extended! -- it must always
	be first cast to a uchar (unsigned char), eg
	INT a = (uchar)*p;
	We can't pass this to O/S isx functions, because some of them
	are table driven, and we certainly don't want to give them an
	offset of several billion negative.
	*/
	
		return c;
	}
#ifndef OS_NOCTYPE
	if (isspace(c))
		return WHITE;
	if (opt_finnish) {
		if (my_isalpha(c)) return LETTER;
	}
	else if (isalpha(c)) return LETTER;
	if (isdigit(c)) return DIGIT;
	return c;
#else
	if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
		return WHITE;
	if (opt_finnish) {
		if( my_isalpha(c) ) return LETTER;
	}
	else {
		if (c >= 'a' && c <= 'z') return LETTER;
		if (c >= 'A' && c <= 'Z') return LETTER;
	}
	if (c >= '0' && c <= '9') return DIGIT;
	return c;
#endif
}
/*=================================
 * iswhite -- Check for white space
 * Note: input character is passed to isspace, so it
 *  should be in unsigned char range.
 *===============================*/
BOOLEAN
iswhite (INT c)
{
#ifndef OS_NOCTYPE
	return (isspace(c));
#else
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
#endif
}

/*=============================
 * isletter -- Check for letter
 *===========================*/
BOOLEAN
isletter (INT c)
{
	if (opt_finnish) return (my_isalpha(c));
#ifndef OS_NOCTYPE
	return (isalpha(c));
#else
	if (c >= 'a' && c <= 'z') return TRUE;
	return c >= 'A' && c <= 'Z';
#endif
}

/*=========================================
 * isnumeric -- Check string for all digits
 *=======================================*/
BOOLEAN
isnumeric (STRING str)
{
	INT c;
	if (!str) return FALSE;
	while ((c = (uchar)*str++)) {
#ifndef OS_NOCTYPE
		if (chartype(c) != DIGIT) return FALSE;
#else
		if (!isdigit(c)) return FALSE;
#endif
	}
	return TRUE;
}
/*======================================
 * lower -- Convert string to lower case
 *  returns static buffer
 *====================================*/
STRING
lower (STRING str)
{
	static char scratch[MAXLINELEN+1];
	STRING p = scratch;
	INT c, i=0;
	while ((c = (uchar)*str++) && (++i < MAXLINELEN+1))
		*p++ = ll_tolower(c);
	*p = '\0';
	return scratch;
}
/*======================================
 * upper -- Convert string to upper case
 *  returns static buffer
 *====================================*/
STRING
upper (STRING str)
{
	static char scratch[MAXLINELEN+1];
	STRING p = scratch;
	INT c, i=0;
	while ((c = (uchar)*str++) && (++i < MAXLINELEN+1))
		*p++ = ll_toupper(c);
	*p = '\0';
	return scratch;
}
/*================================
 * capitalize -- Capitalize string
 *  returns static buffer (borrowed from lower)
 *==============================*/
STRING
capitalize (STRING str)
{
	STRING p = lower(str);
	*p = ll_toupper((uchar)*p);
	return p;
}
/*================================
 * titlecase -- Titlecase string
 * Created: 2001/12/30 (Perry Rapp)
 *  returns static buffer (borrowed from lower)
 *==============================*/
STRING
titlecase (STRING str)
{
	/* % sequences aren't a problem, as % isn't lower */
	STRING p = lower(str), buf=p;
	if (!p[0]) return p;
	while (1) {
		/* capitalize first letter of word */
		*p = ll_toupper((uchar)*p);
		/* skip to end of word */
		while (*p && !iswhite((uchar)*p))
			++p;
		if (!*p) return buf;
		/* skip to start of next word */
		while (*p && iswhite((uchar)*p))
			++p;
		if (!*p) return buf;
	}
}
/*==========================================
 * ll_toupper -- Convert letter to uppercase
 *========================================*/
INT
ll_toupper (INT c)
{
	if(opt_finnish) return(my_toupper(c));
#ifndef OS_NOCTYPE
	if(islower(c)) return( toupper(c) );
	return c;
#else
	if (c < 'a' || c > 'z') return c;
	return c + 'A' - 'a';
#endif
}
/*==========================================
 * ll_tolower -- Convert letter to lowercase
 *========================================*/
INT
ll_tolower (INT c)
{
	if(opt_finnish) return(my_tolower(c));
#ifndef OS_NOCTYPE
	if(isupper(c)) return( tolower(c) );
	return(c);
#else
	if (c < 'A' || c > 'Z') return c;
	return c + 'a' - 'A';
#endif
}
/*================================
 * trim -- Trim string if too long
 *  returns static buffer (or NULL)
 *==============================*/
STRING
trim (STRING str, INT len)
{
	static char scratch[MAXLINELEN+1];
	if (!str || strlen(str) > MAXLINELEN) return NULL;
	if (len < 0) len = 0;
	if (len > MAXLINELEN) len = MAXLINELEN;
	strcpy(scratch, str);
	scratch[len] = '\0';
	return scratch;
}
/*=========================================
 * striptrail -- Strip trailing white space
 *  modifies argument (zeros out trailing whitespace)
 *=======================================*/
void
striptrail (STRING p)
{
	STRING q = p + strlen(p) - 1;
	while (q >= p && iswhite((uchar)*q))
		*q-- = '\0';
}
#ifdef UNUSED_CODE
/*=======================================
 * striplead -- Strip leading white space
 *  modifies argument (shifts up string towards
 *  beginning to eliminate any leading whitespace)
 * UNUSED CODE
 *=====================================*/
void
striplead (STRING p)
{
	INT i = strlen(p);
	STRING  e = p + i - 1;
	STRING b = p;
	STRING q = p;

	while (iswhite((uchar)*q) && q <= e) {
		++q;
		--i; /* keep from copying past end of p */
	}
	if (q == p) return;

	while (b <= e && --i >= 0)
		*b++ = *q++;
	*b++ = '\0';
}
#endif /* UNUSED_CODE */
/*=========================================
 * allwhite -- Check if string is all white
 *=======================================*/
BOOLEAN
allwhite (STRING p)
{
	while (*p)
		if (!iswhite((uchar)*p++)) return FALSE;
	return TRUE;
}
/*==============================
 * utf8len -- Length of a utf-8 character
 *  ch: [in] starting byte
 * Created: 2001/08/02 (Perry Rapp)
 *============================*/
INT
utf8len (char ch)
{
	/* test short cases first, as probably much more common */
	if (!(ch & 0x80 && ch & 0x40))
		return 1;
	if (!(ch & 0x20))
		return 2;
	if (!(ch & 0x10))
		return 3;
	if (!(ch & 0x08))
		return 4;
	if (!(ch & 0x04))
		return 5;
	return 6;
}
/*==============================
 * printpic_arg -- Print an arg to a string
 *  This is the heart of sprintpic, here so we
 *  can use it in all sprintpic's.
 *  b:      [I/O] pointer to output buffer
 *  max:    [IN]  space left in output buffer
 *  arg:    [IN]  arg to insert
 *  arglen: [IN]  (precomputed) length of arg
 * returns FALSE if can't fit entire arg.
 * Created: 2001/12/30 (Perry Rapp)
 *============================*/
static BOOLEAN
printpic_arg (STRING *b, INT max, CNSTRING arg, INT arglen)
{
	if (!arglen) return TRUE;
	if (arglen > max) {
		/* can't fit it all */
		llstrncpy(*b, arg, max+1); 
		b[max] = 0;
		return FALSE;
	} else {
		/* it fits */
		strcpy(*b, arg);
		*b += arglen;
		return TRUE;
	}
}
/*==============================
 * sprintpic1 -- Print using a picture string
 *  with one argument, eg "From %1"
 *  buffer:  [I/O] output buffer
 *  len:     [IN]  size of output buffer
 *  pic:     [IN]  picture string
 *  arg1:    [IN]  argument (for %1)
 * (Multiple occurrences of %1 are allowed.)
 * returns FALSE if it couldn't fit whole thing.
 * Created: 2001/12/30 (Perry Rapp)
 *============================*/
BOOLEAN
sprintpic1 (STRING buffer, INT len, CNSTRING pic, CNSTRING arg1)
{
	STRING b = buffer, bmax = &buffer[len-1];
	CNSTRING p=pic;
	INT arg1len = arg1 ? strlen(arg1) : 0; /* precompute */
	while (1) {
		if (p[0]=='%' && p[1]=='1') {
			if (!printpic_arg(&b, bmax-b, arg1, arg1len))
				return FALSE;
			p += 2;
		} else {
			*b++ = *p++;
		}
		if (!p[0]) { /* ran out of input */
			*b=0;
			return TRUE;
		}
		if (b == bmax) { /* ran out of output room */
			*b=0;
			return FALSE;
		}
	}
}
/*==============================
 * sprintpic2 -- Print using a picture string
 *  with two arguments, eg "From %1 To %s"
 * See sprintpic1 for argument explanation.
 * Created: 2001/12/30 (Perry Rapp)
 *============================*/
BOOLEAN
sprintpic2 (STRING buffer, INT len, CNSTRING pic, CNSTRING arg1, CNSTRING arg2)
{
	STRING b = buffer, bmax = &buffer[len-1];
	CNSTRING p=pic;
	INT arg1len = arg1 ? strlen(arg1) : 0; /* precompute */
	INT arg2len = arg2 ? strlen(arg2) : 0;
	while (1) {
		if (p[0]=='%' && p[1]=='1') {
			if (!printpic_arg(&b, bmax-b, arg1, arg1len))
				return FALSE;
			p += 2;
		} else if (p[0]=='%' && p[1]=='2') {
			if (!printpic_arg(&b, bmax-b, arg2, arg2len))
				return FALSE;
			p += 2;
		} else {
			*b++ = *p++;
		}
		if (!p[0]) { /* ran out of input */
			*b=0;
			return TRUE;
		}
		if (b == bmax) { /* ran out of output room */
			*b=0;
			return FALSE;
		}
	}
}
/*==============================
 * sprintpic3 -- Print using a picture string
 *  with three arguments, eg "%1/%2/%3"
 * See sprintpic1 for argument explanation.
 * Created: 2001/12/30 (Perry Rapp)
 *============================*/
BOOLEAN
sprintpic3 (STRING buffer, INT len, CNSTRING pic, CNSTRING arg1, CNSTRING arg2
	, CNSTRING arg3)
{
	STRING b = buffer, bmax = &buffer[len-1];
	CNSTRING p=pic;
	INT arg1len = arg1 ? strlen(arg1) : 0; /* precompute */
	INT arg2len = arg2 ? strlen(arg2) : 0;
	INT arg3len = arg3 ? strlen(arg3) : 0;
	while (1) {
		if (p[0]=='%' && p[1]=='1') {
			if (!printpic_arg(&b, bmax-b, arg1, arg1len))
				return FALSE;
			p += 2;
		} else if (p[0]=='%' && p[1]=='2' && arg2len) {
			if (!printpic_arg(&b, bmax-b, arg2, arg2len))
				return FALSE;
			p += 2;
		} else if (p[0]=='%' && p[1]=='3') {
			if (!printpic_arg(&b, bmax-b, arg3, arg3len))
				return FALSE;
			p += 2;
		} else {
			*b++ = *p++;
		}
		if (!p[0]) { /* ran out of input */
			*b=0;
			return TRUE;
		}
		if (b == bmax) { /* ran out of output room */
			*b=0;
			return FALSE;
		}
	}
}
/*============================================
 * chomp -- remove any trailing carriage return/linefeed
 * Created: 2002/01/03 (Perry Rapp)
 *==========================================*/
void
chomp (STRING str)
{
	STRING p = str + strlen(str) - 1;
	while (p>=str && (*p=='\r' || *p=='\n')) {
		*p=0;
		--p;
	}
}
