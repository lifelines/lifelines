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

#include "llstdlib.h"
#include "feedback.h"
#include "menuitem.h"

/*********************************************
 * external/imported variables
 *********************************************/

/*********************************************
 * local types
 *********************************************/

struct CmdItem_s {
	char c;
	BOOLEAN direct; /* (T: command value, F: pointer) */
	UNION value; /* command value, or pointer to CommandArray */
};

struct CmdArray_s {
	INT alloc; /* size allocated */
	INT used; /* size in use */
	CMDITEM array;
};

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void add_menu_item(CMDARRAY cmds, MenuItem * mitem);
static CMDARRAY create_cmd_array(INT alloc);
static void copy_cmditem(CMDITEM dest, CMDITEM src);
static BOOLEAN find_cmd(CMDARRAY cmds, char c, INT * pos);
static void free_cmds(CMDARRAY cmds);
static void get_menu_choice(STRING display, STRING choice, INT max);
static void grow_cmd_array(CMDARRAY cmds);
static void insert_cmd(CMDARRAY cmds, STRING str, INT cmdnum
	, STRING display);
static INT menuitem_find_cmd(CMDARRAY cmds, STRING cmd);
static void setup_menu(ScreenInfo * sinfo, STRING Title, INT MenuRows
	, INT MenuCols ,INT Size, MenuItem ** Menu);

/*********************************************
 * local variables
 *********************************************/

ScreenInfo g_ScreenInfo[MAX_SCREEN+1];

/* These are not listed as part of the menus below, because these are
added on-the-fly to every menu page displayed */
MenuItem g_MenuItemOther = { "?  Other menu choices", 0, CMD_MENU_MORE };
MenuItem g_MenuItemQuit = { "q  Return to main menu", 0, CMD_QUIT };

