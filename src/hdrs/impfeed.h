/* 
   Copyright (c) 2002 Perry Rapp

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
 * feedback.h -- Header file for I/O feedback from
 *  GEDCOM import/export code
 * Copyright (c) 2002 by Perry Rapp; all rights reserved
 *============================================================*/

#ifndef _IMPFEED_H
#define _IMPFEED_H

struct import_feedback {
	void (*validation_warning_fnc)(STRING msg);
	void (*validation_error_fnc)(STRING msg);
	void (*error_invalid_fnc)(STRING reason);
	void (*validating_fnc)(void);
	void (*validated_rec_fnc)(char ctype, STRING tag, INT count);
	void (*beginning_import_fnc)(STRING msg);
	void (*error_readonly_fnc)(void);
	void (*adding_unused_keys_fnc)(void);
	void (*import_done_fnc)(INT nindi, INT nfam, INT nsour, INT neven, INT nothr);
	void (*added_rec_fnc)(char ctype, STRING tag, INT count);
};

#endif /* _IMPFEED_H */

