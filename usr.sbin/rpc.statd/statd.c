/*	$OpenBSD: statd.c,v 1.2 2015/01/16 06:40:20 deraadt Exp $	*/

/*
 * Copyright (c) 1997 Christos Zoulas. All rights reserved.
 * Copyright (c) 1995
 *	A.R. Gordon (andrew.gordon@net-tel.co.uk).  All rights reserved.
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
 *	This product includes software developed for the FreeBSD project
 *	This product includes software developed by Christos Zoulas.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ANDREW GORDON AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/* main() function for status monitor daemon.  Some of the code in this	*/
/* file was generated by running rpcgen /usr/include/rpcsvc/sm_inter.x	*/
/* The actual program logic is in the file procs.c			*/

#include <sys/wait.h>

#include <err.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <db.h>

#include <rpc/rpc.h>

#include "statd.h"

struct sigaction sa;
int     	debug = 0;		/* Controls syslog() for debug msgs */
int     	_rpcsvcdirty = 0;	/* XXX ??? */
static DB	*db;			/* Database file */

Header		 status_info;

static char undefdata[] = "\0\1\2\3\4\5\6\7";
static DBT undefkey = {
	undefdata,
	sizeof(undefdata)
};
extern char *__progname;

/* statd.c */
static int walk_one(int (*fun )(DBT *, HostInfo *, void *), DBT *, DBT *, void *);
static int walk_db(int (*fun )(DBT *, HostInfo *, void *), void *);
static int reset_host(DBT *, HostInfo *, void *);
static int check_work(DBT *, HostInfo *, void *);
static int unmon_host(DBT *, HostInfo *, void *);
static int notify_one(DBT *, HostInfo *, void *);
static void init_file(char *);
static int notify_one_host(char *);
static void die(int);

int main(int, char **);

int
main(int argc, char **argv)
{
	SVCXPRT *transp;
	int ch;
	struct sigaction nsa;

	while ((ch = getopt(argc, argv, "d")) != (-1)) {
		switch (ch) {
		case 'd':
			debug = 1;
			break;
		default:
		case '?':
			fprintf(stderr, "usage: %s [-d]\n", __progname);
			exit(1);
			/* NOTREACHED */
		}
	}
	pmap_unset(SM_PROG, SM_VERS);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		errx(1, "cannot create udp service.");
		/* NOTREACHED */
	}
	if (!svc_register(transp, SM_PROG, SM_VERS, sm_prog_1, IPPROTO_UDP)) {
		errx(1, "unable to register (SM_PROG, SM_VERS, udp).");
		/* NOTREACHED */
	}
	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		errx(1, "cannot create tcp service.");
		/* NOTREACHED */
	}
	if (!svc_register(transp, SM_PROG, SM_VERS, sm_prog_1, IPPROTO_TCP)) {
		errx(1, "unable to register (SM_PROG, SM_VERS, tcp).");
		/* NOTREACHED */
	}

	init_file("/var/db/statd.status");

	/*
	 * Note that it is NOT sensible to run this program from inetd - the
	 * protocol assumes that it will run immediately at boot time.
	 */
	daemon(0, 0);

	sigemptyset(&nsa.sa_mask);
	nsa.sa_flags = SA_NOCLDSTOP|SA_NOCLDWAIT;
	nsa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &nsa, NULL);

	openlog("rpc.statd", 0, LOG_DAEMON);
	if (debug)
		syslog(LOG_INFO, "Starting - debug enabled");
	else
		syslog(LOG_INFO, "Starting");

	sa.sa_handler = die;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	sa.sa_handler = SIG_IGN;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGALRM);

	/* Initialisation now complete - start operating */

	/* Notify hosts that need it */
	notify_handler(0);

	while (1)
		svc_run();		/* Should never return */
	die(0);
}

