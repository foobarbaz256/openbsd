/*	$OpenBSD: leave.c,v 1.8 2002/06/12 06:07:15 mpech Exp $	*/
/*	$NetBSD: leave.c,v 1.4 1995/07/03 16:50:13 phil Exp $	*/

/*
 * Copyright (c) 1980, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1980, 1988, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)leave.c	8.1 (Berkeley) 6/6/93";
#endif
static char rcsid[] = "$OpenBSD: leave.c,v 1.8 2002/06/12 06:07:15 mpech Exp $";
#endif /* not lint */

#include <sys/param.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

void	usage(void);
void	doalarm(u_int secs);

/*
 * leave [[+]hhmm]
 *
 * Reminds you when you have to leave.
 * Leave prompts for input and goes away if you hit return.
 * It nags you like a mother hen.
 */
int
main(argc, argv)
	int argc;
	char **argv;
{
	u_int secs;
	int hours, minutes;
	char c, *cp;
	struct tm *t;
	time_t now;
	int plusnow;
	char buf[50];

	if (argc < 2) {
#define	MSG1	"When do you have to leave? "
		(void)write(STDOUT_FILENO, MSG1, sizeof(MSG1) - 1);
		cp = fgets(buf, sizeof(buf), stdin);
		if (cp == NULL || *cp == '\n')
			exit(0);
	} else if (argc > 2)
		usage();
	else
		cp = argv[1];

	if (*cp == '+') {
		plusnow = 1;
		++cp;
	} else {
		plusnow = 0;
		(void)time(&now);
		t = localtime(&now);
	}

	for (hours = 0; (c = *cp) && c != '\n'; ++cp) {
		if (!isdigit(c))
			usage();
		hours = hours * 10 + (c - '0');
	}
	minutes = hours % 100;
	hours /= 100;

	if (minutes < 0 || minutes > 59)
		usage();
	if (plusnow)
		secs = hours * 60 * 60 + minutes * 60;
	else {
		if (hours > 23)
			usage();
		if (t->tm_hour > hours || 
		    (t->tm_hour == hours && t->tm_min >= minutes)) {
			/* determine 24 hours mode */
		    	if (hours >= 13)
				hours += 24;
			else
				hours += 12;
		}

		secs = (hours - t->tm_hour) * 60 * 60;
		secs += (minutes - t->tm_min) * 60;
	}
	doalarm(secs);
	exit(0);
}

void
doalarm(secs)
	u_int secs;
{
	int bother;
	time_t daytime;
	pid_t pid;

	if ((pid = fork())) {
		(void)time(&daytime);
		daytime += secs;
		printf("Alarm set for %.16s. (pid %ld)\n",
		    ctime(&daytime), (long)pid);
		exit(0);
	}
	sleep((u_int)2);		/* let parent print set message */

	/*
	 * if write fails, we've lost the terminal through someone else
	 * causing a vhangup by logging in.
	 */
#define	FIVEMIN	(5 * 60)
#define	MSG2	"\07\07You have to leave in 5 minutes.\n"
	if (secs >= FIVEMIN) {
		sleep(secs - FIVEMIN);
		if (write(STDOUT_FILENO, MSG2, sizeof(MSG2) - 1) != sizeof(MSG2) - 1)
			exit(0);
		secs = FIVEMIN;
	}

#define	ONEMIN	(60)
#define	MSG3	"\07\07Just one more minute!\n"
	if (secs >= ONEMIN) {
		sleep(secs - ONEMIN);
		if (write(STDOUT_FILENO, MSG3, sizeof(MSG3) - 1) != sizeof(MSG3) - 1)
			exit(0);
	}

#define	MSG4	"\07\07Time to leave!\n"
	for (bother = 10; bother--;) {
		sleep((u_int)ONEMIN);
		if (write(STDOUT_FILENO, MSG4, sizeof(MSG4) - 1) != sizeof(MSG4) - 1)
			exit(0);
	}

#define	MSG5	"\07\07That was the last time I'll tell you.  Bye.\n"
	(void)write(STDOUT_FILENO, MSG5, sizeof(MSG5) - 1);
	exit(0);
}

void
usage()
{
	fprintf(stderr, "usage: leave [[+]hhmm]\n");
	exit(1);
}
