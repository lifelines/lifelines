/**
 * Author: Emin Martinian 
 *
 * Some repackaging by Perry Rapp
 */

#include<stdio.h>
#include<stdlib.h>

#include "rbtree.h"

/***********************************************************************
 * Data Structures: tree & node
 ***********************************************************************/

/* Compare(a,b) should return 1 if *a > *b, -1 if *a < *b, and 0 otherwise */
/* Destroy(a) takes a pointer to whatever key might be and frees it accordingly */
struct rb_red_blk_tree {
  KeyCompFuncType CompareFnc; 
  KeyInfoDestFuncType DestroyKeyInfoFnc;
  /*  A sentinel is used for root and for nil.  These sentinels are */
  /*  created when RbTreeCreate is caled.  root->left should always */
  /*  point to the node which is the root of the tree.  nil points to a */
  /*  node which should always be black but has aribtrary children and */
  /*  parent and no key or info.  The point of using these sentinels is so */
  /*  that the root and nil nodes do not require special cases in the code */
  RBNODE root;
  RBNODE nil;
  void * param;
  int count;
}; /* *RBTREE already declared in header */

struct rb_red_blk_node {
  RBKEY key;
  RBVALUE info;
  int red; /* if red=0 then the node is black */
  RBNODE left;
  RBNODE right;
  RBNODE parent;
}; /* *RBNODE already declared in header */

struct rb_red_blk_iter {
	RBTREE rbtree;
	RBKEY low;
	RBKEY high;
	RBNODE next; /* to give back at next call, NULL if done */
}; /* *RBITER already declared in header */

/***********************************************************************
 * Module data
 ***********************************************************************/

static void (*f_AssertFnc)(int assertion, const char* error) = 0;
static void * (*f_SafeMallocFnc)(size_t size) = 0;


/***********************************************************************
 * Internal function declarations
 ***********************************************************************/

static void Assert(int assertion, char* error);
static RBNODE FindFirst(RBTREE tree, RBKEY low);
static void InorderTreePrint(RBTREE tree, RBNODE x, KeyPrintFuncType KeyPrintFunc, InfoPrintFuncType InfoPrintFunc);
static void LeftRotate(RBTREE tree, RBNODE x);
static void RbDeleteFixUp(RBTREE tree, RBNODE x);
static void RightRotate(RBTREE tree, RBNODE y);
static void * SafeMalloc(size_t size);
static int TreeCompare(RBTREE tree, RBKEY key1, RBKEY key2);
static void TreeDestHelper(RBTREE tree, RBNODE x);
static void TreeDestroyKeyInfo(RBTREE tree, RBKEY key, RBVALUE info);
static void TreeInsertHelp(RBTREE tree, RBNODE z);
static void TreePrintKey(RBTREE tree, RBKEY key, KeyPrintFuncType KeyPrintFunc);
static void TreePrintInfo(RBTREE tree, RBVALUE info, InfoPrintFuncType InfoPrintFunc);


/***********************************************************************
 * FUNCTION:  RbInitModule
 *
 * Store pointers to necessary infrastructure functions
 ***********************************************************************/

void
RbInitModule (void (*AssertFunc)(int assertion, const char* error),
                  void * (*SafeMallocFunc)(size_t size))
{
	f_AssertFnc = AssertFunc;
	f_SafeMallocFnc = SafeMallocFunc;

	StackInitModule(AssertFunc, SafeMallocFunc);
}


/***********************************************************************/
/*  FUNCTION:  RbTreeCreate */
/**/
/*  INPUTS:  All the inputs are names of functions.  CompFunc takes to */
/*  void pointers to keys and returns 1 if the first arguement is */
/*  "greater than" the second.   DestFunc takes a pointer to a key and */
/*  destroys it in the appropriate manner when the node containing that */
/*  key is deleted.  InfoDestFunc is similiar to DestFunc except it */
/*  recieves a pointer to the info of a node and destroys it. */
/**/
/*  OUTPUT:  This function returns a pointer to the newly created */
/*  red-black tree. */
/**/
/*  Modifies Input: none */
/***********************************************************************/

