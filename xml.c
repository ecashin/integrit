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
#include	<time.h>
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
#include	"hashtbl/hashtbl.h"
#include	"integrit.h"
#ifdef		ELC_FIND_LEAKS
#include	"leakfind.h"
#endif

#define	XML_VERSION	"1.0"

void xml_declaration(FILE *out)
{
    fputs("<?xml version=\"" XML_VERSION "\" standalone=\"yes\"?>\n", out);
}

void xml_dtd(FILE *out)
{
    const static char dtd[]	 = 
      "<!DOCTYPE report [\n"
      "<!ELEMENT options (output, conffile, knowndb, currentdb, root, check, update)>\n"
      "<!ELEMENT output (#PCDATA)>\n"
      "<!ELEMENT conffile (#PCDATA)>\n"
      "<!ELEMENT knowndb (#PCDATA)>\n"
      "<!ELEMENT currentdb (#PCDATA)>\n"
      "<!ELEMENT root (#PCDATA)>\n"
      "<!ELEMENT check (#PCDATA)>\n"
      "<!ELEMENT update (#PCDATA)>\n"
      "\n"
      "<!ELEMENT change (inode?, permissions?, nlinks?, uid?, gid?, size?, access_time?, modification_time?)>\n"
      "<!ATTLIST change type CDATA #REQUIRED\n"
      "                 file CDATA #REQUIRED>\n"
      "<!ELEMENT inode (old, new)>\n"
      "<!ELEMENT permissions (old, new)>\n"
      "<!ELEMENT nlinks (old, new)>\n"
      "<!ELEMENT uid (old, new)>\n"
      "<!ELEMENT gid (old, new)>\n"
      "<!ELEMENT size (old, new)>\n"
      "<!ELEMENT access_time (old, new)>\n"
      "<!ELEMENT modification_time (old, new)>\n"
      "<!ELEMENT old (#PCDATA)>\n"
      "<!ELEMENT new (#PCDATA)>\n"
      "\n"
      "<!ELEMENT missing (#PCDATA)>\n"
      "<!ELEMENT checksum (#PCDATA)>\n"
      "<!ATTLIST checksum type CDATA #REQUIRED\n"
      "                   file CDATA #REQUIRED>\n"
      "]>\n";

    fputs(dtd, out);
}

void xml_putc(FILE *out, int ch)
{
    switch (ch) {
      case '<':
	fputs("&lt;", out);
	break;
      case '>':
	fputs("&gt;", out);
	break;
      case '&':
	fputs("&amp;", out);
	break;
      default:
	putc(ch, out);
	break;
    }
}

void xml_start_print(FILE *out, const char *tag)
{
    fprintf(out, "<%s>", tag);
}

void xml_end_print(FILE *out, const char *tag)
{
    fprintf(out, "</%s>", tag);
}

void xml_print(FILE *out, const char *buf)
{
    for (; *buf; ++buf)
      xml_putc(out, *buf);
}

void xml_start_report(FILE *out, const integrit_t *it)
{
    fprintf(out, "<report date=\"%ld\" integrit_version=\"%s\">\n",
	    (long) time(NULL), INTEGRIT_VERSION);
}

