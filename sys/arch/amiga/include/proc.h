/*	$OpenBSD: proc.h,v 1.3 1997/01/16 09:26:02 niklas Exp $	*/
/*	$NetBSD: proc.h,v 1.10 1994/10/26 02:06:27 cgd Exp $	*/

/*
 * Copyright (c) 1991 Regents of the University of California.
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
 *	@(#)proc.h	7.1 (Berkeley) 5/15/91
 */
#ifndef _MACHINE_PROC_H_
#define _MACHINE_PROC_H_

/*
 * Machine-dependent part of the proc structure for amiga.
 */
struct mdproc {
	int	*md_regs;		/* registers on current frame */
	long	md_flags;		/* machine-dependent flags */
};

/* md_flags */
#define	MDP_AST		0x0001	/* async trap pending */
#define MDP_STACKADJ	0x0002	/* frame SP adjusted, might have to
				   undo when system call returns
				   ERESTART. */
#define MDP_UNCACHE_WX	0x0004	/* The process might modify code, so
				   don't cache writeable executable pages.  */
#endif /* !_MACHINE_PROC_H_ */
