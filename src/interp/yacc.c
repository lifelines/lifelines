#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define YYPREFIX "yy"
#line 31 "yacc.y"
#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "interp.h"
#undef FORLIST

extern TABLE proctab, functab;
static INTERP this, prev;
extern LIST Plist;
#line 22 "y.tab.c"
#define PROC 257
#define FUNC 258
#define IDEN 259
#define LITERAL 260
#define CHILDREN 261
#define SPOUSES 262
#define IF 263
#define ELSE 264
#define ELSIF 265
#define FAMILIES 266
#define ICONS 267
#define WHILE 268
#define CALL 269
#define FORINDISET 270
#define FORINDI 271
#define FORNOTES 272
#define TRAVERSE 273
#define FORNODES 274
#define FORLIST 275
#define FORFAM 276
#define BREAK 277
#define CONTINUE 278
#define RETURN 279
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,    1,    1,    1,    1,    2,    3,    4,    4,
    6,    6,    5,    5,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
    7,   11,   11,   14,   14,   15,   12,   12,    9,    9,
    9,    9,   13,   13,   16,   16,   10,   10,    8,
};
short yylen[] = {                                         2,
    1,    2,    1,    1,    4,    4,    8,    8,    0,    1,
    1,    3,    1,    2,   12,   14,   14,   14,   12,   10,
   10,   10,   12,   10,   11,    9,    6,    4,    4,    5,
    1,    0,    1,    1,    2,    8,    0,    4,    1,    5,
    1,    1,    0,    1,    1,    3,    0,    2,    0,
};
short yydefred[] = {                                      0,
    0,    0,    0,    0,    1,    3,    4,    0,    0,    0,
    2,    0,    0,    0,    0,    0,    0,   10,    0,    5,
    6,    0,    0,    0,   12,    0,    0,    0,   41,   49,
   49,   49,   49,   42,   49,    0,   49,   49,   49,   49,
   49,   49,   49,   49,   49,   49,    0,   13,   31,    0,
    0,    0,    0,    0,    0,    0,   49,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    7,   14,    8,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   44,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   28,   29,    0,    0,   40,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   30,   46,    0,    0,   48,    0,    0,    0,
   27,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   26,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   33,    0,
    0,    0,   20,   22,    0,   24,    0,   21,    0,    0,
    0,    0,   25,   35,    0,    0,    0,    0,   15,    0,
    0,    0,    0,    0,   23,   19,    0,    0,    0,    0,
    0,   16,    0,   38,   17,   18,    0,    0,   36,
};
short yydgoto[] = {                                       4,
    5,    6,    7,   17,   47,   18,   48,   51,   49,  112,
  178,  193,   89,  179,  180,   90,
};
short yysindex[] = {                                   -235,
 -254, -244,   -8, -235,    0,    0,    0,    8,   10, -233,
    0, -208, -208,   12,   20,   18,   36,    0,   37,    0,
    0, -208,  -43,  -42,    0, -203, -203,    0,    0,    0,
    0,    0,    0,    0,    0, -180,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    2,    0,    0,   23,
   42,   43,   46,   47,   48,   49,    0,   50,   51,   52,
   53,   54,   55,   56,   57,   58,   59,    0,    0,    0,
 -213, -213, -213, -213, -213, -213,   60, -213, -175, -213,
 -213, -213, -213, -158,   61,   62, -213,   63,   64,    0,
   66,   68,   70,   71,   70, -213,   73,   75,   76,   77,
   78,   79,   80,    0,    0,   67, -213,    0, -155, -128,
 -213,   93, -123,   97,   98, -119, -118, -117, -116, -115,
 -113, -112,    0,    0,  105,  107,    0,   29,  111,   33,
    0,  113,  117,  118,  120,  127,  126,  130,  -98,  -87,
 -203,  -86, -203,  -83,   85,   86,  -78,   87,  -77,   88,
  139,  140,   44,  141,   82,  168, -203, -203,  172, -203,
  173, -203,   92,  -76,  -49,  -41,    0,  -35,  121,  142,
   94,  163,  102,  184, -203,  225,  247,   65,    0,  -49,
  267,  283,    0,    0, -203,    0, -203,    0,  205,  202,
 -213,  203,    0,    0,  204,  208,  237,  260,    0, -203,
   70, -203, -203, -203,    0,    0,  281,  287,  302,  323,
  344,    0,  209,    0,    0,    0, -203,  365,    0,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  292,  292,    0,    0,  293,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  -40,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  294,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  294,  295,    0,    0,
    0,    0,  296,    0,  296,  294,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  -73,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  392,    0,  -19,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  296,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
  334,    0,    0,  326,  -25,  318,  -44,   -2,  -62,  -88,
    0,    0,  -79,  166,    0,  240,
};
#define YYTABLESIZE 671
short yytable[] = {                                      49,
   39,   50,   69,   39,    8,   69,  114,  106,   88,   91,
   92,   93,   94,   95,    9,   97,  115,   99,  100,  101,
  102,    1,    2,    3,   88,   14,   15,   52,   53,   54,
   55,   10,   56,   88,   58,   59,   60,   61,   62,   63,
   64,   65,   66,   67,   88,   28,   29,   12,  127,   13,
   16,   32,   20,   34,   77,   28,   29,   30,   31,   32,
   21,   22,   33,   34,   35,   36,   37,   38,   39,   40,
   41,   42,   43,   44,   45,   46,   23,   24,   57,   26,
   27,   71,   72,   98,   39,   73,   74,   75,   76,   78,
   79,   80,   81,   82,   83,   84,   85,   86,   87,   96,
  103,  104,  105,  125,  108,   34,  107,  123,   69,  109,
   69,  110,  208,  111,  113,  153,  116,  155,  117,  118,
  119,  120,  121,  122,   69,   69,   68,   69,  201,   69,
  126,  169,  170,  128,  172,  129,  174,  130,  131,  132,
  133,  134,  135,  136,   69,  137,  138,   70,  139,  189,
  140,  141,   69,   69,  142,  143,  144,  145,  146,  197,
  151,  198,   69,  147,   69,   69,   69,  148,  165,  149,
  150,  152,  154,   69,  207,  156,  209,  210,  211,  163,
  159,  161,  176,  164,  166,   32,   32,   32,   32,   32,
   32,  218,   32,   32,   32,   32,   32,   32,   32,   32,
   32,   32,   32,   32,   32,   32,  167,  157,  158,  160,
  162,  168,  171,  173,  175,  177,  185,  181,   39,   39,
   39,   39,   39,  182,  187,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   34,
   34,   34,   34,   34,   34,  183,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   28,   29,   30,   31,   32,  190,  184,   33,   34,   35,
   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,
   46,   28,   29,   30,   31,   32,  191,  186,   33,   34,
   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,
   45,   46,   28,   29,   30,   31,   32,  195,  188,   33,
   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,
   44,   45,   46,  196,  200,  202,  203,  213,  192,  199,
  204,  217,    9,   11,   43,   45,   47,   11,   19,   25,
   28,   29,   30,   31,   32,  194,  124,   33,   34,   35,
   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,
   46,  205,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   28,
   29,   30,   31,   32,  206,    0,   33,   34,   35,   36,
   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,
   28,   29,   30,   31,   32,  212,    0,   33,   34,   35,
   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,
   46,   28,   29,   30,   31,   32,  214,    0,   33,   34,
   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,
   45,   46,   28,   29,   30,   31,   32,  215,    0,   33,
   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,
   44,   45,   46,   28,   29,   30,   31,   32,  216,    0,
   33,   34,   35,   36,   37,   38,   39,   40,   41,   42,
   43,   44,   45,   46,    0,    0,    0,    0,    0,  219,
    0,    0,    0,    0,    0,   28,   29,   30,   31,   32,
    0,    0,   33,   34,   35,   36,   37,   38,   39,   40,
   41,   42,   43,   44,   45,   46,   37,    0,   28,   29,
   30,   31,   32,    0,    0,   33,   34,   35,   36,   37,
   38,   39,   40,   41,   42,   43,   44,   45,   46,   28,
   29,   30,   31,   32,    0,    0,   33,   34,   35,   36,
   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,
   28,   29,   30,   31,   32,    0,    0,   33,   34,   35,
   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,
   46,   28,   29,   30,   31,   32,    0,    0,   33,   34,
   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,
   45,   46,   28,   29,   30,   31,   32,    0,    0,   33,
   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,
   44,   45,   46,   28,   29,   30,   31,   32,    0,    0,
   33,   34,   35,   36,   37,   38,   39,   40,   41,   42,
   43,   44,   45,   46,    0,    0,    0,    0,    0,    0,
   37,   37,   37,   37,   37,    0,    0,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,
};
short yycheck[] = {                                      40,
   41,   27,   47,   44,  259,   50,   95,   87,   71,   72,
   73,   74,   75,   76,  259,   78,   96,   80,   81,   82,
   83,  257,  258,  259,   87,  259,  260,   30,   31,   32,
   33,   40,   35,   96,   37,   38,   39,   40,   41,   42,
   43,   44,   45,   46,  107,  259,  260,   40,  111,   40,
  259,  125,   41,  267,   57,  259,  260,  261,  262,  263,
   41,   44,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,   41,   41,  259,  123,
  123,   40,   40,  259,  125,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
  259,   41,   41,  259,   41,  125,   44,   41,  153,   44,
  155,   44,  201,   44,   44,  141,   44,  143,   44,   44,
   44,   44,   44,   44,  169,  170,  125,  172,  191,  174,
  259,  157,  158,   41,  160,  259,  162,   41,   41,  259,
  259,  259,  259,  259,  189,  259,  259,  125,   44,  175,
   44,  123,  197,  198,   44,  123,   44,   41,   41,  185,
  259,  187,  207,   44,  209,  210,  211,   41,  125,   44,
   41,  259,  259,  218,  200,  259,  202,  203,  204,   41,
  259,  259,  259,   44,   44,  259,  260,  261,  262,  263,
  264,  217,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  125,  123,  123,  123,
  123,   44,   41,   41,  123,  265,  123,  259,  259,  260,
  261,  262,  263,  259,  123,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  259,
  260,  261,  262,  263,  264,  125,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  259,  260,  261,  262,  263,   41,  125,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  259,  260,  261,  262,  263,   40,  125,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  259,  260,  261,  262,  263,   41,  125,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,
  277,  278,  279,   41,  123,  123,  123,   41,  264,  125,
  123,  123,   41,   41,   41,   41,   41,    4,   13,   22,
  259,  260,  261,  262,  263,  180,  107,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  125,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  259,
  260,  261,  262,  263,  125,   -1,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  259,  260,  261,  262,  263,  125,   -1,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  259,  260,  261,  262,  263,  125,   -1,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  259,  260,  261,  262,  263,  125,   -1,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  259,  260,  261,  262,  263,  125,   -1,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,   -1,   -1,   -1,   -1,   -1,  125,
   -1,   -1,   -1,   -1,   -1,  259,  260,  261,  262,  263,
   -1,   -1,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  125,   -1,  259,  260,
  261,  262,  263,   -1,   -1,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  259,
  260,  261,  262,  263,   -1,   -1,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  259,  260,  261,  262,  263,   -1,   -1,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  259,  260,  261,  262,  263,   -1,   -1,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  259,  260,  261,  262,  263,   -1,   -1,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  259,  260,  261,  262,  263,   -1,   -1,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,   -1,   -1,   -1,   -1,   -1,   -1,
  259,  260,  261,  262,  263,   -1,   -1,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,
};
#define YYFINAL 4
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 279
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'",0,0,"','",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'",0,"'}'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"PROC","FUNC",
"IDEN","LITERAL","CHILDREN","SPOUSES","IF","ELSE","ELSIF","FAMILIES","ICONS",
"WHILE","CALL","FORINDISET","FORINDI","FORNOTES","TRAVERSE","FORNODES",
"FORLIST","FORFAM","BREAK","CONTINUE","RETURN",
};
char *yyrule[] = {
"$accept : rspec",
"rspec : defn",
"rspec : rspec defn",
"defn : proc",
"defn : func",
"defn : IDEN '(' IDEN ')'",
"defn : IDEN '(' LITERAL ')'",
"proc : PROC IDEN '(' idenso ')' '{' tmplts '}'",
"func : FUNC IDEN '(' idenso ')' '{' tmplts '}'",
"idenso :",
"idenso : idens",
"idens : IDEN",
"idens : IDEN ',' idens",
"tmplts : tmplt",
"tmplts : tmplts tmplt",
"tmplt : CHILDREN m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}'",
"tmplt : SPOUSES m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'",
"tmplt : FAMILIES m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'",
"tmplt : FORINDISET m '(' expr ',' IDEN ',' IDEN ',' IDEN ')' '{' tmplts '}'",
"tmplt : FORLIST m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}'",
"tmplt : FORINDI m '(' IDEN ',' IDEN ')' '{' tmplts '}'",
"tmplt : FORFAM m '(' IDEN ',' IDEN ')' '{' tmplts '}'",
"tmplt : FORNOTES m '(' expr ',' IDEN ')' '{' tmplts '}'",
"tmplt : TRAVERSE m '(' expr ',' IDEN ',' IDEN ')' '{' tmplts '}'",
"tmplt : FORNODES m '(' expr ',' IDEN ')' '{' tmplts '}'",
"tmplt : IF m '(' expr secondo ')' '{' tmplts '}' elsifso elseo",
"tmplt : WHILE m '(' expr secondo ')' '{' tmplts '}'",
"tmplt : CALL IDEN m '(' exprso ')'",
"tmplt : BREAK m '(' ')'",
"tmplt : CONTINUE m '(' ')'",
"tmplt : RETURN m '(' exprso ')'",
"tmplt : expr",
"elsifso :",
"elsifso : elsifs",
"elsifs : elsif",
"elsifs : elsif elsifs",
"elsif : ELSIF '(' expr secondo ')' '{' tmplts '}'",
"elseo :",
"elseo : ELSE '{' tmplts '}'",
"expr : IDEN",
"expr : IDEN m '(' exprso ')'",
"expr : LITERAL",
"expr : ICONS",
"exprso :",
"exprso : exprs",
"exprs : expr",
"exprs : expr ',' exprs",
"secondo :",
"secondo : ',' expr",
"m :",
};
#endif
#ifndef YYSTYPE
typedef int YYSTYPE;
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE
#line 249 "yacc.y"

