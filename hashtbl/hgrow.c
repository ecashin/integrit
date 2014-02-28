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

/* hgrow.c - support for hashtbl changing size
 */
#include	<config.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
/* support platforms that don't yet conform to C99 */
#if	HAVE_STDINT_H
#include	<stdint.h>
#elif	HAVE_INTTYPES_H
#include	<inttypes.h>
#else
#error No stdint.h or inttypes.h found.
#endif
#include	<errno.h>
#include	"hashtbl.h"
#include	"hgrow.h"
#include	"hhash.h"
#ifdef	ELC_FIND_LEAKS
#include	"leakfind.h"	/* checking for memory leaks */
#endif

#ifndef	HASHTBL_GROW_DELTA
#define	HASHTBL_GROW_DELTA	2	/* on grow, increase by this factor */
#endif

#ifndef	HASHTBL_SHRINK_DELTA
#define	HASHTBL_SHRINK_DELTA	HASHTBL_GROW_DELTA	/* on shrink,
							 * divide capacity
							 * by this factor */
#endif

/* this code for primes is modified a bit from glib's gprimes.c */
static unsigned long hashtbl_closest_prime(unsigned long num)
{
    static const unsigned long	primelist[] = {
      11,      19,      37,      73,      109,      163,      251,
      367,     557,     823,     1237,    1861,     2777,     4177,
      6247,    9371,    14057,   21089,   31627,    47431,    71143,
      106721,  160073,  240101,  360163,  540217,   810343,   1215497,
      1823231, 2734867, 4102283, 6153409, 9230113,  13845163,
    };
    const static size_t		N = sizeof(primelist) / sizeof(primelist[0]);
    int				i;

    for (i = 0; i < N; i++)
      if (primelist[i] > num)
	return primelist[i];

    return primelist[N - 1];
}

void hashtbl_grow(hashtbl_t *h)
{
    hashtbl_resize(h, hashtbl_closest_prime(h->capacity
					    * HASHTBL_GROW_DELTA));
}

void hashtbl_shrink(hashtbl_t *h)
{
    hashtbl_resize(h, hashtbl_closest_prime(h->capacity
					    / HASHTBL_SHRINK_DELTA));
}

void hashtbl_resize(hashtbl_t *h, size_t capacity)
{
    size_t	oldcapacity	 = h->capacity;
    hashnode_t	**oldtbl	 = h->tbl;
    hashnode_t	**newtbl;
    hashnode_t	*np, *oldnext;
    int		i;
    uint32_t	hashval;

#ifdef	DEBUG
    fprintf(stderr,
	    "debug (%s): resizing table: %d --> %d ... ",
	    __FUNCTION__, oldcapacity, capacity);
#endif
    if (! (newtbl = malloc(sizeof(hashnode_t *) * capacity)) )
      return;			/* can't resize now */

    for (i = 0; i < capacity; ++i)
      newtbl[i]	 = NULL;
    h->tbl	 = newtbl;
    h->capacity	 = capacity;

    for (i = 0; i < oldcapacity; ++i) {
      for (np = oldtbl[i]; np; np = oldnext) {
	oldnext		 = np->next;
	hashval		 = hashtbl_cdb_hash(np->key, np->keylen) % capacity;
	np->next	 = newtbl[hashval];
	newtbl[hashval]	 = np;
      }
    }
    free(oldtbl);
#ifdef	DEBUG
    fputs("done.\n", stderr);
#endif
}

