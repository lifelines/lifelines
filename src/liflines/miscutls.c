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
/*=============================================================
 * miscutils.c -- Miscellaneous utility commands
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 15 Aug 93
 *   2.3.6 - 12 Oct 93    3.0.0 - 05 May 94
 *   3.0.2 - 08 Nov 94
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"

extern STRING btreepath;

/*======================================
 * key_util -- Return person's key value
 *====================================*/
key_util ()
{
	TRANTABLE ttd = tran_tables[MINDS];
	NODE indi = ask_for_indi("Whose key value do you want?", FALSE, FALSE);
	if (!indi) return;
	mprintf("%s - %s", rmvat(nxref(indi)), indi_to_name(indi, ttd, 70));
}
/*===================================================
 * who_is_he_she -- Find who person is from key value
 *=================================================*/
who_is_he_she ()
{
	STRING key, str, rec, ask_for_string();
	NODE indi;
	INT len;
	char nkey[100];
	TRANTABLE ttd = tran_tables[MINDS];

	key = ask_for_string("Please enter person's internal key value.",
	    "enter key:");
	if (!key || *key == 0) return;
	nkey[0] = 'I';
	if (*key == 'I')
		strcpy(nkey, key);
	else
		strcpy(&nkey[1], key);
	if (!(rec = retrieve_record(nkey, &len))) {
		mprintf("No one in database has key value %s.", key);
		return;
	}
	if (!(indi = string_to_node(rec))) {
		mprintf("No one in database has key value %s.", key);
		stdfree(rec);
		return;
	}
	if (!(str = indi_to_name(indi, ttd, 60)) || *str == 0) {
		mprintf("No one in database has key value %s.", key);
		stdfree(rec);
		return;
	}
	mprintf("%s - %s", key, str);
}
/*===========================================
 * show_database_stats -- Show database stats
 *=========================================*/
show_database_stats ()
{
	mprintf("Database `%s' contains (%dP, %dF, %dS, %dE, %dX) records.",
	    btreepath, num_indis(), num_fams(), num_sours(),
	    num_evens(), num_othrs());
}
