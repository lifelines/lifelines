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
 *   3.0.2 - 31 Dec 94
 *===========================================================*/

#include "standard.h"
#include "llstdlib.h"
#include "mystring.h"

extern BOOLEAN opt_finnish;

/*===============================
 * strsave -- Save copy of string
 *=============================*/
STRING
strsave (STRING str)
{
	return strcpy(stdalloc(strlen(str) + 1), str);
}
/*==================================
 * strconcat -- Catenate two strings
 *================================*/
STRING
strconcat (STRING s1,
           STRING s2)
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
#ifndef OS_NOCTYPE
	if ( isspace(c) )
		return WHITE;
	if (opt_finnish) 
	    {
	    if( my_isalpha(c) ) return LETTER;
	    }
	else if ( isalpha(c) ) return LETTER;
	if ( isdigit(c) ) return DIGIT;
	return c;
#else
	if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
		return WHITE;
	if (opt_finnish) 
	    {
	    if( my_isalpha(c) ) return LETTER;
	    }
	else
	    {
	    if (c >= 'a' && c <= 'z') return LETTER;
	    if (c >= 'A' && c <= 'Z') return LETTER;
	    }
	if (c >= '0' && c <= '9') return DIGIT;
	return c;
#endif
}
/*=================================
 * iswhite -- Check for white space
 *===============================*/
BOOLEAN
iswhite (INT c)
{
#ifndef OS_NOCTYPE
	return( isspace(c) );
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
    	if(opt_finnish) return(my_isalpha(c));
#ifndef OS_NOCTYPE
	return( isalpha(c) );
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
	while ((c = *str++)) {
#ifndef OS_NOCTYPE
		if (chartype(c) != DIGIT) return FALSE;
#else
		if ( ! isdigit(c) ) return FALSE;
#endif
	}
	return TRUE;
}
/*======================================
 * lower -- Convert string to lower case
 *====================================*/
STRING
lower (STRING str)
{
	static unsigned char scratch[MAXLINELEN+1];
	STRING p = scratch;
	INT c;
	while ((c = *str++))
		*p++ = ll_tolower(c);
	*p = '\0';
	return scratch;
}
/*======================================
 * upper -- Convert string to upper case
 *====================================*/
STRING
upper (STRING str)
{
	static unsigned char scratch[MAXLINELEN+1];
	STRING p = scratch;
	INT c;
	while ((c = *str++))
		*p++ = ll_toupper(c);
	*p = '\0';
	return scratch;
}
/*================================
 * capitalize -- Capitalize string
 *==============================*/
STRING
capitalize (STRING str)
{
	STRING p = lower(str);
	*p = ll_toupper(*p);
	return p;
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
        if(opt_finnish) return(my_toupper(c));
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
 *==============================*/
STRING
trim (STRING str, INT len)
{
	static unsigned char scratch[MAXLINELEN+1];
	if (!str || strlen(str) > MAXLINELEN) return NULL;
	if (len < 0) len = 0;
	if (len > MAXLINELEN) len = MAXLINELEN;
	strcpy(scratch, str);
	scratch[len] = '\0';
	return scratch;
}
/*=========================================
 * striptrail -- Strip trailing white space
 *=======================================*/
void
striptrail (STRING p)
{
        unsigned char *q = p + strlen(p) - 1;
        while (iswhite(*q) && q >= p)
                *q-- = '\0';
}
/*=======================================
 * striplead -- Strip leading white space
 *=====================================*/
void
striplead (STRING p)
{
	unsigned char *e = p + strlen(p) - 1;
	unsigned char *b = p;
        unsigned char *q = p;

        while (iswhite(*q) && q <= e)
                q++;

        while (b <= e)
            *b++ = *q++;
	*b++ = '\0';
}
/*=========================================
 * allwhite -- Check if string is all white
 *=======================================*/
BOOLEAN
allwhite (STRING p)
{
        while (*p)
                if (!iswhite(*p++)) return FALSE;
        return TRUE;
}
