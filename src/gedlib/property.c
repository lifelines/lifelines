/*=============================================================
 * property.c -- Code for properties available to report programs
 *   Created: 2001/01 by Petter Reinholdtsen
 *==============================================================*/

#include "llstdlib.h"
#include "gedcom.h"
#include "lloptions.h"

#if HAVE_PWD_H
#include <pwd.h>
#endif

/*=====================================================
 * get_user_fullname -- Extract current users full name
 *  returns static buffer (actually system buffer)
 *===================================================*/
static STRING
get_user_fullname(void)
{
  STRING retval = NULL;

#if defined(HAVE_GETPWUID)
  /* Get name using getpwuid() */
  struct passwd *pwent = getpwuid(getuid());
  if (NULL != pwent && NULL != pwent->pw_gecos)
    retval = (STRING) pwent->pw_gecos; /* XXX Is it safe to pass this on? */
#else
	return NULL;
#endif

  return retval;
}

/*=======================================================================
 * get_user_email -- Construct email address using username and host name
 *  returns static buffer
 *=====================================================================*/
static STRING
get_user_email (void)
{
  STRING retval = NULL;

#if defined(HAVE_GETPWUID)
  static char username[256];
  char hostname[256];
  struct passwd *pwent = getpwuid(getuid());
  if (NULL != pwent &&
      (int)sizeof(hostname) > gethostname(hostname, sizeof(hostname)))
    {
      llstrncpyf(username, sizeof(username), uu8, "%s@%s",
	       pwent->pw_name, hostname);
      username[sizeof(username)-1] = '\0';
      retval = (STRING) username;
    }
#else
	return NULL;
#endif

  return retval;
}

/*===============================================================
 * get_property -- 
 * Try getlloptstr_rpt, which tries user options table & config file
 * Then handle builtins
 *=============================================================*/
STRING
get_property (STRING opt)
{
  STRING val;

  if (NULL == opt)
    return NULL;

  val = getlloptstr_rpt(opt, NULL);
  if (NULL == val)
    {
      if (eqstr(opt, "user.fullname"))
        val = get_user_fullname();

      if (eqstr(opt, "user.email"))
        val = get_user_email();
    }
  return val;
}