/* normal menu items */
static MenuItem f_MenuItemEditIndi = { N_("e  Edit the person"), 0, CMD_EDIT };
static MenuItem f_MenuItemEditFamily = { N_("e  Edit the family"), 0, CMD_EDIT };
static MenuItem f_MenuItemEdit = { N_("e  Edit record"), 0, CMD_EDIT };
static MenuItem f_MenuItemEditTop = { N_("e  Edit top person"), 0, CMD_EDIT };
static MenuItem f_MenuItemFather = { N_("f  Browse to father"), 0, CMD_FATHER };
static MenuItem f_MenuItemFatherTop = { N_("f  Browse top father"), 0, CMD_FATHER };
static MenuItem f_MenuItemMother = { N_("m  Browse to mother"), 0, CMD_MOTHER };
static MenuItem f_MenuItemMotherTop = { N_("m  Browse top mother"), 0, CMD_MOTHER };
static MenuItem f_MenuItemSpouse = { N_("s  Browse to spouse/s"), 0, CMD_SPOUSE };
static MenuItem f_MenuItemSpouseTop = { N_("s  Browse top spouse/s"), 0, CMD_SPOUSE };
static MenuItem f_MenuItemChildren = { N_("c  Browse to children"), 0, CMD_CHILDREN };
static MenuItem f_MenuItemChildrenTop = { N_("c  Browse top children"), 0, CMD_CHILDREN };
static MenuItem f_MenuItemOlderSib = { N_("o  Browse to older sib"), 0, CMD_UPSIB };
static MenuItem f_MenuItemYoungerSib = { N_("y  Browse to younger sib"), 0, CMD_DOWNSIB };
static MenuItem f_MenuItemFamily = { N_("g  Browse to family"), 0, CMD_FAMILY };
static MenuItem f_MenuItemParents = { N_("u  Browse to parents"), 0, CMD_PARENTS };
static MenuItem f_MenuItemBrowse = { N_("b  Browse to persons"), 0, CMD_BROWSE };
static MenuItem f_MenuItemBrowseTop = { N_("t  Browse to top"), 0, CMD_TOP };
static MenuItem f_MenuItemBrowseBottom = { N_("b  Browse to bottom"), 0, CMD_BOTTOM };
static MenuItem f_MenuItemAddAsSpouse = { N_("h  Add as spouse"), 0, CMD_ADDASSPOUSE };
static MenuItem f_MenuItemAddAsChild = { N_("i  Add as child"), 0, CMD_ADDASCHILD };
static MenuItem f_MenuItemAddSpouse = { N_("s  Add spouse to family"), 0, CMD_ADDSPOUSE };
static MenuItem f_MenuItemAddChild = { N_("a  Add child to family"), 0, CMD_ADDCHILD };
static MenuItem f_MenuItemAddFamily = { N_("a  Add family"), 0, CMD_ADDFAMILY };
static MenuItem f_MenuItemSwapFamilies = { N_("x  Swap two families"), 0, CMD_SWAPFAMILIES };
static MenuItem f_MenuItemSwapChildren = { N_("x  Swap two children"), 0, CMD_SWAPCHILDREN };
static MenuItem f_MenuItemReorderChild = { N_("%c  Reorder child"), 0, CMD_REORDERCHILD };
static MenuItem f_MenuItemSwitchTopBottom = { N_("x  Switch top/bottom"), 0, CMD_SWAPTOPBOTTOM };
static MenuItem f_MenuItemNewPerson = { N_("n  Create new person"), 0, CMD_NEWPERSON };
static MenuItem f_MenuItemNewFamily = { N_("a  Create new family"), 0, CMD_NEWFAMILY };
static MenuItem f_MenuItemTandem = { N_("tt Enter tandem mode"), 0, CMD_TANDEM };
static MenuItem f_MenuItemTandemFamily = { N_("tt Enter family tandem"), 0, CMD_TANDEM };
static MenuItem f_MenuItemZipIndi = { N_("zi Browse to indi"), 0, CMD_BROWSE_ZIP_INDI };
static MenuItem f_MenuItemZipBrowse = { N_("zz Browse to any"), 0, CMD_BROWSE_ZIP_ANY };
static MenuItem f_MenuItemRemoveAsSpouse = { N_("r  Remove as spouse"), 0, CMD_REMOVEASSPOUSE };
static MenuItem f_MenuItemRemoveAsChild = { N_("d  Remove as child"), 0, CMD_REMOVEASCHILD };
static MenuItem f_MenuItemRemoveSpouseFrom = { N_("r  Remove spouse from"), 0, CMD_REMOVESPOUSE };
static MenuItem f_MenuItemRemoveChildFrom = { N_("d  Remove child from"), 0 , CMD_REMOVECHILD };
static MenuItem f_MenuItemScrollUp = { N_("(  Scroll up"), 0, CMD_SCROLL_UP };
static MenuItem f_MenuItemScrollDown = { N_(")  Scroll down"), 0, CMD_SCROLL_DOWN };
static MenuItem f_MenuItemDepthUp = { N_("]  Increase tree depth"), 0, CMD_DEPTH_UP };
static MenuItem f_MenuItemDepthDown = { N_("[  Decrease tree depth"), 0, CMD_DEPTH_DOWN };
static MenuItem f_MenuItemScrollUpTop = { N_("(t Scroll top up"), 0, CMD_SCROLL_TOP_UP };
static MenuItem f_MenuItemScrollDownTop = { N_(")t Scroll top down"), 0, CMD_SCROLL_TOP_DOWN };
static MenuItem f_MenuItemScrollUpBottom = { N_("(b Scroll bottom up"), 0, CMD_SCROLL_BOTTOM_UP };
static MenuItem f_MenuItemScrollDownBottom = { N_(")b Scroll bottom down"), 0, CMD_SCROLL_BOTTOM_DOWN };
static MenuItem f_MenuItemScrollUpBoth = { N_("(( Scroll both up"), 0, CMD_SCROLL_BOTH_UP };
static MenuItem f_MenuItemScrollDownBoth = { N_(")) Scroll both down"), 0, CMD_SCROLL_BOTH_DOWN };
static MenuItem f_MenuItemToggleChildNos = { N_("#  Toggle childnos"), 0, CMD_TOGGLE_CHILDNUMS };
static MenuItem f_MenuItemModeGedcom = { N_("!g GEDCOM mode"), 0, CMD_MODE_GEDCOM };
static MenuItem f_MenuItemModeGedcomX = { N_("!x GEDCOMX mode"), 0, CMD_MODE_GEDCOMX };
static MenuItem f_MenuItemModeGedcomT = { N_("!t GEDCOMT mode"), 0, CMD_MODE_GEDCOMT };
static MenuItem f_MenuItemModeAncestors = { N_("!a Ancestors mode"), 0, CMD_MODE_ANCESTORS };
static MenuItem f_MenuItemModeDescendants = { N_("!d Descendants mode"), 0, CMD_MODE_DESCENDANTS };
static MenuItem f_MenuItemModeNormal = { N_("!n Normal mode"), 0, CMD_MODE_NORMAL };
static MenuItem f_MenuItemModePedigree = { N_("p  Pedigree mode"), 0, CMD_MODE_PEDIGREE };
static MenuItem f_MenuItemModeCycle = { N_("!! Cycle mode"), 0, CMD_MODE_CYCLE };
/* Note - CMD_CHILD_DIRECT0 has special handling, & is always wired to 123456789 */
static MenuItem f_MenuItemDigits = { N_("(1-9)  Browse to child"), 0, CMD_CHILD_DIRECT0 };
MenuItem f_MenuItemSyncMoves = { N_("y  Turn on sync"), 0, CMD_NONE };
static MenuItem f_MenuItemAdvanced = { N_("A  Advanced view"), 0, CMD_ADVANCED };
static MenuItem f_MenuItemTandemChildren = { N_("tc Tandem to children"), 0, CMD_TANDEM_CHILDREN };
static MenuItem f_MenuItemTandemFathers = { N_("tf Tandem to father/s"), 0, CMD_TANDEM_FATHERS };
static MenuItem f_MenuItemTandemFamilies = { N_("tg Tandem to family/s"), 0, CMD_TANDEM_FAMILIES };
static MenuItem f_MenuItemBothFathers = { N_("f  Browse to fathers"), 0, CMD_BOTH_FATHERS };
static MenuItem f_MenuItemBothMothers = { N_("m  Browse to mothers"), 0, CMD_BOTH_MOTHERS };
static MenuItem f_MenuItemTandemMothers = { N_("tm Tandem to mother/s"), 0, CMD_TANDEM_MOTHERS };
static MenuItem f_MenuItemTandemSpouses = { N_("ts Tandem to spouse/s"), 0, CMD_TANDEM_SPOUSES };
static MenuItem f_MenuItemTandemParents = { N_("tu Tandem to parents"), 0, CMD_TANDEM_PARENTS };
static MenuItem f_MenuItemEnlargeMenu = { N_("<  Enlarge menu area"), 0, CMD_MENU_GROW };
static MenuItem f_MenuItemShrinkMenu = { N_(">  Shrink menu area"), 0, CMD_MENU_SHRINK };
static MenuItem f_MenuItemMoreCols = { N_("M> More menu cols"), 0, CMD_MENU_MORECOLS };
static MenuItem f_MenuItemLessCols = { N_("M< Less menu cols"), 0, CMD_MENU_LESSCOLS };
static MenuItem f_MenuItemNext = { N_("+  Next in db"), 0, CMD_NEXT };
static MenuItem f_MenuItemPrev = { N_("-  Prev in db"), 0, CMD_PREV };
static MenuItem f_MenuItemCopyTopToBottom = { N_("d  Copy top to bottom"), 0, CMD_COPY_TOP_TO_BOTTOM };
static MenuItem f_MenuItemMergeBottomToTop = { N_("j  Merge bottom to top"), 0, CMD_MERGE_BOTTOM_TO_TOP};
static MenuItem f_MenuItemMoveDownList = { N_("j  Move down list"), 0, CMD_NONE };
static MenuItem f_MenuItemMoveUpList = { N_("k  Move up list"), 0, CMD_NONE };
static MenuItem f_MenuItemEditThis = { N_("e  Edit this person"), 0, CMD_NONE };
static MenuItem f_MenuItemBrowseThis = { N_("i  Browse this person"), 0, CMD_BROWSE_INDI };
static MenuItem f_MenuItemMarkThis = { N_("m  Mark this person"), 0, CMD_NONE };
static MenuItem f_MenuItemDeleteFromList = { N_("d  Delete from list"), 0, CMD_NONE };
static MenuItem f_MenuItemNameList = { N_("n  Name this list"), 0, CMD_NONE };
static MenuItem f_MenuItemBrowseNewPersons = { N_("b  Browse new persons"), 0, CMD_NONE };
static MenuItem f_MenuItemAddToList = { N_("a  Add to this list"), 0, CMD_NONE };
static MenuItem f_MenuItemSwapMarkCurrent = { N_("x  Swap mark/current"), 0, CMD_NONE };
static MenuItem f_MenuItemSources = { N_("$s  List sources"), 0, CMD_SOURCES };
static MenuItem f_MenuItemNotes = { N_("$n  List notes"), 0, CMD_NOTES };
static MenuItem f_MenuItemPointers = { N_("$$  List references"), 0, CMD_POINTERS };
static MenuItem f_MenuItemHistoryBack = { N_("^b  History/back"), 0, CMD_HISTORY_BACK };
static MenuItem f_MenuItemHistoryFwd = { N_("^f  History/fwd"), 0, CMD_HISTORY_FWD };
static MenuItem f_MenuItemHistoryList = { N_("^l  History list"), 0, CMD_HISTORY_LIST };
static MenuItem f_MenuItemHistoryClean = { N_("^c Clear history"), 0, CMD_HISTORY_CLEAR };
static MenuItem f_MenuItemAddSour = { N_("%s  Add source"), 0, CMD_ADD_SOUR };
static MenuItem f_MenuItemAddEven = { N_("%e  Add event"), 0, CMD_ADD_EVEN };
static MenuItem f_MenuItemAddOthr = { N_("%o  Add other"), 0, CMD_ADD_OTHR };


