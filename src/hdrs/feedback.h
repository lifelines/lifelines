/* 
   Copyright (c) 2001 Perry Rapp

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
/*==============================================================
 * feedback.h -- Header file for I/O feedback needed by non-ui code
 * Copyright (c) 2001 by Perry Rapp; all rights reserved
 *============================================================*/

#ifndef _FEEDBACK_H
#define _FEEDBACK_H

/* Ways for engine & code to report to ui */
	/* report an error */
void msg_error(STRING fmt, ...);
	/* report a message */
void msg_info(STRING fmt, ...);
	/* report transitory state that should not be preserved */
void msg_status(STRING fmt, ...);
	/* legacy */
#define message(str) msg_error(str)
	/* report to stdout style output (uses embedded carriage returns */
void llwprintf(STRING fmt, ...);

/* called by signal handler before invoking exit() */
void shutdown_ui(BOOLEAN pause);


/* called by edit routines for translation maps &
 edit routines for tables for user options & abbreviations */
void do_edit(void);

/* editvtab calls this */
BOOLEAN ask_yes_or_no_msg(STRING, STRING);

#endif /* _FEEDBACK_H */
