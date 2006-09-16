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
 * ui_cli.c -- UI code for command-line interface (CLI), *not* curses
 * Copyright(c) 2002 by Perry Rapp; all rights reserved
 * Copyright(c) 2003 by Matt Emmerton; all rights reserved
 *===========================================================*/

#include "llstdlib.h"
#include "ui.h"

/*********************************************
 * external variables (no header)
 *********************************************/
extern STRING qSdefttl,qSiddefpath;
extern STRING qSaskynq,qSaskynyn,qSaskyY,qSaskint;
extern STRING qSchlistx,qSvwlistx;
extern INT screen_width;

/*********************************************
 * local function prototypes
 *********************************************/
static void outputln(const char * txt);
static void output(const char * txt);
static INT interact(CNSTRING ptrn);

/*=============================================================
 * Xprintf() implementations
 *===========================================================*/
void
llwprintf (char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
void
llvwprintf (STRING fmt, va_list args)
{
	vprintf(fmt, args);
}
/*=============================================================
 * Report output functions
 *===========================================================*/
void
rpt_print (STRING str)
{
	printf(str);
}
void
refresh_stdout (void)
{
	/* We don't need to do anything as we're using stdout */
}
/*=============================================================
 * Message output functions
 *===========================================================*/
void
msg_error (char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
void
msg_info (char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
void
msg_status (char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
void
msg_output (MSG_LEVEL level, STRING fmt, ...)
{
	va_list args;
	level=level;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
INT
msg_width (void)
{
	/* arbitrarily high number */
	return 999;
}
/*=============================================================
 * MTE: this really belongs in stdlib/
 *===========================================================*/
void
call_system_cmd (STRING cmd)
{
#ifndef WIN32
	system("clear");
#endif
	system(cmd);
}
/*=============================================================
 * ASK Routines
 *===========================================================*/
BOOLEAN
ask_for_program (STRING mode,
                 STRING ttl,
                 STRING *pfname,
                 STRING *pfullpath,
                 STRING path,
                 STRING ext,
                 BOOLEAN picklist)
{
	mode = mode;		/* NOTUSED */
	ttl = ttl;		/* NOTUSED */
	pfname = pfname;	/* NOTUSED */
	pfullpath = pfullpath;	/* NOTUSED */
	path = path;		/* NOTUSED */
	ext = ext;		/* NOTUSED */
	picklist = picklist;	/* NOTUSED */

	/* TODO: We probably want to use the real implementation in askprogram.c */
	return FALSE;
}
BOOLEAN
ask_for_string (CNSTRING ttl, CNSTRING prmpt, STRING buffer, INT buflen)
{
	outputln(ttl);
	printf(prmpt);
	fgets(buffer, buflen, stdin);
	chomp(buffer);
	return strlen(buffer)>0;
}
BOOLEAN
ask_for_string2 (CNSTRING ttl1, CNSTRING ttl2, CNSTRING prmpt, STRING buffer, INT buflen)
{
	outputln(ttl1);
	return ask_for_string(ttl2, prmpt, buffer, buflen);
}
INT
ask_for_char (CNSTRING ttl, CNSTRING prmpt, CNSTRING ptrn)
{
	return ask_for_char_msg(NULL, ttl, prmpt, ptrn);
}
INT
ask_for_char_msg (CNSTRING msg, CNSTRING ttl, CNSTRING prmpt, CNSTRING ptrn)
{
	INT rv;
	if (msg) outputln(msg);
	if (ttl) outputln(ttl);
	output(prmpt);
	rv = interact(ptrn);
	return rv;
}
BOOLEAN
ask_yes_or_no (STRING ttl)
{
	INT c = ask_for_char(ttl, _(qSaskynq), _(qSaskynyn));
	return yes_no_value(c);
}
BOOLEAN
ask_yes_or_no_msg (STRING msg, STRING ttl)
{
	INT c = ask_for_char_msg(msg, ttl, _(qSaskynq), _(qSaskynyn));
	return yes_no_value(c);
}
BOOLEAN
ask_for_db_filename (CNSTRING ttl, CNSTRING prmpt, CNSTRING basedir, STRING buffer, INT buflen)
{
	basedir = basedir;	/* NOTUSED */
	return ask_for_string(ttl, prmpt, buffer, buflen);
}
BOOLEAN
ask_for_output_filename (STRING ttl, STRING path, STRING prmpt, STRING buffer, INT buflen)
{
	/* curses version doesn't differentiate input from output prompts */
	return ask_for_filename_impl(ttl, path, prmpt, buffer, buflen);
}
BOOLEAN
ask_for_input_filename (STRING ttl, STRING path, STRING prmpt, STRING buffer, INT buflen)
{
	/* curses version doesn't differentiate input from output prompts */
	return ask_for_filename_impl(ttl, path, prmpt, buffer, buflen);
}
BOOLEAN
ask_for_filename_impl (STRING ttl, STRING path, STRING prmpt, STRING buffer, INT buflen)
{
	/* display current path (truncated to fit) */
	char curpath[120];
	INT len = sizeof(curpath);
	if (len > screen_width-2)
		len = screen_width-2;
	curpath[0] = 0;
	llstrapps(curpath, len, uu8, _(qSiddefpath));
	llstrapps(curpath, len, uu8, compress_path(path, len-strlen(curpath)-1));

	return ask_for_string2(ttl, curpath, prmpt, buffer, buflen);
}
/*=============================================================
 * CHOOSE Routines
 *===========================================================*/
INT
choose_from_array (STRING ttl, INT no, STRING *pstrngs)
{
	BOOLEAN selectable = TRUE;
	return choose_or_view_array(ttl, no, pstrngs, selectable);
}
void
view_array (STRING ttl, INT no, STRING *pstrngs)
{
	BOOLEAN selectable = FALSE;
	choose_or_view_array(ttl, no, pstrngs, selectable);
}
INT
choose_from_list (STRING ttl, LIST list)
{
	STRING * array=0;
	STRING choice=0;
	INT i=0, rtn=-1;
	INT len = length_list(list);

	if (len < 1) return -1;
	if (!ttl) ttl=_(qSdefttl);

	array = (STRING *) stdalloc(len*sizeof(STRING));
	i = 0;
	FORXLIST(list, el)
		choice = (STRING)el;
		ASSERT(choice);
		array[i] = strsave(choice);
		++i;
	ENDXLIST

	rtn = choose_from_array(ttl, len, array);

	for (i=0; i<len; ++i)
		strfree(&array[i]);
	stdfree(array);
	return rtn;
}
INT
choose_list_from_indiseq (STRING ttl, INDISEQ seq)
{
	return choose_one_or_list_from_indiseq(ttl, seq, TRUE);
}
INT
choose_one_or_list_from_indiseq (STRING ttl, INDISEQ seq, BOOLEAN multi)
{
	ttl = ttl;	/* NOTUSED */
	multi = multi;	/* NOTUSED */

	calc_indiseq_names(seq); /* we certainly need the names */

	/* TODO: imitate choose_from_list & delegate to array chooser */
	return 0;
}
INT
choose_one_from_indiseq (STRING ttl, INDISEQ seq)
{
	return choose_one_or_list_from_indiseq(ttl, seq, FALSE);
}
INT
choose_or_view_array (STRING ttl, INT no, STRING *pstrngs, BOOLEAN selectable)
{
	/* TODO: The q ought to be localized */
	STRING promptline = selectable ? _(qSchlistx) : _(qSvwlistx);
	STRING responses = selectable ? "0123456789udq" : "udq";
	INT i=0;

	ttl = ttl;	/* NOTUSED */

	while (1) {
		INT j;
		INT rv;
		for (j=i; j<i+10 && j<no; ++j) {
			printf("%ld: %s\n", j-i, pstrngs[j]);
		}
		printf("%s\n", promptline);
		rv = interact(responses);
		switch(rv) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			rv = i+rv-'0';
			if (selectable && rv < no) {
				return rv;
			}
			break;
		case 'd':
			if (i+10 < no)
				i += 10;
			break;
		case 'u':
			if (i>9)
				i -= 10;
			break;
		case 'q': return -1;
		}
	}
}

/*=============================================================
 * Misc Routines
 *===========================================================*/
BOOLEAN
yes_no_value (INT c)
{
	STRING ptr;
	for (ptr = _(qSaskyY); *ptr; ptr++) {
		if (c == *ptr) return TRUE;
	}
	return FALSE;
}
INT
prompt_stdout (STRING prompt)
{
	return ask_for_char(NULL, prompt, NULL);
}
/* called from ask.c, curses version in searchui.c */
INDISEQ
invoke_search_menu (void)
{
	return NULL;
}
/*=============================================================
 * Internal Use Only
 *===========================================================*/
/* send string to output, & terminate line */
static void
outputln (const char * txt)
{
	printf(txt);
	printf("\n");
}
/* send string to output */
static void
output (const char * txt)
{
	printf(txt);
}
static INT
interact (CNSTRING ptrn)
{
	char buffer[8];
	CNSTRING t=0;
	while (1) {
		fgets(buffer, sizeof(buffer), stdin);
		if (!ptrn) return buffer[0];
		for (t=ptrn; *t; ++t) {
			if (buffer[0]==*t)
				return buffer[0];
		}
		printf("Invalid option: choose one of %s\n", ptrn);
	}
}

