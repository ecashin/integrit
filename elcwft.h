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
#ifndef	ELC_WFT_H
#define	ELC_WFT_H

#define	WFT_VERBOSE		0x01
#define	WFT_STOP_ON_ERR		0x02

typedef enum wft_ret_t {
  WFT_ERROR	 = -1,
  WFT_PROCEED	 = 0,
  WFT_SOME_ERR,
  WFT_PRUNE
} wft_ret_t;

/* callback type */
typedef
wft_ret_t (*wft_cb_t)(const char *fname, const struct stat *sb, void *data);

typedef struct wft_context_struct {
  char		*rootname;
  wft_cb_t	callback;
  void		*cb_data;
  unsigned int	options;
} wft_context_t;

#endif

