/*	$OpenBSD: carp.c,v 1.3 2006/06/01 22:43:12 mcbride Exp $	*/

/*
 * Copyright (c) 2005 H�kan Olsson.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This code was written under funding by Multicom Security AB.
 */


#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sasyncd.h"

static enum RUNSTATE
carp_map_state(u_char link_state)
{
	enum RUNSTATE state = FAIL;

	switch(link_state) {
	case LINK_STATE_UP:
		state = MASTER;
		break;
	case LINK_STATE_DOWN:
		state = SLAVE;
		break;
	case LINK_STATE_UNKNOWN:
		state = INIT;
		break;
	}

	return state;
}

/* Returns 1 for the CARP MASTER, 0 for BACKUP/INIT, -1 on error.  */
static enum RUNSTATE
carp_get_state(char *ifname)
{
	struct ifreq	ifr;
	struct if_data	ifrdat;
	int		s, saved_errno;

	if (!ifname || !*ifname) {
		errno = ENOENT;
		return FAIL;
	}

	memset(&ifr, 0, sizeof ifr);
	strlcpy(ifr.ifr_name, ifname, sizeof ifr.ifr_name);

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		return FAIL;

	ifr.ifr_data = (caddr_t)&ifrdat;
	if (ioctl(s, SIOCGIFDATA, (caddr_t)&ifr) == -1) {
		saved_errno = errno;
		close(s);
		errno = saved_errno;
		return FAIL;
	}
	close(s);
	return carp_map_state(ifrdat.ifi_link_state);
}

const char*
carp_state_name(enum RUNSTATE state)
{
	static const char	*carpstate[] = CARPSTATES;

	if (state < 0 || state > FAIL)
		state = FAIL;
	return carpstate[state];
}

void
carp_update_state(enum RUNSTATE current_state)
{

	if (current_state < 0 || current_state > FAIL) {
		log_err("carp_update_state: invalid carp state, abort");
		cfgstate.runstate = FAIL;
		return;
	}

	if (current_state != cfgstate.runstate) {
		log_msg(1, "carp_update_state: switching state to %s",
		    carp_state_name(current_state));
		cfgstate.runstate = current_state;
		if (current_state == MASTER)
			pfkey_set_promisc();
		net_ctl_update_state();
	}
}

void
carp_check_state()
{
	carp_update_state(carp_get_state(cfgstate.carp_ifname));
}

void
carp_set_rfd(fd_set *fds)
{
	if (cfgstate.route_socket != -1)
		FD_SET(cfgstate.route_socket, fds);
}

static void
carp_read(void)
{
	char msg[2048];
	struct if_msghdr *ifm = (struct if_msghdr *)&msg;
	int len;

	len = read(cfgstate.route_socket, msg, sizeof(msg));

	if (len >= sizeof(struct if_msghdr) &&
	    ifm->ifm_version == RTM_VERSION &&
	    ifm->ifm_type == RTM_IFINFO)
		carp_update_state(carp_map_state(ifm->ifm_data.ifi_link_state));
}

void
carp_read_message(fd_set *fds)
{
	if (cfgstate.route_socket != -1)
		if (FD_ISSET(cfgstate.route_socket, fds))
			(void)carp_read();
}

/* Initialize the CARP state. */
int
carp_init(void)
{
	cfgstate.route_socket = -1;

	if (cfgstate.lockedstate != INIT) {
		cfgstate.runstate = cfgstate.lockedstate;
		log_msg(1, "carp_init: locking runstate to %s",
		    carp_state_name(cfgstate.runstate));
		return 0;
	}

	if (!cfgstate.carp_ifname || !*cfgstate.carp_ifname) {
		fprintf(stderr, "No carp interface\n");
		return -1;
	}

	cfgstate.carp_ifindex = if_nametoindex(cfgstate.carp_ifname);
	if (!cfgstate.carp_ifindex) {
		fprintf(stderr, "No carp interface index\n");
		return -1;
	}

	cfgstate.route_socket = socket(PF_ROUTE, SOCK_RAW, 0);
	if (cfgstate.route_socket < 0) {
		fprintf(stderr, "No routing socket\n");
		return -1;
	}

	cfgstate.runstate = carp_get_state(cfgstate.carp_ifname);
	if (cfgstate.runstate == FAIL) {
		fprintf(stderr, "Failed to check interface \"%s\".\n",
		    cfgstate.carp_ifname);
		fprintf(stderr, "Correct or manually select runstate.\n");
		return -1;
	}
	log_msg(1, "carp_init: initializing runstate to %s",
	    carp_state_name(cfgstate.runstate));	

	return 0;
}
