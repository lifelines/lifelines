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
#include "arch.h" /* for S_ISDIR - Perry 2001/01/01 */
#include "llstdlib.h"
#include "liflines.h"
#include "screen.h"
#include "btreei.h"

/*********************************************
 * local function prototypes
 *********************************************/

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
		bterrno = BTERRILLEGKF;
		return FALSE;
	}
	if (kfile2->magic != KF2_MAGIC)
	{
		bterrno = BTERRALIGNKF;
		return FALSE;
	}
	if (kfile2->version != KF2_VER)
	{
		bterrno = BTERRVERKF;
		return FALSE;
	}
	return TRUE;
}
/*============================================
 * openbtree -- Alloc and init BTREE structure
 *==========================================*/
BTREE
openbtree (STRING dir,          /* btree base dir */
           BOOLEAN cflag,       /* create btree if no exist? */
           BOOLEAN writ)        /* requesting write access? */
{
	BTREE btree;
	char scratch[200];
	FILE *fp;
	struct stat sbuf;
	KEYFILE1 kfile1;
	KEYFILE2 kfile2;
	BOOLEAN keyed2 = FALSE;
	INDEX master;

/* See if base directory exists */
	bterrno = 0;
	if (strlen(dir) > 150) {
		bterrno = BTERRLNGDIR;
		return NULL;
	}
	if (access(dir, 0)) {
		sprintf(scratch, "%s/", dir);
		if (!cflag || !mkalldirs(scratch)) {
			bterrno = BTERRNOBTRE;
			return NULL;
		}
	}
	if (stat(dir, &sbuf) || !S_ISDIR(sbuf.st_mode)) {
		bterrno = BTERRNOBTRE;
		return NULL;
	}

/* See if key file exists */
	sprintf(scratch, "%s/key", dir);
	if (stat(scratch, &sbuf)) {
		if (!cflag || !initbtree(dir)) {
			bterrno = BTERRKFILE;
			return NULL;
		}
	} else {
		if (cflag) {
			/* keyfile found - don't create on top of it */
			bterrno = BTERREXISTS;
			return NULL;
		}
	}
	if (stat(scratch, &sbuf) || !S_ISREG(sbuf.st_mode)) {
		bterrno = BTERRKFILE;
		return NULL;
	}

/* Open and read key file (KEYFILE1) */
	if (!(fp = fopen(scratch, LLREADBINARYUPDATE)) ||
	    fread(&kfile1, sizeof(kfile1), 1, fp) != 1) {
		bterrno = BTERRKFILE;
		return NULL;
	}
/* Read & validate KEYFILE2 - if not present, we'll add it below */
	if (fread(&kfile2, sizeof(kfile2), 1, fp) == 1) {
		if (!validate_keyfile2(&kfile2))
			return NULL; /* validate set bterrno */
		keyed2=TRUE;
	}
	if (kfile1.k_ostat < 0) {
		bterrno = BTERRWRITER;
		fclose(fp);
		return NULL;
	}

/* Update key file for this new opening */
	if (writ && (kfile1.k_ostat == 0))
		kfile1.k_ostat = -1;
	else
		kfile1.k_ostat++;
	rewind(fp);
	if (fwrite(&kfile1, sizeof(kfile1), 1, fp) != 1) {
		bterrno = BTERRKFILE;
		fclose(fp);
		return NULL;
	}
	if (!keyed2) {
		/* add KEYFILE2 structure */
		init_keyfile2(&kfile2);
		if (fwrite(&kfile2, sizeof(kfile2), 1, fp) != 1) {
		bterrno = BTERRKFILE;
		fclose(fp);
		}
	}
	fflush(fp);

/* Get master index */
	if (!(master = readindex(dir, kfile1.k_mkey)))
		return NULL;	/* bterrno set by getindex */

/* Create new BTREE */
	btree = (BTREE) stdalloc(sizeof *btree);
	bbasedir(btree) = dir;
	bmaster(btree) = master;
	bwrite(btree) = (kfile1.k_ostat == -1);
	bkfp(btree) = fp;
	btree->b_kfile.k_mkey = kfile1.k_mkey;
	btree->b_kfile.k_fkey = kfile1.k_fkey;
	btree->b_kfile.k_ostat = kfile1.k_ostat;
	initcache(btree, 20);
	return btree;
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
	FILE *fk, *fi, *fd;
	char scratch[200];

/* Open file for writing keyfile */
	sprintf(scratch, "%s/key", basedir);
	if ((fk = fopen(scratch, LLWRITEBINARY)) == NULL) {
		bterrno = BTERRKFILE;
		return FALSE;
	}

/* Open file for writing master index */
	sprintf(scratch, "%s/aa/aa", basedir);
	if (!mkalldirs(scratch) || (fi = fopen(scratch, LLWRITEBINARY)) == NULL) {
		bterrno = BTERRINDEX;
		fclose(fk);
		return FALSE;
	}

/* Open file for writing first data block */
	sprintf(scratch, "%s/ab/aa", basedir);
	if (!mkalldirs(scratch) || (fd = fopen(scratch, LLWRITEBINARY)) == NULL) {
		bterrno = BTERRBLOCK;
		fclose(fk);
		fclose(fi);
		return FALSE;
	}

/* Write key file */
	init_keyfile1(&kfile1);
	init_keyfile2(&kfile2);
	if (fwrite(&kfile1, sizeof(kfile1), 1, fk) != 1
		|| fwrite(&kfile2, sizeof(kfile2), 1, fk) != 1) {
		bterrno = BTERRKFILE;
		fclose(fk);
		fclose(fi);
		fclose(fd);
		return FALSE;
	}
	fclose(fk);

/* Write master index */
	master = (INDEX) stdalloc(BUFLEN);
	ixtype(master) = BTINDEXTYPE;
	ixself(master) = path2fkey("aa/aa");
	ixparent(master) = 0;
	master->ix_nkeys = 0;
	master->ix_fkeys[0] = path2fkey("ab/aa");
	if (fwrite(master, BUFLEN, 1, fi) != 1) {
		bterrno = BTERRINDEX;
		fclose(fi);
		fclose(fd);
		return FALSE;
	}
	fclose(fi);

/* Write first data block */
	block = (BLOCK) stdalloc(BUFLEN);
	ixtype(block) = BTBLOCKTYPE;
	ixself(block) = path2fkey("ab/aa");
	ixparent(block) = 0;
	block->ix_nkeys = 0;
	if (fwrite(block, BUFLEN, 1, fd) != 1) {
		bterrno = BTERRBLOCK;
		fclose(fd);
		return FALSE;
	}
	fclose(fd);
	return TRUE;
}
/*==========================
 * closebtree -- Close BTREE
 *========================*/
BOOLEAN
closebtree (BTREE btree)
{
	FILE *fp=0;
	KEYFILE1 kfile1;
	BOOLEAN ret=FALSE;

	if (btree && ((fp = bkfp(btree)) != NULL)) {
		kfile1 = btree->b_kfile;
		if (kfile1.k_ostat <= 0) {
			/* it ought to be -1, for exactly one writer */
			kfile1.k_ostat = 0;
		} else { /* read-only, get current shared status */
			rewind(fp);
			if (fread(&kfile1, sizeof(kfile1), 1, fp) != 1) {
				bterrno = BTERRKFILE;
				goto exit_closebtree;
			}
			if (kfile1.k_ostat <= 0) { /* someone has seized the DB */
				ret=TRUE;
				goto exit_closebtree;
			}
			kfile1.k_ostat--;
		}
		rewind(fp);
		if (fwrite(&kfile1, sizeof(kfile1), 1, fp) != 1) {
			bterrno = BTERRKFILE;
			goto exit_closebtree;
		}
		ret=TRUE;
	} else {
		ret=TRUE;
	}
exit_closebtree:
	if (fp)
		fclose(fp);
	return ret;
}
