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
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 19 Aug 93
 *   3.0.0 - 04 Oct 94
 *===========================================================*/

#ifndef _BTREE_H
#define _BTREE_H

#include "standard.h"

#define BUFLEN 4096
/* see comment at declaration of INDEX below for explanation */
#define NOENTS ((BUFLEN-12)/12)
/* see comment at declaration of BLOCK below for explanation */
#define NORECS ((BUFLEN-12)/16)

/*
All records in a LifeLines btree are indexed on 8 character keys
*/
typedef struct {
	char r_rkey[8];
}  RKEY; /*record key*/

/* This must be sizeof(((RKEY*)(0))->r_rkey) */
#define RKEYLEN 8

typedef INT FKEY; /*file key*/

typedef char *RECORD;

typedef struct {
	FKEY k_mkey;	/*current master key*/
	FKEY k_fkey;	/*current file key*/
	INT k_ostat;	/*current open status*/
} KEYFILE1;

/*
Additional data added to keyfile by Perry in winter of 2000-2001
in order to trap attempt to open a non-keyfile, or an incorrect
version, or a database from a differing byte alignment. KEYFILE2
occurs directly after KEYFILE1, and the program will silently
add it to any database that does not yet have it.
*/
typedef struct {
	char name[18]; /* KF_NAME */
	INT magic;     /* KF_MAGIC */ /* byte alignment check */
	INT version;   /* KF_VER */
} KEYFILE2;

#define KF2_NAME "LifeLines Keyfile"
#define KF2_MAGIC 0x12345678
#define KF2_VER 1

/*==============================================
 * INDEX -- Data structure for BTREE index files
 *  The constant NOENTS above depends on this exact contents:
 * 12=4+2+4+2=sizeof(FKEY)+sizeof(SHORT)+sizeof(FKEY)+sizeof(SHORT)
 * 12=8+4=sizeof(RKEY)+sizeof(FKEY)
 *============================================*/
typedef struct {
	FKEY  ix_self;		/*fkey of index*/
	SHORT ix_type;           /*block/file type*/
	FKEY  ix_parent;         /*parent file's fkey*/
	SHORT ix_nkeys;          /*num of keys in index*/
	RKEY  ix_rkeys[NOENTS];  /*rkeys in index*/
	FKEY  ix_fkeys[NOENTS];  /*fkeys in index*/
} *INDEX, INDEXSTRUCT;
/*=======================================
 * BTREE -- Internal BTREE data structure
 *=====================================*/
typedef struct {
	STRING  b_basedir;	/*btree base directory*/
	INDEX   b_master;	/*master index block*/
	FKEY    b_nkey;		/*next index key*/
	FILE   *b_kfp;		/*keyfile file pointer*/
	KEYFILE1 b_kfile;	/*keyfile contents*/
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
 *  The constant NORECS above depends on this exact contents:
 * 12=4+2+4+2=sizeof(FKEY)+sizeof(SHORT)+sizeof(FKEY)+sizeof(SHORT)
 * 16=8+4+4=sizeof(RKEY)+sizeof(INT)+sizeof(INT)
 *====================================================*/
typedef struct {
	FKEY   ix_self;		/*fkey of this block*/
	SHORT  ix_type;		/*block/file type*/
	FKEY   ix_parent;	/*parent file's fkey*/
	SHORT  ix_nkeys;		/*num of keys in block*/
	RKEY   ix_rkeys[NORECS];	/*rkeys in block/file*/
	INT    ix_offs[NORECS];	/*offsets for data in file*/
	INT    ix_lens[NORECS];	/*lengths for data in file*/
} *BLOCK, BLOCKSTRUCT;

/*============================================
 * Macros for selecting INDEX and BLOCK fields
 *==========================================*/
#define ixself(p)    ((p)->ix_self)
#define ixtype(p)    ((p)->ix_type)
#define ixparent(p)  ((p)->ix_parent)
#define nkeys(p)   ((p)->ix_nkeys)
#define rkeys(p,i) ((p)->ix_rkeys[i])
#define fkeys(p,i) ((p)->ix_fkeys[i])
#define offs(p,i)  ((p)->ix_offs[i])
#define lens(p,i)  ((p)->ix_lens[i])

/*====================================
 * BTREE library function declarations 
 *==================================*/

/* file.c */
BOOLEAN addfile(BTREE, RKEY, STRING file);
BOOLEAN addtextfile(BTREE, RKEY, STRING file, TRANSLFNC);
RECORD_STATUS write_record_to_file(BTREE btree, RKEY rkey, STRING file);
RECORD_STATUS write_record_to_textfile(BTREE btree, RKEY rkey, STRING file, TRANSLFNC);

/* opnbtree.c */
BOOLEAN closebtree(BTREE);
BOOLEAN create_database(STRING dbrequested, STRING dbused);
void describe_dberror(INT dberr, STRING buffer, INT buflen);
int open_database(BOOLEAN forceopen, STRING dbpath, STRING dbactual);
BTREE openbtree(STRING, BOOLEAN, BOOLEAN);
BOOLEAN validate_keyfile2(KEYFILE2 *);

/* names.c */

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

enum {
  BTERR_NODB=8            /* no db directory */
, BTERR_DBBLOCKEDBYFILE   /* db directory is file, not directory */
, BTERR_DBCREATEFAILED    /* failed to create db directory */
, BTERR_DBACCESS          /* access error to db directory */
, BTERR_NOKEY             /* no keyfile */
, BTERR_KFILE             /*problem with the key file*/
, BTERR_INDEX             /*problem with an index file*/
, BTERR_BLOCK             /*problem with a data block file*/
, BTERR_LNGDIR            /*base directory name too long*/
, BTERR_WRITER            /*can't open database because writer has it*/
, BTERR_ILLEGKF           /* illegal keyfile */
, BTERR_ALIGNKF           /* wrong alignment key file */
, BTERR_VERKF             /* wrong version key file */
, BTERR_EXISTS            /* previous database found (create was specified) */
, BTERR_READERS           /* db locked by readers (string in custom string) */

};


#define BTINDEXTYPE 1
#define BTBLOCKTYPE 2

#define BTFLGCRT (1<<0)

#endif
