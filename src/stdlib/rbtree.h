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

/**
 * Typedefs
 */

/* A function which returns 1 if key1>key2, -1 if key1<key2, 0 if equivalent */
typedef int (*KeyCompFuncType)(const void *key1, const void *key2);

/* A function to free both key & info */
typedef void (*KeyInfoDestFuncType)(const void *key, const void *info);

/* A function to visit one node, during traversal (return 0 to stop traversal) */
typedef int (*TraverseFuncType)(const void *key, const void *info, void *param);

/* A function to print a node key */
typedef void (*KeyPrintFuncType)(const void *key);

/* A function to print a node info */
typedef void (*InfoPrintFuncType)(void *info);

/**
 * API
 */

/* Must be called once prior to any other functions in this module */
void RbInitModule(void (*AssertFunc)(int assertion, const char* error),
                  void * (*SafeMalloc)(size_t size));

/* Create tree */
RBTREE RbTreeCreate(KeyCompFuncType KeyCompFunc, KeyInfoDestFuncType KeyInfoDestFunc);

/* Alter tree */
RBNODE RbTreeInsert(RBTREE, void* key, void* info);
void RbDeleteNode(RBTREE, RBNODE);

/* Delete tree */
void RbTreeDestroy(RBTREE);

/* Traverse tree */
void RbTreePrint(RBTREE, KeyPrintFuncType KeyPrintFunc, InfoPrintFuncType InfoPrintFunc);
RBNODE RbTreePredecessor(RBTREE,RBNODE);
RBNODE RbTreeSuccessor(RBTREE,RBNODE);
RBNODE RbExactQuery(RBTREE, void*);
int RbTraverseUp(RBTREE, void *low, void *high, void *param, TraverseFuncType TraverseFunc);
int RbTraverseDown(RBTREE, void *low, void *high, void *param, TraverseFuncType TraverseFunc);

/* Fetch subset of tree in stack*/
STKSTACK RbEnumerate(RBTREE tree,void* low, void* high);

/* utility */
RBNODE RbGetNil(RBTREE tree);
int RbIsNil(RBTREE tree, RBNODE node);
void * RbGetKey(RBNODE node);
void * RbGetInfo(RBNODE node);
void NullFunction(void*);

#endif /* rbtree_h_included */
