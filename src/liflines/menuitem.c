/* 
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
/*=============================================================
 * menuitem.c -- data for menu layout code
 *   Created: 1999/03 by Perry Rapp
 *   Brought into repository: 2001/01/28 by Perry Rapp
 *==============================================================*/

#include "standard.h"
#include "screen.h"
#include "menuitem.h"


/*********************************************
 * local variables
 *********************************************/

ScreenInfo f_ScreenInfo[MAX_SCREEN];


MenuItem g_MenuItemOther = { "?  Other menu choices", "?", CMD_MORE };
MenuItem g_MenuItemQuit = { "q  Return to main menu", "q", CMD_QUIT };
MenuItem g_MenuItemOptions = { "$  View options", "$", CMD_OPTIONS };
static MenuItem f_MenuItemEdit = { "e  Edit the person", "e", CMD_EDIT };
static MenuItem f_MenuItemEditTop = { "e  Edit top person", "e", CMD_EDIT };
static MenuItem f_MenuItemPerson = { "i  Browse to person", "i", CMD_PERSON };
static MenuItem f_MenuItemFather = { "f  Browse to father(s)", "f", CMD_FATHER };
static MenuItem f_MenuItemFatherTop = { "f  Browse top father", "f", CMD_FATHER };
static MenuItem f_MenuItemMother = { "m  Browse to mother(s)", "m", CMD_MOTHER };
static MenuItem f_MenuItemMotherTop = { "m  Browse top mother", "m", CMD_MOTHER };
static MenuItem f_MenuItemSpouse = { "s  Browse to spouse/s", "s", CMD_SPOUSE };
static MenuItem f_MenuItemSpouseTop = { "s  Browse top spouse/s", "s", CMD_SPOUSE };
static MenuItem f_MenuItemChildren = { "c  Browse to children", "c", CMD_CHILDREN };
static MenuItem f_MenuItemChildrenTop = { "c  Browse top children", "c", CMD_CHILDREN };
static MenuItem f_MenuItemOlderSib = { "o  Browse to older sib", "o", CMD_UPSIB };
static MenuItem f_MenuItemYoungerSib = { "y  Browse to younger sib", "y", CMD_DOWNSIB };
static MenuItem f_MenuItemFamily = { "g  Browse to family", "g", CMD_FAMILY };
static MenuItem f_MenuItemFamiliesBoth = { "G  Browse both families", "G", CMD_2FAM };
static MenuItem f_MenuItemParents = { "u  Browse to parents", "u", CMD_PARENTS };
static MenuItem f_MenuItemParentsBoth = { "U  Browse both parents", "U", CMD_2PAR };
static MenuItem f_MenuItemBrowse = { "b  Browse to persons", "b", CMD_BROWSE };
static MenuItem f_MenuItemBrowseTop = { "t  Browse to top", "t", CMD_TOP };
static MenuItem f_MenuItemBrowseBottom = { "b  Browse to bottom", "b", CMD_BOTTOM };
static MenuItem f_MenuItemAddAsSpouse = { "h  Add as spouse", "h", CMD_ADDASSPOUSE };
static MenuItem f_MenuItemAddAsChild = { "i  Add as child", "i", CMD_ADDASCHILD };
static MenuItem f_MenuItemAddSpouse = { "s  Add spouse to family", "s", CMD_ADDSPOUSE };
static MenuItem f_MenuItemAddChild = { "a  Add child to family", "a", CMD_ADDCHILD };
static MenuItem f_MenuItemAddFamily = { "a  Add family", "a", CMD_ADDFAMILY };
static MenuItem f_MenuItemPedigree = { "p  Show pedigree", "p", CMD_PEDIGREE };
static MenuItem f_MenuItemSwapFamilies = { "x  Swap two families", "x", CMD_SWAPFAMILIES };
static MenuItem f_MenuItemSwapChildren = { "x  Swap two children", "x", CMD_SWAPCHILDREN };
static MenuItem f_MenuItemSwitchTopBottom = { "x  Switch top/bottom", "x", CMD_SWAPTOPBOTTOM };
static MenuItem f_MenuItemNewPerson = { "n  Create new person", "n", CMD_NEWPERSON };
static MenuItem f_MenuItemNewFamily = { "a  Create new family", "a", CMD_NEWFAMILY };
static MenuItem f_MenuItemTandem = { "t  Enter tandem mode", "t", CMD_TANDEM };
static MenuItem f_MenuItemZipBrowse = { "z  Browse to person", "z", CMD_BROWSE };
static MenuItem f_MenuItemRemoveAsSpouse = { "r  Remove as spouse", "r", CMD_REMOVEASSPOUSE };
static MenuItem f_MenuItemRemoveAsChild = { "d  Remove as child", "d", CMD_REMOVEASCHILD };
static MenuItem f_MenuItemRemoveSpouseFrom = { "r  Remove spouse from", "r", CMD_REMOVESPOUSE };
static MenuItem f_MenuItemRemoveChildFrom = { "d  Remove child from", "d" , CMD_REMOVECHILD };
static MenuItem f_MenuItemScrollUp = { "(  Scroll up", "(" };
static MenuItem f_MenuItemScrollDown = { ")  Scroll down", ")" };
static MenuItem f_MenuItemScrollUpTop = { "[  Scroll top up", "[" };
static MenuItem f_MenuItemScrollDownTop = { "]  Scroll top down", "]" };
static MenuItem f_MenuItemScrollUpBottom = { "(  Scroll bottom up", "(" };
static MenuItem f_MenuItemScrollDownBottom = { ")  Scroll bottom down", ")" };
static MenuItem f_MenuItemScrollUpBoth = { "{  Scroll both up", "{" };
static MenuItem f_MenuItemScrollDownBoth = { "}  Scroll both down", "}" };
static MenuItem f_MenuItemToggleChildNos = { "#  Toggle childnos", "#" };
static MenuItem f_MenuItemToggleGedcomView = { "!  Toggle GEDCOM view", "!" };
static MenuItem f_MenuItemTogglePedigreeType = { "&  Toggle pedigree type", "&" };
static MenuItem f_MenuItemDigits = { "(1-9)  Browse to child", "123456789" };
MenuItem f_MenuItemSyncMoves = { "y  Turn on sync", "y" };
static MenuItem f_MenuItemAdvanced = { "A  Advanced view", "A" };
static MenuItem f_MenuItemTandemChildren = { "C  Tandem to children", "C" };
static MenuItem f_MenuItemTandemFathers = { "F  Tandem to father/s", "F" };
static MenuItem f_MenuItemTandemFamilies = { "G  Tandem to family/s", "G" };
static MenuItem f_MenuItemBothFathers = { "f  Browse to fathers", "f" };
static MenuItem f_MenuItemBothMothers = { "m  Browse to mothers", "m" };
static MenuItem f_MenuItemTandemMothers = { "M  Tandem to mother/s", "M" };
static MenuItem f_MenuItemTandemSpouses = { "S  Tandem to spouse/s", "S" };
static MenuItem f_MenuItemTandemParents = { "U  Tandem to parents", "U" };
static MenuItem f_MenuItemEnlargeMenu = { "<  Enlarge menu area", "<" };
static MenuItem f_MenuItemShrinkMenu = { ">  Shrink menu area", ">" };
static MenuItem f_MenuItemEnlargePedigree = { "+  Enlarge pedigree", "+" };
static MenuItem f_MenuItemShrinkPedigree = { "-  Shrink pedigree", "-" };
static MenuItem f_MenuItemCopyTopToBottom = { "d  Copy top to bottom", "d" };
static MenuItem f_MenuItemMergeBottomToTop = { "j  Merge bottom to top", "j" };
static MenuItem f_MenuItemMoveDownList = { "j  Move down list", "j" };
static MenuItem f_MenuItemMoveUpList = { "k  Move up list", "k" };
static MenuItem f_MenuItemEditThis = { "e  Edit this person", "e" };
static MenuItem f_MenuItemBrowseThis = { "i  Browse this person", "i" };
static MenuItem f_MenuItemMarkThis = { "m  Mark this person", "m" };
static MenuItem f_MenuItemDeleteFromList = { "d  Delete from list", "d" };
static MenuItem f_MenuItemNameList = { "n  Name this list", "n" };
static MenuItem f_MenuItemBrowseNewPersons = { "b  Browse new persons", "b" };
static MenuItem f_MenuItemAddToList = { "a  Add to this list", "a" };
static MenuItem f_MenuItemSwapMarkCurrent = { "x  Swap mark/current", "x" };
static MenuItem f_MenuItemSources = { "$  List sources", "$", CMD_SOURCES };
static MenuItemOption f_MenuItemOptionSources =
	{ "$s  show sources", "$s  hide sources", "$S", CMD_SHOWSOURCES };


