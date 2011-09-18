/*	$OpenBSD: altq_hfsc.h,v 1.9 2011/09/18 20:34:29 henning Exp $	*/
/*	$KAME: altq_hfsc.h,v 1.8 2002/11/29 04:36:23 kjc Exp $	*/

/*
 * Copyright (c) 1997-1999 Carnegie Mellon University. All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation is hereby granted (including for commercial or
 * for-profit use), provided that both the copyright notice and this
 * permission notice appear in all copies of the software, derivative
 * works, or modified versions, and any portions thereof.
 *
 * THIS SOFTWARE IS EXPERIMENTAL AND IS KNOWN TO HAVE BUGS, SOME OF
 * WHICH MAY HAVE SERIOUS CONSEQUENCES.  CARNEGIE MELLON PROVIDES THIS
 * SOFTWARE IN ITS ``AS IS'' CONDITION, AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * Carnegie Mellon encourages (but does not require) users of this
 * software to return any improvements or extensions that they make,
 * and to grant Carnegie Mellon the rights to redistribute these
 * changes without encumbrance.
 */
#ifndef _ALTQ_ALTQ_HFSC_H_
#define	_ALTQ_ALTQ_HFSC_H_

#include <altq/altq.h>
#include <altq/altq_classq.h>
#include <altq/altq_red.h>