join (list, last)
INTERP list, last;
{
	INTERP prev = NULL;
	while (list) {
		prev = list;
		list = inext(list);
	}
	ASSERT(prev);
	inext(prev) = last;
}

yyerror (str)
STRING str;
{
	extern INT Plineno;
	extern STRING Pfname;

	wprintf("Syntax Error: %s: line %d\n", Pfname, Plineno);
	Perrors++;
}
#line 401 "y.tab.c"
#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 5:
#line 55 "yacc.y"
{
			if (eqstr("global", (STRING) yyvsp[-3]))
				insert_table(globtab, yyvsp[-1], NULL);
		}
break;
case 6:
#line 59 "yacc.y"
{
			if (eqstr("include", (STRING) yyvsp[-3]))
				enqueue_list(Plist, iliteral(((INTERP) yyvsp[-1])));
		}
break;
case 7:
#line 65 "yacc.y"
{
			insert_table(proctab, yyvsp[-6], proc_node(yyvsp[-6], yyvsp[-4], yyvsp[-1]));
		}
break;
case 8:
#line 69 "yacc.y"
{
			insert_table(functab, yyvsp[-6], fdef_node(yyvsp[-6], yyvsp[-4], yyvsp[-1]));
		}
break;
case 9:
#line 73 "yacc.y"
{
			yyval = 0;
		}
