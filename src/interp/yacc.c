
/*  A Bison parser, made from yacc.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	PROC	257
#define	FUNC_TOK	258
#define	IDEN	259
#define	SCONS	260
#define	CHILDREN	261
#define	SPOUSES	262
#define	IF	263
#define	ELSE	264
#define	ELSIF	265
#define	FAMILIES	266
#define	ICONS	267
#define	WHILE	268
#define	CALL	269
#define	FORINDISET	270
#define	FORINDI	271
#define	FORNOTES	272
#define	TRAVERSE	273
#define	FORNODES	274
#define	FORLIST_TOK	275
#define	FORFAM	276
#define	FORSOUR	277
#define	FOREVEN	278
#define	FOROTHR	279
#define	BREAK	280
#define	CONTINUE	281
#define	RETURN	282
#define	FATHERS	283
#define	MOTHERS	284
#define	PARENTS	285
#define	FCONS	286

#line 36 "yacc.y"

/*#define YACC_C */
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "indiseq.h"
#include "interp.h"
#include "liflines.h"
#include "screen.h"
#include <stdlib.h>

extern TABLE proctab, functab;
static PNODE this, prev;
extern LIST Plist;
INT Yival;
FLOAT Yfval;

static void join (PNODE list, PNODE last);
static void yyerror (STRING str);

/* Make sure it can hold any pointer */
#define YYSTYPE void *
#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		292
#define	YYFLAG		-32768
#define	YYNTBASE	38

#define YYTRANSLATE(x) ((unsigned)(x) <= 286 ? yytranslate[x] : 55)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    33,
    34,     2,     2,    37,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    35,     2,    36,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,     7,     9,    14,    19,    28,    37,    38,
    40,    42,    46,    48,    51,    64,    79,    94,   109,   124,
   137,   152,   165,   176,   187,   198,   209,   220,   231,   244,
   255,   267,   277,   284,   289,   294,   300,   302,   303,   305,
   307,   310,   319,   320,   325,   327,   333,   335,   337,   339,
   340,   342,   344,   348,   349,   352
};

static const short yyrhs[] = {    39,
     0,    38,    39,     0,    40,     0,    41,     0,     5,    33,
     5,    34,     0,     5,    33,     6,    34,     0,     3,     5,
    33,    42,    34,    35,    44,    36,     0,     4,     5,    33,
    42,    34,    35,    44,    36,     0,     0,    43,     0,     5,
     0,     5,    37,    43,     0,    45,     0,    44,    45,     0,
     7,    54,    33,    50,    37,     5,    37,     5,    34,    35,
    44,    36,     0,     8,    54,    33,    50,    37,     5,    37,
     5,    37,     5,    34,    35,    44,    36,     0,    12,    54,
    33,    50,    37,     5,    37,     5,    37,     5,    34,    35,
    44,    36,     0,    29,    54,    33,    50,    37,     5,    37,
     5,    37,     5,    34,    35,    44,    36,     0,    30,    54,
    33,    50,    37,     5,    37,     5,    37,     5,    34,    35,
    44,    36,     0,    31,    54,    33,    50,    37,     5,    37,
     5,    34,    35,    44,    36,     0,    16,    54,    33,    50,
    37,     5,    37,     5,    37,     5,    34,    35,    44,    36,
     0,    21,    54,    33,    50,    37,     5,    37,     5,    34,
    35,    44,    36,     0,    17,    54,    33,     5,    37,     5,
    34,    35,    44,    36,     0,    18,    54,    33,    50,    37,
     5,    34,    35,    44,    36,     0,    22,    54,    33,     5,
    37,     5,    34,    35,    44,    36,     0,    23,    54,    33,
     5,    37,     5,    34,    35,    44,    36,     0,    24,    54,
    33,     5,    37,     5,    34,    35,    44,    36,     0,    25,
    54,    33,     5,    37,     5,    34,    35,    44,    36,     0,
    19,    54,    33,    50,    37,     5,    37,     5,    34,    35,
    44,    36,     0,    20,    54,    33,    50,    37,     5,    34,
    35,    44,    36,     0,     9,    54,    33,    50,    53,    34,
    35,    44,    36,    46,    49,     0,    14,    54,    33,    50,
    53,    34,    35,    44,    36,     0,    15,     5,    54,    33,
    51,    34,     0,    26,    54,    33,    34,     0,    27,    54,
    33,    34,     0,    28,    54,    33,    51,    34,     0,    50,
     0,     0,    47,     0,    48,     0,    48,    47,     0,    11,
    33,    50,    53,    34,    35,    44,    36,     0,     0,    10,
    35,    44,    36,     0,     5,     0,     5,    54,    33,    51,
    34,     0,     6,     0,    13,     0,    32,     0,     0,    52,
     0,    50,     0,    50,    37,    52,     0,     0,    37,    50,
     0,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    69,    70,    73,    74,    75,    79,    89,    93,    97,   100,
   103,   106,   111,   114,   119,   124,   129,   134,   139,   144,
   149,   154,   159,   164,   168,   172,   177,   182,   187,   191,
   195,   211,   216,   220,   224,   228,   232,   236,   239,   243,
   246,   251,   255,   258,   262,   266,   270,   273,   276,   280,
   283,   287,   290,   295,   298,   302
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","PROC","FUNC_TOK",
"IDEN","SCONS","CHILDREN","SPOUSES","IF","ELSE","ELSIF","FAMILIES","ICONS","WHILE",
"CALL","FORINDISET","FORINDI","FORNOTES","TRAVERSE","FORNODES","FORLIST_TOK",
"FORFAM","FORSOUR","FOREVEN","FOROTHR","BREAK","CONTINUE","RETURN","FATHERS",
"MOTHERS","PARENTS","FCONS","'('","')'","'{'","'}'","','","rspec","defn","proc",
"func","idenso","idens","tmplts","tmplt","elsifso","elsifs","elsif","elseo",
"expr","exprso","exprs","secondo","m", NULL
};
#endif

