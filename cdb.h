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

#ifndef CDB_H
#define CDB_H

#define CDB_HASHSTART 5381

struct cdb {
  char *map; /* 0 if no map is available */
  int fd;
  uint32_t size; /* initialized if map is nonzero */
  uint32_t loop; /* number of hash slots searched under this key */
  uint32_t khash; /* initialized if loop is nonzero */
  uint32_t kpos; /* initialized if loop is nonzero */
  uint32_t hpos; /* initialized if loop is nonzero */
  uint32_t hslots; /* initialized if loop is nonzero */
  uint32_t dpos; /* initialized if cdb_findnext() returns 1 */
  uint32_t dlen; /* initialized if cdb_findnext() returns 1 */
} ;

extern void cdb_free(struct cdb *);
extern void cdb_init(struct cdb *,int fd);

extern int cdb_read(struct cdb *,char *,unsigned int,uint32_t);

extern void cdb_findstart(struct cdb *);
extern int cdb_findnext(struct cdb *,char *,unsigned int);
extern int cdb_find(struct cdb *,char *,unsigned int);

#define cdb_datapos(c) ((c)->dpos)
#define cdb_datalen(c) ((c)->dlen)

#endif
