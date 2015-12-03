/*	$OpenBSD: main.c,v 1.3 2015/12/03 15:15:04 mpi Exp $ */

/*
 * Copyright (c) 2015 Martin Pieuchot
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

#include "srp_compat.h"

#include <sys/socket.h>
#include <net/route.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

#ifdef ART
#include <net/rtable.h>
#include <net/art.h>

#include <assert.h>

extern void  *rtable_get(unsigned int, sa_family_t);
#endif /* ART */

__dead void
usage(void)
{
	extern const char *__progname;
	fprintf(stderr, "Usage: %s <file>\n", __progname);
	exit(1);
}

int
main(int argc, char *argv[])
{
	char *filename;

	if (argc != 2)
		usage();

	filename = argv[1];

	rtable_init();
	if (rtable_add(0))
		errx(1, "can't add rtable\n");

	do_from_file(0, AF_INET6, filename, route_insert);

	rtable_walk(0, AF_INET6, rtentry_delete, NULL);

	rtable_walk(0, AF_INET6, rtentry_dump, NULL);

#ifdef ART
	struct art_root *ar;
	ar = rtable_get(0, AF_INET6);
	assert(ar != NULL);
	assert(ar->ar_root == NULL);
#endif /* ART */

	return (0);
}