static const short yyr1[] = {     0,
    38,    38,    39,    39,    39,    39,    40,    41,    42,    42,
    43,    43,    44,    44,    45,    45,    45,    45,    45,    45,
    45,    45,    45,    45,    45,    45,    45,    45,    45,    45,
    45,    45,    45,    45,    45,    45,    45,    46,    46,    47,
    47,    48,    49,    49,    50,    50,    50,    50,    50,    51,
    51,    52,    52,    53,    53,    54
};

static const short yyr2[] = {     0,
     1,     2,     1,     1,     4,     4,     8,     8,     0,     1,
     1,     3,     1,     2,    12,    14,    14,    14,    14,    12,
    14,    12,    10,    10,    10,    10,    10,    10,    12,    10,
    11,     9,     6,     4,     4,     5,     1,     0,     1,     1,
     2,     8,     0,     4,     1,     5,     1,     1,     1,     0,
     1,     1,     3,     0,     2,     0
};

static const short yydefact[] = {     0,
     0,     0,     0,     0,     1,     3,     4,     0,     0,     0,
     2,     9,     9,     0,     0,    11,     0,    10,     0,     5,
     6,     0,     0,     0,    12,     0,     0,    45,    47,    56,
    56,    56,    56,    48,    56,     0,    56,    56,    56,    56,
    56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
    56,    56,    49,     0,    13,    37,     0,     0,     0,     0,
     0,     0,     0,    56,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     7,    14,     8,    50,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    50,     0,     0,     0,    52,     0,    51,     0,
     0,    54,     0,    54,    50,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    34,    35,     0,     0,     0,
     0,     0,    46,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    36,     0,     0,     0,    53,     0,     0,    55,     0,     0,
     0,    33,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    38,     0,    32,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    43,    39,    40,     0,     0,    23,    24,     0,
    30,     0,    25,    26,    27,    28,     0,     0,     0,     0,
     0,     0,     0,    31,    41,     0,     0,     0,     0,     0,
     0,     0,    15,     0,    54,     0,     0,     0,    29,    22,
     0,     0,    20,     0,     0,     0,     0,     0,     0,     0,
    16,     0,    44,    17,    21,    18,    19,     0,     0,    42,
     0,     0
};

static const short yydefgoto[] = {     4,
     5,     6,     7,    17,    18,    54,    55,   233,   234,   235,
   254,    56,   108,   109,   137,    58
};

