/* 
   menuitem.h
   Copyright (c) 1999-2001 Perry Rapp
   Created: 1999/03 for private build of LifeLines
	Brought into repository: 2001/01/28

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
/*
 menuitem.h - header file for menu items
 Copyright (c) 1999-2001 by Perry Rapp; all rights reserved
  Created in 1999/03 private build of LifeLines
  Added to repository during 3.0.6 development
 
 Menu layout code
 This is a reimplementation of the menus to move from
 fixed-size static menus, to paging, resizable menus.
*/

#ifndef _MENUITEM_H
#define _MENUITEM_H

/* each menu item has display text and selection character(s) */
typedef struct MenuItem_struct {
	STRING Display;
	STRING Choices;
	INT Command;
} MenuItem;
typedef struct MenuItemOption_struct {
	STRING Display1;
	STRING Display2;
	STRING Choices;
	INT Command;
} MenuItemOption;


/* special menu items added on the fly */
MenuItem g_MenuItemOther, g_MenuItemQuit, g_MenuItemOptions;
MenuItem f_MenuItemSyncMoves;

/* each screen has a lot of menu information */
typedef struct ScreenInfo_struct {
	STRING Title;    /* string at bottom of screen */
	INT MenuRows;    /* height of menu (at start) */
	INT MenuCols;    /* columns in this menu (3 for big, 1 for list) */
	INT MenuSize;    /* total #items in this menu */
	INT MenuPage;    /* which page of menu currently displayed */
	char Commands[50]; /* valid commands */
	char ExtCommands[50]; /* valid extended commands (after *) */
	char OptionCommands[50]; /* valid option commands (after $) */
	MenuItem ** Menu;  /* array of pointers to items */
	MenuItemOption ** MenuOptions;
	} ScreenInfo;

ScreenInfo f_ScreenInfo[MAX_SCREEN];


void menuitem_initialize();

enum { 
  CMD_QUIT, CMD_MORE, CMD_OPTIONS
  , CMD_EDIT, CMD_PERSON, CMD_FATHER
  , CMD_MOTHER, CMD_SPOUSE, CMD_CHILDREN, CMD_UPSIB, CMD_DOWNSIB
  , CMD_FAMILY, CMD_2FAM, CMD_PARENTS, CMD_2PAR, CMD_BROWSE
  , CMD_TOP, CMD_BOTTOM, CMD_ADDASSPOUSE, CMD_ADDASCHILD
  , CMD_ADDSPOUSE, CMD_ADDCHILD, CMD_ADDFAMILY, CMD_PEDIGREE
  , CMD_SWAPFAMILIES, CMD_SWAPCHILDREN, CMD_SWAPTOPBOTTOM
  , CMD_NEWPERSON, CMD_NEWFAMILY, CMD_TANDEM
  , CMD_REMOVEASSPOUSE, CMD_REMOVEASCHILD, CMD_REMOVESPOUSE, CMD_REMOVECHILD
  , CMD_SOURCES
  , CMD_SHOWSOURCES, CMD_HIDESOURCES
};

#endif /* _MENUITEM_H */
