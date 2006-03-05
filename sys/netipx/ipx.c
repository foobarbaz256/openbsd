/*	$OpenBSD: ipx.c,v 1.14 2006/03/05 21:48:57 miod Exp $	*/

/*-
 *
 * Copyright (c) 1996 Michael Shalayeff
 * Copyright (c) 1995, Mike Mitchell
 * Copyright (c) 1984, 1985, 1986, 1987, 1993
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
 * 3. Neither the name of the University nor the names of its contributors
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
 * 
 *	@(#)ipx.c
 *
 * from FreeBSD Id: ipx.c,v 1.4 1996/03/11 15:13:46 davidg Exp
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/ioctl.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include <net/if.h>
#include <net/route.h>

#include <netipx/ipx.h>
#include <netipx/ipx_if.h>

/*
 * Generic internet control operations (ioctl's).
 */
/* ARGSUSED */
int
ipx_control(so, cmd, data, ifp)
	struct socket *so;
	u_long	cmd;
	caddr_t	data;
	struct ifnet *ifp;
{
	struct ifreq *ifr = (struct ifreq *)data;
	struct ipx_aliasreq *ifra = (struct ipx_aliasreq *)data;
	struct ipx_ifaddr *ia;
	int dstIsNew, hostIsNew;
	int error = 0;

	/*
	 * Find address for this interface, if it exists.
	 */
	if (ifp)
		TAILQ_FOREACH(ia, &ipx_ifaddr, ia_list)
			if (ia->ia_ifp == ifp)
				break;

	switch (cmd) {
	case SIOCAIFADDR:
	case SIOCDIFADDR:
		if (ifra->ifra_addr.sipx_family == AF_IPX)
			for (; ia != TAILQ_END(&ipx_ifaddr);
			    ia = TAILQ_NEXT(ia, ia_list)) {
				if (ia->ia_ifp == ifp  &&
				    ipx_neteq(ia->ia_addr.sipx_addr,
					      ifra->ifra_addr.sipx_addr))
					break;
			}
		if (cmd == SIOCDIFADDR && ia == 0)
			return (EADDRNOTAVAIL);
		/* FALLTHROUGH */

	case SIOCSIFADDR:
	case SIOCSIFDSTADDR:
		if ((so->so_state & SS_PRIV) == 0)
			return (EPERM);

		if (ia == (struct ipx_ifaddr *)NULL) {
			ia = (struct ipx_ifaddr *)
				malloc(sizeof(*ia), M_IFADDR, M_WAITOK);
			bzero((caddr_t)ia, sizeof(*ia));
			TAILQ_INSERT_TAIL(&ifp->if_addrlist,
				(struct ifaddr *)ia, ifa_list);
			TAILQ_INSERT_TAIL(&ipx_ifaddr, ia, ia_list);
			ia->ia_ifp = ifp;
			ia->ia_ifa.ifa_addr = (struct sockaddr *)&ia->ia_addr;

			ia->ia_ifa.ifa_netmask =
				(struct sockaddr *)&ipx_netmask;

			ia->ia_ifa.ifa_dstaddr =
				(struct sockaddr *)&ia->ia_dstaddr;
			if (ifp->if_flags & IFF_BROADCAST) {
				ia->ia_broadaddr.sipx_family = AF_IPX;
				ia->ia_broadaddr.sipx_len =
				    sizeof(ia->ia_addr);
				ia->ia_broadaddr.sipx_addr.ipx_host =
				    ipx_broadhost;
			}
		}
		break;

	case SIOCSIFBRDADDR:
		if ((so->so_state & SS_PRIV) == 0)
			return (EPERM);
		/* FALLTHROUGH */

	case SIOCGIFADDR:
	case SIOCGIFDSTADDR:
	case SIOCGIFBRDADDR:
		if (ia == (struct ipx_ifaddr *)NULL)
			return (EADDRNOTAVAIL);
		break;
	}

