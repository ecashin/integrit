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
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<errno.h>
/* support platforms that don't yet conform to C99 */
#if	HAVE_STDINT_H
#include	<stdint.h>
#elif	HAVE_INTTYPES_H
#include	<inttypes.h>
#else
#error No stdint.h or inttypes.h found.
#endif
#include	"digest.h"
#include	"cdb.h"
#include	"cdb_make.h"
#include	"cdb_seq.h"
#include	"hashtbl/hashtbl.h"
#include	"integrit.h"
#include	"elcerror.h"
#include	"missing.h"
#include	"xml.h"
#include	"missing_p.h"
#include	"elcerror_p.h"
#include	"xml_p.h"
#include	"dbinfo.h"
#include	"show.h"
#ifdef		ELC_FIND_LEAKS
#include	"leakfind.h"
#endif

static void print_missing_xml(FILE *out, integrit_t *it,
			      const char *key, size_t klen,
                              const dbinfo *dbinf)
{
    const struct stat *s = &dbinf->stat;

    XML_MISSING_START_PRINT(out, key);

    show_xml_long(stdout, "permissions", s->st_mode & PERM_MASK);
    show_xml_long(stdout, "type", s->st_mode & S_IFMT);
    show_xml_long(stdout, "uid", s->st_uid);
    show_xml_long(stdout, "gid", s->st_gid);
    show_xml_long(stdout, "size", s->st_size);
    show_xml_long(stdout, "modification_time", s->st_mtime);

    XML_END_PRINT(out, "missing");
    putc('\n', out);
}

inline static void announce_missing(const char *s, size_t n, FILE *out)
{
    fputs("missing: ", out);
    while (n--)
      putc(*s++, out);
    fputs("   ", out);
}

static void print_missing_lines(FILE *out, integrit_t *it,
				const char *key, size_t klen,
				const dbinfo *dbinf, size_t vlen)
{
    const struct stat	*s	 = &dbinf->stat;

    announce_missing(key, klen, out);

    /* show some interesting information about the missing file: pugzm */
    if (S_ISLNK(s->st_mode)) /* for a symlink ... */
      fputs("p(sym) ", out); /* display "sym" instead of "777" */
    else
      show_octal(out, 'p', s->st_mode & PERM_MASK);

    show_long(out, 'u', s->st_uid);
    show_long(out, 'g', s->st_gid);
    show_long(out, 'z', s->st_size);
    show_time(out, 'm', s->st_mtime);

    if (vlen == sizeof(dbinfo)) { /* if there's a checksum */
      putc('\n', out);
      announce_missing(key, klen, out);
      show_checksum(out, dbinf->sum, sizeof(dbinf->sum));
    }

    putc('\n', out);
}

static void do_currdb_check(integrit_t *it, struct cdb *curr_cdb,
			    char *key, size_t klen,
			    dbinfo *dbinf, size_t vlen)
{
    int		ret;

    if ( (ret = cdb_find(curr_cdb, key, klen)) == -1)
      die(__FUNCTION__,
	  "Error: looking up file (%s) in current-state database (%s): %s",
	  key, it->currdbname, strerror(errno));
    else if (! ret) {
      it->exit_status |= INTEGRIT_EXIT_CHANGES;
      if (it->output == OUTPUT_XML)
	print_missing_xml(stdout, it, key, klen, dbinf);
      else
	print_missing_lines(stdout, it, key, klen, dbinf, vlen);
    }
}

/* based on open_known_cdb in main.c */
void open_current_state_cdb(const char *dbname, struct cdb *curr_cdb)
{
    int		fd	 = open(dbname, O_RDONLY | O_NDELAY);

    if (fd == -1)
      die(__FUNCTION__, "Error: opening current-state database (%s): %s",
	  dbname, strerror(errno));
    cdb_init(curr_cdb, fd);
}

void check_for_missing(integrit_t *it)
{
    cdb_seq	knownseq;
    struct cdb	curr_cdb;
    int		err;
    unsigned	ksiz	 = 1024; /* beginning key space allotment */
    char	*key	 = malloc(ksiz);
    dbinfo	dbinf;
    size_t	klen, vlen;

    if (! key)
      DIE("malloc key");

    open_current_state_cdb(it->currdbname, &curr_cdb);

    if ((it->verbose > 0)
	&& (it->output != OUTPUT_XML))
      puts(PROGNAME ": checking for missing files --------------");

    knownseq.fd	 = it->knowndb.fd;
    if (cdb_seq_start(&knownseq) == -1)
      DIE("cdb_seq_start");
    while (! cdb_seq_eod(&knownseq)) { /* while not end of data */
      if ( (err = cdb_seq_sizes(&knownseq, &klen, &vlen)) == -1)
	DIE("cdb_seq_sizes");
      else if (err == 1)	/* no more in db sequence */
	break;
      while (klen > ksiz) {
	if (ksiz > (UINT_MAX / 2)) /* guard against overflow */
	  DIE("key size too big");
	ksiz	 *= 2;
	if (! (key = realloc(key, ksiz)) )
	  DIE("realloc key");
      }
      if (cdb_seq_get(&knownseq, key, klen, &dbinf, vlen) == -1)
	DIE("cdb_seq_getkey");
      do_currdb_check(it, &curr_cdb, key, klen, &dbinf, vlen);
    }

    free(key);
}
