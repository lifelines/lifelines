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

typedef enum { W8BITS, W16BITS } WBITS;

/* fullscreen==1 to use screen buffer size instead of window size */
static int mycur_init(int fullscreen);
static int mycur_getc(WBITS wbits);
static void adjust_linescols(void);
static void console_resize_callback(void);


/* Windows Console */

static int first = TRUE;
static HANDLE hStdin = INVALID_HANDLE_VALUE;
static HANDLE hStdout = INVALID_HANDLE_VALUE;
static DWORD dwModeIn = 0;
static DWORD dwModeOut = 0;
static CONSOLE_CURSOR_INFO ConCursorInfo = {0};
static COORD cOrigin = {0,0};
static int redirected_in = 0;
static int redirected_out = 0;
static int redir_unflushed = 0; /* used to separate output when redirected */
static int CurrentScreenWidth = -1;
static int CurrentScreenHeight = -1;

/* Others Windows environment data */
extern int _fmode;		/* O_TEXT or O_BINARY */

/* Windows NT stuff */
static BOOL WindowsNt = 0; /* set to 1 if NT/2000/XP */
static HINSTANCE LibKernel32 = 0;
typedef BOOL (CALLBACK *ApiFncReadConsoleInput)(HANDLE, PINPUT_RECORD, DWORD, LPDWORD);
static ApiFncReadConsoleInput ApiReadConsoleInput = 0;

/* Debugging */

/* #define DEBUG */

#ifdef DEBUG
static FILE *errfp = NULL;
#endif

/* Curses data */

static int echoing = TRUE;
static int keypading = FALSE;

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


WINDOW	*initscr()
{
	_fmode = O_BINARY;	/* use binary mode for reading and writing */
	/* open an error file for debugging */

#ifdef DEBUG
	if(errfp == NULL)
	 	errfp = fopen("mycurses.err", "w");
#endif

	if (!mycur_init(1))
		return NULL;
	if (curscr)
	{
		delwin(curscr);
		curscr = 0;
	}
	curscr = newwin(LINES, COLS, 0, 0);
	/* set update point so whole screen is displayed */
	curscr->_mincy = 0;
	curscr->_maxcy = curscr->_maxy-1;

	if (stdscr)
	{
		delwin(stdscr);
		stdscr = 0;
	}
	stdscr = newwin(LINES, COLS, 0, 0);

	return(stdscr);
}

WINDOW  *newwin(int nlines, int ncols, int begy, int begx)
{
	WINDOW *wp;
	static int wincnt = 0;

	/* limit to screen size */
	if(nlines > LINES) nlines = LINES;
	if(ncols > COLS) ncols = COLS;

	wp = calloc(1, sizeof(WINDOW));
	wp->_ylines = nlines;
	wp->_ycols = ncols;
	wp->_yarr = calloc(sizeof(char), wp->_ylines * wp->_ycols);
	wp->_num = wincnt++;
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
	free(wp->_yarr);
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

int curs_set(int mode)
{
	/* mode 0 = hide, 1 = show */
	ConCursorInfo.bVisible = mode;
	SetConsoleCursorInfo(hStdout, &ConCursorInfo);
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

int keypad(WINDOW *win, int bf)
{
	keypading = !!bf;
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
	wnoutrefresh(wp);
	doupdate();
	return(0);
}

int redrawwin(WINDOW *wp)
{
	return wrefresh(wp);
}

static char atw(WINDOW * wp, int line, int col)
{
	if (! (line < wp->_ylines && line >= 0) ) 
	{
		return 0;
	}
	if (! (col < wp->_ycols && col >= 0) )
	{
		return 0;
	}
	return wp->_yarr[line * wp->_ycols + col];
}
static void setw(WINDOW * wp, int line, int col, char ch)
{
	if (! (line < wp->_ylines && line >= 0) ) 
	{
		return;
	}
	if (! (col < wp->_ycols && col >= 0) )
	{
		return;
	}
	wp->_yarr[line * wp->_ycols + col] = ch;
}
static char * getlinew(WINDOW *wp, int line)
{
	if (! (line < wp->_ylines && line >= 0) ) return "";
	return &wp->_yarr[line * wp->_ycols];
}

/* refresh virtual image of screen (curscr) */
int wnoutrefresh(WINDOW *wp)
{
	int i, j;
	int y, x;

	if(wp->_parent)
		wrefresh(wp->_parent);
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
					if(atw(curscr, y, x) != atw(wp, i, j))
					{
						if(y < curscr->_mincy) curscr->_mincy = y;
						if(y > curscr->_maxcy) curscr->_maxcy = y;
						setw(curscr, y, x, atw(wp, i, j));
					}
				}
			}
 		}
	}
	return(0);
}

