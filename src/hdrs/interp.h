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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * interp.h - Header file for report interpreter
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *  2.3.4 - 24 Jun 93   2.3.5 - 16 Aug 93
 *  3.0.0 - 19 Jun 94   3.0.2 - 21 Dec 94
 *  3.0.3 - 07 Aug 95
 *===========================================================*/

#ifndef _INTERP_H
#define _INTERP_H

void poutput(STRING, BOOLEAN *eflg);
void interp_main(LIST lifiles, STRING ofile, BOOLEAN picklist, BOOLEAN timing);
void init_interpreter(void);
void shutdown_interpreter(void);
ZSTR get_report_error_msg (STRING msg);

extern BOOLEAN prog_trace;

#endif /* _INTERP_H */
