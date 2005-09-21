/* curses.h */
/* header for windows implementation of curses library */

#ifndef CURSES_H
#define CURSES_H	/* define prevents multiple includes */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif


#define NCURSES_VERSION_MAJOR 5
#define NCURSES_VERSION_MINOR 0

/* we truncate screen to no larger than this */
#define CUR_MAXLINES 80
#define CUR_MAXCOLS 200

#define CHTYPE unsigned long
#ifdef	CHTYPE
	typedef	CHTYPE chtype;
#else
	typedef unsigned long chtype;
#endif /* CHTYPE */

extern CHTYPE *acs_map;

#define ERR (-1)

struct tag_window
{
	short      _cury, _curx;   /* current coordinates */
	short      _maxy, _maxx;   /* max coordinates */
	short      _begy, _begx;   /* (0,0) screen coordinates */
	int        _ylines;        /* major axis size of _y */
	int        _ycols;         /* minor axis size of _y */
	char      *_yarr;          /* [_ylines][_ycols] array */
	int        _num;           /* window number */
	int        _scroll;        /* scroll allowed */
	int        _boxed;		   /* a box surrounds the window */
	int        _mincy;         /* mimimum changed y (for refresh) */
	int        _maxcy;         /* mimimum changed y (for refresh) */
	struct tag_window * _parent;	/* parent window if a sub window */
};

typedef struct tag_window WINDOW;
typedef	char bool;

extern	int	LINES, COLS;
extern	WINDOW	*stdscr, *curscr;

/* routine prototypes */

int box(WINDOW *wp, chtype x, chtype y);
int cbreak();
int clearok(WINDOW *wp, int okay);
int crmode();
int curs_set(int mode);
int delwin(WINDOW *wp);
int doupdate(void);
int echo();
int endwin();
int has_key(int ch);
WINDOW  *initscr();
int keypad(WINDOW *wp, int bf);
int mvwaddch(WINDOW *wp, int y, int x, chtype ch);
int mvwaddnstr(WINDOW *wp, int y, int x, char *cp, int n);
int mvwaddstr(WINDOW *wp, int y, int x, char *cp);
int mvwgetnstr(WINDOW *wp, int y, int x, char *cp, int n);
int mvwgetstr(WINDOW *wp, int y, int x, char *cp);
int mvwprintw(WINDOW *wp, int x, int y, ...);
WINDOW  *newwin(int nlines, int ncols, int begy, int begx);
int nocbreak();
int nocrmode();
int noecho();
int redrawwin(WINDOW *wp);
int scrollok(WINDOW *wp, int okay);
WINDOW  *subwin(WINDOW *wp, int nlines, int ncols, int begy, int begx);
int touchwin(WINDOW *wp);
int vwprintw(WINDOW *wp, char *fmt, va_list ap);
int waddch(WINDOW *wp, chtype ch);
int waddnstr(WINDOW *wp, const char *cp, int n);
int waddstr(WINDOW *wp, const char *cp);
int wborder(WINDOW *wp, chtype ls, chtype rs, chtype ts, chtype bs
	, chtype tl, chtype tr, chtype bl, chtype br);
int wclrtoeol(WINDOW *wp);
int werase(WINDOW *wp);
int wgetch(WINDOW *wp);
int wgetnstr(WINDOW *wp, char *cp, int n);
int wgetstr(WINDOW *wp, char *cp);
int wmove(WINDOW *wp, int x, int y);
int wnoutrefresh(WINDOW *wp);
int wprintw(WINDOW *wp, ...);
int wrefresh(WINDOW *wp);
/* win32 specific routines */
void wtitle(const char * title); 

/*
 * Standard alternate character set.  The current ACS world is evolving,
 * so we support only a widely available subset: the line drawing characters
 * from the VT100, plus a few from the Teletype 5410v1.  Eventually there
 * may be support of more sophisticated ACS line drawing, such as that
 * in the Teletype 5410, the HP line drawing set, and the like.  There may
 * be support for some non line oriented characters as well.
 *
 * Line drawing ACS names are of the form ACS_trbl, where t is the top, r
 * is the right, b is the bottom, and l is the left.  t, r, b, and l might
 * be B (blank), S (single), D (double), or T (thick).  The subset defined
 * here only uses B and S.
 */

