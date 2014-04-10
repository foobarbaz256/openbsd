/*	$OpenBSD: buf.h,v 1.94 2014/04/10 13:48:24 tedu Exp $	*/
/*	$NetBSD: buf.h,v 1.25 1997/04/09 21:12:17 mycroft Exp $	*/

/*
 * Copyright (c) 1982, 1986, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *	@(#)buf.h	8.7 (Berkeley) 1/21/94
 */

#ifndef _SYS_BUF_H_
#define	_SYS_BUF_H_
#include <sys/queue.h>
#include <sys/tree.h>
#include <sys/mutex.h>

#define NOLIST ((struct buf *)0x87654321)

struct buf;
struct vnode;

struct buf_rb_bufs;
RB_PROTOTYPE(buf_rb_bufs, buf, b_rbbufs, rb_buf_compare);

LIST_HEAD(bufhead, buf);

/*
 * To avoid including <ufs/ffs/softdep.h>
 */

LIST_HEAD(workhead, worklist);

/*
 * Buffer queues
 */
#define BUFQ_NSCAN_N	128
#define BUFQ_FIFO	0
#define BUFQ_NSCAN	1
#define BUFQ_DEFAULT	BUFQ_NSCAN
#define BUFQ_HOWMANY	2

/*
 * Write limits for bufq - defines high and low water marks for how
 * many kva slots are allowed to be consumed to parallelize writes from
 * the buffer cache from any individual bufq.
 */
#define BUFQ_HI		128
#define BUFQ_LOW	64

struct bufq_impl;

struct bufq {
	SLIST_ENTRY(bufq)	 bufq_entries;
	struct mutex	 	 bufq_mtx;
	void			*bufq_data;
	u_int			 bufq_outstanding;
	u_int			 bufq_hi;
	u_int			 bufq_low;
	int			 bufq_waiting;
	int			 bufq_stop;
	int			 bufq_type;
	const struct bufq_impl	*bufq_impl;
};

int		 bufq_init(struct bufq *, int);
int		 bufq_switch(struct bufq *, int);
void		 bufq_destroy(struct bufq *);

void		 bufq_queue(struct bufq *, struct buf *);
struct buf	*bufq_dequeue(struct bufq *);
void		 bufq_requeue(struct bufq *, struct buf *);
int		 bufq_peek(struct bufq *);
void		 bufq_drain(struct bufq *);

void		 bufq_wait(struct bufq *, struct buf *);
void		 bufq_done(struct bufq *, struct buf *);
void		 bufq_quiesce(void);
void		 bufq_restart(void);

/* disksort */
struct bufq_disksort {
	struct buf	 *bqd_actf;
	struct buf	**bqd_actb;
};

/* fifo */
SIMPLEQ_HEAD(bufq_fifo_head, buf);
struct bufq_fifo {
	SIMPLEQ_ENTRY(buf)	bqf_entries;
};

/* nscan */
SIMPLEQ_HEAD(bufq_nscan_head, buf);
struct bufq_nscan {
	SIMPLEQ_ENTRY(buf)	bqf_entries;
};

/* bufq link in struct buf */
union bufq_data {
	struct bufq_disksort	bufq_data_disksort;
	struct bufq_fifo	bufq_data_fifo;
	struct bufq_nscan	bufq_data_nscan;
};

/*
 * These are currently used only by the soft dependency code, hence
 * are stored once in a global variable. If other subsystems wanted
 * to use these hooks, a pointer to a set of bio_ops could be added
 * to each buffer.
 */
extern struct bio_ops {
	void	(*io_start)(struct buf *);
	void	(*io_complete)(struct buf *);
	void	(*io_deallocate)(struct buf *);
	void	(*io_movedeps)(struct buf *, struct buf *);
	int	(*io_countdeps)(struct buf *, int, int);
} bioops;

/* XXX: disksort(); */
#define b_actf	b_bufq.bufq_data_disksort.bqd_actf
#define b_actb	b_bufq.bufq_data_disksort.bqd_actb

/* The buffer header describes an I/O operation in the kernel. */
struct buf {
	RB_ENTRY(buf) b_rbbufs;		/* vnode "hash" tree */
	LIST_ENTRY(buf) b_list;		/* All allocated buffers. */
	LIST_ENTRY(buf) b_vnbufs;	/* Buffer's associated vnode. */
	TAILQ_ENTRY(buf) b_freelist;	/* Free list position if not active. */
	struct  proc *b_proc;		/* Associated proc; NULL if kernel. */
	volatile long	b_flags;	/* B_* flags. */
	int	b_error;		/* Errno value. */
	long	b_bufsize;		/* Allocated buffer size. */
	long	b_bcount;		/* Valid bytes in buffer. */
	size_t	b_resid;		/* Remaining I/O. */
	dev_t	b_dev;			/* Device associated with buffer. */
	caddr_t	b_data;			/* associated data */
	void	*b_saveaddr;		/* Original b_data for physio. */

