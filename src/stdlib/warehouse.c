/* 
   warehouse.c
   Copyright (c) 2000 Perry Rapp
   Created: 2000/12/02 for LifeLines & Ethel programs

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include "llstdlib.h"
#include "warehouse.h"

typedef struct
{
	INT magic;
	INT version;
	INT count; /* number of wareblocks */
	INT list; /* offset to beginning of wareblock list */
} WH_HDR;

/* wareblock entry in block list */
/* remember that offset actually holds value for BT_INT */
typedef struct
{
	INT blockid; /* any number */
	INT blocktype; /* BT_VAR or BT_INT */
	INT offset; /* All offsets are from beginning of warehouse */
	INT len;
} WB_ENTRY;

#define ENTSIZE (sizeof(WB_ENTRY))
#define WH_MAGIC 0x7552
#define WH_VERSION 0x1


/*============================================
 * wh_verify - Check warehouse header
 * Created: 2000/12, Perry Rapp
 *==========================================*/
void
wh_verify(WAREHOUSE wh)
{
	WH_HDR * hdr = (WH_HDR *)wh->data;
	ASSERT(hdr->magic == WH_MAGIC);
	if (hdr->version != WH_VERSION)
		FATAL(); /* no forwards or backwards compatibility right now */
	/* Could verify size by figuring last block offset & len */
}
/*============================================
 * find_block - search for block in warehouse
 *  return existence, and set position in argument (if not null)
 * Created: 2000/12, Perry Rapp
 *==========================================*/
static BOOLEAN
find_block(WAREHOUSE wh, INT blockid, int * position)
{
	WH_HDR * hdr = (WH_HDR *)wh->data;
	char * whptr = (char *)hdr;
	WB_ENTRY * wbent = (WB_ENTRY *)(whptr+hdr->list);
	int i, lo, hi;
	lo=0; hi=hdr->count-1;

	/* check warehouse's last cache */
	if (wh->lastid == blockid && blockid == wbent[wh->last].blockid)
		hi = lo = wh->last;

	while (lo<=hi)
	{
		i=(lo+hi)/2;
		if (blockid > wbent[i].blockid)
		{
			lo=i+1;
			continue;
		}
		if (blockid < wbent[i].blockid)
		{
			hi=i-1;
			continue;
		}
		/* found it */
		if (position)
			*position = i;
		wh->last = i;
		wh->lastid = blockid;
		return TRUE;
	}
	if (position)
		*position = lo;
	return FALSE;
}
/*===========================================
 * wh_find_block_var - Lookup variable length
 *  block in warehouse
 *  return pointer to it if found, else NULL
 * Created: 2000/12, Perry Rapp
 *=========================================*/
void *
wh_find_block_var (WAREHOUSE wh, INT blockid, INT * len)
{
	WH_HDR * hdr = (WH_HDR *)wh->data;
	char * whptr = (char *)hdr;
	int i, offset;
	WB_ENTRY * wbent = (WB_ENTRY *)(whptr+hdr->list);
	if (!find_block(wh, blockid, &i))
		return NULL;
	ASSERT(wbent[i].blocktype == BT_VAR);
	/* found it */
	offset = wbent[i].offset;
	if (len)
		*len = wbent[i].len;
	return &whptr[offset];
}
/*===========================================
 * wh_find_block_int - Lookup INT block in warehouse
 *  return TRUE if found (& set val)
 * Created: 2000/12, Perry Rapp
 *=========================================*/
BOOLEAN
wh_find_block_int (WAREHOUSE wh, INT blockid, INT * val)
{
	WH_HDR * hdr = (WH_HDR *)wh->data;
	char * whptr = (char *)hdr;
	int i;
	WB_ENTRY * wbent = (WB_ENTRY *)(whptr+hdr->list);
	if (!find_block(wh, blockid, &i))
		return FALSE;
	ASSERT(wbent[i].blocktype == BT_INT);
	/* found it */
	if (val) /* ints stored in offset */
		*val = wbent[i].offset;
	return TRUE;
}
/*============================================
 * wh_resizing - Ensure warehouse is at least a certain size
 *  this is total size: header + blocklist + arena
 * Created: 2000/12, Perry Rapp
 *==========================================*/
