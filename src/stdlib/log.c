/* 
   Copyright (c) 2003 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * log.c -- Very simple logging function
 *==============================================================*/

#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#include "arch.h" /* vsnprintf */
#include "log.h"

/*===============================
 * log_outf -- Append string to log file, with date prefix and \n suffix
 *=============================*/
void
log_outf (const char * filepath, const char * fmt, ...)
{
	va_list args;
	FILE * fp = 0;
	char buffer[4096];
	LLDATE creation;

	fp = fopen(filepath, LLAPPENDTEXT);
	if (!fp) return;

	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	
	get_current_lldate(&creation);
	fprintf(fp, "%s: %s\n", creation.datestr, buffer);

	fclose(fp);

	va_end(args);
}