RBTREE
RbTreeCreate (void * param, KeyCompFuncType KeyCompFunc, KeyInfoDestFuncType KeyInfoDestFunc)
{
  RBTREE newTree;
  RBNODE temp;

  newTree=(RBTREE) SafeMalloc(sizeof(*newTree));
  newTree->param = param;
  newTree->CompareFnc =  KeyCompFunc;
  newTree->DestroyKeyInfoFnc = KeyInfoDestFunc;

  /*  see the comment in the rb_red_blk_tree structure in red_black_tree.h */
  /*  for information on nil and root */
  temp=newTree->nil= (RBNODE) SafeMalloc(sizeof(*temp));
  temp->parent=temp->left=temp->right=temp;
  temp->red=0;
  temp->key=0;
  temp=newTree->root= (RBNODE) SafeMalloc(sizeof(*temp));
  temp->parent=temp->left=temp->right=newTree->nil;
  temp->key=0;
  temp->red=0;
  return(newTree);
}

/***********************************************************************/
/*  FUNCTION:  LeftRotate */
/**/
/*  INPUTS:  This takes a tree so that it can access the appropriate */
/*           root and nil pointers, and the node to rotate on. */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input: tree, x */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly. */
/***********************************************************************/

static void
LeftRotate (RBTREE tree, RBNODE x)
{
  RBNODE y;
  RBNODE nil=tree->nil;

  /*  I originally wrote this function to use the sentinel for */
  /*  nil to avoid checking for nil.  However this introduces a */
  /*  very subtle bug because sometimes this function modifies */
  /*  the parent pointer of nil.  This can be a problem if a */
  /*  function which calls LeftRotate also uses the nil sentinel */
  /*  and expects the nil sentinel's parent pointer to be unchanged */
  /*  after calling this function.  For example, when RbDeleteFixUP */
  /*  calls LeftRotate it expects the parent pointer of nil to be */
  /*  unchanged. */

  y=x->right;
  x->right=y->left;

  if (y->left != nil) y->left->parent=x; /* used to use sentinel here */
  /* and do an unconditional assignment instead of testing for nil */
  
  y->parent=x->parent;   

  /* instead of checking if x->parent is the root as in the book, we */
  /* count on the root sentinel to implicitly take care of this case */
  if( x == x->parent->left) {
    x->parent->left=y;
  } else {
    x->parent->right=y;
  }
  y->left=x;
  x->parent=y;

#ifdef DEBUG_ASSERT
  Assert(!tree->nil->red,"nil not red in LeftRotate");
#endif
}


/***********************************************************************/
/*  FUNCTION:  RighttRotate */
/**/
/*  INPUTS:  This takes a tree so that it can access the appropriate */
/*           root and nil pointers, and the node to rotate on. */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input?: tree, y */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly. */
/***********************************************************************/

static void
RightRotate (RBTREE tree, RBNODE y)
{
  RBNODE x;
  RBNODE nil=tree->nil;

  /*  I originally wrote this function to use the sentinel for */
  /*  nil to avoid checking for nil.  However this introduces a */
  /*  very subtle bug because sometimes this function modifies */
  /*  the parent pointer of nil.  This can be a problem if a */
  /*  function which calls LeftRotate also uses the nil sentinel */
  /*  and expects the nil sentinel's parent pointer to be unchanged */
  /*  after calling this function.  For example, when RbDeleteFixUP */
  /*  calls LeftRotate it expects the parent pointer of nil to be */
  /*  unchanged. */

  x=y->left;
  y->left=x->right;

  if (nil != x->right)  x->right->parent=y; /*used to use sentinel here */
  /* and do an unconditional assignment instead of testing for nil */

  /* instead of checking if x->parent is the root as in the book, we */
  /* count on the root sentinel to implicitly take care of this case */
  x->parent=y->parent;
  if( y == y->parent->left) {
    y->parent->left=x;
  } else {
    y->parent->right=x;
  }
  x->right=y;
  y->parent=x;

#ifdef DEBUG_ASSERT
  Assert(!tree->nil->red,"nil not red in RightRotate");
#endif
}

/***********************************************************************/
/*  FUNCTION:  TreeInsertHelp  */
/**/
/*  INPUTS:  tree is the tree to insert into and z is the node to insert */
/**/
/*  OUTPUT:  none */
/**/
/*  Modifies Input:  tree, z */
/**/
/*  EFFECTS:  Inserts z into the tree as if it were a regular binary tree */
/*            using the algorithm described in _Introduction_To_Algorithms_ */
/*            by Cormen et al.  This funciton is only intended to be called */
/*            by the RbTreeInsert function and not by the user */
/***********************************************************************/

