#include "sys_inc.h"
#include "standard.h"
#include "llstdlib.h"
#include "feedback.h"

static int llmkdir(STRING);

/*=====================================================
 * llmkdir -- Make directory (some UNIXes have a mkdir)
 *===================================================*/
static int
llmkdir (STRING dir)    /* dir to create */
{
	static int status;
#ifdef WIN32
	status = mkdir(dir);
#else
	status = mkdir(dir,0777);
#endif
	return status == 0;
}
/*===================================
 * mkalldirs -- Make all dirs in path
 *=================================*/
#define exists(p)  (!(*p) || access((p),00) == 0)

BOOLEAN
mkalldirs (STRING path) /* path with dirs to be made */
{
	int i, n;
	char *p = path;

	for (i = 0, n = strlen(path); i < n; i++, p++)  {
		if (!is_dir_sep(*p)) continue;
		*p = 0;
		if (exists(path) || llmkdir(path))  {
			*p = '/';
			continue;
		}
		msg_error("Can't create directory %s", path);
		return FALSE;
	}
	return TRUE;
}
