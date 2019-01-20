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
 * version.c -- Generate and display version information
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 29 Aug 93
 *   3.0.0 - 05 Oct 94    3.0.2 - 09 Nov 94
 *   3.0.3 - 21 Sep 95
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "arch.h"
#include "version.h"

/*===============================================
 * get_lifelines_version -- Return version string
 *  returns static buffer
 *=============================================*/
STRING
get_lifelines_version (INT maxlen)
{
        static char version[48];
        INT len=sizeof(version);
        if (len>maxlen)
                len=maxlen;
        llstrncpyf(version, len, 0, "%s %s", LIFELINES_VERSION, LIFELINES_VERSION_EXTRA);
        return version;
}

/*===============================================
 * print_version -- display program version
 *  displays to stdout
 *=============================================*/
void
print_version (CNSTRING program)
{
        printf("%s (lifelines) %s\n", program, get_lifelines_version(48));
        printf("\n");

        printf(_("Copyright (C) 1991-2019 Thomas T. Wetmore IV and contributors."));
        printf("\n");
        printf(_("This is free software; see the source for copying conditions.  There is NO\n"
                "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."));
        printf("\n");

        printf(_("For more information, visit our web site at http://marcno.github.io/lifelines/"));
        printf("\n");

}
