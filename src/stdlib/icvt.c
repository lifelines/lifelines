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

#include "llstdlib.h"
#include <errno.h>
#ifdef HAVE_ICONV
# ifdef WIN32_ICONV_SHIM
#  include "win32/iconvshim.h"
# else
#  include <iconv.h>
# endif
#endif
#include "zstr.h"
#include "icvt.h"

/*===================================================
 * iconv_trans -- Translate string via iconv
 *  src:     [IN]  source codeset
 *  dest:    [IN]  string to translate (& delete)
 *  zin:     [I/O] source string (may be returned if iconv can't translate)
 *  success: [OUT] success flag (optional)
 * Only called if HAVE_ICONV
 *=================================================*/
ZSTR
iconv_trans (CNSTRING src, CNSTRING dest, ZSTR zin, CNSTRING illegal, BOOLEAN * success)
{
#ifdef HAVE_ICONV
	ZSTR zout;
	iconv_t ict;
	const char * inptr;
	char * outptr;
	size_t inleft;
	size_t outleft;
	size_t cvted;
	int transliterate=2; 
	double expand=1.3;
	int chwidth=1;

	ASSERT(src && dest);

	ict = iconv_open(dest, src);

	if (ict == (iconv_t)-1) {
		if (success)
			*success = FALSE;
		return zin;
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

	/* testing recursive transliteration in my private iconv, Perry, 2002.07.11 */
#ifdef ICONV_SET_TRANSLITERATE
	iconvctl(ict, ICONV_SET_TRANSLITERATE, &transliterate);
#endif

	zout = zs_newn((unsigned int)(zs_len(zin)*expand+6));

	inptr = zs_str(zin);
	outptr = zs_str(zout);
	inleft = zs_len(zin);
	/* we are terminating with 4 zero bytes just in case dest is UCS-4 */
	outleft = zs_allocsize(zout)-zs_len(zout)-4;
	cvted = 0;

cvting:
	/* main convert */
	cvted = iconv (ict, &inptr, &inleft, &outptr, &outleft);

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
			zs_reserve_extra(zout, (unsigned int)(inleft * expand + 6));
		} else {
			/* unconvertible input character */
			/* append placeholder & skip over */
			size_t wid = 1;
			CNSTRING placeholder = illegal ? illegal : "%";
			if (eqstr(src, "UTF-8")) {
				wid = utf8len(*inptr);
			}
			if (wid > inleft)
				wid = inleft;
			inptr += wid;
			inleft -= wid;
			zs_cat(zout, placeholder);
		}
		/* update output variables */
		/* (may have reallocated, plus need to point to end */
		outptr = zs_str(zout)+zs_len(zout);
		outleft = zs_allocsize(zout)-zs_len(zout)-4;
		goto cvting;
	}

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
	zs_free(zin);
	if (success)
		*success = TRUE;
	return zout;
#else
	if (success)
		*success = FALSE;
	src=src; /* unused */
	dest=dest; /* unused */
	return zin;
#endif /* HAVE_ICONV */
}
