/*	$OpenBSD: perm.c,v 1.1 1997/03/01 23:40:12 millert Exp $	*/

/* 
 * perm.c - check user permission for at(1)
 * Copyright (C) 1994  Thomas Koenig
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author(s) may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* System Headers */

#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Local headers */

#include "at.h"
#include "panic.h"
#include "pathnames.h"
#include "privs.h"

/* File scope variables */

#ifndef lint
static char rcsid[] = "$OpenBSD: perm.c,v 1.1 1997/03/01 23:40:12 millert Exp $";
#endif

/* Function declarations */

static int check_for_user __P((FILE *, const char *));

/* Local functions */

static int
check_for_user(fp, name)
	FILE *fp;
	const char *name;
{
	char *buffer;
	size_t len;
	int found = 0;

	len = strlen(name);
	if ((buffer = malloc(len + 2)) == NULL)
		panic("Insufficient virtual memory");

	while (fgets(buffer, len + 2, fp) != NULL) {
		if (strncmp(name, buffer, len) == 0 && buffer[len] == '\n') {
			found = 1;
			break;
		}
	}
	(void)fclose(fp);
	free(buffer);
	return (found);
}


/* Global functions */

int
check_permission()
{
	FILE *fp;
	uid_t uid = geteuid();
	struct passwd *pentry;

	if (uid==0)
		return 1;

	if ((pentry = getpwuid(uid)) == NULL) {
		perror("Cannot access user database");
		exit(EXIT_FAILURE);
	}

	PRIV_START

	fp = fopen(_PATH_AT_ALLOW, "r");

	PRIV_END

	if (fp != NULL) {
		return (check_for_user(fp, pentry->pw_name));
	} else {
		PRIV_START

		fp = fopen(_PATH_AT_DENY, "r");

		PRIV_END

		if (fp != NULL)
			return (!check_for_user(fp, pentry->pw_name));
		else
			perror(_PATH_AT_DENY);
	}
	return (0);
}
