#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "standard.h"
#include "gedcom.h"

#if HAVE_PWD_H
#include <pwd.h>
#endif

/*=====================================================
 * get_user_fullname -- Extract current users full name
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
#elif defined(WIN32)
  return "unimplemented";
#else
# error "port me"
#endif

  return retval;
}

/*=======================================================================
 * get_user_email -- Construct email address using username and host name
 *=====================================================================*/
static STRING
get_user_email(void)
{
  STRING retval = NULL;

#if defined(HAVE_GETPWUID)
  static unsigned char username[256];
  unsigned char hostname[256];
  struct passwd *pwent = getpwuid(getuid());
  if (NULL != pwent &&
      (int)sizeof(hostname) > gethostname(hostname, sizeof(hostname)))
    {
      snprintf(username, sizeof(username), "%s@%s", pwent->pw_name, hostname);
      username[sizeof(username)-1] = '\0';
      retval = (STRING) username;
    }
#elif WIN32
  return "unimplementable";
#else
# error "port me"
#endif

  return retval;
}

/*===============================================================
 * get_property -- Get value from useropts, environment or system
 *=============================================================*/
STRING
get_property(STRING opt)
{
  STRING val;

  if (NULL == opt)
    return NULL;

  val = (STRING) valueof(useropts, opt);
  if (NULL == val)
    {
      if (0 == strcmp(opt, "user.fullname"))
        val = get_user_fullname();

      if (0 == strcmp(opt, "user.email"))
        val = get_user_email();
    }
  return val;
}
