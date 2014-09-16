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
#include	<stdlib.h>
#include	<string.h>
#include	<fcntl.h>
#include	<dirent.h>
#include	<time.h>
#include	<utime.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/time.h>
#include	<unistd.h>
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
#include	"cdb_get.h"
#include	"cdb_put.h"
#include	"hashtbl.h"
#include	"elcerror.h"
#include	"xstrdup.h"
#include	"xstradd.h"
#include	"integrit.h"
#include	"rules.h"
#include	"checkset.h"
#include	"elcwft.h"
#include	"eachfile.h"
#include	"xml.h"
#include	"elcerror_p.h"
#include	"rules_p.h"
#include	"hexprint_p.h"
#include	"eachfile_p.h"
#include	"xml_p.h"
#include	"dbinfo.h"
#include	"show.h"
#ifdef		ELC_FIND_LEAKS
#include	"leakfind.h"
#endif
#define		ALGO_NAME	"checksum"
#define		ELC_TIMEBUFSIZ	16


typedef struct fileinfo {	/* database file information */
  dbinfo	dbinf;
  char		*path;		/* null-terminated */
  size_t	pathlen;	/* save on strlen calls,
				 * since we know it already */
  unsigned long	ruleset;
  unsigned	found_ruleset: 1; /* flag indicates whether ruleset
				   * member has been initialized */
  unsigned	did_sum: 1;	/* flag shows whether sum member
				 * contains a valid checksum */
} fileinfo;

static void fileinfo_init(fileinfo *inf, char *path, size_t pathlen)
{
    inf->ruleset	 = 0;
    inf->found_ruleset	 = 0;
    inf->did_sum	 = 0;
    inf->path		 = path;
    inf->pathlen	 = pathlen;
}

static unsigned long fileinfo_ruleset(integrit_t *it, fileinfo *inf)
{
    if (! inf->found_ruleset) {
      inf->ruleset		 = rules_for_path(it, inf->path);
      inf->found_ruleset	 = 1;
    }

    return inf->ruleset;
}

/* There's a race condition between lstat and open in the following
 * sequence:
 *
 *	lstat "foo"
 *	if is regular file, open and read
 *
 * ... so we do an extra (cheap, says Andrew Gierth) stat using fstat
 * on the file descriptor returned by open.  Then we can avoid accidentally
 * following symlinks or reading from fifos, sockets, etc., that have
 * become file "foo" (e.g. with "mv mysock foo") since we did lstat.
 *
 * Thanks to lumpy <lumpy@9mm.com> for pointing out the race condition.
 *
 */
inline static int do_checksum(integrit_t *it, fileinfo *inf)
{
    DIGEST_CONTEXT	context;
    char		buf[BUFSIZ];
    int			n;
    struct stat		fd_sb;		/* file-descriptor-based stat buffer */
    int			fd;
    dbinfo		*dbinf	 = &inf->dbinf;
    struct utimbuf	utb;	/* for resetting access time */

    digest_init(&context);

    /*
     * For a symlink, read the name in the symlink as if it were "contents".
     * Note that we can't make this check on the open fd's stat buffer,
     * because fstat() works on the underlying file, not the symlink.
     */
    if (S_ISLNK(inf->dbinf.stat.st_mode)) {
      int ret;
      ret = readlink(inf->path, buf, BUFSIZ);
      if (ret < 0) {
	warn(__FUNCTION__, "Warning: could not readlink symlink (%s): %s",
	     inf->path, strerror(errno));
	return 0;
      }
      digest_write(&context, (uint8_t*) buf, ret);
    } else {
      if ( (fd = open(inf->path, O_RDONLY)) == -1) {
        warn(__FUNCTION__, "Warning: could not open file (%s) for reading: %s",
	     inf->path, strerror(errno));
        return 0;			/* didn't do checksum */
      }
      if (fstat(fd, &fd_sb) == -1)
        die(__FUNCTION__, "Error: could not fstat file (%s): %s",
	    inf->path, strerror(errno));

      /* check that the file we've opened is the same one we did lstat on */
      if (fd_sb.st_ino != inf->dbinf.stat.st_ino) {
        warn(__FUNCTION__, "Warning: file (%s) changed before we opened it",
	     inf->path);
        return 0;			/* didn't do checksum */
      }
      /* if read fails, n gets -1 */
      while ( (n = read(fd, buf, BUFSIZ)) > 0)
        digest_write(&context, (uint8_t*) buf, n);

      if (n == -1)		/* read error */
        warn(__FUNCTION__, "Warning: read from file (%s) failed: %s",
	     inf->path, strerror(errno));

      close(fd);
    }

    digest_final(&context);
    memcpy(dbinf->sum, digest_read(&context), DIGEST_LENGTH);

    /* reset access time on request */
    if (fileinfo_ruleset(it, inf) & RULE_RESET_ATIME) {
#ifdef	DEBUG
      fprintf(stderr, "debug (%s): resetting atime for file (%s)\n",
	      __FUNCTION__, inf->path);
#endif
      utb.actime	 = dbinf->stat.st_atime;
      utb.modtime	 = dbinf->stat.st_mtime;
      if (utime(inf->path, &utb) == -1)
	warn(__FUNCTION__,
	     "Warning: resetting access time for file (%s): %s",
	     inf->path, strerror(errno));
    }

    return 1;			/* did do checksum */
}

