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

static BOOLEAN initbtree (STRING basedir);

/*============================================
 * init_keyfilex -- Initialize KEYFILEX structure
 *==========================================*/
static void
init_keyfilex(KEYFILEX * kfilex)
{
	strncpy(kfilex->name, KF_NAME, sizeof(kfilex->name));
	kfilex->magic = KF_MAGIC;
	kfilex->version = KF_VER;
}
/*============================================
 * validate_keyfilex -- Is KEYFILEX structure valid ?
 *==========================================*/
BOOLEAN
validate_keyfilex (KEYFILEX * kfilex)
{
	if (strcmp(kfilex->name, KF_NAME))
	{
		bterrno = BTERRILLEGKF;
		return FALSE;
	}
	if (kfilex->magic != KF_MAGIC)
	{
		bterrno = BTERRALIGNKF;
		return FALSE;
	}
	if (kfilex->version != KF_VER)
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
	KEYFILE kfile;
	KEYFILEX kfilex;
	BOOLEAN keyed = FALSE;
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
	}
	if (stat(scratch, &sbuf) || !S_ISREG(sbuf.st_mode)) {
		bterrno = BTERRKFILE;
		return NULL;
	}

/* Open and read key file */
	if (!(fp = fopen(scratch, LLREADBINARYUPDATE)) ||
	    fread(&kfile, sizeof(kfile), 1, fp) != 1) {
		bterrno = BTERRKFILE;
		return NULL;
	}
/* Read & validate keyfilex - if not present, we'll add it below */
	if (fread(&kfilex, sizeof(kfilex), 1, fp) == 1) {
		if (!validate_keyfilex(&kfilex))
			return NULL; /* validate set bterrno */
		keyed=TRUE;
	}
	if (kfile.k_ostat < 0) {
		bterrno = BTERRWRITER;
		fclose(fp);
		return NULL;
	}

/* Update key file for this new opening */
	if (writ && (kfile.k_ostat == 0))
		kfile.k_ostat = -1;
	else
		kfile.k_ostat++;
	rewind(fp);
	if (fwrite(&kfile, sizeof(KEYFILE), 1, fp) != 1) {
		bterrno = BTERRKFILE;
		fclose(fp);
		return NULL;
	}
	if (!keyed) {
		/* add KEYFILEX structure */
		init_keyfilex(&kfilex);
		if (fwrite(&kfilex, sizeof(kfilex), 1, fp) != 1) {
		bterrno = BTERRKFILE;
		fclose(fp);
		}
	}
	fflush(fp);

/* Get master index */
	if (!(master = readindex(dir, kfile.k_mkey)))
		return NULL;	/* bterrno set by getindex */

/* Create new BTREE */
	btree = (BTREE) stdalloc(sizeof *btree);
	bbasedir(btree) = dir;
	bmaster(btree) = master;
	bwrite(btree) = (kfile.k_ostat == -1);
	bkfp(btree) = fp;
	btree->b_kfile.k_mkey = kfile.k_mkey;
	btree->b_kfile.k_fkey = kfile.k_fkey;
	btree->b_kfile.k_ostat = kfile.k_ostat;
	initcache(btree, 20);
	return btree;
}
/*==================================
 * initbtree -- Initialize new BTREE
 *================================*/
static BOOLEAN
initbtree (STRING basedir)
{
	KEYFILE kfile;
	KEYFILEX kfilex;
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
	kfile.k_mkey = path2fkey("aa/aa");
	kfile.k_fkey = path2fkey("ab/ab");
	kfile.k_ostat = 0;
	init_keyfilex(&kfilex);
	if (fwrite(&kfile, sizeof(kfile), 1, fk) != 1
		|| fwrite(&kfilex, sizeof(kfilex), 1, fk) != 1) {
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
	FILE *fp;
	KEYFILE kfile;

	if(btree && ((fp = bkfp(btree)) != NULL)) {
		kfile = btree->b_kfile;
		if (kfile.k_ostat <= 0)
			kfile.k_ostat = 0;
		else { /* read-only, get current shared status */
			rewind(fp);
			if (fread(&kfile, sizeof(KEYFILE), 1, fp) != 1) {
				bterrno = BTERRKFILE;
				fclose(fp);
				return FALSE;
			}
			if (kfile.k_ostat <= 0) { /* someone has seized the DB */
				fclose(fp);
				return TRUE;
			}
			kfile.k_ostat--;
		}
		rewind(fp);
		if (fwrite(&kfile, sizeof(KEYFILE), 1, fp) != 1) {
			bterrno = BTERRKFILE;
			fclose(fp);
			return FALSE;
		}
		fclose(fp);
	}
	return TRUE;
}
