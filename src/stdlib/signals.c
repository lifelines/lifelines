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
 * signals.c -- Catch signals and exit
 * Copyright(c) 1993-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 30 Jul 94    3.0.2 - 10 Nov 94
 *   3.0.3 - 19 Sep 95
 *===========================================================*/

#include "llstdlib.h"
#include <signal.h>
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interp.h"
#include "liflines.h"
#include "feedback.h"

#include "stdlibi.h"


/*********************************************
 * external/imported variables
 *********************************************/
extern STRING qScoredump, qSprogsig, qSsignal,qSsigunk;
extern STRING qSsig00, qSsig01, qSsig02, qSsig03, qSsig04;
extern STRING qSsig05, qSsig06, qSsig07, qSsig08, qSsig09;
extern STRING qSsig10, qSsig11, qSsig12, qSsig13, qSsig14;
extern STRING qSsig15, qSsig16, qSsig17, qSsig18, qSsig19;
extern STRING qSsig20;
extern STRING qSaskyY;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static BOOLEAN is_yes(INT ch);
static void load_signames(void);
static void on_signals(int);

/*********************************************
 * local variables
 *********************************************/

static char *sig_msgs[20];

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*======================================
 * load_signames -- Load descriptive signal names
 *====================================*/
static void
load_signames (void)
{
	sig_msgs[ 0] = _(qSsig00);
	sig_msgs[ 1] = _(qSsig01);
	sig_msgs[ 2] = _(qSsig02);
	sig_msgs[ 3] = _(qSsig03);
	sig_msgs[ 4] = _(qSsig04);
	sig_msgs[ 5] = _(qSsig05);
	sig_msgs[ 6] = _(qSsig06);
	sig_msgs[ 7] = _(qSsig07);
	sig_msgs[ 8] = _(qSsig08);
	sig_msgs[ 9] = _(qSsig09);
	sig_msgs[10] = _(qSsig10);
	sig_msgs[11] = _(qSsig11);
	sig_msgs[12] = _(qSsig12);
	sig_msgs[13] = _(qSsig13);
	sig_msgs[14] = _(qSsig14);
	sig_msgs[15] = _(qSsig15);
	sig_msgs[16] = _(qSsig16);
	sig_msgs[17] = _(qSsig17);
	sig_msgs[18] = _(qSsig18);
	sig_msgs[19] = _(qSsig19);
	sig_msgs[20] = _(qSsig20);
}
/*======================================
 * set_signals -- Install signal handler
 *====================================*/
void
set_signals (void)
{
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, on_signals);
#ifdef SIGHUP
	signal(SIGHUP, on_signals);
#endif
#ifdef SIGQUIT
	signal(SIGQUIT, on_signals);
#endif
	signal(SIGILL, on_signals);
#ifdef SIGEMT
	signal(SIGEMT, on_signals);
#endif
	signal(SIGFPE, on_signals);
#ifdef SIGBUS
	signal(SIGBUS, on_signals);
#endif
	signal(SIGSEGV, on_signals);
#ifdef SIGSYS
	signal(SIGSYS, on_signals);
#endif
#ifdef SIGPIPE
	signal(SIGPIPE, on_signals);
#endif
}
/*======================================
 * on_signals -- Catch and handle signal
 *====================================*/
static void
on_signals (int sig)
{
	extern BOOLEAN progrunning;
	extern PNODE Pnode;
	char msg[100], signum[20];
	STRING signame;

	/* Ok, we'll want the descriptive name of the signal */
	load_signames();

	/* We don't know whether curses is up or not right now */
	/* so we build the report msg, then close curses, then print it */

	if (progrunning) {
		char line[20];
		snprintf(line, sizeof(line), "%d", iline(Pnode));
		sprintpic2(msg, sizeof(msg), uu8, _(qSprogsig), ifname(Pnode), line);
	}

	close_lifelines();
	shutdown_ui(TRUE); /* pause */
	/* now print report msg if we had one */
	if (msg[0])
		printf(msg);
	/* now build description of signal (# and name) */
	/* name is not translated til sprint'd into msg */
	snprintf(signum, sizeof(signum), "%d", sig);
	if (sig>=0 && sig<=ARRSIZE(sig_msgs))
		signame = sig_msgs[sig];
	else
		signame = _(qSsigunk);
	sprintpic2(msg, sizeof(msg), uu8, _(qSsignal), signum, signame); 
	ll_abort(msg);
}
/*================================
 * ll_abort -- print msg & stop
 *  caller translated msg
 *===============================*/
void
ll_abort (STRING sigdesc)
{
	INT ch;
	if (sigdesc)
		printf(sigdesc);
	printf(_(qScoredump));
	fflush(stdout);

	/* TODO: how do we i18n this ? This getchar assumes that 
	the answer is one byte */

	ch = getchar();
	putchar(ch);
	if (is_yes(ch))
		abort();
	else
		exit(1);
}
/*==================================================
 * is_yes -- is this an abbreviated yes answer ?
 *=================================================*/
static BOOLEAN
is_yes (INT ch)
{
	STRING ptr;
	for (ptr = _(qSaskyY); *ptr; ptr++) {
		if (ch == *ptr) return TRUE;
	}
	return FALSE;
}