static void show_diff_xml_long(FILE *out, char T, const char *type,
			       long old, long new)
{
    xml_start_print(out, type);
    XML_START_PRINT(out, "old");
    fprintf(out, "%ld", old);
    XML_END_PRINT(out, "old");
    XML_START_PRINT(out, "new");
    fprintf(out, "%ld", new);
    XML_END_PRINT(out, "new");
    xml_end_print(out, type);
}

static void show_diff_long(FILE *out, char T, const char *type,
				 long old, long new)
{
    fprintf(out, "%c(%ld:%ld) ", T, old, new);
}

static void show_diff_time(FILE *out, char T, const char *type,
				 time_t old, time_t new)
{
    char	buf[ELC_TIMEBUFSIZ];

    putc(T, out);
    putc('(', out);

    if (! strftime(buf, ELC_TIMEBUFSIZ, "%Y%m%d-%H%M%S", localtime(&old)) )
      DIE("strftime");
    fputs(buf, out);

    putc(':', out);

    if (! strftime(buf, ELC_TIMEBUFSIZ, "%Y%m%d-%H%M%S", localtime(&new)) )
      DIE("strftime");
    fputs(buf, out);

    fputs(") ", out);
}

static void show_diff_xml_octal(FILE *out, char T, const char *type,
				unsigned long old, unsigned long new)
{
    xml_start_print(out, type);
    XML_START_PRINT(out, "old");
    fprintf(out, "0%lo", old);
    XML_END_PRINT(out, "old");
    XML_START_PRINT(out, "new");
    fprintf(out, "0%lo", new);
    XML_END_PRINT(out, "new");
    xml_end_print(out, type);
}

static void show_diff_octal(FILE *out, char T, const char *type,
				  unsigned long old, unsigned long new)
{
    fprintf(out, "%c(0%lo:0%lo) ", T, old, new);
}

static void show_diff_perms(FILE *out, mode_t old, mode_t new)
{
    fputs("p(", out);
    fprintf(out, "0%o", (unsigned int) old);

    putc(':', out);

    fprintf(out, "0%o) ", (unsigned int) new);
}

static void show_diff_type(FILE *out, mode_t old, mode_t new)
{
    fputs("t(", out);
    fprintf(out, "%o", (unsigned int) old);

    putc(':', out);

    fprintf(out, "%o) ", (unsigned int) new);
}