static MenuItem f_MenuItemBrowseFamily = { N_("B  Browse new family"), 0, CMD_BROWSE_FAM };


/* Actual menus initializations
 NB: All menus include g_MenuItemOther & g_MenuItemQuit, and they are not listed
*/

static MenuItem * f_MenuPerson[] =
{
	&f_MenuItemEditIndi,
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
	&f_MenuItemNewPerson,
	&f_MenuItemNewFamily,
	&f_MenuItemAddSour,
	&f_MenuItemAddEven,
	&f_MenuItemAddOthr,
	&f_MenuItemSwapFamilies,
	&f_MenuItemTandem,
	&f_MenuItemZipBrowse,
	&f_MenuItemZipIndi,
	&f_MenuItemScrollUp,
	&f_MenuItemScrollDown,
	&f_MenuItemToggleChildNos,
	&f_MenuItemDigits,
	&f_MenuItemModeGedcom,
	&f_MenuItemModeGedcomX,
	&f_MenuItemModeGedcomT,
	&f_MenuItemModeNormal,
	&f_MenuItemModePedigree,
	&f_MenuItemModeAncestors,
	&f_MenuItemModeDescendants,
	&f_MenuItemModeCycle,
	&f_MenuItemAdvanced,
	&f_MenuItemTandemChildren,
	&f_MenuItemTandemFathers,
	&f_MenuItemTandemFamilies,
	&f_MenuItemTandemMothers,
	&f_MenuItemTandemSpouses,
	&f_MenuItemTandemParents,
	&f_MenuItemDepthUp,
	&f_MenuItemDepthDown,
	&f_MenuItemEnlargeMenu,
	&f_MenuItemShrinkMenu,
	&f_MenuItemMoreCols,
	&f_MenuItemLessCols,
	&f_MenuItemSources,
	&f_MenuItemNotes,
	&f_MenuItemPointers,
	&f_MenuItemNext,
	&f_MenuItemPrev,
	&f_MenuItemHistoryBack,
	&f_MenuItemHistoryFwd,
	&f_MenuItemHistoryList,
	&f_MenuItemHistoryClean,
	0
};

