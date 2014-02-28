/* viewdb.c
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
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
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
#include	"elcerror_p.h"
#include	"elcerror.h"
#include	"cdb.h"
#include	"cdb_seq.h"
#include	"dbinfo.h"
#include	"show.h"
#ifdef		ELC_FIND_LEAKS
#include	"leakfind.h"
#endif

#undef	PROGNAME
#define	PROGNAME "i-viewdb"

typedef struct options {
  char		*targetname;
  unsigned	checksums: 1;
} options;  

void viewdb(options *opts)
{
    cdb_seq	seq;
    int		err;
    unsigned	ksiz	 = 1024; /* beginning key space allotment */
    char	*key	 = malloc(ksiz);
    dbinfo	val;
    size_t	klen, vlen;

    if (! key)
      DIE("malloc key");

    if ( (seq.fd = open(opts->targetname, O_RDONLY | O_NDELAY)) == -1)
      die(__FUNCTION__, "Error: opening integrit database (%s): %s",
	  opts->targetname, strerror(errno));
      
    if (cdb_seq_start(&seq) == -1)
      DIE("cdb_seq_start");
    while (! cdb_seq_eod(&seq)) { /* while not end of data */
      if ( (err = cdb_seq_sizes(&seq, &klen, &vlen)) == -1)
	DIE("cdb_seq_sizes");
      else if (err == 1)	/* no more in db sequence */
	break;
      while (klen > ksiz) {
	if (ksiz > (UINT_MAX / 2))
	  DIE("key size too big");
	ksiz	 *= 2;
	if (! (key = realloc(key, ksiz)) )
	  die(__FUNCTION__,
	      "Error: realloc key (length %d) with size (%d): %s",
	      klen, ksiz, strerror(errno));
      }
      if (vlen > sizeof(val))
	die(__FUNCTION__, "Error: bad entry (too big value) in DB (%s)",
	    opts->targetname);
      if (cdb_seq_get(&seq, key, klen, &val, vlen) == -1)
	DIE("cdb_seq_getkey");
      show_entry(opts->checksums, key, klen, &val, vlen);
    }

    free(key);
}

void usage(void)
{
    fputs("Usage:\n    " PROGNAME " [-s] {dbfile}\n"
	  "Options:\n    -s    don't show checksums\n",
	  stderr);
}

int main(int argc, char *argv[])
{
    options	opts	 = { NULL, 1 };
    char	*arg	 = argv[1];

    if (argc > 2)
      /* handle the checksums option */
      if (! strcmp(arg, "-s")) {
	opts.checksums	 = 0;
	arg	 = argv[2];
      }
    if (argc > 1)
      opts.targetname	 = arg;
    
    if (opts.targetname) {
      viewdb(&opts);
    } else {
      usage();
      exit(EXIT_FAILURE);
    }

#ifdef		ELC_FIND_LEAKS
    GC_gcollect();		/* find the leaks before exiting */
#endif
    return 0;
}
