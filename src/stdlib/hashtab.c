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

/*
 hashtab contains a simple hash table implementation
 keys are strings (hash table copies & manages memory itself for keys
 values (void * pointers, they are client's responsibility to free) 
*/

#include "llstdlib.h"
#include "hashtab.h"

/*********************************************
 * local enums & defines
 *********************************************/

#define MAXHASH_DEF 512

/*********************************************
 * local types
 *********************************************/

/* entry in hash table */
struct tag_hashent {
	CNSTRING magic;
	CNSTRING ekey;
	HVALUE val;
	struct tag_hashent *enext;
};
typedef struct tag_hashent *HASHENT;

/* hash table */
struct tag_hashtab {
	CNSTRING magic;
	HASHENT *entries;
	INT count; /* #entries */
	INT maxhash;
};
/* typedef struct tag_hashtab *HASHTAB */ /* in hashtab.h */

/* hash table iterator */
struct tag_hashtab_iter {
	CNSTRING magic;
	HASHTAB hashtab;
	INT index;
	HASHENT enext;
};

/*********************************************
 * local function prototypes
 *********************************************/

static HASHENT create_entry(CNSTRING key, HVALUE val);
static HASHENT fndentry(HASHTAB tab, CNSTRING key);
static INT hash(HASHTAB tab, CNSTRING key);

/*********************************************
 * local variables
 *********************************************/

/* fixed magic strings to verify object identity */
static CNSTRING hashtab_magic = "HASHTAB_MAGIC";
static CNSTRING hashent_magic = "HASHENT_MAGIC";
static CNSTRING hashtab_iter_magic = "HASHTAB_ITER_MAGIC";

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*================================
 * create_hashtab -- Create & return new hash table
 *==============================*/
HASHTAB
create_hashtab (void)
{
	HASHTAB tab = (HASHTAB)stdalloc(sizeof(*tab));
	tab->magic = hashtab_magic;
	tab->maxhash = MAXHASH_DEF;
	tab->entries = (HASHENT *)stdalloc(tab->maxhash * sizeof(HASHENT));
	return tab;
}
/*================================
 * destroy_hashtab -- Destroy hash table
 *==============================*/
void
destroy_hashtab (HASHTAB tab, DELFUNC func)
{
	INT i=0;
	if (!tab) return;
	ASSERT(tab->magic == hashtab_magic);
	for (i=0; i<tab->maxhash; ++i) {
		HASHENT entry = tab->entries[i];
		HASHENT next=0;
		while (entry) {
			ASSERT(entry->magic == hashent_magic);
			next = entry->enext;
			if (func)
				(*func)(entry->val);
			entry->val = 0;
			strfree((STRING *)&entry->ekey);
			stdfree(entry);
			entry = next;
		}
	}
	stdfree(tab->entries);
	tab->entries = 0;
	stdfree(tab);
}
/*======================
 * get_hashtab_count -- Return number of elements currently contained
 *====================*/
INT
get_hashtab_count (HASHTAB tab)
{
	ASSERT(tab);
	ASSERT(tab->magic == hashtab_magic);

	return tab->count;
}
/*======================
 * insert_hashtab -- Add new value to hash table
 * return previous value for this key, if any
 *====================*/
HVALUE
insert_hashtab (HASHTAB tab, CNSTRING key, HVALUE val)
{
	HASHENT entry=0;
	INT hval=0;

	ASSERT(tab);
	ASSERT(tab->magic == hashtab_magic);

	/* find appropriate has chain */
	hval = hash(tab, key);
	if (!tab->entries[hval]) {
		/* table lacks entry for this key, create it */
		entry = create_entry(key, val);
		tab->entries[hval] = entry;
		++tab->count;
		return 0; /* no old value */
	}
	entry = tab->entries[hval];
	while (TRUE) {
		ASSERT(entry->magic == hashent_magic);
		if (eqstr(key, entry->ekey)) {
			/* table already has entry for this key, replace it */
			HVALUE old = entry->val;
			entry->val = val;
			return old;
		}
		if (!entry->enext) {
			/* table lacks entry for this key, create it */
			HASHENT newent = create_entry(key, val);
			entry->enext = newent;
			++tab->count;
			return 0; /* no old value */
		}
		entry = entry->enext;
	}
}
/*======================
 * remove_hashtab -- Remove element from table
 * return old value if found
 *====================*/
