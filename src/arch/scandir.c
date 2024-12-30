/* -*- Mode: C; c-file-style: "gnu" -*- */
/*
   Copyright (c) 2000 Petter Reinholdtsen

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
 * scandir.c -- if scandir() is missing, make a replacement
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef HAVE_SCANDIR

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "arch.h"
#include "standard.h"

#ifdef HAVE_DIRENT_H
#  include <dirent.h>
#endif

#ifdef HAVE_WINDOWS_H

#include <windows.h>

/*
 * MS-Windows version of scandir (uses FindFirst...)
 */
int
scandir (const char *dir, struct dirent ***namelist,
        int (*select)(const struct dirent *),
        int (*compar)(const struct dirent **, const struct dirent **))
{
  WIN32_FIND_DATA file_data;
  HANDLE handle;
  int pos = 0;
  struct dirent **names;
  struct dirent cur;
  struct dirent *current = &cur;
  char *pattern;

  cur.d_reclen = sizeof(struct dirent);

  /* 3 for "*.*", 1 for "\", 1 for zero termination */
  pattern = (char*)malloc(strlen(dir) + 3 +1 +1);
  if (NULL == pattern)
    return -1;

  strcpy(pattern, dir);
  if (pattern[ strlen(pattern) - 1] != '\\')
    strcat(pattern, "\\");
  strcat(pattern, "*.*");

  handle = FindFirstFile(pattern, &file_data);
  if (handle == INVALID_HANDLE_VALUE)
  {
      free(pattern);
      return -1;
  }

  while (1)
  {
      int rtn;

      strcpy(current->d_name, file_data.cFileName);

      if (!select || select(current))
      {
          struct dirent **newnames = realloc(names, sizeof(*names) * (pos + 1));
          if (NULL == newnames)
              continue;

          names = newnames;

          struct dirent *copyentry = malloc(current->d_reclen);
          if (NULL == copyentry)
             continue;

          memcpy(copyentry, &current, current->d_reclen);

          names[pos] = copyentry;
          pos++;
        }

      rtn = FindNextFile(handle, &file_data);
      if (!rtn || rtn==ERROR_NO_MORE_FILES)
        break;
  }

  free(pattern);

  /* Now sort them */
  if (compar)
    qsort(names, pos, sizeof(names[0]), compar);

  *namelist = names;
  return pos;
}
#else
int
scandir(const char *dir, struct dirent ***namelist,
        int (*select)(const struct dirent *),
        HINT_PARAM_UNUSED int (*compar)(const struct dirent **, const struct dirent **))
{
  DIR *d = opendir(dir);
  struct dirent *current;
  struct dirent **names = NULL;
  int pos = 0;

  if (NULL == d)
    return -1;

  while (NULL != (current = readdir(d)))
  {
    if ( NULL == select || select(current) )
    {
      struct dirent **newnames = realloc(names, sizeof(*names) * (pos + 1));
      if (NULL == newnames)
        continue;

      names = newnames;

      struct dirent *copyentry = malloc(current->d_reclen);
      if (NULL == copyentry)
        continue;

      memcpy(copyentry, current, current->d_reclen);

      names[pos] = copyentry;
      pos++;
    }
  }
  closedir(d);

  *namelist = names;

  return pos;
}
#endif
#endif /* HAVE_SCANDIR */
