/* 
   Copyright (c) 1996-2000 Paul B. McBride

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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/* mycurses.c - minimal curses library routines for Win32 LifeLines */

#include <windows.h>
#include <stdarg.h>
#include <fcntl.h>
#include <curses.h>

/* fullscreen==1 to use screen buffer size instead of window size */
static void mycur_init(int fullscreen);
static int mycur_getc();


/* Windows Console */

static int first = TRUE;
static HANDLE hStdin = INVALID_HANDLE_VALUE;
static HANDLE hStdout = INVALID_HANDLE_VALUE;
static DWORD dwModeIn = 0;
static DWORD dwModeOut = 0;
static CONSOLE_SCREEN_BUFFER_INFO sScreenInfo = {0};
static COORD cOrigin = {0,0};

/* Others Windows environment data */

extern int _fmode;		/* O_TEXT or O_BINARY */

/* Debugging */

/* #define DEBUG */

#ifdef DEBUG
static FILE *errfp = NULL;
#endif

/* Curses data */

static int echoing = TRUE;

static chtype myacsmap[256] = {
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*',
	'*', '*', '*', '*', '*', '*', '*', '*'
	};

int LINES = 25;
int COLS  = CUR_MAXCOLS;
chtype *acs_map = myacsmap;
WINDOW *curscr = NULL;
WINDOW *stdscr = NULL;

int endwin()
{
	COORD cCursor;

	cCursor.X = 0;
	cCursor.Y = LINES-1;
	SetConsoleCursorPosition(hStdout, cCursor);
	return(0);
}

void wtitle(const char *title)
{
	SetConsoleTitle(title);
}

WINDOW	*initscr()
{
	_fmode = O_BINARY;	/* use binary mode for reading and writing */
	/* open an error file for debugging */

#ifdef DEBUG
	if(errfp == NULL)
	 	errfp = fopen("mycurses.err", "w");
#endif

	mycur_init(1);
	if(curscr == NULL)
		{
		curscr = newwin(LINES, COLS, 0, 0);
		/* set update point so whole screen is displayed */
		curscr->_mincy = 0;
		curscr->_maxcy = curscr->_maxy-1;
		}
	if(stdscr == NULL) stdscr = newwin(LINES, COLS, 0, 0);
	return(stdscr);
}

WINDOW  *newwin(int nlines, int ncols, int begy, int begx)
{
	WINDOW *wp;
	static int wincnt = 0;

	wp = calloc(1, sizeof(WINDOW));
	wp->_num = wincnt++;
	if(nlines > LINES) nlines = LINES;
	if(ncols > COLS) ncols = COLS;
	wp->_maxy = nlines;
	wp->_maxx = ncols;
	if(begy < 0) begy = 0;
	if(begx < 0) begx = 0;
	wp->_begy = begy;
	wp->_begx = begx;
	wp->_curx = 0;
	wp->_cury = 0;
	wp->_scroll = TRUE;
	werase(wp);
	return(wp);
}

int delwin(WINDOW *wp)
{
	free(wp);
	return(0);
}

WINDOW  *subwin(WINDOW *wp, int nlines, int ncols, int begy, int begx)
{
	WINDOW *swp;
	swp = newwin(nlines, ncols, begy, begx);
	swp->_parent = wp;
	return(swp);
}

int crmode()
{
	SetConsoleMode(hStdin, dwModeIn);
	return(0);
}

int nocrmode()
{
	SetConsoleMode(hStdin, dwModeIn);
	return(0);
}

int cbreak()
{
	SetConsoleMode(hStdin, dwModeIn);
	return(0);
}

int nocbreak()
{
	SetConsoleMode(hStdin, dwModeIn);
	return(0);
}

int echo()
{
	echoing = TRUE;
	return(0);
}

int noecho()
{
	echoing = FALSE;
	return(0);
}

int sleep(int secs)

{
	Sleep(secs*1000);
	return(0);
}

int clearok(WINDOW *wp, int okay)
{
	return(0);
}

int scrollok(WINDOW *wp, int okay)
{
	wp->_scroll = okay;
	return(0);
}

int touchwin(WINDOW *wp)
{
	curscr->_mincy = 0;
	curscr->_maxcy = curscr->_maxy-1;
	return(0);
}

