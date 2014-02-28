/* ls.c
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
#include	<limits.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<dirent.h>
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
#include	"dbinfo.h"
#include	"xstradd.h"
#include	"show.h"
#ifdef		ELC_FIND_LEAKS
#include	"leakfind.h"
#endif

#undef	PROGNAME
#define	PROGNAME "i-ls"

typedef struct options {
  char		*targetname;
  unsigned	checksums: 1;
} options;  

/* based on (but simplified from) function by the same name in eachfile.c */
static void do_checksum(const char *filename, dbinfo *dbinf)
{
    DIGEST_CONTEXT	context;
    char		buf[BUFSIZ];
    int			n;
    int			fd	 = open(filename, O_RDONLY);

    if (fd == -1) {
      /* just zero out the checksum on errors (usually permission denied) */
      memset(dbinf->sum, 0, sizeof(dbinf->sum));
      return;
    }

    digest_init(&context);
    while ( (n = read(fd, buf, BUFSIZ)) > 0)
      digest_write(&context, (uint8_t*) buf, n);

    if (n == -1)		/* read error */
      warn(__FUNCTION__, "Warning: read from file (%s) failed: %s",
	   filename, strerror(errno));

    close(fd);
    digest_final(&context);
    memcpy(dbinf->sum, digest_read(&context), DIGEST_LENGTH);
}

/* over-optimized version of (! strcmp(filename, "..")) */
inline static int is_self_or_parent(const char *filename)
{
    const char	*p;
    
    if (! filename)
      return 0;
    p = filename;
    if (*p && (*p++ == '.')) {
      if (! *p)
	return 1;		/* "." */
      if ((*p++ == '.') && (! *p))
	return 1;		/* ".." */
    }
    
    return 0;
}

void ls_directory(options *opts, dbinfo *dbinf_buf, const char *dirname)
{
    DIR			*d	 = opendir(dirname);
    struct dirent	*entry;
    char		*filename;
    char		*fullpath	 = NULL;
    int			doing_sum;

    if (! d)
      die(__FUNCTION__, "Error: opening directory (%s): %s",
	  dirname, strerror(errno));

    while ( (entry = readdir(d)) ) {
      filename	 = entry->d_name;
      if (is_self_or_parent(filename))
	continue;
      if (fullpath)
	free(fullpath);
      fullpath	 = xstradd(dirname, "/", filename, NULL);

      if (lstat(fullpath, &dbinf_buf->stat) == -1)
	die(__FUNCTION__, "Error: could not lstat file (%s): %s",
	    fullpath, strerror(errno));

      if (opts->checksums) {
	if ( (doing_sum = S_ISREG(dbinf_buf->stat.st_mode)) )
	  do_checksum(fullpath, dbinf_buf);
      } else {
	doing_sum	 = 0;
      }      

      show_entry(opts->checksums, fullpath, strlen(fullpath), dbinf_buf,
		 doing_sum ? sizeof(*dbinf_buf) : sizeof(dbinf_buf->stat));
    }

    if (fullpath)
      free(fullpath);
    closedir(d);
}

void ls(options *opts)
{
    dbinfo	dbinf;
    char	*target	 = opts->targetname;
    int		doing_sum	 = 0;
    
    if (lstat(target, &dbinf.stat) == -1)
      die(__FUNCTION__, "Error: could not lstat file (%s): %s",
	  target, strerror(errno));
    
    /* did lstat first so we know whether it's a regular file */
    if (opts->checksums && (S_ISREG(dbinf.stat.st_mode))) {
      doing_sum	 = 1;
      do_checksum(target, &dbinf);
    }

    show_entry(opts->checksums, target, strlen(target),
	       &dbinf, doing_sum ? sizeof(dbinf) : sizeof(dbinf.stat));

    if (S_ISDIR(dbinf.stat.st_mode))
      ls_directory(opts, &dbinf, target); /* reuse dbinf in ls_directory */
}

void usage(void)
{
    fputs("Usage:\n    " PROGNAME " [-s] [filename] [filename ...]\n"
          "    " PROGNAME " -h\n"
	  "Options:\n"
          "    -s    don't show checksums\n"
          "    -h    display this usage summary\n"
          ,
	  stderr);
}

int main(int argc, char *argv[])
{
    options	opts	 = { NULL, 1 };
    int i;

    for (i = 1; i < argc; ++i) {
      if (! strcmp(argv[i], "-h")) {
        usage();
        exit(EXIT_SUCCESS);
      } else if (! strcmp(argv[i], "-s")) {      /* handle the checksums option */
	opts.checksums = 0;
      } else {
        opts.targetname = argv[i];
        ls(&opts);
      }
    }
    
#ifdef		ELC_FIND_LEAKS
    GC_gcollect();		/* find the leaks before exiting */
#endif
    return 0;
}
