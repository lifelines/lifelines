#ifndef rbtree_h_included
#define rbtree_h_included

#include"stack.h"

/*************************************************************************
 *  CONVENTIONS:  All data structures for red-black trees have the prefix
 *                "rb_" to prevent name conflicts.
 *
 *                Function names: Each word in a function name begins with
 *                a capital letter.  An example funcntion name is
 *                CreateRedTree(a,b,c). Furthermore, each function name
 *                should begin with a capital letter to easily distinguish
 *                them from variables.
 *
 *                Variable names: Each word in a variable name begins with
 *                a capital letter EXCEPT the first letter of the variable
 *                name.  For example, int newLongInt.  Global variables have
 *                names beginning with "g".  An example of a global
 *                variable name is gNewtonsConstant.
 *
 *                Function pointers in arguments end in Func
 *                Function pointer variables end in Fnc
 *************************************************************************/

/* comment out the line below to remove all the debugging assertion */
/* checks from the compiled code.  */
#define DEBUG_ASSERT 1

typedef struct rb_red_blk_tree * RBTREE;
typedef struct rb_red_blk_node * RBNODE;
typedef struct rb_red_blk_iter * RBITER;
typedef const void * RBKEY;
typedef void * RBVALUE;

/**
 * Typedefs
 */

/* A function which returns 1 if key1>key2, -1 if key1<key2, 0 if equivalent */
typedef int (*KeyCompFuncType)(RBKEY key1, RBKEY key2);

/* A function to free both key & info */
typedef void (*KeyInfoDestFuncType)(void * param, RBKEY key, RBVALUE info);

/* A function to visit one node, during traversal (return 0 to stop traversal) */
typedef int (*TraverseFuncType)(RBKEY key, RBVALUE info, void *param);

/* A function to print a node key */
typedef void (*KeyPrintFuncType)(RBKEY key);

/* A function to print a node info */
typedef void (*InfoPrintFuncType)(RBVALUE info);

/**
 * API
 */

/* Must be called once prior to any other functions in this module */
void RbInitModule(void (*AssertFunc)(int assertion, const char* error),
                  void * (*SafeMalloc)(size_t size));

/* Create tree */
RBTREE RbTreeCreate(void * param, KeyCompFuncType KeyCompFunc, KeyInfoDestFuncType KeyInfoDestFunc);

/* Alter tree */
RBNODE RbTreeInsert(RBTREE, RBKEY key, RBVALUE info);
void RbDeleteNode(RBTREE, RBNODE);

/* Delete tree */
void RbTreeDestroy(RBTREE);

/* Traverse tree */
void RbTreePrint(RBTREE, KeyPrintFuncType KeyPrintFunc, InfoPrintFuncType InfoPrintFunc);
RBNODE RbTreePredecessor(RBTREE,RBNODE);
RBNODE RbTreeSuccessor(RBTREE,RBNODE);
RBNODE RbExactQuery(RBTREE, RBKEY q);
int RbTraverseUp(RBTREE, RBKEY low, RBKEY high, void *param, TraverseFuncType TraverseFunc);
int RbTraverseDown(RBTREE, RBKEY low, RBKEY high, void *param, TraverseFuncType TraverseFunc);
RBNODE RbTreeFirst(RBTREE tree);

/* Fetch subset of tree in stack*/
STKSTACK RbEnumerate(RBTREE tree, RBKEY low, RBKEY high);

/* utility */
RBNODE RbGetNil(RBTREE tree);
int RbIsNil(RBTREE tree, RBNODE node);
void NullFunction(void*);
int RbGetCount(RBTREE);

/* working with node in tree */
RBKEY RbGetKey(RBNODE node);
RBVALUE RbGetInfo(RBNODE node);
RBVALUE RbSetInfo(RBNODE node, RBVALUE info);

/* iteration */
RBITER RbBeginIter(RBTREE tree, RBKEY low, RBKEY high);
int RbNext(RBITER rbit, RBKEY * pkey, RBVALUE * pinfo);
void RbEndIter(RBITER rbiter);

#endif /* rbtree_h_included */
