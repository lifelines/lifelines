/*!
  \file    bfs.h
  \author  Perry Rapp, 2000
  \date    Created: 2000/10/28

  Simple string buffer
  client must typedef STRING & CNSTRING before including this

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

	LifeLines version created 2001/07/19
 */

#ifndef bfs_h
#define bfs_h

typedef struct Buffer_s
{
	STRING str;
	STRING end;
	int size;
} Buffer;

typedef struct Buffer_s * bfptr;

/* bfInit & bfTerm may be used with stack buffers */
/*** Initialize buffer (eg, stack variable) */
void bfInit(bfptr bfs, int initsize);
/*** Release buffer memory (complement of bfInit) */
void bfTerm(bfptr bfs);

/** Allocate buffer on heap, initialize, & return */
bfptr bfNew(int initsize);
/** Destroy & free buffer from heap (complement of bfNew) */
void bfDelete(bfptr bfs);
/** Extract buffer string & delete buffer wrapper (bfs is wiped) */
STRING bfDetachAndKill(bfptr * bfs);

/*** ensure sufficient space in buffer */
STRING bfReserve(bfptr bfs, int size);

/*** ensure sufficient space after end of current string */
STRING bfReserveExtra(bfptr bfs, int extra);

/*** copy string into buffer, reallocating if necessary */
STRING bfCpy(bfptr bfs, CNSTRING src);

/*** append string to buffer, reallocating if necessary */
STRING bfCat(bfptr bfs, CNSTRING src);

/*** append char to buffer, reallocating if necessary */
STRING bfCatChar(bfptr bfs, char c);

/*** append non-zero-terminated string to buffer, reallocating if necessary */
STRING bfCatnFixed(bfptr bfs, CNSTRING src, int len);

/*** append string to buffer, reallocating if necessary */
STRING bfCatNum(bfptr bfs, int num);

/*** return length of buffer (not the alloc size) */
int bfLen(bfptr bfs);

/*** save current state of buffer */
void bfMark(bfptr bfs, STRING * mark);

/*** restore buffer back to previous state */
STRING bfRestore(bfptr bfs, STRING mark);

/*** recompute end pointer, after external messing about */
STRING bfSync(bfptr bfs);

/** access underlying string */
#define bfStr(bfs) (bfs->str)

#endif /* bfs_h */
