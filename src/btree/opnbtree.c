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

#include <sys/types.h>
#include <sys/stat.h>
#include "standard.h"
#include "btree.h"
#ifdef WIN32
#include <dir.h>
#include <io.h>
#endif

/*============================================
 * openbtree -- Alloc and init BTREE structure
 *==========================================*/
BTREE openbtree (dir, cflag, writ)
STRING dir;	/* btree base dir */
BOOLEAN cflag;	/* create btree if no exist? */
BOOLEAN writ;	/* requesting write access? */
{
	BTREE btree;
	char scratch[200];
	FILE *fp;
	struct stat sbuf;
	KEYFILE kfile;
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
	if (stat(dir, &sbuf) || !sbuf.st_mode&S_IFDIR) {
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
	if (stat(scratch, &sbuf) || !sbuf.st_mode&S_IFREG) {
		bterrno = BTERRKFILE;
		return NULL;
	}

/* Open and read key file */
	if (!(fp = fopen(scratch, LLREADBINARYUPDATE)) ||
	    fread(&kfile, sizeof(KEYFILE), 1, fp) != 1) {
		bterrno = BTERRKFILE;
		return NULL;
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
BOOLEAN initbtree (basedir)
STRING basedir;
{
	KEYFILE kfile;
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
	if (fwrite(&kfile, sizeof(KEYFILE), 1, fk) != 1) {
		bterrno = BTERRKFILE;
		fclose(fk);
		fclose(fi);
		fclose(fd);
		return FALSE;
	}
	fclose(fk);

/* Write master index */
	master = (INDEX) stdalloc(BUFLEN);
	itype(master) = BTINDEXTYPE;
	iself(master) = path2fkey("aa/aa");
	iparent(master) = 0;
	master->i_nkeys = 0;
	master->i_fkeys[0] = path2fkey("ab/aa");
	if (fwrite(master, BUFLEN, 1, fi) != 1) {
		bterrno = BTERRINDEX;
		fclose(fi);
		fclose(fd);
		return FALSE;
	}
	fclose(fi);

/* Write first data block */
	block = (BLOCK) stdalloc(BUFLEN);
	itype(block) = BTBLOCKTYPE;
	iself(block) = path2fkey("ab/aa");
	iparent(block) = 0;
	block->i_nkeys = 0;
	if (fwrite(block, BUFLEN, 1, fd) != 1) {
		bterrno = BTERRBLOCK;
		fclose(fd);
		return FALSE;
	}
	fclose(fd);
	return TRUE;
}
/*=====================================================
 * llmkdir -- Make directory (some UNIXes have a mkdir)
 *===================================================*/
int llmkdir (dir)
STRING dir;	/* dir to create */
{
	static status;
#ifndef WIN32
	register pid;
	if (pid = fork())
		while (wait(&status) != pid);
	else  {
		close(2);
		execl("/bin/mkdir", "mkdir", dir, 0);
		exit(2);
	}
	return status>>8 == 0;
#else
	status = mkdir(dir);
	return status == 0;
#endif
}
/*===================================
 * mkalldirs -- Make all dirs in path
 *=================================*/
#define exists(p)  (!(*p) || access((p),00) == 0)

BOOLEAN mkalldirs (path)
char  *path;	/* path with dirs to be made */
{
	int i, n;
	char *p = path;

	for (i = 0, n = strlen(path); i < n; i++, p++)  {
		if (*p != '/') continue;
		*p = 0;
		if (exists(path) || llmkdir(path))  {
			*p = '/';
			continue;
		}
		llwprintf("Can't create directory %s", path);
		return FALSE;
	}
	return TRUE;
}
/*==========================
 * closebtree -- Close BTREE
 *========================*/
BOOLEAN closebtree (btree)
BTREE btree;
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
