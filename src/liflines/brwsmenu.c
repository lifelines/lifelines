/* 
   Copyright (c) 1999-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * menuitem.c -- data for menu layout code
 *   Created: 1999/03 by Perry Rapp
 *   Brought into repository: 2001/01/28 by Perry Rapp
 *==============================================================*/

#include "llstdlib.h"
#include "feedback.h"
#include "gedcom.h"
#include "menuitem.h"

/*********************************************
 * global/exported variables
 *********************************************/

/* These are not listed as part of the menus below, because these are
added on-the-fly to every menu page displayed */
MenuItem g_MenuItemOther = { N_("?  Other menu choices"), 0, CMD_MENU_MORE, 0 };
MenuItem g_MenuItemQuit = { N_("q  Return to main menu"), 0, CMD_QUIT, 0 };

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSttlindibrw, qSttlfambrw, qSttl2perbrw, qSttl2fambrw;
extern STRING qSttlauxbrw, qSttllstbrw;

/*********************************************
 * local types
 *********************************************/

struct BrowseScreenInfo {
	STRING title;
	struct tag_dynmenu dynmenu;
};

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void browsescreen_init(struct BrowseScreenInfo * sinfo , STRING title, INT MenuRows, INT MenuCols
	, INT MinCols, INT MaxCols
	, INT MinRows, INT MaxRows
	, INT MenuTop, INT MenuLeft, INT MenuWidth
	, INT MenuSize, MenuItem ** MenuItems);
static void brwsmenu_on_lang_change(VPTR uparm);
static void register_brwsmenu_lang_callbacks(BOOLEAN registering);

/*********************************************
 * local variables
 *********************************************/

struct BrowseScreenInfo f_BrowseScreenInfo[MAX_SCREEN+1]; /* init'd by brwsmenu_initialize */
static BOOLEAN f_initialized=FALSE;
static BOOLEAN f_reloading=FALSE;
static INT f_screenheight;
static INT f_screenwidth;
static INT f_cols;

