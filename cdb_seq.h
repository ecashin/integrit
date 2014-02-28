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
#ifndef	ELC_CDB_SEQ_H
#define	ELC_CDB_SEQ_H

typedef struct cdb_seq {
  int		fd;
  uint32_t	eod;		/* end of data */
} cdb_seq;

int cdb_seq_start(cdb_seq *c);
int cdb_seq_eod(cdb_seq *c);
int cdb_seq_sizes(cdb_seq *c, size_t *klen, size_t *vlen);
int cdb_seq_get(cdb_seq *c, void *key, size_t klen, void *data, size_t dlen);
/* just get key and disregard data */
int cdb_seq_getkey(cdb_seq *c, void *key, size_t klen, size_t dlen);

#endif