break;
case 10:
#line 76 "yacc.y"
{
			yyval = yyvsp[0];
		}
break;
case 11:
#line 79 "yacc.y"
{
			yyval = (INT) iden_node(yyvsp[0]);
		}
break;
case 12:
#line 82 "yacc.y"
{
			yyval = (INT) iden_node(yyvsp[-2]);
			inext(((INTERP)yyval)) = (INTERP) yyvsp[0];
		}
break;
case 13:
#line 87 "yacc.y"
{
			yyval = yyvsp[0];
		}
break;
case 14:
#line 90 "yacc.y"
{
			join((INTERP) yyvsp[-1], (INTERP) yyvsp[0]);
			yyval = yyvsp[-1];
		}
break;
case 15:
#line 96 "yacc.y"
{
			yyval = (INT) children_node(yyvsp[-8], yyvsp[-6], yyvsp[-4], yyvsp[-1]);
			((INTERP)yyval)->i_line = yyvsp[-10];
		}
break;
case 16:
#line 101 "yacc.y"
{
			yyval = spouses_node(yyvsp[-10], yyvsp[-8], yyvsp[-6], yyvsp[-4], yyvsp[-1]);
			((INTERP)yyval)->i_line = yyvsp[-12];
		}
break;
case 17:
#line 106 "yacc.y"
{
			yyval = families_node(yyvsp[-10], yyvsp[-8], yyvsp[-6], yyvsp[-4], yyvsp[-1]);
			((INTERP)yyval)->i_line = yyvsp[-12];
		}
