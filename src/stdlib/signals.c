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

char *sig_msgs[] = {
	N_("SIGNAL 0"),
	N_("HANGUP"),
	N_("INTERRUPT"),
	N_("QUIT"),
	N_("ILLEGAL INSTRUCTION"),
	N_("TRACE TRAP"),
	N_("ABORT"),
	N_("EMT INST"),
	N_("FLOATING POINT EXCEPTION"),
	N_("KILL"),
	N_("BUS ERROR"),
	N_("SEGMENTATION ERROR"),
	N_("SYSTEM CALL ERROR"),
	N_("PIPE WRITE"),
	N_("ALARM CLOCK"),
	N_("TEMINATE FROM KILL"),
	N_("USER SIGNAL 1"),
	N_("USER SIGNAL 2"),
	N_("DEATH OF CHILD"),
	N_("POWER-FAIL RESTART"),
	N_("WINDOW CHANGE"),
};

static void on_signals(int);

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

	/* We don't know whether curses is up or not right now */
	/* so we build the report msg, then close curses, then print it */

	if (progrunning) {
		char line[20];
		snprintf(line, sizeof(line), "%d", iline(Pnode));
		sprintpic2(msg, sizeof(msg)
			, _("Looks like a program was running.\nCheck file %1 around line %2.\n")
			, ifname(Pnode), line);
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
		signame = N_("Unknown signal");
	sprintpic2(msg, sizeof(msg), _("signal %1: %2"), signum
		, _(signame)); 
	ll_abort(msg);
}
/*================================
 * ll_abort -- print msg & stop
 *  caller translated msg
 *===============================*/
void
ll_abort (STRING sigdesc)
{
	int c;
	if (sigdesc)
		printf(sigdesc);
	printf(_("\nAborting now. Core dump? [y/n]"));
	fflush(stdout);
	c = getchar();
	putchar(c);
	/* TODO: how do we i18n this ? This getchar assumes that 
	the answer is one byte */
	if((c == 'y') || (c == 'Y')) abort();
	exit(1);
}
