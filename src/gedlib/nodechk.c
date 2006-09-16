/* 
   Copyright (c) 2005 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * nodechk.c -- Routine to sanity check node tree
 *==============================================================*/

#include "llstdlib.h"
#include "gedcom.h"
#include "gedcomi.h"



/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void check_node(NODE node, NODE parent, INT level, CNSTRING key, CNSTRING scope);
static void check_node_recursively(NODE node, NODE parent, INT level, CNSTRING key, CNSTRING scope);
static void failreport(CNSTRING msg, INT level, CNSTRING key, CNSTRING scope);

/*********************************************
 * local variables
 *********************************************/

static int nodechecking = 0;

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=================================================
 * nodechk -- Check node tree to which this node belongs
 *===============================================*/
void
nodechk_enable (BOOLEAN enable)
{
	nodechecking = enable;
}
/*=================================================
 * nodechk -- Check node tree to which this node belongs
 *===============================================*/
void
nodechk (NODE node, CNSTRING scope)
{
	NODE root = node;
	CNSTRING key = 0;

	if (!nodechecking) return;

	while (nparent(root)) {
		root = nparent(root);
	}
	key = nxref(node);
	check_node_recursively(root, NULL, 1, key, scope);
}
/*==============================================
 * check_node_recursively -- Check node and all descendant nodes
 *============================================*/
static void
check_node_recursively (NODE node, NODE parent, INT level, CNSTRING key, CNSTRING scope)
{
	NODE child=0;

	check_node(node, parent, level, key, scope);

	for (child = nchild(node); child; child = nsibling(child)) {
		check_node_recursively(child, node, level+1, key, scope);
	}
}
/*==============================================
 * check_node -- Check node (that structure is valid)
 *============================================*/
static void
check_node (NODE node, NODE parent, INT level, CNSTRING key, CNSTRING scope)
{
	if (nparent(node) != parent) {
		failreport("bad parent link", level, key, scope);
	}
	if (parent) {
		if (node->n_cel != parent->n_cel) {
			failreport("bad cel", level, key, scope);
		}
	}
}
/*==============================================
 * failreport -- Report fatal error via FATAL
 *============================================*/
static void
failreport (CNSTRING msg, INT level, CNSTRING key, CNSTRING scope)
{
	char buffer[512];
	snprintf(buffer, 512, "(%s:%s level %ld) %s", scope, key, level, msg);
	FATAL2(buffer);
}
