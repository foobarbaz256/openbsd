/*	$OpenBSD: krpc_subr.c,v 1.8 1998/02/23 09:46:53 deraadt Exp $	*/
/*	$NetBSD: krpc_subr.c,v 1.12.4.1 1996/06/07 00:52:26 cgd Exp $	*/

/*
 * Copyright (c) 1995 Gordon Ross, Adam Glass
 * Copyright (c) 1992 Regents of the University of California.
 * All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
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
 *	California, Lawrence Berkeley Laboratory and its contributors.
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
 *
 * partially based on:
 *      libnetboot/rpc.c
 *               @(#) Header: rpc.c,v 1.12 93/09/28 08:31:56 leres Exp  (LBL)
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/mbuf.h>
#include <sys/reboot.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include <net/if.h>
#include <netinet/in.h>

#include <nfs/rpcv2.h>
#include <nfs/krpc.h>
#include <nfs/xdr_subs.h>
#include <dev/rndvar.h>

/*
 * Kernel support for Sun RPC
 *
 * Used currently for bootstrapping in nfs diskless configurations.
 */

/*
 * Generic RPC headers
 */

struct auth_info {
	u_int32_t 	authtype;	/* auth type */
	u_int32_t	authlen;	/* auth length */
};

struct auth_unix {
	int32_t   ua_time;
	int32_t   ua_hostname;	/* null */
	int32_t   ua_uid;
	int32_t   ua_gid;
	int32_t   ua_gidlist;	/* null */
};

struct rpc_call {
	u_int32_t	rp_xid;		/* request transaction id */
	int32_t 	rp_direction;	/* call direction (0) */
	u_int32_t	rp_rpcvers;	/* rpc version (2) */
	u_int32_t	rp_prog;	/* program */
	u_int32_t	rp_vers;	/* version */
	u_int32_t	rp_proc;	/* procedure */
	struct	auth_info rpc_auth;
	struct	auth_unix rpc_unix;
	struct	auth_info rpc_verf;
};

struct rpc_reply {
	u_int32_t rp_xid;		/* request transaction id */
	int32_t   rp_direction;		/* call direction (1) */
	int32_t   rp_astatus;		/* accept status (0: accepted) */
	union {
		u_int32_t rpu_errno;
		struct {
			struct auth_info rok_auth;
			u_int32_t	rok_status;
		} rpu_rok;
	} rp_u;
};
#define rp_errno  rp_u.rpu_errno
#define rp_auth   rp_u.rpu_rok.rok_auth
#define rp_status rp_u.rpu_rok.rok_status

#define MIN_REPLY_HDR 16	/* xid, dir, astat, errno */

/*
 * What is the longest we will wait before re-sending a request?
 * Note this is also the frequency of "RPC timeout" messages.
 * The re-send loop count sup linearly to this maximum, so the
 * first complaint will happen after (1+2+3+4+5)=15 seconds.
 */
#define	MAX_RESEND_DELAY 5	/* seconds */

/*
 * Call portmap to lookup a port number for a particular rpc program
 * Returns non-zero error on failure.
 */
int
krpc_portmap(sin,  prog, vers, portp)
	struct sockaddr_in *sin;		/* server address */
	u_int prog, vers;	/* host order */
	u_int16_t *portp;	/* network order */
{
	struct sdata {
		u_int32_t prog;		/* call program */
		u_int32_t vers;		/* call version */
		u_int32_t proto;	/* call protocol */
		u_int32_t port;		/* call port (unused) */
	} *sdata;
	struct rdata {
		u_int16_t pad;
		u_int16_t port;
	} *rdata;
	struct mbuf *m;
	int error;

	/* The portmapper port is fixed. */
	if (prog == PMAPPROG) {
		*portp = htons(PMAPPORT);
		return 0;
	}

	m = m_get(M_WAIT, MT_DATA);
	if (m == NULL)
		return ENOBUFS;
	sdata = mtod(m, struct sdata *);
	m->m_len = sizeof(*sdata);

	/* Do the RPC to get it. */
	sdata->prog = txdr_unsigned(prog);
	sdata->vers = txdr_unsigned(vers);
	sdata->proto = txdr_unsigned(IPPROTO_UDP);
	sdata->port = 0;

	sin->sin_port = htons(PMAPPORT);
	error = krpc_call(sin, PMAPPROG, PMAPVERS,
					  PMAPPROC_GETPORT, &m, NULL);
	if (error) 
		return error;

	if (m->m_len < sizeof(*rdata)) {
		m = m_pullup(m, sizeof(*rdata));
		if (m == NULL)
			return ENOBUFS;
	}
	rdata = mtod(m, struct rdata *);
	*portp = rdata->port;

	m_freem(m);
	return 0;
}