static MenuItem * f_MenuPerson[] =
{
	&f_MenuItemEdit,
	&f_MenuItemFather,
	&f_MenuItemMother,
	&f_MenuItemSpouse,
	&f_MenuItemChildren,
	&f_MenuItemOlderSib,
	&f_MenuItemYoungerSib,
	&f_MenuItemFamily,
	&f_MenuItemParents,
	&f_MenuItemBrowse,
	&f_MenuItemAddAsSpouse,
	&f_MenuItemAddAsChild,
	&f_MenuItemRemoveAsSpouse,
	&f_MenuItemRemoveAsChild,
	&f_MenuItemPedigree,
	&f_MenuItemNewPerson,
	&f_MenuItemNewFamily,
	&f_MenuItemSwapFamilies,
	&f_MenuItemTandem,
	&f_MenuItemZipBrowse,
	&f_MenuItemScrollUp,
	&f_MenuItemScrollDown,
	&f_MenuItemToggleChildNos,
	&f_MenuItemDigits,
	&f_MenuItemToggleGedcomView,
	&f_MenuItemAdvanced,
	&f_MenuItemTandemChildren,
	&f_MenuItemTandemFathers,
	&f_MenuItemTandemFamilies,
	&f_MenuItemTandemMothers,
	&f_MenuItemTandemSpouses,
	&f_MenuItemTandemParents,
	&f_MenuItemEnlargeMenu,
	&f_MenuItemShrinkMenu,
	&f_MenuItemSources,
	0
};
static MenuItemOption * f_MenuPersonOptions[] =
{
	&f_MenuItemOptionSources,
	0
};
static MenuItem * f_MenuFamily[] =
{
	&f_MenuItemEdit,
	&f_MenuItemFather,
	&f_MenuItemMother,
	&f_MenuItemChildren,
	&f_MenuItemBrowse,
	&f_MenuItemAddSpouse,
	&f_MenuItemAddChild,
	&f_MenuItemSwapFamilies,
	&f_MenuItemNewPerson,
	&f_MenuItemNewFamily,
	&f_MenuItemTandem,
	&f_MenuItemZipBrowse,
	&f_MenuItemRemoveSpouseFrom,
	&f_MenuItemRemoveChildFrom,
	&f_MenuItemScrollUp,
	&f_MenuItemScrollDown,
	&f_MenuItemToggleChildNos,
	&f_MenuItemDigits,
	&f_MenuItemAdvanced,
	&f_MenuItemTandemChildren,
	&f_MenuItemTandemFathers,
	&f_MenuItemTandemMothers,
	&f_MenuItemEnlargeMenu,
	&f_MenuItemShrinkMenu,
	0
};

