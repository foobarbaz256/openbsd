/*	$OpenBSD: kvm.h,v 1.5 2002/06/08 22:31:24 art Exp $	*/
/*	$NetBSD: kvm.h,v 1.7 1996/04/19 12:02:50 leo Exp $	*/

/*-
 * Copyright (c) 1989, 1993
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
 *	@(#)kvm.h	8.1 (Berkeley) 6/2/93
 */

#ifndef _KVM_H_
#define	_KVM_H_

/* Default version symbol. */
#define	VRS_SYM		"_version"
#define	VRS_KEY		"VERSION"

/*
 * Flag for kvm_open* that disables opening of any files. We're just
 * interested in a handle to run sysctls.
 */
#define KVM_NO_FILES	0x80000000

#include <nlist.h>
#include <sys/cdefs.h>
#include <stdio.h>

__BEGIN_DECLS

typedef struct __kvm kvm_t;

struct kinfo_proc;
int	  kvm_close(kvm_t *);
int	  kvm_dump_inval(kvm_t *);
int	  kvm_dump_mkheader(kvm_t *, off_t);
int	  kvm_dump_wrtheader(kvm_t *, FILE *, int);
char	**kvm_getargv(kvm_t *, const struct kinfo_proc *, int);
char	**kvm_getenvv(kvm_t *, const struct kinfo_proc *, int);
char	 *kvm_geterr(kvm_t *);
int	  kvm_getloadavg(kvm_t *, double [], int);
char	 *kvm_getfiles(kvm_t *, int, int, int *);
struct kinfo_proc *
	  kvm_getprocs(kvm_t *, int, int, int *);
int	  kvm_nlist(kvm_t *, struct nlist *);
kvm_t	 *kvm_open
(const char *, const char *, const char *, int, const char *);
kvm_t	 *kvm_openfiles
(const char *, const char *, const char *, int, char *);
ssize_t	  kvm_read(kvm_t *, u_long, void *, size_t);
ssize_t	  kvm_write(kvm_t *, u_long, const void *, size_t);

__END_DECLS

#endif /* !_KVM_H_ */