static MenuItem * f_MenuFamily[] =
{
	&f_MenuItemEditFamily,
	&f_MenuItemFather,
	&f_MenuItemMother,
	&f_MenuItemChildren,
	&f_MenuItemNewPerson,
	&f_MenuItemAddSour,
	&f_MenuItemAddEven,
	&f_MenuItemAddOthr,
	&f_MenuItemAddSpouse,
	&f_MenuItemAddChild,
	&f_MenuItemRemoveSpouseFrom,
	&f_MenuItemRemoveChildFrom,
	&f_MenuItemSwapChildren,
	&f_MenuItemReorderChild,
	&f_MenuItemTandemFamily,
	&f_MenuItemBrowse,
	&f_MenuItemZipBrowse,
	&f_MenuItemZipIndi,
	&f_MenuItemBrowseFamily,
	&f_MenuItemScrollUp,
	&f_MenuItemScrollDown,
	&f_MenuItemToggleChildNos,
	&f_MenuItemDigits,
	&f_MenuItemModeGedcom,
	&f_MenuItemModeGedcomX,
	&f_MenuItemModeGedcomT,
	&f_MenuItemModeNormal,
	&f_MenuItemModeCycle,
	&f_MenuItemAdvanced,
	&f_MenuItemTandemChildren,
	&f_MenuItemTandemFathers,
	&f_MenuItemTandemMothers,
	&f_MenuItemEnlargeMenu,
	&f_MenuItemShrinkMenu,
	&f_MenuItemMoreCols,
	&f_MenuItemLessCols,
	&f_MenuItemSources,
	&f_MenuItemNotes,
	&f_MenuItemPointers,
	&f_MenuItemNext,
	&f_MenuItemPrev,
	&f_MenuItemHistoryBack,
	&f_MenuItemHistoryFwd,
	&f_MenuItemHistoryList,
	&f_MenuItemHistoryClean,
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
/*
	&f_MenuItemParentsBoth,
	&f_MenuItemFamiliesBoth,
	&f_MenuItemBrowse,
	&f_MenuItemToggleChildNos,
	&f_MenuItemDigits,
	&f_MenuItemSyncMoves,
	*/
	&f_MenuItemCopyTopToBottom,
	&f_MenuItemAddFamily,
	&f_MenuItemMergeBottomToTop,
	&f_MenuItemSwitchTopBottom,
	&f_MenuItemModeGedcom,
	&f_MenuItemModeGedcomX,
	&f_MenuItemModeGedcomT,
	&f_MenuItemModeNormal,
	&f_MenuItemModePedigree,
	&f_MenuItemModeAncestors,
	&f_MenuItemModeDescendants,
	&f_MenuItemModeCycle,
	&f_MenuItemScrollUpTop,
	&f_MenuItemScrollDownTop,
	&f_MenuItemScrollUpBottom,
	&f_MenuItemScrollDownBottom,
	&f_MenuItemScrollUpBoth,
	&f_MenuItemScrollDownBoth,
	&f_MenuItemDepthUp,
	&f_MenuItemDepthDown,
	&f_MenuItemEnlargeMenu,
	&f_MenuItemShrinkMenu,
	&f_MenuItemMoreCols,
	&f_MenuItemLessCols,
	&f_MenuItemBrowse,
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
	&f_MenuItemMergeBottomToTop,
	&f_MenuItemSwitchTopBottom,
	&f_MenuItemModeGedcom,
	&f_MenuItemModeGedcomX,
	&f_MenuItemModeGedcomT,
	&f_MenuItemModeNormal,
	&f_MenuItemModeCycle,
	&f_MenuItemEnlargeMenu,
	&f_MenuItemShrinkMenu,
	&f_MenuItemMoreCols,
	&f_MenuItemLessCols,
	0
};

