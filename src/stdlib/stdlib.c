#include "llstdlib.h"
#include "table.h"


#ifdef UNUSED_CODE
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
#endif /* UNUSED_CODE */
/*=================================================
 * init_stdlib -- do appropriate module initialization
 *  for entire stdlib module
 *===============================================*/
void
init_stdlib (void)
{
	init_table_module();
}
