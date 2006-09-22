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

/*=============================================================
 * opnbtree.c -- Create and open BTREE database
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 05 Jul 93
 *   2.3.6 - 17 Oct 93    3.0.0 - 04 Oct 94
 *   3.0.2 - 01 Dec 94
 *===========================================================*/
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/* modified 2000-01-20 J.F.Chandler */

#include "sys_inc.h"
#include "llstdlib.h"
#include "arch.h" /* for S_ISDIR - Perry 2001.01.01 */
#include "btreei.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void init_keyfile1(KEYFILE1 * kfile1);
static void init_keyfile2(KEYFILE2 * kfile2);
static BOOLEAN initbtree (STRING basedir, INT *lldberr);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*============================================
 * init_keyfile1 -- Initialize KEYFILE1 structure
 *  for new database
 * 2001/01/24, Perry Rapp
 *==========================================*/
static void
init_keyfile1 (KEYFILE1 * kfile1)
{
	kfile1->k_mkey = path2fkey("aa/aa");
	kfile1->k_fkey = path2fkey("ab/ab");
	kfile1->k_ostat = 0;
}
/*============================================
 * init_keyfile2 -- Initialize KEYFILE2 structure
 *  for new database or old database before KEYFILE2 
 *  was added
 * 2000/12/08, Perry Rapp
 *==========================================*/
static void
init_keyfile2 (KEYFILE2 * kfile2)
{
	memset(kfile2, 0, sizeof(*kfile2));
	strncpy(kfile2->name, KF2_NAME, sizeof(kfile2->name));
	kfile2->magic = KF2_MAGIC;
	kfile2->version = KF2_VER;
}
/*============================================
 * validate_keyfile2 -- Is KEYFILE2 structure valid ?
 * 2000/12/08, Perry Rapp
 *==========================================*/
BOOLEAN
validate_keyfile2 (KEYFILE2 * kfile2, INT *lldberr)
{
	if (strcmp(kfile2->name, KF2_NAME))
	{
		*lldberr = BTERR_ILLEGKF;
		return FALSE;
	}
	if (kfile2->magic != KF2_MAGIC)
	{
		*lldberr = BTERR_ALIGNKF;
		return FALSE;
	}
	if (kfile2->version != KF2_VER)
	{
		*lldberr = BTERR_VERKF;
		return FALSE;
	}
	return TRUE;
}
/*============================================
 * bt_openbtree -- Alloc and init BTREE structure
 *  If it fails, it returns NULL and sets the *lldberr
 *  dir:     [IN]  btree base dir
 *  cflag:   [IN]  create btree if no exist?
 *  writ:    [IN]  requesting write access? 1=yes, 2=requiring 
 *  immut:   [I/O] user can/will not change anything including keyfile
 *  lldberr: [OUT] error code (if returns NULL)
 * If this succeeds, it sets readonly & immutable flags in btree structure
 *  as appropriate (eg, if keyfile couldn't be opened in readwrite mode)
 *==========================================*/