static MenuItem * f_MenuAux[] =
{
	&f_MenuItemEdit,
	&f_MenuItemAddSour,
	&f_MenuItemAddEven,
	&f_MenuItemAddOthr,
	&f_MenuItemZipBrowse,
	&f_MenuItemZipIndi,
	&f_MenuItemScrollUp,
	&f_MenuItemScrollDown,
	&f_MenuItemEnlargeMenu,
	&f_MenuItemShrinkMenu,
	&f_MenuItemMoreCols,
	&f_MenuItemLessCols,
	&f_MenuItemModeGedcom,
	&f_MenuItemModeGedcomX,
	&f_MenuItemModeGedcomT,
	&f_MenuItemNotes,
	&f_MenuItemPointers,
	&f_MenuItemNext,
	&f_MenuItemPrev,
	&f_MenuItemHistoryBack,
	&f_MenuItemHistoryFwd,
	&f_MenuItemHistoryList,
	&f_MenuItemHistoryClean,
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
	&f_MenuItemMoreCols,
	&f_MenuItemLessCols,
	0
};

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*============================
 * setup_menu - initialize runtime memory
 *  structures for one menu
 * Title is strdup'd
 * Created: 2001/01/28, Perry Rapp
 *==========================*/
static void
setup_menu (ScreenInfo * sinfo, STRING Title, INT MenuRows, INT MenuCols
	, INT Size, MenuItem ** Menu)
{
	INT i;
	CMDARRAY cmds = create_cmd_array(32);

	if (sinfo->Title)
		stdfree(sinfo->Title);
	sinfo->Title = strdup(Title);
	sinfo->MenuRows = MenuRows;
	sinfo->MenuCols = MenuCols;
	sinfo->MenuSize = Size;
	/* MenuPage set by caller */
	sinfo->Menu = Menu;
	sinfo->Commands = cmds;
	for (i=0; i<Size; i++)
		add_menu_item(cmds, Menu[i]);
	add_menu_item(cmds, &g_MenuItemOther);
	add_menu_item(cmds, &g_MenuItemQuit);
}
/*============================
 * add_menu_item - add cmd for menu to cmdarray
 *  cmds:  [I/O] cmdarray (tree used for command recognition)
 *  mitem: [IN]  new menu item to add to cmds
 * Created: 2002/01/24
 *==========================*/
