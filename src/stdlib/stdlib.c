#include "llstdlib.h"
#include "rbtree.h"
#include "stack.h"


/*=================================================
 * stdlib_assert -- if first value is zero, abort program
 *  with informative message as specified
 *===============================================*/
static void
stdlib_assert (int val, const char * msg)
{
	if (val)
		return;
	FATAL2(msg);
}
/*=================================================
 * stdlib_alloc -- alloc mem size (abort if fails)
 *===============================================*/
static void *
stdlib_alloc (size_t numbytes)
{
	void * mem = stdalloc(numbytes);
	ASSERT(mem);
	return mem;
}
/*=================================================
 * init_stdlib -- do appropriate module initialization
 *  for entire stdlib module
 *===============================================*/
void
init_stdlib (void)
{
	/* Red/Black tree and stack modules need external assert & alloc handlers */
	RbInitModule(stdlib_assert, stdlib_alloc);
	StackInitModule(stdlib_assert, stdlib_alloc);
}