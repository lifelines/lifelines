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
/*=============================================================
 * stdstrng.c -- Standard string routines
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 31 Dec 94
 *===========================================================*/

#include "standard.h"

STRING strcpy();

/*===============================
 * strsave -- Save copy of string
 *=============================*/
STRING strsave (str)
STRING str;
{
	return strcpy(stdalloc(strlen(str) + 1), str);
}
/*==================================
 * strconcat -- Catenate two strings
 *================================*/
STRING strconcat (s1, s2)
STRING s1, s2;
{
	INT c, len;
	STRING s3, p;
	if (!s1) return strsave(s2);
	if (!s2) return strsave(s1);
	len = strlen(s1) + strlen(s2);
	p = s3 = (STRING) stdalloc(len+1);
	while (c = *s1++) *p++ = c;
	while (c = *s2++) *p++ = c;
	*p = 0;
	return s3;
}
/*==================================
 * chartype -- Return character type 
 *================================*/
INT chartype (c)
INT c;
{
	if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
		return WHITE;
	if (c >= 'a' && c <= 'z') return LETTER;
	if (c >= 'A' && c <= 'Z') return LETTER;
	if (c >= '0' && c <= '9') return DIGIT;
	return c;
}
/*=================================
 * iswhite -- Check for white space
 *===============================*/
BOOLEAN iswhite (c)
INT c;
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
/*=============================
 * isletter -- Check for letter
 *===========================*/
BOOLEAN isletter (c)
INT c;
{
	if (c >= 'a' && c <= 'z') return TRUE;
	return c >= 'A' && c <= 'Z';
}
/*=========================================
 * isnumeric -- Check string for all digits
 *=======================================*/
BOOLEAN isnumeric (str)
STRING str;
{
	INT c;
	if (!str) return FALSE;
	while (c = *str++) {
		if (chartype(c) != DIGIT) return FALSE;
	}
	return TRUE;
}
/*======================================
 * lower -- Convert string to lower case
 *====================================*/
STRING lower (str)
STRING str;
{
	static unsigned char scratch[MAXLINELEN+1];
	STRING p = scratch;
	INT c;
	while (c = *str++)
		*p++ = ll_tolower(c);
	*p = 0;
	return scratch;
}
/*======================================
 * upper -- Convert string to upper case
 *====================================*/
STRING upper (str)
STRING str;
{
	static unsigned char scratch[MAXLINELEN+1];
	STRING p = scratch;
	INT c;
	while (c = *str++)
		*p++ = ll_toupper(c);
	*p = 0;
	return scratch;
}
/*================================
 * capitalize -- Capitalize string
 *==============================*/
STRING capitalize (str)
STRING str;
{
	STRING p = lower(str);
	*p = ll_toupper(*p);
	return p;
}
/*==========================================
 * ll_toupper -- Convert letter to uppercase
 *========================================*/
INT ll_toupper (c)
INT c;
{
	if (c < 'a' || c > 'z') return c;
	return c + 'A' - 'a';
}
/*==========================================
 * ll_tolower -- Convert letter to lowercase
 *========================================*/
INT ll_tolower (c)
INT c;
{
	if (c < 'A' || c > 'Z') return c;
	return c + 'a' - 'A';
}
/*================================
 * trim -- Trim string if too long
 *==============================*/
STRING trim (str, len)
STRING str;
INT len;
{
	static unsigned char scratch[MAXLINELEN+1];
	if (!str || strlen(str) > MAXLINELEN) return NULL;
	if (len < 0) len = 0;
	if (len > MAXLINELEN) len = MAXLINELEN;
	strcpy(scratch, str);
	scratch[len] = 0;
	return scratch;
}
