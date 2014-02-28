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
#include	<stdarg.h>
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
#include	"elcerror.h"
#include	"elcerror_p.h"
#ifdef		ELC_FIND_LEAKS
#include	"leakfind.h"
#endif

#ifndef	PROGNAME
#error define PROGNAME before compiling, e.g.,
#error gcc -Wall -DPROGNAME='"foobar"' -c elcerror.c
#endif

void die(const char *function, const char *fmt, ...)
{
    va_list	args;

    fprintf(stderr, "%s (%s): ", PROGNAME, function);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    va_end(args);
    exit(INTEGRIT_EXIT_FAILURE);
}

void warn(const char *function, const char *fmt, ...)
{
    va_list	args;

    fprintf(stderr, "%s (%s): ", PROGNAME, function);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    va_end(args);
}
