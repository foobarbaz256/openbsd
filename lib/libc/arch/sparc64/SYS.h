/*	$OpenBSD: SYS.h,v 1.13 2015/08/31 02:53:57 guenther Exp $	*/
/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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
 *	@(#)SYS.h	8.1 (Berkeley) 6/4/93
 *
 *	from: Header: SYS.h,v 1.2 92/07/03 18:57:00 torek Exp
 *	$NetBSD: SYS.h,v 1.6 2001/07/23 07:26:50 thorpej Exp $
 */

#include <machine/asm.h>
#include <sys/syscall.h>
#include <machine/trap.h>

#define _CAT(x,y) x##y

#define	__ENTRY(p,x)		ENTRY(_CAT(p,x)) ; .weak x; x = _CAT(p,x)
#define	__ENTRY_HIDDEN(p,x)	ENTRY(_CAT(p,x))


/*
 * We define a hidden alias with the prefix "_libc_" for each global symbol
 * that may be used internally.  By referencing _libc_x instead of x, other
 * parts of libc prevent overriding by the application and avoid unnecessary
 * relocations.
 */
#define _HIDDEN(x)		_libc_##x
#define _HIDDEN_ALIAS(x,y)			\
	STRONG_ALIAS(_HIDDEN(x),y);		\
	.hidden _HIDDEN(x)
#define _HIDDEN_FALIAS(x,y)			\
	_HIDDEN_ALIAS(x,y);			\
	.type _HIDDEN(x),@function

/*
 * For functions implemented in ASM that aren't syscalls.
 *   END_STRONG(x)	Like DEF_STRONG() in C; for standard/reserved C names
 *   END_WEAK(x)	Like DEF_WEAK() in C; for non-ISO C names
 */
#define	END_STRONG(x)	END(x); _HIDDEN_FALIAS(x,x); END(_HIDDEN(x))
#define	END_WEAK(x)	END_STRONG(x); .weak x


/*
 * ERROR branches to cerror.  This is done with a macro so that I can
 * change it to be position independent later, if need be.
 */
#ifdef __PIC__
#define	CALL(name) \
	PIC_PROLOGUE(%g1,%g2); \
	sethi %hi(name),%g2; \
	or %g2,%lo(name),%g2; \
	ldx [%g1+%g2],%g2; \
	jmp %g2; \
	nop
#else
#define	CALL(name) \
	sethi %hi(name),%g1; or %lo(name),%g1,%g1; \
	jmp %g1; nop
#endif
#define	ERROR()	CALL(_C_LABEL(__cerror))

/*
 * SYSCALL is used when further action must be taken before returning.
 * Note that it adds a `nop' over what we could do, if we only knew what
 * came at label 1....
 */
#define	_SYSCALL(p,x,y) \
	__ENTRY(p,x); mov _CAT(SYS_,y),%g1; t ST_SYSCALL; bcc 1f; nop; ERROR(); 1:
#define	_SYSCALL_HIDDEN(p,x,y) \
	__ENTRY_HIDDEN(p,x); mov _CAT(SYS_,y),%g1; t ST_SYSCALL; bcc 1f; nop; ERROR(); 1:

#define	__SYSCALL(p,x) \
	_SYSCALL(p,x,x)

#define	__SYSCALL_HIDDEN(p,x) \
	_SYSCALL_HIDDEN(p,x,x)

/*
 * RSYSCALL is used when the system call should just return.  Here
 * we use the SYSCALL_G2RFLAG to put the `success' return address in %g2
 * and avoid a branch.
 */
#define	__RSYSCALL(p,x) \
	__ENTRY(p,x); mov (_CAT(SYS_,x))|SYSCALL_G2RFLAG,%g1; \
	add %o7,8,%g2; t ST_SYSCALL; ERROR()
#define	__RSYSCALL_HIDDEN(p,x) \
	__ENTRY_HIDDEN(p,x); mov (_CAT(SYS_,x))|SYSCALL_G2RFLAG,%g1; \
	add %o7,8,%g2; t ST_SYSCALL; ERROR()

/*
 * PSEUDO(x,y) is like RSYSCALL(y) except that the name is x.
 */
#define	__PSEUDO(p,x,y) \
	__ENTRY(p,x); mov (_CAT(SYS_,y))|SYSCALL_G2RFLAG,%g1; add %o7,8,%g2; \
	t ST_SYSCALL; ERROR()

/*
 * SYSCALL_NOERROR is like SYSCALL, except it's used for syscalls 
 * that never fail.
 *
 * XXX - This should be optimized.
 */
#define __SYSCALL_NOERROR(p,x) \
	__ENTRY(p,x); mov _CAT(SYS_,x),%g1; t ST_SYSCALL

/*
 * RSYSCALL_NOERROR is like RSYSCALL, except it's used for syscalls 
 * that never fail.
 *
 * XXX - This should be optimized.
 */
#define __RSYSCALL_NOERROR(p,x) \
	__ENTRY(p,x); mov (_CAT(SYS_,x))|SYSCALL_G2RFLAG,%g1; add %o7,8,%g2; \
	t ST_SYSCALL

/*
 * PSEUDO_NOERROR(x,y) is like RSYSCALL_NOERROR(y) except that the name is x.
 */
#define __PSEUDO_NOERROR(p,x,y) \
	__ENTRY(p,x); mov (_CAT(SYS_,y))|SYSCALL_G2RFLAG,%g1; add %o7,8,%g2; \
	t ST_SYSCALL

	.globl	_C_LABEL(__cerror)

/*
 * SYSENTRY is for functions that pretend to be syscalls.
 */
#define __SYSENTRY(p,x) __ENTRY(p,x)

#define	SYSCALL(x)		__SYSCALL(_thread_sys_,x)
#define	RSYSCALL(x)		__RSYSCALL(_thread_sys_,x)
#define	RSYSCALL_HIDDEN(x)	__RSYSCALL_HIDDEN(_thread_sys_,x)
#define	RSYSCALL_NOERROR(x,y)	__RSYSCALL_NOERROR(_thread_sys_,x,y)
#define	PSEUDO(x,y)		__PSEUDO(_thread_sys_,x,y)
#define	PSEUDO_NOERROR(x,y)	__PSEUDO_NOERROR(_thread_sys_,x,y)
#define	SYSENTRY(x)		__SYSENTRY(_thread_sys_,x)
