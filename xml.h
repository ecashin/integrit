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
#ifndef	ELC_XML_H
#define	ELC_XML_H

#define	XML_START(tag)	("<" tag ">")
#define	XML_END(tag)	("</" tag ">")
#define	XML_START_PRINT(out, tag)	(fprintf(out, XML_START(tag)))
#define	XML_END_PRINT(out, tag)	(fprintf(out, XML_END(tag)))
#define	XML_ELEMENT_PRINT(out, tag, content) do {	\
    fputs("<" tag ">", (out));				\
    xml_print((out), (content));			\
    fputs("</" tag ">\n", (out));			\
} while (0);
#define	XML_CHANGE_START_PRINT(out, type, path) do {	\
    fprintf(out, "<change type=\"%s\" file=\"", type);	\
    xml_print(out, path);				\
    fprintf(out, "\">");				\
} while (0)
#define XML_MISSING_START_PRINT(out, path) do {         \
    fprintf(out, "<missing file=\"");                   \
    xml_print(out, path);                               \
    fprintf(out, "\">");                                \
} while (0)

#endif
