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
#ifndef	ELC_RULES_H
#define	ELC_RULES_H

typedef enum ruleflag {
  /* prefix rules */
  RULE_IGNORE		= 0x01,
  RULE_NOCHILDREN	= 0x02,
  RULE_NOINHERIT	= 0x04, /* true if this file has a non-inheriting
				  * checkset */

  /* suffix rules */
  RULE_SUM		= 0x04,
  RULE_INODE		= 0x08,
  RULE_PERMS		= 0x10,
  RULE_NLINK		= 0x20,
  RULE_UID		= 0x40,
  RULE_GID		= 0x80,
  RULE_SIZE		= 0x100,
  RULE_ATIME		= 0x200,
  RULE_MTIME		= 0x400,
  RULE_CTIME		= 0x800,
  RULE_RESET_ATIME	= 0x1000,
  RULE_TYPE		= 0x2000,
  RULE_DEVICETYPE	= 0x4000
} ruleflag;

#endif