	switch (cmd) {

	case SIOCGIFADDR:
		*(struct sockaddr_ipx *)&ifr->ifr_addr = ia->ia_addr;
		return (0);

	case SIOCGIFBRDADDR:
		if ((ifp->if_flags & IFF_BROADCAST) == 0)
			return (EINVAL);
		*(struct sockaddr_ipx *)&ifr->ifr_dstaddr = ia->ia_broadaddr;
		return (0);

	case SIOCGIFDSTADDR:
		if ((ifp->if_flags & IFF_POINTOPOINT) == 0)
			return (EINVAL);
		*(struct sockaddr_ipx *)&ifr->ifr_dstaddr = ia->ia_dstaddr;
		return (0);

	case SIOCSIFDSTADDR:
		if ((ifp->if_flags & IFF_POINTOPOINT) == 0)
			return (EINVAL);
		if (ia->ia_flags & IFA_ROUTE) {
			rtinit(&(ia->ia_ifa), (int)RTM_DELETE, RTF_HOST);
			ia->ia_flags &= ~IFA_ROUTE;
		}
		if (ifp->if_ioctl) {
			error = (*ifp->if_ioctl)(ifp, SIOCSIFDSTADDR,
			    (void *)ia);
			if (error)
				return (error);
		}
		*(struct sockaddr *)&ia->ia_dstaddr = ifr->ifr_dstaddr;
		return (0);

	case SIOCSIFADDR:
		return (ipx_ifinit(ifp, ia,
				(struct sockaddr_ipx *)&ifr->ifr_addr, 1));

	case SIOCDIFADDR:
		ipx_ifscrub(ifp, ia);
		TAILQ_REMOVE(&ifp->if_addrlist, (struct ifaddr *)ia, ifa_list);
		TAILQ_REMOVE(&ipx_ifaddr, ia, ia_list);
		IFAFREE((&ia->ia_ifa));
		return (0);
	
	case SIOCAIFADDR:
		dstIsNew = 0; hostIsNew = 1;
		if (ia->ia_addr.sipx_family == AF_IPX) {
			if (ifra->ifra_addr.sipx_len == 0) {
				ifra->ifra_addr = ia->ia_addr;
				hostIsNew = 0;
			} else if (ipx_neteq(ifra->ifra_addr.sipx_addr,
					 ia->ia_addr.sipx_addr))
				hostIsNew = 0;
		}
		if ((ifp->if_flags & IFF_POINTOPOINT) &&
		    (ifra->ifra_dstaddr.sipx_family == AF_IPX)) {
			if (hostIsNew == 0)
				ipx_ifscrub(ifp, ia);
			ia->ia_dstaddr = ifra->ifra_dstaddr;
			dstIsNew  = 1;
		}
		if (ifra->ifra_addr.sipx_family == AF_IPX &&
					    (hostIsNew || dstIsNew))
			error = ipx_ifinit(ifp, ia, &ifra->ifra_addr, 0);
		return (error);

	default:
		if (ifp->if_ioctl == 0)
			return (EOPNOTSUPP);
		return ((*ifp->if_ioctl)(ifp, cmd, data));
	}
}

/*
 * Delete any previous route for an old address.
 */
void
ipx_ifscrub(ifp, ia)
	struct ifnet *ifp;
	struct ipx_ifaddr *ia; 
{
	if (ia->ia_flags & IFA_ROUTE) {
		if (ifp->if_flags & IFF_POINTOPOINT) {
			rtinit(&(ia->ia_ifa), (int)RTM_DELETE, RTF_HOST);
		} else
			rtinit(&(ia->ia_ifa), (int)RTM_DELETE, 0);
		ia->ia_flags &= ~IFA_ROUTE;
	}
}

/*
 * Initialize an interface's IPX address
 * and routing table entry.
 */
