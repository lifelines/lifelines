#ifndef hashtab_h_included
#define hashtab_h_included

/*
 A simple hash table whose keys are strings and values are HVALUE (void *)
 Hash table copies & manages its own copies of keys
*/

typedef void * HVALUE;
typedef struct tag_hashtab *HASHTAB;
typedef struct tag_hashtab_iter * HASHTAB_ITER;

typedef void (*DELFUNC)(HVALUE val);

/* create and destroy hash table */
HASHTAB create_hashtab(void);
void destroy_hashtab(HASHTAB tab, DELFUNC func);

/* add and remove elements */
HVALUE insert_hashtab(HASHTAB tab, CNSTRING key, HVALUE val);
HVALUE remove_hashtab(HASHTAB tab, CNSTRING key);

/* count of elements */
INT get_hashtab_count(HASHTAB tab);

/* find elements */
HVALUE find_hashtab(HASHTAB tab, CNSTRING key, BOOLEAN * present);
BOOLEAN in_hashtab(HASHTAB tab, CNSTRING key);

/* iterate */
HASHTAB_ITER begin_hashtab(HASHTAB tab);
BOOLEAN next_hashtab(HASHTAB_ITER tabit, CNSTRING *pkey, HVALUE *pval);
HVALUE change_hashtab(HASHTAB_ITER tabit, HVALUE newval);
void end_hashtab(HASHTAB_ITER * ptabit);


#endif /* hashtab_h_included */