#define ACS_BSSB	(acs_map['l'])
#define ACS_SSBB	(acs_map['m'])
#define ACS_BBSS	(acs_map['k'])
#define ACS_SBBS	(acs_map['j'])
#define ACS_SBSS	(acs_map['u'])
#define ACS_SSSB	(acs_map['t'])
#define ACS_SSBS	(acs_map['v'])
#define ACS_BSSS	(acs_map['w'])
#define ACS_BSBS	(acs_map['q'])
#define ACS_SBSB	(acs_map['x'])
#define ACS_SSSS	(acs_map['n'])

/*
 * Human readable names for the most commonly used characters.
 * "Upper", "right", etc. are chosen to be consistent with the vt100 manual.
 */

#define ACS_ULCORNER	ACS_BSSB
#define ACS_LLCORNER	ACS_SSBB
#define ACS_URCORNER	ACS_BBSS
#define ACS_LRCORNER	ACS_SBBS
#define ACS_RTEE	ACS_SBSS
#define ACS_LTEE	ACS_SSSB
#define ACS_BTEE	ACS_SSBS
#define ACS_TTEE	ACS_BSSS
#define ACS_HLINE	ACS_BSBS
#define ACS_VLINE	ACS_SBSB
#define ACS_PLUS	ACS_SSSS
#define ACS_S1		(acs_map['o'])	/* scan line 1 */
#define ACS_S9		(acs_map['s'])	/* scan line 9 */
#define ACS_DIAMOND	(acs_map['`'])	/* diamond */
#define ACS_CKBOARD	(acs_map['a'])	/* checker board (stipple) */
#define ACS_DEGREE	(acs_map['f'])	/* degree symbol */
#define ACS_PLMINUS	(acs_map['g'])	/* plus/minus */
#define ACS_BULLET	(acs_map['~'])	/* bullet */
	/* Teletype 5410v1 symbols */
#define ACS_LARROW	(acs_map[','])	/* arrow pointing left */
#define ACS_RARROW	(acs_map['+'])	/* arrow pointing right */
#define ACS_DARROW	(acs_map['.'])	/* arrow pointing down */
#define ACS_UARROW	(acs_map['-'])	/* arrow pointing up */
#define ACS_BOARD	(acs_map['h'])	/* board of squares */
#define ACS_LANTERN	(acs_map['i'])	/* lantern symbol */
#define ACS_BLOCK	(acs_map['0'])	/* solid square block */

/* Funny "characters" enabled for various special function keys for input */
/* This list is created from caps and curses.ed. Do not edit it! */
#define KEY_MIN		0401		/* Minimum curses key */
#define KEY_BREAK	0401		/* break key (unreliable) */
#define KEY_DOWN	0402		/* Sent by terminal down arrow key */
#define KEY_UP		0403		/* Sent by terminal up arrow key */
#define KEY_LEFT	0404		/* Sent by terminal left arrow key */
#define KEY_RIGHT	0405		/* Sent by terminal right arrow key */
#define KEY_HOME	0406		/* Sent by home key. */
#define KEY_BACKSPACE	0407		/* Sent by backspace key */
#define KEY_F0		0410		/* function key f0. */
#define KEY_F(n)	(KEY_F0+(n))	/* Space for 64 function keys is reserved. */
#define KEY_DL		0510		/* Sent by delete line key. */
#define KEY_IL		0511		/* Sent by insert line. */
#define KEY_DC		0512		/* Sent by delete character key. */
#define KEY_IC		0513		/* Sent by ins char/enter ins mode key. */
#define KEY_EIC		0514		/* Sent by rmir or smir in insert mode. */
#define KEY_CLEAR	0515		/* Sent by clear screen or erase key. */
#define KEY_EOS		0516		/* Sent by clear-to-end-of-screen key. */
#define KEY_EOL		0517		/* Sent by clear-to-end-of-line key. */
#define KEY_SF		0520		/* Sent by scroll-forward/down key */
#define KEY_SR		0521		/* Sent by scroll-backward/up key */
#define KEY_NPAGE	0522		/* Sent by next-page key */
#define KEY_PPAGE	0523		/* Sent by previous-page key */
#define KEY_STAB	0524		/* Sent by set-tab key */
#define KEY_CTAB	0525		/* Sent by clear-tab key */
#define KEY_CATAB	0526		/* Sent by clear-all-tabs key. */
#define KEY_ENTER	0527		/* Enter/send (unreliable) */
#define KEY_SRESET	0530		/* soft (partial) reset (unreliable) */
#define KEY_RESET	0531		/* reset or hard reset (unreliable) */
#define KEY_PRINT	0532		/* print or copy */
#define KEY_LL		0533		/* Sent by home-down key */
					/* The keypad is arranged like this: */
					/*    a1    up    a3   */
					/*   left   b2  right  */
					/*    c1   down   c3   */
