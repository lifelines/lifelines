/* 
   warehouse.h
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

/*==============================================================
 warehouse.h -- Header file for metadata
 Copyright (c) 1999-2001 by Perry Rapp; all rights reserved
  Added during 3.0.6 development
 Created by Perry Rapp, 2000/12/02
 
 A warehouse is made up of three parts:
  header, block list, and block data arena.
 The header is a fixed size WH_HDR.
 The block list is a list of block entries (WB_ENTRY)
  Each entry gives the block type, location, and size.
 The arena holds the data of all the blocks
  (stored in the order of the blocklist).

 All offsets are counted from the beginning of the warehouse.

 Forward compatibility - there may be blocks in the warehouse
  of unknown type
 Backward compatibility - blocks may be missing from the warehouse
 *============================================================*/

#ifndef _WAREHOUSE_H
#define _WAREHOUSE_H

struct WAREHOUSE_S
{
	void * data;
	INT len; /* used length of data */
	INT alloc; /* allocated length of data */
	INT last; /* last block accessed */
	INT lastid;
};

/* block types */
#define BT_MISSING -1
#define BT_VAR 1
#define BT_INT 2

void wh_verify(WAREHOUSE wh);
void wh_allocate(WAREHOUSE wh);
void * wh_find_block_var(WAREHOUSE wh, INT blockid, INT * len);
BOOLEAN wh_find_block_int(WAREHOUSE wh, INT blockid, INT * val);
void wh_replace_block_var(WAREHOUSE wh, INT blockid, void * data, INT len);
void wh_replace_block_int (WAREHOUSE wh, INT blockid, INT val);
void wh_add_block_var(WAREHOUSE wh, INT blockid, void * data, INT len);
void wh_add_block_int (WAREHOUSE wh, INT blockid, INT val);
void wh_delete_block(WAREHOUSE wh, INT blockid);
INT wh_get_blocktype(WAREHOUSE wh, INT blockid);
void wh_assign_from_blob(WAREHOUSE wh, void * data, INT len);

#endif /* _WAREHOUSE_H */