/* normal menu items */
static MenuItem f_MenuItemEditIndi = { N_("e  Edit the person"), 0, CMD_EDIT, 0 };
static MenuItem f_MenuItemEditFamily = { N_("e  Edit the family"), 0, CMD_EDIT, 0 };
static MenuItem f_MenuItemEdit = { N_("e  Edit record"), 0, CMD_EDIT, 0 };
static MenuItem f_MenuItemEditTopIndi = { N_("e  Edit top person"), 0, CMD_EDIT, 0 };
static MenuItem f_MenuItemEditTopFam = { N_("e  Edit top family"), 0, CMD_EDIT, 0 };
static MenuItem f_MenuItemFather = { N_("f  Browse to father"), 0, CMD_FATHER, 0 };
static MenuItem f_MenuItemFatherTop = { N_("f  Browse top father"), 0, CMD_FATHER, 0 };
static MenuItem f_MenuItemMother = { N_("m  Browse to mother"), 0, CMD_MOTHER, 0 };
static MenuItem f_MenuItemMotherTop = { N_("m  Browse top mother"), 0, CMD_MOTHER, 0 };
static MenuItem f_MenuItemSpouse = { N_("s  Browse to spouse/s"), 0, CMD_SPOUSE, 0 };
static MenuItem f_MenuItemSpouseTop = { N_("s  Browse top spouse/s"), 0, CMD_SPOUSE, 0 };
static MenuItem f_MenuItemChildren = { N_("c  Browse to children"), 0, CMD_CHILDREN, 0 };
static MenuItem f_MenuItemChildrenTop = { N_("c  Browse top children"), 0, CMD_CHILDREN, 0 };
static MenuItem f_MenuItemOlderSib = { N_("o  Browse to older sib"), 0, CMD_UPSIB, 0 };
static MenuItem f_MenuItemYoungerSib = { N_("y  Browse to younger sib"), 0, CMD_DOWNSIB, 0 };
static MenuItem f_MenuItemFamily = { N_("g  Browse to family"), 0, CMD_FAMILY, 0 };
static MenuItem f_MenuItemParents = { N_("u  Browse to parents"), 0, CMD_PARENTS, 0 };
static MenuItem f_MenuItemBrowse = { N_("b  Browse to persons"), 0, CMD_BROWSE, 0 };
static MenuItem f_MenuItemBrowseTop = { N_("t  Browse to top"), 0, CMD_TOP, 0 };
static MenuItem f_MenuItemBrowseBottom = { N_("b  Browse to bottom"), 0, CMD_BOTTOM, 0 };
static MenuItem f_MenuItemAddAsSpouse = { N_("h  Add as spouse"), 0, CMD_ADDASSPOUSE, 0 };
static MenuItem f_MenuItemAddAsChild = { N_("i  Add as child"), 0, CMD_ADDASCHILD, 0 };
static MenuItem f_MenuItemAddSpouse = { N_("s  Add spouse to family"), 0, CMD_ADDSPOUSE, 0 };
static MenuItem f_MenuItemAddChild = { N_("a  Add child to family"), 0, CMD_ADDCHILD, 0 };
static MenuItem f_MenuItemAddFamily = { N_("a  Add family"), 0, CMD_ADDFAMILY, 0 };
static MenuItem f_MenuItemSwapFamilies = { N_("x  Swap two families"), 0, CMD_SWAPFAMILIES, 0 };
static MenuItem f_MenuItemSwapChildren = { N_("x  Swap two children"), 0, CMD_SWAPCHILDREN, 0 };
/* xgettext:no-c-format */
static MenuItem f_MenuItemReorderChild = { N_("%c  Reorder child"), 0, CMD_REORDERCHILD, 0 };
static MenuItem f_MenuItemSwitchTopBottom = { N_("x  Switch top/bottom"), 0, CMD_SWAPTOPBOTTOM, 0 };
static MenuItem f_MenuItemNewPerson = { N_("n  Create new person"), 0, CMD_NEWPERSON, 0 };
static MenuItem f_MenuItemNewFamily = { N_("a  Create new family"), 0, CMD_NEWFAMILY, 0 };
static MenuItem f_MenuItemTandem = { N_("tt Enter tandem mode"), 0, CMD_TANDEM, 0 };
static MenuItem f_MenuItemTandemFamily = { N_("tt Enter family tandem"), 0, CMD_TANDEM, 0 };
static MenuItem f_MenuItemZipIndi = { N_("zi Browse to indi"), 0, CMD_BROWSE_ZIP_INDI, 0 };
static MenuItem f_MenuItemZipBrowse = { N_("zz Browse to any"), 0, CMD_BROWSE_ZIP_ANY, 0 };
static MenuItem f_MenuItemRemoveAsSpouse = { N_("r  Remove as spouse"), 0, CMD_REMOVEASSPOUSE, 0 };
static MenuItem f_MenuItemRemoveAsChild = { N_("d  Remove as child"), 0, CMD_REMOVEASCHILD, 0 };
static MenuItem f_MenuItemRemoveSpouseFrom = { N_("r  Remove spouse from"), 0, CMD_REMOVESPOUSE, 0 };
static MenuItem f_MenuItemRemoveChildFrom = { N_("d  Remove child from"), 0 , CMD_REMOVECHILD, 0 };
static MenuItem f_MenuItemScrollUp = { N_("(  Scroll up"), 0, CMD_SCROLL_UP, 0 };
static MenuItem f_MenuItemScrollDown = { N_(")  Scroll down"), 0, CMD_SCROLL_DOWN, 0 };
static MenuItem f_MenuItemDepthUp = { N_("]  Increase tree depth"), 0, CMD_DEPTH_UP, 0 };
static MenuItem f_MenuItemDepthDown = { N_("[  Decrease tree depth"), 0, CMD_DEPTH_DOWN, 0 };
static MenuItem f_MenuItemScrollUpTop = { N_("(t Scroll top up"), 0, CMD_SCROLL_TOP_UP, 0 };
static MenuItem f_MenuItemScrollDownTop = { N_(")t Scroll top down"), 0, CMD_SCROLL_TOP_DOWN, 0 };
static MenuItem f_MenuItemScrollUpBottom = { N_("(b Scroll bottom up"), 0, CMD_SCROLL_BOTTOM_UP, 0 };
static MenuItem f_MenuItemScrollDownBottom = { N_(")b Scroll bottom down"), 0, CMD_SCROLL_BOTTOM_DOWN, 0 };
static MenuItem f_MenuItemScrollUpBoth = { N_("(( Scroll both up"), 0, CMD_SCROLL_BOTH_UP, 0 };
static MenuItem f_MenuItemScrollDownBoth = { N_(")) Scroll both down"), 0, CMD_SCROLL_BOTH_DOWN, 0 };
static MenuItem f_MenuItemToggleChildNos = { N_("#  Toggle childnos"), 0, CMD_TOGGLE_CHILDNUMS, 0 };
static MenuItem f_MenuItemModeGedcom = { N_("!g GEDCOM mode"), 0, CMD_MODE_GEDCOM, 0 };
static MenuItem f_MenuItemModeGedcomX = { N_("!x GEDCOMX mode"), 0, CMD_MODE_GEDCOMX, 0 };
static MenuItem f_MenuItemModeGedcomT = { N_("!t GEDCOMT mode"), 0, CMD_MODE_GEDCOMT, 0 };
static MenuItem f_MenuItemModeAncestors = { N_("!a Ancestors mode"), 0, CMD_MODE_ANCESTORS, 0 };
static MenuItem f_MenuItemModeDescendants = { N_("!d Descendants mode"), 0, CMD_MODE_DESCENDANTS, 0 };
static MenuItem f_MenuItemModeNormal = { N_("!n Normal mode"), 0, CMD_MODE_NORMAL, 0 };
static MenuItem f_MenuItemModePedigree = { N_("p  Pedigree mode"), 0, CMD_MODE_PEDIGREE, 0 };
static MenuItem f_MenuItemModeCycle = { N_("!! Cycle mode"), 0, CMD_MODE_CYCLE, 0 };
/* Note - CMD_CHILD_DIRECT0 has special handling, & is always wired to 123456789 */
static MenuItem f_MenuItemDigits = { N_("(1-9)  Browse to child"), 0, CMD_CHILD_DIRECT0, 0 };
MenuItem f_MenuItemSyncMoves = { N_("y  Turn on sync"), 0, CMD_NONE, 0 };
static MenuItem f_MenuItemAdvanced = { N_("A  Advanced view"), 0, CMD_ADVANCED, 0 };
static MenuItem f_MenuItemTandemChildren = { N_("tc Tandem to children"), 0, CMD_TANDEM_CHILDREN, 0 };
static MenuItem f_MenuItemTandemFathers = { N_("tf Tandem to father/s"), 0, CMD_TANDEM_FATHERS, 0 };
static MenuItem f_MenuItemTandemFamilies = { N_("tg Tandem to family/s"), 0, CMD_TANDEM_FAMILIES, 0 };
static MenuItem f_MenuItemBothFathers = { N_("f  Browse to fathers"), 0, CMD_BOTH_FATHERS, 0 };
static MenuItem f_MenuItemBothMothers = { N_("m  Browse to mothers"), 0, CMD_BOTH_MOTHERS, 0 };
static MenuItem f_MenuItemTandemMothers = { N_("tm Tandem to mother/s"), 0, CMD_TANDEM_MOTHERS, 0 };
static MenuItem f_MenuItemTandemSpouses = { N_("ts Tandem to spouse/s"), 0, CMD_TANDEM_SPOUSES, 0 };
static MenuItem f_MenuItemTandemParents = { N_("tu Tandem to parents"), 0, CMD_TANDEM_PARENTS, 0 };
static MenuItem f_MenuItemEnlargeMenu = { N_("<  Enlarge menu area"), 0, CMD_MENU_GROW, 0 };
static MenuItem f_MenuItemShrinkMenu = { N_(">  Shrink menu area"), 0, CMD_MENU_SHRINK, 0 };
static MenuItem f_MenuItemMoreCols = { N_("M> More menu cols"), 0, CMD_MENU_MORECOLS, 0 };
static MenuItem f_MenuItemLessCols = { N_("M< Less menu cols"), 0, CMD_MENU_LESSCOLS, 0 };
static MenuItem f_MenuItemNext = { N_("+  Next in db"), 0, CMD_NEXT, 0 };
static MenuItem f_MenuItemPrev = { N_("-  Prev in db"), 0, CMD_PREV, 0 };
static MenuItem f_MenuItemCopyTopToBottom = { N_("d  Copy top to bottom"), 0, CMD_COPY_TOP_TO_BOTTOM, 0 };
static MenuItem f_MenuItemMergeBottomToTop = { N_("j  Merge bottom to top"), 0, CMD_MERGE_BOTTOM_TO_TOP, 0 };
static MenuItem f_MenuItemMoveDownList = { N_("j  Move down list"), 0, CMD_NONE, 0 };
static MenuItem f_MenuItemMoveUpList = { N_("k  Move up list"), 0, CMD_NONE, 0 };
static MenuItem f_MenuItemEditThis = { N_("e  Edit this person"), 0, CMD_NONE, 0 };
static MenuItem f_MenuItemBrowseThis = { N_("i  Browse this person"), 0, CMD_BROWSE_INDI, 0 };
static MenuItem f_MenuItemMarkThis = { N_("m  Mark this person"), 0, CMD_NONE, 0 };
static MenuItem f_MenuItemDeleteFromList = { N_("d  Delete from list"), 0, CMD_NONE, 0 };
static MenuItem f_MenuItemNameList = { N_("n  Name this list"), 0, CMD_NONE, 0 };
static MenuItem f_MenuItemBrowseNewPersons = { N_("b  Browse new persons"), 0, CMD_NONE, 0 };
static MenuItem f_MenuItemAddToList = { N_("a  Add to this list"), 0, CMD_NONE, 0 };
static MenuItem f_MenuItemSwapMarkCurrent = { N_("x  Swap mark/current"), 0, CMD_NONE, 0 };
static MenuItem f_MenuItemSources = { N_("$s  List sources"), 0, CMD_SOURCES, 0 };
static MenuItem f_MenuItemNotes = { N_("$n  List notes"), 0, CMD_NOTES, 0 };
static MenuItem f_MenuItemPointers = { N_("$$  List references"), 0, CMD_POINTERS, 0 };
static MenuItem f_MenuItemHistoryBack = { N_("^b  History/back"), 0, CMD_VHISTORY_BACK, 0 };
static MenuItem f_MenuItemHistoryFwd = { N_("^f  History/fwd"), 0, CMD_VHISTORY_FWD, 0 };
static MenuItem f_MenuItemHistoryList = { N_("^l  History list"), 0, CMD_VHISTORY_LIST, 0 };
static MenuItem f_MenuItemHistoryClean = { N_("^c Clear history"), 0, CMD_VHISTORY_CLEAR, 0 };
static MenuItem f_MenuItemChHistoryBack = { N_("^xb  ChngHist/back"), 0, CMD_CHISTORY_BACK, 0 };
static MenuItem f_MenuItemChHistoryFwd = { N_("^xf  ChngHist/fwd"), 0, CMD_CHISTORY_FWD, 0 };
static MenuItem f_MenuItemChHistoryList = { N_("^xl  ChngHist list"), 0, CMD_CHISTORY_LIST, 0 };
static MenuItem f_MenuItemChHistoryClean = { N_("^xc Clear ChngHist"), 0, CMD_CHISTORY_CLEAR, 0 };
/* xgettext:no-c-format */
static MenuItem f_MenuItemAddSour = { N_("%s  Add source"), 0, CMD_ADD_SOUR, 0 };
/* xgettext:no-c-format */
static MenuItem f_MenuItemAddEven = { N_("%e  Add event"), 0, CMD_ADD_EVEN, 0 };
/* xgettext:no-c-format */
static MenuItem f_MenuItemAddOthr = { N_("%o  Add other"), 0, CMD_ADD_OTHR, 0 };


