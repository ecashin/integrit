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
/* header file for rmd160 routines from gnupg, based directly on rmd160.c */

#ifndef	ELC_RMD160_H
#define	ELC_RMD160_H

#define RMD160_DIGEST_LENGTH 20

typedef struct {
    uint32_t h0,h1,h2,h3,h4;
    uint32_t nblocks;
    uint8_t buf[64];
    int count;
} RMD160_CONTEXT;

void rmd160_init( RMD160_CONTEXT *hd );
void rmd160_write( RMD160_CONTEXT *hd, uint8_t *inbuf, size_t inlen);
void rmd160_final( RMD160_CONTEXT *hd );
uint8_t *rmd160_read( RMD160_CONTEXT *hd );

#endif
