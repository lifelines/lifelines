/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * btree.h -- BTREE database header
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 19 Aug 93
 *   3.0.0 - 04 Oct 94
 *===========================================================*/

#ifndef _BTREE_H
#define _BTREE_H

#include "standard.h"

#define BUFLEN 4096
#define NOENTS ((BUFLEN-12)/12)
#define NORECS ((BUFLEN-12)/16)

typedef struct {
	char r_rkey[8];
}  RKEY; /*record key*/

#define RKEYLEN 8

typedef INT FKEY; /*file key*/

typedef char *RECORD;

typedef struct {
	FKEY k_mkey;	/*current master key*/
	FKEY k_fkey;	/*current file key*/
	INT k_ostat;	/*current open status*/
} KEYFILE;

typedef struct {
	char name[18]; /* KF_NAME */
	INT magic;     /* KF_MAGIC */
	INT version;   /* KF_VER */
} KEYFILEX;

#define KF_NAME "LifeLines Keyfile"
#define KF_MAGIC 0x12345678
#define KF_VER 1

/*==============================================
 * INDEX -- Data structure for BTREE index files
 *============================================*/
typedef struct {
	FKEY  i_self;		/*fkey of index*/
	SHORT i_type;           /*block/file type*/
	FKEY  i_parent;         /*parent file's fkey*/
	SHORT i_nkeys;          /*num of keys in index*/
	RKEY  i_rkeys[NOENTS];  /*rkeys in index*/
	FKEY  i_fkeys[NOENTS];  /*fkeys in index*/
} *INDEX, INDEXSTRUCT;
/*=======================================
 * BTREE -- Internal BTREE data structure
 *=====================================*/
typedef struct {
	STRING  b_basedir;	/*btree base directory*/
	INDEX   b_master;	/*master index block*/
	FKEY    b_nkey;		/*next index key*/
	FILE   *b_kfp;		/*keyfile file pointer*/
	KEYFILE b_kfile;	/*keyfile contents*/
	INT     b_ncache;	/*number of indices in cache*/
	INDEX  *b_cache;	/*index cache*/
	BOOLEAN b_write;	/*database writeable?*/
} *BTREE, BTREESTRUCT;
#define bbasedir(b) ((b)->b_basedir)
#define bmaster(b)  ((b)->b_master)
#define bnkey(b)    ((b)->b_nkey)
#define bkfp(b)     ((b)->b_kfp)
#define bkfile(b)   ((b)->b_kfile)
#define bncache(b)  ((b)->b_ncache)
#define bcache(b)   ((b)->b_cache)
#define bwrite(b)   ((b)->b_write)

/*======================================================
 * BLOCK -- Data structure for BTREE record file headers
 *====================================================*/
typedef struct {
	FKEY   i_self;		/*fkey of this block*/
	SHORT  i_type;		/*block/file type*/
	FKEY   i_parent;	/*parent file's fkey*/
	SHORT  i_nkeys;		/*num of keys in block*/
	RKEY   i_rkeys[NORECS];	/*rkeys in block/file*/
	INT    i_offs[NORECS];	/*offsets for data in file*/
	INT    i_lens[NORECS];	/*lengths for data in file*/
} *BLOCK, BLOCKSTRUCT;

/*============================================
 * Macros for selecting INDEX and BLOCK fields
 *==========================================*/
#define iself(p)    ((p)->i_self)
#define itype(p)    ((p)->i_type)
#define iparent(p)  ((p)->i_parent)
#define nkeys(p)   ((p)->i_nkeys)
#define rkeys(p,i) ((p)->i_rkeys[i])
#define fkeys(p,i) ((p)->i_fkeys[i])
#define offs(p,i)  ((p)->i_offs[i])
#define lens(p,i)  ((p)->i_lens[i])

/*====================================
 * BTREE library function declarations 
 *==================================*/

/* file.c */
BOOLEAN addfile(BTREE, RKEY, STRING);
BOOLEAN getfile(BTREE, RKEY, STRING);

/* opnbtree.c */
BOOLEAN closebtree(BTREE);
BTREE openbtree(STRING, BOOLEAN, BOOLEAN);
BOOLEAN validate_keyfilex(KEYFILEX *);

/* names.c */
RKEY name_hi(void);
RKEY name_lo(void);

/* record.c */
BOOLEAN addrecord(BTREE, RKEY, RECORD, INT);
RECORD getrecord(BTREE, RKEY, INT*);
BOOLEAN isrecord(BTREE, RKEY);
RECORD readrec(BTREE btree, BLOCK block, INT i, INT *plen);

/* traverse.c */
BOOLEAN traverse_index_blocks(BTREE, INDEX, BOOLEAN (*ifunc)(BTREE, INDEX), BOOLEAN (*dfunc)(BTREE, BLOCK));
void traverse_db_rec_rkeys(BTREE, RKEY lo, RKEY hi, BOOLEAN(*func)(RKEY, STRING, INT len, void *param), void *param);

/* utils.c */
STRING rkey2str(RKEY);
RKEY   str2rkey(STRING);
STRING fkey2path(FKEY);

extern INT bterrno;

#define BTERRNOBTRE   1 /*problem with database */
#define BTERRKFILE    2	/*problem with the key file*/
#define BTERRINDEX    3 /*problem with an index file*/
#define BTERRBLOCK    4 /*problem with a data block file*/
#define BTERRLNGDIR   5	/*base directory name too long*/
#define BTERRWRITER   6 /*can't open database because writer has it*/
#define BTERRILLEGKF  7 /* illegal keyfile */
#define BTERRALIGNKF  8 /* wrong alignment key file */
#define BTERRVERKF    9 /* wrong version key file */

#define BTINDEXTYPE 1
#define BTBLOCKTYPE 2

#define BTFLGCRT (1<<0)

#endif
