/*	$OpenBSD: util.c,v 1.3 2015/10/21 03:52:12 renato Exp $ */

/*
 * Copyright (c) 2015 Renato Westphal <renato@openbsd.org>
 * Copyright (c) 2012 Alexander Bluhm <bluhm@openbsd.org>
 * Copyright (c) 2004 Esben Norby <norby@openbsd.org>
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <netinet/in.h>
#include <string.h>

#include "eigrpd.h"
#include "log.h"

uint8_t
mask2prefixlen(in_addr_t ina)
{
	if (ina == 0)
		return (0);
	else
		return (33 - ffs(ntohl(ina)));
}

uint8_t
mask2prefixlen6(struct sockaddr_in6 *sa_in6)
{
	uint8_t	l = 0, *ap, *ep;

	/*
	 * sin6_len is the size of the sockaddr so substract the offset of
	 * the possibly truncated sin6_addr struct.
	 */
	ap = (uint8_t *)&sa_in6->sin6_addr;
	ep = (uint8_t *)sa_in6 + sa_in6->sin6_len;
	for (; ap < ep; ap++) {
		/* this "beauty" is adopted from sbin/route/show.c ... */
		switch (*ap) {
		case 0xff:
			l += 8;
			break;
		case 0xfe:
			l += 7;
			return (l);
		case 0xfc:
			l += 6;
			return (l);
		case 0xf8:
			l += 5;
			return (l);
		case 0xf0:
			l += 4;
			return (l);
		case 0xe0:
			l += 3;
			return (l);
		case 0xc0:
			l += 2;
			return (l);
		case 0x80:
			l += 1;
			return (l);
		case 0x00:
			return (l);
		default:
			fatalx("non contiguous inet6 netmask");
		}
	}

	return (l);
}

in_addr_t
prefixlen2mask(uint8_t prefixlen)
{
	if (prefixlen == 0)
		return (0);

	return (htonl(0xffffffff << (32 - prefixlen)));
}

struct in6_addr *
prefixlen2mask6(uint8_t prefixlen)
{
	static struct in6_addr	mask;
	int			i;

	memset(&mask, 0, sizeof(mask));
	for (i = 0; i < prefixlen / 8; i++)
		mask.s6_addr[i] = 0xff;
	i = prefixlen % 8;
	if (i)
		mask.s6_addr[prefixlen / 8] = 0xff00 >> i;

	return (&mask);
}

void
eigrp_applymask(int af, union eigrpd_addr *dest, const union eigrpd_addr *src,
    int prefixlen)
{
	struct in6_addr	mask;
	int		i;

	switch (af) {
	case AF_INET:
		dest->v4.s_addr = src->v4.s_addr & prefixlen2mask(prefixlen);
		break;
	case AF_INET6:
		memset(&mask, 0, sizeof(mask));
		for (i = 0; i < prefixlen / 8; i++)
			mask.s6_addr[i] = 0xff;
		i = prefixlen % 8;
		if (i)
			mask.s6_addr[prefixlen / 8] = 0xff00 >> i;

		for (i = 0; i < 16; i++)
			dest->v6.s6_addr[i] = src->v6.s6_addr[i] &
			    mask.s6_addr[i];
		break;
	default:
		fatalx("eigrp_applymask: unknown af");
	}
}

int
eigrp_addrcmp(int af, union eigrpd_addr *a, union eigrpd_addr *b)
{
	switch (af) {
	case AF_INET:
		if (a->v4.s_addr != b->v4.s_addr)
			return (1);
		break;
	case AF_INET6:
		if (!IN6_ARE_ADDR_EQUAL(&a->v6, &b->v6))
			return (1);
		break;
	default:
		fatalx("eigrp_addrcmp: unknown af");
	}

	return (0);
}

int
eigrp_addrisset(int af, union eigrpd_addr *addr)
{
	switch (af) {
	case AF_UNSPEC:
		return (0);
	case AF_INET:
		if (addr->v4.s_addr != INADDR_ANY)
			return (1);
		break;
	case AF_INET6:
		if (!IN6_IS_ADDR_UNSPECIFIED(&addr->v6))
			return (1);
		break;
	default:
		fatalx("eigrp_addrisset: unknown af");
	}

	return (0);
}