/* repaint physical screen from virtual image (curscr) */
int doupdate(void)
{
	int linecount=0;
	int i=0;
	/* update the screen */
	if((linecount  = (curscr->_maxcy - curscr->_mincy)) >= 0)
	{
		COORD cLocation = {0,0};
		cLocation.X = 0;
		cLocation.Y = curscr->_mincy;
		linecount++;
		for (i=0; i<linecount; i++, cLocation.Y++)
		{
			DWORD dwLen;
			WriteConsoleOutputCharacter(hStdout,
				(LPTSTR)(getlinew(curscr, curscr->_mincy+i)),
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
		for(j = 0; j < COLS; j++)
			setw(wp, i, j, ' ');
	return(0);
}

int wclrtoeol(WINDOW *wp)
{
	int i;
	for(i = wp->_curx; i < wp->_maxx; i++)
		setw(wp, wp->_cury, i, ' ');
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
	redir_unflushed = 1;
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

	if (redirected_out)
	{
		if (redir_unflushed)
		{
			putc('\n', stdout);
			redir_unflushed=0;
		}
		putc(ch, stdout);
		return 0;
	}

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
						ch = atw(wp, i+1, j);
						setw(wp, i, j, screenchar(ch));
					}
				}
				i = wp->_maxy-1;
				for(j = 0; j < wp->_maxx; j++)
				setw(wp, i, j, ' ');
			}
			wp->_cury = wp->_maxy-1;
		}
	}
	else
	{
		setw(wp, wp->_cury, wp->_curx, screenchar(ch));
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
							ch = atw(wp, i+1, j);
							setw(wp, i, j, screenchar(ch));
						}
					}
					i = wp->_maxy-1;
					for(j = 0; j < wp->_maxx; j++)
						setw(wp, i, j, ' ');
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

int waddnstr(WINDOW *wp, const char *cp, int n)
{
	while(n && *cp)
	{
		waddch(wp, *cp);
		cp++;
		--n;
	}
	return(0);
}

int mvwaddstr(WINDOW *wp, int y, int x, char *cp)
{
	wmove(wp, y, x);
	waddstr(wp, cp);
	return(0);
}

int mvwaddnstr(WINDOW *wp, int y, int x, char *cp, int n)
{
	wmove(wp, y, x);
	waddnstr(wp, cp, n);
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
		setw(wp, i, 0, screenchar(ls));
		setw(wp, i, wp->_maxx-1, screenchar(rs));
	}
	for(i = 0; i < wp->_maxx; i++)
	{
		setw(wp, 0, i, screenchar(ts));
		setw(wp, wp->_maxy-1, i, screenchar(bs));
	}
	setw(wp, 0, 0, screenchar(tl));
	setw(wp, 0, wp->_maxx-1, screenchar(tr));
	setw(wp, wp->_maxy-1, 0, screenchar(bl));
	setw(wp, wp->_maxy-1, wp->_maxx-1, screenchar(br));
	return(0);
}

int wgetch(WINDOW *wp)
{
	COORD cCursor;
	int ch;

	cCursor.X = wp->_curx + wp->_begx;
	cCursor.Y = wp->_cury + wp->_begy;
	SetConsoleCursorPosition(hStdout, cCursor);
	ch = mycur_getc(W8BITS);
	if (echoing && ch>31 && ch<256 && ch!=EOF)
	{
		waddch(wp, ch);
		wrefresh(wp);
		cCursor.X = wp->_curx + wp->_begx;
		cCursor.Y = wp->_cury + wp->_begy;
		SetConsoleCursorPosition(hStdout, cCursor);
	}
	return(ch);
}