#define KEY_A1		0534		/* Upper left of keypad */
#define KEY_A3		0535		/* Upper right of keypad */
#define KEY_B2		0536		/* Center of keypad */
#define KEY_C1		0537		/* Lower left of keypad */
#define KEY_C3		0540		/* Lower right of keypad */
#define KEY_BTAB	0541		/* Back tab key */
#define KEY_BEG		0542		/* beg(inning) key */
#define KEY_CANCEL	0543		/* cancel key */
#define KEY_CLOSE	0544		/* close key */
#define KEY_COMMAND	0545		/* cmd (command) key */
#define KEY_COPY	0546		/* copy key */
#define KEY_CREATE	0547		/* create key */
#define KEY_END		0550		/* end key */
#define KEY_EXIT	0551		/* exit key */
#define KEY_FIND	0552		/* find key */
#define KEY_HELP	0553		/* help key */
#define KEY_MARK	0554		/* mark key */
#define KEY_MESSAGE	0555		/* message key */
#define KEY_MOVE	0556		/* move key */
#define KEY_NEXT	0557		/* next object key */
#define KEY_OPEN	0560		/* open key */
#define KEY_OPTIONS	0561		/* options key */
#define KEY_PREVIOUS	0562		/* previous object key */
#define KEY_REDO	0563		/* redo key */
#define KEY_REFERENCE	0564		/* ref(erence) key */
#define KEY_REFRESH	0565		/* refresh key */
#define KEY_REPLACE	0566		/* replace key */
#define KEY_RESTART	0567		/* restart key */
#define KEY_RESUME	0570		/* resume key */
#define KEY_SAVE	0571		/* save key */
#define KEY_SBEG	0572		/* shifted beginning key */
#define KEY_SCANCEL	0573		/* shifted cancel key */
#define KEY_SCOMMAND	0574		/* shifted command key */
#define KEY_SCOPY	0575		/* shifted copy key */
#define KEY_SCREATE	0576		/* shifted create key */
#define KEY_SDC		0577		/* shifted delete char key */
#define KEY_SDL		0600		/* shifted delete line key */
#define KEY_SELECT	0601		/* select key */
#define KEY_SEND	0602		/* shifted end key */
#define KEY_SEOL	0603		/* shifted clear line key */
#define KEY_SEXIT	0604		/* shifted exit key */
#define KEY_SFIND	0605		/* shifted find key */
#define KEY_SHELP	0606		/* shifted help key */
#define KEY_SHOME	0607		/* shifted home key */
#define KEY_SIC		0610		/* shifted input key */
#define KEY_SLEFT	0611		/* shifted left arrow key */
#define KEY_SMESSAGE	0612		/* shifted message key */
#define KEY_SMOVE	0613		/* shifted move key */
#define KEY_SNEXT	0614		/* shifted next key */
#define KEY_SOPTIONS	0615		/* shifted options key */
#define KEY_SPREVIOUS	0616		/* shifted prev key */
#define KEY_SPRINT	0617		/* shifted print key */
#define KEY_SREDO	0620		/* shifted redo key */
#define KEY_SREPLACE	0621		/* shifted replace key */
#define KEY_SRIGHT	0622		/* shifted right arrow */
#define KEY_SRSUME	0623		/* shifted resume key */
#define KEY_SSAVE	0624		/* shifted save key */
#define KEY_SSUSPEND	0625		/* shifted suspend key */
#define KEY_SUNDO	0626		/* shifted undo key */
#define KEY_SUSPEND	0627		/* suspend key */
#define KEY_UNDO	0630		/* undo key */
#define KEY_MOUSE	0631		/* Mouse event has occured */
#define KEY_MAX		0777		/* Maximum curses key */