	TAILQ_ENTRY(buf) b_valist;	/* LRU of va to reuse. */

	union	bufq_data b_bufq;
	struct	bufq	  *b_bq;	/* What bufq this buf is on */

	struct uvm_object *b_pobj;	/* Object containing the pages */
	off_t	b_poffs;		/* Offset within object */

	daddr_t	b_lblkno;		/* Logical block number. */
	daddr_t	b_blkno;		/* Underlying physical block number. */
					/* Function to call upon completion.
					 * Will be called at splbio(). */
	void	(*b_iodone)(struct buf *);
	struct	vnode *b_vp;		/* Device vnode. */
	int	b_dirtyoff;		/* Offset in buffer of dirty region. */
	int	b_dirtyend;		/* Offset of end of dirty region. */
	int	b_validoff;		/* Offset in buffer of valid region. */
	int	b_validend;		/* Offset of end of valid region. */
 	struct	workhead b_dep;		/* List of filesystem dependencies. */
};

/* Device driver compatibility definitions. */
#define	b_active b_bcount		/* Driver queue head: drive active. */

/*
 * These flags are kept in b_flags.
 */
#define	B_WRITE		0x00000000	/* Write buffer (pseudo flag). */
#define	B_AGE		0x00000001	/* Move to age queue when I/O done. */
#define	B_NEEDCOMMIT	0x00000002	/* Needs committing to stable storage */
#define	B_ASYNC		0x00000004	/* Start I/O, do not wait. */
#define	B_BAD		0x00000008	/* Bad block revectoring in progress. */
#define	B_BUSY		0x00000010	/* I/O in progress. */
#define	B_CACHE		0x00000020	/* Bread found us in the cache. */
#define	B_CALL		0x00000040	/* Call b_iodone from biodone. */
#define	B_DELWRI	0x00000080	/* Delay I/O until buffer reused. */
#define	B_DONE		0x00000100	/* I/O completed. */
#define	B_EINTR		0x00000200	/* I/O was interrupted */
#define	B_ERROR		0x00000400	/* I/O error occurred. */
#define	B_INVAL		0x00000800	/* Does not contain valid info. */
#define	B_NOCACHE	0x00001000	/* Do not cache block after use. */
#define	B_PHYS		0x00002000	/* I/O to user memory. */
#define	B_RAW		0x00004000	/* Set by physio for raw transfers. */
#define	B_READ		0x00008000	/* Read buffer. */
#define	B_WANTED	0x00010000	/* Process wants this buffer. */
#define	B_WRITEINPROG	0x00020000	/* Write in progress. */
#define	B_XXX		0x00040000	/* Debugging flag. */
#define	B_DEFERRED	0x00080000	/* Skipped over for cleaning */
#define	B_SCANNED	0x00100000	/* Block already pushed during sync */
#define	B_PDAEMON	0x00200000	/* I/O started by pagedaemon */
#define	B_RELEASED	0x00400000	/* free this buffer after its kvm */

#define	B_BITS	"\20\001AGE\002NEEDCOMMIT\003ASYNC\004BAD\005BUSY" \
    "\006CACHE\007CALL\010DELWRI\011DONE\012EINTR\013ERROR" \
    "\014INVAL\015NOCACHE\016PHYS\017RAW\020READ" \
    "\021WANTED\022WRITEINPROG\023XXX(FORMAT)\024DEFERRED" \
    "\025SCANNED\026DAEMON\027RELEASED"

/*
 * This structure describes a clustered I/O.  It is stored in the b_saveaddr
 * field of the buffer on which I/O is done.  At I/O completion, cluster
 * callback uses the structure to parcel I/O's to individual buffers, and
 * then free's this structure.
 */
struct cluster_save {
	long	bs_bcount;		/* Saved b_bcount. */
	long	bs_bufsize;		/* Saved b_bufsize. */
	void	*bs_saveaddr;		/* Saved b_addr. */
	int	bs_nchildren;		/* Number of associated buffers. */
	struct buf **bs_children;	/* List of associated buffers. */
};

/*
 * Zero out the buffer's data area.
 */
#define	clrbuf(bp) {							\
	bzero((bp)->b_data, (u_int)(bp)->b_bcount);			\
	(bp)->b_resid = 0;						\
}


/* Flags to low-level allocation routines. */
#define B_CLRBUF	0x01	/* Request allocated buffer be cleared. */
#define B_SYNC		0x02	/* Do all allocations synchronously. */

