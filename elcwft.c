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
/*
 * Looks like struct dirent's on my Linux box always have DT_UNKNOWN
 * as their d_type member:
 * 
 * if (result)
 *    printf("type: %d %s DT_UNKNOWN\t",
 *           result->d_type, (result->d_type == DT_UNKNOWN) ? "==" : "!=");
 *
 * ... as the docs say they might.
 */

/* This file tree walker silently ignores files that vanish between the
 * time the readdir is done and the time the lstat is done.
 */
#include	<config.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<dirent.h>
#include	<errno.h>
#include	"xstrdup.h"
#include	"xstradd.h"
#include	"elcwft.h"
#include	"elcwft_p.h"
#include	"elcerror.h"
#include	"elcerror_p.h"
#ifdef		ELC_FIND_LEAKS
#include	"leakfind.h"
#endif

typedef struct wftnode {		/* walk file tree node */
  DIR			*dir;
  struct wftnode	*next;
} wftnode;

inline static int is_directory(const char *path, struct stat *sb)
{
    if (lstat(path, sb) == -1)
      return -1;
    return S_ISDIR(sb->st_mode);
}

inline static wftnode *endoflist(wftnode *head)
{
    wftnode	*np;
    
    for (np = head; np->next; np = np->next)
      if (np->next && (! np->next->dir)) {
	/* clean up used node */
	free(np->next);
	np->next	 = NULL;
	break;
      }
    return np;
}

inline static int backupdir(char *dirname)
{
    char	*lastslash	 = strrchr(dirname, '/');
    if (! lastslash) {
      errno	 = EINVAL;
      return -1;
    }
    *lastslash	 = '\0';

    return 0;
}

/* the function below is tied to the internal workings of
 * walk_file_tree, and it helps make the big loop in walk_file_tree
 * more readable/managable. */
inline static int finishdir(wftnode *np, wftnode *head,
			    int *done, char *curr_dirname)
{
    if (np == head) {
      *done	 = 1;
      closedir(np->dir);
      return 0;
    }

    closedir(np->dir);
    np->dir	 = NULL; /* signal to endoflist that we're done here */
    if (backupdir(curr_dirname) == -1)
      return -1;

    return 0;
}

/* the old parameters were:
 * const char *rootname, wft_cb_t callback, void *cbdata */
int walk_file_tree(wft_context_t *ctx)
{
    DIR			*root	 = opendir(ctx->rootname);
    wftnode		dirs;
    wftnode		*np;
    char		*curr_dirname	 = xstrdup(ctx->rootname);
    char		*path		 = NULL;
    struct dirent	*result;
    struct stat		statbuf;
    int			ret, cb_ret;
    int			done	 = 0;
    int			err	 = 0;

    dirs.dir	 = root;
    dirs.next	 = NULL;

    if (! root)
      return WFT_ERROR;

    while (! done) {
      /* find the deepest directory and maintain list */
      np	 = endoflist(&dirs);

      /* while we don't want to go deeper into file tree */
      while (! np->next) {
	result	 = readdir(np->dir);
	if (!result) {
	  /* no more entries in this dir */
	  if (finishdir(np, &dirs, &done, curr_dirname) == -1)
	    return WFT_ERROR;
	  break;
	}
	if (!strcmp(result->d_name, ".") || !strcmp(result->d_name, "..")) {
	  /* skip the current and parent dir */
	  continue;
	}

	if (path)
	  free(path);
	path	= xstradd(curr_dirname, "/", result->d_name, NULL);
	if ((ret = is_directory(path, &statbuf)) == -1) {
	  if (errno == ENOENT)	/* no such file or directory */
	    continue;		/* silently skip the vanished file */
	  else
	    return WFT_ERROR;
	}
	cb_ret	= ctx->callback(path, &statbuf, ctx->cb_data);
	switch (cb_ret) {
	case WFT_ERROR:
	  if (ctx->options & WFT_STOP_ON_ERR)
	    return WFT_ERROR;
          err = 1;

	case WFT_PRUNE:
	  continue;
	}

	if (ret == 1) { /* directory */
	  if (! (np->next = malloc(sizeof(* np->next))) )
	    return WFT_ERROR;
	  if (! (np->next->dir = opendir(path)) ) {
	    warn(__FUNCTION__, "Warning: cannot open directory (%s): %s",
		 path, strerror(errno));
	    if (ctx->options & WFT_STOP_ON_ERR)
	      return WFT_ERROR;
	    err = 1;
	    continue;
	  }
	  free(curr_dirname);
	  curr_dirname		 = xstrdup(path);
	  np->next->next	 = NULL;
	}
      } /* end iterating through a given dir */
    } /* end while not done */

    if (path)
      free(path);
    free(curr_dirname);
    return (err? WFT_SOME_ERR: WFT_PROCEED);
}

