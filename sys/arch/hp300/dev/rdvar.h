/*	$OpenBSD: rdvar.h,v 1.5 1997/01/12 15:13:00 downsj Exp $	*/
/*	$NetBSD: rdvar.h,v 1.5 1996/06/06 16:17:42 thorpej Exp $	*/

/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1982, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
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
 * from: Utah $Hdr: rdvar.h 1.1 92/12/21$
 *
 *	@(#)rdvar.h	8.1 (Berkeley) 6/10/93
 */

struct	rdidentinfo {
	short	ri_hwid;		/* 2 byte HW id */
	short	ri_maxunum;		/* maximum allowed unit number */
	char	*ri_desc;		/* drive type description */
	int	ri_nbpt;		/* DEV_BSIZE blocks per track */
	int	ri_ntpc;		/* tracks per cylinder */
	int	ri_ncyl;		/* cylinders per unit */
	int	ri_nblocks;		/* DEV_BSIZE blocks on disk */
};

struct	rd_softc {
	struct	hp_device *sc_hd;
	struct	disk sc_dkdev;
	char	sc_xname[8];
	int	sc_flags;
	short	sc_type;
	short	sc_punit;
	char	*sc_addr;
	int	sc_resid;
	struct	rd_describe sc_rddesc;
	struct	devqueue sc_dq;
	struct	rd_iocmd sc_ioc;
	struct	rd_rscmd sc_rsc;
	struct	rd_stat sc_stat;
	struct	rd_ssmcmd sc_ssmc;
	struct	rd_srcmd sc_src;
	struct	rd_clearcmd sc_clear;
};

/* sc_flags values */
#define	RDF_ALIVE	0x01
#define	RDF_SEEK	0x02
#define RDF_SWAIT	0x04
#define RDF_OPENING	0x08
#define RDF_CLOSING	0x10
#define RDF_WANTED	0x20
#define RDF_WLABEL	0x40

struct rdstats {
	long	rdretries;
	long	rdresets;
	long	rdtimeouts;
	long	rdpolltries;
	long	rdpollwaits;
};

#define	rdunit(x)	(minor(x) >> 3)
#define rdpart(x)	(minor(x) & 0x7)
#define	rdpunit(x)	((x) & 7)
#define rdlabdev(d)	(dev_t)(((int)(d)&~7)|2)	/* rd?c */

#define	b_cylin		b_resid

#define	RDRETRY		5
#define RDWAITC		1	/* min time for timeout in seconds */

#ifdef _KERNEL
extern	struct rdidentinfo rdidentinfo[];
extern	struct rd_softc rd_softc[];
#endif
