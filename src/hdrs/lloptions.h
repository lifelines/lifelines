/* 
   options.h
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
 options.h - metadata types for liflines records
 Copyright (c) 2000-2001 by Perry Rapp; all rights reserved
  Added to repository during 3.0.6 development

  Options read from config file & from database options
*/

#ifndef _OPTIONS_H
#define _OPTIONS_H

/*
  strings might be strsave'd("")
  but they are never NULL, and are always in heap
  after read_lloptions has been called
*/
struct lloptions_s {
  /* options set by either user's config or db user options */
	INT list_detail_lines;
	INT add_metadata;
	STRING email_addr;
	/* options only set by user's config */
	STRING lleditor;
	STRING llprograms;
	STRING llreports;
	STRING llarchives;
	STRING lldatabases;
	STRING llnewdbdir; /* location for new unqualified databases */
	STRING inputpath; /* prefix for unqualified utility/read paths */
	INT deny_system_calls; /* from within reports */
	STRING reportlog; /* report errors appended to this if non-blank */
	INT per_error_delay; /* sec delay for each report error */
	INT report_error_callstack; /* full call stack for report errors */
	INT date_customize_long; /* non-zero for customized display */
	INT date_long_dfmt;
	INT date_long_mfmt;
	INT date_long_yfmt;
	INT date_long_sfmt;
};

extern struct lloptions_s lloptions;

void read_lloptions_from_config(void);
void read_lloptions_from_db(void);
void changeoptstr(STRING * str, STRING newval);
void term_lloptions(void);
void cleanup_lloptions(void);

#endif /* _OPTIONS_H */