static void
TreeInsertHelp (RBTREE tree, RBNODE z)
{
  /*  This function should only be called by InsertRbTree (see above) */
  RBNODE x;
  RBNODE y;
  RBNODE nil=tree->nil;
  
  z->left=z->right=nil;
  y=tree->root;
  x=tree->root->left;
  while( x != nil) {
    y=x;
    if (0 < TreeCompare(tree, x->key,z->key)) { /* x.key > z.key */
      x=x->left;
    } else { /* x,key <= z.key */
      x=x->right;
    }
  }
  z->parent=y;
  if ( (y == tree->root) ||
       (0 < TreeCompare(tree, y->key,z->key))) { /* y.key > z.key */
    y->left=z;
  } else {
    y->right=z;
  }

#ifdef DEBUG_ASSERT
  Assert(!tree->nil->red,"nil not red in TreeInsertHelp");
#endif
}

/*  Before calling Insert RbTree the node x should have its key set */

/***********************************************************************/
/*  FUNCTION:  RbTreeInsert */
/**/
/*  INPUTS:  tree is the red-black tree to insert a node which has a key */
/*           pointed to by key and info pointed to by info.  */
/**/
/*  OUTPUT:  This function returns a pointer to the newly inserted node */
/*           which is guarunteed to be valid until this node is deleted. */
/*           What this means is if another data structure stores this */
/*           pointer then the tree does not need to be searched when this */
/*           is to be deleted. */
/**/
/*  Modifies Input: tree */
/**/
/*  EFFECTS:  Creates a node node which contains the appropriate key and */
/*            info pointers and inserts it into the tree. */
/***********************************************************************/

RBNODE
RbTreeInsert (RBTREE tree, RBKEY key, RBVALUE info)
{
  RBNODE y;
  RBNODE x;
  RBNODE newNode;

  ++tree->count;
  x=(RBNODE) SafeMalloc(sizeof(*x));
  x->key=key;
  x->info=info;

  TreeInsertHelp(tree,x);
  newNode=x;
  x->red=1;
  while(x->parent->red) { /* use sentinel instead of checking for root */
    if (x->parent == x->parent->parent->left) {
      y=x->parent->parent->right;
      if (y->red) {
	x->parent->red=0;
	y->red=0;
	x->parent->parent->red=1;
	x=x->parent->parent;
      } else {
	if (x == x->parent->right) {
	  x=x->parent;
	  LeftRotate(tree,x);
	}
	x->parent->red=0;
	x->parent->parent->red=1;
	RightRotate(tree,x->parent->parent);
      } 
    } else { /* case for x->parent == x->parent->parent->right */
      y=x->parent->parent->left;
      if (y->red) {
	x->parent->red=0;
	y->red=0;
	x->parent->parent->red=1;
	x=x->parent->parent;
      } else {
	if (x == x->parent->left) {
	  x=x->parent;
	  RightRotate(tree,x);
	}
	x->parent->red=0;
	x->parent->parent->red=1;
	LeftRotate(tree,x->parent->parent);
      } 
    }
  }
  tree->root->left->red=0;
  return(newNode);

#ifdef DEBUG_ASSERT
  Assert(!tree->nil->red,"nil not red in RbTreeInsert");
  Assert(!tree->root->red,"root not red in RbTreeInsert");
#endif
}

/***********************************************************************/
/*  FUNCTION:  RbTreeSuccessor  */
/**/
/*    INPUTS:  tree is the tree in question, and x is the node we want the */
/*             the successor of. */
/**/
/*    OUTPUT:  This function returns the successor of x or NULL if no */
/*             successor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/
  
RBNODE
RbTreeSuccessor (RBTREE tree, RBNODE x)
{
  RBNODE y;
  RBNODE nil=tree->nil;
  RBNODE root=tree->root;

  if (nil != (y = x->right)) { /* assignment to y is intentional */
    while(y->left != nil) { /* returns the minium of the right subtree of x */
      y=y->left;
    }
    return(y);
  } else {
    y=x->parent;
    while(x == y->right) { /* sentinel used instead of checking for nil */
      x=y;
      y=y->parent;
    }
    if (y == root) return(nil);
    return(y);
  }
}

