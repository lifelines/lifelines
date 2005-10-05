#ifndef DBCONTEXT_H_INCLUDED
#define DBCONTEXT_H_INCLUDED

void dbnotify_close(void);
void dbnotify_set(void (*notify)(STRING db, BOOLEAN opening));


#endif /* DBCONTEXT_H_INCLUDED */

