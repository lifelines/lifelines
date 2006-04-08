/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * icvt.c -- Module that handles iconv calls
 *==============================================================*/

#include "llstdlib.h" /* includes standard.h, sys_inc.h, llnls.h, config.h */
#include <errno.h>
#ifdef HAVE_ICONV
# ifdef WIN32_ICONV_SHIM
#  include "win32/iconvshim.h"
# else
#  include <iconv.h>
# endif
#endif
/* wcslen may be declared in "arch.h" or <wchar.h> */
#include "arch.h"
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#include "zstr.h"
#include "icvt.h"


/*===================================================
 * iconv_can_trans -- Can iconv do this translation ?
 *=================================================*/
BOOLEAN
iconv_can_trans (CNSTRING src, CNSTRING dest)
{
#ifdef HAVE_ICONV
	iconv_t ict;

	ict = iconv_open(dest, src);
	if (ict == (iconv_t)-1)
		return FALSE;
    iconv_close(ict);
	return TRUE;
#else
	src=src; /* unused */
	dest=dest; /* unused */
	return FALSE;
#endif
}
/*===================================================
 * iconv_trans -- Translate string via iconv
 *  src:     [IN]  source codeset
 *  dest:    [IN]  string to translate (& delete)
 *  zin:     [I/O] source string (may be returned if iconv can't translate)
 *  success: [OUT] success flag (optional)
 * Only called if HAVE_ICONV
 *=================================================*/
BOOLEAN
iconv_trans (CNSTRING src, CNSTRING dest, CNSTRING sin, ZSTR zout, char illegal)
{
#ifdef HAVE_ICONV
	iconv_t ict;
	const char * inptr;
	char * outptr;
	size_t inleft;
	size_t outleft;
	size_t cvted;
#ifdef ICONV_SET_TRANSLITERATE
	int transliterate=2; 
#endif
	double expand=1.3;
	int chwidth=1;
	int inlen = sin ? strlen(sin) : 0;

	ASSERT(src);
	ASSERT(dest);

	ict = iconv_open(dest, src);

	if (ict == (iconv_t)-1) {
		return FALSE;
	}
	if (!strncmp(src, "UCS-2", strlen("UCS-2"))) {
		/* assume MS-Windows makenarrow call */
		inlen = 2 * wcslen((const wchar_t *)sin);
	}
	if (!strncmp(src, "UCS-2", strlen("UCS-2"))) {
		/* assume UNIX makenarrow call */
		inlen = 4 * wcslen((const wchar_t *)sin);
	}
	if (!strncmp(dest, "UCS-2", strlen("UCS-2"))) {
		chwidth = expand = 2;
	}
	if (!strncmp(dest, "UCS-4", strlen("UCS-4"))) {
		chwidth = expand = 4;
	}
	if (eqstr(dest, "wchar_t")) {
		chwidth = expand = sizeof(wchar_t);

	}
	/* TODO: What about UTF-16 or UTF-32 ? */

	zs_reserve(zout, (unsigned int)(inlen*expand+6));

	if (!inlen) {
		outptr = zs_str(zout);
		goto icvt_terminate_and_exit;
	}

	/* testing recursive transliteration in my private iconv, Perry, 2002.07.11 */
#ifdef ICONV_SET_TRANSLITERATE
	iconvctl(ict, ICONV_SET_TRANSLITERATE, &transliterate);
#endif


	inptr = sin;
	outptr = zs_str(zout);
	inleft = inlen;
	/* we are terminating with 4 zero bytes just in case dest is UCS-4 */
	outleft = zs_allocsize(zout)-zs_len(zout)-4;
	cvted = 0;

cvting:
	/* main convert */
	cvted = iconv (ict, (char **)&inptr, &inleft, &outptr, &outleft);

	/* zero terminate & fix output zstring */
	/* there may be embedded nulls, if UCS-2/4 is target! */
	*outptr=0;
	zs_set_len(zout, outptr-zs_str(zout));

	/* handle error cases */
	if (cvted == (size_t)-1) {
		/* errno is not reliable, because on MS-Windows we called
		iconv in a dll & didn't get errno */
		if (outleft<3) {
			/* may be out of space, so grow & retry */
			zs_reserve(zout, (unsigned int)(inleft * expand + 6 + zs_allocsize(zout)));
		} else {
			/* unconvertible input character */
			/* append placeholder & skip over */
			size_t wid = 1;
			if (eqstr(src, "UTF-8")) {
				wid = utf8len(*inptr);
			}
			if (wid > inleft)
				wid = inleft;
			inptr += wid;
			inleft -= wid;
			/* Following code is only correct for UCS-2LE, UCS-4LE */
			if (chwidth == 2)
			{
				unsigned short * u = (unsigned short *)outptr;
				*u = illegal;
				outptr += sizeof(u);
			}
			else if (chwidth == 4)
			{
				unsigned int * u = (unsigned int *)outptr;
				*u = illegal;
				outptr += sizeof(u);
			}
			else
			{
				*outptr++ = illegal;
			}
			zs_set_len(zout, outptr-zs_str(zout));
		}
		/* update output variables */
		/* (may have reallocated, plus need to point to end */
		outptr = zs_str(zout)+zs_len(zout);
		outleft = zs_allocsize(zout)-zs_len(zout)-4;
		if (inleft)
			goto cvting;
	}

icvt_terminate_and_exit:
	/* zero-terminate with appropriately wide zero */
	if (chwidth > 1) {
		*outptr++=0;
		if (chwidth > 2) {
			*outptr++=0;
			*outptr++=0;
		}
	}
	*outptr=0;
	zs_set_len(zout, outptr-zs_str(zout));

	iconv_close(ict);
	return TRUE;
#else
	src=src; /* unused */
	dest=dest; /* unused */
	sin=sin; /* unused */
	zout=zout; /* unused */
	illegal=illegal; /* unused */
	return FALSE;
#endif /* HAVE_ICONV */
}
/*===================================================
 * init_win32_iconv_shim -- Helper for loading iconv.dll on win32
 *=================================================*/
void
init_win32_iconv_shim (CNSTRING dllpath)
{
#ifdef WIN32_ICONV_SHIM
	if (dllpath && dllpath[0])
		iconvshim_set_property("dll_path", dllpath);
#else
	dllpath=dllpath; /* unused */
#endif
}
