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
#include "screen.h"
#include "menuitem.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING mn_ambig;
extern STRING mn_titindi,mn_titfam,mn_titaux,mn_titmain;
extern STRING mn_tit2indi, mn_tit2fam;

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

static void setup_menu(INT screen, STRING Title, INT MenuRows
	, INT MenuCols ,INT Size, MenuItem ** Menu);
static CMDARRAY create_cmd_array(INT alloc);
static void grow_cmd_array(CMDARRAY cmds);
static void copy_cmditem(CMDITEM dest, CMDITEM src);
static BOOLEAN find_cmd(CMDARRAY cmds, char c, INT * pos);
static void insert_cmd(CMDARRAY cmds, STRING str, INT cmdnum
	, STRING display);
static void free_cmds(CMDARRAY cmds);
static INT menuitem_find_cmd(CMDARRAY cmds, STRING cmd);

/*********************************************
 * local variables
 *********************************************/

ScreenInfo g_ScreenInfo[MAX_SCREEN];

/*
Note - these are hardcoded as "?" and "q" in menuitem_check_cmd
 - 2001/02/11, Perry Rapp
*/
MenuItem g_MenuItemOther = { "?  Other menu choices", "?", CMD_MENU_MORE };
MenuItem g_MenuItemQuit = { "q  Return to main menu", "q", CMD_QUIT };