static const short yypact[] = {    33,
     4,    10,    -8,    30,-32768,-32768,-32768,    -6,    -4,    13,
-32768,    34,    34,     6,     7,     5,    12,-32768,    14,-32768,
-32768,    34,     9,    15,-32768,   899,   899,    18,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,    40,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,    77,-32768,-32768,   109,    19,    25,    27,
    44,    45,    46,-32768,    47,    48,    54,    78,    79,    86,
    87,   110,   111,   113,   114,   115,   116,   117,   119,   121,
-32768,-32768,-32768,    11,    11,    11,    11,    11,    11,   122,
    11,    42,    11,    11,    11,    11,   105,   137,   151,   152,
   125,   126,    11,    11,    11,    11,   124,   128,-32768,   127,
   129,   130,   131,   130,    11,   134,   138,   140,   145,   146,
   147,   148,   149,   150,   153,-32768,-32768,   154,   157,   159,
   164,    11,-32768,   158,   160,    11,   155,   186,   168,   170,
   200,   201,   202,   203,   204,   205,   206,   207,   209,   211,
-32768,   212,   213,   214,-32768,   183,   184,-32768,   188,   189,
   190,-32768,   191,   193,   195,   194,   196,   197,   208,   215,
   221,   222,   198,   210,   220,   233,   238,   899,   241,   899,
   253,   224,   225,   257,   234,   265,   258,   266,   267,   289,
   287,   320,   321,   260,   296,   259,   297,   291,   319,   899,
   899,   324,   899,   331,   899,   899,   899,   899,   329,   351,
   356,   322,   384,   386,   393,-32768,   415,   323,   355,   394,
   387,   395,   419,   451,   483,   515,   416,   417,   418,   899,
   420,   428,   442,-32768,   386,   450,   452,-32768,-32768,   899,
-32768,   899,-32768,-32768,-32768,-32768,   459,   460,   899,   547,
   427,    11,   481,-32768,-32768,   482,   490,   579,   611,   491,
   513,   643,-32768,   899,   130,   899,   899,   899,-32768,-32768,
   899,   899,-32768,   675,   484,   707,   739,   771,   803,   835,
-32768,   514,-32768,-32768,-32768,-32768,-32768,   899,   867,-32768,
   485,-32768
};

static const short yypgoto[] = {-32768,
   546,-32768,-32768,   544,   536,   -27,   -26,-32768,   345,-32768,
-32768,   -83,   -89,   449,  -107,    24
};


#define	YYLAST		931