/* notify_handler ---------------------------------------------------------- */
/*
 * Purpose:	Catch SIGALRM and collect process status
 * Returns:	Nothing.
 * Notes:	No special action required, other than to collect the
 *		process status and hence allow the child to die:
 *		we only use child processes for asynchronous transmission
 *		of SM_NOTIFY to other systems, so it is normal for the
 *		children to exit when they have done their work.
 */
void 
notify_handler(int sig)
{
	time_t now;

	NO_ALARM;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGALRM, &sa, NULL);

	now = time(NULL);

	walk_db(notify_one, &now);

	if (walk_db(check_work, &now) == 0) {
		/*
		 * No more work to be done.
		 */
		CLR_ALARM;
		return;
	}
	sync_file();
	ALARM;
	alarm(5);
}

/* sync_file --------------------------------------------------------------- */
/*
 * Purpose:	Packaged call of msync() to flush changes to mmap()ed file
 * Returns:	Nothing.  Errors to syslog.
 */
void 
sync_file()
{
	DBT data;

	data.data = &status_info;
	data.size = sizeof(status_info);
	switch ((*db->put)(db, &undefkey, &data, 0)) {
	case 0:
		return;
	case -1:
		goto bad;
	default:
		abort();
	}
	if ((*db->sync)(db, 0) == -1) {
bad:
		syslog(LOG_ERR, "database corrupted %m");
		die(1);
	}
}

/* change_host -------------------------------------------------------------- */
/*
 * Purpose:	Update/Create an entry for host
 * Returns:	Nothing
 * Notes:
 *
 */
void
change_host(char *hostnamep, HostInfo *hp)
{
	DBT key, data;
	char *ptr;
	char hostname[HOST_NAME_MAX+1 + 1];
	HostInfo h;

	strlcpy(hostname, hostnamep, sizeof(hostname));
	h = *hp;

	for (ptr = hostname; *ptr; ptr++)
		if (isupper((unsigned char) *ptr))
			*ptr = tolower((unsigned char) *ptr);

	key.data = hostname;
	key.size = ptr - hostname + 1;
	data.data = &h;
	data.size = sizeof(h);

	switch ((*db->put)(db, &key, &data, 0)) {
	case -1:
		syslog(LOG_ERR, "database corrupted %m");
		die(1);
	case 0:
		return;
	default:
		abort();
	}
}


/* find_host -------------------------------------------------------------- */
/*
 * Purpose:	Find the entry in the status file for a given host
 * Returns:	Copy of entry in hd, or NULL
 * Notes:
 *
 */
HostInfo *
find_host(char *hostname, HostInfo *hp)
{
	DBT key, data;
	char *ptr;

	for (ptr = hostname; *ptr; ptr++)
		if (isupper((unsigned char) *ptr))
			*ptr = tolower((unsigned char) *ptr);

	key.data = hostname;
	key.size = ptr - hostname + 1;
	switch ((*db->get)(db, &key, &data, 0)) {
	case 0:
		if (data.size != sizeof(*hp))
			goto bad;
		return memcpy(hp, data.data, sizeof(*hp));
	case 1:
		return NULL;
	case -1:
		goto bad;
	default:
		abort();
	}

bad:
	syslog(LOG_ERR, "Database corrupted %m");
	return NULL;
}

/* walk_one ------------------------------------------------------------- */
/*
 * Purpose:	Call the given function if the element is valid
 * Returns:	Nothing - exits on error
 * Notes:	
 */
static int
walk_one(int (*fun)(DBT *, HostInfo *, void *), DBT *key, DBT *data, void *ptr)
{
	HostInfo h;
	if (key->size == undefkey.size &&
	    memcmp(key->data, undefkey.data, key->size) == 0)
		return 0;
	if (data->size != sizeof(HostInfo)) {
		syslog(LOG_ERR, "Bad data in database");
		die(1);
	}
	memcpy(&h, data->data, sizeof(h));
	return (*fun)(key, &h, ptr);
}