/* Various video attributes */
#define A_STANDOUT	000000200000L
#define	_STANDOUT	A_STANDOUT    /* for compatability with old curses */
#define A_UNDERLINE	000000400000L
#define A_REVERSE	000001000000L
#define A_BLINK		000002000000L
#define A_DIM		000004000000L
#define A_BOLD		000010000000L
#define A_ALTCHARSET	000100000000L

/* The next two are subject to change so don't depend on them */
#define A_INVIS		000020000000L
#define A_PROTECT	000040000000L

#define A_NORMAL	000000000000L
#define A_ATTRIBUTES	037777600000L	/* 0xFFFF0000 */
#define A_CHARTEXT	000000177777L	/* 0x0000FFFF */
#define A_COLOR		017600000000L

#define COLOR_PAIR(n)	((n) << 25)
#define PAIR_NUMBER(n)	(((n) & A_COLOR) >> 25)

/* definition of 8 basic color	*/
#define COLOR_BLACK	0
#define COLOR_RED	1
#define COLOR_GREEN	2
#define COLOR_YELLOW	3
#define COLOR_BLUE	4
#define COLOR_MAGENTA	5
#define COLOR_CYAN	6
#define COLOR_WHITE	7

#ifdef CURSES_MOUSE
/* mouse related macros: don't change these definitions or bit-masks. */
/* they are interdependent (used by _map_button() in tgetch()	      */
#define BUTTON_RELEASED            0
#define BUTTON_PRESSED             1
#define BUTTON_CLICKED             2
#define BUTTON_DOUBLE_CLICKED      3
#define BUTTON_TRIPLE_CLICKED      4

#define MOUSE_X_POS                (Mouse_status.x)
#define MOUSE_Y_POS                (Mouse_status.y)
#define A_BUTTON_CHANGED           (Mouse_status.changes & 7)
#define MOUSE_MOVED                (Mouse_status.changes & 8)
#define MOUSE_POS_REPORT	   (Mouse_status.changes & 16)
#define BUTTON_CHANGED(x)          (Mouse_status.changes & (1 << ((x) - 1)))
#define BUTTON_STATUS(x)           (Mouse_status.button[(x)-1])

/* definition of mouse bit-masks	*/
#define	BUTTON1_RELEASED	000000000001L
#define	BUTTON1_PRESSED		000000000002L
#define	BUTTON1_CLICKED		000000000004L
#define	BUTTON1_DOUBLE_CLICKED	000000000010L
#define	BUTTON1_TRIPLE_CLICKED	000000000020L
#define	BUTTON2_RELEASED	000000000040L
#define	BUTTON2_PRESSED		000000000100L
#define	BUTTON2_CLICKED		000000000200L
#define	BUTTON2_DOUBLE_CLICKED	000000000400L
#define	BUTTON2_TRIPLE_CLICKED	000000001000L
#define	BUTTON3_RELEASED	000000002000L
#define	BUTTON3_PRESSED		000000004000L
#define	BUTTON3_CLICKED		000000010000L
#define	BUTTON3_DOUBLE_CLICKED	000000020000L
#define	BUTTON3_TRIPLE_CLICKED	000000040000L
#define ALL_MOUSE_EVENTS	000000077777L
#define REPORT_MOUSE_POSITION	000000100000L
#endif	/* CURSES_MOUSE */

#ifdef __cplusplus
}
#endif

#endif /* CURSES_H */
