#ifndef stack_h_included
#define stack_h_included

/*  CONVENTIONS:  All data structures for stacks have the prefix */
/*                "stk_" to prevent name conflicts. */
/*                                                                      */
/*                Function names: Each word in a function name begins with */
/*                a capital letter.  An example funcntion name is  */
/*                CreateRedTree(a,b,c). Furthermore, each function name */
/*                should begin with a capital letter to easily distinguish */
/*                them from variables. */
/*                                                                     */
/*                Variable names: Each word in a variable name begins with */
/*                a capital letter EXCEPT the first letter of the variable */
/*                name.  For example, int newLongInt.  Global variables have */
/*                names beginning with "g".  An example of a global */
/*                variable name is gNewtonsConstant. */

/*  if DATA_TYPE is undefined then stack.h and stack.c will be code for */
/*  stacks of void *, if they are defined then they will be stacks of the */
/*  appropriate data_type */

#ifndef DATA_TYPE
#define DATA_TYPE void *
#endif


typedef struct stk_stack_node *STKNODE;
typedef struct stk_stack *STKSTACK ;

/* Must be called once prior to any other functions in this module */
void StackInitModule(void (*AssertFunc)(int assertion, const char* error),
                  void * (*SafeMalloc)(size_t size));


/*  These functions are all very straightforward and self-commenting so */
/*  I didn't think additional comments would be useful */
STKSTACK StackJoin(STKSTACK stack1, STKSTACK stack2);
STKSTACK StackCreate(void);
void StackDestroy(STKSTACK theStack,void DestFunc(void * a));
void StackPush(STKSTACK theStack, DATA_TYPE newInfoPointer);
DATA_TYPE StackPop(STKSTACK theStack);
int StackNotEmpty(STKSTACK);

#endif /* stack_h_included */