/* walk_db -------------------------------------------------------------- */
/*
 * Purpose:	Iterate over all elements calling the given function
 * Returns:	-1 if function failed, 0 on success
 * Notes:	
 */
static int
walk_db(int (*fun)(DBT *, HostInfo *, void *), void *ptr)
{
	DBT key, data;

	switch ((*db->seq)(db, &key, &data, R_FIRST)) {
	case -1:
		goto bad;
	case 1:
		/* We should have at least the magic entry at this point */
		abort();
	case 0:
		if (walk_one(fun, &key, &data, ptr) == -1)
			return -1;
		break;
	default:
		abort();
	}

	for (;;)
		switch ((*db->seq)(db, &key, &data, R_NEXT)) {
		case -1:
			goto bad;
		case 0:
			if (walk_one(fun, &key, &data, ptr) == -1)
				return -1;
			break;
		case 1:
			return 0;
		default:
			abort();
		}
bad:
	syslog(LOG_ERR, "Corrupted database %m");
	die(1);
}

/* reset_host ------------------------------------------------------------ */
/*
 * Purpose:	Clean up existing hosts in file.
 * Returns:	Always success 0.
 * Notes:	Clean-up of existing file - monitored hosts will have a
 *		pointer to a list of clients, which refers to memory in
 *		the previous incarnation of the program and so are
 *		meaningless now.  These pointers are zeroed and the fact
 *		that the host was previously monitored is recorded by
 *		setting the notifyReqd flag, which will in due course
 *		cause a SM_NOTIFY to be sent.
 *		 
 *		Note that if we crash twice in quick succession, some hosts
 *		may already have notifyReqd set, where we didn't manage to
 *		notify them before the second crash occurred.
 */
static int
reset_host(DBT *key, HostInfo *hi, void *ptr)
{
	if (hi->monList) {
		hi->notifyReqd = *(time_t *) ptr;
		hi->attempts = 0;
		hi->monList = NULL;
		change_host((char *)key->data, hi);
	}
	return 0;
}

/* check_work ------------------------------------------------------------ */
/*
 * Purpose:	Check if there is work to be done.
 * Returns:	0 if there is no work to be done -1 if there is.
 * Notes:	
 */
static int
check_work(DBT *key, HostInfo *hi, void *ptr)
{
	return hi->notifyReqd ? -1 : 0;
}

/* unmon_host ------------------------------------------------------------ */
/*
 * Purpose:	Unmonitor a host
 * Returns:	0
 * Notes:	
 */
static int
unmon_host(DBT *key, HostInfo *hi, void *ptr)
{
	char *name = key->data;

	if (do_unmon(name, hi, ptr))
		change_host(name, hi);
	return 0;
}

/* notify_one ------------------------------------------------------------ */
/*
 * Purpose:	Notify one host.
 * Returns:	0 if success -1 on failure
 * Notes:	
 */
static int
notify_one(DBT *key, HostInfo *hi, void *ptr)
{
	time_t now = *(time_t *) ptr;
	char *name = key->data;
	int error;

	if (hi->notifyReqd == 0 || hi->notifyReqd > now)
		return 0;

	/*
	 * If one of the initial attempts fails, we wait
	 * for a while and have another go.  This is necessary
	 * because when we have crashed, (eg. a power outage)
	 * it is quite possible that we won't be able to
	 * contact all monitored hosts immediately on restart,
	 * either because they crashed too and take longer
	 * to come up (in which case the notification isn't
	 * really required), or more importantly if some
	 * router etc. needed to reach the monitored host
	 * has not come back up yet.  In this case, we will
	 * be a bit late in re-establishing locks (after the
	 * grace period) but that is the best we can do.  We
	 * try 10 times at 5 sec intervals, 10 more times at
	 * 1 minute intervals, then 24 more times at hourly
	 * intervals, finally giving up altogether if the
	 * host hasn't come back to life after 24 hours.
	 */
	if (notify_one_host(name) || hi->attempts++ >= 44) {
		error = 0;
		hi->notifyReqd = 0;
		hi->attempts = 0;
	} else {
		error = -1;
		if (hi->attempts < 10)
			hi->notifyReqd += 5;
		else if (hi->attempts < 20)
			hi->notifyReqd += 60;
		else
			hi->notifyReqd += 60 * 60;
	}
	change_host(name, hi);
	return error;
}

