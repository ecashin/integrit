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
/* There are public domain versions of cdb.c, cdb.h, cdb_make.c,
 * cdb_make.h, and cdb_hash.c included in cdb-0.75 by Dan Bernstein.
 *
 * These files are distributed in integrit under the GPL and may have
 * been modified from the versions in cdb-0.75.
 */

#include	<config.h>
/* support platforms that don't yet conform to C99 */
#if	HAVE_STDINT_H
#include	<stdint.h>
#elif	HAVE_INTTYPES_H
#include	<inttypes.h>
#else
#error No stdint.h or inttypes.h found.
#endif
#include "cdb.h"
#include "cdb_hash.h"

uint32_t cdb_hashadd(uint32_t h,unsigned char c)
{
  h += (h << 5);
  return h ^ c;
}

uint32_t cdb_hash(char *buf,unsigned int len)
{
  uint32_t h;

  h = CDB_HASHSTART;
  while (len) {
    h = cdb_hashadd(h,*buf++);
    --len;
  }
  return h;
}
