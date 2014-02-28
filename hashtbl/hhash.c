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

/* hhash.c - the actual hash function based on cdb's
 */
#include	<config.h>
#include	<stdio.h>
#include	<stdlib.h>
/* support platforms that don't yet conform to C99 */
#if	HAVE_STDINT_H
#include	<stdint.h>
#elif	HAVE_INTTYPES_H
#include	<inttypes.h>
#else
#error No stdint.h or inttypes.h found.
#endif
#include	"hhash.h"
#ifdef	ELC_FIND_LEAKS
#include	"leakfind.h"	/* checking for memory leaks */
#endif

/* These two functions are modified only slightly from the public domain
 * code in Dan Bernstein's cdb package.
 * */
#define	CDB_HASHSTART	5381

inline static uint32_t cdb_hashadd(uint32_t h, uint8_t c)
{
    h += (h << 5);
    return h ^ c;
}

uint32_t hashtbl_cdb_hash(const char *key, size_t len)
{
    uint32_t	h;

    h	 = CDB_HASHSTART;
    while (len) {
      h	 = cdb_hashadd(h, *key++);
      --len;
    }
    return h;
}