static void
add_menu_item (CMDARRAY cmds, MenuItem * mitem)
{
	INT i;
	char display[32];
	/* now we translate the display string, because it was initialized in static
	array at top of this file */
	llstrncpy(display, _(mitem->Display), ARRSIZE(display));
	if (mitem->LocalizedDisplay)
		strfree(&mitem->LocalizedDisplay);
	mitem->LocalizedDisplay = strdup(display);
	if (mitem->Command == CMD_CHILD_DIRECT0) {
		/* CMD_CHILD_DIRECT0 is always hooked up to digits */
		for (i=1; i<=9; i++) {
			char choice[2];
			sprintf(choice, "%d", i);
			insert_cmd(cmds, choice, CMD_CHILD_DIRECT0+i, display);
		}
	} else {
		char choice[9];
		if (mitem->Choices)
			strcpy(choice, mitem->Choices);
		else
			get_menu_choice(display, choice, sizeof(choice));
		/* add to nested menu arrays (stored by choice keys */
		insert_cmd(cmds, choice, mitem->Command, display);
	}
}
/*============================
 * get_menu_choice -- extract menu key sequence
 *  This must be first characters of display, ending with space
 * Created: 2001/12/23, Perry Rapp
 *==========================*/
/* This will work now, but it will break if we add arrows, PageUp, ... */
static void
get_menu_choice (STRING display, STRING choice, INT max)
{
	INT i;
	for (i=0; i<max && display[i] && display[i]!=' ' ; ++i) {
		choice[i] = display[i];
	}
	if (i == max) {
		msg_error(_("Menu choice sequence too long: %s"), display);
		FATAL();
	}
	if (display[i] != ' ') {
		msg_error(_("Menu item lacked choice sequence: %s"), display);
		FATAL();
	}
	choice[i]=0;
}
/*============================
 * create_cmd_array -- create an empty array of commands
 * Created: 2001/02/01, Perry Rapp
 *==========================*/
static CMDARRAY
create_cmd_array (INT alloc)
{
	CMDARRAY cmds = (CMDARRAY)stdalloc(sizeof(*cmds));
	cmds->alloc = alloc;
	cmds->used = 0;
	cmds->array = (CMDITEM)stdalloc(alloc * sizeof(cmds->array[0]));
	return cmds;
}
/*============================
 * grow_cmd_array -- grow an array of commands
 * Created: 2001/02/01, Perry Rapp
 *==========================*/
static void
grow_cmd_array (CMDARRAY cmds)
{
	INT alloc = cmds->alloc + cmds->alloc/2;
	CMDITEM old = cmds->array;
	INT i;
	cmds->alloc = alloc;
	cmds->array = (CMDITEM)stdalloc(alloc * sizeof(cmds->array[0]));
	for (i=0; i<cmds->used; i++)
		copy_cmditem(&cmds->array[i], &old[i]);
	stdfree(old);
}
/*============================
 * copy_cmditem -- copy a CmdItem_s struct
 * Created: 2001/02/01, Perry Rapp
 *==========================*/