/*
 * Do a remote procedure call (RPC) and wait for its reply.
 * If from_p is non-null, then we are doing broadcast, and
 * the address from whence the response came is saved there.
 */
int
krpc_call(sa, prog, vers, func, data, from_p)
	struct sockaddr_in *sa;
	u_int prog, vers, func;
	struct mbuf **data;	/* input/output */
	struct mbuf **from_p;	/* output */
{
	struct socket *so;
	struct sockaddr_in *sin;
	struct mbuf *m, *nam, *mhead, *from;
	struct rpc_call *call;
	struct rpc_reply *reply;
	struct uio auio;
	int error, rcvflg, timo, secs, len;
	static u_int32_t xid = 0;
	u_int32_t newxid;
	u_int16_t tport;

	/*
	 * Validate address family.
	 * Sorry, this is INET specific...
	 */
	if (sa->sin_family != AF_INET)
		return (EAFNOSUPPORT);

	/* Free at end if not null. */
	nam = mhead = NULL;
	from = NULL;

	/*
	 * Create socket and set its recieve timeout.
	 */
	if ((error = socreate(AF_INET, &so, SOCK_DGRAM, 0)))
		goto out;

	m = m_get(M_WAIT, MT_SOOPTS);
	if (m == NULL) {
		error = ENOBUFS;
		goto out;
	} else {
		struct timeval *tv;
		tv = mtod(m, struct timeval *);
		m->m_len = sizeof(*tv);
		tv->tv_sec = 1;
		tv->tv_usec = 0;
		if ((error = sosetopt(so, SOL_SOCKET, SO_RCVTIMEO, m)))
			goto out;
	}

	/*
	 * Enable broadcast if necessary.
	 */
	if (from_p) {
		int32_t *on;
		m = m_get(M_WAIT, MT_SOOPTS);
		if (m == NULL) {
			error = ENOBUFS;
			goto out;
		}
		on = mtod(m, int32_t *);
		m->m_len = sizeof(*on);
		*on = 1;
		if ((error = sosetopt(so, SOL_SOCKET, SO_BROADCAST, m)))
			goto out;
	}

	/*
	 * Bind the local endpoint to a reserved port,
	 * because some NFS servers refuse requests from
	 * non-reserved (non-privileged) ports.
	 */
	m = m_getclr(M_WAIT, MT_SONAME);
	sin = mtod(m, struct sockaddr_in *);
	sin->sin_len = m->m_len = sizeof(*sin);
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = INADDR_ANY;
	/* XXX should do random allocation */
	tport = IPPORT_RESERVED;
	do {
		tport--;
		sin->sin_port = htons(tport);
		error = sobind(so, m);
	} while (error == EADDRINUSE &&
			 tport > IPPORT_RESERVED / 2);
	m_freem(m);
	if (error) {
		printf("bind failed\n");
		goto out;
	}

	/*
	 * Setup socket address for the server.
	 */
	nam = m_get(M_WAIT, MT_SONAME);
	if (nam == NULL) {
		error = ENOBUFS;
		goto out;
	}
	sin = mtod(nam, struct sockaddr_in *);
	bcopy((caddr_t)sa, (caddr_t)sin,
		  (nam->m_len = sa->sin_len));

