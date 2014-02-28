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
#if	DEBUG
#include	<stdio.h>
#endif
#include	<string.h>
#include	<sys/types.h>
#include	<unistd.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
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
#include	"packint.h"

#ifndef CDB_MMAP_MAX
#warning CDB_MAX_MMAP not defined.  Using default (0xffffffff)
#define	CDB_MMAP_MAX	0xffffffff
#endif

void cdb_free(struct cdb *c)
{
  if (c->map) {
    munmap(c->map,c->size);
    c->map = 0;
  }
}

void cdb_findstart(struct cdb *c)
{
  c->loop = 0;
}

void cdb_init(struct cdb *c,int fd)
{
  struct stat st;
  char *x;

  cdb_free(c);
  cdb_findstart(c);
  c->fd = fd;

  if (fstat(fd,&st) == 0) {
    if (st.st_size <= CDB_MMAP_MAX) {
#if	DEBUG
      fputs("debug: using mmap for cdb\n", stderr);
#endif
      x = mmap(0,st.st_size,PROT_READ,MAP_SHARED,fd,0);
      if (x + 1) {
	c->size = st.st_size;
	c->map = x;
      }
    }
#if	DEBUG
    else {
      fputs("debug: NOT using mmap for cdb\n", stderr);
    }
#endif
  }
}

int cdb_read(struct cdb *c,char *buf,unsigned int len,uint32_t pos)
{
  if (c->map) {
    if ((pos > c->size) || (c->size - pos < len)) goto FORMAT;
    memmove(buf, c->map + pos, len);
  }
  else {
    if (lseek(c->fd, pos, SEEK_SET) == -1)
      return -1;
    while (len > 0) {
      int r;
      do {
        r = read(c->fd,buf,len);
      } while ((r == -1) && (errno == EINTR));
      if (r == -1) return -1;
      if (r == 0) goto FORMAT;
      buf += r;
      len -= r;
    }
  }
  return 0;

  FORMAT:
  errno	 = EINVAL;
  return -1;
}

static int match(struct cdb *c,char *key,unsigned int len,uint32_t pos)
{
  char buf[32];
  int n;

  while (len > 0) {
    n = sizeof buf;
    if (n > len) n = len;
    if (cdb_read(c,buf,n,pos) == -1) return -1;
    /* if (byte_diff(buf,n,key)) return 0; */
    if (memcmp(buf, key, n))
      return 0;
    pos += n;
    key += n;
    len -= n;
  }
  return 1;
}

int cdb_findnext(struct cdb *c,char *key,unsigned int len)
{
  char buf[8];
  uint32_t pos;
  uint32_t u;

  if (!c->loop) {
    u = cdb_hash(key,len);
    if (cdb_read(c,buf,8,(u << 3) & 2047) == -1) return -1;
    UINT32_UNPACK(buf + 4,&c->hslots);
    if (!c->hslots) return 0;
    UINT32_UNPACK(buf,&c->hpos);
    c->khash = u;
    u >>= 8;
    u %= c->hslots;
    u <<= 3;
    c->kpos = c->hpos + u;
  }

  while (c->loop < c->hslots) {
    if (cdb_read(c,buf,8,c->kpos) == -1) return -1;
    UINT32_UNPACK(buf + 4,&pos);
    if (!pos) return 0;
    c->loop += 1;
    c->kpos += 8;
    if (c->kpos == c->hpos + (c->hslots << 3)) c->kpos = c->hpos;
    UINT32_UNPACK(buf,&u);
    if (u == c->khash) {
      if (cdb_read(c,buf,8,pos) == -1) return -1;
      UINT32_UNPACK(buf,&u);
      if (u == len)
	switch(match(c,key,len,pos + 8)) {
	  case -1:
	    return -1;
	  case 1:
	    UINT32_UNPACK(buf + 4,&c->dlen);
	    c->dpos = pos + 8 + len;
	    return 1;
	}
    }
  }

  return 0;
}

int cdb_find(struct cdb *c,char *key,unsigned int len)
{
  cdb_findstart(c);
  return cdb_findnext(c,key,len);
}
