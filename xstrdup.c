/*
hashtbl
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
#include	<syslog.h>
#include	<errno.h>
#include	"elcerror_p.h"
#include	"elcerror.h"
#include	"xstrdup.h"
#ifdef	ELC_FIND_LEAKS
#include	"leakfind.h"	/* checking for memory leaks */
#endif

char *xstrdup(const char *str)
{
    char	*newstr	 = malloc(sizeof(char) * (strlen(str) + 1));
    if (! newstr)
      DIE("malloc");
    strcpy(newstr, str);
    return newstr;
}

