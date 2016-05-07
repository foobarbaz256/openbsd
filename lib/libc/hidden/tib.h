/*	$OpenBSD: tib.h,v 1.1 2016/05/07 19:05:22 guenther Exp $	*/
/*
 * Copyright (c) 2015 Philip Guenther <guenther@openbsd.org>
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

#ifndef	_LIBC_TIB_H_
#define	_LIBC_TIB_H_

#include_next <tib.h>

__BEGIN_HIDDEN_DECLS

#ifndef PIC
void	_static_tls_init(char *_base);

/* size of static TLS allocation in staticly linked programs */
extern size_t	_static_tls_size;
#endif

#if ! TCB_HAVE_MD_GET
/*
 * For archs without a fast TCB_GET(): the pointer to the TCB in
 * single-threaded programs, whether linked staticly or dynamically.
 */
extern void	*_libc_single_tcb;
#endif

__END_HIDDEN_DECLS


PROTO_NORMAL(__get_tcb);
PROTO_NORMAL(__set_tcb);

#endif /* _LIBC_TIB_H_ */
