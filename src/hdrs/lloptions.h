/* 
   lloptions.h
   Copyright (c) 2000-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*
 lloptions.h - handling for (opaque) string & numeric options

  Options are fetched on-the-fly when requested
   first from report properties (if getoptstr_rpt or getoptint_rpt)
   then from db properties
   then from global properties
   then from fallback properties
*/

#ifndef LLOPTIONS_H_INCLUDED
#define LLOPTIONS_H_INCLUDED

#ifndef TABLE_H_INCLUDED
typedef struct table_s *TABLE;
#endif

/* initialization & termination */
void get_db_options(TABLE opts); /* free with FREEBOTH */
void get_global_options(TABLE opts); /* free with FREEBOTH */
BOOLEAN load_global_options(STRING configfile, STRING * pmsg);
void register_notify(CALLBACK_FNC fncptr);
void set_db_options(TABLE opts);
void set_global_options(TABLE opts);
void setoptstr_fallback(STRING optname, STRING newval);
void term_lloptions(void);
void unregister_notify(CALLBACK_FNC fncptr);


/* use */
/* TODO: fix const-correctness */
STRING getoptstr(STRING optname, STRING defval);
INT getoptint(STRING optname, INT defval);
STRING getoptstr_dbonly(STRING optname, STRING defval);
STRING getoptstr_rpt(STRING optname, STRING defval);



#endif /* OPTIONS_H_INCLUDED */