static void
wh_resizing (WAREHOUSE wh, INT newsize)
{
	void * data;
	if (newsize <= wh->alloc)
		return;
	/* TO DO - scale up newsize when finished testing */
	data = stdalloc(newsize);
	memcpy(data, wh->data, wh->len);
	wh->alloc = newsize;
	stdfree(wh->data);
	wh->data = data;
}
/*=================================================
 * replace_block_var - Replace variable-sized block
 *  (find has already located block)
 * before:
 * hdr, preceding blocklist, succeeding blocklist,
 *           preceding blocks, succeeding blocks
 * after:
 * hdr, preceding blocklist, succeeding blocklist
 *          preceding blocks, NEW BLOCK, succeeding blocks
 * Created: 2000/12, Perry Rapp
 *===============================================*/
static void
replace_block_var (WAREHOUSE wh, INT blockid, INT position, void * data, INT len)
{
	WH_HDR * hdr = (WH_HDR *)wh->data;
	char * whptr = (char *)hdr;
	WB_ENTRY * wbent = (WB_ENTRY *)(whptr+hdr->list);
	/* ptr is correct position of new block in original warehouse */
	STRING ptr = whptr + wbent[position].offset;
	INT newsize, delta, i;

	ASSERT(wbent[position].blocktype == BT_VAR);

	/* check for inplace change */
	delta = len - wbent[position].len;
	if (!delta)
	{
		memcpy(ptr, data, len);
		return;
	}
	/* ensure sufficient space */
	newsize = wh->len + delta;
	wh_resizing(wh, newsize);

	/* reload pointers in case warehouse reallocated */
	hdr = (WH_HDR *)wh->data;
	whptr = (char *)hdr;
	wbent = (WB_ENTRY *)(whptr+hdr->list);
	ptr = whptr + wbent[position].offset;
	
	if (position+1 < hdr->count)
	{
		INT nextoffset=0;
		/* shift down following blocks in arena */
		/* not position, because it is being replaced */
		for (i=position+1; i<hdr->count; i++) {
			if (wbent[i].blocktype == BT_VAR) {
				wbent[i].offset += delta;
				if (!nextoffset)
					nextoffset = wbent[i].offset;
			}
		}
		if (nextoffset) /* there following variable-sized blocks */
			memmove(ptr+delta, ptr,	wh->len - nextoffset);
	}

	/* insert new block */
	memmove(ptr, data, len);

	/* adjust block entry */
	wbent[position].len = len;

	/* adjust warehouse size */
	wh->len += delta;
}
/*======================================
 * replace_block_int - Replace int block
 *  (find has already located block)
 * Created: 2000/12, Perry Rapp
 *====================================*/
static void
replace_block_int (WAREHOUSE wh, INT blockid, INT position, INT val)
{
	WH_HDR * hdr = (WH_HDR *)wh->data;
	char * whptr = (char *)hdr;
	WB_ENTRY * wbent = (WB_ENTRY *)(whptr+hdr->list);

	wbent[position].offset = val;
}
/*======================================================
 * add_block_var - Add variable-sized block to warehouse
 *  (find has already verified block does not exist)
 * before:
 * hdr, preceding blocklist, succeeding blocklist,
 *          preceding blocks, succeeding blocks
 * after:
 * hdr, preceding blocklist, NEW BLOCKENTRY, succeeding blocklist
 *          preceding blocks, NEW BLOCK, succeeding blocks
 * Created: 2000/12, Perry Rapp
 *=====================================================*/
