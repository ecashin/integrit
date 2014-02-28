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
#include	<limits.h>
#include	<errno.h>
#include	"checkset.h"
#include	"rules.h"
#include	"elcerror.h"
#include	"elcerror_p.h"
#include	"checkset_p.h"
#ifdef		ELC_FIND_LEAKS
#include	"leakfind.h"
#endif

void checkset_init(checkset *c)
{
    c->flags		 = 0;
    c->switches		 = NULL;
}

void ni_checkset_init(ni_checkset *nic)
{
    nic->regular.flags		 = 0;
    nic->regular.switches	 = NULL;
    nic->ni_switches		 = NULL;
}

inline static void
checkset_warn_overwrite(checkset *old, const char *filename)
{
    fprintf(stderr, "Warning: overwriting old checkset (");
    checkset_show(stderr, old);
    fprintf(stderr, ") for file (%s)\n", filename);
}

/* We must free and overwrite the old checkset for the simple case,
 * where both checksets are regular, cascading checksets.
 *
 * If the new one is non-inheriting and the old one is regular, we
 * take the cascading stuff from the old one and add it to the new one.
 *
 * If the new one is regular and the old one is non-inheriting, we override
 * the regular part of the old one and free the new one, returning the
 * updated old one.
 *
 * If both are non-inheriting, we free and overwrite the old checkset,
 * returning the new one.  We keep the cascading part of the old checkset
 * if the new one doesn't have one.
 */
checkset *checkset_merge(checkset *new, checkset *old,
			 const char *filename)
{
    char	new_is_ni	 = new->flags & RULE_NOINHERIT;
    char	old_is_ni	 = old->flags & RULE_NOINHERIT;
    ni_checkset	*nic;

    if (! filename)
      filename	 = "(null)";

#if	DEBUG
    fprintf(stderr, "debug: merging rules for (%s) old(", filename);
    checkset_show(stderr, old);
    fputs(") + new(", stderr);
    checkset_show(stderr, new);
    fputs(") ", stderr);
#endif

    if (! new_is_ni && ! old_is_ni) {
      checkset_warn_overwrite(old, filename);
      checkset_free(old);
    } else if (new_is_ni && ! old_is_ni) {
      nic	 = (ni_checkset *) new;
      /* sanity check: since the new one is non-inheriting, there
       * shouldn't be any regular switches */
      if (nic->regular.switches)
	die(__FUNCTION__,
	    "Error: cascading switches in non-inheriting "
	    "checkset for file(%s)", filename);

      /* just juggle pointers to avoid copying. */
      nic->regular.switches	 = old->switches;
      old->switches		 = NULL;
      /* if old OR new wants ignoring or nochildren-ing, they get it */
      nic->regular.flags	|= old->flags & RULE_NOINHERIT;
      nic->regular.flags	|= old->flags & RULE_NOCHILDREN;
      checkset_free(old);
    } else if (! new_is_ni && old_is_ni) {
      nic	 = (ni_checkset *) old;
      if (nic->regular.switches) {
	checkset_warn_overwrite(old, filename);
	free(nic->regular.switches);
      }
      nic->regular.switches	 = new->switches;
      new->switches		 = NULL;

      /* if old OR new wants ignoring or nochildren-ing, they get it */
      nic->regular.flags	|= new->flags & RULE_NOINHERIT;
      nic->regular.flags	|= new->flags & RULE_NOCHILDREN;
      checkset_free(new);
      new	 = old;
    } else if (new_is_ni && old_is_ni) {
      ni_checkset *new_nic	 = (ni_checkset *) new;
      ni_checkset *old_nic	 = (ni_checkset *) old;

      fprintf(stderr,
	      "Warning: overwriting non-inheriting part of old checkset (");
      checkset_show(stderr, old);
      fprintf(stderr, ") for file (%s)\n", filename);
      
      if (! new_nic->regular.switches) {
	if (old_nic->regular.switches) {
	  new_nic->regular.switches	 = old_nic->regular.switches;
	  old_nic->regular.switches	 = NULL;
	} else {
	  fprintf(stderr,
		  "Warning: overwriting cascading part of old checkset (");
	  checkset_show(stderr, old);
	  fprintf(stderr, ") for file (%s)\n", filename);
	}
      }

      checkset_free(old);
    } else {
      die(__FUNCTION__,
	  "Error: unexpected condition: new checkset %s & old %s",
	  new_is_ni ? "doesn't inherit" : "cascades",
	  old_is_ni ? "doesn't inherit" : "cascades");
    }
    
#if	DEBUG
    fputs("= (", stderr);
    checkset_show(stderr, new);
    fputs(")\n", stderr);
#endif

    return new;
}

void checkset_show(FILE *out, checkset *c)
{
    ni_checkset	*nic;

    if (c->flags & RULE_IGNORE)
      putc('!', out);
    if (c->flags & RULE_NOCHILDREN)
      putc('=', out);
    if (c->switches)
      fputs(c->switches, out);
    if (c->flags & RULE_NOINHERIT) {
      nic	 = (ni_checkset *) c;
      fprintf(out, "; $%s", nic->ni_switches);
    }
}

/* this can handle non-inheriting and cascading checksets properly */
void checkset_free(checkset *c)
{
    ni_checkset	*nic;

    if (c->switches)
      free(c->switches);
    if (c->flags & RULE_NOINHERIT) {
      nic	 = (ni_checkset *) c;
      if (nic->ni_switches)
	free(nic->ni_switches);
    }
    free(c);
}

checkset *checkset_new()
{
    checkset	*new	 = malloc(sizeof(checkset));

    if (! new)
      DIE("malloc checkset");
    checkset_init(new);

    return new;
}

ni_checkset *ni_checkset_new()
{
    ni_checkset	*new	 = malloc(sizeof(ni_checkset));

    if (! new)
      DIE("malloc ni_checkset");
    ni_checkset_init(new);

    return new;
}
