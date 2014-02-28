/*
hashtbl - a handy and fast, free hash table implementation
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
#include	<unistd.h>
#include	<errno.h>
#include	"hashtbl.h"
#ifdef	ELC_FIND_LEAKS
#include	"leakfind.h"	/* checking for memory leaks */
#endif

void remove_first_newline(char *chp)
{
    /* remove trailing newlines from a one-line string */
    for( ; *chp; ++chp)
      if(*chp == '\n')
	*chp = '\0';
}

int *copyint(int i)
{
    int	*copy;
    if (! (copy = malloc(sizeof(int))) ) {
      fprintf(stderr, "hashtest Error: malloc int: %s\n",
	      strerror(errno));
      exit(EXIT_FAILURE);
    }
    *copy	 = i;
    return copy;
}
    

int main(void)
{
    int		i	 = 0;
    int		*intp;
    char	buf[BUFSIZ];
    hashtbl_t	h;
    FILE	*words	 = fopen(SYSTEM_DICTIONARY, "r");

    if (! words) {
      fprintf(stderr, "Error: opening words file (%s): %s\n",
	      SYSTEM_DICTIONARY, strerror(errno));
      exit(EXIT_FAILURE);
    }
      
    puts("using an ititial size of 1 for the hashtbl to give it a workout.");
    /* 45402 */
    if (hashtbl_init(&h, 1) == -1) {
      perror("hashtbl_init");
      exit(EXIT_FAILURE);
    }
    puts("about to start test.");
    sleep(1);
    
    while (fgets(buf, BUFSIZ, words)) {
      void *replaced;
      
      remove_first_newline(buf);
      intp	 = copyint(i++);
      if (hashtbl_store(&h, buf, strlen(buf), intp, &replaced) == -1) {
	perror("hashtbl_store");
	exit(EXIT_FAILURE);
      }
      if (replaced)
	free(replaced);
    }
    puts("done entering stuff");
    sleep(1);

    rewind(words);
    while (fgets(buf, BUFSIZ, words)) {
      remove_first_newline(buf);
      if (! (intp = hashtbl_lookup(&h, buf, strlen(buf))) )
	puts("not found");
      else
	printf("found %d with key %s\n", *intp, buf);
    }
    puts("done looking up stuff");
    sleep(1);
    
    rewind(words);
    while (fgets(buf, BUFSIZ, words)) {
      remove_first_newline(buf);
      if (! (intp = hashtbl_remove(&h, buf, strlen(buf))) ) {
	puts("not found");
      } else {
	printf("removed %d with key %s\n", *intp, buf);
	free(intp);
      }
    }

    rewind(words);
    fputs("re-entering stuff ... ", stdout);
    fflush(stdout);
    while (fgets(buf, BUFSIZ, words)) {
      void *replaced;

      remove_first_newline(buf);
      intp	 = copyint(i++);
      if (hashtbl_store(&h, buf, strlen(buf), intp, &replaced) == -1) {
	perror("hashtbl_store");
	exit(EXIT_FAILURE);
      }
      if (replaced)
	free(replaced);
    }
    puts("done.");
    sleep(1);

    fputs("freeing and destroying hashtbl ... ", stdout);
    fflush(stdout);
    hashtbl_free(&h, NULL, NULL);
    hashtbl_destroy(&h);
    puts("done.");

#ifdef	ELC_FIND_LEAKS
    GC_gcollect();		/* have boehm look for leaks */
#endif
    return 0;
}
