/* 
   Copyright (c) 2003 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * rptui.c -- Wrappers for UI functions used by report interpreter
 *==============================================================*/


#include <time.h>
#include "llstdlib.h"
#include "liflines.h"
#include "rptui.h"

static time_t uitime=0;
static time_t begint=0;

/*=================================================
 * begin_ui -- begin a UI call from report interpreter
 *===============================================*/
static void
begin_ui (void)
{
	uilocale();
	begint = time(NULL);
}
/*=================================================
 * end_ui -- finish a UI call & return to report interpreter
 *===============================================*/
static void
end_ui (void)
{
	rptlocale();
	uitime += time(NULL) - begint;
	begint = 0;
}
/*=================================================
 * rptui_init -- begin UI timing for report
 *===============================================*/
void
rptui_init (void)
{
	uitime = 0;
}
/*=================================================
 * rptui_elapsed -- report elapsed UI time for report
 *===============================================*/
int
rptui_elapsed (void)
{
	return (int)uitime;
}
/*==========================================================
 * Wrappers for ui functions
 *========================================================*/
RECORD
rptui_ask_for_fam (STRING s1, STRING s2)
{
	RECORD rec;
	begin_ui();
	rec = ask_for_fam(s1, s2);
	end_ui();
	return rec;
}
INDISEQ
rptui_ask_for_indi_list (STRING ttl, BOOLEAN reask)
{
	INDISEQ seq;
	begin_ui();
	seq = ask_for_indi_list(ttl, reask);
	end_ui();
	return seq;
}
STRING
rptui_ask_for_indi_key (STRING ttl, CONFIRMQ confirmq, ASK1Q ask1)
{
	STRING s;
	begin_ui();
	s = ask_for_indi_key(ttl, confirmq, ask1);
	end_ui();
	return s;
}
BOOLEAN
rptui_ask_for_int (STRING ttl, INT * prtn)
{
	BOOLEAN b;
	begin_ui();
	b = ask_for_int(ttl, prtn);
	end_ui();
	return b;
}
FILE *
rptui_ask_for_output_file (STRING mode, STRING ttl, STRING *pfname
	, STRING *pfullpath, STRING path, STRING ext)
{
	FILE * fp;
	begin_ui();
	fp = ask_for_output_file(mode, ttl, pfname, pfullpath, path, ext);
	end_ui();
	return fp;
}
BOOLEAN
rptui_ask_for_program (STRING mode, STRING ttl, STRING *pfname
	, STRING *pfullpath, STRING path, STRING ext, BOOLEAN picklist)
{
	BOOLEAN b;
	begin_ui();
	b = ask_for_program(mode, ttl, pfname, pfullpath, path, ext, picklist);
	end_ui();
	return b;
}
INT
rptui_choose_from_array (STRING ttl, INT no, STRING *pstrngs)
{
	INT i;
	begin_ui();
	i = choose_from_array(ttl, no, pstrngs);
	end_ui();
	return i;
}
INT
rptui_prompt_stdout (STRING prompt)
{
	INT i;
	begin_ui();
	i = prompt_stdout(prompt);
	end_ui();
	return i;
}
void
rptui_view_array (STRING ttl, INT no, STRING *pstrngs)
{
	begin_ui();
	view_array(ttl, no, pstrngs);
	end_ui();
}
