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
#ifdef HAVE_ICONV
# ifdef WIN32_ICONV_SHIM
#  include "win32/iconvshim.h"
# else
#  include <iconv.h>
# endif
#endif
#include "bfs.h"
#include "icvt.h"

/*===================================================
 * iconv_trans -- Translate string via iconv
 *  src:     [IN]  source codeset
 *  dest:    [IN]  string to translate (& delete)
 *  bfsIn:   [I/O]  output buffer
 *  success: [OUT] success flag (optional)
 * Only called if HAVE_ICONV
 *=================================================*/
bfptr
iconv_trans (CNSTRING src, CNSTRING dest, bfptr bfsIn, CNSTRING illegal, BOOLEAN * success)
{
#ifdef HAVE_ICONV
	bfptr bfsOut;
	iconv_t ict;
	const char * inptr;
	char * outptr;
	size_t inleft;
	size_t outleft;
	size_t cvted;

	ASSERT(src && dest);

	ict = iconv_open(dest, src);

	if (ict == (iconv_t)-1) {
		if (success)
			*success = FALSE;
		return bfsIn;
	}

	bfReserve(bfsIn, (int)(strlen(src)*1.3+2));

	bfsOut = bfNew(bfsIn->size);
	inptr = bfsIn->str;
	outptr = bfStr(bfsOut);
	inleft = bfLen(bfsIn);
	outleft = bfsOut->size-1;
	cvted = 0;

cvting:
	cvted = iconv (ict, &inptr, &inleft, &outptr, &outleft);
	if (cvted == (size_t)-1) {
		if (!outleft) {
			/* zero terminate & fix bfptr */
			*outptr=0;
			bfsOut->end = outptr;

			bfReserveExtra(bfsOut, (int)(inleft * 1.3+2));
			outleft = bfsOut->size - bfLen(bfsOut) - 1;
			goto cvting;
		} else {
			CNSTRING placeholder = illegal ? illegal : "%";
			if (eqstr(src, "UTF-8")) {
				inptr += utf8len(*inptr);
			} else {
				/*
				invalid multibyte sequence, but we don't know how long, so advance
				one byte & retry
				*/
				++inptr;
			}
			/* zero terminate & fix bfptr */
			*outptr=0;
			bfsOut->end = outptr;

			bfCat(bfsOut, placeholder);
			goto cvting;
		}
	}
	/* zero terminate & fix bfptr */
	*outptr=0;
	bfsOut->end = outptr;

	iconv_close(ict);
	bfDelete(bfsIn);
	if (success)
		*success = TRUE;
	return bfsOut;
#else
	if (success)
		*success = FALSE;
	src=src; /* unused */
	dest=dest; /* unused */
	return bfsIn;
#endif /* HAVE_ICONV */
}