BTREE
bt_openbtree (STRING dir, BOOLEAN cflag, INT writ, BOOLEAN immut, INT *lldberr)
{
	BTREE btree;
	char scratch[200];
	FILE *fk=NULL;
	struct stat sbuf;
	KEYFILE1 kfile1;
	KEYFILE2 kfile2;
	BOOLEAN keyed2 = FALSE;
	STRING dbmode;

	/* we only allow 150 characters in base directory name */
	*lldberr = 0;
	if (strlen(dir) > 150) {
		*lldberr = BTERR_LNGDIR;
		goto failopenbtree;
	}
	/* See if base directory exists */
	if (stat(dir, &sbuf)) {
		/* db directory not found */
		if (!cflag || !writ || immut) {
			*lldberr = BTERR_NODB;
			goto failopenbtree;
		}
		/* create flag set, so try to create it & stat again */
		sprintf(scratch, "%s/", dir);
		if (!mkalldirs(scratch) || stat(dir, &sbuf)) {
			*lldberr = BTERR_DBCREATEFAILED;
			goto failopenbtree;
		}
	} else if (!S_ISDIR(sbuf.st_mode)) {
		/* found but not directory */
		*lldberr = BTERR_DBBLOCKEDBYFILE;
		goto failopenbtree;
	}
	/* db dir was found, or created & then found */
	if (access(dir, 0)) {
		*lldberr = BTERR_DBACCESS;
		goto failopenbtree;
	}

/* See if key file exists */
	sprintf(scratch, "%s/key", dir);
	if (stat(scratch, &sbuf)) {
		/* no keyfile */
		if (!cflag) {
			*lldberr = BTERR_NOKEY;
			goto failopenbtree;
		}
		/* create flag set, so try to create it & stat again */
		if (!initbtree(dir, lldberr) || stat(scratch, &sbuf)) {
			/* initbtree actually set *lldberr, but we ignore it */
			*lldberr = BTERR_DBCREATEFAILED;
			goto failopenbtree;
		}
	} else {
		if (cflag) {
			/* keyfile found - don't create on top of it */
			*lldberr = BTERR_EXISTS;
			goto failopenbtree;
		}
	}
	if (!S_ISREG(sbuf.st_mode)) {
		/* keyfile is a directory! */
		*lldberr = BTERR_KFILE;
		goto failopenbtree;
	}

/* Open and read key file (KEYFILE1) */
immutretry:
	dbmode = immut ? LLREADBINARY : LLREADBINARYUPDATE;
	if (!(fk = fopen(scratch, dbmode))) {
		if (!immut && writ<2) {
			/* maybe it is read-only media */
			immut = TRUE;
			goto immutretry;
		}
		*lldberr = BTERR_KFILE;
		goto failopenbtree;
	}
	if (fread(&kfile1, sizeof(kfile1), 1, fk) != 1) {
		*lldberr = BTERR_KFILE;
		goto failopenbtree;
	}
/* Read & validate KEYFILE2 - if not present, we'll add it below */
	/* see btree.h for explanation of KEYFILE2 */
	if (fread(&kfile2, sizeof(kfile2), 1, fk) == 1) {
		if (!validate_keyfile2(&kfile2, lldberr))
			goto failopenbtree; /* validate set *lldberr */
		keyed2=TRUE;
	}
	if (writ < 2 && kfile1.k_ostat == -2)
		immut = TRUE; /* keyfile contains the flag for immutable access only */
	/* if not immutable, handle reader/writer protection update */
	if (!immut) {
		if (kfile1.k_ostat == -1) {
			*lldberr = BTERR_WRITER;
			goto failopenbtree;
		}
		if (kfile1.k_ostat == -2) {
			*lldberr = BTERR_LOCKED;
			goto failopenbtree;
		}

	/* Update key file for this new opening */
		if (writ>0 && (kfile1.k_ostat == 0))
			kfile1.k_ostat = -1;
		else
			kfile1.k_ostat++;
		rewind(fk);
		if (fwrite(&kfile1, sizeof(kfile1), 1, fk) != 1) {
			*lldberr = BTERR_KFILE;
			goto failopenbtree;
		}
		if (!keyed2) {
			/* add KEYFILE2 structure */
			init_keyfile2(&kfile2);
			if (fwrite(&kfile2, sizeof(kfile2), 1, fk) != 1) {
				*lldberr = BTERR_KFILE;
				goto failopenbtree;
			}
		}
		if (fflush(fk) != 0) {
			*lldberr = BTERR_KFILE;
			goto failopenbtree;
		}
	}

/* Create BTREE structure */
	btree = (BTREE) stdalloc(sizeof *btree);
	bbasedir(btree) = dir;
	bmaster(btree) = readindex(btree, kfile1.k_mkey, TRUE);

	if (!(bmaster(btree)))
	{
		stdfree(btree);
		*lldberr = BTERR_MASTER_INDEX;
		goto failopenbtree;
	}
	
	bwrite(btree) = !immut && writ && (kfile1.k_ostat == -1);
	bimmut(btree) = immut; /* includes case that ostat is -2 */
	bkfp(btree) = fk;
	btree->b_kfile.k_mkey = kfile1.k_mkey;
	btree->b_kfile.k_fkey = kfile1.k_fkey;
	btree->b_kfile.k_ostat = kfile1.k_ostat;
	initcache(btree, 20);
	return btree;

failopenbtree:
	if (fk) fclose(fk);
	return NULL;
}
/*==================================
 * initbtree -- Initialize new BTREE
 *================================*/