static MenuItem f_MenuItemBrowseFamily = { N_("B  Browse new family"), 0, CMD_BROWSE_FAM, 0 };


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
	&f_MenuItemChHistoryBack,
	&f_MenuItemChHistoryFwd,
	&f_MenuItemChHistoryList,
	&f_MenuItemChHistoryClean,
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
	&f_MenuItemEditTopIndi,
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
	&f_MenuItemEditTopFam,
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
 * browsescreen_init - initialize one browse screen
 * title is strsaved inside here
 * Can be called to reinitialize an existing browsescreen
 * Created: 2002/10/27, Perry Rapp
 *==========================*/
static void
browsescreen_init (struct BrowseScreenInfo * sinfo , STRING title, INT MenuRows, INT MenuCols
	, INT MinCols, INT MaxCols
	, INT MinRows, INT MaxRows
	, INT MenuTop, INT MenuLeft, INT MenuWidth
	, INT MenuSize, MenuItem ** MenuItems)
{
	DYNMENU dynmenu = &sinfo->dynmenu;

	strfree(&sinfo->title);
	sinfo->title = strsave(title);
	dynmenu_init(dynmenu, title, MenuRows, MenuCols
		, MinCols, MaxCols
		, MinRows, MaxRows
		, MenuTop, MenuLeft, MenuWidth
		, MenuSize, MenuItems);
}
/*============================
 * brwsmenu_initialize -- set up menu arrays
 *  screenheightx:  [IN]  user's terminal width
 *  screenweidthx:  [IN]  user's terminal height
 *   (After the first time, these are passed as 0, meaning use earlier values)
 * Created: 2001/01/28, Perry Rapp
 *==========================*/