static MenuItem * f_MenuPedigree[] =
{
	&f_MenuItemEdit,
	&f_MenuItemPerson,
	&f_MenuItemFather,
	&f_MenuItemMother,
	&f_MenuItemSpouse,
	&f_MenuItemChildren,
	&f_MenuItemOlderSib,
	&f_MenuItemYoungerSib,
	&f_MenuItemDigits,
	&f_MenuItemFamily,
	&f_MenuItemBrowse,
	&f_MenuItemEnlargePedigree,
	&f_MenuItemShrinkPedigree,
	&f_MenuItemTogglePedigreeType,
	&f_MenuItemEnlargeMenu,
	&f_MenuItemShrinkMenu,
	0
};

static MenuItem * f_Menu2Person[] =
{
	&f_MenuItemEditTop,
	&f_MenuItemBrowseTop,
	&f_MenuItemFatherTop,
	&f_MenuItemMotherTop,
	&f_MenuItemSpouseTop,
	&f_MenuItemChildrenTop,
	&f_MenuItemParentsBoth,
	&f_MenuItemFamiliesBoth,
	&f_MenuItemBrowse,
	&f_MenuItemScrollUpTop,
	&f_MenuItemScrollDownTop,
	&f_MenuItemScrollUpBottom,
	&f_MenuItemScrollDownBottom,
	&f_MenuItemScrollUpBoth,
	&f_MenuItemScrollDownBoth,
	&f_MenuItemToggleChildNos,
	&f_MenuItemDigits,
	&f_MenuItemSyncMoves,
	&f_MenuItemCopyTopToBottom,
	&f_MenuItemAddFamily,
	&f_MenuItemMergeBottomToTop,
	&f_MenuItemSwitchTopBottom,
	&f_MenuItemEnlargeMenu,
	&f_MenuItemShrinkMenu,
	0
};

