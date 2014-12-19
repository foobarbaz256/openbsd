/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <sys/endian.h>

#undef htons

u_int16_t		htons(u_int16_t);

u_int16_t
htons(u_int16_t x)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	u_char *s = (u_char *) &x;
	return (u_int16_t)(s[0] << 8 | s[1]);
#else
	return x;
#endif
}
