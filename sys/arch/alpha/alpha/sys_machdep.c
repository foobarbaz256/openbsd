/*	$NetBSD: sys_machdep.c,v 1.2 1995/09/19 23:00:53 thorpej Exp $	*/

/*
 * Copyright (c) 1994, 1995 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 * 
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND 
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

#include <sys/param.h>
#include <sys/systm.h>

#include <sys/mount.h>
#include <sys/syscallargs.h>

int
sysarch(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct sysarch_args /* {
		syscallarg(int) op;
		syscallarg(char *) parms;
	} */ *uap = v;

	return (ENOSYS);
}
