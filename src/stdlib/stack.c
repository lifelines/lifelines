/**
 * Author: Emin Martinian 
 *
 * Some repackaging by Perry Rapp
 */

#include<stdio.h>
#include<stdlib.h>

#include "stack.h"

/***********************************************************************
 * Module data
 ***********************************************************************/

static void (*f_AssertFnc)(int assertion, const char* error) = 0;
static void * (*f_SafeMallocFnc)(size_t size) = 0;

/***********************************************************************
 * Data Structures: tree & node
 ***********************************************************************/

struct stk_stack_node {
  DATA_TYPE info;
  struct stk_stack_node * next;
}; /* *STKNODE already declared in header */

struct stk_stack { 
  STKNODE top;
  STKNODE tail;
}; /* *STKSTACK already declared in header */

/***********************************************************************
 * Internal functions
 ***********************************************************************/

/* unused
static void Assert(int assertion, char* error);
*/
static void * SafeMalloc(size_t size);

/***********************************************************************
 * FUNCTION:  StackInitModule
 *
 * Store pointers to necessary infrastructure functions
 ***********************************************************************/

void StackInitModule(void (*AssertFunc)(int assertion, const char* error),
                  void * (*SafeMallocFunc)(size_t size))
{
	f_AssertFnc = AssertFunc;
	f_SafeMallocFnc = SafeMallocFunc;
}



int StackNotEmpty(STKSTACK theStack)
{
  return( theStack ? (int) theStack->top : 0);
}

STKSTACK StackJoin(STKSTACK stack1, STKSTACK stack2)
{
	if (!stack1->tail) {
		free(stack1);
		return(stack2);
	} else {
		stack1->tail->next=stack2->top;
		stack1->tail=stack2->tail;
		free(stack2);
		return(stack1);
	}
}

STKSTACK StackCreate(void)
{
	STKSTACK newStack;

	newStack=(STKSTACK) SafeMalloc(sizeof(*newStack));
	newStack->top=newStack->tail=NULL;
	return(newStack);
}


void StackPush(STKSTACK theStack, DATA_TYPE newInfoPointer)
{
	STKNODE newNode=0;

	newNode=(STKNODE) SafeMalloc(sizeof(*newNode));

	if(!theStack->top) {
		newNode->info=newInfoPointer;
		newNode->next=theStack->top;
		theStack->top=newNode;
		theStack->tail=newNode;
	} else {
		newNode->info=newInfoPointer;
		newNode->next=theStack->top;
		theStack->top=newNode;
	}
  
}

DATA_TYPE StackPop(STKSTACK theStack)
{
	DATA_TYPE popInfo;
	STKNODE oldNode=0;

	if(theStack->top) {
		popInfo=theStack->top->info;
		oldNode=theStack->top;
		theStack->top=theStack->top->next;
		free(oldNode);
		if (!theStack->top) theStack->tail=NULL;
	} else {
		popInfo=NULL;
	}
	return(popInfo);
}

void StackDestroy(STKSTACK theStack,void DestFunc(void * a))
{
	STKNODE x=theStack->top;
	STKNODE y=0;

	if(theStack) {
		while(x) {
			y=x->next;
			DestFunc(x->info);
			free(x);
			x=y;
		}
		free(theStack);
	}
} 
    
/***********************************************************************
 * Wrappers to call the client-supplied utility functions
 ***********************************************************************/

/* unused
static void Assert(int assertion, char* error)
{
	if (!f_AssertFnc) return;
	(*f_AssertFnc)(assertion, error);
}
*/
static void * SafeMalloc(size_t size)
{
	return (*f_SafeMallocFnc)(size);
}