int wrefresh(WINDOW *wp)
{
	DWORD dwLen;
	COORD cLocation = {0,0};
	int i, j;
	int y, x;
	int linecount;

	if(wp->_parent) wrefresh(wp->_parent);
	for(i = 0; i < wp->_maxy; i++)
	{
		y = wp->_begy + i;
		if(y < curscr->_maxy)
		{
			for(j = 0; j < wp->_maxx; j++)
			{
				x = wp->_begx + j;
				if(x < curscr->_maxx)
				{
					if(curscr->_y[y][x] != wp->_y[i][j])
					{
						if(y < curscr->_mincy) curscr->_mincy = y;
						if(y > curscr->_maxcy) curscr->_maxcy = y;
						curscr->_y[y][x] = wp->_y[i][j];
					}
				}
			}
 		}
	}
	/* update the screen */
	if((linecount  = (curscr->_maxcy - curscr->_mincy)) >= 0)
	{
		linecount++;
		cLocation.X = 0;
		cLocation.Y = curscr->_mincy;
		for (i=0; i<linecount; i++, cLocation.Y++)
		{
			WriteConsoleOutputCharacter(hStdout,
				(LPTSTR)(curscr->_y[curscr->_mincy+i]),
				(DWORD)curscr->_maxx,
				cLocation, &dwLen);
		}
		curscr->_mincy = curscr->_maxy;
		curscr->_maxcy = -1;
	}	
	return(0);
}

int werase(WINDOW *wp)
{
	int i, j;

	for(i = 0; i < LINES; i++)
		for(j = 0; j < COLS; j++) wp->_y[i][j] = ' ';
	return(0);
}

int wclrtoeol(WINDOW *wp)
{
	int i;
	for(i = wp->_curx; i < wp->_maxx; i++)
		wp->_y[wp->_cury][i] = ' ';
	return(0);
}

int wmove(WINDOW *wp, int y, int x)
{
	if(x < 0) x = 0;
	if(x >= wp->_maxx) x = wp->_maxx -1;
	if(y < 0) y = 0;
	if(y >= wp->_maxy) y = wp->_maxy -1;
	wp->_curx = x;
	wp->_cury = y;
	return(0);
}

int vwprintw(WINDOW *wp, char *fmtp, va_list ap)
{
	char tmpbuf[2048]; /* help what should this be ? */
	_vsnprintf(tmpbuf, sizeof(tmpbuf), fmtp, ap);
	waddstr(wp, tmpbuf);
	return(0);
}

int wprintw(WINDOW *wp, ...)
{
	va_list ap;
	char tmpbuf[256];
	char * fmtp;
	
	va_start(ap, wp);
	fmtp = va_arg(ap, char *);
	vsprintf(tmpbuf, fmtp, ap);
	waddstr(wp, tmpbuf);
	va_end(ap);
	return(0);
}

static char
screenchar(chtype ch)
{
	/* This implementation only stores chars */
	return (char)ch;
}

int waddch(WINDOW *wp, chtype ch)
{
	int i, j;

	if(ch == '\r') 
		wp->_curx = 0;
	else if(ch == '\n')
	{
		wp->_curx = 0;
		wp->_cury++;
		if(wp->_cury >= wp->_maxy)
		{
			if(wp->_scroll)
			{
				for(i = 0; i < (wp->_maxy - 1); i++)
				{
					for(j = 0; j < wp->_maxx; j++)
					{
						ch = wp->_y[i+1][j];
						wp->_y[i][j] = screenchar(ch);
					}
				}
				i = wp->_maxy-1;
				for(j = 0; j < wp->_maxx; j++)
				wp->_y[i][j] = ' ';
			}
			wp->_cury = wp->_maxy-1;
		}
	}
	else
	{
		wp->_y[wp->_cury][wp->_curx] = screenchar(ch);
		wp->_curx++;
		if(wp->_curx >= wp->_maxx)
		{
			wp->_curx = 0;
			wp->_cury++;
			if(wp->_cury >= wp->_maxy)
			{
				if(wp->_scroll)
				{
					for(i = 0; i < (wp->_maxy - 1); i++)
					{
						for(j = 0; j < wp->_maxx; j++)
						{
							ch = wp->_y[i+1][j];
							wp->_y[i][j] = screenchar(ch);
						}
					}
					i = wp->_maxy-1;
					for(j = 0; j < wp->_maxx; j++)
						wp->_y[i][j] = ' ';
				}
				else wp->_curx = wp->_maxx-1;
				wp->_cury = wp->_maxy-1;
			}
		}
	}
	return(0);
}

