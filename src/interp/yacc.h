/* A Bison parser, made from ../../../src/interp/yacc.y, by GNU bison 1.75.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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

#ifndef BISON_YACC_H
# define BISON_YACC_H

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     PROC = 258,
     FUNC_TOK = 259,
     IDEN = 260,
     SCONS = 261,
     CHILDREN = 262,
     SPOUSES = 263,
     IF = 264,
     ELSE = 265,
     ELSIF = 266,
     FAMILIES = 267,
     ICONS = 268,
     WHILE = 269,
     CALL = 270,
     FORINDISET = 271,
     FORINDI = 272,
     FORNOTES = 273,
     TRAVERSE = 274,
     FORNODES = 275,
     FORLIST_TOK = 276,
     FORFAM = 277,
     FORSOUR = 278,
     FOREVEN = 279,
     FOROTHR = 280,
     BREAK = 281,
     CONTINUE = 282,
     RETURN = 283,
     FATHERS = 284,
     MOTHERS = 285,
     PARENTS = 286,
     FCONS = 287
   };
#endif
#define PROC 258
#define FUNC_TOK 259
#define IDEN 260
#define SCONS 261
#define CHILDREN 262
#define SPOUSES 263
#define IF 264
#define ELSE 265
#define ELSIF 266
#define FAMILIES 267
#define ICONS 268
#define WHILE 269
#define CALL 270
#define FORINDISET 271
#define FORINDI 272
#define FORNOTES 273
#define TRAVERSE 274
#define FORNODES 275
#define FORLIST_TOK 276
#define FORFAM 277
#define FORSOUR 278
#define FOREVEN 279
#define FOROTHR 280
#define BREAK 281
#define CONTINUE 282
#define RETURN 283
#define FATHERS 284
#define MOTHERS 285
#define PARENTS 286
#define FCONS 287




#ifndef YYSTYPE
typedef int yystype;
# define YYSTYPE yystype
#endif




#endif /* not BISON_YACC_H */