void
brwsmenu_initialize (INT screenheightx, INT screenwidthx)
{
	INT i;
	INT scr;
	INT MenuRows, MenuCols, MenuSize;
	MenuItem ** MenuItems;
	struct BrowseScreenInfo * sinfo=0;
	char title[120];
	/* defaults used by all browse screens except list browse */
	INT MinRows=4;
	INT MaxRows=10;
	INT MinCols=1;
	INT MaxCols=7;
	INT MenuTop=0;
	INT MenuLeft=3;
	INT MenuWidth=0;
	if (screenheightx > 0)
		f_screenheight = screenheightx;
	if (screenwidthx > 0) {
		f_screenwidth = screenwidthx;
		f_cols = (f_screenwidth-5)/22; /* # of menu cols to start with */
	}
	MenuWidth = f_screenwidth-6;
	MenuCols = f_cols;

	if (!f_initialized) {
		memset(f_BrowseScreenInfo, 0, sizeof(f_BrowseScreenInfo));
		for (i=1; i<=MAX_SCREEN; i++)
		{
			DYNMENU dynmenu;
			sinfo = &f_BrowseScreenInfo[i];
			dynmenu = get_screen_dynmenu(i);
			memset(sinfo, 0, sizeof(*sinfo));
			sinfo->title = strsave(_("Missing title"));
		}
		f_initialized = TRUE;
	}

	scr = ONE_PER_SCREEN;
	sinfo = &f_BrowseScreenInfo[scr];
	llstrncpy(title, _(qSttlindibrw), ARRSIZE(title), uu8);
	MenuRows = 8;
	MenuTop = f_screenheight-MenuRows - 3;
	MenuSize = ARRSIZE(f_MenuPerson)-1;
	MenuItems = f_MenuPerson;
	browsescreen_init(sinfo, title, MenuRows, MenuCols
		, MinCols, MaxCols
		, MinRows, MaxRows
		, MenuTop, MenuLeft, MenuWidth
		, MenuSize, MenuItems);

	scr = ONE_FAM_SCREEN;
	sinfo = &f_BrowseScreenInfo[scr];
	llstrncpy(title, _(qSttlfambrw), ARRSIZE(title), uu8);
	MenuRows = 6;
	MenuTop = f_screenheight-MenuRows - 3;
	MenuSize = ARRSIZE(f_MenuFamily)-1;
	MenuItems = f_MenuFamily;
	browsescreen_init(sinfo, title, MenuRows, MenuCols
		, MinCols, MaxCols
		, MinRows, MaxRows
		, MenuTop, MenuLeft, MenuWidth
		, MenuSize, MenuItems);

	scr = TWO_PER_SCREEN;
	sinfo = &f_BrowseScreenInfo[scr];
	llstrncpy(title, _(qSttl2perbrw), ARRSIZE(title), uu8);
	MenuRows = 5;
	MenuTop = f_screenheight-MenuRows - 3;
	MenuSize = ARRSIZE(f_Menu2Person)-1;
	MenuItems = f_Menu2Person;
	browsescreen_init(sinfo, title, MenuRows, MenuCols
		, MinCols, MaxCols
		, MinRows, MaxRows
		, MenuTop, MenuLeft, MenuWidth
		, MenuSize, MenuItems);

	scr = TWO_FAM_SCREEN;
	sinfo = &f_BrowseScreenInfo[scr];
	llstrncpy(title, _(qSttl2fambrw), ARRSIZE(title), uu8);
	MenuRows = 5;
	MenuTop = f_screenheight-MenuRows - 3;
	MenuSize = ARRSIZE(f_Menu2Family)-1;
	MenuItems = f_Menu2Family;
	browsescreen_init(sinfo, title, MenuRows, MenuCols
		, MinCols, MaxCols
		, MinRows, MaxRows
		, MenuTop, MenuLeft, MenuWidth
		, MenuSize, MenuItems);

	scr = AUX_SCREEN;
	sinfo = &f_BrowseScreenInfo[scr];
	llstrncpy(title, _(qSttlauxbrw), ARRSIZE(title), uu8);
	MenuRows = 4;
	MenuTop = f_screenheight-MenuRows - 3;
	MenuSize = ARRSIZE(f_MenuAux)-1;
	MenuItems = f_MenuAux;
	browsescreen_init(sinfo, title, MenuRows, MenuCols
		, MinCols, MaxCols
		, MinRows, MaxRows
		, MenuTop, MenuLeft, MenuWidth
		, MenuSize, MenuItems);

	/* TO DO: this is not used right now */
	scr = LIST_SCREEN;
	sinfo = &f_BrowseScreenInfo[scr];
	llstrncpy(title, _(qSttllstbrw), ARRSIZE(title), uu8);
	MenuRows = 13;
	MenuCols = 1;
	MenuSize = ARRSIZE(f_MenuListPersons)-1;
	MenuItems = f_MenuListPersons;
	MinRows = 5;
	MaxRows = 14;
	browsescreen_init(sinfo, title, MenuRows, MenuCols
		, MinCols, MaxCols
		, MinRows, MaxRows
		, MenuTop, MenuLeft, MenuWidth
		, MenuSize, MenuItems);

	if (!f_reloading) {
		register_brwsmenu_lang_callbacks(TRUE);
	}
}
/*============================
 * menuitem_terminate_worker -- free menu arrays
 *==========================*/
