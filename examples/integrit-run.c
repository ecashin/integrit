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

/* many people ask about adding a feature to integrit whereby
 * multiple roots could be in one config file.  It isn't necessary
 * to add this feature, since you can easily run integrit more
 * than once on multiple config files.
 *
 * this integrit-run program serves as an example of a simple
 * C program that can be statically linked and used to run
 * integrit multiple times on different config files.
 *
 * for many sites, a simple bourne shell script will be
 * sufficient, and this kind of program is overkill.
 *
 * to compile this example, gcc users may use this command:
 *
 *   gcc -W -Wall -g -o integrit-run -static integrit-run.c
 *
 * you will need to modify the parts of the code below that
 * are marked system-specific.
 *
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* SYSTEM-SPECIFIC: modify these for your own purposes */
#define INTEGRIT_BIN "/tmp/inst/sbin/integrit"
#define CONFIG_DIR "/tmp/cfg"

const char *configs[] = {
  "integrit-a.conf",
  "integrit-b.conf",
  "integrit-c.conf",
};
/* END SYSTEM-SPECIFIC */

int run_integrit(const char *ibin, const char *cdir, const char *cfile)
{
    char cpath_buf[1024];
    int status;
    int snprintf_ret = snprintf(cpath_buf, sizeof(cpath_buf),
				"%s/%s", cdir, cfile);

    /* current glibc returns -1 when not enough room, but future ones will
     * return the number of chars that would have been printed (per C99)
     */
    if ((snprintf_ret == -1) || (snprintf_ret > (((int) sizeof(cpath_buf)) - 1))) {
      fputs("integrit-run Error: config file path too long\n", stderr);
      exit(EXIT_FAILURE);
    }

    switch (fork()) {
      case -1:
	perror("fork");
	exit(EXIT_FAILURE);
	break;			/* not reached */
      case 0:
	/* the child */
	execl(ibin, ibin, "-C", cpath_buf, "-c", "-u", NULL);
	perror("execl");
	_exit(EXIT_FAILURE);
	break;			/* not reached */
      default:
	break;
    }
    /* only parent gets here */
    for (;;) {
      if (wait(&status) == -1) {
	perror("wait");
	exit(EXIT_FAILURE);
      }
      if (WIFEXITED(status))
	return WEXITSTATUS(status);
      else if (WIFSIGNALED(status))
	return -(WTERMSIG(status));
      else if (WIFSTOPPED(status))
	continue;		/* keep waiting */
    }

    /* shouldn't ever get here */
    fputs("integrit-run Error: undefined child status\n", stderr);
    exit(EXIT_FAILURE);

    return 0;			/* not reached */
}

int main(void)
{
    int n = sizeof(configs) / sizeof(char *);
    int i;
    FILE *out = stdout;

    for (i = 0; i < n; ++i) {
      int ret = run_integrit(INTEGRIT_BIN, CONFIG_DIR, configs[i]);

      fprintf(out, "integrit-run: integrit on %s finished: ", configs[i]);

      if (ret == 0) {
	  fputs("no changes detected\n", out);
      } else if (ret == 1) {
	fputs("changes were detected\n", out);
      } else if (ret < 0) {
	fprintf(out, "integrit killed by signal %d\n", -ret);
      } else {
	fputs("an error occurred\n", out);
      }
      fflush(out);
    }

    return 0;
}
