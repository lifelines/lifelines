/*!
  \file    bfs.c
  \author  Perry Rapp, 2000
  \date    Created: 2000/10/28

  Implementation of a very simple C language dynamic buffer


LICENSE:
   Copyright (c) 2000-2001 Perry Rapp

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
 * bfs.c -- Simple dynamic string buffer for C
 *   Added to LifeLines: 2000/07 by Perry Rapp
 *==============================================================*/

#include "llstdlib.h"
#include "bfs.h"



void
bfInit(bfptr bfs, int initsize)
{
	bfs->str = 0;
	bfs->size = 0;
	bfs->end = 0;
	bfReserve(bfs, initsize);
}

void
bfTerm(bfptr bfs)
{
	if (bfs->str)
	{
		free(bfs->str);
		bfs->str = 0;
	}
}

bfptr
bfNew(int initsize)
{
	bfptr bfs = stdalloc(sizeof(*bfs));
	bfInit(bfs, initsize);
	return bfs;
}

void
bfDelete(bfptr bfs)
{
	bfTerm(bfs);
	stdfree(bfs);
}

STRING
bfDetachAndKill(bfptr * bfs)
{
	STRING str = (*bfs)->str;
	stdfree(*bfs);
	*bfs = NULL;
	return str;
}

STRING
bfReserve(bfptr bfs, int req)
{
	if (bfs->size >= req+1)
		return bfs->str;
	bfs->size = req+32;
	if (bfs->end == bfs->str)
	{
		if (bfs->str)
			free(bfs->str);
		bfs->str = malloc(bfs->size);
		bfs->end = bfs->str;
		bfs->str[0] = 0;
	}
	else
	{
		int len = bfs->end - bfs->str;
		bfs->str = stdrealloc(bfs->str, bfs->size);
		bfs->end = bfs->str + len;
	}
	return bfs->str;
}

STRING
bfReserveExtra(bfptr bfs, int extra)
{
	int req = (bfs->end - bfs->str) + extra;
	return bfReserve(bfs, req);
}

STRING
bfCpy(bfptr bfs, CNSTRING src)
{
	if (src)
	{
		int len = strlen(src);
		bfReserve(bfs, len);
		strcpy(bfs->str, src);
		bfs->end = bfs->str + len;
	}
	else
	{
		bfReserve(bfs, 1);
		*bfs->str = 0;
	}
	return bfs->str;
}

STRING
bfCat(bfptr bfs, CNSTRING src)
{
	if (src)
	{
		int len = strlen(src);
		bfReserveExtra(bfs, len);
		strcpy(bfs->end, src);
		bfs->end += len;
	}
	return bfs->str;
}

STRING
bfCatnFixed(bfptr bfs, CNSTRING src, int len)
{
	int i;
	bfReserveExtra(bfs, len);
	for (i=0; i<len; i++)
		bfs->end[i] = src[i];
	bfs->end += len;
	return bfs->str;
}

STRING
bfCatNum(bfptr bfs, int num)
{
	char scratch[33];
	/*	itoa(num, scratch, 10); */
	sprintf(scratch, "33%d", num);/* nozell fixed this */
	bfCat(bfs, scratch);
	return bfs->str;
}

int
bfLen(bfptr bfs)
{
	return bfs->end - bfs->str;
}

void
bfMark(bfptr bfs, STRING * mark)
{
	*mark = bfs->end;
}

STRING
bfRestore(bfptr bfs, STRING mark)
{
	bfs->end = mark;
	*mark = 0;
	return bfs->str;
}

STRING
bfSync(bfptr bfs)
{
	bfs->end = bfs->str + strlen(bfs->str);
	return bfs->str;
}

STRING
bfCatChar(bfptr bfs, char c)
{
	if (c)
	{
		bfReserveExtra(bfs, 1);
		bfs->end[0] = c;
		++bfs->end;
		bfs->end[0] = 0;
	}
	return bfs->str;
}