static void
copy_cmditem (CMDITEM dest, CMDITEM src)
{
	dest->c = src->c;
	dest->direct = src->direct;
	dest->value = src->value;
}
/*============================
 * find_cmd -- search commands for command by character
 * Created: 2001/02/01, Perry Rapp
 *==========================*/
static BOOLEAN
find_cmd (CMDARRAY cmds, char c, INT * pos)
{
	INT lo=0, hi=cmds->used-1, i;
	while (lo<=hi) {
		i=(lo+hi)/2;
		if (cmds->array[i].c < c)
			lo=i+1;
		else if (cmds->array[i].c > c)
			hi=i-1;
		else {
			*pos = i;
			return TRUE;
		}
	}
	*pos = lo;
	return FALSE;
}
/*============================
 * insert_cmd -- add cmd to array (recursive)
 * Created: 2001/02/01, Perry Rapp
 *==========================*/
static void
insert_cmd (CMDARRAY cmds, STRING str, INT cmdnum, STRING display)
{
	INT len = strlen(str);
	INT pos;
	char error[128];
	char c = str[0];
	if (find_cmd(cmds, c, &pos)) {
		if (len==1) {
			msg_error(error, _("Conflicting command string: %s"), display);
			FATAL();
		} else {
			/* multicharacter new cmd */
			if (cmds->array[pos].direct) {
				msg_error(error, _("Conflicting command string: %s"), display);
				FATAL();
			} else {
				CMDARRAY subarr = (CMDARRAY)cmds->array[pos].value.w;
				insert_cmd(subarr, &str[1], cmdnum, display);
			}
		}
	} else {
		INT i;
		if (cmds->used == cmds->alloc)
			grow_cmd_array(cmds);
		/* not found */
		for (i=cmds->used; i>pos; i--)
			copy_cmditem(&cmds->array[i], &cmds->array[i-1]);
		cmds->array[pos].c = c;
		if (len==1) {
			cmds->array[pos].direct = TRUE;
			cmds->array[pos].value.i = cmdnum;
		} else {
			/* multicharacter new cmd */
			CMDARRAY newcmds = create_cmd_array(8);
			cmds->array[pos].direct = FALSE;
			cmds->array[pos].value.w = newcmds;
			insert_cmd(newcmds, &str[1], cmdnum, display);
		}
		cmds->used++;
	}
}
/*============================
 * menuitem_initialize -- set up menu arrays
 * Created: 2001/01/28, Perry Rapp
 *==========================*/
