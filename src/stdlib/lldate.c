/* 
   lldate.c
   Copyright (c) 2001 Perry Rapp
   Created: 2001/02/04 for metadata for LifeLines & Ethel programs

   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <time.h>
#include "llstdlib.h"

/*===============================
 * get_current_lldate -- fill in ISO style string for current time
 *=============================*/
void
get_current_lldate (LLDATE * creation)
{
	struct tm *pt;
	time_t curtime;
	curtime = time(NULL);
	pt = gmtime(&curtime);
	sprintf(creation->datestr, "%04d-%02d-%02d-%02d:%02d:%02dZ", 
		pt->tm_year+1900, pt->tm_mon+1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec);
}