break;
case 18:
#line 111 "yacc.y"
{
			yyval = forindiset_node(yyvsp[-10], yyvsp[-8], yyvsp[-6], yyvsp[-4], yyvsp[-1]);
			((INTERP)yyval)->i_line = (INT) yyvsp[-12];
		}
break;
case 19:
#line 116 "yacc.y"
{
			yyval = forlist_node(yyvsp[-8], yyvsp[-6], yyvsp[-4], yyvsp[-1]);
			((INTERP)yyval)->i_line = (INT) yyvsp[-10];
		}
break;
case 20:
#line 121 "yacc.y"
{
			yyval = forindi_node(yyvsp[-6], yyvsp[-4], yyvsp[-1]);
			((INTERP)yyval)->i_line = (INT) yyvsp[-8];
		}
break;
case 21:
#line 125 "yacc.y"
{
			yyval = forfam_node(yyvsp[-6], yyvsp[-4], yyvsp[-1]);
			((INTERP)yyval)->i_line = (INT) yyvsp[-8];
		}
break;
case 22:
#line 129 "yacc.y"
{
			yyval = fornotes_node(yyvsp[-6], yyvsp[-4], yyvsp[-1]);
			((INTERP)yyval)->i_line = (INT) yyvsp[-8];
		}
break;
case 23:
#line 133 "yacc.y"
{
			yyval = traverse_node(yyvsp[-8], yyvsp[-6], yyvsp[-4], yyvsp[-1]);
			((INTERP)yyval)->i_line = (INT) yyvsp[-10];
		}
