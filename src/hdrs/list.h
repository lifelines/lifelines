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

/* actual list */
struct tag_vtable;
typedef struct tag_list {
	struct tag_vtable * vtable;
	INT l_refcnt;
	LNODE l_head;
	LNODE l_tail;
	INT l_len;
	INT l_type;
} *LIST;
#define lrefcnt(l) ((l)->l_refcnt)
#define ltype(l)   ((l)->l_type)
#define lhead(l)   ((l)->l_head)
#define ltail(l)   ((l)->l_tail)
#define llen(l)    ((l)->l_len)

#define LISTNOFREE 0
#define LISTDOFREE 1

/* list iterator */
struct tag_list_iter {
	LNODE current;
	LIST list;
	INT status; /* 1=forward, 1=reverse, 0=EOF */
};
typedef struct tag_list_iter * LIST_ITER;


/* for caller-defined function to create new values */
typedef VPTR (*LIST_CREATE_VALUE)(LIST);

/* cycle through list from tail to head */
#define FORLIST(l,e)\
	{\
		LNODE _lnode = l->l_tail;\
		VPTR e;\
		while (_lnode) {\
			e = _lnode->l_element;\
			lock_list_node(_lnode);
#define ENDLIST\
			unlock_list_node(_lnode);\
			_lnode = _lnode->l_prev;\
		}\
	}

/* cycle through list from head to tail */
#define FORXLIST(l,e)\
	{\
		LNODE _lnode = l->l_head;\
		VPTR e;\
		while (_lnode) {\
			e = _lnode->l_element;
#define ENDXLIST\
			_lnode = _lnode->l_next;\
		}\
	}


/* list.c */
void back_list(LIST, VPTR);
BOOLEAN begin_list(LIST list, LIST_ITER listit);
BOOLEAN begin_list_rev(LIST list, LIST_ITER listit);
BOOLEAN change_list_ptr(LIST_ITER listit, VPTR newptr);
LIST create_list(void);
BOOLEAN delete_list_element(LIST list, INT index1b, void (*func)(VPTR));
VPTR dequeue_list(LIST);
BOOLEAN is_empty_list(const LIST);
void enqueue_list(LIST, VPTR);
VPTR get_list_element(LIST, INT, LIST_CREATE_VALUE);
INT in_list(LIST, VPTR param, BOOLEAN (*func)(VPTR param, VPTR el));
INT length_list(LIST);
void lock_list_node(LNODE node);
void make_list_empty(LIST);
BOOLEAN next_list_ptr(LIST_ITER listit, VPTR *pptr);
VPTR peek_list_head(LIST);
VPTR pop_list(LIST);
VPTR pop_list_tail(LIST);
void push_list(LIST, VPTR);
void remove_list(LIST, void (*func)(VPTR));
void set_list_element(LIST, INT, VPTR, LIST_CREATE_VALUE);
void set_list_type(LIST, INT);
void unlock_list_node(LNODE node);


#endif /* list_h_included */