/***********************************************************************/
/*  FUNCTION:  RbTreePredecessor  */
/**/
/*    INPUTS:  tree is the tree in question, and x is the node we want the */
/*             the predecessor of. */
/**/
/*    OUTPUT:  This function returns the predecessor of x or NULL if no */
/*             predecessor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/

RBNODE
RbTreePredecessor (RBTREE tree, RBNODE x)
{
	RBNODE y;
	RBNODE nil=tree->nil;
	RBNODE root=tree->root;

	if (nil != (y = x->left)) { /* assignment to y is intentional */
		while(y->right != nil) { /* returns the maximum of the left subtree of x */
			y=y->right;
		}
		return(y);
	} else {
		y=x->parent;
		while(x == y->left) { 
			if (y == root) return(nil); 
			x=y;
			y=y->parent;
		}
		return(y);
	}
}

/***********************************************************************/
/*  FUNCTION:  InorderTreePrint */
/**/
/*    INPUTS:  tree is the tree to print and x is the current inorder node */
/**/
/*    OUTPUT:  none  */
/**/
/*    EFFECTS:  This function recursively prints the nodes of the tree */
/*              inorder using the PrintKey and PrintInfo functions. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:    This function should only be called from RBTreePrint */
/***********************************************************************/

static void
InorderTreePrint (RBTREE tree, RBNODE x, KeyPrintFuncType KeyPrintFunc, InfoPrintFuncType InfoPrintFunc)
{
  RBNODE nil=tree->nil;
  RBNODE root=tree->root;
  if (x != tree->nil) {
    InorderTreePrint(tree,x->left, KeyPrintFunc, InfoPrintFunc);
    printf("info=");
    TreePrintInfo(tree, x->info, InfoPrintFunc);
    printf("  key="); 
    TreePrintKey(tree, x->key, KeyPrintFunc);
    printf("  l->key=");
    if( x->left == nil) printf("NULL"); else TreePrintKey(tree, x->left->key, KeyPrintFunc);
    printf("  r->key=");
    if( x->right == nil) printf("NULL"); else TreePrintKey(tree, x->right->key, KeyPrintFunc);
    printf("  p->key=");
    if( x->parent == root) printf("NULL"); else TreePrintKey(tree, x->parent->key, KeyPrintFunc);
    printf("  red=%i\n",x->red);
    InorderTreePrint(tree, x->right, KeyPrintFunc, InfoPrintFunc);
  }
}


/***********************************************************************/
/*  FUNCTION:  TreeDestHelper */
/**/
/*    INPUTS:  tree is the tree to destroy and x is the current node */
/**/
/*    OUTPUT:  none  */
/**/
/*    EFFECTS:  This function recursively destroys the nodes of the tree */
/*              postorder using the DestroyKey and DestroyInfo functions. */
/**/
/*    Modifies Input: tree, x */
/**/
/*    Note:    This function should only be called by RbTreeDestroy */
/***********************************************************************/

static void
TreeDestHelper (RBTREE tree, RBNODE x)
{
  RBNODE nil=tree->nil;
  if (x != nil) {
    TreeDestHelper(tree,x->left);
    TreeDestHelper(tree,x->right);
    TreeDestroyKeyInfo(tree, x->key, x->info);
    free(x);
  }
}


/***********************************************************************/
/*  FUNCTION:  RBTreeDestroy */
/**/
/*    INPUTS:  tree is the tree to destroy */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  Destroys the key and frees memory */
/**/
/*    Modifies Input: tree */
/**/
/***********************************************************************/

void
RbTreeDestroy (RBTREE tree)
{
  TreeDestHelper(tree,tree->root->left);
  free(tree->root);
  free(tree->nil);
  free(tree);
}


/***********************************************************************/
/*  FUNCTION:  RbTreePrint */
/**/
/*    INPUTS:  tree is the tree to print */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  This function recursively prints the nodes of the tree */
/*             inorder using the PrintKey and PrintInfo functions. */
/**/
/*    Modifies Input: none */
/**/
/***********************************************************************/

void
RbTreePrint (RBTREE tree, KeyPrintFuncType KeyPrintFunc, InfoPrintFuncType InfoPrintFunc)
{
  InorderTreePrint(tree, tree->root->left, KeyPrintFunc, InfoPrintFunc);
}


/***********************************************************************/
/*  FUNCTION:  RbExactQuery */
/**/
/*    INPUTS:  tree is the tree to print and q is a pointer to the key */
/*             we are searching for */
/**/
/*    OUTPUT:  returns the a node with key equal to q.  If there are */
/*             multiple nodes with key equal to q this function returns */
/*             the one highest in the tree */
/**/
/*    Modifies Input: none */
/**/
/***********************************************************************/
  