break;
case 24:
#line 137 "yacc.y"
{
			yyval = fornodes_node(yyvsp[-6], yyvsp[-4], yyvsp[-1]);
			((INTERP)yyval)->i_line = (INT) yyvsp[-8];
		}
break;
case 25:
#line 141 "yacc.y"
{
			inext(((INTERP)yyvsp[-7])) = (INTERP)yyvsp[-6];
			prev = NULL;  this = (INTERP)yyvsp[-1];
			while (this) {
				prev = this;
				this = (INTERP) ielse(this);
			}
			if (prev) {
				ielse(prev) = (WORD)yyvsp[0];
				yyval = if_node((INTERP)yyvsp[-7], (INTERP)yyvsp[-3],
				    (INTERP)yyvsp[-1]);
			} else
				yyval = if_node((INTERP)yyvsp[-7], (INTERP)yyvsp[-3],
				    (INTERP)yyvsp[0]);
			((INTERP)yyval)->i_line = (INT) yyvsp[-9];
		}
break;
case 26:
#line 157 "yacc.y"
{
			inext(((INTERP)yyvsp[-5])) = (INTERP)yyvsp[-4];
			yyval = while_node(yyvsp[-5], yyvsp[-1]);
			((INTERP)yyval)->i_line = (INT) yyvsp[-7];
		}
