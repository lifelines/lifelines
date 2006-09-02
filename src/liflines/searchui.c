/* 
   Copyright (c) 2006 Perry Rapp

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
 * searchui.c -- menus for full database search
 *  (see scan.c for code that actually scans entire database)
 * Copyright(c) 2006
 *===========================================================*/

#include <time.h>
#include "llstdlib.h"
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "table.h"
#include "liflines.h"
#include "arch.h"
#include "lloptions.h"
#include "interp.h"
#include "llinesi.h"
#include "menuitem.h"
#include "screen.h"
#include "cscurses.h"
#include "zstr.h"
#include "cache.h"
#ifdef WIN32_ICONV_SHIM
#include "iconvshim.h"
#endif
#include "codesets.h"
#include "charprops.h"
