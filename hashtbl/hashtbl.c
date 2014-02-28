/*
hashtbl - a handy and fast, free hash table implementation
Copyright (C) 2006 Ed L. Cashin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

/* hashtbl keys are always null-terminated arrays of char
 *
 * data are void pointers.  Although hashtbl doesn't "own" the data
 * stored in a hashtbl, the "hashtbl_free" function is provided as
 * a convenience to the user: it frees the nodes in the hashtbl,
 * calling free on each void pointer datum.
 */
#include	<config.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
/* support platforms that don't yet conform to C99 */
#if	HAVE_STDINT_H
#include	<stdint.h>
#elif	HAVE_INTTYPES_H
#include	<inttypes.h>
#else
#error No stdint.h or inttypes.h found.
#endif
#include	"hashtbl.h"
#include	"hgrow.h"
#include	"hhash.h"
#ifdef	ELC_FIND_LEAKS
#include	"leakfind.h"	/* checking for memory leaks */
#endif
/* #define	DEBUG */

#ifndef	HASHTBL_FULLISH
#define	HASHTBL_FULLISH	0.60	/* ((entries / capacity) > this) means
				 * it's time to grow */
#endif

#ifndef	HASHTBL_EMPTYISH
#define	HASHTBL_EMPTYISH	0.05	/* ((entries / capacity) < this) means
					 * it's time to shrink */
#endif

#ifndef	HASHTBL_BIG_CAPACITY
#define	HASHTBL_BIG_CAPACITY	240101	/* if capacity bigger than this,
					 * we may occasionally shrink. */
#endif

#ifndef	HASHTBL_DEFAULT_CAPACITY
#define	HASHTBL_DEFAULT_CAPACITY	109
#endif

/* hashtbl strdup for n-char string */
static char *h_strndup(const char *orig, size_t len)
{
    char	*newstr;

    if ( (newstr = malloc(len + 1)) )
      strcpy(newstr, orig);

    return newstr;
}

int hashtbl_init(hashtbl_t *h, size_t capacity)
{
    int		i;
    hashnode_t	**tbl;

    if (! capacity)
      capacity	 = HASHTBL_DEFAULT_CAPACITY;

    h->capacity	 = capacity;
    h->entries	 = 0;

    if (! (tbl = malloc(sizeof(hashnode_t *) * capacity)) )
      return -1; /* fatal_error("malloc table"); */
    for (i = 0; i < capacity; ++i)
      tbl[i]	 = NULL;
    h->tbl	 = tbl;

    return 0;
}

void hashtbl_destroy(hashtbl_t *h)
{
    free(h->tbl);
}

inline static hashnode_t *hashtbl_find(hashtbl_t *h,
				       const char *key, size_t keylen)
{
    hashnode_t	*np;
    hashnode_t	**tbl	 = h->tbl;
    
    for (np = tbl[hashtbl_cdb_hash(key, keylen) % h->capacity]; np; np = np->next)
      if (! strcmp(key, np->key))
	return np;

    return NULL;
}

void *hashtbl_lookup(hashtbl_t *h, const char *key, size_t keylen)
{
    hashnode_t	*np;

    if (! (np = hashtbl_find(h, key, keylen)) )
      return NULL;
    return np->data;
}

/* hashtbl doesn't do memory management of the stored data.
 * When an entry is replaced by hashtbl_store, it returns the pointer
 * to data that was replaced.
 *
 * The caller can use the returned pointer to free the associated
 * memory appropriately.
 */
int hashtbl_store(hashtbl_t *h,
		  const char *key, size_t keylen,
		  void *data, void **replaced)
{
    hashnode_t	**tbl		 = h->tbl;
    hashnode_t	*np;
    uint32_t	hashval		 = hashtbl_cdb_hash(key, keylen) % h->capacity;

    *replaced	 = NULL;

    for (np = tbl[hashval]; np; np = np->next) {
      if (! strcmp(key, np->key)) { /* replace entry with identical key */
	*replaced	 = np->data;
	np->data	 = data;
	break;
      }
    }
    if (! *replaced) {
      if (! (np = malloc(sizeof(hashnode_t))) )
	return -1; /* fatal_error("malloc hash table node"); */
      if (! (np->key = h_strndup(key, keylen)))
	return -1; /* fatal_error("duplicating hash key"); */
      np->next		 = tbl[hashval];
      tbl[hashval]	 = np;
      np->data		 = data;
      np->keylen	 = keylen;
      ++h->entries;

      if (((float) h->entries / h->capacity) > HASHTBL_FULLISH)
	hashtbl_grow(h);
    }

    return 0;
}

inline void hashtbl_node_destroy(hashnode_t *np)
{
    /* hashtbl allocates memory for its nodes' keys */
    free(np->key);
}

/* hashtbl doesn't do memory management of the stored data.
 * When an entry is deleted by hashtbl_remove, it returns the pointer
 * to data that was removed.
 *
 * The caller can use the returned pointer to free the associated
 * memory appropriately.
 */
void *hashtbl_remove(hashtbl_t *h, const char *key, size_t keylen)
{
    void	*rmdata	 = NULL;
    hashnode_t	**tbl		 = h->tbl;
    hashnode_t	*np, *rmnode;
    uint32_t	hashval		 = hashtbl_cdb_hash(key, keylen) % h->capacity;

    if (! (np = tbl[hashval]) )
      return NULL;

    /* test the entry in the table first; then go through the list
     * if necessary */
    if (! strcmp(np->key, key)) {
      rmdata		 = np->data;
      tbl[hashval]	 = np->next;
      hashtbl_node_destroy(np);
      free(np);
      --h->entries;
    } else {
      for (; (rmnode = np->next); np = np->next) {
	if (! strcmp(key, rmnode->key)) {
	  rmdata	 = rmnode->data;
	  np->next	 = rmnode->next;
	  hashtbl_node_destroy(rmnode);
	  free(rmnode);
	  --h->entries;
	  break;
	}
      }
    }

    if ((h->capacity > HASHTBL_BIG_CAPACITY)
	&& (((float) h->entries / h->capacity) < HASHTBL_EMPTYISH))
      hashtbl_shrink(h);

   return rmdata;
}
