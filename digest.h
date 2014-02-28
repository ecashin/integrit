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
/* this file contains the declaration of the digest functions and name */

#ifndef	ELC_DIGEST_H
#define	ELC_DIGEST_H

#include "gnupg/rmd160.h"

#define DIGEST_NAME "RMD160"
#define DIGEST_LENGTH RMD160_DIGEST_LENGTH
#define DIGEST_CONTEXT RMD160_CONTEXT

#define digest_init(context) rmd160_init(context)
#define digest_write(context, buf, len) rmd160_write(context, buf, len)
#define digest_final(context) rmd160_final(context)
#define digest_read(context) ((context)->buf)

#endif