HVALUE
remove_hashtab (HASHTAB tab, CNSTRING key)
{
	HVALUE val=0;
	INT hval=0;
	HASHENT preve=0, thise=0;

	ASSERT(tab);
	ASSERT(tab->magic == hashtab_magic);

	hval = hash(tab, key);
	thise = tab->entries[hval];
	while (thise && nestr(key, thise->ekey)) {
		ASSERT(thise->magic == hashent_magic);
		preve = thise;
		thise = thise->enext;
	}
	if (!thise) return 0;
	if (preve)
		preve->enext = thise->enext;
	else
		tab->entries[hval] = thise->enext;

	val = thise->val;
	strfree((STRING *)&thise->ekey);
	thise->val = 0;
	stdfree(thise);
	--tab->count;
	return val;
}
/*======================
 * find_hashtab -- Find and return value
 *  set optional present arg to indicate whether value was found
 *====================*/
HVALUE
find_hashtab (HASHTAB tab, CNSTRING key, BOOLEAN * present)
{
	HASHENT entry=0;

	ASSERT(tab);
	ASSERT(tab->magic == hashtab_magic);

	entry = fndentry(tab, key);
	if (present) *present = !!entry;
	if (!entry) return 0;

	ASSERT(entry->magic == hashent_magic);
	return entry->val;
}
/*======================
 * in_hashtab -- Find and return value
 *====================*/
BOOLEAN
in_hashtab (HASHTAB tab, CNSTRING key)
{
	HASHENT entry=0;

	ASSERT(tab);
	ASSERT(tab->magic == hashtab_magic);

	entry = fndentry(tab, key);
	return (entry != 0);
}
/*================================
 * fndentry -- Find entry in table
 *==============================*/
static HASHENT
fndentry (HASHTAB tab, CNSTRING key)
{
	HASHENT entry=0;
	if (!tab || !key) return NULL;
	entry = tab->entries[hash(tab, key)];
	while (entry) {
		if (eqstr(key, entry->ekey)) return entry;
		entry = entry->enext;
	}
	return NULL;
}
/*======================
 * hash -- Hash function
 *====================*/
static INT
hash (HASHTAB tab, CNSTRING key)
{
	const unsigned char *ckey = (const unsigned char *)key;
	INT hval = 0;
	while (*ckey)
		hval += *ckey++;
	hval %= tab->maxhash;
	ASSERT(hval>=0);
	ASSERT(hval < tab->maxhash);
	return hval;
}
/*================================
 * create_entry -- Create and return new hash entry
 *==============================*/
static HASHENT
create_entry (CNSTRING key, HVALUE val)
{
	HASHENT entry = (HASHENT)stdalloc(sizeof(*entry));
	entry->magic = hashent_magic;
	entry->ekey = strsave(key);
	entry->val = val;
	return entry;
}
/*================================
 * begin_hashtab -- Create new iterator for hash table
 *==============================*/
HASHTAB_ITER
begin_hashtab (HASHTAB tab)
{
	HASHTAB_ITER tabit=0;
	ASSERT(tab);
	ASSERT(tab->magic == hashtab_magic);

	tabit = (HASHTAB_ITER)stdalloc(sizeof(*tabit));
	tabit->magic = hashtab_iter_magic;
	tabit->hashtab = tab;
	/* table iterator starts at index=0, enext=0 */
	/* stdalloc gave us all zero memory */
	return tabit;
}
/*================================
 * next_hashtab -- Advance hash table iterator
 * If not finished, set pointers and return TRUE
 * If no more entries, return FALSE
 *==============================*/
BOOLEAN
next_hashtab (HASHTAB_ITER tabit, CNSTRING *pkey, HVALUE *pval)
{
	HASHTAB tab=0;
	ASSERT(tabit);
	ASSERT(tabit->magic == hashtab_iter_magic);

	if (!tabit->hashtab) return FALSE;
	tab = tabit->hashtab;

	if (tabit->index == -1 || tab->count == 0)
		return FALSE;

	/* continue current hash chain */
	if (tabit->enext) {
		tabit->enext = tabit->enext->enext;
		if (tabit->enext)
			goto returnit;
		++tabit->index;
	}
	/* find next populated hash chain */
	for ( ; tabit->index < tab->maxhash; ++tabit->index) {
			tabit->enext = tab->entries[tabit->index];
			if (tabit->enext)
				goto returnit;
	}
	/* finished (ran out of hash chains) */
	tabit->index = -1;
	tabit->enext = 0;
	return FALSE;

	/* found entry */
returnit:
	*pkey = tabit->enext->ekey;
	*pval = tabit->enext->val;
	return TRUE;

}
/*================================
 * end_hashtab -- Release/destroy hash table iterator
 *==============================*/
void
end_hashtab (HASHTAB_ITER * ptabit)
{
	HASHTAB_ITER tabit = *ptabit;
	ASSERT(tabit);
	ASSERT(tabit->magic == hashtab_iter_magic);

	memset(tabit, 0, sizeof(*tabit));
	stdfree(tabit);
	*ptabit = 0;
}
