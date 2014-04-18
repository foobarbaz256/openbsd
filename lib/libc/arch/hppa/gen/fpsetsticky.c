/*	$OpenBSD: fpsetsticky.c,v 1.6 2014/04/18 15:09:52 guenther Exp $	*/

/*
 * Written by Miodrag Vallat.  Public domain
 */

#include <sys/types.h>
#include <ieeefp.h>

fp_except
fpsetsticky(mask)
	fp_except mask;
{
	u_int64_t fpsr;
	fp_except old;

	__asm__ volatile("fstd %%fr0,0(%1)" : "=m" (fpsr) : "r" (&fpsr));
	old = (fpsr >> 59) & 0x1f;
	fpsr = (fpsr & 0x07ffffff00000000LL) | ((u_int64_t)(mask & 0x1f) << 59);
	__asm__ volatile("fldd 0(%0),%%fr0" : : "r" (&fpsr));
	return (old);
}
