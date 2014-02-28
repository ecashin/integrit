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
#include	<stdio.h>	/* for FILE * in struct cdb */
#include	<stdlib.h>
#include	<sys/types.h>
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
#include	"packint.h"

int cdb_make_start(struct cdb_make *c, FILE *newdb)
{
  c->head = 0;
  c->split = 0;
  c->hash = 0;
  c->numentries = 0;
  c->fout = newdb;
  c->pos = sizeof c->final;
  return fseek(newdb, c->pos, SEEK_SET);
}

static int posplus(struct cdb_make *c,uint32_t len)
{
  uint32_t newpos = c->pos + len;

  if (newpos < len) {
    errno	 = ENOMEM;
    return -1;
  }
  c->pos = newpos;
  return 0;
}

int cdb_make_addend(struct cdb_make *c,unsigned int keylen,unsigned int datalen,uint32_t h)
{
  struct cdb_hplist *head;

  head = c->head;
  if (!head || (head->num >= CDB_HPLIST)) {
    head = (struct cdb_hplist *) malloc(sizeof(struct cdb_hplist));
    if (!head) return -1;
    head->num = 0;
    head->next = c->head;
    c->head = head;
  }
  head->hp[head->num].h = h;
  head->hp[head->num].p = c->pos;
  ++head->num;
  ++c->numentries;
  if (posplus(c,8) == -1) return -1;
  if (posplus(c,keylen) == -1) return -1;
  if (posplus(c,datalen) == -1) return -1;
  return 0;
}

int cdb_make_addbegin(struct cdb_make *c,unsigned int keylen,unsigned int datalen)
{
  char buf[8];

  if (keylen > 0xffffffff) {
    errno	  = ENOMEM;
    return -1;
  }
  if (datalen > 0xffffffff) {
    errno	  = ENOMEM;
    return -1;
  }

  UINT32_PACK(buf,keylen);
  UINT32_PACK(buf + 4,datalen);
  if (fwrite(buf, 8, 1, c->fout) < 1)
    return -1;

  return 0;
}

int cdb_make_add(struct cdb_make *c,char *key,unsigned int keylen,char *data,unsigned int datalen)
{
  if (cdb_make_addbegin(c,keylen,datalen) == -1) return -1;
  if (fwrite(key, keylen, 1, c->fout) < 1)
    return -1;
  if (fwrite(data, datalen, 1, c->fout) < 1)
    return -1;
  return cdb_make_addend(c,keylen,datalen,cdb_hash(key,keylen));
}

int cdb_make_finish(struct cdb_make *c)
{
  char buf[8];
  int i;
  uint32_t len;
  uint32_t u;
  uint32_t memsize;
  uint32_t count;
  uint32_t where;
  struct cdb_hplist *x;
  struct cdb_hp *hp;

  for (i = 0;i < 256;++i)
    c->count[i] = 0;

  for (x = c->head;x;x = x->next) {
    i = x->num;
    while (i--)
      ++c->count[255 & x->hp[i].h];
  }

  memsize = 1;
  for (i = 0;i < 256;++i) {
    u = c->count[i] * 2;
    if (u > memsize)
      memsize = u;
  }

  memsize += c->numentries; /* no overflow possible up to now */
  u = (uint32_t) 0 - (uint32_t) 1;
  u /= sizeof(struct cdb_hp);
  if (memsize > u) {
    errno = ENOMEM;
    return -1;
  }

  c->split = (struct cdb_hp *) malloc(memsize * sizeof(struct cdb_hp));
  if (!c->split) return -1;

  c->hash = c->split + c->numentries;

  u = 0;
  for (i = 0;i < 256;++i) {
    u += c->count[i]; /* bounded by numentries, so no overflow */
    c->start[i] = u;
  }

  for (x = c->head;x;x = x->next) {
    i = x->num;
    while (i--)
      c->split[--c->start[255 & x->hp[i].h]] = x->hp[i];
  }

  for (i = 0;i < 256;++i) {
    count = c->count[i];

    len = count + count; /* no overflow possible */
    UINT32_PACK(c->final + 8 * i,c->pos);
    UINT32_PACK(c->final + 8 * i + 4,len);

    for (u = 0;u < len;++u)
      c->hash[u].h = c->hash[u].p = 0;

    hp = c->split + c->start[i];
    for (u = 0;u < count;++u) {
      where = (hp->h >> 8) % len;
      while (c->hash[where].p)
	if (++where == len)
	  where = 0;
      c->hash[where] = *hp++;
    }

    for (u = 0;u < len;++u) {
      UINT32_PACK(buf,c->hash[u].h);
      UINT32_PACK(buf + 4,c->hash[u].p);
      if (fwrite(buf, 8, 1, c->fout) < 1)
	return -1;
      if (posplus(c,8) == -1) return -1;
    }
  }

  /* if (buffer_flush(&c->b) == -1) return -1; */
  if (fseek(c->fout, 0L, SEEK_SET) == -1)
    return -1;
  return (fwrite(c->final, sizeof c->final, 1, c->fout) < 1) ? -1 : 1;
}