	/*
	 * Prepend RPC message header.
	 */
	mhead = m_gethdr(M_WAIT, MT_DATA);
	mhead->m_next = *data;
	call = mtod(mhead, struct rpc_call *);
	mhead->m_len = sizeof(*call);
	bzero((caddr_t)call, sizeof(*call));
	/* rpc_call part */
	while ((newxid = arc4random()) == xid);
	xid = newxid;
	call->rp_xid = txdr_unsigned(xid);
	/* call->rp_direction = 0; */
	call->rp_rpcvers = txdr_unsigned(2);
	call->rp_prog = txdr_unsigned(prog);
	call->rp_vers = txdr_unsigned(vers);
	call->rp_proc = txdr_unsigned(func);
	/* rpc_auth part (auth_unix as root) */
	call->rpc_auth.authtype = txdr_unsigned(RPCAUTH_UNIX);
	call->rpc_auth.authlen  = txdr_unsigned(sizeof(struct auth_unix));
	/* rpc_verf part (auth_null) */
	call->rpc_verf.authtype = 0;
	call->rpc_verf.authlen  = 0;

	/*
	 * Setup packet header
	 */
	len = 0;
	m = mhead;
	while (m) {
		len += m->m_len;
		m = m->m_next;
	}
	mhead->m_pkthdr.len = len;
	mhead->m_pkthdr.rcvif = NULL;

	/*
	 * Send it, repeatedly, until a reply is received,
	 * but delay each re-send by an increasing amount.
	 * If the delay hits the maximum, start complaining.
	 */
	timo = 0;
	for (;;) {
		/* Send RPC request (or re-send). */
		m = m_copym(mhead, 0, M_COPYALL, M_WAIT);
		if (m == NULL) {
			error = ENOBUFS;
			goto out;
		}
		error = sosend(so, nam, NULL, m, NULL, 0);
		if (error) {
			printf("krpc_call: sosend: %d\n", error);
			goto out;
		}
		m = NULL;

		/* Determine new timeout. */
		if (timo < MAX_RESEND_DELAY)
			timo++;
		else
			printf("RPC timeout for server 0x%x\n",
			       ntohl(sin->sin_addr.s_addr));

		/*
		 * Wait for up to timo seconds for a reply.
		 * The socket receive timeout was set to 1 second.
		 */
		secs = timo;
		while (secs > 0) {
			if (from) {
				m_freem(from);
				from = NULL;
			}
			if (m) {
				m_freem(m);
				m = NULL;
			}
			auio.uio_resid = len = 1<<16;
			rcvflg = 0;
			error = soreceive(so, &from, &auio, &m, NULL, &rcvflg);
			if (error == EWOULDBLOCK) {
				secs--;
				continue;
			}
			if (error)
				goto out;
			len -= auio.uio_resid;

			/* Does the reply contain at least a header? */
			if (len < MIN_REPLY_HDR)
				continue;
			if (m->m_len < MIN_REPLY_HDR)
				continue;
			reply = mtod(m, struct rpc_reply *);

			/* Is it the right reply? */
			if (reply->rp_direction != txdr_unsigned(RPC_REPLY))
				continue;

			if (reply->rp_xid != txdr_unsigned(xid))
				continue;

			/* Was RPC accepted? (authorization OK) */
			if (reply->rp_astatus != 0) {
				error = fxdr_unsigned(u_int32_t, reply->rp_errno);
				printf("rpc denied, error=%d\n", error);
				continue;
			}

			/* Did the call succeed? */
			if (reply->rp_status != 0) {
				error = fxdr_unsigned(u_int32_t, reply->rp_status);
				printf("rpc denied, status=%d\n", error);
				continue;
			}

			goto gotreply;	/* break two levels */

		} /* while secs */
	} /* forever send/receive */

	error = ETIMEDOUT;
	goto out;

 gotreply:

	/*
	 * Get RPC reply header into first mbuf,
	 * get its length, then strip it off.
	 */
	len = sizeof(*reply);
	if (m->m_len < len) {
		m = m_pullup(m, len);
		if (m == NULL) {
			error = ENOBUFS;
			goto out;
		}
	}
	reply = mtod(m, struct rpc_reply *);
	if (reply->rp_auth.authtype != 0) {
		len += fxdr_unsigned(u_int32_t, reply->rp_auth.authlen);
		len = (len + 3) & ~3; /* XXX? */
	}
	m_adj(m, len);

	/* result */
	*data = m;
	if (from_p) {
		*from_p = from;
		from = NULL;
	}