static void
add_block_var (WAREHOUSE wh, INT blockid, int position, void * data, INT len)
{
	WH_HDR * hdr;
	char * whptr;
	WB_ENTRY * wbent;
	/* ptr is correct position of new block in original warehouse */
	STRING ptr;
	STRING p1;
	INT newsize, i, nextoffset;

	/* ensure sufficient space */
	newsize = wh->len + ENTSIZE +len;
	wh_resizing(wh, newsize);

	/* reload pointers in case warehouse reallocated */
	hdr = (WH_HDR *)wh->data;
	whptr = (char *)hdr;
	wbent = (WB_ENTRY *)(whptr+hdr->list);

	/* shift down following blocks in arena */
	/* including position, because the new entry is before it */
	/* remember we just changed wbent[i].offset */
	/* we have to scan for variable-sized blocks */
	ptr=0;
	for (i=position; i<hdr->count; i++) {
		if (wbent[i].blocktype == BT_VAR) {
			if (!ptr) {
				/* store ptr & nextoffset before changing offset ! */
				ptr = whptr + wbent[i].offset;
				nextoffset = wbent[i].offset;
			}
			wbent[i].offset += len+ENTSIZE;
		}
	}
	if (ptr) /* ptr & nextoffset are before adjustment */
		memmove(ptr+len+ENTSIZE, ptr, wh->len - nextoffset);
	else /* insert at end */
		ptr = whptr + wh->len;

	/* insert new block */
	memmove(ptr+ENTSIZE, data, len);

	if (position)
	{
		/* shift down any preceding variable-sized blocks in arena */
		for (i=0; i<position; i++) {
			if (wbent[i].blocktype == BT_VAR)
				wbent[i].offset += ENTSIZE;
		}
		p1 = whptr + hdr->count * ENTSIZE;
		i = ptr-p1; /* difference between pointers to old positions */
		if (i)
			memmove(p1+ENTSIZE, p1, i);
	}

	if (position < hdr->count)
	{
		/* shift down following blocklist */
		p1 = (void *)&wbent[position];
		memmove(p1+ENTSIZE, p1, (hdr->count-position)*ENTSIZE);
	}

	/* fill in new block entry */
	wbent[position].blockid = blockid;
	wbent[position].blocktype = BT_VAR;
	wbent[position].offset = (ptr+ENTSIZE)-whptr;
	wbent[position].len = len;

	/* increase block count */
	hdr->count++;

	/* increase warehouse size */
	wh->len += ENTSIZE +len;
}
/*=========================================================
 * add_block_int - Add int block to warehouse
 *  (find has already verified block does not exist)
 * before:
 * hdr, preceding blocklist, succeeding blocklist,
 *          blocks
 * after:
 * hdr, preceding blocklist, NEW BLOCKENTRY, succeeding blocklist
 *          blocks
 * Created: 2000/12, Perry Rapp
 *========================================================*/
static void
add_block_int (WAREHOUSE wh, INT blockid, int position, INT val)
{
	WH_HDR * hdr;
	char * whptr;
	WB_ENTRY * wbent;
	STRING ptr;
	int newsize, i;

	/* ensure sufficient space */
	newsize = wh->len + ENTSIZE;
	wh_resizing(wh, newsize);

	/* reload pointers in case warehouse reallocated */
	hdr = (WH_HDR *)wh->data;
	whptr = (char *)hdr;
	wbent = (WB_ENTRY *)(whptr+hdr->list);

	/* update following blocklist */
	for (i=position; i<hdr->count; i++) {
		if (wbent[i].blocktype == BT_VAR)
			wbent[i].offset += ENTSIZE;
	}

	/* shift down everything from following blocklist on */
	ptr = (void *)&wbent[position];
	memmove(ptr+ENTSIZE, ptr, wh->len - (ptr - whptr));

	/* fill in new block entry */
	wbent[position].blockid = blockid;
	wbent[position].blocktype = BT_INT;
	wbent[position].offset = val;
	wbent[position].len = 0;

	/* increase block count */
	hdr->count++;

	/* increase warehouse size */
	wh->len += ENTSIZE;
}
/*============================================
 * wh_allocate - Create a new warehouse
 * Created: 2000/12, Perry Rapp
 *==========================================*/
void
wh_allocate (WAREHOUSE wh)
{
	WH_HDR * hdr;
	INT len = sizeof(*hdr);
	hdr = stdalloc(len);
	wh->data = hdr;
	wh->len = len;
	wh->last = -1;
	wh->lastid = -1;
	hdr->magic = WH_MAGIC;
	hdr->version = WH_VERSION;
	hdr->count = 0;
	hdr->list = len;
}
/*===============================================
 * wh_assign_from_blob - Load warehouse from blob
 *  of data (eg, read from a file)
 * Created: 2000/12, Perry Rapp
 *=============================================*/
void
wh_assign_from_blob (WAREHOUSE wh, void * data, INT len)
{
	ASSERT(!wh->data);
	wh->data = data;
	wh->len = len;
	wh->alloc = len;
	wh->last = -1;
	wh->lastid = -1;
	wh_verify(wh);
}
/*============================================
 * wh_free - Free data inside warehouse
 * Created: 2001/02/04, Perry Rapp
 *==========================================*/
void
wh_free (WAREHOUSE wh)
{
	ASSERT(wh);
	if (wh->data) {
		stdfree(wh->data);
		wh->data = 0;
	}
}
/*====================================================
 * wh_replace_block_var - Replace variable-sized block
 *  in warehouse with new data
 * Created: 2000/12, Perry Rapp
 *==================================================*/
