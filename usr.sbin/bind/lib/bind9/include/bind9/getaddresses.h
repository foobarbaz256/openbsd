/*
 * Copyright (C) 2004  Internet Systems Consortium, Inc. ("ISC")
 * Copyright (C) 2001  Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* $ISC: getaddresses.h,v 1.2.200.3 2004/03/08 09:04:28 marka Exp $ */

#ifndef BIND9_GETADDRESSES_H
#define BIND9_GETADDRESSES_H 1

#include <isc/lang.h>
#include <isc/types.h>

#include <isc/net.h>

ISC_LANG_BEGINDECLS

isc_result_t
bind9_getaddresses(const char *hostname, in_port_t port,
		   isc_sockaddr_t *addrs, int addrsize, int *addrcount);
/*
 * Use the system resolver to get the addresses associated with a hostname.
 * If successful, the number of addresses found is returned in 'addrcount'.
 * If a hostname lookup is performed and addresses of an unknown family is
 * seen, it is ignored.  If more than 'addrsize' addresses are seen, the
 * first 'addrsize' are returned and the remainder silently truncated.
 *
 * This routine may block.  If called by a program using the isc_app
 * framework, it should be surounded by isc_app_block()/isc_app_unblock().
 *
 *  Requires:
 *	'hostname' is not NULL.
 *	'addrs' is not NULL.
 *	'addrsize' > 0
 *	'addrcount' is not NULL.
 *
 * 
 * Returns:
 *	ISC_R_SUCCESS
 *	ISC_R_NOTFOUND
 *	ISC_R_NOFAMILYSUPPORT - 'hostname' is an IPv6 address, and IPv6 is
 *		not supported.
 */

ISC_LANG_ENDDECLS

#endif /* BIND9_GETADDRESSES_H */
