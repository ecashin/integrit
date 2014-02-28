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
#include	<stdarg.h>
#include	<errno.h>
/* support platforms that don't yet conform to C99 */
#if	HAVE_STDINT_H
#include	<stdint.h>
#elif	HAVE_INTTYPES_H
#include	<inttypes.h>
#else
#error No stdint.h or inttypes.h found.
#endif
#include	"cdb.h"
#include	"cdb_make.h"
#include	"hashtbl.h"
#include	"integrit.h"
#ifdef	ELC_FIND_LEAKS
#include	"leakfind.h"
#endif

#ifndef	PROGNAME
#define	PROGNAME	"test"
#endif
#ifndef	fatal_error
#define	fatal_error(s)	do {					\
    fprintf(stderr, PROGNAME ": Error in %s (%s): %s\n",	\
	    __FUNCTION__, s, strerror(errno));			\
    exit(INTEGRIT_EXIT_FAILURE);				\
} while (0)
#endif

char *xstradd(const char *str1, ...)
{
    va_list	ap;		/* arg pointer */
    char	*str;
    char	*newstr;
    int		totalchars	 = 1;

    totalchars	 += strlen(str1);

    va_start(ap, str1);
    while ( (str = va_arg(ap, char *)) )
      totalchars         += strlen(str);

    newstr	 = malloc(sizeof(char) * totalchars);
    if (! newstr)
      fatal_error("malloc new string");

    strcpy(newstr, str1);
    
    va_start(ap, str1);
    while ( (str = va_arg(ap, char *)) )
      strcat(newstr, str);

    va_end(ap);
#if defined	ELC_FIND_LEAKS && 0
    fprintf(stderr, "alloc %d (%s) in %s\n",
	    strlen(newstr), newstr, __FUNCTION__);
#endif
    return newstr;
}