#ifdef __cplusplus
extern "C" {
#endif

struct service_curve {
	u_int	m1;	/* slope of the first segment in bits/sec */
	u_int	d;	/* the x-projection of the first segment in msec */
	u_int	m2;	/* slope of the second segment in bits/sec */
};

/* special class handles */
#define	HFSC_NULLCLASS_HANDLE	0
#define	HFSC_MAX_CLASSES	64

/* hfsc class flags */
#define	HFCF_RED		0x0001	/* use RED */
#define	HFCF_ECN		0x0002  /* use RED/ECN */
#define	HFCF_RIO		0x0004  /* use RIO */
#define	HFCF_DEFAULTCLASS	0x1000	/* default class */

/* service curve types */
#define	HFSC_REALTIMESC		1
#define	HFSC_LINKSHARINGSC	2
#define	HFSC_UPPERLIMITSC	4
#define	HFSC_DEFAULTSC		(HFSC_REALTIMESC|HFSC_LINKSHARINGSC)

struct hfsc_classstats {
	u_int			class_id;
	u_int32_t		class_handle;
	struct service_curve	rsc;
	struct service_curve	fsc;
	struct service_curve	usc;	/* upper limit service curve */

	u_int64_t		total;	/* total work in bytes */
	u_int64_t		cumul;	/* cumulative work in bytes
					   done by real-time criteria */
	u_int64_t		d;		/* deadline */
	u_int64_t		e;		/* eligible time */
	u_int64_t		vt;		/* virtual time */
	u_int64_t		f;		/* fit time for upper-limit */

	/* info helpful for debugging */
	u_int64_t		initvt;		/* init virtual time */
	u_int64_t		vtoff;		/* cl_vt_ipoff */
	u_int64_t		cvtmax;		/* cl_maxvt */
	u_int64_t		myf;		/* cl_myf */
	u_int64_t		cfmin;		/* cl_mincf */
	u_int64_t		cvtmin;		/* cl_mincvt */
	u_int64_t		myfadj;		/* cl_myfadj */
	u_int64_t		vtadj;		/* cl_vtadj */
	u_int64_t		cur_time;
	u_int32_t		machclk_freq;

	u_int			qlength;
	u_int			qlimit;
	struct pktcntr		xmit_cnt;
	struct pktcntr		drop_cnt;
	u_int			period;

	u_int			vtperiod;	/* vt period sequence no */
	u_int			parentperiod;	/* parent's vt period seqno */
	int			nactive;	/* number of active children */

	/* red and rio related info */
	int			qtype;
	struct redstats		red[3];
};

#ifdef _KERNEL
/*
 * kernel internal service curve representation
 *	coordinates are given by 64 bit unsigned integers.
 *	x-axis: unit is clock count.  for the intel x86 architecture,
 *		the raw Pentium TSC (Timestamp Counter) value is used.
 *		virtual time is also calculated in this time scale.
 *	y-axis: unit is byte.
 *
 *	the service curve parameters are converted to the internal
 *	representation.
 *	the slope values are scaled to avoid overflow.
 *	the inverse slope values as well as the y-projection of the 1st
 *	segment are kept in order to to avoid 64-bit divide operations
 *	that are expensive on 32-bit architectures.
 *
 *  note: Intel Pentium TSC never wraps around in several thousands of years.
 *	x-axis doesn't wrap around for 1089 years with 1GHz clock.
 *      y-axis doesn't wrap around for 4358 years with 1Gbps bandwidth.
 */

/* kernel internal representation of a service curve */
struct internal_sc {
	u_int64_t	sm1;	/* scaled slope of the 1st segment */
	u_int64_t	ism1;	/* scaled inverse-slope of the 1st segment */
	u_int64_t	dx;	/* the x-projection of the 1st segment */
	u_int64_t	dy;	/* the y-projection of the 1st segment */
	u_int64_t	sm2;	/* scaled slope of the 2nd segment */
	u_int64_t	ism2;	/* scaled inverse-slope of the 2nd segment */
};

/* runtime service curve */
struct runtime_sc {
	u_int64_t	x;	/* current starting position on x-axis */
	u_int64_t	y;	/* current starting position on x-axis */
	u_int64_t	sm1;	/* scaled slope of the 1st segment */
	u_int64_t	ism1;	/* scaled inverse-slope of the 1st segment */
	u_int64_t	dx;	/* the x-projection of the 1st segment */
	u_int64_t	dy;	/* the y-projection of the 1st segment */
	u_int64_t	sm2;	/* scaled slope of the 2nd segment */
	u_int64_t	ism2;	/* scaled inverse-slope of the 2nd segment */
};

/* for TAILQ based ellist and actlist implementation */
struct altq_hfsc_class;
typedef TAILQ_HEAD(_eligible, altq_hfsc_class) ellist_t;
typedef TAILQ_ENTRY(altq_hfsc_class) elentry_t;
typedef TAILQ_HEAD(_active, altq_hfsc_class) actlist_t;
typedef TAILQ_ENTRY(altq_hfsc_class) actentry_t;
#define	ellist_first(s)		TAILQ_FIRST(s)
#define	actlist_first(s)	TAILQ_FIRST(s)
#define	actlist_last(s)		TAILQ_LAST(s, _active)

struct altq_hfsc_class {
	u_int		cl_id;		/* class id (just for debug) */
	u_int32_t	cl_handle;	/* class handle */
	struct altq_hfsc_if *cl_hif;	/* back pointer to altq_hfsc_if */
	int		cl_flags;	/* misc flags */

	struct altq_hfsc_class *cl_parent;	/* parent class */
	struct altq_hfsc_class *cl_siblings;	/* sibling classes */
	struct altq_hfsc_class *cl_children;	/* child classes */

	class_queue_t	*cl_q;		/* class queue structure */
	struct red	*cl_red;	/* RED state */
	struct altq_pktattr *cl_pktattr; /* saved header used by ECN */

	u_int64_t	cl_total;	/* total work in bytes */
	u_int64_t	cl_cumul;	/* cumulative work in bytes
					   done by real-time criteria */
	u_int64_t	cl_d;		/* deadline */
	u_int64_t	cl_e;		/* eligible time */
	u_int64_t	cl_vt;		/* virtual time */
	u_int64_t	cl_f;		/* time when this class will fit for
					   link-sharing, max(myf, cfmin) */
	u_int64_t	cl_myf;		/* my fit-time (as calculated from this
					   class's own upperlimit curve) */
	u_int64_t	cl_myfadj;	/* my fit-time adjustment
					   (to cancel history dependence) */
	u_int64_t	cl_cfmin;	/* earliest children's fit-time (used
					   with cl_myf to obtain cl_f) */
	u_int64_t	cl_cvtmin;	/* minimal virtual time among the
					   children fit for link-sharing
					   (monotonic within a period) */
	u_int64_t	cl_vtadj;	/* intra-period cumulative vt
					   adjustment */
	u_int64_t	cl_vtoff;	/* inter-period cumulative vt offset */
	u_int64_t	cl_cvtmax;	/* max child's vt in the last period */

	u_int64_t	cl_initvt;	/* init virtual time (for debugging) */

	struct internal_sc *cl_rsc;	/* internal real-time service curve */
	struct internal_sc *cl_fsc;	/* internal fair service curve */
	struct internal_sc *cl_usc;	/* internal upperlimit service curve */
	struct runtime_sc  cl_deadline;	/* deadline curve */
	struct runtime_sc  cl_eligible;	/* eligible curve */
	struct runtime_sc  cl_virtual;	/* virtual curve */
	struct runtime_sc  cl_ulimit;	/* upperlimit curve */

	u_int		cl_vtperiod;	/* vt period sequence no */
	u_int		cl_parentperiod;  /* parent's vt period seqno */
	int		cl_nactive;	/* number of active children */
	actlist_t	*cl_actc;	/* active children list */

	actentry_t	cl_actlist;	/* active children list entry */
	elentry_t	cl_ellist;	/* eligible list entry */

	struct {
		struct pktcntr	xmit_cnt;
		struct pktcntr	drop_cnt;
		u_int period;
	} cl_stats;
};

/*
 * hfsc interface state
 */
struct altq_hfsc_if {
	struct altq_hfsc_if	*hif_next;	/* interface state list */
	struct ifaltq		*hif_ifq;	/* backpointer to ifaltq */
	struct altq_hfsc_class	*hif_rootclass;		/* root class */
	struct altq_hfsc_class	*hif_defaultclass;	/* default class */
	struct altq_hfsc_class	*hif_class_tbl[HFSC_MAX_CLASSES];
	struct altq_hfsc_class	*hif_pollcache;	/* cache for poll operation */

	u_int	hif_classes;			/* # of classes in the tree */
	u_int	hif_packets;			/* # of packets in the tree */
	u_int	hif_classid;			/* class id sequence number */

	ellist_t *hif_eligible;			/* eligible list */
};

#endif /* _KERNEL */

#ifdef __cplusplus
}
#endif

#endif /* _ALTQ_ALTQ_HFSC_H_ */