int
eigrp_prefixcmp(int af, const union eigrpd_addr *a, const union eigrpd_addr *b,
    uint8_t prefixlen)
{
	in_addr_t	mask, aa, ba;
	int		i;
	uint8_t		m;

	switch (af) {
	case AF_INET:
		if (prefixlen == 0)
			return (0);
		if (prefixlen > 32)
			fatalx("eigrp_prefixcmp: bad IPv4 prefixlen");
		mask = htonl(prefixlen2mask(prefixlen));
		aa = htonl(a->v4.s_addr) & mask;
		ba = htonl(b->v4.s_addr) & mask;
		return (aa - ba);
	case AF_INET6:
		if (prefixlen == 0)
			return (0);
		if (prefixlen > 128)
			fatalx("eigrp_prefixcmp: bad IPv6 prefixlen");
		for (i = 0; i < prefixlen / 8; i++)
			if (a->v6.s6_addr[i] != b->v6.s6_addr[i])
				return (a->v6.s6_addr[i] - b->v6.s6_addr[i]);
		i = prefixlen % 8;
		if (i) {
			m = 0xff00 >> i;
			if ((a->v6.s6_addr[prefixlen / 8] & m) !=
			    (b->v6.s6_addr[prefixlen / 8] & m))
				return ((a->v6.s6_addr[prefixlen / 8] & m) -
				    (b->v6.s6_addr[prefixlen / 8] & m));
		}
		return (0);
	default:
		fatalx("eigrp_prefixcmp: unknown af");
	}
	return (-1);
}

#define IN6_IS_SCOPE_EMBED(a)   \
	((IN6_IS_ADDR_LINKLOCAL(a)) ||  \
	 (IN6_IS_ADDR_MC_LINKLOCAL(a)) || \
	 (IN6_IS_ADDR_MC_INTFACELOCAL(a)))

void
embedscope(struct sockaddr_in6 *sin6)
{
	uint16_t	 tmp16;

	if (IN6_IS_SCOPE_EMBED(&sin6->sin6_addr)) {
		memcpy(&tmp16, &sin6->sin6_addr.s6_addr[2], sizeof(tmp16));
		if (tmp16 != 0) {
			log_warnx("%s: address %s already has embeded scope %u",
			    __func__, log_sockaddr(sin6), ntohs(tmp16));
		}
		tmp16 = htons(sin6->sin6_scope_id);
		memcpy(&sin6->sin6_addr.s6_addr[2], &tmp16, sizeof(tmp16));
		sin6->sin6_scope_id = 0;
	}
}

void
recoverscope(struct sockaddr_in6 *sin6)
{
	uint16_t	 tmp16;

	if (sin6->sin6_scope_id != 0) {
		log_warnx("%s: address %s already has scope id %u",
		    __func__, log_sockaddr(sin6), sin6->sin6_scope_id);
	}

	if (IN6_IS_SCOPE_EMBED(&sin6->sin6_addr)) {
		memcpy(&tmp16, &sin6->sin6_addr.s6_addr[2], sizeof(tmp16));
		sin6->sin6_scope_id = ntohs(tmp16);
		sin6->sin6_addr.s6_addr[2] = 0;
		sin6->sin6_addr.s6_addr[3] = 0;
	}
}

void
addscope(struct sockaddr_in6 *sin6, uint32_t id)
{
	if (sin6->sin6_scope_id != 0) {
		log_warnx("%s: address %s already has scope id %u", __func__,
		    log_sockaddr(sin6), sin6->sin6_scope_id);
	}

	if (IN6_IS_SCOPE_EMBED(&sin6->sin6_addr)) {
		sin6->sin6_scope_id = id;
	}
}

void
clearscope(struct in6_addr *in6)
{
	if (IN6_IS_SCOPE_EMBED(in6)) {
		in6->s6_addr[2] = 0;
		in6->s6_addr[3] = 0;
	}
}

#undef IN6_IS_SCOPE_EMBED
