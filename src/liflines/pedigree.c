/* 
   Copyright (c) 2000-2001 Perry Rapp

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
 * pedigree.c -- Display the pedigree browse screen
 *   Created: 2000/10 by Perry Rapp
 *==============================================================*/


#include "llstdlib.h"
#include "liflines.h"
#include "screen.h"
#include "mystring.h"

#include "llinesi.h"

/*********************************************
 * global/exported variables
 *********************************************/

/*********************************************
 * external/imported variables
 *********************************************/

/*********************************************
 * local types
 *********************************************/

struct treenode_s
{
	struct treenode_s * firstchild;
	struct treenode_s * nextsib;
	int keynum;
};
typedef struct treenode_s *treenode;
typedef STRING (*LINEPRINT_FNC)(INT width, void * param);
typedef struct indi_print_param_s
{
	INT keynum;
} *INDI_PRINT_PARAM;
typedef struct node_print_param_s
{
	NODE node;
	INT gdvw;
} *NODE_PRINT_PARAM;

/*********************************************
 * local enums & defines
 *********************************************/

#define GENS_MAX 7
#define GENS_MIN 2

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static treenode add_children(NODE indi, INT gen, INT maxgen, INT * count);
static treenode add_parents(NODE indi, INT gen, INT maxgen, INT * count);
static treenode alloc_treenode(void);
static void count_nodes(NODE node, INT gen, INT maxgen, INT * count);
static void free_tree(treenode root);
static void free_treenode(treenode tn);
static STRING indi_lineprint(INT width, void * param);
static void print_to_screen(INT gen, INT * row, LINEPRINT_FNC, void *param, INT minrow, INT maxrow);
static void trav_pre_print_tn(treenode tn, INT * row, INT gen, INT minrow, INT maxrow);
static void trav_pre_print_nd(NODE node, INT * row, INT gen, INT minrow, INT maxrow, INT gdvw);
static void trav_bin_in_print_tn(treenode tn, INT * row, INT gen, INT minrow, INT maxrow);
static void SetScrollMax(INT row, INT hgt);

/*********************************************
 * local variables
 *********************************************/

static int Gens = 4;
static int Ancestors_mode = 1;
static int Scrollp = 0;
static int ScrollMax = 0;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=================================================
 * alloc_treenode -- get new treenode
 *  In preparation for using a block allocator
 * Created: 2001/04/14, Perry Rapp
 *===============================================*/
static treenode
alloc_treenode (void)
{
	treenode tn = (treenode)malloc(sizeof(*tn));
	return tn;
}
/*========================================
 * free_treenode -- return treenode to free-list
 * (see alloc_treenode comments)
 * Created: 2001/04/04, Perry Rapp
 *======================================*/
static void
free_treenode (treenode tn)
{
	free(tn);
}
/*=================================================
 * add_children -- add children to tree recursively
 * Created: 2000/12/07, Perry Rapp
 *===============================================*/
static treenode
add_children (NODE indi, INT gen, INT maxgen, INT * count)
{
	treenode tn = alloc_treenode();
	treenode tn0, tn1;
	int i;

	tn->keynum = indi_to_keynum(indi);
	tn->firstchild = 0;
	tn->nextsib = 0;
	(*count)++;

	if (gen < maxgen) {
		INDISEQ childseq = indi_to_children(indi);
		if (childseq) {
			tn0=0;
			for (i=0; i<length_indiseq(childseq); i++) {
				NODE child;
				STRING childkey, childname;
				element_indiseq(childseq, i, &childkey, &childname);
				child = key_to_indi(childkey);
				tn1 = add_children(child, gen+1, maxgen, count);
				if (tn0)
					tn0 = tn0->nextsib = tn1;
				else
					tn0 = tn->firstchild = tn1;
			}
			remove_indiseq(childseq);
		}
	}
	return tn;
}
/*=================================================
 * count_nodes -- count descendent nodes
 * Created: 2001/01/27, Perry Rapp
 *===============================================*/
static void
count_nodes (NODE node, INT gen, INT maxgen, INT * count)
{
	(*count)++;

	if (gen < maxgen) {
		NODE child;
		for (child = nchild(node); child; child = nsibling(child)) {
			count_nodes(child, gen+1, maxgen, count);
		}
	}
}
/*===============================================
 * add_parents -- add parents to tree recursively
 * Created: 2000/12/07, Perry Rapp
 *=============================================*/
