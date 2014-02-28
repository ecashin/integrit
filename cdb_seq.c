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
#include	"cdb_make.h"
#include	"elcerror.h"
#include	"cdb_seq.h"
#include	"packint.h"

#define	CDB_HEADER_SIZE	(2048)

/* endian issues will bite us if we don't call uint32_unpack when
 * reading numbers from the cdb file, so we do it here. */
static int read_uint32(int fd, uint32_t *n)
{
    unsigned char	buf[4];
    ssize_t		err;

    if ( (err = read(fd, &buf, sizeof(*n))) == -1)
      return -1;
    else if (err == 0)
      return 1;
    else if (err < sizeof(buf)) {
      /* Error: could not read a whole uint32_t */
      errno	 = EIO;		/* ENODATA not on FreeBSD */
      return -1;
    }
    
    UINT32_UNPACK(buf, n);

    return 0;
}

int cdb_seq_start(cdb_seq *c)
{
    long int		err;
    int			fd	 = c->fd;

    if ( (err = lseek(fd, 0, SEEK_SET)) == -1)
      return -1;
    if ( (err = read_uint32(fd, &c->eod)) == -1)
      return -1;
    else if (err == 1) {	/* no more data */
      errno	 = EIO;
      return -1;
    }	

    if ( (err = lseek(fd, CDB_HEADER_SIZE, SEEK_SET)) == -1) {
      return -1;
    } else if (err != CDB_HEADER_SIZE) {
      errno	 = EIO;		/* short read on cdb header */
      return -1;
    }

    return 0;
}

int cdb_seq_eod(cdb_seq *c)
{
    off_t	pos;

    if ( (pos = lseek(c->fd, 0, SEEK_CUR)) == -1)
      return -1;

    return pos >= c->eod;
}

int cdb_seq_sizes(cdb_seq *c, size_t *klen, size_t *vlen)
{
    ssize_t	err;
    uint32_t	n;

    if ( (err = read_uint32(c->fd, &n)) == -1)
      return -1;
    else if (err == 1)
      return 1;			/* no more data */
    *klen	 = n;

    if ( (err = read_uint32(c->fd, &n)) == -1)
      return -1;
    else if (err == 1)
      return 1;			/* no more data */
    *vlen	 = n;

    return 0;
}

int cdb_seq_get(cdb_seq *c, void *key, size_t klen, void *data, size_t dlen)
{
    ssize_t	err;

    if ( (err = read(c->fd, key, klen)) == -1) {
      return -1;
    } else if (err == 0) {
      return 1;			/* no more data */
    } else if (err < klen) {
      errno	 = EIO;		/* I/O error */
      return -1;		/* short read */
    }

    if ( (err = read(c->fd, data, dlen)) == -1) {
      return -1;
    } else if (err == 0) {
      return 1;			/* no more data */
    } else if (err < dlen) {
      errno	 = EIO;		/* I/O error */
      return -1;		/* short read */
    }

    return 0;
}

int cdb_seq_getkey(cdb_seq *c, void *key, size_t klen, size_t dlen)
{
    ssize_t	err;

    if ( (err = read(c->fd, key, klen)) == -1) {
      return -1;
    } else if (err == 0) {
      return 1;			/* no more */
    } else if (err < klen) {
      errno	 = EIO;		/* I/O error */
      return -1;		/* short read */
    }

    if (lseek(c->fd, dlen, SEEK_CUR) == -1)
      return -1;

    return 0;
}

