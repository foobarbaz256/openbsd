/*	$OpenBSD: htonl.c,v 1.9 2014/12/20 18:15:29 miod Exp $	*/
/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <sys/types.h>
#include <sys/endian.h>

#undef htonl

u_int32_t	htonl(u_int32_t);

u_int32_t
htonl(u_int32_t x)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	u_char *s = (u_char *)&x;
	return (u_int32_t)(s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3]);
#else
	return x;
#endif
}