static void show_diffs(integrit_t *it,
		       const char *path, unsigned long diffs,
		       const struct stat *sa, const struct stat *sb)
{
    /* these function pointers use "sho" for "show" to avoid confusion
     * with functions from show.c */
    void (*sho_long)(FILE *out, char T, const char *type, long old, long new);
    void (*sho_time)(FILE *out, char T, const char *type,
		      time_t old, time_t new);
    void (*sho_oct)(FILE *out, char T, const char *type,
		     unsigned long old, unsigned long new);

    if (it->output == OUTPUT_XML) {
      sho_long	 = show_diff_xml_long;
      /* this cast surpresses warnings on some systems */
      sho_time
	= (void (*)(FILE *, char, const char *, time_t, time_t)) show_diff_xml_long;
      sho_oct	 = show_diff_xml_octal;
      XML_CHANGE_START_PRINT(stdout, "stat", path);
    } else {
      sho_long	 = show_diff_long;
      sho_time	 = show_diff_time;
      sho_oct	 = show_diff_octal;
      fprintf(stdout, "changed: %s   ", path);
    }

    if (diffs & RULE_INODE)
      sho_long(stdout, 'i', "inode", sa->st_ino, sb->st_ino);
    if (diffs & RULE_PERMS) {
      if (it->output == OUTPUT_XML) {
	sho_oct(stdout, 'p', "permissions",
		 sa->st_mode & PERM_MASK, sb->st_mode & PERM_MASK);
      } else {
	show_diff_perms(stdout,
                        sa->st_mode & PERM_MASK, sb->st_mode & PERM_MASK);
      }
    }
    if (diffs & RULE_TYPE) {
      if (it->output == OUTPUT_XML) {
	sho_oct(stdout, 't', "type",
		 sa->st_mode & S_IFMT, sb->st_mode & S_IFMT);
      } else {
	show_diff_type(stdout,
                       sa->st_mode & S_IFMT, sb->st_mode & S_IFMT);
      }
    }
    if (diffs & RULE_DEVICETYPE) {
      if (it->output == OUTPUT_XML) {
	sho_oct(stdout, 'd', "devicetype", sa->st_rdev, sb->st_rdev);
      } else {
	show_diff_type(stdout, sa->st_rdev, sb->st_rdev);
      }
    }
    if (diffs & RULE_NLINK)
      sho_long(stdout, 'l', "nlinks", sa->st_nlink, sb->st_nlink);
    if (diffs & RULE_UID)
      sho_long(stdout, 'u', "uid", sa->st_uid, sb->st_uid);
    if (diffs & RULE_GID)
      sho_long(stdout, 'g', "gid", sa->st_gid, sb->st_gid);
    if (diffs & RULE_SIZE)
      sho_long(stdout, 'z', "size", sa->st_size, sb->st_size);
    if (diffs & RULE_ATIME)
      sho_time(stdout, 'a', "access_time", sa->st_atime, sb->st_atime);
    if (diffs & RULE_MTIME)
      sho_time(stdout, 'm', "modification_time", sa->st_mtime, sb->st_mtime);
    if (diffs & RULE_CTIME)
      sho_time(stdout, 'c', "change_time", sa->st_ctime, sb->st_ctime);

    if (it->output == OUTPUT_XML)
      XML_END_PRINT(stdout, "change");

    putc('\n', stdout);
}

static void report_stat_differences(integrit_t *it,
				    fileinfo *currinf, dbinfo *old)
{
    struct stat		*sa	 = &old->stat;
    struct stat		*sb	 = &currinf->dbinf.stat;
    const char *path		 = currinf->path;
    unsigned long	flags	 = fileinfo_ruleset(it, currinf);
    unsigned long	diffs	 = 0;
    
    if ((flags & RULE_INODE)
	&& (sa->st_ino != sb->st_ino))
      diffs	 |= RULE_INODE;
    if ((flags & RULE_PERMS)
	&& ((sa->st_mode & PERM_MASK) != (sb->st_mode & PERM_MASK)))
      diffs	 |= RULE_PERMS;
    if ((flags & RULE_TYPE)
	&& ((sa->st_mode & S_IFMT) != (sb->st_mode & S_IFMT)))
      diffs	 |= RULE_TYPE;
    if ((flags & RULE_DEVICETYPE)
	&& ((S_ISCHR(sa->st_mode) || S_ISBLK(sa->st_mode))
	&& ((S_ISCHR(sb->st_mode) || S_ISBLK(sb->st_mode)))
	&& (sa->st_rdev != sb->st_rdev)))
      diffs	 |= RULE_DEVICETYPE;
    if ((flags & RULE_NLINK)
	&& (sa->st_nlink != sb->st_nlink))
      diffs	 |= RULE_NLINK;
    if ((flags & RULE_UID)
	&& (sa->st_uid != sb->st_uid))
      diffs	 |= RULE_UID;
    if ((flags & RULE_GID)
	&& (sa->st_gid != sb->st_gid))
      diffs	 |= RULE_GID;
    if ((flags & RULE_SIZE)
	&& (sa->st_size != sb->st_size))
      diffs	 |= RULE_SIZE;
    if ((flags & RULE_ATIME)
	&& (sa->st_atime != sb->st_atime))
      diffs	 |= RULE_ATIME;
    if ((flags & RULE_MTIME)
	&& (sa->st_mtime != sb->st_mtime))
      diffs	 |= RULE_MTIME;
    if ((flags & RULE_CTIME) && (! (flags & RULE_RESET_ATIME))
	&& (sa->st_ctime != sb->st_ctime))
      diffs	 |= RULE_CTIME;

    if (diffs) {
      it->exit_status |= INTEGRIT_EXIT_CHANGES;
      show_diffs(it, path, diffs, sa, sb);
    }
}

