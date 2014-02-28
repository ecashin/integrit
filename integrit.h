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
#ifndef	INT_INTEGRIT_H
#define	INT_INTEGRIT_H

enum integrit_output {
  OUTPUT_LINES,			/* human-readable lines output */
  OUTPUT_XML			/* XML output */
};  

enum integrit_exit_status {
  INTEGRIT_EXIT_NOCHANGE = 0,
  INTEGRIT_EXIT_CHANGES = 1,
  INTEGRIT_EXIT_FAILURE = 2
};
  
typedef struct integrit {
  /* program state information */
  enum integrit_exit_status exit_status;

  /* options */
  char			*conffile;	/* from argv, so don't free */
  char			*knowndbname;
  struct cdb		knowndb;
  char			*currdbname;
  struct cdb_make	currdb;
  char			*root;
  hashtbl_t		*ruleset;
  int			verbose;
  unsigned long		default_flags;
  enum integrit_output	output;	/* type of output */
  unsigned		stop_on_err: 1;	/* stop on errors */
  unsigned		do_check: 1;	/* test files against known-state db */
  unsigned		do_update: 1;	/* generate current-state db */
} integrit_t;

#endif
