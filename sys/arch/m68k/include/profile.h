/*	$NetBSD: profile.h,v 1.3 1995/03/26 17:08:37 briggs Exp $	*/

/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)profile.h	8.1 (Berkeley) 6/10/93
 */

#define	_MCOUNT_DECL static __inline void _mcount

#define	MCOUNT \
extern void mcount() __asm("mcount"); void mcount() { \
	int selfpc, frompcindex; \
	__asm("movl a6@(4),%0" : "=r" (selfpc)); \
	__asm("movl a6@(0)@(4),%0" : "=r" (frompcindex)); \
	_mcount(frompcindex, selfpc); \
}

#ifdef _KERNEL
/*
 * The following two macros do splhigh and splx respectively.
 * They have to be defined this way because these are real
 * functions on the HP, and we do not want to invoke mcount
 * recursively.
 */
#define MCOUNT_ENTER \
	__asm("movw	sr,%0" : "=g" (s)); \
	__asm("movw	#0x2700,sr")

#define MCOUNT_EXIT \
	__asm("movw	%0,sr" : : "g" (s))
#endif /* _KERNEL */