RBNODE
RbExactQuery (RBTREE tree, RBKEY q)
{
  RBNODE x=tree->root->left;
  RBNODE nil=tree->nil;
  int compVal;
  if (x == nil) return(0);
  compVal= TreeCompare(tree, x->key,(int*) q);
  while(0 != compVal) {/*assignemnt*/
    if (0 < compVal) { /* x->key > q */
      x=x->left;
    } else {
      x=x->right;
    }
    if ( x == nil) return(0);
    compVal = TreeCompare(tree, x->key,(int*) q);
  }
  return(x);
}


/***********************************************************************/
/*  FUNCTION:  RbDeleteFixUp */
/**/
/*    INPUTS:  tree is the tree to fix and x is the child of the spliced */
/*             out node in RBTreeDelete. */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  Performs rotations and changes colors to restore red-black */
/*             properties after a node is deleted */
/**/
/*    Modifies Input: tree, x */
/**/
/*    The algorithm from this function is from _Introduction_To_Algorithms_ */
/***********************************************************************/

static void
RbDeleteFixUp (RBTREE tree, RBNODE x)
{
  RBNODE root=tree->root->left;
  RBNODE w;

  while( (!x->red) && (root != x)) {
    if (x == x->parent->left) {
      w=x->parent->right;
      if (w->red) {
	w->red=0;
	x->parent->red=1;
	LeftRotate(tree,x->parent);
	w=x->parent->right;
      }
      if ( (!w->right->red) && (!w->left->red) ) { 
	w->red=1;
	x=x->parent;
      } else {
	if (!w->right->red) {
	  w->left->red=0;
	  w->red=1;
	  RightRotate(tree,w);
	  w=x->parent->right;
	}
	w->red=x->parent->red;
	x->parent->red=0;
	w->right->red=0;
	LeftRotate(tree,x->parent);
	x=root; /* this is to exit while loop */
      }
    } else { /* the code below is has left and right switched from above */
      w=x->parent->left;
      if (w->red) {
	w->red=0;
	x->parent->red=1;
	RightRotate(tree,x->parent);
	w=x->parent->left;
      }
      if ( (!w->right->red) && (!w->left->red) ) { 
	w->red=1;
	x=x->parent;
      } else {
	if (!w->left->red) {
	  w->right->red=0;
	  w->red=1;
	  LeftRotate(tree,w);
	  w=x->parent->left;
	}
	w->red=x->parent->red;
	x->parent->red=0;
	w->left->red=0;
	RightRotate(tree,x->parent);
	x=root; /* this is to exit while loop */
      }
    }
  }
  x->red=0;

#ifdef DEBUG_ASSERT
  Assert(!tree->nil->red,"nil not black in RbDeleteFixUp");
#endif
}


/***********************************************************************/
/*  FUNCTION:  RbDeleteNode */
/**/
/*    INPUTS:  tree is the tree to delete node z from */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  Deletes z from tree and frees the key and info of z */
/*             using DestoryKey and DestoryInfo.  Then calls */
/*             RBDeleteFixUp to restore red-black properties */
/**/
/*    Modifies Input: tree, z */
/**/
/*    The algorithm from this function is from _Introduction_To_Algorithms_ */
/***********************************************************************/

void
RbDeleteNode (RBTREE tree, RBNODE z)
{
  RBNODE y;
  RBNODE x;
  RBNODE nil=tree->nil;
  RBNODE root=tree->root;

  y= ((z->left == nil) || (z->right == nil)) ? z : RbTreeSuccessor(tree,z);
  x= (y->left == nil) ? y->right : y->left;
  if (root == (x->parent = y->parent)) { /* assignment of y->p to x->p is intentional */
    root->left=x;
  } else {
    if (y == y->parent->left) {
      y->parent->left=x;
    } else {
      y->parent->right=x;
    }
  }
  if (y != z) { /* y should not be nil in this case */

#ifdef DEBUG_ASSERT
    Assert( (y!=tree->nil),"y is nil in RBDelete\n");
#endif
    /* y is the node to splice out and x is its child */

    if (!(y->red)) RbDeleteFixUp(tree,x);
  
	--tree->count;
    TreeDestroyKeyInfo(tree, z->key, z->info);
    y->left=z->left;
    y->right=z->right;
    y->parent=z->parent;
    y->red=z->red;
    z->left->parent=z->right->parent=y;
    if (z == z->parent->left) {
      z->parent->left=y; 
    } else {
      z->parent->right=y;
    }
    free(z); 
  } else {
	--tree->count;
    TreeDestroyKeyInfo(tree, y->key, y->info);
    if (!(y->red)) RbDeleteFixUp(tree,x);
    free(y);
  }
  
#ifdef DEBUG_ASSERT
  Assert(!tree->nil->red,"nil not black in RbDeleteNode");
#endif
}


