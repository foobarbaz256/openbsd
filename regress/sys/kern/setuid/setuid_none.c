/*	$OpenBSD: setuid_none.c,v 1.1 2014/08/27 07:36:14 blambert Exp $	*/
/*
 *	Written by Bret Stephen Lambert <blambert@openbsd.org> 2014
 *	Public Domain.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/sysctl.h>
#include <sys/wait.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>

#include "setuid_regress.h"

int
main(int argc, char *argv[])
{
	struct kinfo_proc	 kproc;
	uid_t			 uid;

	uid = getuid();

	checkuids(uid, uid, uid, "getuid");

	/* should only respond to setuid upon exec */
	if (issetugid())
		errx(1, "process incorrectly marked as issetugid()");

	if (read_kproc_pid(&kproc, getpid()) == -1)
		err(1, "kproc read failed");

	if (kproc.p_psflags & PS_SUGID)
		errx(1, "PS_SUGID incorrectly set");
	if (kproc.p_psflags & PS_SUGIDEXEC)
		errx(1, "PS_SUGIDEXEC incorrectly set");

	exit(0);
}