int has_key(int ch)
{
	return (ch>=KEY_MIN && ch<=KEY_MAX);
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
		if (ch == '\b')
		{
			if(bp != cp)
			{
				bp--;
				if(echoing)
				{
					if(wp->_curx > 0) wp->_curx--;
					setw(wp, wp->_cury, wp->_curx, ' ');
					wrefresh(wp);
					cCursor.X = wp->_curx + wp->_begx;
					cCursor.Y = wp->_cury + wp->_begy;
					SetConsoleCursorPosition(hStdout, cCursor);
				}
			}
		}
		if (ch==EOF || ch=='\n' || ch=='\r') 
		{
			break;
		}
		else if (ch>31 && ch<256)
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
/* returns 0 if fails */
static int mycur_init(int fullscreen)
{
	COORD cStartPos;
	DWORD dwLen;

	if(first)
	{
		CONSOLE_SCREEN_BUFFER_INFO ConScreenBuffer = {0};
		OSVERSIONINFO verinfo;
		verinfo.dwOSVersionInfoSize = sizeof(verinfo);
		if (GetVersionEx(&verinfo) && verinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			WindowsNt = 1;
			LibKernel32 = LoadLibrary("Kernel32.dll");
			ApiReadConsoleInput = (ApiFncReadConsoleInput)GetProcAddress(LibKernel32, "ReadConsoleInputW");
		}

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
		if(hStdin == INVALID_HANDLE_VALUE)
		{
			printf("Error opening console window for input\n");
			return 0;
		}
		if (!GetConsoleMode(hStdin, &dwModeIn))
			redirected_in = 1;
		dwModeIn &= ~(ENABLE_LINE_INPUT
			| ENABLE_ECHO_INPUT
			| ENABLE_MOUSE_INPUT
			| ENABLE_WINDOW_INPUT);
		dwModeIn |=	ENABLE_PROCESSED_INPUT;
		SetConsoleMode(hStdin, dwModeIn);

		hStdout = GetStdHandle((DWORD)STD_OUTPUT_HANDLE);
		if (hStdout == INVALID_HANDLE_VALUE)
		{
			printf("Error opening console window for output\n");
			return 0;
		}

		if (!GetConsoleScreenBufferInfo(hStdout, &ConScreenBuffer)) {
			redirected_out = 1;
			ConScreenBuffer.dwSize.X = 80;
			ConScreenBuffer.dwSize.Y = 25;
			ConScreenBuffer.srWindow.Bottom = 25;
			ConScreenBuffer.srWindow.Right = 80;
			/*
			TODO: What to do with console output functions when output redirected ?
			2002-10-19, Perry
			*/
		}
		CurrentScreenWidth = ConScreenBuffer.dwSize.X;
		CurrentScreenHeight = ConScreenBuffer.dwSize.Y;

		/* get info on cursor */
		GetConsoleCursorInfo(hStdout, &ConCursorInfo);
		/*
		TODO: What to do with console cursor functions when output redirected?
		2005-09-21, Matt
		*/

#ifdef DEBUG
		fprintf(errfp, "Screen buffer: (%d,%d) pos=(%d,%d)\n",
			ConScreenBuffer.dwSize.X, ConScreenBuffer.dwSize.Y,
			ConScreenBuffer.dwCursorPosition.X, ConScreenBuffer.dwCursorPosition.Y);
		fprintf(errfp, "Window: (%d-%d,%d-%d) max=(%d,%d)\n",
			ConScreenBuffer.srWindow.Left, ConScreenBuffer.srWindow.Right,
			ConScreenBuffer.srWindow.Top, ConScreenBuffer.srWindow.Bottom,
			ConScreenBuffer.dwMaximumWindowSize.X,
			ConScreenBuffer.dwMaximumWindowSize.Y);
#endif
		if (ConScreenBuffer.srWindow.Left || ConScreenBuffer.srWindow.Top)
		{
			/* WARNING: this code will not work correctly with offset window */
		}
		cStartPos.X = 0;
		cStartPos.Y = 0;

		/* clear console (fill with blanks) */
		FillConsoleOutputCharacter(hStdout, (TCHAR)' ',
			(DWORD)(ConScreenBuffer.dwSize.X*ConScreenBuffer.dwSize.Y),
			cStartPos, &dwLen);
		/* save size into curses globals LINES & COLS */
		if (fullscreen)
		{
			LINES = ConScreenBuffer.dwSize.Y;
			COLS = ConScreenBuffer.dwSize.X;
		}
		else
		{
			LINES = ConScreenBuffer.srWindow.Bottom;
			COLS = ConScreenBuffer.srWindow.Right;
		}
		adjust_linescols();
	}
	return 1;
}

/*
Adjust LINES & COLS so they're not ridiculously large
because screen buffer size can be very large (eg, 9000 lines)
*/
static void adjust_linescols()
{
	if (LINES > CUR_MAXLINES)
		LINES = CUR_MAXLINES;
	if (COLS > CUR_MAXCOLS)
		COLS = CUR_MAXCOLS;
}

/* helper structure for key table in mycur_getc */
struct keycvt { int win; int curs; };

/* Check if console size has changed, and update LINES & COLS if so */
static void check_for_resize (void)
{
	CONSOLE_SCREEN_BUFFER_INFO ConScreenBuffer = {0};
	if (!GetConsoleScreenBufferInfo(hStdout, &ConScreenBuffer))
		return;

	if (CurrentScreenWidth != ConScreenBuffer.dwSize.X
		|| CurrentScreenHeight != ConScreenBuffer.dwSize.Y) {
		/* screen has been resized */
		LINES = CurrentScreenHeight= ConScreenBuffer.dwSize.Y;
		COLS = CurrentScreenWidth = ConScreenBuffer.dwSize.X;
		adjust_linescols();
		console_resize_callback();
	}
}
/*
 In May of 2002 Perry changed from ReadConsole to ReadConsoleInput
 in order to be able to get arrow keys etc
*/
/* Caller can request either 8-bit input (ReadConsoleInputA) or 16-bit input (ReadConsoleInputW) */

static int mycur_getc(WBITS wbits)
{
	/* table of curses names for special keys */
	/* NB: These are not single-byte constants, so return value must be int */
	static struct keycvt keymap[] = {
		{ VK_DOWN, KEY_DOWN }
		, { VK_UP, KEY_UP }
		, { VK_NEXT, KEY_NPAGE }
		, { VK_PRIOR, KEY_PPAGE }
		, { VK_HOME, KEY_HOME }
		, { VK_END, KEY_END }
		, { VK_RETURN, KEY_ENTER }
	};
	static struct keycvt skeymap[] = {
		{ VK_NEXT, KEY_SNEXT }
		, { VK_PRIOR, KEY_SPREVIOUS }
		, { VK_HOME, KEY_SHOME }
	};
	static int altctrls = LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED | LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED;
	static int numpadbase=0;
	static int numpadval=0;

	if (hStdin == INVALID_HANDLE_VALUE || redirected_in)
		return getchar();

	while (1)
	{
		INPUT_RECORD inrec;
		int nrecs=0, i;

		if (WindowsNt && wbits==W16BITS)
		{
			(*ApiReadConsoleInput)(hStdin, &inrec, 1, &nrecs);
		}
		else
		{
			ReadConsoleInput(hStdin, &inrec, 1, &nrecs);
		}

		/* To get special keys, have to use ReadConsoleInput */
		if (!nrecs)
			return EOF;

		/* 2006-07-23, Perry:
		Not receiving any inrec.EventType == WINDOW_BUFFER_SIZE_EVENT)
			int width= inrec.Event.WindowBufferSizeEvent.dwSize.X;
			int height = inrec.Event.WindowBufferSizeEvent.dwSize.Y;
		*/
		check_for_resize();
		if (inrec.EventType == KEY_EVENT && inrec.Event.KeyEvent.bKeyDown
			&& inrec.Event.KeyEvent.wVirtualKeyCode != VK_MENU)
		{
			int keystate, keycode;
			/* return normal keys */
			if (WindowsNt && wbits==W16BITS)
			{
				if (inrec.Event.KeyEvent.uChar.UnicodeChar)
				{
					unsigned int ch = (wchar_t)inrec.Event.KeyEvent.uChar.UnicodeChar;
					return ch;
				}
			}
			else
			{
				if (inrec.Event.KeyEvent.uChar.AsciiChar)
				{
					unsigned int ch = (unsigned char)inrec.Event.KeyEvent.uChar.AsciiChar;
					return ch;
				}
			}
			keystate = inrec.Event.KeyEvent.dwControlKeyState;
			keycode = inrec.Event.KeyEvent.wVirtualKeyCode;
			/* handle keyboard numeric escapes for non-ASCII characters */
			if ((keystate & LEFT_ALT_PRESSED || keystate & RIGHT_ALT_PRESSED)
				&& (VK_NUMPAD0 <= keycode && keycode <= VK_NUMPAD9))
			{
				if (!numpadval)
				{
					if (keycode == VK_NUMPAD0)
					{
						/* TODO: 0-prefixed alt-codes are not octal codes in the console
						codeset at all, but rather decimal codes but in the Windows codeset !
						2003-04-20, Perry
						*/
						numpadbase = 8;
						numpadval = -1;
					}
					else
					{
						numpadbase = 10;
						numpadval = (keycode - VK_NUMPAD0);
					}
				}
				else
				{
					if (numpadval == -1)
						numpadval = keycode - VK_NUMPAD0;
					else
						numpadval = numpadval * numpadbase + (keycode - VK_NUMPAD0);
					if (numpadval > 127)
					{
						int val = numpadval;
						numpadval = 0;
						return val;
					}
				}
			}
			else
				numpadval = 0;
			/* check if it is a hardware key we handle */
			/* if so, map it to curses constant */
			if (!(keystate & altctrls))
			{
				if (keystate & SHIFT_PRESSED)
				{
					for (i=0; i<sizeof(skeymap)/sizeof(skeymap[0]); ++i)
					{
						if (inrec.Event.KeyEvent.wVirtualKeyCode == skeymap[i].win)
							return skeymap[i].curs;
					}
				}
				else /* not shifted */
				{
					for (i=0; i<sizeof(keymap)/sizeof(keymap[0]); ++i)
					{
						if (inrec.Event.KeyEvent.wVirtualKeyCode == keymap[i].win)
							return keymap[i].curs;
					}
				}
			}
			else
			{
			}
		}
	}
}

void wtitle(const char *title)
{
	SetConsoleTitle(title);
}

static void * cbparam = 0;
static void (*cbfunc)(void *) = 0;
void w_set_console_resize_callback(void (*fptr)(void * param), void * param)
{
	cbparam = param;
	cbfunc = fptr;
}
static void console_resize_callback(void)
{
	if (cbfunc)
		(cbfunc)(cbparam);
}
