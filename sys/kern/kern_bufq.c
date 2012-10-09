/*	$OpenBSD: kern_bufq.c,v 1.21 2012/10/09 15:36:50 beck Exp $	*/
/*
 * Copyright (c) 2010 Thordur I. Bjornsson <thib@openbsd.org>
 * Copyright (c) 2010 David Gwynne <dlg@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/mutex.h>
#include <sys/buf.h>
#include <sys/errno.h>
#include <sys/queue.h>

#include <sys/disklabel.h>

SLIST_HEAD(, bufq)	bufqs = SLIST_HEAD_INITIALIZER(&bufq);
struct mutex		bufqs_mtx = MUTEX_INITIALIZER(IPL_NONE);
int			bufqs_stop;

struct bufq_impl {
	void		*(*impl_create)(void);
	void		 (*impl_destroy)(void *);

	void		 (*impl_queue)(void *, struct buf *);
	struct buf	*(*impl_dequeue)(void *);
	void		 (*impl_requeue)(void *, struct buf *);
	int		 (*impl_peek)(void *);
};

void		*bufq_disksort_create(void);
void		 bufq_disksort_destroy(void *);
void		 bufq_disksort_queue(void *, struct buf *);
struct buf	*bufq_disksort_dequeue(void *);
void		 bufq_disksort_requeue(void *, struct buf *);
int		 bufq_disksort_peek(void *);

void		*bufq_fifo_create(void);
void		 bufq_fifo_destroy(void *);
void		 bufq_fifo_queue(void *, struct buf *);
struct buf	*bufq_fifo_dequeue(void *);
void		 bufq_fifo_requeue(void *, struct buf *);
int		 bufq_fifo_peek(void *);

void		*bufq_nscan_create(void);
void		 bufq_nscan_destroy(void *);
void		 bufq_nscan_queue(void *, struct buf *);
struct buf	*bufq_nscan_dequeue(void *);
void		 bufq_nscan_requeue(void *, struct buf *);
int		 bufq_nscan_peek(void *);

const struct bufq_impl bufq_impls[BUFQ_HOWMANY] = {
	{
		bufq_disksort_create,
		bufq_disksort_destroy,
		bufq_disksort_queue,
		bufq_disksort_dequeue,
		bufq_disksort_requeue,
		bufq_disksort_peek
	},
	{
		bufq_fifo_create,
		bufq_fifo_destroy,
		bufq_fifo_queue,
		bufq_fifo_dequeue,
		bufq_fifo_requeue,
		bufq_fifo_peek
	},
	{
		bufq_nscan_create,
		bufq_nscan_destroy,
		bufq_nscan_queue,
		bufq_nscan_dequeue,
		bufq_nscan_requeue,
		bufq_nscan_peek
	}
};

int
bufq_init(struct bufq *bq, int type)
{
	u_int hi = BUFQ_HI, low = BUFQ_LOW;

	if (type > BUFQ_HOWMANY)
		panic("bufq_init: type %i unknown", type);

	/*
	 * Ensure that writes can't consume the entire amount of kva
	 * available the buffer cache if we only have a limited amount
	 * of kva available to us.
	 */
	if (hi >= (bcstats.kvaslots / 16)) {
		hi = bcstats.kvaslots / 16;
		if (hi < 2)
			hi = 2;
		low = hi / 2;
	}

	mtx_init(&bq->bufq_mtx, IPL_BIO);
	bq->bufq_hi = hi;
	bq->bufq_low = low;
	bq->bufq_type = type;
	bq->bufq_impl = &bufq_impls[type];
	bq->bufq_data = bq->bufq_impl->impl_create();
	if (bq->bufq_data == NULL) {
		/*
		 * we should actually return failure so disks attaching after
		 * boot in low memory situations dont panic the system.
		 */
		panic("bufq init fail");
	}

	mtx_enter(&bufqs_mtx);
	while (bufqs_stop) {
		msleep(&bufqs_stop, &bufqs_mtx, PRIBIO, "bqinit", 0);
	}
	SLIST_INSERT_HEAD(&bufqs, bq, bufq_entries);
	mtx_leave(&bufqs_mtx);

	return (0);
}

