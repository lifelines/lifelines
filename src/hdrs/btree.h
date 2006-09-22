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
Note that this must be the same as MAXKEYWIDTH in gedcom.h
*/
#define RKEYLEN 8
typedef struct {
	char r_rkey[RKEYLEN];
}  RKEY; /*record key*/


typedef INT FKEY; /*file key*/

typedef char *RAWRECORD;

typedef struct {
	FKEY k_mkey;  /* current master key*/
	FKEY k_fkey;  /* current file key*/
	/* ostat: -2=immutable, -1=writer, 0=closed, 1+=reader count */
	INT k_ostat;
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
	STRING  b_basedir;   /* btree base directory */
	INDEX   b_master;    /* master index block */
	FKEY    b_nkey;      /* next index key */
	FILE   *b_kfp;       /* keyfile file pointer */
	KEYFILE1 b_kfile;    /* keyfile contents */
	INT     b_ncache;    /* number of indices in cache */
	INDEX  *b_cache;     /* index cache */
	BOOLEAN b_write;     /* database writeable? */
	BOOLEAN b_immut;     /* database immutable? */
} *BTREE, BTREESTRUCT;
#define bbasedir(b) ((b)->b_basedir)
#define bmaster(b)  ((b)->b_master)
/* #define bnkey(b)    ((b)->b_nkey) */ /* UNUSED */
#define bkfp(b)     ((b)->b_kfp)
#define bkfile(b)   ((b)->b_kfile)
#define bncache(b)  ((b)->b_ncache)
#define bcache(b)   ((b)->b_cache)
#define bwrite(b)   ((b)->b_write)
#define bimmut(b)   ((b)->b_immut)

/*======================================================
 * BLOCK -- Data structure for BTREE record file headers
 *  The constant NORECS above depends on this exact contents:
 * 12=4+2+4+2=sizeof(FKEY)+sizeof(SHORT)+sizeof(FKEY)+sizeof(SHORT)
 * 16=8+4+4=sizeof(RKEY)+sizeof(INT)+sizeof(INT)
 *====================================================*/
typedef struct {
	FKEY   ix_self;             /*fkey of this block*/
	SHORT  ix_type;		/*block/file type*/
	FKEY   ix_parent;           /*parent file's fkey*/
	SHORT  ix_nkeys;            /*num of keys in block*/
	RKEY   ix_rkeys[NORECS];    /*rkeys in block/file*/
	INT    ix_offs[NORECS];     /*offsets for data in file*/
	INT    ix_lens[NORECS];     /*lengths for data in file*/
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
BOOLEAN addtextfile(BTREE, RKEY, CNSTRING file, TRANSLFNC);
RECORD_STATUS write_record_to_file(BTREE btree, RKEY rkey, STRING file);
RECORD_STATUS write_record_to_textfile(BTREE btree, RKEY rkey, STRING file, TRANSLFNC);

/* opnbtree.c */
BOOLEAN closebtree(BTREE);
void describe_dberror(INT dberr, STRING buffer, INT buflen);
BTREE bt_openbtree(STRING dir, BOOLEAN cflag, INT writ, BOOLEAN immut, INT *lldberr);
BOOLEAN validate_keyfile2(KEYFILE2 * kfile2, INT *lldberr);

/* index.c */
void get_index_file(STRING path, BTREE btr, FKEY ikey);
INDEX readindex(BTREE btr, FKEY ikey, BOOLEAN robust);

/* names.c */

/* btrec.c */
BOOLEAN bt_addrecord(BTREE, RKEY, RAWRECORD, INT);
RAWRECORD bt_getrecord(BTREE, const RKEY *, INT*);
BOOLEAN isrecord(BTREE, RKEY);
INT cmpkeys(const RKEY * rk1, const RKEY * rk2);

/* traverse.c */
BOOLEAN traverse_index_blocks(BTREE, INDEX, void *, BOOLEAN (*ifunc)(BTREE, INDEX, void *), BOOLEAN (*dfunc)(BTREE, BLOCK, void *));
void traverse_db_rec_rkeys(BTREE, RKEY lo, RKEY hi, BOOLEAN(*func)(RKEY, STRING data, INT len, void *param), void *param);

/* utils.c */
STRING rkey2str(RKEY);
RKEY   str2rkey(CNSTRING);
STRING fkey2path(FKEY);


enum {
  BTERR_NODB=8            /* no db directory */
, BTERR_DBBLOCKEDBYFILE   /* db directory is file, not directory */
, BTERR_DBCREATEFAILED    /* failed to create db directory */
, BTERR_DBACCESS          /* access error to db directory */
, BTERR_NOKEY             /* no keyfile */
, BTERR_KFILE             /*problem with the key file*/
, BTERR_KFILE_ALTERDB     /*problem with the key file trying to alter a db*/
, BTERR_INDEX             /*problem with an index file*/
, BTERR_MASTER_INDEX      /*problem with master index file*/
, BTERR_BLOCK             /*problem with a data block file*/
, BTERR_LNGDIR            /*base directory name too long*/
, BTERR_WRITER            /*can't open database because writer has it & -w was specified*/
, BTERR_LOCKED            /* error because db was locked */
, BTERR_UNLOCKED          /* error because db was unlocked */
, BTERR_ILLEGKF           /* illegal keyfile */
, BTERR_ALIGNKF           /* wrong alignment key file */
, BTERR_VERKF             /* wrong version key file */
, BTERR_EXISTS            /* previous database found (create was specified) */
, BTERR_READERS           /* db locked by readers (string in custom string) */
, BTERR_BADPROPS          /* new db properties invalid */

};


#define BTINDEXTYPE 1
#define BTBLOCKTYPE 2

#define BTFLGCRT (1<<0)

#endif