int waddstr(WINDOW *wp, const char *cp)
{
	while(*cp)
	{
		waddch(wp, *cp);
		cp++;
	}
	return(0);
}

int mvwaddstr(WINDOW *wp, int y, int x, char *cp)
{
	wmove(wp, y, x);
	waddstr(wp, cp);
	return(0);
}

int mvwaddch(WINDOW *wp, int y, int x, chtype ch)
{
	wmove(wp, y, x);
	waddch(wp, ch);
	return(0);
}

int mvwprintw(WINDOW *wp, int y, int x, ...)
{
	va_list ap;
	char tmpbuf[256];
	char * fmtp;

	wmove(wp, y, x);
	
	va_start(ap, x);
	fmtp = va_arg(ap, char *);
	vsprintf(tmpbuf, fmtp, ap);
	waddstr(wp, tmpbuf);
	va_end(ap);
	return(0);
}

int mvwgetstr(WINDOW *wp, int y, int x, char *cp)
{
	wmove(wp, y, x);
	wgetstr(wp, cp);
	return(0);
}

int mvwgetnstr(WINDOW *wp, int y, int x, char *cp, int n)
{
	wmove(wp, y, x);
	wgetnstr(wp, cp, n);
	return(0);
}


int box(WINDOW *wp, chtype v, chtype h)
{
	return wborder(wp, v, v, h, h, 0, 0, 0, 0);
}

int wborder(WINDOW *wp, chtype ls, chtype rs, chtype ts, chtype bs
	, chtype tl, chtype tr, chtype bl, chtype br)
{
	int i;

	wp->_boxed = TRUE;
	if (!ls) ls = ACS_VLINE;
	if (!rs) rs = ACS_VLINE;
	if (!ts) ts = ACS_HLINE;
	if (!bs) bs = ACS_HLINE;
	if (!tl) tl = ACS_ULCORNER;
	if (!tr) tr = ACS_URCORNER;
	if (!bl) bl = ACS_LLCORNER;
	if (!br) br = ACS_LRCORNER;
	for(i = 0; i < wp->_maxy; i++)
	{
		wp->_y[i][0] = screenchar(ls);
		wp->_y[i][wp->_maxx-1] = screenchar(rs);
	}
	for(i = 0; i < wp->_maxx; i++)
	{
		wp->_y[0][i] = screenchar(ts);
		wp->_y[wp->_maxy-1][i] = screenchar(bs);
	}
	wp->_y[0][0] = screenchar(tl);
	wp->_y[0][wp->_maxx-1] = screenchar(tr);
	wp->_y[wp->_maxy-1][0] = screenchar(bl);
	wp->_y[wp->_maxy-1][wp->_maxx-1] = screenchar(br);
	return(0);
}

int wgetch(WINDOW *wp)
{
	COORD cCursor;
	char ch;

	cCursor.X = wp->_curx + wp->_begx;
	cCursor.Y = wp->_cury + wp->_begy;
	SetConsoleCursorPosition(hStdout, cCursor);
	ch = mycur_getc();
	if(echoing)
	{
		if(ch == '\b');
		else
		{
			waddch(wp, ch);
			wrefresh(wp);
			cCursor.X = wp->_curx + wp->_begx;
			cCursor.Y = wp->_cury + wp->_begy;
			SetConsoleCursorPosition(hStdout, cCursor);
		}
	}
	return(ch);
}

int wgetstr(WINDOW *wp, char *cp)
{
	return wgetnstr(wp, cp, -1);
}

int wgetnstr(WINDOW *wp, char *cp, int n)
{
	COORD cCursor;
	char *bp;
	int ch;
	int overflowed=0, echoprev;

	bp = cp;
	while(1)
	{
		overflowed = (n>=0 && bp-cp>=n-1);
		echoprev = echoing;
		if(overflowed)
			echoing = 0;
		ch = wgetch(wp);
		echoing = echoprev;
		if(ch==EOF || ch=='\n' || ch=='\r') 
		{
			break;
		}
		else if(ch == '\b')
		{
			if(bp != cp)
			{
				bp--;
				if(echoing)
				{
					if(wp->_curx > 0) wp->_curx--;
					wp->_y[wp->_cury][wp->_curx] = ' ';
					wrefresh(wp);
					cCursor.X = wp->_curx + wp->_begx;
					cCursor.Y = wp->_cury + wp->_begy;
					SetConsoleCursorPosition(hStdout, cCursor);
				}
			}
		}
		else
		{
			if(overflowed)
				MessageBeep(MB_OK);
			else
				*bp++ = ch;
		}
	}
	*bp = '\0';
	return(0);
}

