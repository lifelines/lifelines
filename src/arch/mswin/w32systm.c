/* 
   Copyright (c) 1996-2000 Paul B. McBride

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
 * w32systm.c -- replacement for system() that waits for program to complete
 *   Created: ? by Paul B. McBride (pmcbride@tiac.net)
 *   C RTL version of system does not correctly block on Win95,98
 *    (altho it is ok on WinNT)
 *==============================================================*/

#include <process.h>
#include <stdio.h>

int w32system(const char *cp)
{
  char argbuf[256];
  char *argv[32];
  char *tp;
  int argc;

  tp = argbuf;
  argc = 0;
  while(*cp) {
    while(*cp && (*cp == ' ')) cp++;
    if(*cp) {
      argv[argc++] = tp;
      if(*cp == '"') {
       cp++;
       while(*cp && (*cp != '"')) *tp++ = *cp++;
       if(*cp == '"') cp++;
      }
      else {
       while(*cp &&  (*cp != ' ')) *tp++ = *cp++;
      }
      *tp++ = '\0';
    }
  }
  argv[argc] = NULL;

  return(spawnvp(P_WAIT, argv[0], argv));
}
