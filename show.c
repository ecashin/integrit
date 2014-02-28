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
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<time.h>
#include	<errno.h>
#include	<sys/stat.h>
/* support platforms that don't yet conform to C99 */
#if	HAVE_STDINT_H
#include	<stdint.h>
#elif	HAVE_INTTYPES_H
#include	<inttypes.h>
#else
#error No stdint.h or inttypes.h found.
#endif
#include	"digest.h"
#include	"hexprint_p.h"
#include	"elcerror_p.h"
#include	"elcerror.h"
#include	"dbinfo.h"
#include	"show.h"
#define		ELC_TIMEBUFSIZ	16 /* "20011215-010101" (with null char) */


/* based on show_diff_lines_long in eachfile.c */
void show_long(FILE *out, char T, long val)
{
    fprintf(out, "%c(%ld) ", T, val);
}

void show_octal(FILE *out, char T, unsigned long val)
{
    fprintf(out, "%c(%lo) ", T, val);
}

/* based on show_diff_time from eachfile.c */
void show_time(FILE *out, char T, time_t val)
{
    char	buf[ELC_TIMEBUFSIZ];

    putc(T, out);
    putc('(', out);

    if (! strftime(buf, ELC_TIMEBUFSIZ, "%Y%m%d-%H%M%S", localtime(&val)) )
      DIE("strftime");
    fputs(buf, out);

    fputs(") ", out);
}

void show_checksum(FILE *out, const unsigned char *sum, size_t siz)
{
    fputs("s(", out);
    hexprint(out, sum, siz);
    fputs(") ", out);
}

void show_entry(int show_checksums,
		const char *key, size_t klen, dbinfo *val, size_t vlen)
{
    int			i;
    struct stat		*s		 = &val->stat;

    /* show the filename */
    for (i = 0; i < klen; ++i)
      putc(key[i], stdout);
    fputs("   ", stdout);
    show_long(stdout, 'i', s->st_ino);

    if (S_ISLNK(s->st_mode)) /* for a symlink ... */
      fputs("p(sym) ", stdout); /* display "sym" instead of "777" */
    else
      show_octal(stdout, 'p', s->st_mode & PERM_MASK);

    show_long(stdout, 'l', s->st_nlink);
    show_long(stdout, 'u', s->st_uid);
    show_long(stdout, 'g', s->st_gid);
    show_long(stdout, 'z', s->st_size);
    show_time(stdout, 'a', s->st_atime);
    show_time(stdout, 'm', s->st_mtime);
    show_time(stdout, 'c', s->st_ctime);
    if (show_checksums && (vlen == sizeof(dbinfo)))
      show_checksum(stdout, val->sum, sizeof(val->sum));
    putc('\n', stdout);
}