struct cluster_info {
	daddr_t	ci_lastr;	/* last read (read-ahead) */
	daddr_t	ci_lastw;	/* last write (write cluster) */
	daddr_t	ci_cstart;	/* start block of cluster */
	daddr_t	ci_lasta;	/* last allocation */
	int	ci_clen; 	/* length of current cluster */
	int	ci_ralen;	/* Read-ahead length */
	daddr_t	ci_maxra;	/* last readahead block */
};

#ifdef _KERNEL
__BEGIN_DECLS
/* Kva slots (of size MAXPHYS) reserved for syncer and cleaner. */
#define RESERVE_SLOTS 4
/* Buffer cache pages reserved for syncer and cleaner. */
#define RESERVE_PAGES (RESERVE_SLOTS * MAXPHYS / PAGE_SIZE)
/* Minimum size of the buffer cache, in pages. */
#define BCACHE_MIN (RESERVE_PAGES * 2)
#define UNCLEAN_PAGES (bcstats.numbufpages - bcstats.numcleanpages)

extern struct proc *cleanerproc;
extern long bufpages;		/* Max number of pages for buffers' data */
extern struct pool bufpool;
extern struct bufhead bufhead;

void	bawrite(struct buf *);
void	bdwrite(struct buf *);
void	biodone(struct buf *);
int	biowait(struct buf *);
int bread(struct vnode *, daddr_t, int, struct buf **);
int breadn(struct vnode *, daddr_t, int, daddr_t *, int *, int,
    struct buf **);
void	brelse(struct buf *);
#define bremfree bufcache_take
void	bufinit(void);
void	buf_dirty(struct buf *);
void    buf_undirty(struct buf *);
int	bwrite(struct buf *);
struct buf *getblk(struct vnode *, daddr_t, int, int, int);
struct buf *geteblk(int);
struct buf *incore(struct vnode *, daddr_t);

/*
 * bufcache functions
 */
void bufcache_init(void);

void bufcache_take(struct buf *);
void bufcache_release(struct buf *);

struct buf *bufcache_getcleanbuf(void);
struct buf *bufcache_getdirtybuf(void);

/*
 * buf_kvm_init initializes the kvm handling for buffers.
 * buf_acquire sets the B_BUSY flag and ensures that the buffer is
 * mapped in the kvm.
 * buf_release clears the B_BUSY flag and allows the buffer to become
 * unmapped.
 * buf_unmap is for internal use only. Unmaps the buffer from kvm.
 */
void	buf_mem_init(vsize_t);
void	buf_acquire(struct buf *);
void	buf_acquire_unmapped(struct buf *);
void	buf_acquire_nomap(struct buf *);
void	buf_map(struct buf *);
void	buf_release(struct buf *);
int	buf_dealloc_mem(struct buf *);
void	buf_fix_mapping(struct buf *, vsize_t);
void	buf_alloc_pages(struct buf *, vsize_t);
void	buf_free_pages(struct buf *);


void	minphys(struct buf *bp);
int	physio(void (*strategy)(struct buf *), dev_t dev, int flags,
	    void (*minphys)(struct buf *), struct uio *uio);
void  brelvp(struct buf *);
void  reassignbuf(struct buf *);
void  bgetvp(struct vnode *, struct buf *);

void  buf_replacevnode(struct buf *, struct vnode *);
void  buf_daemon(struct proc *);
void  buf_replacevnode(struct buf *, struct vnode *);
void  buf_daemon(struct proc *);
int bread_cluster(struct vnode *, daddr_t, int, struct buf **);

#ifdef DEBUG
void buf_print(struct buf *);
#endif

static __inline void
buf_start(struct buf *bp)
{
	if (bioops.io_start)
		(*bioops.io_start)(bp);
}

static __inline void
buf_complete(struct buf *bp)
{
	if (bioops.io_complete)
		(*bioops.io_complete)(bp);
}

static __inline void
buf_deallocate(struct buf *bp)
{
	if (bioops.io_deallocate)
		(*bioops.io_deallocate)(bp);
}

static __inline void
buf_movedeps(struct buf *bp, struct buf *bp2)
{
	if (bioops.io_movedeps)
		(*bioops.io_movedeps)(bp, bp2);
}

static __inline int
buf_countdeps(struct buf *bp, int i, int islocked)
{
	if (bioops.io_countdeps)
		return ((*bioops.io_countdeps)(bp, i, islocked));
	else
		return (0);
}

void	cluster_write(struct buf *, struct cluster_info *, u_quad_t);

__END_DECLS
#endif /* _KERNEL */
#endif /* !_SYS_BUF_H_ */