int
bufq_switch(struct bufq *bq, int type)
{
	void		*data;
	void		*odata;
	int		otype;
	struct buf	*bp;
	int		ret;

	mtx_enter(&bq->bufq_mtx);
	ret = (bq->bufq_type == type);
	mtx_leave(&bq->bufq_mtx);
	if (ret)
		return (0);

	data = bufq_impls[type].impl_create();
	if (data == NULL)
		return (ENOMEM);

	mtx_enter(&bq->bufq_mtx);
	if (bq->bufq_type != type) { /* might have changed during create */
		odata = bq->bufq_data;
		otype = bq->bufq_type;

		while ((bp = bufq_impls[otype].impl_dequeue(odata)) != NULL)
			bufq_impls[type].impl_queue(data, bp);

		bq->bufq_data = data;
		bq->bufq_type = type;
		bq->bufq_impl = &bufq_impls[type];
	} else {
		otype = type;
		odata = data;
	}
	mtx_leave(&bq->bufq_mtx);

	bufq_impls[otype].impl_destroy(odata);

	return (0);
}

void
bufq_destroy(struct bufq *bq)
{
	bufq_drain(bq);

	bq->bufq_impl->impl_destroy(bq->bufq_data);
	bq->bufq_data = NULL;

	mtx_enter(&bufqs_mtx);
	while (bufqs_stop) {
		msleep(&bufqs_stop, &bufqs_mtx, PRIBIO, "bqdest", 0);
	}
	SLIST_REMOVE(&bufqs, bq, bufq, bufq_entries);
	mtx_leave(&bufqs_mtx);
}


void
bufq_queue(struct bufq *bq, struct buf *bp)
{
	mtx_enter(&bq->bufq_mtx);
	while (bq->bufq_stop) {
		msleep(&bq->bufq_stop, &bq->bufq_mtx, PRIBIO, "bqqueue", 0);
	}

	bp->b_bq = bq;
	bq->bufq_outstanding++;
	bq->bufq_impl->impl_queue(bq->bufq_data, bp);
	mtx_leave(&bq->bufq_mtx);
}

struct buf *
bufq_dequeue(struct bufq *bq)
{
	struct buf	*bp;

	mtx_enter(&bq->bufq_mtx);
	bp = bq->bufq_impl->impl_dequeue(bq->bufq_data);
	mtx_leave(&bq->bufq_mtx);

	return (bp);
}

void
bufq_requeue(struct bufq *bq, struct buf *bp)
{
	mtx_enter(&bq->bufq_mtx);
	bq->bufq_impl->impl_requeue(bq->bufq_data, bp);
	mtx_leave(&bq->bufq_mtx);
}

int
bufq_peek(struct bufq *bq)
{
	int		rv;

	mtx_enter(&bq->bufq_mtx);
	rv = bq->bufq_impl->impl_peek(bq->bufq_data);
	mtx_leave(&bq->bufq_mtx);

	return (rv);
}

void
bufq_drain(struct bufq *bq)
{
	struct buf	*bp;
	int		 s;

	while ((bp = bufq_dequeue(bq)) != NULL) {
		bp->b_error = ENXIO;
		bp->b_flags |= B_ERROR;
		s = splbio();
		biodone(bp);
		splx(s);
	}
}

void
bufq_wait(struct bufq *bq, struct buf *bp)
{
	if (bq->bufq_hi) {
		assertwaitok();
		mtx_enter(&bq->bufq_mtx);
		while (bq->bufq_outstanding >= bq->bufq_hi) {
			bq->bufq_waiting++;
			msleep(&bq->bufq_waiting, &bq->bufq_mtx,
			    PRIBIO, "bqwait", 0);
			bq->bufq_waiting--;
		}
		mtx_leave(&bq->bufq_mtx);
	}
}