void
menuitem_initialize (INT cols)
{
	INT i;
	INT scr;
	INT MenuRows, MenuCols=cols, MenuSize;
	MenuItem ** Menu;
	INT ItemSize;
	ScreenInfo * sinfo=0;
	char title[120];

	ItemSize = sizeof(f_MenuPerson[0]);
	for (i=1; i<=MAX_SCREEN; i++)
	{
		g_ScreenInfo[i].Title = strdup(_("Missing title"));
		g_ScreenInfo[i].MenuRows = 0;
		g_ScreenInfo[i].MenuCols = cols;
		g_ScreenInfo[i].MenuSize = 0;
		g_ScreenInfo[i].Commands = NULL;
		g_ScreenInfo[i].Menu = NULL;
	}

	scr = ONE_PER_SCREEN;
	sinfo = &g_ScreenInfo[scr];
	llstrncpy(title, _("LifeLines -- Person Browse Screen (* toggles menu)")
		, ARRSIZE(title));
	MenuRows = 8;
	MenuSize = sizeof(f_MenuPerson)/ItemSize-1;
	Menu = f_MenuPerson;
	setup_menu(sinfo, title, MenuRows, MenuCols, MenuSize, Menu);

	scr = ONE_FAM_SCREEN;
	sinfo = &g_ScreenInfo[scr];
	llstrncpy(title, _("LifeLines -- Family Browse Screen (* toggles menu)")
		, ARRSIZE(title));
	MenuRows = 6;
	MenuSize = sizeof(f_MenuFamily)/ItemSize-1;
	Menu = f_MenuFamily;
	setup_menu(sinfo, title, MenuRows, MenuCols, MenuSize, Menu);

	scr = TWO_PER_SCREEN;
	sinfo = &g_ScreenInfo[scr];
	llstrncpy(title, _("LifeLines -- Two Person Browse Screen (* toggles menu)")
		, ARRSIZE(title));
	MenuRows = 5;
	MenuSize = sizeof(f_Menu2Person)/ItemSize-1;
	Menu = f_Menu2Person;
	setup_menu(sinfo, title, MenuRows, MenuCols, MenuSize, Menu);

	scr = TWO_FAM_SCREEN;
	sinfo = &g_ScreenInfo[scr];
	llstrncpy(title, _("LifeLines -- Two Family Browse Screen (* toggles menu)")
		, ARRSIZE(title));
	MenuRows = 5;
	MenuSize = sizeof(f_Menu2Family)/ItemSize-1;
	Menu = f_Menu2Family;
	setup_menu(sinfo, title, MenuRows, MenuCols, MenuSize, Menu);

	scr = AUX_SCREEN;
	sinfo = &g_ScreenInfo[scr];
	llstrncpy(title, _("LifeLines -- Auxiliary Browse Screen (* toggles menu)")
		, ARRSIZE(title));
	MenuRows = 4;
	MenuSize = sizeof(f_MenuAux)/ItemSize-1;
	Menu = f_MenuAux;
	setup_menu(sinfo, title, MenuRows, MenuCols, MenuSize, Menu);

	/* TO DO: this is not used right now */
	scr = LIST_SCREEN;
	sinfo = &g_ScreenInfo[scr];
	llstrncpy(title, _("LifeLines -- List Browse Screen (* toggles menu)")
		, ARRSIZE(title));
	MenuRows = 13;
	MenuCols = 1;
	MenuSize = sizeof(f_MenuListPersons)/ItemSize-1;
	Menu = f_MenuListPersons;
	setup_menu(sinfo, title, MenuRows, MenuCols, MenuSize, Menu);

	for (i=1; i<=MAX_SCREEN; i++)
		g_ScreenInfo[i].MenuPage = 0;
}
/*============================
 * menuitem_terminate -- free menu arrays
 * Created: 2001/02/01, Perry Rapp
 *==========================*/
void
menuitem_terminate (void)
{
	INT i;
	for (i=1; i<=MAX_SCREEN; i++) {
		if (g_ScreenInfo[i].Commands) {
			free_cmds(g_ScreenInfo[i].Commands);
			g_ScreenInfo[i].Commands=0;
		}
		if (g_ScreenInfo[i].Title)
			stdfree(g_ScreenInfo[i].Title);
	}
}
/*============================
 * free_cmds -- free menu arrays
 * Created: 2001/02/01, Perry Rapp
 *==========================*/
static void
free_cmds (CMDARRAY cmds)
{
	INT i;
	for (i=0; i<cmds->used; i++) {
		if (!cmds->array[i].direct) {
			CMDARRAY subarr = (CMDARRAY)cmds->array[i].value.w;
			free_cmds(subarr);
		}
	}
	stdfree(cmds->array);
	stdfree(cmds);
}
/*============================
 * menuitem_check_cmd -- check input string & return cmd
 * Created: 2001/02/01, Perry Rapp
 *==========================*/
INT
menuitem_check_cmd (INT screen, STRING str)
{
	CMDARRAY cmds = g_ScreenInfo[screen].Commands;
	if (*str == '*') return CMD_MENU_TOGGLE;
	return menuitem_find_cmd(cmds, str);
}
/*============================
 * menuitem_find_cmd -- search cmd array for cmd
 *  recursive
 * Created: 2001/02/01, Perry Rapp
 *==========================*/
static INT
menuitem_find_cmd (CMDARRAY cmds, STRING str)
{
	INT pos;
	if (!find_cmd(cmds, *str, &pos))
		return CMD_NONE;
	if (cmds->array[pos].direct) {
		INT cmd = cmds->array[pos].value.i;
		return cmd;
	} else {
		CMDARRAY subarr = (CMDARRAY)cmds->array[pos].value.w;
		if (!str[1])
			return CMD_PARTIAL;
		return menuitem_find_cmd(subarr, &str[1]);
	}
}
