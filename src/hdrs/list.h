/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * list.h -- Declare doubly-linked list type
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *===========================================================*/
#ifndef list_h_included
#define list_h_included

/* node in list */
typedef struct tag_lnode *LNODE;
struct tag_lnode {
	LNODE l_prev;
	LNODE l_next;
	VPTR l_element;
	int l_locks;
};
#define lprev(n)    ((n)->l_prev)
#define lnext(n)    ((n)->l_next)
#define lelement(n) ((n)->l_element)
#define llocks(n)   ((n)->l_locks)

/* a LIST is an OBJECT */
typedef struct tag_list *LIST;

typedef struct tag_list_iter * LIST_ITER;

#define LISTNOFREE 0
#define LISTDOFREE 1

typedef void (*ELEMENT_DESTRUCTOR)(VPTR);



/* for caller-defined function to create new values */
typedef VPTR (*LIST_CREATE_VALUE)(LIST);

/* cycle through list from tail to head */
#define FORLIST(l,e)\
	{\
		LNODE _lnode = trav_list_tail(l);\
		VPTR e;\
		while (_lnode) {\
			e = _lnode->l_element;\
			lock_list_node(_lnode);
#define ENDLIST\
			unlock_list_node(_lnode);\
			_lnode = _lnode->l_prev;\
		}\
	}
#define STOPLIST\
			unlock_list_node(_lnode);\
			_lnode = 0;

/* cycle through list from head to tail */
#define FORXLIST(l,e)\
	{\
		LNODE _lnode = trav_list_head(l);\
		VPTR e;\
		while (_lnode) {\
			e = _lnode->l_element;
#define ENDXLIST\
			_lnode = _lnode->l_next;\
		}\
	}

/* creating and deleting list */
void addref_list(LIST list);
LIST create_list(void);
LIST create_list2(INT whattofree);
LIST create_list3(ELEMENT_DESTRUCTOR func);
void release_list(LIST list);
void destroy_empty_list(LIST list);
void destroy_list(LIST list);

/* working with elements of list */
void back_list(LIST, VPTR);
/*BOOLEAN delete_list_element(LIST list, INT index1b, ELEMENT_DESTRUCTOR func);*/
INT find_delete_list_elements(LIST list, VPTR param, BOOLEAN (*func)(VPTR param, VPTR el), BOOLEAN deleteall);
VPTR dequeue_list(LIST);
void enqueue_list(LIST, VPTR);
VPTR get_list_element(LIST, INT, LIST_CREATE_VALUE);
INT in_list(LIST, VPTR param, BOOLEAN (*func)(VPTR param, VPTR el));
VPTR peek_list_head(LIST);
VPTR pop_list(LIST);
VPTR pop_list_tail(LIST);
void push_list(LIST, VPTR);
void set_list_element(LIST, INT, VPTR, LIST_CREATE_VALUE);

/* working with entire list */
BOOLEAN is_empty_list(const LIST);
INT length_list(LIST);
void make_list_empty(LIST);

/* list iteration */
LIST_ITER begin_list(LIST list);
LIST_ITER begin_list_rev(LIST list);
BOOLEAN change_list_ptr(LIST_ITER listit, VPTR newptr);
void end_list_iter(LIST_ITER * plistit);
BOOLEAN next_list_ptr(LIST_ITER listit, VPTR *pptr);

/* list macro support functions */
void lock_list_node(LNODE node);
LNODE trav_list_head(LIST list);
LNODE trav_list_tail(LIST list);
void unlock_list_node(LNODE node);



#endif /* list_h_included */