/* Win32 Stuff */

static void mycur_init(int fullscreen)
{
	COORD cStartPos;
	DWORD dwLen;

	if(first)
	{
		/* set up alternate character set values */

		ACS_VLINE = (CHTYPE)179;
		ACS_HLINE = (CHTYPE)196;
		ACS_BTEE = (CHTYPE)193;
		ACS_TTEE = (CHTYPE)194;
		ACS_RTEE = (CHTYPE)180;
		ACS_LTEE = (CHTYPE)195;
		ACS_ULCORNER = (CHTYPE)218;
		ACS_LRCORNER = (CHTYPE)217;
		ACS_URCORNER = (CHTYPE)191;
		ACS_LLCORNER = (CHTYPE)192;

		/* set up console I/O */
		first = FALSE;
		hStdin = GetStdHandle((DWORD)STD_INPUT_HANDLE);
		if(hStdin != INVALID_HANDLE_VALUE)
		{
			GetConsoleMode(hStdin, &dwModeIn);
			dwModeIn &= ~(ENABLE_LINE_INPUT
				| ENABLE_ECHO_INPUT
				| ENABLE_MOUSE_INPUT
				| ENABLE_WINDOW_INPUT);
			dwModeIn |=	ENABLE_PROCESSED_INPUT;
			SetConsoleMode(hStdin, dwModeIn);
		}
		/* else fprintf(errfp, "GetStdHandle() failed.\n"); */

		hStdout = GetStdHandle((DWORD)STD_OUTPUT_HANDLE);
		if(hStdout != INVALID_HANDLE_VALUE)
		{
/*			{ 
				COORD coord = { 100, 30 };
				if (!SetConsoleScreenBufferSize(hStdout, coord))
				{
					dwLen=GetLastError();
				}
			}
			*/
			GetConsoleMode(hStdout, &dwModeOut);
			/* SetConsoleMode(hStdout, dwModeOut); */
			GetConsoleScreenBufferInfo(hStdout, &sScreenInfo);
#ifdef DEBUG
			fprintf(errfp, "Screen buffer: (%d,%d) pos=(%d,%d)\n",
				sScreenInfo.dwSize.X, sScreenInfo.dwSize.Y,
				sScreenInfo.dwCursorPosition.X, sScreenInfo.dwCursorPosition.Y);
			fprintf(errfp, "Window: (%d-%d,%d-%d) max=(%d,%d)\n",
				sScreenInfo.srWindow.Left, sScreenInfo.srWindow.Right,
				sScreenInfo.srWindow.Top, sScreenInfo.srWindow.Bottom,
				sScreenInfo.dwMaximumWindowSize.X,
				sScreenInfo.dwMaximumWindowSize.Y);
#endif
			if (sScreenInfo.srWindow.Left || sScreenInfo.srWindow.Top)
			{
				/* WARNING: this code will not work correctly with offset window */
			}
			cStartPos.X = 0;
			cStartPos.Y = 0;
			FillConsoleOutputCharacter(hStdout, (TCHAR)' ',
			(DWORD)(sScreenInfo.dwSize.X*sScreenInfo.dwSize.Y),
			cStartPos, &dwLen);
			if (fullscreen)
			{
				LINES = sScreenInfo.dwSize.Y;
				COLS = sScreenInfo.dwSize.X;
			}
			else
			{
				LINES = sScreenInfo.srWindow.Bottom;
				COLS = sScreenInfo.srWindow.Right;
			}
			if(LINES > CUR_MAXLINES)
				LINES = CUR_MAXLINES;
			if(COLS > CUR_MAXCOLS)
				COLS = CUR_MAXCOLS;
		}
	/* else fprintf(errfp, "GetStdHandle() failed.\n"); */
	}
}

static int mycur_getc()
{
	char buf[64];
	DWORD dwLen;
	int ch;

	if(hStdin != INVALID_HANDLE_VALUE)
	{
		if(ReadConsole(hStdin, (LPVOID)buf, (DWORD)1, &dwLen, (LPVOID)NULL))
			ch = buf[0];
		else ch = EOF;
	}
	else ch = getchar();
	return(ch);
}