static const short yytable[] = {    57,
   107,   110,   111,   112,   113,   114,   139,   116,     8,   118,
   119,   120,   121,   128,     9,    28,    29,    14,    15,   107,
   129,   130,   131,    34,    10,   140,    12,    82,    13,   291,
    82,   107,     1,     2,     3,     1,     2,     3,    16,    20,
    21,    22,    53,    26,    64,    23,   117,    24,   107,    27,
   -56,    84,   158,    59,    60,    61,    62,    85,    63,    86,
    65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
    75,    76,    77,    78,    79,    80,    87,    88,    89,    91,
    92,    28,    29,    30,    31,    32,    93,    90,    33,    34,
    35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
    45,    46,    47,    48,    49,    50,    51,    52,    53,   122,
    94,    95,    81,    28,    29,    30,    31,    32,    96,    97,
    33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
    43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
    53,   123,    98,    99,    83,   100,   101,   102,   103,   104,
   196,   105,   198,   106,   115,   124,   125,   275,   126,   127,
   132,   133,   156,   134,   157,   135,   136,   138,   265,    82,
   141,    82,   218,   219,   142,   221,   143,   223,   224,   225,
   226,   144,   145,   146,   147,   148,   149,   151,   159,   150,
   160,    82,    82,   152,    82,   153,    82,    82,    82,    82,
   154,   161,   250,   162,   163,   164,   165,   166,   167,   168,
   169,   170,   258,   171,   259,   172,   173,   174,   175,   176,
   177,   262,   178,    82,   180,   179,   182,   181,   183,   185,
   184,    82,    82,   186,   191,    82,   274,   194,   276,   277,
   278,   187,   195,   279,   280,   197,   192,    82,   188,    82,
    82,    82,    82,    82,   189,   190,   193,   199,   200,   201,
   289,   202,    82,    28,    29,    30,    31,    32,   203,   204,
    33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
    43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
    53,   209,   205,   212,   214,    28,    29,    30,    31,    32,
   206,   207,    33,    34,    35,    36,    37,    38,    39,    40,
    41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
    51,    52,    53,   208,   210,   211,   216,    28,    29,    30,
    31,    32,   213,   215,    33,    34,    35,    36,    37,    38,
    39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
    49,    50,    51,    52,    53,   217,   230,   220,   238,    28,
    29,    30,    31,    32,   222,   227,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,   228,   231,   229,
   239,    28,    29,    30,    31,    32,   232,   236,    33,    34,
    35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
    45,    46,    47,    48,    49,    50,    51,    52,    53,   237,
   247,   248,   241,    28,    29,    30,    31,    32,   240,   242,
    33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
    43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
    53,   253,   249,   251,   243,    28,    29,    30,    31,    32,
   252,   264,    33,    34,    35,    36,    37,    38,    39,    40,
    41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
    51,    52,    53,   256,   292,   257,   244,    28,    29,    30,
    31,    32,   260,   261,    33,    34,    35,    36,    37,    38,
    39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
    49,    50,    51,    52,    53,   266,   267,   282,   245,    28,
    29,    30,    31,    32,   268,   271,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,   272,   288,    11,
   246,    28,    29,    30,    31,    32,    19,    25,    33,    34,
    35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
    45,    46,    47,    48,    49,    50,    51,    52,    53,   255,
   155,     0,   263,    28,    29,    30,    31,    32,     0,     0,
    33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
    43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
    53,     0,     0,     0,   269,    28,    29,    30,    31,    32,
     0,     0,    33,    34,    35,    36,    37,    38,    39,    40,
    41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
    51,    52,    53,     0,     0,     0,   270,    28,    29,    30,
    31,    32,     0,     0,    33,    34,    35,    36,    37,    38,
    39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
    49,    50,    51,    52,    53,     0,     0,     0,   273,    28,
    29,    30,    31,    32,     0,     0,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,     0,     0,     0,
   281,    28,    29,    30,    31,    32,     0,     0,    33,    34,
    35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
    45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
     0,     0,   283,    28,    29,    30,    31,    32,     0,     0,
    33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
    43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
    53,     0,     0,     0,   284,    28,    29,    30,    31,    32,
     0,     0,    33,    34,    35,    36,    37,    38,    39,    40,
    41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
    51,    52,    53,     0,     0,     0,   285,    28,    29,    30,
    31,    32,     0,     0,    33,    34,    35,    36,    37,    38,
    39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
    49,    50,    51,    52,    53,     0,     0,     0,   286,    28,
    29,    30,    31,    32,     0,     0,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,     0,     0,     0,
   287,    28,    29,    30,    31,    32,     0,     0,    33,    34,
    35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
    45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
     0,     0,   290,    28,    29,    30,    31,    32,     0,     0,
    33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
    43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
    53
};