/* normal menu items */
static MenuItem f_MenuItemEditIndi = { "e  Edit the person", "e", CMD_EDIT };
static MenuItem f_MenuItemEditFamily = { "e  Edit the family", "e", CMD_EDIT };
static MenuItem f_MenuItemEdit = { "e  Edit record", "e", CMD_EDIT };
static MenuItem f_MenuItemEditTop = { "e  Edit top person", "e", CMD_EDIT };
static MenuItem f_MenuItemPerson = { "i  Browse to person", "i", CMD_PERSON };
static MenuItem f_MenuItemFather = { "f  Browse to father", "f", CMD_FATHER };
static MenuItem f_MenuItemFatherTop = { "f  Browse top father", "f", CMD_FATHER };
static MenuItem f_MenuItemMother = { "m  Browse to mother", "m", CMD_MOTHER };
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
static MenuItem f_MenuItemSwapFamilies = { "x  Swap two families", "x", CMD_SWAPFAMILIES };
static MenuItem f_MenuItemSwapChildren = { "x  Swap two children", "x", CMD_SWAPCHILDREN };
static MenuItem f_MenuItemReorderChild = { "%c  Reorder child", "%c", CMD_REORDERCHILD };
static MenuItem f_MenuItemSwitchTopBottom = { "x  Switch top/bottom", "x", CMD_SWAPTOPBOTTOM };
static MenuItem f_MenuItemNewPerson = { "n  Create new person", "n", CMD_NEWPERSON };
static MenuItem f_MenuItemNewFamily = { "a  Create new family", "a", CMD_NEWFAMILY };
static MenuItem f_MenuItemTandem = { "tt Enter tandem mode", "tt", CMD_TANDEM };
static MenuItem f_MenuItemTandemFamily = { "tt Enter family tandem", "tt", CMD_TANDEM };
static MenuItem f_MenuItemZipBrowse = { "z  Browse to person", "z", CMD_BROWSE_ZIP };
static MenuItem f_MenuItemRemoveAsSpouse = { "r  Remove as spouse", "r", CMD_REMOVEASSPOUSE };
static MenuItem f_MenuItemRemoveAsChild = { "d  Remove as child", "d", CMD_REMOVEASCHILD };
static MenuItem f_MenuItemRemoveSpouseFrom = { "r  Remove spouse from", "r", CMD_REMOVESPOUSE };
static MenuItem f_MenuItemRemoveChildFrom = { "d  Remove child from", "d" , CMD_REMOVECHILD };
static MenuItem f_MenuItemScrollUp = { "(  Scroll up", "(", CMD_SCROLL_UP };
static MenuItem f_MenuItemScrollDown = { ")  Scroll down", ")", CMD_SCROLL_DOWN };
static MenuItem f_MenuItemDepthUp = { "]  Increase tree depth", "]", CMD_DEPTH_UP };
static MenuItem f_MenuItemDepthDown = { "[  Decrease tree depth", "[", CMD_DEPTH_DOWN };
static MenuItem f_MenuItemScrollUpTop = { "(t Scroll top up", "(t", CMD_SCROLL_TOP_UP };
static MenuItem f_MenuItemScrollDownTop = { ")t Scroll top down", ")t", CMD_SCROLL_TOP_DOWN };
static MenuItem f_MenuItemScrollUpBottom = { "(b Scroll bottom up", "(b", CMD_SCROLL_BOTTOM_UP };
static MenuItem f_MenuItemScrollDownBottom = { ")b Scroll bottom down", ")b", CMD_SCROLL_BOTTOM_DOWN };
static MenuItem f_MenuItemScrollUpBoth = { "(( Scroll both up", "((", CMD_SCROLL_BOTH_UP };
static MenuItem f_MenuItemScrollDownBoth = { ")) Scroll both down", "))", CMD_SCROLL_BOTH_DOWN };
static MenuItem f_MenuItemToggleChildNos = { "#  Toggle childnos", "#", CMD_TOGGLE_CHILDNUMS };
static MenuItem f_MenuItemModeGedcom = { "!g GEDCOM mode", "!g", CMD_MODE_GEDCOM };
static MenuItem f_MenuItemModeGedcomX = { "!x GEDCOMX mode", "!x", CMD_MODE_GEDCOMX };
static MenuItem f_MenuItemModeGedcomT = { "!t GEDCOMT mode", "!t", CMD_MODE_GEDCOMT };
static MenuItem f_MenuItemModeAncestors = { "!a Ancestors mode", "!a", CMD_MODE_ANCESTORS };
static MenuItem f_MenuItemModeDescendants = { "!d Descendants mode", "!d", CMD_MODE_DESCENDANTS };
static MenuItem f_MenuItemModeNormal = { "!n Normal mode", "!n", CMD_MODE_NORMAL };
static MenuItem f_MenuItemModePedigree = { "p  Pedigree mode", "p", CMD_MODE_PEDIGREE };
static MenuItem f_MenuItemModeCycle = { "!! Cycle mode", "!!", CMD_MODE_CYCLE };
/* Note - f_MenuItemDigits has special handling, and must be 123456789 */
static MenuItem f_MenuItemDigits = { "(1-9)  Browse to child", "123456789", CMD_CHILD_DIRECT0 };
MenuItem f_MenuItemSyncMoves = { "y  Turn on sync", "y", CMD_NONE };
static MenuItem f_MenuItemAdvanced = { "A  Advanced view", "A", CMD_ADVANCED };
static MenuItem f_MenuItemTandemChildren = { "C  Tandem to children", "C", CMD_TANDEM_CHILDREN };
static MenuItem f_MenuItemTandemFathers = { "tf  Tandem to father/s", "tf", CMD_TANDEM_FATHERS };
static MenuItem f_MenuItemTandemFamilies = { "G  Tandem to family/s", "G", CMD_TANDEM_FAMILIES };
static MenuItem f_MenuItemBothFathers = { "f  Browse to fathers", "f", CMD_BOTH_FATHERS };
static MenuItem f_MenuItemBothMothers = { "m  Browse to mothers", "m", CMD_BOTH_MOTHERS };
static MenuItem f_MenuItemTandemMothers = { "M  Tandem to mother/s", "M", CMD_TANDEM_MOTHERS };
static MenuItem f_MenuItemTandemSpouses = { "S  Tandem to spouse/s", "S", CMD_TANDEM_SPOUSES };
static MenuItem f_MenuItemTandemParents = { "U  Tandem to parents", "U", CMD_TANDEM_PARENTS };
static MenuItem f_MenuItemEnlargeMenu = { "<  Enlarge menu area", "<", CMD_MENU_GROW };
static MenuItem f_MenuItemShrinkMenu = { ">  Shrink menu area", ">", CMD_MENU_SHRINK };
static MenuItem f_MenuItemNext = { "+  Next in db", "+", CMD_NEXT };
static MenuItem f_MenuItemPrev = { "-  Prev in db", "-", CMD_PREV };
static MenuItem f_MenuItemCopyTopToBottom = { "d  Copy top to bottom", "d", CMD_COPY_TOP_TO_BOTTOM };
static MenuItem f_MenuItemMergeBottomToTop = { "j  Merge bottom to top", "j", CMD_MERGE_BOTTOM_TO_TOP};
static MenuItem f_MenuItemMoveDownList = { "j  Move down list", "j", CMD_NONE };
static MenuItem f_MenuItemMoveUpList = { "k  Move up list", "k", CMD_NONE };
static MenuItem f_MenuItemEditThis = { "e  Edit this person", "e", CMD_NONE };
static MenuItem f_MenuItemBrowseThis = { "i  Browse this person", "i", CMD_BROWSE_INDI };
static MenuItem f_MenuItemMarkThis = { "m  Mark this person", "m", CMD_NONE };
static MenuItem f_MenuItemDeleteFromList = { "d  Delete from list", "d", CMD_NONE };
static MenuItem f_MenuItemNameList = { "n  Name this list", "n", CMD_NONE };
static MenuItem f_MenuItemBrowseNewPersons = { "b  Browse new persons", "b", CMD_NONE };
static MenuItem f_MenuItemAddToList = { "a  Add to this list", "a", CMD_NONE };
static MenuItem f_MenuItemSwapMarkCurrent = { "x  Swap mark/current", "x", CMD_NONE };
static MenuItem f_MenuItemSources = { "$s  List sources", "$s", CMD_SOURCES };
static MenuItem f_MenuItemNotes = { "$n  List notes", "$n", CMD_NOTES };
static MenuItem f_MenuItemPointers = { "$$  List references", "$$", CMD_POINTERS };
static MenuItem f_MenuItemHistoryBack = { "^b  History/back", "^b", CMD_HISTORY_BACK };
static MenuItem f_MenuItemHistoryFwd = { "^f  History/fwd", "^f", CMD_HISTORY_FWD };
static MenuItem f_MenuItemHistoryList = { "^l  History list", "^l", CMD_HISTORY_LIST };
static MenuItem f_MenuItemAddOther = { "%o  Add other ref", "%o", CMD_ADD_OTHER_REF };