/* report that the checksum has changed. */
static void report_sumchange(integrit_t *it, const char *path,
			     const unsigned char *old, size_t oldsiz,
			     const unsigned char *new, size_t newsiz)
{
    it->exit_status |= INTEGRIT_EXIT_CHANGES;

    switch (it->output) {
      case OUTPUT_LINES:
	printf("changed: %s   s(", path);
	hexprint(stdout, old, oldsiz);
	putc(':', stdout);
	hexprint(stdout, new, newsiz);
	fputs(")\n", stdout);
	break;
      case OUTPUT_XML:
	XML_CHANGE_START_PRINT(stdout, ALGO_NAME, path);
	XML_START_PRINT(stdout, "old");
	hexprint(stdout, old, oldsiz);	
	XML_END_PRINT(stdout, "old");
	XML_START_PRINT(stdout, "new");
	hexprint(stdout, new, newsiz);	
	XML_END_PRINT(stdout, "new");
	XML_END_PRINT(stdout, "change");
	putc('\n', stdout);
	break;
      default:
	abort();		/* shouldn't happen */
    }
}

static void report_differences(integrit_t *it, fileinfo *currinf)
{
    dbinfo		old;
    struct cdb		*knowndb	 = &it->knowndb;
    size_t		knownsiz	 = cdb_datalen(knowndb);
    char		*path		 = currinf->path;
    unsigned long	flags		 = fileinfo_ruleset(it, currinf);
    
    if (knownsiz != sizeof(old)
	&& knownsiz != sizeof(old.stat))
      die(__FUNCTION__, "Error: bad db entry for file (%s)", path);

    /* in do_check we already did cdb_find (and cdb_datalen) */
    if (cdb_get(knowndb, &old) == -1)
      die(__FUNCTION__, "Error: cdb_get entry for file (%s)", path);

    if (S_ISREG(currinf->dbinf.stat.st_mode) ||
	S_ISLNK(currinf->dbinf.stat.st_mode)) {

      /* if it's a regular file or symlink */
      if (flags & RULE_SUM) {
	/* clean the old checksum buffer when needed */
	if (knownsiz != sizeof(old))
	  memset(old.sum, 0, DIGEST_LENGTH);

	if (! currinf->did_sum)
	  currinf->did_sum	 = do_checksum(it, currinf);

	if (currinf->did_sum &&
	    memcmp(currinf->dbinf.sum, old.sum, sizeof(old.sum)))
	  report_sumchange(it, path, old.sum, sizeof(old.sum),
			   currinf->dbinf.sum, sizeof(currinf->dbinf.sum));
      }
    }
    report_stat_differences(it, currinf, &old);
}

void show_xml_long(FILE *out, const char *type, long val)
{
    xml_start_print(out, type);
    fprintf(out, "%ld", val);
    xml_end_print(out, type);
}
void show_xml_octal(FILE *out, const char *type, unsigned long val)
{
    xml_start_print(out, type);
    fprintf(out, "%lo", val);
    xml_end_print(out, type);
}


/* similar to utils/show.c: show_entry */
static void report_newfile(integrit_t *it, fileinfo *inf)
{
    char		*path	 = inf->path;
    struct stat		*s	 = &inf->dbinf.stat;
    unsigned long	flags	 = fileinfo_ruleset(it, inf);

    switch (it->output) {
      case OUTPUT_XML:
	XML_CHANGE_START_PRINT(stdout, "newfile", path);
	show_xml_octal(stdout, "permissions", s->st_mode & PERM_MASK);
	show_xml_octal(stdout, "type", s->st_mode & S_IFMT);
	if (S_ISBLK(s->st_mode) || S_ISCHR(s->st_mode)) {
	  show_xml_octal(stdout, "devicetype", s->st_rdev);
	}
	show_xml_long(stdout, "uid", s->st_uid);
	show_xml_long(stdout, "gid", s->st_gid);
	show_xml_long(stdout, "size", s->st_size);
	show_xml_long(stdout, "modification_time", s->st_mtime);
	XML_END_PRINT(stdout, "change");
	break;
      case OUTPUT_LINES:
	/* use some extra spaces for alignment
	 * with "missing:" and "changed:" */
	printf("new:     %s   ", path); /* this file wasn't in known db */

	show_octal(stdout, 'p', s->st_mode & PERM_MASK);
	show_octal(stdout, 't', s->st_mode & S_IFMT);
	if (S_ISBLK(s->st_mode) || S_ISCHR(s->st_mode)) {
	  show_octal(stdout, 'd', s->st_rdev);
        }

	show_long(stdout, 'u', s->st_uid);
	show_long(stdout, 'g', s->st_gid);
	show_long(stdout, 'z', s->st_size);
	show_time(stdout, 'm', s->st_mtime);
	break;
      default:
	abort();		/* this shouldn't happen */
	break;
    }
    putc('\n', stdout);
    /* If it's a regular file or symlink, show the checksum */
    if (S_ISREG(s->st_mode) || S_ISLNK(s->st_mode)) {
      if (flags & RULE_SUM) {
        if (! inf->did_sum)
          inf->did_sum	 = do_checksum(it, inf);
        if (inf->did_sum) {
          if (it->output == OUTPUT_XML) {
            char zerosum[DIGEST_LENGTH];
            memset(zerosum, 0, sizeof(zerosum));
            report_sumchange(it, path,
                             (const unsigned char *)zerosum, sizeof(zerosum),
                             inf->dbinf.sum, sizeof(inf->dbinf.sum));
          } else {
            printf("new:     %s   ", path);
            show_checksum(stdout, inf->dbinf.sum, sizeof(inf->dbinf.sum));
            putc('\n', stdout);
          }
        }
      }
    }
}    