static MenuItem * f_Menu2Family[] =
{
	&f_MenuItemEditTop,
	&f_MenuItemBrowseTop,
	&f_MenuItemBrowseBottom,
	&f_MenuItemBothFathers,
	&f_MenuItemBothMothers,
	&f_MenuItemScrollUpTop,
	&f_MenuItemScrollDownTop,
	&f_MenuItemScrollUpBottom,
	&f_MenuItemScrollDownBottom,
	&f_MenuItemScrollUpBoth,
	&f_MenuItemScrollDownBoth,
	&f_MenuItemToggleChildNos,
	&f_MenuItemDigits,
	&f_MenuItemSwitchTopBottom,
	&f_MenuItemMergeBottomToTop,
	&f_MenuItemEnlargeMenu,
	&f_MenuItemShrinkMenu,
	0
};

static MenuItem * f_MenuListPersons[] =
{
	&f_MenuItemMoveDownList,
	&f_MenuItemMoveUpList,
	&f_MenuItemEditThis,
	&f_MenuItemBrowseThis,
	&f_MenuItemMarkThis,
	&f_MenuItemDeleteFromList,
	&f_MenuItemTandem,
	&f_MenuItemNameList,
	&f_MenuItemBrowseNewPersons,
	&f_MenuItemAddToList,
	&f_MenuItemSwapMarkCurrent,
	&f_MenuItemEnlargeMenu,
	&f_MenuItemShrinkMenu,
	0
};

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*============================
 * setup_menu - initialize runtime memory
 *  structures for one menu
 * Created: 1999/03, Perry Rapp
 * Joined repository: 2001/01/28, Perry Rapp
 *==========================*/
static void
setup_menu (INT screen, STRING Title, INT MenuRows, INT MenuCols
           , INT Size, MenuItem ** Menu, MenuItemOption ** MenuOptions)
{
	INT i;
	STRING MenuString;
	INT totlen, itlen;
	STRING defaults = "q?";
	f_ScreenInfo[screen].Title = Title;
	f_ScreenInfo[screen].MenuRows = MenuRows;
	f_ScreenInfo[screen].MenuCols = MenuCols;
	f_ScreenInfo[screen].MenuSize = Size;
	f_ScreenInfo[screen].Menu = Menu;
	MenuString = f_ScreenInfo[screen].Commands;
	totlen = 0;
	itlen = strlen(defaults);
	strcpy(MenuString, defaults);
	totlen += itlen;
	MenuString += itlen;
	for (i=0; i<Size; i++) {
		itlen = strlen(Menu[i]->Choices);
		if (itlen + totlen >= sizeof(f_ScreenInfo[screen].Commands))
			break;
		strcpy(MenuString, Menu[i]->Choices);
		totlen += itlen;
		MenuString += itlen;
	}
	f_ScreenInfo[screen].ExtCommands[0] = '\0';
	f_ScreenInfo[screen].OptionCommands[0] = '\0';
	if (MenuOptions) {
		strcpy(MenuString, "$");
	}
}
/*============================
 * menuitem_initialize - set up menu arrays
 * Created: 1999/03, Perry Rapp
 * Joined repository: 2001/01/28, Perry Rapp
 *==========================*/
