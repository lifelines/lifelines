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
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 30 Jul 94    3.0.2 - 10 Nov 94
 *   3.0.3 - 19 Sep 95
 *===========================================================*/

#include <signal.h>
#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "interp.h"

#define NUM_SIGNALS 21
char *sig_msgs[] = {
	"SIGNAL 0",
	"HANGUP",
	"INTERRUPT",
	"QUIT",
	"ILLEGAL INSTRUCTION",
	"TRACE TRAP",
	"ABORT",
	"EMT INST",
	"FLOATING POINT EXCEPTION",
	"KILL",
	"BUS ERROR",
	"SEGMENTATION ERROR",
	"SYSTEM CALL ERROR",
	"PIPE WRITE",
	"ALARM CLOCK",
	"TEMINATE FROM KILL",
	"USER SIGNAL 1",
	"USER SIGNAL 2",
	"DEATH OF CHILD",
	"POWER-FAIL RESTART",
	"WINDOW CHANGE",
};
/*======================================
 * set_signals -- Install signal handler
 *====================================*/
set_signals ()
{
	extern void on_signals();
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
void on_signals (sig)
int sig;
{
	extern BOOLEAN progrunning;
	extern PNODE Pnode;

	if (progrunning) {
		llwprintf("Looks like you were running a program.\n");
		llwprintf("Check file \"%s\" around line %d.\n", ifname(Pnode),
		    iline(Pnode));
	}

	llwprintf("Exiting on signal %d:%s\n", sig, sig_msgs[sig]);
	close_lifelines();
	endwin();
	ll_abort(sig);
}

ll_abort(sig)
{
	int c;
	fputs("\nCore Dump? [n/y] ", stdout);
	fflush(stdout);
	c = getchar();
	putchar(c);
	if((c == 'y') || (c == 'Y')) abort();
	exit(1);
}
