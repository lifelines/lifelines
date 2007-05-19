/* 
   Copyright (c) 2000-2002 Perry Rapp
   Copyright (c) 2003 Matt Emmerton
   "The MIT license"

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * ui.h -- UI function prototypes
 * Copyright(c) 2002 by Perry Rapp; all rights reserved
 * Copyright(c) 2003 by Matt Emmerton; all rights reserved
 *===========================================================*/

#include "llstdlib.h"

#ifndef _INDISEQ_H
#include "indiseq.h"
#endif

#include "feedback.h"

/* Prototypes */
INT ask_for_char(CNSTRING ttl, CNSTRING prmpt, CNSTRING ptrn);
INT ask_for_char_msg(CNSTRING msg, CNSTRING ttl, CNSTRING prmpt, CNSTRING ptrn);
BOOLEAN ask_for_db_filename (CNSTRING ttl, CNSTRING prmpt, CNSTRING basedir, STRING buffer, INT buflen);
BOOLEAN ask_for_filename_impl(STRING ttl, STRING path, STRING prmpt, STRING buffer, INT buflen);
BOOLEAN ask_for_input_filename (STRING ttl, STRING path, STRING prmpt, STRING buffer, INT buflen);
BOOLEAN ask_for_output_filename (STRING ttl, STRING path, STRING prmpt, STRING buffer, INT buflen);
BOOLEAN ask_for_program (STRING mode, STRING ttl, STRING *pfname, STRING *pfullpath, STRING path, STRING ext, BOOLEAN picklist);
BOOLEAN ask_for_string (CNSTRING ttl, CNSTRING prmpt, STRING buffer, INT buflen);
BOOLEAN ask_for_string2 (CNSTRING ttl1, CNSTRING ttl2, CNSTRING prmpt, STRING buffer, INT buflen);
BOOLEAN ask_yes_or_no (STRING ttl);
BOOLEAN ask_yes_or_no_msg (STRING msg, STRING ttl);
void call_system_cmd (STRING cmd);
INT choose_from_array (STRING ttl, INT no, STRING *pstrngs);
INT choose_from_list (STRING ttl, LIST list);
INT choose_list_from_indiseq (STRING ttl, INDISEQ seq);
INT choose_one_from_indiseq (STRING ttl, INDISEQ seq);
INT choose_one_or_list_from_indiseq(STRING ttl, INDISEQ seq, BOOLEAN multi); /* XXX */
INT choose_or_view_array(STRING ttl, INT no, STRING *pstrngs, BOOLEAN selectable);
void llvwprintf (STRING fmt, va_list args);
void llwprintf (char *fmt, ...);
void msg_error (char *fmt, ...);
void msg_info (char *fmt, ...);
void msg_status (char *fmt, ...);
void msg_output (MSG_LEVEL level, STRING fmt, ...);
INT msg_width (void);
INT prompt_stdout (STRING prompt);
void refresh_stdout (void);
void rpt_print (STRING str);
void view_array (STRING ttl, INT no, STRING *pstrngs);
BOOLEAN yes_no_value(INT c);
INDISEQ invoke_search_menu (void);