/***********************************************************************/
/*  FUNCTION:  RbEnumerate */
/**/
/*    INPUTS:  tree is the tree to look for keys >= low */
/*             and <= high with respect to the Compare function */
/**/
/*    OUTPUT:  stack containing pointers to the nodes between [low,high] */
/**/
/*    Modifies Input: none */
/***********************************************************************/

STKSTACK
RbEnumerate (RBTREE tree, RBKEY low, RBKEY high)
{
	STKSTACK enumResultStack=0;
	RBNODE nil=tree->nil;
	RBNODE x=tree->root->left;
	RBNODE lastBest=nil;

	enumResultStack=StackCreate();
	while(nil != x) {
		if ( 0 < (TreeCompare(tree, x->key, high)) ) { /* x->key > high */
			x=x->left;
		} else {
			lastBest=x;
			x=x->right;
		}
	}
	while ( (lastBest != nil) && (0 >= TreeCompare(tree, low,lastBest->key))) {
		StackPush(enumResultStack,lastBest);
		lastBest=RbTreePredecessor(tree,lastBest);
	}
	return(enumResultStack);
}

/***********************************************************************
 * FUNCTION: RbTraverseDown
 *
 * Traverse tree area between low & high (including them)
 * calling TraverseFunc for each node
 * If TraverseFunc returns 0, stop traversal
 *
 ***********************************************************************/

int
RbTraverseDown (RBTREE tree, RBKEY low, RBKEY high, void *param, TraverseFuncType TraverseFunc)
{
	RBNODE nil=tree->nil;
	RBNODE x=tree->root->left;
	RBNODE lastBest=nil;
	int rtn=1;

	/* Find starting location */
	while(nil != x) {
		if ( 0 < (TreeCompare(tree, x->key, high)) ) { /* x->key > high */
			x=x->left;
		} else {
			lastBest=x;
			x=x->right;
		}
	}

	/* Now traverse, watching for ending location */
	while ( (lastBest != nil) && (0 >= TreeCompare(tree, low, lastBest->key))) {

		rtn = (*TraverseFunc)(lastBest->key, lastBest->info, param);
		if (!rtn) return rtn;
		lastBest=RbTreePredecessor(tree,lastBest);
	}
	return rtn;
}

/***********************************************************************
 * FUNCTION: RbTraverseUp
 *
 * Traverse tree area between low & high (including them)
 * calling TraverseFunc for each node
 * If TraverseFunc returns 0, stop traversal
 *
 ***********************************************************************/

int
RbTraverseUp (RBTREE tree, RBKEY low, RBKEY high, void *param, TraverseFuncType TraverseFunc)
{
	RBNODE nil=tree->nil;
	RBNODE lastBest=nil;
	int rtn=1;

	/* Find starting location */
	lastBest = FindFirst(tree, low);

	/* Now traverse, watching for ending location */
	while ( (lastBest != nil) && (0 <= TreeCompare(tree, high, lastBest->key))) {

		rtn = (*TraverseFunc)(lastBest->key, lastBest->info, param);
		if (!rtn) return rtn;
		lastBest=RbTreeSuccessor(tree,lastBest);
	}
	return rtn;
}

RBNODE
RbTreeFirst (RBTREE tree)
{
	return FindFirst(tree, NULL);
}

RBITER
RbBeginIter (RBTREE tree, RBKEY low, RBKEY high)
{
	RBITER rbit = (RBITER)SafeMalloc(sizeof(*rbit));
	rbit->rbtree = tree;
	rbit->low = low;
	rbit->high = high;
	rbit->next = FindFirst(tree, low);
	return rbit;
}

/***********************************************************************
 * FUNCTION: FindFirst
 *
 * Find lowest key in tree no lower than low
 *
 ***********************************************************************/
