/*
 * Copyright (c) 1994 Mats O Jansson <moj@stacken.kth.se>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef LINT
static char rcsid[] = "$Id: stdhosts.c,v 1.1 1995/10/23 07:46:23 deraadt Exp $";
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <ctype.h>

static int read_line(fp, buf, size)
FILE *fp;
char *buf;
int size;
{
	int done = 0;

	do {
		while (fgets(buf, size, fp)) {
			int len = strlen(buf);
			done += len;
			if (len > 1 && buf[len-2] == '\\' &&
					buf[len-1] == '\n') {
				int ch;
				buf += len - 2;
				size -= len - 2;
				*buf = '\n'; buf[1] = '\0';
				/*
				 * Skip leading white space on next line
				 */
				while ((ch = getc(fp)) != EOF &&
					isascii(ch) && isspace(ch))
						;
				(void) ungetc(ch, fp);
			} else {
				return done;
			}
		}
	} while (size > 0 && !feof(fp));

	return done;
}

int
main (argc,argv)
int argc;
char *argv[];
{
  FILE	*data_file;
  char	 data_line[1024];
  int	 usage = 0;
  int	 line_no = 0;
  int	 len;
  char	*p,*k,*v;
  struct in_addr host_addr;

  if (argc > 2) {
    usage++;
  }

  if (usage) {
    fprintf(stderr,
	    "%s",
	    "usage: stdhosts [file]\n");
    exit(1);
  }

  if (argc == 2) {
    data_file = fopen(argv[argc-1], "r");
  } else {
    data_file = stdin;
  }
  
  while (read_line(data_file,data_line,sizeof(data_line))) {
    
    line_no++;
    len = strlen(data_line);
    
    if (len > 0) {
      if (data_line[0] == '#')
	continue;
    }

    /*
     * Check if we have the whole line
     */ 

    if (data_line[len-1] != '\n') {
      if (argc == 2) {
	fprintf(stderr, "line %d in \"%s\" is too long", line_no, argv[1]);
      } else {
	fprintf(stderr, "line %d in \"stdin\" is too long", line_no);
      }
    } else {
      data_line[len-1] = '\0';
    }

    p = (char *) &data_line;

    k  = p;					/* save start of key */
    while (!isspace(*p)) { p++; };		/* find first "space" */
    while (isspace(*p)) { *p = '\0'; p++; };	/* replace space with <NUL> */
    
    v = p;					/* save start of value */
    while(*p != '\0') { p++; };			/* find end of string */

    (void)inet_aton(k,&host_addr);
    printf("%s %s\n",inet_ntoa(host_addr),v);

  }

  return(0);
  
}
