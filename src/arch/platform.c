/* platform.c */
/* platform-specific functions */

#include "platform.h"

#if defined(_WIN32) || defined(__CYGWIN__)
/* Win32 code also needed by cygwin */
#include <windows.h>
/*=================================================
 * w_get_codepage -- get current Windows codeset
 * Created: 2002/11/27 (Perry Rapp)
 *===============================================*/
int
w_get_codepage (void)
{
	return GetACP();
}
/*=================================================
 * w_get_oemout_codepage -- get current output console codeset
 * Created: 2002/11/27 (Perry Rapp)
 *===============================================*/
int
w_get_oemout_codepage (void)
{
	return GetConsoleOutputCP();
}
/*=================================================
 * w_get_oemin_codepage -- get current input console codeset
 * Created: 2002/11/27 (Perry Rapp)
 *===============================================*/
int
w_get_oemin_codepage (void)
{
	return GetConsoleCP();
}
#endif /* defined(_WIN32) || defined(__CYGWIN__) */
