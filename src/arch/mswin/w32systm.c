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
#include <stdlib.h>
#include <string.h>

int w32system(const char *cp)
{
  char *argbuf = NULL;
  char **argv = NULL;
  char *tp = NULL;
  int argc = 0;
  int rval = -1;
  size_t len;
  size_t maxargv;

  if (!cp) return -1;

  len = strlen(cp);
  maxargv = len + 2;
  if ((argbuf = (char *)malloc(len + 1)) == NULL) goto done;
  if ((argv = (char **)malloc(maxargv * sizeof(*argv))) == NULL) goto done;

  tp = argbuf;
  while(*cp) {
    while(*cp && (*cp == ' ')) cp++;
    if(*cp) {
      if ((size_t)argc + 1 >= maxargv) goto done;
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

  if (!argc) goto done;
  argv[argc] = NULL;
  rval = spawnvp(P_WAIT, argv[0], argv);

done:
  if (argv) free(argv);
  if (argbuf) free(argbuf);
  return rval;
}
