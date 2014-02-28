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

/* this file contains experimental code for cross-platform stat data
 */
#include <config.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "istat.h"

static unsigned long unpack_int(packed p)
{
    int i, end;
    unsigned long n = 0;

    for (i = 0, end = sizeof(p); i < end; ++i)
      n |= (((unsigned long) p[i]) >> i) & UCHAR_MAX;

    return n;
}

static void pack_int(packed p, unsigned long n)
{
    int i, end;

    for (i = 0, end = sizeof(p); i < end; ++i)
      p[i] = (n >> i) & UCHAR_MAX;
}

void copy_sb_to_istat(istat_t *istat, struct stat *sb)
{
    pack_int(istat->size, sb->st_size);
    pack_int(istat->ino, sb->st_ino);
    pack_int(istat->atime, sb->st_atime);
    pack_int(istat->mtime, sb->st_mtime);
    pack_int(istat->ctime, sb->st_ctime);
    pack_int(istat->mode, sb->st_mode);
    pack_int(istat->nlink, sb->st_nlink);
    pack_int(istat->uid, sb->st_uid);
    pack_int(istat->gid, sb->st_gid);
}

void copy_istat_to_sb(struct stat *sb, istat_t *istat)
{
    sb->st_size = unpack_int(istat->size);
    sb->st_ino = unpack_int(istat->ino);
    sb->st_atime = unpack_int(istat->atime);
    sb->st_mtime = unpack_int(istat->mtime);
    sb->st_ctime = unpack_int(istat->ctime);
    sb->st_mode = unpack_int(istat->mode);
    sb->st_nlink = unpack_int(istat->nlink);
    sb->st_uid = unpack_int(istat->uid);
    sb->st_gid = unpack_int(istat->gid);
}
