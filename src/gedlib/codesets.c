/* 
   Copyright (c) 2000-2001 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * codesets.c -- Manage the various codesets we work with
 *   Created: 2002/11 by Perry Rapp
 *==============================================================*/


#include "llstdlib.h"
#include "lloptions.h"

#ifdef HAVE_LANGINFO_CODESET
# include <langinfo.h>
#else
# include "langinfz.h"
#endif

#ifdef WIN32
/* need the Win32-specific codeset stuff in mycurses */
#include "curses.h"
#endif

/*********************************************
 * global/exported variables
 *********************************************/

/* internal codeset of current database */
BOOLEAN uu8=0;            /* flag if internal codeset is UTF-8 */
STRING int_codeset=0;     /* internal codeset */

STRING editor_codeset_out=0; /* output to editor */
STRING editor_codeset_in=0;  /* input from editor */
STRING gedcom_codeset_out=0; /* output GEDCOM files */
STRING gedcom_codeset_in=0;  /* default for reading GEDCOM files */
STRING gui_codeset_in=0;     /* reading characters from GUI */
STRING gui_codeset_out=0;    /* writing characters to GUI */
STRING report_codeset_out=0; /* default for report output */
STRING report_codeset_in=0;  /* default for input from reports */


/*********************************************
 * external/imported variables
 *********************************************/

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */


/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=================================================
 * init_codesets -- initialize all codeset variables
 *  config file needs to have been loaded at this point
 * Created: 2002/11/27 (Perry Rapp)
 *===============================================*/
void
init_codesets (void)
{
	STRING e;
#ifndef WIN32
	char wincs[32];
	STRING defval = wincs;
	int n = w_get_codepage();
	sprintf(wincs, "CP%d", n);
#else
	STRING defval = nl_langinfo (CODESET);
	/*
	We are using Markus Kuhn's emulator for systems without nl_langinfo
	see arch/langinfo.c
	An alternative would be to use localcharset.c, but it isn't as easy 
	to use; you have to configure config.aliases. Anyway, I have no idea
	if anyone needs this.
	*/
	/*
	The Win32 case is special because we care about the Console codepage
	as distinct from the general Windows one.
	*/
#endif

/* internal */
	/* TODO: Move int_codeset code to here */
	/* have to figure out exactly when this happens, as int_codeset needs db loaded */

/* GuiCodesetOut */
	e = getoptstr("GuiCodesetOut", "");
	if (!e)
		e = getoptstr("GuiCodeset", "");
	if (!e) {
#ifdef WIN32
		char temp[32];
		int cs = w_get_oemout_codepage();
		sprintf(temp, "CP%d", cs);
		e = temp;
#else
		e = defval;
#endif
	}
	if (e) {
		strfree(&gui_codeset_out);
		gui_codeset_out = strsave(e);
	}

/* GuiCodesetIn */
	e = getoptstr("GuiCodesetIn", "");
	if (!e)
		e = getoptstr("GuiCodeset", "");
	if (!e) {
#ifdef WIN32
		char temp[32];
		int cs = w_get_oemin_codepage();
		sprintf(temp, "CP%d", cs);
		e = temp;
#else
		e = defval;
#endif
	}
	if (e) {
		strfree(&gui_codeset_in);
		gui_codeset_in = strsave(e);
	}

/* GedcomCodesetOut */
	e = getoptstr("GedcomCodesetOut", "");
	if (!e)
		e = getoptstr("GedcomCodesetCodeset", "");
	if (!e)
		e = defval;
	if (e) {
		strfree(&gedcom_codeset_out);
		gedcom_codeset_out = strsave(e);
	}

/* GedcomCodesetIn */
	e = getoptstr("GedcomCodesetIn", "");
	if (!e)
		e = getoptstr("GedcomCodeset", "");
	if (!e)
		e = defval;
	if (e) {
		strfree(&gedcom_codeset_in);
		gedcom_codeset_in = strsave(e);
	}


}

/*=================================================
 * term_codesets -- free all codeset variables
 * Created: 2002/11/27 (Perry Rapp)
 *===============================================*/
void
term_codesets (void)
{
	strfree(&gui_codeset_out);
	strfree(&gui_codeset_in);
}