static treenode
add_parents (NODE indi, INT gen, INT maxgen, INT * count)
{
	treenode tn = (treenode)malloc(sizeof(*tn));
	tn->keynum = indi_to_keynum(indi);
	tn->firstchild = 0;
	tn->nextsib = 0;
	(*count)++;
	if (gen<maxgen) {
		tn->firstchild = 
			add_parents(indi_to_fath(indi), gen+1, maxgen, count);
		/* reload indi in case lost from cache */
		if (tn->keynum)
			indi=keynum_to_indi(tn->keynum);
		tn->firstchild->nextsib = 
			add_parents(indi_to_moth(indi), gen+1, maxgen, count);
	}
	return tn;
}
/*=====================================
 * print_to_screen -- print output line
 *  using caller-supplied print function
 *  this routine handles scroll & indent
 * Created: 2000/12/07, Perry Rapp
 *===================================*/
static void
print_to_screen (INT gen, INT * row, LINEPRINT_FNC fnc,
	void * param, INT minrow, INT maxrow)
{
	char buffer[140], *ptr=buffer;
	STRING line;
	int mylen = sizeof(buffer);
	INT width = ll_cols;
	NODE indi = 0;
	int i, overflow=0;
	WINDOW *w = main_win;
	if (mylen > width-2)
		mylen = width-2;
	if (*row-Scrollp>=minrow && *row-Scrollp<=maxrow) {
		if (*row-Scrollp==minrow && Scrollp>0)
			overflow=1;
		if (*row-Scrollp==maxrow && Scrollp<ScrollMax)
			overflow=1;
		strcpy(ptr, "");
		for (i=0; i<gen*6; i++)
			llstrcatn(&ptr, " ", &mylen);
		line = (*fnc)(mylen, param);
		llstrcatn(&ptr, line, &mylen);
		put_out_line(w, *row-Scrollp, 1, buffer, width, overflow);
	}
	(*row)++;
}
/*=================================
 * indi_lineprint -- print an indi line
 *  in an ancestral or descendant tree
 * returns static buffer
 * Created: 2001/01/27, Perry Rapp
 *===============================*/
static STRING
indi_lineprint (INT width, void * param)
{
	INDI_PRINT_PARAM ipp = (INDI_PRINT_PARAM)param;
	NODE indi=0;
	if (ipp->keynum)
		indi = keynum_to_indi(ipp->keynum);
	return indi_to_ped_fix(indi, width);
}
/*=================================
 * node_lineprint -- print a node line
 *  in a node tree
 * This is used in gedcom view & extended
 *  gedcom views
 * returns static buffer
 * Created: 2001/01/27, Perry Rapp
 *===============================*/
static STRING
node_lineprint (INT width, void * param)
{
	static char line[120];
	STRING ptr=line;
	INT mylen=sizeof(line);
	NODE_PRINT_PARAM npp = (NODE_PRINT_PARAM)param;
	NODE node=npp->node;
	if (mylen>width)
		mylen=width;
	if (ntag(node)) {
		llstrcatn(&ptr, ntag(node), &mylen);
		llstrcatn(&ptr, " ", &mylen);
	}
	if (nxref(node)) {
		llstrcatn(&ptr, nxref(node), &mylen);
		llstrcatn(&ptr, " ", &mylen);
	}
	if (nval(node)) {
		llstrcatn(&ptr, nval(node), &mylen);
		llstrcatn(&ptr, " ", &mylen);
	}
	if (npp->gdvw == GDVW_EXPANDED && nval(node)) {
		STRING key = rmvat(nval(node)), str;
		if (key) {
			str = generic_to_list_string(NULL, key, mylen, ",");
			llstrcatn(&ptr, " : ", &mylen);
			llstrcatn(&ptr, str, &mylen);
		}
	}
	return line;
}
/*=================================
 * trav_pre_print_tn -- traverse treenode tree,
 *  printing indis in preorder
 * Created: 2000/12/07, Perry Rapp
 *===============================*/
static void
trav_pre_print_tn (treenode tn, INT * row, INT gen, 
	INT minrow, INT maxrow)
{
	treenode n0;
	struct indi_print_param_s ipp;
	ipp.keynum = tn->keynum;
	print_to_screen(gen, row, &indi_lineprint, &ipp, minrow, maxrow);
	for (n0=tn->firstchild; n0; n0=n0->nextsib)
		trav_pre_print_tn(n0, row, gen+1, minrow, maxrow);
}
/*=================================
 * trav_pre_print_nd -- traverse node tree,
 *  printing nodes in preorder
 * Created: 2001/01/27, Perry Rapp
 *===============================*/
static void
trav_pre_print_nd (NODE node, INT * row, INT gen, 
	INT minrow, INT maxrow, INT gdvw)
{
	NODE child;
	struct node_print_param_s npp;
	npp.node = node;
	npp.gdvw = gdvw;
	print_to_screen(gen, row, &node_lineprint, &npp, minrow, maxrow);
	for (child=nchild(node); child; child=nsibling(child))
		trav_pre_print_nd(child, row, gen+1, minrow, maxrow, gdvw);
}
/*===========================================
 * trav_bin_in_print_tn -- traverse binary tree,
 *  printing indis in inorder
 * Created: 2000/12/07, Perry Rapp
 *=========================================*/
