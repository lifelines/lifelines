/* 
   lloptions.h
   Copyright (c) 2000-2001 Perry Rapp
   Created: 2000/12, Perry Rapp
   Brought into repository: 2001/02/04

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
 lloptions.h - handling for (opaque) string & numeric options
 Copyright (c) 2000-2001 by Perry Rapp; all rights reserved
  Added to repository during 3.0.6 development

  Options are fetched on-the-fly when requested
   first from report properties (if getoptstr_rpt or getoptint_rpt)
   then from db properties
   then from global properties
   then from fallback properties
*/

#ifndef _OPTIONS_H
#define _OPTIONS_H

typedef struct table_s *TABLE;
typedef void (*options_notify_fnc)(void);

/* initialization & termination */
BOOLEAN load_global_options(STRING configfile, STRING * pmsg);
void register_notify(options_notify_fnc fncptr);
void unregister_notify(options_notify_fnc fncptr);
void term_lloptions(void);
void get_db_options(TABLE dbopts); /* free with FREEBOTH */
void set_db_options(TABLE dbopts);
void setoptstr_fallback(STRING optname, STRING newval);


/* use */
STRING getoptstr(STRING optname, STRING defval);
INT getoptint(STRING optname, INT defval);
STRING getoptstr_dbonly(STRING optname, STRING defval);
STRING getoptstr_rpt(STRING optname, STRING defval);



#endif /* _OPTIONS_H */