int
ipx_ifinit(ifp, ia, sipx, scrub)
	struct ifnet *ifp;
	struct ipx_ifaddr *ia;
	struct sockaddr_ipx *sipx;
	int scrub;
{
	struct sockaddr_ipx oldaddr;
	int s = splnet(), error;

	/*
	 * Set up new addresses.
	 */
	oldaddr = ia->ia_addr;
	ia->ia_addr = *sipx;

	/*
	 * The convention we shall adopt for naming is that
	 * a supplied address of zero means that "we don't care".
	 * Use the MAC address of the interface. If it is an
	 * interface without a MAC address, like a serial line, the
	 * address must be supplied.
	 *
	 * Give the interface a chance to initialize
	 * if this is its first address,
	 * and to validate the address if necessary.
	 */
	if (ifp->if_ioctl != NULL &&
	    (error = (*ifp->if_ioctl)(ifp, SIOCSIFADDR, (void *)ia))) {
		ia->ia_addr = oldaddr;
		splx(s);
		return (error);
	}
	ia->ia_ifa.ifa_metric = ifp->if_metric;
	/*
	 * Add route for the network.
	 */
	if (scrub) {
		ia->ia_ifa.ifa_addr = (struct sockaddr *)&oldaddr;
		ipx_ifscrub(ifp, ia);
		ia->ia_ifa.ifa_addr = (struct sockaddr *)&ia->ia_addr;
	}
	if (ifp->if_flags & IFF_POINTOPOINT)
		rtinit(&(ia->ia_ifa), (int)RTM_ADD, RTF_HOST|RTF_UP);
	else {
		ia->ia_broadaddr.sipx_addr.ipx_net =
		    ia->ia_addr.sipx_addr.ipx_net;
		rtinit(&(ia->ia_ifa), (int)RTM_ADD, RTF_UP);
	}
	ia->ia_flags |= IFA_ROUTE;
	splx(s);
	return (0);
}

/*
 * Return address info for specified IPX network.
 */
struct ipx_ifaddr *
ipx_iaonnetof(dst)
	struct ipx_addr *dst;
{
	struct ipx_ifaddr *ia;
	struct ipx_addr *compare;
	struct ifnet *ifp;
	struct ipx_ifaddr *ia_maybe = NULL;
	union ipx_net net = dst->ipx_net;

	TAILQ_FOREACH(ia, &ipx_ifaddr, ia_list) {
		if ((ifp = ia->ia_ifp)) {
			if (ifp->if_flags & IFF_POINTOPOINT) {
				compare = &satoipx_addr(ia->ia_dstaddr);
				if (ipx_hosteq(*dst, *compare))
					return (ia);
				if (ipx_neteqnn(net,
				    ia->ia_addr.sipx_addr.ipx_net))
					ia_maybe = ia;
			} else {
				if (ipx_neteqnn(net,
				    ia->ia_addr.sipx_addr.ipx_net))
					return (ia);
			}
		}
	}
	return (ia_maybe);
}

void
ipx_printhost(addr)
register struct ipx_addr *addr;
{
	u_short port;
	struct ipx_addr work = *addr;
	char *p;
	u_char *q;
	char *net = "", *host = "";
	char cport[10], chost[15], cnet[15];

	port = ntohs(work.ipx_port);

	if (ipx_nullnet(work) && ipx_nullhost(work)) {

		if (port)
			printf("*.%x", port);
		else
			printf("*.*");

		return;
	}

	if (ipx_wildnet(work))
		net = "any";
	else if (ipx_nullnet(work))
		net = "*";
	else {
		q = work.ipx_net.c_net;
		snprintf(cnet, sizeof(cnet), "%x%x%x%x",
			q[0], q[1], q[2], q[3]);
		for (p = cnet; *p == '0' && p < cnet + 8; p++)
			continue;
		net = p;
	}

	if (ipx_wildhost(work))
		host = "any";
	else if (ipx_nullhost(work))
		host = "*";
	else {
		q = work.ipx_host.c_host;
		snprintf(chost, sizeof(chost), "%x%x%x%x%x%x",
			q[0], q[1], q[2], q[3], q[4], q[5]);
		for (p = chost; *p == '0' && p < chost + 12; p++)
			continue;
		host = p;
	}

	if (port) {
		if (strcmp(host, "*") == 0) {
			host = "";
			snprintf(cport, sizeof(cport), "%x", port);
		} else
			snprintf(cport, sizeof(cport), ".%x", port);
	} else
		*cport = 0;

	printf("%s.%s%s", net, host, cport);
}

