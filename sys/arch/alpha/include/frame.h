/*	$OpenBSD: frame.h,v 1.2 1996/07/29 22:58:47 niklas Exp $	*/
/*	$NetBSD: frame.h,v 1.1 1995/02/13 23:07:39 cgd Exp $	*/

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

#ifndef _ALPHA_FRAME_H_
#define	_ALPHA_FRAME_H_

/*
 * XXX where did this info come from?
 */

/*
 * Trap and syscall frame.
 *
 * Hardware puts fields marked "[HW]" on stack.  We have to add
 * all of the general-purpose registers except for zero, for sp,
 * which is automatically saved in usp for traps, and implicitly
 * saved for syscalls, and for a0-a2, which are saved by hardware.
 */

/* Number of registers saved, including padding. */
#define	FRAME_NSAVEREGS	28

/* The offsets of the registers to be saved, into the array. */
#define	FRAME_V0	0
#define	FRAME_T0	1
#define	FRAME_T1	2
#define	FRAME_T2	3
#define	FRAME_T3	4
#define	FRAME_T4	5
#define	FRAME_T5	6
#define	FRAME_T6	7
#define	FRAME_T7	8
#define	FRAME_S0	9
#define	FRAME_S1	10
#define	FRAME_S2	11
#define	FRAME_S3	12
#define	FRAME_S4	13
#define	FRAME_S5	14
#define	FRAME_S6	15
#define	FRAME_A3	16
#define	FRAME_A4	17
#define	FRAME_A5	18
#define	FRAME_T8	19
#define	FRAME_T9	20
#define	FRAME_T10	21
#define	FRAME_T11	22
#define	FRAME_RA	23
#define	FRAME_T12	24
#define	FRAME_AT	25
#define	FRAME_SP	26
#define	FRAME_SPARE	27	/* spare; padding */

struct trapframe {
	u_int64_t	tf_regs[FRAME_NSAVEREGS]; /* GPRs (listed above) */
	u_int64_t	tf_ps;			/* processor status [HW] */
	u_int64_t	tf_pc;			/* program counter [HW] */
	u_int64_t	tf_gp;			/* global pointer [HW] */
	u_int64_t	tf_a0;			/* saved a0 [HW] */
	u_int64_t	tf_a1;			/* saved a1 [HW] */
	u_int64_t	tf_a2;			/* saved a2 [HW] */
};

#endif /* _ALPHA_FRAME_H_ */
