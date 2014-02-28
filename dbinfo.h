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
/* this file contains the definition of the dbinfo data structure that
 * is stored in the integrit database.
 *
 * it's been separated out from eachfile.c in order to make it available
 * auxiliary programs. */

#ifndef	ELC_DBINFO_H
#define	ELC_DBINFO_H

typedef struct dbinfo {	/* file information for the database */
  struct stat	stat;
  unsigned char	sum[DIGEST_LENGTH];
} dbinfo;

#endif
