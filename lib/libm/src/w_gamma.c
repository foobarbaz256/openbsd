/* @(#)w_gamma.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

#if defined(LIBM_SCCS) && !defined(lint)
static char rcsid[] = "$NetBSD: w_gamma.c,v 1.7 1995/11/20 22:06:43 jtc Exp $";
#endif

/* double gamma(double x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call gamma_r
 */

#include "math.h"
#include "math_private.h"

extern int signgam;

double
gamma(double x)
{
	return lgamma_r(x,&signgam);
}             
