#ifndef array_h_included
#define array_h_included


/*=================================================
 * ARRAY -- Data type for an array
 *===============================================*/
struct tag_array {
	struct tag_vtable * vtable; /* generic object table (see vtable.h) */
	INT ar_refcnt; /* reference counted object */
	INT ar_size;    /* currently populated size */
	INT ar_max;     /* allocated size */
	void **ar_data;  /* actual array data */
};
typedef struct tag_array *ARRAY;

typedef INT (*OBJCMPFNC)(OBJECT obj1, OBJECT obj2, VPTR param);

#define ARefcnt(s)   ((s)->ar_refcnt)
#define ASize(s)     ((s)->ar_size)
#define AMax(s)      ((s)->ar_max)
#define AData(s)     ((s)->ar_data)

void add_array_obj(ARRAY array, OBJECT obj);
ARRAY create_array_objval(INT size);
BOOLEAN delete_array_obj(ARRAY, INT i);
void destroy_array(ARRAY array);
void enlarge_array(ARRAY array, INT space);
OBJECT get_array_obj(ARRAY array, INT i);
INT get_array_size(ARRAY array);
void set_array_obj(ARRAY array, INT i, OBJECT obj);
void sort_array(ARRAY array, OBJCMPFNC cmp, VPTR param);

#endif /* array_h_included */
