/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * listd.h -- Declare doubly-linked list type
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *===========================================================*/
#ifndef list_h_included
#define list_h_included


/* types for lists */
typedef struct lntag *LNODE;
struct lntag {
	LNODE l_prev;
	LNODE l_next;
	VPTR l_element;
};
#define lprev(n) ((n)->l_prev)
#define lnext(n) ((n)->l_next)
#define lelement(n) ((n)->l_element)

typedef struct ltag {
	LNODE l_head;
	LNODE l_tail;
	INT l_len;
	INT l_type;
	INT l_refcnt;
} *LIST;
#define ltype(l) ((l)->l_type)
#define lhead(l) ((l)->l_head)
#define ltail(l) ((l)->l_tail)
#define llen(l) ((l)->l_len)
#define lrefcnt(l) ((l)->l_refcnt)

#define LISTNOFREE 0
#define LISTDOFREE 1

/* cycle through list from head to tail */
#define FORLIST(l,e)\
	{\
		LNODE _lnode = l->l_head;\
		VPTR e;\
		while (_lnode) {\
			e = _lnode->l_element;
#define ENDLIST\
			_lnode = _lnode->l_next;\
		}\
	}

#define ISNULL(k)	(!k || *k == 0)


#endif /* list_h_included */
