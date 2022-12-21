/*
   Copyright (c) 2022 Matthew Emmerton

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

/*
 * backtrace.c -- routines to generate stack backtraces
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <execinfo.h>
#include "llstdlib.h"

#define MAX_FRAMES 32

void
dump_backtrace(FILE* fp)
{
#ifdef HAVE_BACKTRACE
	int actual_num_frames = 0;
	void *frames[MAX_FRAMES];
	char **symbols = NULL;

	// collect backtrace (addresses)
	actual_num_frames = backtrace(frames, MAX_FRAMES);

#ifdef HAVE_BACKTRACE_SYMBOLS
	// resolve symbols
	symbols = backtrace_symbols(frames, actual_num_frames);
#endif

	// dump stack backtrace
	fprintf(fp,"Stack Backtrace\n");
	fprintf(fp,"---------------\n");
	if (actual_num_frames >= MAX_FRAMES)
	{
		fprintf(fp,"NOTE: Backtrace may be truncated.\n");
	}
	if (symbols == NULL)
	{
		fprintf(fp,"NOTE: Could not collect symbols.\n");
	}

	for (int frameno=0; frameno<actual_num_frames; frameno++)
	{
		fprintf(fp,"%d: %p %s\n",frameno,frames[frameno],symbols[frameno]?symbols[frameno]:"(none)");
	}
	fprintf(fp,"---------------\n");

	if (symbols) {
		free(symbols);
	}
#endif
	return;
}