void
menuitem_terminate (void)
{
	INT i;
	if (!f_reloading) {
		register_brwsmenu_lang_callbacks(FALSE);
	}
	for (i=1; i<=MAX_SCREEN; i++) {
		struct BrowseScreenInfo * sinfo=&f_BrowseScreenInfo[i];
		dynmenu_clear(&sinfo->dynmenu);
		strfree(&sinfo->title);
	}
	f_initialized = FALSE;
}
/*============================
 * register_brwsmenu_lang_callbacks -- (un)register our callbacks
 *  for language or codeset changes
 *==========================*/
static void
register_brwsmenu_lang_callbacks (BOOLEAN registering)
{
	if (registering) {
		register_uilang_callback(brwsmenu_on_lang_change, 0);
		register_uicodeset_callback(brwsmenu_on_lang_change, 0);
	} else {
		unregister_uilang_callback(brwsmenu_on_lang_change, 0);
		unregister_uicodeset_callback(brwsmenu_on_lang_change, 0);
	}
}
/*============================
 * brwsmenu_on_lang_change -- UI language or codeset has changed
 *==========================*/
static void
brwsmenu_on_lang_change (VPTR uparm)
{
	uparm = uparm; /* unused */
	f_reloading = TRUE;
	menuitem_terminate();
	brwsmenu_initialize(0, 0); /* 0 means use stored values */
	f_reloading = FALSE;
}
/*============================
 * get_screen_menuset -- get menuset of specified browse screen
 * Created: 2002/10/27, Perry Rapp
 *==========================*/
MENUSET
get_screen_menuset (INT screen)
{
	return dynmenu_get_menuset(get_screen_dynmenu(screen));
}
/*============================
 * get_screen_dynmenu -- get dynmenu of specified browse screen
 * Created: 2002/10/27, Perry Rapp
 *==========================*/
DYNMENU
get_screen_dynmenu (INT screen)
{
	return &f_BrowseScreenInfo[screen].dynmenu;
}
/*============================
 * get_screen_title -- get title of specified browse screen
 * Created: 2002/10/27, Perry Rapp
 *==========================*/
STRING
get_screen_title (INT screen)
{
	return f_BrowseScreenInfo[screen].title;
}