static MenuItem f_MenuItemBrowseFamily = { "B  Browse new family", "B", CMD_BROWSE_FAM };



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
	&f_MenuItemAddOther,
	&f_MenuItemSwapFamilies,
	&f_MenuItemTandem,
	&f_MenuItemZipBrowse,
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
	&f_MenuItemSources,
	&f_MenuItemNotes,
	&f_MenuItemPointers,
	&f_MenuItemNext,
	&f_MenuItemPrev,
	&f_MenuItemHistoryBack,
	&f_MenuItemHistoryFwd,
	&f_MenuItemHistoryList,
	0
};

static MenuItem * f_MenuFamily[] =
{
	&f_MenuItemEditFamily,
	&f_MenuItemFather,
	&f_MenuItemMother,
	&f_MenuItemChildren,
	&f_MenuItemNewPerson,
	&f_MenuItemAddSpouse,
	&f_MenuItemAddChild,
	&f_MenuItemRemoveSpouseFrom,
	&f_MenuItemRemoveChildFrom,
	&f_MenuItemSwapChildren,
	&f_MenuItemReorderChild,
	&f_MenuItemTandemFamily,
	&f_MenuItemBrowse,
	&f_MenuItemZipBrowse,
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
	&f_MenuItemSources,
	&f_MenuItemNotes,
	&f_MenuItemPointers,
	&f_MenuItemNext,
	&f_MenuItemPrev,
	&f_MenuItemHistoryBack,
	&f_MenuItemHistoryFwd,
	&f_MenuItemHistoryList,
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
	0
};

