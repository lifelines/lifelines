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
static BOOLEAN initbtree (STRING basedir);

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
	strncpy(kfile2->name, KF2_NAME, sizeof(kfile2->name));
	kfile2->magic = KF2_MAGIC;
	kfile2->version = KF2_VER;
}
/*============================================
 * validate_keyfile2 -- Is KEYFILE2 structure valid ?
 * 2000/12/08, Perry Rapp
 *==========================================*/
BOOLEAN
validate_keyfile2 (KEYFILE2 * kfile2)
{
	if (strcmp(kfile2->name, KF2_NAME))
	{
		bterrno = BTERR_ILLEGKF;
		return FALSE;
	}
	if (kfile2->magic != KF2_MAGIC)
	{
		bterrno = BTERR_ALIGNKF;
		return FALSE;
	}
	if (kfile2->version != KF2_VER)
	{
		bterrno = BTERR_VERKF;
		return FALSE;
	}
	return TRUE;
}
/*============================================
 * openbtree -- Alloc and init BTREE structure
 *  If it fails, it returns NULL and sets the global bterrno
 *  dir:   [in] btree base dir
 *  cflag: [in] create btree if no exist?
 *  writ:  [in] requesting write access? 1=yes, 2=requiring 
 *  immut: [in,out] user can/will not change anything including keyfile
 *==========================================*/
BTREE
openbtree (STRING dir, BOOLEAN cflag, INT writ, BOOLEAN immut)
{
	BTREE btree;
	char scratch[200];
	FILE *fp=NULL;
	struct stat sbuf;
	KEYFILE1 kfile1;
	KEYFILE2 kfile2;
	BOOLEAN keyed2 = FALSE;
	INDEX master;
	STRING dbmode;

	/* we only allow 150 characters in base directory name */
	bterrno = 0;
	if (strlen(dir) > 150) {
		bterrno = BTERR_LNGDIR;
		goto failopenbtree;
	}
	/* See if base directory exists */
	if (stat(dir, &sbuf)) {
		/* db directory not found */
		if (!cflag || !write || immut) {
			bterrno = BTERR_NODB;
			goto failopenbtree;
		}
		/* create flag set, so try to create it & stat again */
		sprintf(scratch, "%s/", dir);
		if (!mkalldirs(scratch) || stat(dir, &sbuf)) {
			bterrno = BTERR_DBCREATEFAILED;
			goto failopenbtree;
		}
	} else if (!S_ISDIR(sbuf.st_mode)) {
		/* found but not directory */
		bterrno = BTERR_DBBLOCKEDBYFILE;
		goto failopenbtree;
	}
	/* db dir was found, or created & then found */
	if (access(dir, 0)) {
		bterrno = BTERR_DBACCESS;
		goto failopenbtree;
	}

/* See if key file exists */
	sprintf(scratch, "%s/key", dir);
	if (stat(scratch, &sbuf)) {
		/* no keyfile */
		if (!cflag) {
			bterrno = BTERR_NOKEY;
			goto failopenbtree;
		}
		/* create flag set, so try to create it & stat again */
		if (!initbtree(dir) || stat(scratch, &sbuf)) {
			/* initbtree actually set bterrno, but we ignore it */
			bterrno = BTERR_DBCREATEFAILED;
			goto failopenbtree;
		}
	} else {
		if (cflag) {
			/* keyfile found - don't create on top of it */
			bterrno = BTERR_EXISTS;
			goto failopenbtree;
		}
	}
	if (!S_ISREG(sbuf.st_mode)) {
		/* keyfile is a directory! */
		bterrno = BTERR_KFILE;
		goto failopenbtree;
	}

/* Open and read key file (KEYFILE1) */
	dbmode = immut ? LLREADBINARY : LLREADBINARYUPDATE;
	if (!(fp = fopen(scratch, dbmode)) ||
	    fread(&kfile1, sizeof(kfile1), 1, fp) != 1) {
		bterrno = BTERR_KFILE;
		goto failopenbtree; /* validate set bterrno */
	}
/* Read & validate KEYFILE2 - if not present, we'll add it below */
	if (fread(&kfile2, sizeof(kfile2), 1, fp) == 1) {
		if (!validate_keyfile2(&kfile2))
			goto failopenbtree; /* validate set bterrno */
		keyed2=TRUE;
	}
	if (writ < 2 && kfile1.k_ostat == -2)
		immut = TRUE;
	/* if not immutable, handle reader/writer protection update */
	if (!immut) {
		if (kfile1.k_ostat == -1) {
			bterrno = BTERR_WRITER;
			goto failopenbtree;
		}
		if (kfile1.k_ostat == -2) {
			bterrno = BTERR_LOCKED;
			goto failopenbtree;
		}

	/* Update key file for this new opening */
		if (writ>0 && (kfile1.k_ostat == 0))
			kfile1.k_ostat = -1;
		else
			kfile1.k_ostat++;
		rewind(fp);
		if (fwrite(&kfile1, sizeof(kfile1), 1, fp) != 1) {
			bterrno = BTERR_KFILE;
			goto failopenbtree;
		}
		if (!keyed2) {
			/* add KEYFILE2 structure */
			init_keyfile2(&kfile2);
			if (fwrite(&kfile2, sizeof(kfile2), 1, fp) != 1) {
				bterrno = BTERR_KFILE;
				goto failopenbtree;
			}
		}
		fflush(fp);
	}

/* Get master index */
	if (!(master = readindex(dir, kfile1.k_mkey, TRUE)))
		goto failopenbtree; /* bterrno set by readindex */

/* Create new BTREE */
	btree = (BTREE) stdalloc(sizeof *btree);
	bbasedir(btree) = dir;
	bmaster(btree) = master;
	bwrite(btree) = !immut && writ && (kfile1.k_ostat == -1);
	bimmut(btree) = immut; /* includes case that ostat is -2 */
	bkfp(btree) = fp;
	btree->b_kfile.k_mkey = kfile1.k_mkey;
	btree->b_kfile.k_fkey = kfile1.k_fkey;
	btree->b_kfile.k_ostat = kfile1.k_ostat;
	initcache(btree, 20);
	return btree;

failopenbtree:
	if (fp) fclose(fp);
	return NULL;
}
/*==================================
 * initbtree -- Initialize new BTREE
 *================================*/
