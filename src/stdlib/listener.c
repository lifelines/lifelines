/* 
   Copyright (c) 2000-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*==========================================================
 * listener.c -- manage a simple list of listeners
 *   created 2002-09-26 (Perry Rapp)
 *========================================================*/

#include "llstdlib.h"
#include "gedcom.h"

/*********************************************
 * local types
 *********************************************/

struct callback_info { CALLBACK_FNC fnc; VPTR uparm; };

/*********************************************
 * local function prototypes
 *********************************************/
static void remove_callback_info (VPTR vptr);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*===============================================
 * add_listener -- add new listener onto list (no dup check)
 *=============================================*/
void
add_listener (LIST * notifiees, CALLBACK_FNC fncptr, VPTR uparm)
{
	struct callback_info * info = 
		(struct callback_info *)malloc(sizeof(*info));
	info->fnc = fncptr;
	info->uparm = uparm;
	if (!*notifiees)
		*notifiees = create_list();
	enqueue_list(*notifiees, (VPTR)info);
}
/*===============================================
 * remove_callback_info -- Delete contents of one slot of f_listeners
 * Created: 2003-02-02 (Perry Rapp)
 *=============================================*/
static void
remove_callback_info (VPTR vptr)
{
	struct callback_info * info = (struct callback_info *)vptr;
	ASSERT(info);
	free(info);
}
/*===============================================
 * remove_listeners -- Empty & remove list
 *=============================================*/
void
remove_listeners (LIST * notifiees)
{
	if (*notifiees) {
		remove_list(*notifiees, remove_callback_info);
		*notifiees = 0;
	}
}
/*===============================================
 * delete_listener -- Remove one callback from a callback list
 *  (removes first instance)
 *=============================================*/
void
delete_listener (LIST * notifiees, CALLBACK_FNC fncptr, VPTR uparm)
{
	/* Our lists don't have remove from middle, so we just make a new copy */
	LIST lold = 0;
	BOOLEAN found = FALSE;
	if (!*notifiees || is_empty_list(*notifiees))
		return;
	lold = *notifiees;
	*notifiees = create_list();
	while (!is_empty_list(lold)) {
		struct callback_info * info = (struct callback_info *)pop_list(lold);
		if (!found && info->fnc == fncptr && info->uparm == uparm) {
			found = TRUE;
			info->fnc = NULL;
			free(info);
		} else {
			enqueue_list(*notifiees, (VPTR)info);
		}
	}
	make_list_empty(lold);
	remove_list(lold, 0);
}
/*===============================================
 * notify_listeners -- Send notifications to any registered listeners
 *=============================================*/
void
notify_listeners (LIST * notifiees)
{
	struct callback_info * info;
	LIST list;
	if (!notifiees || is_empty_list(*notifiees))
		return;
	list = *notifiees;
	FORLIST(list, el)
		info = (struct callback_info *)el;
		(*info->fnc)(info->uparm);
	ENDLIST
}