static BOOLEAN
initbtree (STRING basedir, INT *lldberr)
{
	KEYFILE1 kfile1;
	KEYFILE2 kfile2;
	INDEX master=0;
	BLOCK block=0;
	FILE *fk=NULL, *fi=NULL, *fd=NULL;
	char scratch[200];
	BOOLEAN result=FALSE; /* only set to good at end */
	INT rtn=0;

/* Open file for writing keyfile */
	sprintf(scratch, "%s/key", basedir);
	if ((fk = fopen(scratch, LLWRITEBINARY)) == NULL) {
		*lldberr = BTERR_KFILE;
		goto initbtree_exit;
	}

/* Open file for writing master index */
	sprintf(scratch, "%s/aa/aa", basedir);
	if (!mkalldirs(scratch) || (fi = fopen(scratch, LLWRITEBINARY)) == NULL) {
		*lldberr = BTERR_INDEX;
		goto initbtree_exit;
	}

/* Open file for writing first data block */
	sprintf(scratch, "%s/ab/aa", basedir);
	if (!mkalldirs(scratch) || (fd = fopen(scratch, LLWRITEBINARY)) == NULL) {
		*lldberr = BTERR_BLOCK;
		goto initbtree_exit;
	}

/* Write key file */
	init_keyfile1(&kfile1);
	init_keyfile2(&kfile2);
	if (fwrite(&kfile1, sizeof(kfile1), 1, fk) != 1
		|| fwrite(&kfile2, sizeof(kfile2), 1, fk) != 1) {
		*lldberr = BTERR_KFILE;
		goto initbtree_exit;
	}
	if (fclose(fk) != 0) {
		fk = NULL;
		*lldberr = BTERR_KFILE;
		goto initbtree_exit;
	}
	fk=NULL;

/* Write master index */
	master = (INDEX) stdalloc(BUFLEN);
	ixtype(master) = BTINDEXTYPE;
	ixself(master) = path2fkey("aa/aa");
	ixparent(master) = 0;
	master->ix_nkeys = 0;
	master->ix_fkeys[0] = path2fkey("ab/aa");
	rtn = fwrite(master, BUFLEN, 1, fi);
	stdfree(master);
	master = 0;
	if (rtn != 1) {
		*lldberr = BTERR_INDEX;
		goto initbtree_exit;
	}
	if (fclose(fi) != 0) {
		fi = NULL;
		*lldberr = BTERR_INDEX;
		goto initbtree_exit;
	}
	fi=NULL;

/* Write first data block */
	block = (BLOCK) stdalloc(BUFLEN);
	ixtype(block) = BTBLOCKTYPE;
	ixself(block) = path2fkey("ab/aa");
	ixparent(block) = 0;
	block->ix_nkeys = 0;
	rtn = fwrite(block, BUFLEN, 1, fd);
	stdfree(block);
	block = 0;
	if (rtn != 1) {
		*lldberr = BTERR_BLOCK;
		goto initbtree_exit;
	}
	if (fclose(fd) != 0) {
		fd = NULL;
		*lldberr = BTERR_BLOCK;
		goto initbtree_exit;
	}
	fd = NULL;

	/* we can finally say everything is ok */
	result=TRUE;

initbtree_exit:
	/* close any open files */
	if (fd) fclose(fd);
	if (fi) fclose(fi);
	if (fk) fclose(fk);
	return result;
}
/*==========================
 * closebtree -- Close BTREE
 *========================*/
BOOLEAN
closebtree (BTREE btree)
{
	FILE *fk=NULL;
	KEYFILE1 kfile1;
	BOOLEAN result=FALSE;
	if (btree && ((fk = bkfp(btree)) != NULL) && !bimmut(btree)) {
		kfile1 = btree->b_kfile;
		if (kfile1.k_ostat <= 0) {
			/* writer-locked, should be -1 because we don't
			cater for multiple writers */
			kfile1.k_ostat = 0;
		} else { /* read-only, get current shared status */
			rewind(fk);
			if (fread(&kfile1, sizeof(kfile1), 1, fk) != 1) {
				/* *lldberr = BTERR_KFILE */
				/* closebtree does not report specific errors */
				goto exit_closebtree;
			}
			if (kfile1.k_ostat <= 0) {
				/* probably someone has forcibly opened it for write,
				making this -1, and may also have since closed it, leaving
				it at 0 */
				result=TRUE;
				goto exit_closebtree;
			}
			kfile1.k_ostat--;
		}
		rewind(fk);
		if (fwrite(&kfile1, sizeof(kfile1), 1, fk) != 1) {
			/* *lldberr = BTERR_KFILE */
			/* closebtree does not report specific errors */
			goto exit_closebtree;
		}
		if (fclose(fk) != 0) {
			fk = NULL;
			/* *lldberr = BTERR_KFILE; */
			goto exit_closebtree;
		}
		fk = NULL;

		result=TRUE;
	} else {
		result=TRUE;
	}
exit_closebtree:
	if (fk) fclose(fk);
	if (btree) {
		freecache(btree);
		if(bmaster(btree)) {
			stdfree(bmaster(btree));
		}
		stdfree(btree);
	}
	return result;
}