static BOOLEAN
initbtree (STRING basedir)
{
	KEYFILE1 kfile1;
	KEYFILE2 kfile2;
	INDEX master;
	BLOCK block;
	FILE *fk=NULL, *fi=NULL, *fd=NULL;
	char scratch[200];
	BOOLEAN result=FALSE; /* only set to good at end */

/* Open file for writing keyfile */
	sprintf(scratch, "%s/key", basedir);
	if ((fk = fopen(scratch, LLWRITEBINARY)) == NULL) {
		bterrno = BTERR_KFILE;
		goto initbtree_exit;
	}

/* Open file for writing master index */
	sprintf(scratch, "%s/aa/aa", basedir);
	if (!mkalldirs(scratch) || (fi = fopen(scratch, LLWRITEBINARY)) == NULL) {
		bterrno = BTERR_INDEX;
		goto initbtree_exit;
	}

/* Open file for writing first data block */
	sprintf(scratch, "%s/ab/aa", basedir);
	if (!mkalldirs(scratch) || (fd = fopen(scratch, LLWRITEBINARY)) == NULL) {
		bterrno = BTERR_BLOCK;
		goto initbtree_exit;
	}

/* Write key file */
	init_keyfile1(&kfile1);
	init_keyfile2(&kfile2);
	if (fwrite(&kfile1, sizeof(kfile1), 1, fk) != 1
		|| fwrite(&kfile2, sizeof(kfile2), 1, fk) != 1) {
		bterrno = BTERR_KFILE;
		goto initbtree_exit;
	}
	fclose(fk);
	fk=NULL;

/* Write master index */
	master = (INDEX) stdalloc(BUFLEN);
	ixtype(master) = BTINDEXTYPE;
	ixself(master) = path2fkey("aa/aa");
	ixparent(master) = 0;
	master->ix_nkeys = 0;
	master->ix_fkeys[0] = path2fkey("ab/aa");
	if (fwrite(master, BUFLEN, 1, fi) != 1) {
		bterrno = BTERR_INDEX;
		goto initbtree_exit;
	}
	fclose(fi);
	fi=NULL;

/* Write first data block */
	block = (BLOCK) stdalloc(BUFLEN);
	ixtype(block) = BTBLOCKTYPE;
	ixself(block) = path2fkey("ab/aa");
	ixparent(block) = 0;
	block->ix_nkeys = 0;
	if (fwrite(block, BUFLEN, 1, fd) != 1) {
		bterrno = BTERR_BLOCK;
		goto initbtree_exit;
	}
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
	FILE *fp=0;
	KEYFILE1 kfile1;
	BOOLEAN result=FALSE;

	if (btree && ((fp = bkfp(btree)) != NULL)) {
		kfile1 = btree->b_kfile;
		if (kfile1.k_ostat <= 0) {
			/* writer-locked, should be -1 because we don't
			cater for multiple writers */
			kfile1.k_ostat = 0;
		} else { /* read-only, get current shared status */
			rewind(fp);
			if (fread(&kfile1, sizeof(kfile1), 1, fp) != 1) {
				bterrno = BTERR_KFILE;
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
		rewind(fp);
		if (fwrite(&kfile1, sizeof(kfile1), 1, fp) != 1) {
			bterrno = BTERR_KFILE;
			goto exit_closebtree;
		}
		result=TRUE;
	} else {
		result=TRUE;
	}
exit_closebtree:
	if (fp) fclose(fp);
	return result;
}