void
bufq_done(struct bufq *bq, struct buf *bp)
{
	mtx_enter(&bq->bufq_mtx);
	bq->bufq_outstanding--;
	KASSERT(bq->bufq_outstanding >= 0);
	if (bq->bufq_stop && bq->bufq_outstanding == 0)
		wakeup(&bq->bufq_outstanding);
	if (bq->bufq_waiting && bq->bufq_outstanding < bq->bufq_low)
		wakeup_one(&bq->bufq_waiting);
	mtx_leave(&bq->bufq_mtx);
	bp->b_bq = NULL;
}

void
bufq_quiesce(void)
{
	struct bufq		*bq;

	mtx_enter(&bufqs_mtx);
	bufqs_stop = 1;
	mtx_leave(&bufqs_mtx);
	/*
	 * We can safely walk the list since it can't be modified as
	 * long as bufqs_stop is non-zero.
	 */
	SLIST_FOREACH(bq, &bufqs, bufq_entries) {
		mtx_enter(&bq->bufq_mtx);
		bq->bufq_stop = 1;
		while (bq->bufq_outstanding) {
			msleep(&bq->bufq_outstanding, &bq->bufq_mtx,
			    PRIBIO, "bqquies", 0);
		}
		mtx_leave(&bq->bufq_mtx);
	}
}

void
bufq_restart(void)
{
	struct bufq		*bq;

	mtx_enter(&bufqs_mtx);
	SLIST_FOREACH(bq, &bufqs, bufq_entries) {
		mtx_enter(&bq->bufq_mtx);
		bq->bufq_stop = 0;
		wakeup(&bq->bufq_stop);
		mtx_leave(&bq->bufq_mtx);
	}
	bufqs_stop = 0;
	wakeup(&bufqs_stop);
	mtx_leave(&bufqs_mtx);
}

/*
 * disksort implementation.
 */

void *
bufq_disksort_create(void)
{
	return (malloc(sizeof(struct buf), M_DEVBUF, M_NOWAIT | M_ZERO));
}

void
bufq_disksort_destroy(void *data)
{
	free(data, M_DEVBUF);
}

void
bufq_disksort_queue(void *data, struct buf *bp)
{
	disksort((struct buf *)data, bp);
}

struct buf *
bufq_disksort_dequeue(void *data)
{
	struct buf	*bufq = data;
	struct buf	*bp;

	bp = bufq->b_actf;
	if (bp != NULL)
		bufq->b_actf = bp->b_actf;
	if (bufq->b_actf == NULL)
		bufq->b_actb = &bufq->b_actf;

	return (bp);
}

void
bufq_disksort_requeue(void *data, struct buf *bp)
{
	struct buf	*bufq = data;

	bp->b_actf = bufq->b_actf;
	bufq->b_actf = bp;
	if (bp->b_actf == NULL)
		bufq->b_actb = &bp->b_actf;
}

int
bufq_disksort_peek(void *data)
{
	struct buf	*bufq = data;

	return (bufq->b_actf != NULL);
}

/*
 * fifo implementation
 */

void *
bufq_fifo_create(void)
{
	struct bufq_fifo_head	*head;

	head = malloc(sizeof(*head), M_DEVBUF, M_NOWAIT | M_ZERO);
	if (head == NULL)
		return (NULL);

	SIMPLEQ_INIT(head);

	return (head);
}

void
bufq_fifo_destroy(void *data)
{
	free(data, M_DEVBUF);
}

void
bufq_fifo_queue(void *data, struct buf *bp)
{
	struct bufq_fifo_head	*head = data;

	SIMPLEQ_INSERT_TAIL(head, bp, b_bufq.bufq_data_fifo.bqf_entries);
}

struct buf *
bufq_fifo_dequeue(void *data)
{
	struct bufq_fifo_head	*head = data;
	struct buf		*bp;

	bp = SIMPLEQ_FIRST(head);
	if (bp != NULL)
		SIMPLEQ_REMOVE_HEAD(head, b_bufq.bufq_data_fifo.bqf_entries);

	return (bp);
}

void
bufq_fifo_requeue(void *data, struct buf *bp)
{
	struct bufq_fifo_head	*head = data;

	SIMPLEQ_INSERT_HEAD(head, bp, b_bufq.bufq_data_fifo.bqf_entries);
}

int
bufq_fifo_peek(void *data)
{
	struct bufq_fifo_head	*head = data;

	return (SIMPLEQ_FIRST(head) != NULL);
}