void
menuitem_initialize (void)
{
	INT i;
	INT scr;
	STRING Title;
	INT MenuRows, MenuCols, MenuSize;
	MenuItem ** Menu;
	MenuItemOption ** MenuOpts;
	INT ItemSize;

	ItemSize = sizeof(f_MenuPerson[0]);
	for (i=1; i<MAX_SCREEN; i++)
	{
		f_ScreenInfo[i].MenuPage = 0;
		f_ScreenInfo[i].MenuCols = 3;
	}

	scr = MAIN_SCREEN;
	f_ScreenInfo[scr].Title = (STRING)"LifeLines -- Main Menu";
	strcpy(f_ScreenInfo[MAIN_SCREEN].Commands, "badrtuxq");

	scr = ONE_PER_SCREEN;
	Title = (STRING)"LifeLines -- Person Browse Screen (* toggles menu)";
	MenuRows = 8;
	MenuCols = 3;
	MenuSize = sizeof(f_MenuPerson)/ItemSize-1;
	Menu = f_MenuPerson;
	MenuOpts = f_MenuPersonOptions;
	setup_menu(scr, Title, MenuRows, MenuCols, MenuSize, Menu, MenuOpts);

	MenuOpts = 0;

	scr = ONE_FAM_SCREEN;
	Title = (STRING)"LifeLines -- Family Browse Screen (* toggles menu)";
	MenuRows = 6;
	MenuCols = 3;
	MenuSize = sizeof(f_MenuFamily)/ItemSize-1;
        Menu = f_MenuFamily;
	setup_menu(scr, Title, MenuRows, MenuCols, MenuSize, Menu, MenuOpts);

	scr = TWO_PER_SCREEN;
	Title = (STRING)"LifeLines -- Tandem Browse Screen (* toggles menu)";
	MenuRows = 5;
	MenuCols = 3;
	MenuSize = sizeof(f_Menu2Person)/ItemSize-1;
	Menu = f_Menu2Person;
	setup_menu(scr, Title, MenuRows, MenuCols, MenuSize, Menu, MenuOpts);

	scr = TWO_FAM_SCREEN;
	Title = (STRING)"LifeLines -- Two Family Browse Screen (* toggles menu)";
	MenuRows = 5;
	MenuCols = 3;
	MenuSize = sizeof(f_Menu2Family)/ItemSize-1;
	Menu = f_Menu2Family;
	setup_menu(scr, Title, MenuRows, MenuCols, MenuSize, Menu, MenuOpts);

	scr = PED_SCREEN;
	Title = (STRING)"LifeLines -- Pedigree Browse Screen (* toggles menu)";
	MenuRows = 5;
	MenuCols = 3;
	MenuSize = sizeof(f_MenuPedigree)/ItemSize-1;
        Menu = f_MenuPedigree;
	setup_menu(scr, Title, MenuRows, MenuCols, MenuSize, Menu, MenuOpts);

	scr = LIST_SCREEN;
	Title = (STRING)"LifeLines -- List Browse Screen (* toggles menu)";
	MenuRows = 13;
	MenuCols = 1;
	MenuSize = sizeof(f_MenuListPersons)/ItemSize-1;
	Menu = f_MenuListPersons;
	setup_menu(scr, Title, MenuRows, MenuCols, MenuSize, Menu, MenuOpts);

	f_ScreenInfo[AUX_SCREEN].Title = (STRING)"LifeLines -- Auxilliary Browse Screen";
	strcpy(f_ScreenInfo[AUX_SCREEN].Commands, "eq");

	for (i=1; i<MAX_SCREEN; i++)
		f_ScreenInfo[i].MenuPage = 0;
}