void
wh_replace_block_var (WAREHOUSE wh, INT blockid, void * data, INT len)
{
	/* same, as we first test for whether it is really present or not */
	wh_add_block_var(wh, blockid, data, len);
}
/*======================================================
 * wh_replace_block_int - Replace int block in warehouse
 *  with new data
 * Created: 2000/12, Perry Rapp
 *====================================================*/
void
wh_replace_block_int (WAREHOUSE wh, INT blockid, INT val)
{
	/* same, as we first test for whether it is really present or not */
	wh_add_block_int(wh, blockid, val);
}
/*=========================================================
 * wh_add_block_var - Add variable-sized block to warehouse
 * Created: 2000/12, Perry Rapp
 *=======================================================*/
void
wh_add_block_var (WAREHOUSE wh, INT blockid, void * data, INT len)
{
	int i;
	/* find position & existence for new block */
	if (find_block(wh, blockid, &i))
		replace_block_var(wh, blockid, i, data, len);
	else
		add_block_var(wh, blockid, i, data, len);
}
/*==============================================
 * wh_add_block_int - Add int block to warehouse
 * Created: 2000/12, Perry Rapp
 *============================================*/
void
wh_add_block_int (WAREHOUSE wh, INT blockid, INT val)
{
	int i;
	/* find position & existence for new block */
	if (find_block(wh, blockid, &i))
		replace_block_int(wh, blockid, i, val);
	else
		add_block_int(wh, blockid, i, val);
}
/*============================================
 * wh_delete_block - Add block to warehouse
 * before:
 * hdr, preceding blocklist, BLOCK ENTRY, succeeding blocklist,
 *           preceding blocks, BLOCK, succeeding blocks
 * after:
 * hdr, preceding blocklist, succeeding blocklist
 *          preceding blocks, succeeding blocks
 * Created: 2000/12, Perry Rapp
 *==========================================*/
void
wh_delete_block (WAREHOUSE wh, INT blockid)
{
	WH_HDR * hdr = (WH_HDR *)wh->data;
	char * whptr = (char *)hdr;
	WB_ENTRY * wbent = (WB_ENTRY *)(whptr+hdr->list);
	/* ptr is position of block in original warehouse */
	STRING ptr;
	STRING p1;
	int position, newsize, len;
	int i;

	if (!find_block(wh, blockid, &position))
		return;

	len = wbent[position].len;
	/* just because add & replace do it */
	newsize = wh->len - (ENTSIZE +len);
	wh_resizing(wh, newsize);

	/* reload pointers in case warehouse reallocated */
	hdr = (WH_HDR *)wh->data;
	whptr = (char *)hdr;
	wbent = (WB_ENTRY *)(whptr+hdr->list);
	ptr = whptr + wbent[position].offset;

	if (position+1 < hdr->count)
	{
		/* shift up succeeding blocklist */
		for (i=position+1; i<hdr->count; i++)
			wbent[i].offset -= len+ENTSIZE;
		p1 = (void *)&wbent[position+1];
		memmove(p1-ENTSIZE, p1, (hdr->count-(position+1))*ENTSIZE);
	}

	if (position)
	{
		/* shift up preceding blocks */
		for(i=0; i<position; i++)
			wbent[i].offset -= ENTSIZE;
		p1 = whptr + hdr->count * ENTSIZE;
		memmove(p1-ENTSIZE, p1, (position-i)*ENTSIZE);
	}

	if (position+1 < hdr->count)
	{
		/* shift up succeeding blocks */
		memmove((ptr+len)-ENTSIZE-len, ptr+len,
			wh->len - (wbent[position+1].offset +(len+ENTSIZE)));
	}

	/* decrease block count */
	hdr->count--;

	/* decrease warehouse size */
	wh->len -= ENTSIZE +len;
}
/*==============================================
 * wh_get_blocktype - Lookup block & report type
 * Created: 2000/12, Perry Rapp
 *============================================*/
INT
wh_get_blocktype (WAREHOUSE wh, INT blockid)
{
	int i;
	WH_HDR * hdr = (WH_HDR *)wh->data;
	char * whptr = (char *)hdr;
	WB_ENTRY * wbent = (WB_ENTRY *)(whptr+hdr->list);

	/* find position & existence for new block */
	if (find_block(wh, blockid, &i))
		return wbent[i].blocktype;
	else
		return BT_MISSING;
}