inline static void do_check(integrit_t *it, fileinfo *inf)
{
    struct cdb	*db	 = &it->knowndb;
    int		err;

    if ( (err = cdb_find(db, inf->path, inf->pathlen)) == -1)
      die(__FUNCTION__,
	  "Error: looking up file (%s) in known database (%s): %s",
	  inf->path, it->knowndbname, strerror(errno));

    if (!err) {
      it->exit_status |= INTEGRIT_EXIT_CHANGES;
      report_newfile(it, inf);
    } else {
      report_differences(it, inf);
    }
}

inline static int do_update(integrit_t *it, fileinfo *inf)
{
    struct cdb_make	*db	 = &it->currdb;
    dbinfo		*dbinf	 = &inf->dbinf;
    size_t		datasiz;
    
    if (S_ISREG(dbinf->stat.st_mode) ||
	S_ISLNK(dbinf->stat.st_mode)) {
      if (fileinfo_ruleset(it, inf) & RULE_SUM) {
	/* BUGGISH: don't test inf->did_sum, since this function's called
	 *          before do_check (very minorly buggish :)
	 */
	inf->did_sum	 = do_checksum(it, inf);
        if (! inf->did_sum)
	  return 0;
      }
    }

    datasiz	 = inf->did_sum ? sizeof(*dbinf) : sizeof(dbinf->stat);

    if (cdb_put(db, inf->path, inf->pathlen, dbinf, datasiz) == -1)
      DIE("adding record to current-state db");

    return 1;
}

wft_ret_t process_file(const char *path, const struct stat *sb, void *data)
{
    integrit_t	*it		 = (integrit_t *) data;
    fileinfo	inf;		/* initialize before use */
    checkset	*cset;
    size_t	plen		 = strlen(path);
    char	*pathcopy	 = xstrdup(path);
    char	*p		 = pathcopy;
#if defined(DEBUG) && 0
    static int	counter;
#endif

    if (p[0] == '/' && p[1] == '/') {
      ++p;		/* pass the first of double initial slashes */
      --plen;
    }
    
    if (plen > 2
	&& p[plen - 2] == '/'
	&& p[plen - 1] == '.') {
      /* ignore trailing "/." */
      p[plen - 2]	 = '\0';
      plen		 -= 2;
    } else if (plen == 2
	       && p[0] == '/'
	       && p[1] == '.') {
      /* it's "/." */
      p[1]	 = '\0';
      --plen;
    }

#if defined(DEBUG) && 0
    fprintf(stderr, "debug (%s): count (%d) path (%s)\n",
	    __FUNCTION__, ++counter, p); /* debug */
    usleep(5);			/* debug */
#endif

    /* initialize inf now that we've settled on a file path */
    fileinfo_init(&inf, p, plen);

    cset	 = hashtbl_lookup(it->ruleset, p, plen);

    /* see whether checkset includes boolean to ignore the file */
    if (cset && (cset->flags & RULE_IGNORE)) { 
#ifdef	DEBUG
      fprintf(stderr, "debug: ignoring file (%s)\n", p);
#endif
      free(pathcopy);
      return WFT_PRUNE;
    }

    memcpy(&inf.dbinf.stat, sb, sizeof(inf.dbinf.stat));

    if (it->do_update) {
      if (! do_update(it, &inf)) {
	free(pathcopy);
	return WFT_ERROR;
      }
    }

    if (it->do_check)
      do_check(it, &inf);

    free(pathcopy);

    /* see whether the nochildren boolean is true in this checkset */
    if (cset && (cset->flags & RULE_NOCHILDREN)) {
#ifdef	DEBUG
      fprintf(stderr, "debug: no children for file (%s)\n", path);
#endif
      return WFT_PRUNE;
    }

    return WFT_PROCEED;
}
