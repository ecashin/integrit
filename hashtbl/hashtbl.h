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
#ifndef	ELC_HASH_H
#define	ELC_HASH_H

/* #include	<stdlib.h> */
/* #include	<stdint.h> */

typedef struct elc_hashnode_struct {
  struct elc_hashnode_struct	*next;
  char				*key;
  size_t			keylen;
  void				*data;
} hashnode_t;

typedef struct elc_hashtbl_struct {
  hashnode_t	**tbl;
  int		entries;
  size_t	capacity;
} hashtbl_t;

/* use zero capacity for hashtbl's default capacity */
int hashtbl_init(hashtbl_t *h, size_t capacity);
void hashtbl_destroy(hashtbl_t *h);

/* free all nodes in hash table, calling *op for each one's data if
 * op is not NULL.  This gives caller a way to clean up a complex data
 * type that is pointed to by a hashtbl element. */
void hashtbl_free(hashtbl_t *h,
		  void *(*op)(void *elem, void *data),
		  void *data);

int hashtbl_store(hashtbl_t *h,
		  const char *key, size_t keylen,
		  void *data, void **replaced);
void *hashtbl_remove(hashtbl_t *h, const char *key, size_t keylen);
void *hashtbl_lookup(hashtbl_t *h, const char *key, size_t keylen);

/* run the function, "op", on each element in the hash table, passing along
 * to op a data pointer ("data") supplied by the caller
 */
void hashtbl_foreach(hashtbl_t *h,
		     void *(*op)(void *elem, void *data),
		     void *data);

#endif