/* init_file -------------------------------------------------------------- */
/*
 * Purpose:	Open file, create if necessary, initialise it.
 * Returns:	Nothing - exits on error
 * Notes:	Called before process becomes daemon, hence logs to
 *		stderr rather than syslog.
 *		Opens the file, then mmap()s it for ease of access.
 *		Also performs initial clean-up of the file, zeroing
 *		monitor list pointers, setting the notifyReqd flag in
 *		all hosts that had a monitor list, and incrementing
 *		the state number to the next even value.
 */
static void 
init_file(char *filename)
{
	DBT data;

	db = dbopen(filename, O_RDWR|O_CREAT|O_NDELAY|O_EXLOCK, 0644, DB_HASH, 
	    NULL);
	if (db == NULL)
		err(1, "Cannot open `%s'", filename);

	switch ((*db->get)(db, &undefkey, &data, 0)) {
	case 1:
		/* New database */
		memset(&status_info, 0, sizeof(status_info));
		sync_file();
		return;
	case -1:
		err(1, "error accessing database (%m)");
	case 0:
		/* Existing database */
		if (data.size != sizeof(status_info))
			errx(1, "database corrupted %lu != %lu",
			    (u_long)data.size, (u_long)sizeof(status_info));
		memcpy(&status_info, data.data, data.size);
		break;
	default:
		abort();
	}

	reset_database();
	return;
}

/* reset_database --------------------------------------------------------- */
/*
 * Purpose:	Clears the statd database
 * Returns:	Nothing
 * Notes:	If this is not called on reset, it will leak memory.
 */
void
reset_database(void)
{
	time_t now = time(NULL);
	walk_db(reset_host, &now);

	/* Select the next higher even number for the state counter */
	status_info.ourState =
	    (status_info.ourState + 2) & 0xfffffffe;
	status_info.ourState++;	/* XXX - ??? */
	sync_file();
}

/* unmon_hosts --------------------------------------------------------- */
/*
 * Purpose:	Unmonitor all the hosts
 * Returns:	Nothing
 * Notes:
 */
void
unmon_hosts(void)
{
	time_t now = time(NULL);
	walk_db(unmon_host, &now);
	sync_file();
}

static int 
notify_one_host(char *hostname)
{
	struct timeval timeout = {20, 0};	/* 20 secs timeout */
	CLIENT *cli;
	char dummy;
	stat_chge arg;
	char our_hostname[HOST_NAME_MAX+1 + 1];

	gethostname(our_hostname, sizeof(our_hostname));
	our_hostname[sizeof(our_hostname) - 1] = '\0';
	arg.mon_name = our_hostname;
	arg.state = status_info.ourState;

	if (debug)
		syslog(LOG_DEBUG, "Sending SM_NOTIFY to host %s from %s",
		    hostname, our_hostname);

	cli = clnt_create(hostname, SM_PROG, SM_VERS, "udp");
	if (!cli) {
		syslog(LOG_ERR, "Failed to contact host %s%s", hostname,
		    clnt_spcreateerror(""));
		return (FALSE);
	}
	if (clnt_call(cli, SM_NOTIFY, xdr_stat_chge, &arg, xdr_void,
	    &dummy, timeout) != RPC_SUCCESS) {
		syslog(LOG_ERR, "Failed to contact rpc.statd at host %s",
		    hostname);
		clnt_destroy(cli);
		return (FALSE);
	}
	clnt_destroy(cli);
	return (TRUE);
}

static void
die(int n)
{
	(*db->close)(db);
	exit(n);
}