 out:
	if (nam) m_freem(nam);
	if (mhead) m_freem(mhead);
	if (from) m_freem(from);
	soclose(so);
	return error;
}

/*
 * eXternal Data Representation routines.
 * (but with non-standard args...)
 */

/*
 * String representation for RPC.
 */
struct xdr_string {
	u_int32_t len;		/* length without null or padding */
	char data[4];	/* data (longer, of course) */
    /* data is padded to a long-word boundary */
};

struct mbuf *
xdr_string_encode(str, len)
	char *str;
	int len;
{
	struct mbuf *m;
	struct xdr_string *xs;
	int dlen;	/* padded string length */
	int mlen;	/* message length */

	dlen = (len + 3) & ~3;
	mlen = dlen + 4;

	if (mlen > MCLBYTES)		/* If too big, we just can't do it. */
		return (NULL);

	m = m_get(M_WAIT, MT_DATA);
	if (mlen > MLEN) {
		MCLGET(m, M_WAIT);
		if ((m->m_flags & M_EXT) == 0) {
			(void) m_free(m);	/* There can be only one. */
			return (NULL);
		}
	}
	xs = mtod(m, struct xdr_string *);
	m->m_len = mlen;
	xs->len = txdr_unsigned(len);
	bcopy(str, xs->data, len);
	return (m);
}

struct mbuf *
xdr_string_decode(m, str, len_p)
	struct mbuf *m;
	char *str;
	int *len_p;		/* bufsize - 1 */
{
	struct xdr_string *xs;
	int mlen;	/* message length */
	int slen;	/* string length */

	if (m->m_len < 4) {
		m = m_pullup(m, 4);
		if (m == NULL)
			return (NULL);
	}
	xs = mtod(m, struct xdr_string *);
	slen = fxdr_unsigned(u_int32_t, xs->len);
	mlen = 4 + ((slen + 3) & ~3);

	if (slen > *len_p)
		slen = *len_p;
	if (slen > m->m_pkthdr.len) {
		m_freem(m);
		return (NULL);
	}
	m_copydata(m, 4, slen, str);
	m_adj(m, mlen);

	str[slen] = '\0';
	*len_p = slen;

	return (m);
}


/*
 * Inet address in RPC messages
 * (Note, really four ints, NOT chars.  Blech.)
 */
struct xdr_inaddr {
	u_int32_t atype;
	u_int32_t addr[4];
};

struct mbuf *
xdr_inaddr_encode(ia)
	struct in_addr *ia;		/* already in network order */
{
	struct mbuf *m;
	struct xdr_inaddr *xi;
	u_int8_t *cp;
	u_int32_t *ip;

	m = m_get(M_WAIT, MT_DATA);
	xi = mtod(m, struct xdr_inaddr *);
	m->m_len = sizeof(*xi);
	xi->atype = txdr_unsigned(1);
	ip = xi->addr;
	cp = (u_int8_t *)&ia->s_addr;
	*ip++ = txdr_unsigned(*cp++);
	*ip++ = txdr_unsigned(*cp++);
	*ip++ = txdr_unsigned(*cp++);
	*ip++ = txdr_unsigned(*cp++);

	return (m);
}

struct mbuf *
xdr_inaddr_decode(m, ia)
	struct mbuf *m;
	struct in_addr *ia;		/* already in network order */
{
	struct xdr_inaddr *xi;
	u_int8_t *cp;
	u_int32_t *ip;

	if (m->m_len < sizeof(*xi)) {
		m = m_pullup(m, sizeof(*xi));
		if (m == NULL)
			return (NULL);
	}
	xi = mtod(m, struct xdr_inaddr *);
	if (xi->atype != txdr_unsigned(1)) {
		ia->s_addr = INADDR_ANY;
		goto out;
	}
	ip = xi->addr;
	cp = (u_int8_t *)&ia->s_addr;
	*cp++ = fxdr_unsigned(u_int8_t, *ip++);
	*cp++ = fxdr_unsigned(u_int8_t, *ip++);
	*cp++ = fxdr_unsigned(u_int8_t, *ip++);
	*cp++ = fxdr_unsigned(u_int8_t, *ip++);

out:
	m_adj(m, sizeof(*xi));
	return (m);
}
