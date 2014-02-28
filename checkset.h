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
/* checkset.h - interface for checkset type, a set of rules about what
 *              runtime checks to perform
 *
 * Originally this was a byte array, but trying to do non-cascading
 * rules in a funky byte array would be too messy.
 * */
#ifndef	ELC_CHECKSET_H
#define	ELC_CHECKSET_H

typedef struct checkset_struct {
  unsigned char 	flags;	/* booleans */
  char			*switches;
} checkset;

/* the data structure for a file that has a non-inheriting checkset
 * is more complex.  This allows us to use the cheaper type for most
 * cases.  If the RULE_NOINHERIT flag is set, then we know that we
 * can cast to ni_checkset and access the extra fields.
 */
typedef struct ni_checkset_struct {
  checkset		regular;
  char			*ni_switches;
} ni_checkset;

#define CHECKSET_SETBOOLEAN(c, b, v) do {	\
    if (v)					\
      c->flags	 |= b;				\
    else					\
      c->flags	 &= ~b;				\
} while (0)

#endif
