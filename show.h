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
#ifndef	ELC_SHOW_H
#define	ELC_SHOW_H

void show_long(FILE *out, char T, long val);
void show_octal(FILE *out, char T, unsigned long val);
void show_time(FILE *out, char T, time_t val);
void show_checksum(FILE *out, const unsigned char *sum, size_t siz);
void show_entry(int show_checksums,
		const char *key, size_t klen, dbinfo *val, size_t vlen);
void show_xml_long(FILE *out, const char *type, long val);
void show_xml_octal(FILE *out, const char *type, unsigned long val);

/* for showing the last twelve bits (four octal digits) */
#define		PERM_MASK	(07777)

#endif
