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
#ifndef	ELC_LEAKFIND_H
#define	ELC_LEAKFIND_H

#define	GC_DEBUG
#include	"gc.h"
#define	malloc(bytes)	GC_MALLOC(bytes)
#define	realloc(oldmem, bytes)	GC_REALLOC(oldmem, bytes)

/* user can define DISABLE_MEMORY_FREE elsewhere or change the zero to
 * a one if they want to cause memory leaks for testing purposes
 * when using "./configure --with-leakfind=~/build/gc-5.13".
 */
#ifndef DISABLE_MEMORY_FREE
#define DISABLE_MEMORY_FREE 0
#endif
#if DISABLE_MEMORY_FREE
#warning *** undefining memory freeing for testing leak detection ***
#define free(mem)
#define	CORE_free(mem)
#else
#define free(mem)	GC_FREE(mem)
#define	CORE_free(mem)	free(mem)
#endif

#endif
