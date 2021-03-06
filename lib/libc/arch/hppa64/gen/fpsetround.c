/*	$OpenBSD: fpsetround.c,v 1.2 2014/04/18 15:09:52 guenther Exp $	*/

/*
 * Written by Miodrag Vallat.  Public domain
 */

#include <sys/types.h>
#include <ieeefp.h>

fp_rnd
fpsetround(rnd_dir)
	fp_rnd rnd_dir;
{
	u_int64_t fpsr;
	fp_rnd old;

	__asm__ volatile("fstd %%fr0,0(%1)" : "=m" (fpsr) : "r" (&fpsr));
	old = (fpsr >> 41) & 0x03;
	fpsr = (fpsr & 0xfffff9ff00000000LL) |
	    ((u_int64_t)(rnd_dir & 0x03) << 41);
	__asm__ volatile("fldd 0(%0),%%fr0" : : "r" (&fpsr));
	return (old);
}
