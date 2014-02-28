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

#ifndef CDB_MAKE_H
#define CDB_MAKE_H

#define CDB_HPLIST 1000

struct cdb_hp { uint32_t h; uint32_t p; } ;

struct cdb_hplist {
  struct cdb_hp hp[CDB_HPLIST];
  struct cdb_hplist *next;
  int num;
} ;

struct cdb_make {
  char bspace[8192];
  char final[2048];
  uint32_t count[256];
  uint32_t start[256];
  struct cdb_hplist *head;
  struct cdb_hp *split; /* includes space for hash */
  struct cdb_hp *hash;
  uint32_t numentries;
  uint32_t pos;
  FILE *fout;		/* we use stdio's fwrite instead
			 * of DJB's buffer stuff */
} ;

int cdb_make_start(struct cdb_make *c, FILE *newdb);
extern int cdb_make_addbegin(struct cdb_make *,unsigned int,unsigned int);
extern int cdb_make_addend(struct cdb_make *,unsigned int,unsigned int,uint32_t);
extern int cdb_make_add(struct cdb_make *,char *,unsigned int,char *,unsigned int);
extern int cdb_make_finish(struct cdb_make *);

#endif
