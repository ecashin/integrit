/*
integrit - file integrity verification system
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
#include	<config.h>
#include	<stdio.h>	/* for FILE * in struct cdb */
#include	<string.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
/* support platforms that don't yet conform to C99 */
#if	HAVE_STDINT_H
#include	<stdint.h>
#elif	HAVE_INTTYPES_H
#include	<inttypes.h>
#else
#error No stdint.h or inttypes.h found.
#endif
#include	"cdb.h"
#include	"cdb_hash.h"
#include	"cdb_make.h"
#include	"elcerror.h"
#include	"elcerror_p.h"

int cdb_put(struct cdb_make *c,
	    unsigned char *key, size_t klen, void *val, size_t vlen)
{
    int			i;
    uint32_t		h;

    if (cdb_make_addbegin(c, klen, vlen) == -1)
      return -1;
    if (fwrite((void *) key, klen, 1, c->fout) < 1)
      DIE("writing to current database");
    for (i = 0, h = CDB_HASHSTART; i < klen; ++i)
      h	 = cdb_hashadd(h, key[i]);
    if (fwrite(val, vlen, 1, c->fout) < 1)
      DIE("writing to current database");
      
    if (cdb_make_addend(c, klen, vlen, h) == -1)
      return -1;

    return 0;
}