/*
 * nscan implementation
 */

#define BUF_INORDER(ba, bb)			 \
    (((ba)->b_cylinder < (bb)->b_cylinder) || \
    ((ba)->b_cylinder == (bb)->b_cylinder && (ba)->b_blkno < (bb)->b_blkno))

#define dsentries b_bufq.bufq_data_nscan.bqf_entries

struct bufq_nscan_data {
	struct bufq_nscan_head sorted;
	struct bufq_nscan_head fifo;
	int dir;
	int leftoverroom;
};

void bufq_nscan_resort(struct bufq_nscan_data *data);
void bufq_simple_nscan(struct bufq_nscan_head *, struct buf *);

void
bufq_simple_nscan(struct bufq_nscan_head *head, struct buf *bp)
{
	struct buf *cur, *prev;

	prev = NULL;
	/*
	 * We look for the first slot where we would fit, then insert
	 * after the element we just passed.
	 */
	SIMPLEQ_FOREACH(cur, head, dsentries) {
		if (BUF_INORDER(bp, cur))
			break;
		prev = cur;
	}
	if (prev)
		SIMPLEQ_INSERT_AFTER(head, prev, bp, dsentries);
	else
		SIMPLEQ_INSERT_HEAD(head, bp, dsentries);

}

/*
 * Take N elements from the fifo queue and sort them
 */
void
bufq_nscan_resort(struct bufq_nscan_data *data)
{
	struct bufq_nscan_head *fifo = &data->fifo;
	struct bufq_nscan_head *sorted = &data->sorted;
	int count, segmentsize = BUFQ_NSCAN_N;
	struct buf *bp;

	for (count = 0; count < segmentsize; count++) {
		bp = SIMPLEQ_FIRST(fifo);
		if (!bp)
			break;
		SIMPLEQ_REMOVE_HEAD(fifo, dsentries);
		bufq_simple_nscan(sorted, bp);
	}
	data->leftoverroom = segmentsize - count;
}

void *
bufq_nscan_create(void)
{
	struct bufq_nscan_data *data;

	data = malloc(sizeof(*data), M_DEVBUF, M_NOWAIT | M_ZERO);
	if (!data)
		return NULL;
	SIMPLEQ_INIT(&data->sorted);
	SIMPLEQ_INIT(&data->fifo);

	return data;
}

void
bufq_nscan_destroy(void *vdata)
{
	free(vdata, M_DEVBUF);
}

void
bufq_nscan_queue(void *vdata, struct buf *bp)
{
	struct bufq_nscan_data *data = vdata;

	/*
	 * if the previous sorted segment was small, we will continue
	 * packing in bufs as long as they're in order.
	 */
	if (data->leftoverroom) {
		struct buf *next = SIMPLEQ_FIRST(&data->sorted);
		if (next && BUF_INORDER(next, bp)) {
			bufq_simple_nscan(&data->sorted, bp);
			data->leftoverroom -= 1;
			return;
		}
	}

	SIMPLEQ_INSERT_TAIL(&data->fifo, bp, dsentries);

}

struct buf *
bufq_nscan_dequeue(void *vdata)
{
	struct bufq_nscan_data *data = vdata;
	struct bufq_nscan_head *sorted = &data->sorted;
	struct buf	*bp;

	if (SIMPLEQ_FIRST(sorted) == NULL)
		bufq_nscan_resort(data);

	bp = SIMPLEQ_FIRST(sorted);
	if (bp != NULL)
		SIMPLEQ_REMOVE_HEAD(sorted, dsentries);

	return (bp);
}

void
bufq_nscan_requeue(void *vdata, struct buf *bp)
{
	struct bufq_nscan_data *data = vdata;

	SIMPLEQ_INSERT_HEAD(&data->fifo, bp, dsentries);
}

int
bufq_nscan_peek(void *vdata)
{
	struct bufq_nscan_data *data = vdata;

	return (SIMPLEQ_FIRST(&data->sorted) != NULL) ||
	    (SIMPLEQ_FIRST(&data->fifo) != NULL);
}
