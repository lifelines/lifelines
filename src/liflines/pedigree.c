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
 * external variables (no header)
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

/*********************************************
 * local enums
 *********************************************/

#define GENS_MAX 7
#define GENS_MIN 2

/*********************************************
 * local function prototypes
 *********************************************/

static treenode add_children(NODE indi, int gen, int maxgen, int * count);
static treenode add_parents(NODE indi, int gen, int maxgen, int * count);
static void print_to_buffer(int keynum, int gen, int * row);
static void trav_pre_print(treenode tn, int * row, int gen);
static void trav_bin_in_print(treenode tn, int * row, int gen);
static void SetScrollMax(int row);
static void show_descendants(NODE indi);
static void show_ancestors(NODE indi);

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
 * add_children -- add children to tree recursively
 *===============================================*/
static treenode
add_children (NODE indi, int gen, int maxgen, int * count)
{
	treenode tn = (treenode)malloc(sizeof(*tn));
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
			remove_indiseq(childseq, FALSE);
		}
	}
	return tn;
}
/*===============================================
 * add_parents -- add parents to tree recursively
 *=============================================*/
static treenode
add_parents (NODE indi, int gen, int maxgen, int * count)
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
 * print_to_buffer -- print output line
 *===================================*/
static void
print_to_buffer (int keynum, int gen, int * row)
{
	char buffer[140], *ptr=buffer;
	int mylen = sizeof(buffer);
	INT width = ll_cols;
	NODE indi = 0;
	int i, overflow=0;
	WINDOW *w = main_win;
	if (mylen > width-5)
		mylen = width-5;
	if (*row>Scrollp && *row-Scrollp<=ll_lines-9) {
		if (*row==Scrollp+1 && Scrollp>0)
			overflow=1;
		if (*row-Scrollp==ll_lines-9 && Scrollp<ScrollMax)
			overflow=1;
		strcpy(ptr, "");
		for (i=0; i<gen*6; i++)
			llstrcatn(&ptr, " ", &mylen);
		if (keynum)
			indi = keynum_to_indi(keynum);
		llstrcatn(&ptr, indi_to_ped_fix(indi, mylen), &mylen);
		put_out_line(w, *row-Scrollp, 1, buffer, width, overflow);
	}
	(*row)++;
}
/*=================================
 * trav_pre_print -- traverse tree,
 *  printing indis in inorder
 *===============================*/
static void
trav_pre_print (treenode tn, int * row, int gen)
{
	treenode n0;
	print_to_buffer(tn->keynum, gen, row);
	for (n0=tn->firstchild; n0; n0=n0->nextsib)
		trav_pre_print(n0, row, gen+1);
}
/*===========================================
 * trav_bin_in_print -- traverse binary tree,
 *  printing indis in inorder
 *=========================================*/
static void
trav_bin_in_print (treenode tn, int * row, int gen)
{
	if (tn->firstchild)
		trav_bin_in_print(tn->firstchild, row, gen+1);
	print_to_buffer(tn->keynum, gen, row);
	if (tn->firstchild && tn->firstchild->nextsib)
		trav_bin_in_print(tn->firstchild->nextsib, row, gen+1);
}
/*======================================================
 * SetScrollMax -- compute max allowable scroll based on
 *  number of rows in this pedigree tree
 *====================================================*/
static void
SetScrollMax (int row)
{
	ScrollMax = row-(ll_lines-9);
	if (ScrollMax<0)
		ScrollMax=0;
}
/*=========================================================
 * show_descendants -- build descendant tree & print it out
 *=======================================================*/
static void
show_descendants (NODE indi)
{
	treenode root;
	int count, row, gen;
	count=0;
	root = add_children(indi, 1, Gens, &count);
	SetScrollMax(count);
	/* inorder traversal */
	row=1;
	gen=0;
	trav_pre_print(root, &row, gen);
}
/*=====================================================
 * show_ancestors -- build ancestor tree & print it out
 *===================================================*/
static void
show_ancestors (NODE indi)
{
	treenode root;
	int count, row, gen;
	count=0;
	root = add_parents(indi, 1, Gens, &count);
	SetScrollMax(count);
	/* inorder traversal */
	row=1;
	gen=0;
	trav_bin_in_print(root, &row, gen);
}
/*=============================================
 * pedigree_show -- display ancestors or
 *  descendants tree, depending on current mode
 *===========================================*/
void
pedigree_show (NODE indi)
{
	if (Ancestors_mode)
		show_ancestors(indi);
	else
		show_descendants(indi);
}
/*===========================================
 * pedigree_toggle_mode -- toggle between 
 *  ancestral & descendant mode pedigree tree
 *=========================================*/
void
pedigree_toggle_mode (void)
{
	Ancestors_mode = !Ancestors_mode;
}
/*======================================================
 * pedigree_increase_generations -- adjust # generations
 *  displayed in pedigree tree
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
 *=============================================================*/
void
pedigree_reset_scroll (void)
{
	Scrollp=0;
}