static void
trav_bin_in_print_tn (treenode tn, INT * row, INT gen,
	INT minrow, INT maxrow)
{
	struct indi_print_param_s ipp;
	ipp.keynum = tn->keynum;
	if (tn->firstchild)
		trav_bin_in_print_tn(tn->firstchild, row, gen+1,
			minrow, maxrow);
	print_to_screen(gen, row, &indi_lineprint, &ipp, minrow, maxrow);
	if (tn->firstchild && tn->firstchild->nextsib)
		trav_bin_in_print_tn(tn->firstchild->nextsib, row, gen+1,
			minrow, maxrow);
}
/*======================================================
 * SetScrollMax -- compute max allowable scroll based on
 *  number of rows in this pedigree tree
 * Created: 2000/12/07, Perry Rapp
 *====================================================*/
static void
SetScrollMax (INT row, INT hgt)
{
	ScrollMax = row-hgt;
	if (ScrollMax<0)
		ScrollMax=0;
}
/*=========================================================
 * draw_descendants -- build descendant tree & print it out
 * Created: 2000/12/07, Perry Rapp
 *  refer optimization note in show_ancestors
 *=======================================================*/
void
pedigree_draw_descendants (NODE indi, INT row, INT hgt)
{
	treenode root;
	int count, gen;
	count=0;
	root = add_children(indi, 1, Gens, &count);
	SetScrollMax(count, hgt);
	/* inorder traversal */
	gen=0;
	trav_pre_print_tn(root, &row, gen, row, row+hgt-1);
	free_tree(root);
}
/*=========================================================
 * pedigree_draw_gedcom -- print out gedcom node tree
 * Created: 2001/01/27, Perry Rapp
 *=======================================================*/
void
pedigree_draw_gedcom (NODE node, INT gdvw, INT row, INT hgt)
{
	INT count, gen;
	count=0;
	count_nodes(node, 1, Gens, &count);
	SetScrollMax(count, hgt);
	/* preorder traversal */
	gen=0;
	trav_pre_print_nd(node, &row, gen, row, row+hgt-1, gdvw);
}
/*=====================================================
 * show_ancestors -- build ancestor tree & print it out
 * Created: 2000/12/07, Perry Rapp
 *  Possible optimization (courtesy Petter Reinholdtsen):
 *  Call add_parents traversal twice - first time just
 *  count, then alloc one big block, then traverse again
 *  filling it in - one malloc instead of N mallocs
 *  Easiest way is add a flag to add_parents so use same
 *  traversal code both times
 * Another optimization would be to use a custom allocator
 *  (like PVALUES, NODES, etc)
 *===================================================*/
void
pedigree_draw_ancestors (NODE indi, INT row, INT hgt)
{
	treenode root;
	int count, gen;
	count=0;
	root = add_parents(indi, 1, Gens, &count);
	SetScrollMax(count, hgt);
	/* inorder traversal */
	gen=0;
	trav_bin_in_print_tn(root, &row, gen, row, row+hgt-1);
	free_tree(root);
}
/*===========================================
 * pedigree_toggle_mode -- toggle between 
 *  ancestral & descendant mode pedigree tree
 * Created: 2000/12/07, Perry Rapp
 *=========================================*/
void
pedigree_toggle_mode (void)
{
	Ancestors_mode = !Ancestors_mode;
}
/*======================================================
 * pedigree_increase_generations -- adjust # generations
 *  displayed in pedigree tree
 * Created: 2000/12/07, Perry Rapp
 *====================================================*/
void
pedigree_increase_generations (INT delta)
{
	Gens += delta;
	if (Gens > GENS_MAX)
		Gens = GENS_MAX;
	else if (Gens < GENS_MIN)
		Gens = GENS_MIN;
}
/*===========================================================
 * pedigree_scroll -- scroll up/down rows of pedigree display
 * Created: 2000/12/07, Perry Rapp
 *=========================================================*/
void
pedigree_scroll (INT delta)
{
	Scrollp += delta;
	if (Scrollp < 0)
		Scrollp = 0;
	else if (Scrollp > ScrollMax)
		Scrollp = ScrollMax;
}
/*===============================================================
 * pedigree_reset_scroll -- clear scroll when entering a new indi
 * Created: 2000/12/07, Perry Rapp
 *=============================================================*/
void
pedigree_reset_scroll (void)
{
	Scrollp=0;
}
/*===============================================================
 * free_tree -- free a treenode tree
 * Created: 2000/02/01, Perry Rapp
 *=============================================================*/
static void
free_tree (treenode root)
{
	treenode child=root->firstchild;
	treenode sib=root->nextsib;
	free_treenode(root);
	if (child)
		free_tree(child);
	if (sib)
		free_tree(sib);
}
