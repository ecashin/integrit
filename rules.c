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
#include	<ctype.h>
#include	<limits.h>
/* support platforms that don't yet conform to C99 */
#if	HAVE_STDINT_H
#include	<stdint.h>
#elif	HAVE_INTTYPES_H
#include	<inttypes.h>
#else
#error No stdint.h or inttypes.h found.
#endif
#include	"hashtbl.h"
#include	"xstrdup.h"
#include	"cdb.h"
#include	"cdb_make.h"
#include	"integrit.h"
#include	"elcerror.h"
#include	"rules.h"
#include	"checkset.h"
#include	"elcerror_p.h"
#include	"rules_p.h"
#ifdef		ELC_FIND_LEAKS
#include	"leakfind.h"
#endif

inline static void binprint(void *data, size_t n)
{
    char	*d	 = (char *) data;
    char	c;
    int		i;

    while (n--) {
      c	 = d[n];
      for (i = CHAR_BIT; i; --i)
	putchar((c >> (i - 1) & 1) ? '1' : '0');
    }
}

unsigned char2rulebit(unsigned char c)
{
    /* SsIiPpTtDdLlUuGgZzAaMmCcRr */
    switch (c) {
      case 's':
      case 'S':
	return	RULE_SUM;	break;
      case 'I':
      case 'i':
	return	RULE_INODE;	break;
      case 'p':
      case 'P':
	return	RULE_PERMS;	break;
      case 't':
      case 'T':
	return	RULE_TYPE;	break;
      case 'd':
      case 'D':
	return	RULE_DEVICETYPE;break;
      case 'L':
      case 'l':
	return	RULE_NLINK;	break;
      case 'U':
      case 'u':
	return	RULE_UID;	break;
      case 'G':
      case 'g':
	return	RULE_GID;	break;
      case 'Z':
      case 'z':
	return	RULE_SIZE;	break;
      case 'A':
      case 'a':
	return	RULE_ATIME;	break;
      case 'M':
      case 'm':
	return	RULE_MTIME;	break;
      case 'C':
      case 'c':
	return	RULE_CTIME;	break;
      case 'R':
      case 'r':
	return	RULE_RESET_ATIME;	break;
      default:
	abort();		/* shouldn't happen.  dump core */
	break;
    };
}

inline static void
set_flags_for_switches(unsigned long *flags, const char *switches)
{
    const char	*p	 = switches;
    
    for (; *p; ++p) {
      if (isupper((unsigned char) *p))
	*flags	 &= ~char2rulebit(*p); /* turn off */
      else
	*flags	 |= char2rulebit(*p); /* turn on */
    }
}

static void rule_override(unsigned long *flags, checkset *rules)
{
    if (rules->flags & RULE_IGNORE) {
      *flags	 |= RULE_IGNORE;
      return;			/* nothing else matters */
    }

    if (rules->flags & RULE_NOCHILDREN)
      *flags	 |= RULE_NOCHILDREN;

    /* there may not be any regular (cascading) switches if this 
     * checkset is non-inheriting.
     */
    if (rules->switches)
      set_flags_for_switches(flags, rules->switches);
}

unsigned long rules_for_path(integrit_t *it, const char *path_orig)
{
    char	*path	 = xstrdup(path_orig);
    char	*p	 = path; /* pointer for running along path */
    char	*root	 = it->root;
    checkset	*thisrule;	/* for each level of the path */
    hashtbl_t		*rules	 = it->ruleset;
    unsigned long	flags	 = it->default_flags;

    if (strstr(path, root) != path)
      die(__FUNCTION__, "Error: root not found in path (%s)", path);
    p	 += strlen(root);

    for (;;) {
      char save	 = *p;
      *p	 = '\0';
#if 0				/* debug */
      fputs(" orig: ", stdout);
      puts(path_orig);
      puts(path);
#endif
      if ( (thisrule = hashtbl_lookup(rules, path, strlen(path))) ) {
	rule_override(&flags, thisrule); /* override *r with *thisrule */
	if (thisrule->flags & RULE_NOINHERIT) {
	  ni_checkset *nic	 = (ni_checkset *) thisrule;
	  if (! strcmp(path, path_orig))
	    set_flags_for_switches(&flags, nic->ni_switches);
	}
      }
      
      if (! (*p++ = save) )	/* reset the separator */
	break;
      while (*p && (*p != '/'))	/* find the next slash */
	++p;
    }

    free(path);
    return flags;
}