break;
case 27:
#line 162 "yacc.y"
{
			yyval = call_node(yyvsp[-4], yyvsp[-1]);
			((INTERP)yyval)->i_line = (INT) yyvsp[-3];
		}
break;
case 28:
#line 166 "yacc.y"
{
			yyval = break_node();
			((INTERP)yyval)->i_line = (INT) yyvsp[-2];
		}
break;
case 29:
#line 170 "yacc.y"
{
			yyval = continue_node();
			((INTERP)yyval)->i_line = (INT) yyvsp[-2];
		}
break;
case 30:
#line 174 "yacc.y"
{
			yyval = return_node(yyvsp[-1]);
			((INTERP)yyval)->i_line = (INT) yyvsp[-3];
		}
break;
case 31:
#line 178 "yacc.y"
{
			yyval = yyvsp[0];
		}
break;
case 32:
#line 182 "yacc.y"
{
			yyval = 0;
		}
break;
case 33:
#line 185 "yacc.y"
{
			yyval = yyvsp[0];
		}
break;
case 34:
#line 189 "yacc.y"
{
			yyval = yyvsp[0];
		}
break;
case 35:
#line 192 "yacc.y"
{
			ielse((INTERP)yyvsp[-1]) = (WORD)yyvsp[0];
			yyval = yyvsp[-1];
		}
break;
case 36:
#line 197 "yacc.y"
{
			inext(((INTERP)yyvsp[-5])) = (INTERP)yyvsp[-4];
			yyval = if_node((INTERP)yyvsp[-5], (INTERP)yyvsp[-1], (INTERP)NULL);
		}
break;
case 37:
#line 201 "yacc.y"
{
			yyval = 0;
		}
break;
case 38:
#line 204 "yacc.y"
{
			yyval = yyvsp[-1];
		}
break;
case 39:
#line 208 "yacc.y"
{
			yyval = (INT) iden_node(yyvsp[0]);
			ielist(((INTERP)yyval)) = NULL;
		}
break;
case 40:
#line 212 "yacc.y"
{
			yyval = (INT) func_node(yyvsp[-4], yyvsp[-1]);
			((INTERP)yyval)->i_line = yyvsp[-3];
		}
break;
case 41:
#line 216 "yacc.y"
{
			yyval = yyvsp[0];
		}
break;
case 42:
#line 219 "yacc.y"
{
			yyval = (INT) icons_node(yyvsp[0]);
		}
break;
case 43:
#line 223 "yacc.y"
{
			yyval = 0;
		}
break;
case 44:
#line 226 "yacc.y"
{
			yyval = yyvsp[0];
		}
break;
case 45:
#line 230 "yacc.y"
{
			yyval = yyvsp[0];
		}
break;
case 46:
#line 233 "yacc.y"
{
			inext(((INTERP)yyvsp[-2])) = (INTERP) yyvsp[0];
			yyval = yyvsp[-2];
		}
break;
case 47:
#line 238 "yacc.y"
{
			yyval = 0;
		}
break;
case 48:
#line 241 "yacc.y"
{
			yyval = yyvsp[0];
		}
break;
case 49:
#line 245 "yacc.y"
{
			yyval = Plineno;
		}
break;
#line 850 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
