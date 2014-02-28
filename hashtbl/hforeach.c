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

/* do op for all nodes in a particular slot from the hash table array
 * don't call this function with a NULL head */
inline static void
hashtbl_slot_foreach(hashnode_t *list, int *n_left,
		     void *(*op)(void *elem, void *data),
		     void *data)
{
    hashnode_t	*np;

    while ( (np = list->next) ) {
      list->next	 = np->next; /* remove node from list */
      op(np->data, data);
      --(*n_left);
    }
    op(list->data, data);
    --(*n_left);
}


void hashtbl_foreach(hashtbl_t *h,
		     void *(*op)(void *elem, void *data),
		     void *data)
{
    hashnode_t	**tbl		 = h->tbl;
    hashnode_t	*np;
    int		i;
    int		end		 = h->capacity;
    int		remaining	 = h->entries;

    for (i = 0; (i < end) && remaining; ++i)
      if ( (np = tbl[i]) ) {
	hashtbl_slot_foreach(np, &remaining, op, data);
	tbl[i]	 = NULL;
      }
}

