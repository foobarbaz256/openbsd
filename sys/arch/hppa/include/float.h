/*	$OpenBSD: float.h,v 1.3 2000/01/11 10:09:49 mickey Exp $	*/

/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
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
 *	California, Berkeley and its contributors.
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
 *	@(#)float.h	7.1 (Berkeley) 5/8/90
 */

#ifndef _MACHINE_FLOAT_H_
#define _MACHINE_FLOAT_H_

#include <sys/cdefs.h>

#define	FLT_RADIX	2
#define	FLT_ROUNDS	1

#define	FLT_MANT_DIG	24
#define	FLT_EPSILON	1.19209290E-07F
#define	FLT_DIG		6
#define	FLT_MIN_EXP	(-126)
#define	FLT_MIN		1.17549435E-38F
#define	FLT_MIN_10_EXP	(-37)
#define	FLT_MAX_EXP	127
#define	FLT_MAX		3.40282347E+38F
#define	FLT_MAX_10_EXP	38

#define	DBL_MANT_DIG	53
#define	DBL_EPSILON	2.2204460492503131E-16
#define	DBL_DIG		15
#define	DBL_MIN_EXP	(-1022)
#define	DBL_MIN		2.2250738585072014E-308
#define	DBL_MIN_10_EXP	(-307)
#define	DBL_MAX_EXP	1023
#define	DBL_MAX		1.7976931348623157E+308
#define	DBL_MAX_10_EXP	308

#define	LDBL_MANT_DIG	113
#define	LDBL_EPSILON	1.9259299443872358530559779425849273E-34L
#define	LDBL_DIG	33
#define	LDBL_MIN_EXP	(-16382)
#define	LDBL_MIN	3.3621031431120935062626778173217526026E-4932L
#define	LDBL_MIN_10_EXP	(-4931)
#define	LDBL_MAX_EXP	16383
#define	LDBL_MAX	1.1897314953572317650857593266280070162E4932L
#define	LDBL_MAX_10_EXP	4932

#endif /* _MACHINE_FLOAT_H_ */

