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
/* $Header: /cvsroot/integrit/integrit/main.c,v 1.31 2006/04/07 09:41:10 wavexx Exp $ */
#include	<config.h>
#include	<stdio.h>
#include	<unistd.h>
#include	<string.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<fcntl.h>
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
#include	"cdb.h"
#include	"cdb_make.h"
#include	"hashtbl/hashtbl.h"
#include	"elcwft.h"
#include	"checkset.h"
#include	"integrit.h"
#include	"rules.h"
#include	"eachfile.h"
#include	"missing.h"
#include	"xml.h"
#include	"elcerror.h"
#include	"elcerror_p.h"
#include	"hexprint_p.h"
#include	"options_p.h"
#include	"elcwft_p.h"
#include	"eachfile_p.h"
#include	"missing_p.h"
#include	"xml_p.h"
#ifdef		ELC_FIND_LEAKS
#include	"leakfind.h"
#endif

static void open_known_cdb(integrit_t *it)
{
    int		fd	 = open(it->knowndbname,
				O_RDONLY | O_NDELAY);

    if (fd == -1)
      die(__FUNCTION__, "Error: opening known-state database (%s): %s",
	  it->knowndbname, strerror(errno));
    cdb_init(&it->knowndb, fd);
}

static void open_current_cdb(integrit_t *it)
{
    FILE	*newdb	 = NULL; /* get rid of superfluous compiler warning */
    int		fd;

    fd	 = open(it->currdbname,
		O_TRUNC | O_CREAT | O_WRONLY | O_NDELAY, 0640);
    if ((fd == -1) || ! (newdb = fdopen(fd, "wb")) )
      DIE("opening current-state database");
    memset(&it->currdb, 0, sizeof(it->currdb));
    if (cdb_make_start(&it->currdb, newdb) == -1)
      DIE("start cdb_make");
}

static void close_current_cdb(integrit_t *it)
{
    if (cdb_make_finish(&it->currdb) == -1)
      DIE("finishing current-state database");
    (void) fclose(it->currdb.fout);
}    

static void close_known_cdb(integrit_t *it)
{
    (void) close(it->knowndb.fd);
    cdb_free(&it->knowndb);
}

static void get_currdb_checksum(integrit_t *it, char sumbuf[])
{
    DIGEST_CONTEXT	context;
    char		buf[BUFSIZ];
    char		*fname	 = it->currdbname;
    long		n;
    int			fd	 = open(fname, O_RDONLY);

    if (fd == -1)
      die(__FUNCTION__, "Error: opening file (%s): %s",
	  fname, strerror(errno));
    digest_init(&context);
    while ( (n = read(fd, buf, BUFSIZ)) ) {
      if (n == -1)
	die(__FUNCTION__, "Error: reading file (%s): %s",
	    fname, strerror(errno));
      digest_write(&context, (uint8_t*) buf, (unsigned long) n);
    }
    close(fd);
    digest_final(&context);
    memcpy(sumbuf, digest_read(&context), DIGEST_LENGTH);
}

static void show_currdb_checksum_abbrev(integrit_t *it, char *sumbuf, size_t n)
{
    char	*fname	 = it->currdbname;

    fputs(PROGNAME ": current-state db " DIGEST_NAME " -------------- \n"
	  PROGNAME ": ", stdout);
    hexprint(stdout, (const unsigned char*) sumbuf, n);
    fputs("  ", stdout);
    puts(fname);
}

static void show_currdb_checksum_xml(integrit_t *it,
				     const char *checksum_type,
				     char *sumbuf, size_t n)
{
    char	*fname	 = it->currdbname;

    fprintf(stdout, "<checksum type=\"%s\" file=\"%s\">",
	    checksum_type, fname);
    hexprint(stdout, (const unsigned char*) sumbuf, n);
    XML_END_PRINT(stdout, "checksum");
    putc('\n', stdout);
}

int main(int argc, char *argv[])
{
    integrit_t	it;
    char	newdb_checksum[DIGEST_LENGTH];
    wft_context_t wftctx	 = { 0, };
    int		ret;

#if	defined(ELC_FIND_LEAKS) && 0 /* disabled */
    GC_find_leak	 = 1;		/* turn on boehm leak detection
					 * in newer versions of gc.
					 * comment this out for old
					 * versions of gc */
#endif

    it.exit_status = INTEGRIT_EXIT_NOCHANGE;

    options_init(&it);
    options_set(&it, argc, argv);
    if (it.output == OUTPUT_XML) {
      xml_declaration(stdout);
      /* xml_dtd(stdout); */
      xml_start_report(stdout, &it);
    }
    options_announce(stdout, &it);

    /* todo: use shared file locking with fcntl */
    if (it.do_check && it.do_update &&
	!strcmp(it.knowndbname, it.currdbname))
      die(__FUNCTION__, "current and known db cannot be the same file");

    if (it.do_check)
      open_known_cdb(&it);
    if (it.do_update)
      open_current_cdb(&it);
    
    if (it.do_check || it.do_update) {
      wftctx.rootname	 = it.root;
      wftctx.callback	 = process_file;
      wftctx.cb_data	 = &it;
      wftctx.options	 = it.verbose > 0 ? WFT_VERBOSE : 0;
      wftctx.options	|= it.stop_on_err > 0 ? WFT_STOP_ON_ERR : 0;
      if ( (ret = walk_file_tree(&wftctx)) == WFT_ERROR )
	DIE("walk_file_tree");
      else if(ret == WFT_SOME_ERR)
	it.exit_status	|= INTEGRIT_EXIT_FAILURE;
    }
    
    if (it.do_update) {
      close_current_cdb(&it);
      get_currdb_checksum(&it, newdb_checksum);
    }

    if (it.do_check) {
      if (it.do_update)
	check_for_missing(&it);	/* only do this after new current cdb
					   is closed (i.e. cdb_make is done) */
      else if (it.verbose > 0)
	fputs(PROGNAME ": not doing update, so no check for missing files\n",
	      stderr);
      
      close_known_cdb(&it);
    }
    if (it.do_update) {
      if (it.output == OUTPUT_XML)
	show_currdb_checksum_xml(&it, DIGEST_NAME,
				 newdb_checksum, sizeof(newdb_checksum));
      else
	show_currdb_checksum_abbrev(&it,
				    newdb_checksum, sizeof(newdb_checksum));
    }
    
    if (it.output == OUTPUT_XML) {
      XML_END_PRINT(stdout, "report");
      putc('\n', stdout);
    }

    options_destroy(&it);
#ifdef		ELC_FIND_LEAKS
    GC_gcollect();		/* find the leaks before exiting */
#endif

    return it.exit_status;
}
