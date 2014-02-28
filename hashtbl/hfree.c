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

/* hfree.c - support for freeing hashtbl's contents
 */
#include	<config.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	"hashtbl.h"
#ifdef	ELC_FIND_LEAKS
#include	"leakfind.h"	/* checking for memory leaks */
#endif

inline static void
hashtbl_free_node(hashnode_t *np, int *n_left,
		  void *(*op)(void *elem, void *data),
		  void *data)
{
    --(*n_left);
#ifdef	DEBUG
    fprintf(stderr, "debug (%s): free key(%s) left(%d)\n",
	    __FUNCTION__, np->key, * n_left);
#endif
    free(np->key);
    if (op)
      op(np->data, data);
    else
      free(np->data);
    free(np);
}

/* free all nodes in a slot in the hash table array
 * don't call this function with a NULL head */
static void
hashtbl_free_slot(hashnode_t *list, int *n_left,
		  void *(*op)(void *elem, void *data),
		  void *data)
{
    hashnode_t	*np;

    while ( (np = list->next) ) {
      list->next	 = np->next; /* remove node from list */
      hashtbl_free_node(np, n_left, op, data);
    }
    hashtbl_free_node(list, n_left, op, data);
}


/* free all nodes in hash table, calling *op for each one if op is
 * not NULL */
void hashtbl_free(hashtbl_t *h,
		  void *(*op)(void *elem, void *data),
		  void *data)
{
    hashnode_t	**tbl	 = h->tbl;
    hashnode_t	*np;
    int		i;
    int		end	 = h->capacity;
    int		left	 = h->entries;

    for (i = 0; (i < end) && left; ++i)
      if ( (np = tbl[i]) ) {
	hashtbl_free_slot(np, &left, op, data);
	tbl[i]	 = NULL;
      }
}