static const short yycheck[] = {    27,
    84,    85,    86,    87,    88,    89,   114,    91,     5,    93,
    94,    95,    96,   103,     5,     5,     6,     5,     6,   103,
   104,   105,   106,    13,    33,   115,    33,    54,    33,     0,
    57,   115,     3,     4,     5,     3,     4,     5,     5,    34,
    34,    37,    32,    35,     5,    34,     5,    34,   132,    35,
    33,    33,   136,    30,    31,    32,    33,    33,    35,    33,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    33,    33,    33,    33,
    33,     5,     6,     7,     8,     9,    33,    64,    12,    13,
    14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,     5,
    33,    33,    36,     5,     6,     7,     8,     9,    33,    33,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
    32,     5,    33,    33,    36,    33,    33,    33,    33,    33,
   178,    33,   180,    33,    33,     5,     5,   265,    34,    34,
    37,    34,     5,    37,     5,    37,    37,    37,   252,   196,
    37,   198,   200,   201,    37,   203,    37,   205,   206,   207,
   208,    37,    37,    37,    37,    37,    37,    34,    34,    37,
     5,   218,   219,    37,   221,    37,   223,   224,   225,   226,
    37,    34,   230,    34,     5,     5,     5,     5,     5,     5,
     5,     5,   240,     5,   242,     5,     5,     5,     5,    37,
    37,   249,    35,   250,    35,    37,    34,    37,    34,    34,
    37,   258,   259,    37,    37,   262,   264,     5,   266,   267,
   268,    34,     5,   271,   272,     5,    37,   274,    34,   276,
   277,   278,   279,   280,    34,    34,    37,     5,    35,    35,
   288,     5,   289,     5,     6,     7,     8,     9,    35,     5,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
    32,     5,    35,    34,    36,     5,     6,     7,     8,     9,
    35,    35,    12,    13,    14,    15,    16,    17,    18,    19,
    20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
    30,    31,    32,    35,     5,     5,    36,     5,     6,     7,
     8,     9,    37,    37,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    30,    31,    32,    37,    35,    34,    36,     5,
     6,     7,     8,     9,    34,    37,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    37,     5,    34,
    36,     5,     6,     7,     8,     9,    11,     5,    12,    13,
    14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,     5,
     5,     5,    36,     5,     6,     7,     8,     9,    35,    35,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
    32,    10,    35,    34,    36,     5,     6,     7,     8,     9,
    33,    35,    12,    13,    14,    15,    16,    17,    18,    19,
    20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
    30,    31,    32,    34,     0,    34,    36,     5,     6,     7,
     8,     9,    34,    34,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    30,    31,    32,    35,    35,    34,    36,     5,
     6,     7,     8,     9,    35,    35,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    35,    35,     4,
    36,     5,     6,     7,     8,     9,    13,    22,    12,    13,
    14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,   235,
   132,    -1,    36,     5,     6,     7,     8,     9,    -1,    -1,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
    32,    -1,    -1,    -1,    36,     5,     6,     7,     8,     9,
    -1,    -1,    12,    13,    14,    15,    16,    17,    18,    19,
    20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
    30,    31,    32,    -1,    -1,    -1,    36,     5,     6,     7,
     8,     9,    -1,    -1,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    30,    31,    32,    -1,    -1,    -1,    36,     5,
     6,     7,     8,     9,    -1,    -1,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    -1,    -1,    -1,
    36,     5,     6,     7,     8,     9,    -1,    -1,    12,    13,
    14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,    -1,
    -1,    -1,    36,     5,     6,     7,     8,     9,    -1,    -1,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
    32,    -1,    -1,    -1,    36,     5,     6,     7,     8,     9,
    -1,    -1,    12,    13,    14,    15,    16,    17,    18,    19,
    20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
    30,    31,    32,    -1,    -1,    -1,    36,     5,     6,     7,
     8,     9,    -1,    -1,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    30,    31,    32,    -1,    -1,    -1,    36,     5,
     6,     7,     8,     9,    -1,    -1,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    -1,    -1,    -1,
    36,     5,     6,     7,     8,     9,    -1,    -1,    12,    13,
    14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,    -1,
    -1,    -1,    36,     5,     6,     7,     8,     9,    -1,    -1,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
    32
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 5:
#line 75 "yacc.y"
{
			if (eqstr("global", (STRING) yyvsp[-3]))
				insert_pvtable(globtab, (STRING)yyvsp[-1], PANY, NULL);
		;
    break;}
case 6:
#line 79 "yacc.y"
{
			if (eqstr("include", (STRING) yyvsp[-3]))
/*
				enqueue_list(Plist, iliteral(((PNODE) $3)));
*/
enqueue_list(Plist, pvalue((PVALUE) ivalue((PNODE) yyvsp[-1])));

		;
    break;}
case 7:
#line 89 "yacc.y"
{
			insert_table(proctab, (STRING)yyvsp[-6], (WORD)proc_node((STRING)yyvsp[-6], (PNODE)yyvsp[-4], (PNODE)yyvsp[-1]));
		;
    break;}
case 8:
#line 93 "yacc.y"
{
			insert_table(functab, (STRING)yyvsp[-6], (WORD)fdef_node((STRING)yyvsp[-6], (PNODE)yyvsp[-4], (PNODE)yyvsp[-1]));
		;
    break;}
case 9:
#line 97 "yacc.y"
{
			yyval = 0;
		;
    break;}
case 10:
#line 100 "yacc.y"
{
			yyval = yyvsp[0];
		;
    break;}
case 11:
#line 103 "yacc.y"
{
			yyval = iden_node((STRING)yyvsp[0]);
		;
    break;}
case 12:
#line 106 "yacc.y"
{
			yyval = iden_node((STRING)yyvsp[-2]);
			inext(((PNODE)yyval)) = (PNODE) yyvsp[0];
		;
    break;}
case 13:
#line 111 "yacc.y"
{
			yyval = yyvsp[0];
		;
    break;}
case 14:
#line 114 "yacc.y"
{
			join((PNODE) yyvsp[-1], (PNODE) yyvsp[0]);
			yyval = yyvsp[-1];
		;
    break;}
case 15:
#line 120 "yacc.y"
{
			yyval = children_node((PNODE)yyvsp[-8], (STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT)yyvsp[-10];
		;
    break;}
case 16:
#line 125 "yacc.y"
{
			yyval = spouses_node((PNODE)yyvsp[-10], (STRING)yyvsp[-8], (STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT)yyvsp[-12];
		;
    break;}
case 17:
#line 130 "yacc.y"
{
			yyval = families_node((PNODE)yyvsp[-10], (STRING)yyvsp[-8], (STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT)yyvsp[-12];
		;
    break;}
case 18:
#line 135 "yacc.y"
{
			yyval = fathers_node((PNODE)yyvsp[-10], (STRING)yyvsp[-8], (STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT)yyvsp[-12];
		;
    break;}
case 19:
#line 140 "yacc.y"
{
			yyval = mothers_node((PNODE)yyvsp[-10], (STRING)yyvsp[-8], (STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT)yyvsp[-12];
		;
    break;}
case 20:
#line 145 "yacc.y"
{
			yyval = parents_node((PNODE)yyvsp[-8], (STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT)yyvsp[-10];
		;
    break;}
case 21:
#line 150 "yacc.y"
{
			yyval = forindiset_node((PNODE)yyvsp[-10], (STRING)yyvsp[-8], (STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT) yyvsp[-12];
		;
    break;}
case 22:
#line 155 "yacc.y"
{
			yyval = forlist_node((PNODE)yyvsp[-8], (STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT) yyvsp[-10];
		;
    break;}
case 23:
#line 160 "yacc.y"
{
			yyval = forindi_node((STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT) yyvsp[-8];
		;
    break;}
case 24:
#line 164 "yacc.y"
{
                        yyval = fornotes_node((PNODE)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
                        ((PNODE)yyval)->i_line = (INT) yyvsp[-8];
                ;
    break;}
case 25:
#line 168 "yacc.y"
{
			yyval = forfam_node((STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT) yyvsp[-8];
		;
    break;}
case 26:
#line 173 "yacc.y"
{
			yyval = forsour_node((STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT) yyvsp[-8];
		;
    break;}
case 27:
#line 178 "yacc.y"
{
			yyval = foreven_node((STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT) yyvsp[-8];
		;
    break;}
case 28:
#line 183 "yacc.y"
{
			yyval = forothr_node((STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT) yyvsp[-8];
		;
    break;}
case 29:
#line 187 "yacc.y"
{
			yyval = traverse_node((PNODE)yyvsp[-8], (STRING)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT) yyvsp[-10];
		;
    break;}
case 30:
#line 191 "yacc.y"
{
			yyval = fornodes_node((PNODE)yyvsp[-6], (STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT) yyvsp[-8];
		;
    break;}
case 31:
#line 195 "yacc.y"
{
			inext(((PNODE)yyvsp[-7])) = (PNODE)yyvsp[-6];
			prev = NULL;  this = (PNODE)yyvsp[-1];
			while (this) {
				prev = this;
				this = (PNODE) ielse(this);
			}
			if (prev) {
				ielse(prev) = (WORD)yyvsp[0];
				yyval = if_node((PNODE)yyvsp[-7], (PNODE)yyvsp[-3],
				    (PNODE)yyvsp[-1]);
			} else
				yyval = if_node((PNODE)yyvsp[-7], (PNODE)yyvsp[-3],
				    (PNODE)yyvsp[0]);
			((PNODE)yyval)->i_line = (INT) yyvsp[-9];
		;
    break;}
case 32:
#line 211 "yacc.y"
{
			inext(((PNODE)yyvsp[-5])) = (PNODE)yyvsp[-4];
			yyval = while_node((PNODE)yyvsp[-5], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT) yyvsp[-7];
		;
    break;}
case 33:
#line 216 "yacc.y"
{
			yyval = call_node((STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT) yyvsp[-3];
		;
    break;}
case 34:
#line 220 "yacc.y"
{
			yyval = break_node();
			((PNODE)yyval)->i_line = (INT) yyvsp[-2];
		;
    break;}
case 35:
#line 224 "yacc.y"
{
			yyval = continue_node();
			((PNODE)yyval)->i_line = (INT) yyvsp[-2];
		;
    break;}
case 36:
#line 228 "yacc.y"
{
			yyval = return_node((PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT) yyvsp[-3];
		;
    break;}
case 37:
#line 232 "yacc.y"
{
			yyval = yyvsp[0];
		;
    break;}
case 38:
#line 236 "yacc.y"
{
			yyval = 0;
		;
    break;}
case 39:
#line 239 "yacc.y"
{
			yyval = yyvsp[0];
		;
    break;}
case 40:
#line 243 "yacc.y"
{
			yyval = yyvsp[0];
		;
    break;}
case 41:
#line 246 "yacc.y"
{
			ielse((PNODE)yyvsp[-1]) = (WORD)yyvsp[0];
			yyval = yyvsp[-1];
		;
    break;}
case 42:
#line 251 "yacc.y"
{
			inext(((PNODE)yyvsp[-5])) = (PNODE)yyvsp[-4];
			yyval = if_node((PNODE)yyvsp[-5], (PNODE)yyvsp[-1], (PNODE)NULL);
		;
    break;}
case 43:
#line 255 "yacc.y"
{
			yyval = 0;
		;
    break;}
case 44:
#line 258 "yacc.y"
{
			yyval = yyvsp[-1];
		;
    break;}
case 45:
#line 262 "yacc.y"
{
			yyval = iden_node((STRING)yyvsp[0]);
			iargs(((PNODE)yyval)) = NULL;
		;
    break;}
case 46:
#line 266 "yacc.y"
{
			yyval = func_node((STRING)yyvsp[-4], (PNODE)yyvsp[-1]);
			((PNODE)yyval)->i_line = (INT)yyvsp[-3];
		;
    break;}
case 47:
#line 270 "yacc.y"
{
			yyval = yyvsp[0];
		;
    break;}
case 48:
#line 273 "yacc.y"
{
			yyval = icons_node(Yival);
		;
    break;}
case 49:
#line 276 "yacc.y"
{
			yyval = fcons_node(Yfval);
		;
    break;}
case 50:
#line 280 "yacc.y"
{
			yyval = 0;
		;
    break;}
case 51:
#line 283 "yacc.y"
{
			yyval = yyvsp[0];
		;
    break;}
case 52:
#line 287 "yacc.y"
{
			yyval = yyvsp[0];
		;
    break;}
case 53:
#line 290 "yacc.y"
{
			inext(((PNODE)yyvsp[-2])) = (PNODE) yyvsp[0];
			yyval = yyvsp[-2];
		;
    break;}
case 54:
#line 295 "yacc.y"
{
			yyval = 0;
		;
    break;}
case 55:
#line 298 "yacc.y"
{
			yyval = yyvsp[0];
		;
    break;}
case 56:
#line 302 "yacc.y"
{
			yyval = (YYSTYPE)Plineno;
		;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 305 "yacc.y"


void
join (PNODE list,
      PNODE last)
{
	PNODE prev = NULL;
	while (list) {
		prev = list;
		list = inext(list);
	}
	ASSERT(prev);
	inext(prev) = last;
}

void
yyerror (STRING str)
{
	extern INT Plineno;
	extern STRING Pfname;

	llwprintf("Syntax Error (%s): %s: line %d\n", str, Pfname, Plineno);
	Perrors++;
}