static RBNODE
FindFirst (RBTREE tree, RBKEY low)
{
	RBNODE nil=tree->nil;
	RBNODE x=tree->root->left;
	RBNODE lastBest=nil;

	/* Find starting location */
	while(nil != x) {
		if ( low && 0 > (TreeCompare(tree, x->key, low)) ) { /* x->key < low */
			x=x->right;
		} else {
			lastBest=x;
			x=x->left;
		}
	}
	return lastBest;
}

/***********************************************************************
 * FUNCTION: FindLast
 *
 * Find highest key in tree no higher than high
 *
 ***********************************************************************/
static RBNODE
FindLast (RBTREE tree, RBKEY high)
{
	RBNODE nil=tree->nil;
	RBNODE x=tree->root->left;
	RBNODE lastBest=nil;

	/* Find starting location */
	while(nil != x) {
		if ( high && 0 < (TreeCompare(tree, x->key, high)) ) { /* x->key > high */
			x=x->left;
		} else {
			lastBest=x;
			x=x->right;
		}
	}
	return lastBest;
}

int
RbNext (RBITER rbit, RBKEY * pkey, RBVALUE * pinfo)
{
	RBNODE next = rbit->next;
	RBNODE nil = rbit->rbtree->nil;
	if (next == nil) return 0;
	rbit->next = RbTreeSuccessor(rbit->rbtree, next);
	if (rbit->high) {
		if (0 < TreeCompare(rbit->rbtree, next->key, rbit->high)) { /* next->key > rbit->high */
			rbit->next = rbit->rbtree->nil;
			return 0;
		}
	}
	*pkey = next->key;
	*pinfo = next->info;
	return 1;
}

void
RbEndIter (RBITER rbiter)
{
	free(rbiter);
}

int
RbIsNil (RBTREE tree, RBNODE node)
{
	return tree->nil == node;
}
 
RBNODE
RbGetNil (RBTREE tree)
{
	Assert(tree!=0, "RbGetNil(NULL) called");
	return tree->nil;
}


RBKEY
RbGetKey (RBNODE node)
{
	Assert(node!=0, "RbGetKey(NULL) called");
	return node->key;
}

RBVALUE
RbGetInfo(RBNODE node)
{
	Assert(node!=0, "RbGetInfo(NULL) called");
	return node->info;
}
RBVALUE
RbSetInfo (RBNODE node, RBVALUE info)
{
	RBVALUE old;
	Assert(node!=0, "RbSetInfo(NULL,) called");
	old = node->info;
	node->info = info;
	return old;
}

/***********************************************************************
 * Wrappers to call the client-supplied utility functions
 ***********************************************************************/

static int
TreeCompare (RBTREE tree, RBKEY key1, RBKEY key2)
{
	Assert(tree && tree->CompareFnc, "Bad argument to TreeCompare");
	return (*tree->CompareFnc)(key1, key2);
}

static void
TreeDestroyKeyInfo (RBTREE tree, RBKEY key, RBVALUE info)
{
	Assert(tree && tree->DestroyKeyInfoFnc, "Bad argument to TreeDestroyKeyInfo");
	(*tree->DestroyKeyInfoFnc)(tree->param, key, info);
}
static void
TreePrintKey (RBTREE tree, RBKEY key, KeyPrintFuncType KeyPrintFunc)
{
	Assert(tree && KeyPrintFunc, "Bad argument to TreePrintKey");
	(*KeyPrintFunc)(key);
}
static void
TreePrintInfo (RBTREE tree, RBVALUE info, InfoPrintFuncType InfoPrintFunc)
{
	Assert(tree && InfoPrintFunc, "Bad argument to TreePrintInfo");
	(*InfoPrintFunc)(info);
}
static void
Assert (int assertion, char* error)
{
	if (!f_AssertFnc) return;
	(*f_AssertFnc)(assertion, error);
}
static void *
SafeMalloc (size_t size)
{
	return (*f_SafeMallocFnc)(size);
}

/***********************************************************************
 *  NullFunction does nothing it is included so that it can be passed
 *  as a function to RBTreeCreate when no other suitable function has
 *  been defined
 ***********************************************************************/

void
NullFunction(void * junk)
{
	junk=junk; /* unused */
}
int
RbGetCount (RBTREE rbtree)
{
	return rbtree->count;
}
