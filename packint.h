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
/* pack and unpack numbers from unsigned 32-bit native-format integers
 * into little-endian 32-bit representation for storage in CDB-format
 * database.
 */

#ifndef	ELC_PACKINT_H
#define	ELC_PACKINT_H

/* based on macro from glib */
#define UINT32_SWAP_BYTES_LE_BE(val)	((uint32_t) ( \
    (((uint32_t) (val) & (uint32_t) 0x000000ffU) << 24) | \
    (((uint32_t) (val) & (uint32_t) 0x0000ff00U) <<  8) | \
    (((uint32_t) (val) & (uint32_t) 0x00ff0000U) >>  8) | \
    (((uint32_t) (val) & (uint32_t) 0xff000000U) >> 24)))

#if	BIG_ENDIAN_HOST
#define	UINT32_UNPACK(in, n) do {			\
    uint32_t tmpint	 = *((uint32_t *) (in));	\
    (*(n))	 = UINT32_SWAP_BYTES_LE_BE(tmpint);		\
} while (0)
#elif LITTLE_ENDIAN_HOST
#define	UINT32_UNPACK(in, n) do {		\
    *(n)	 = *((uint32_t *) (in));	\
} while (0)
#else
#error Unsupported byte order
#endif

#if	BIG_ENDIAN_HOST
#define	UINT32_PACK(b, n)	do {				\
    uint32_t	tmpuint	 = (uint32_t)(n);			\
    *((uint32_t *) (b))	 = UINT32_SWAP_BYTES_LE_BE(tmpuint);	\
} while (0)
#elif	LITTLE_ENDIAN_HOST
#define	UINT32_PACK(b, n)	do {		\
    *((uint32_t *) (b))	 = (n);			\
} while (0)
#else
#error Unsupported byte order
#endif

#endif