static MenuItem * f_MenuAux[] =
{
	&f_MenuItemEdit,
	&f_MenuItemScrollUp,
	&f_MenuItemScrollDown,
	&f_MenuItemEnlargeMenu,
	&f_MenuItemShrinkMenu,
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
	, INT Size, MenuItem ** Menu)
{
	INT i, j;
	CMDARRAY cmds = create_cmd_array(32);
	STRING defaults = "q?";
	g_ScreenInfo[screen].Title = Title;
	g_ScreenInfo[screen].MenuRows = MenuRows;
	g_ScreenInfo[screen].MenuCols = MenuCols;
	g_ScreenInfo[screen].MenuSize = Size;
	/* MenuPage set by caller */
	g_ScreenInfo[screen].Menu = Menu;
	g_ScreenInfo[screen].Commands = cmds;
	for (i=0; i<Size; i++) {
		if (Menu[i]->Command == CMD_CHILD_DIRECT0) {
			ASSERT(eqstr(Menu[i]->Choices, "123456789"));
			for (j=1; j<=9; j++) {
				char choice[2];
				sprintf(choice, "%d\0", j);
				insert_cmd(cmds, choice, CMD_CHILD_DIRECT0+j, Menu[i]->Display);
			}
		} else {
			/* TO DO - check that width is not too large */
			insert_cmd(cmds, Menu[i]->Choices, Menu[i]->Command, Menu[i]->Display);
		}
	}
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
			sprintf(error, mn_ambig, display);
			message(error);
			FATAL();
		} else {
			/* multicharacter new cmd */
			if (cmds->array[pos].direct) {
				sprintf(error, mn_ambig, display);
				message(error);
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
	INT ItemSize;

	ItemSize = sizeof(f_MenuPerson[0]);
	for (i=1; i<MAX_SCREEN; i++)
	{
		g_ScreenInfo[i].Title = "Invalid";
		g_ScreenInfo[i].MenuRows = 0;
		g_ScreenInfo[i].MenuCols = 3;
		g_ScreenInfo[i].MenuSize = 0;
		g_ScreenInfo[i].Commands = NULL;
		g_ScreenInfo[i].Menu = NULL;
	}

	scr = MAIN_SCREEN;
	g_ScreenInfo[scr].Title = mn_titmain;

	scr = ONE_PER_SCREEN;
	Title = mn_titindi;
	MenuRows = 8;
	MenuCols = 3;
	MenuSize = sizeof(f_MenuPerson)/ItemSize-1;
	Menu = f_MenuPerson;
	setup_menu(scr, Title, MenuRows, MenuCols, MenuSize, Menu);

	scr = ONE_FAM_SCREEN;
	Title = mn_titfam;
	MenuRows = 6;
	MenuCols = 3;
	MenuSize = sizeof(f_MenuFamily)/ItemSize-1;
	Menu = f_MenuFamily;
	setup_menu(scr, Title, MenuRows, MenuCols, MenuSize, Menu);

	scr = TWO_PER_SCREEN;
	Title = mn_tit2indi;
	MenuRows = 5;
	MenuCols = 3;
	MenuSize = sizeof(f_Menu2Person)/ItemSize-1;
	Menu = f_Menu2Person;
	setup_menu(scr, Title, MenuRows, MenuCols, MenuSize, Menu);

	scr = TWO_FAM_SCREEN;
	Title = mn_tit2fam;
	MenuRows = 5;
	MenuCols = 3;
	MenuSize = sizeof(f_Menu2Family)/ItemSize-1;
	Menu = f_Menu2Family;
	setup_menu(scr, Title, MenuRows, MenuCols, MenuSize, Menu);

	scr = LIST_SCREEN;
	Title = (STRING)"LifeLines -- List Browse Screen";
	MenuRows = 13;
	MenuCols = 1;
	MenuSize = sizeof(f_MenuListPersons)/ItemSize-1;
	Menu = f_MenuListPersons;
	setup_menu(scr, Title, MenuRows, MenuCols, MenuSize, Menu);

	scr = AUX_SCREEN;
	Title = mn_titaux;
	MenuRows = 4;
	MenuCols = 3;
	MenuSize = sizeof(f_MenuAux)/ItemSize-1;
	Menu = f_MenuAux;
	setup_menu(scr, Title, MenuRows, MenuCols, MenuSize, Menu);


	for (i=1; i<MAX_SCREEN; i++)
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
	for (i=1; i<MAX_SCREEN; i++) {
		if (g_ScreenInfo[i].Commands) {
			free_cmds(g_ScreenInfo[i].Commands);
			g_ScreenInfo[i].Commands=0;
		}
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
	if (*str == 'q') return CMD_QUIT;
	if (*str == '?') return CMD_MENU_MORE;
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
