/* 
   Copyright (c) 1999-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * dynmenu.c -- dynamically resizable & pageable menus
 *  This module holds the code to resize & page thru the menus
 *   Created: 1999/03 by Perry Rapp
 *   Brought into repository: 2001/01/28 by Perry Rapp
 *==============================================================*/

#include "llstdlib.h"
#include "feedback.h"
#include "gedcom.h"
#include "menuitem.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSttlindibrw, qSttlfambrw, qSttl2perbrw, qSttl2fambrw;
extern STRING qSttlauxbrw, qSttllstbrw;

/*********************************************
 * local types
 *********************************************/


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */

/*********************************************
 * local variables
 *********************************************/

/* menu items added to all menus */
static MenuItem * f_ExtraItems[] =
{
	&g_MenuItemOther,
	&g_MenuItemQuit,
	0
};


/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*============================
 * dynmenu_init - initialize one dynamic menu
 * Safe to call on either empty or full DYNMENU
 * Created: 2002/10/24, Perry Rapp
 *==========================*/
void
dynmenu_init (DYNMENU dynmenu , STRING title, INT MenuRows, INT MenuCols
	, INT MinCols, INT MaxCols
	, INT MinRows, INT MaxRows
	, INT MenuTop, INT MenuLeft, INT MenuWidth
	, INT MenuSize, MenuItem ** MenuItems)
{
	dynmenu->rows = MenuRows;
	dynmenu->cols = MenuCols;
	dynmenu->size = MenuSize;
	dynmenu->page = 0;
	/* dynmenu->pages */
	/* dynmenu->pageitems */
	dynmenu->mincols = MinCols;
	dynmenu->maxcols = MaxCols;
	dynmenu->minrows = MinRows;
	dynmenu->maxrows = MaxRows;
	dynmenu->hidden = 0;
	dynmenu->dirty = 1;
	dynmenu->top = MenuTop;
	dynmenu->bottom = MenuTop + MenuRows-1;
	dynmenu->left = MenuLeft;
	dynmenu->width = MenuLeft + MenuWidth-1;
	menuset_init(&dynmenu->menuset, title, MenuItems, f_ExtraItems);
}
/*============================
 * dynmenu_clear - Free memory in dynmenu
 * reentrant
 * Created: 2002/10/24, Perry Rapp
 *==========================*/
void
dynmenu_clear (DYNMENU dynmenu)
{
	menuset_clear(&dynmenu->menuset);
}
/*==================================================================
 * dynmenu_next_page - show next page of menu choices
 * Created: 2002/10/24, Perry Rapp
 *================================================================*/
void
dynmenu_next_page (DYNMENU dynmenu)
{
	INT MenuSize = dynmenu->size;
	INT pageitems = dynmenu->pageitems;
	if (dynmenu->pages == 1) return;
	++dynmenu->page;
	if (dynmenu->page > (MenuSize-1)/pageitems)
		dynmenu->page = 0;
	dynmenu->dirty = 1;
}
/*==================================================================
 * dynmenu_adjust_height - Increase or decrease screen size of menu
 * Created: 2002/10/28, Perry Rapp
 *================================================================*/
void
dynmenu_adjust_height (DYNMENU dynmenu, INT delta)
{
	INT oldrows = dynmenu->rows;
	dynmenu->rows += delta;
	if (dynmenu->rows < dynmenu->minrows)
		dynmenu->rows = dynmenu->minrows;
	if (dynmenu->rows > dynmenu->maxrows)
		dynmenu->rows = dynmenu->maxrows;
	if (dynmenu->rows != oldrows)
		dynmenu->dirty = 1;
	dynmenu->top = dynmenu->bottom - (dynmenu->rows-1);
}
/*==================================================================
 * dynmenu_adjust_menu_cols - Change # of columns in menu
 * Created: 2002/10/28, Perry Rapp
 *================================================================*/
void
dynmenu_adjust_menu_cols (DYNMENU dynmenu, INT delta)
{
	INT oldcols = dynmenu->cols;
	dynmenu->cols += delta;
	if (dynmenu->cols < dynmenu->mincols)
		dynmenu->cols = dynmenu->mincols;
	else if (dynmenu->cols > dynmenu->maxcols)
		dynmenu->cols = dynmenu->maxcols;
	if (dynmenu->cols != oldcols)
		dynmenu->dirty = 1;
}
/*==================================================================
 * dynmenu_get_menuset - Return the menuset (keystroke to command info)
 * Created: 2002/10/28, Perry Rapp
 *================================================================*/
MENUSET
dynmenu_get_menuset (DYNMENU dynmenu)
{
	return &dynmenu->menuset;
}
/*==================================================================
 * dynmenu_toggle_menu() - hide/show menu
 * Created: 2002/10/28, Perry Rapp
 *================================================================*/
void
dynmenu_toggle_menu (DYNMENU dynmenu)
{
	dynmenu->hidden = !dynmenu->hidden;
	dynmenu->dirty = 1;
}
