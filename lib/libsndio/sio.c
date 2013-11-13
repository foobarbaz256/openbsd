/*	$OpenBSD: sio.c,v 1.15 2013/11/13 22:38:22 ratchov Exp $	*/
/*
 * Copyright (c) 2008 Alexandre Ratchov <alex@caoua.org>
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
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "debug.h"
#include "sio_priv.h"

#define SIO_PAR_MAGIC	0x83b905a4

void
sio_initpar(struct sio_par *par)
{
	memset(par, 0xff, sizeof(struct sio_par));
	par->__magic = SIO_PAR_MAGIC;
}

struct sio_hdl *
sio_open(const char *str, unsigned int mode, int nbio)
{
	static char devany[] = SIO_DEVANY;
	struct sio_hdl *hdl;
	const char *p;

#ifdef DEBUG
	_sndio_debug_init();
#endif
	if ((mode & (SIO_PLAY | SIO_REC)) == 0)
		return NULL;
	if (str == NULL) /* backward compat */
		str = devany;
	if (strcmp(str, devany) == 0 && !issetugid()) {
		str = getenv("AUDIODEVICE");
		if (str == NULL)
			str = devany;
	}
	if (strcmp(str, devany) == 0) {
		hdl = _sio_aucat_open("/0", mode, nbio);
		if (hdl != NULL)
			return hdl;
		return _sio_sun_open("/0", mode, nbio);
	}
	if ((p = _sndio_parsetype(str, "snd")) != NULL ||
	    (p = _sndio_parsetype(str, "aucat")) != NULL)
		return _sio_aucat_open(p, mode, nbio);
	if ((p = _sndio_parsetype(str, "rsnd")) != NULL ||
	    (p = _sndio_parsetype(str, "sun")) != NULL) {
		return _sio_sun_open(p, mode, nbio);
	}
	DPRINTF("sio_open: %s: unknown device type\n", str);
	return NULL;
}

void
_sio_create(struct sio_hdl *hdl, struct sio_ops *ops,
    unsigned int mode, int nbio)
{
	hdl->ops = ops;
	hdl->mode = mode;
	hdl->nbio = nbio;
	hdl->started = 0;
	hdl->eof = 0;
	hdl->move_cb = NULL;
	hdl->vol_cb = NULL;
}

void
sio_close(struct sio_hdl *hdl)
{
	hdl->ops->close(hdl);
}

int
sio_start(struct sio_hdl *hdl)
{
#ifdef DEBUG
	struct timespec ts;
#endif

	if (hdl->eof) {
		DPRINTF("sio_start: eof\n");
		return 0;
	}
	if (hdl->started) {
		DPRINTF("sio_start: already started\n");
		hdl->eof = 1;
		return 0;
	}
#ifdef DEBUG
	if (!sio_getpar(hdl, &hdl->par))
		return 0;
	hdl->pollcnt = hdl->wcnt = hdl->rcnt = hdl->cpos = 0;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	hdl->start_nsec = 1000000000LL * ts.tv_sec + ts.tv_nsec;
#endif
	hdl->rdrop = hdl->wsil = 0;
	if (!hdl->ops->start(hdl))
		return 0;
	hdl->started = 1;
	return 1;
}

int
sio_stop(struct sio_hdl *hdl)
{
	if (hdl->eof) {
		DPRINTF("sio_stop: eof\n");
		return 0;
	}
	if (!hdl->started) {
		DPRINTF("sio_stop: not started\n");
		hdl->eof = 1;
		return 0;
	}
	if (!hdl->ops->stop(hdl))
		return 0;
#ifdef DEBUG
	DPRINTF("libsndio: polls: %llu, written = %llu, read: %llu\n",
	    hdl->pollcnt, hdl->wcnt, hdl->rcnt);
#endif
	hdl->started = 0;
	return 1;
}

int
sio_setpar(struct sio_hdl *hdl, struct sio_par *par)
{
	if (hdl->eof) {
		DPRINTF("sio_setpar: eof\n");
		return 0;
	}
	if (par->__magic != SIO_PAR_MAGIC) {
		DPRINTF("sio_setpar: use of uninitialized sio_par structure\n");
		hdl->eof = 1;
		return 0;
	}
	if (hdl->started) {
		DPRINTF("sio_setpar: already started\n");
		hdl->eof = 1;
		return 0;
	}
	if (par->bufsz != ~0U) {
		DPRINTF("sio_setpar: setting bufsz is deprecated\n");
		par->appbufsz = par->bufsz;
		par->bufsz = ~0U;
	}
	if (par->rate != ~0U && par->appbufsz == ~0U)
		par->appbufsz = par->rate * 200 / 1000;
	return hdl->ops->setpar(hdl, par);
}

int
sio_getpar(struct sio_hdl *hdl, struct sio_par *par)
{
	if (hdl->eof) {
		DPRINTF("sio_getpar: eof\n");
		return 0;
	}
	if (hdl->started) {
		DPRINTF("sio_getpar: already started\n");
		hdl->eof = 1;
		return 0;
	}
	if (!hdl->ops->getpar(hdl, par)) {
		par->__magic = 0;
		return 0;
	}
	par->__magic = 0;
	return 1;
}

int
sio_getcap(struct sio_hdl *hdl, struct sio_cap *cap)
{
	if (hdl->eof) {
		DPRINTF("sio_getcap: eof\n");
		return 0;
	}
	if (hdl->started) {
		DPRINTF("sio_getcap: already started\n");
		hdl->eof = 1;
		return 0;
	}
	return hdl->ops->getcap(hdl, cap);
}

static int
sio_psleep(struct sio_hdl *hdl, int event)
{
	struct pollfd pfd[SIO_MAXNFDS];
	int revents;
	int nfds;

	nfds = sio_nfds(hdl);
	if (nfds > SIO_MAXNFDS) {
		DPRINTF("sio_psleep: %d: too many descriptors\n", nfds);
		hdl->eof = 1;
		return 0;
	}
	for (;;) {
		nfds = sio_pollfd(hdl, pfd, event);
		while (poll(pfd, nfds, -1) < 0) {
			if (errno == EINTR)
				continue;
			DPERROR("sio_psleep: poll");
			hdl->eof = 1;
			return 0;
		}
		revents = sio_revents(hdl, pfd);
		if (revents & POLLHUP) {
			DPRINTF("sio_psleep: hang-up\n");
			return 0;
		}
		if (revents & event)
			break;
	}
	return 1;
}

static int
sio_rdrop(struct sio_hdl *hdl)
{
#define DROP_NMAX 0x1000
	static char dummy[DROP_NMAX];
	ssize_t n, todo;

	while (hdl->rdrop > 0) {
		todo = hdl->rdrop;
		if (todo > DROP_NMAX)
			todo = DROP_NMAX;
		n = hdl->ops->read(hdl, dummy, todo);
		if (n == 0)
			return 0;
		hdl->rdrop -= n;
		DPRINTF("sio_rdrop: dropped %zu bytes\n", n);
	}
	return 1;
}

static int
sio_wsil(struct sio_hdl *hdl)
{
#define ZERO_NMAX 0x1000
	static char zero[ZERO_NMAX];
	ssize_t n, todo;

	while (hdl->wsil > 0) {
		todo = hdl->wsil;
		if (todo > ZERO_NMAX)
			todo = ZERO_NMAX;
		n = hdl->ops->write(hdl, zero, todo);
		if (n == 0)
			return 0;
		hdl->wsil -= n;
		DPRINTF("sio_wsil: inserted %zu bytes\n", n);
	}
	return 1;
}

size_t
sio_read(struct sio_hdl *hdl, void *buf, size_t len)
{
	unsigned int n;
	char *data = buf;
	size_t todo = len;

	if (hdl->eof) {
		DPRINTF("sio_read: eof\n");
		return 0;
	}
	if (!hdl->started || !(hdl->mode & SIO_REC)) {
		DPRINTF("sio_read: recording not started\n");
		hdl->eof = 1;
		return 0;
	}
	if (todo == 0) {
		DPRINTF("sio_read: zero length read ignored\n");
		return 0;
	}
	if (!sio_rdrop(hdl))
		return 0;
	while (todo > 0) {
		n = hdl->ops->read(hdl, data, todo);
		if (n == 0) {
			if (hdl->nbio || hdl->eof || todo < len)
				break;
			if (!sio_psleep(hdl, POLLIN))
				break;
			continue;
		}
		data += n;
		todo -= n;
#ifdef DEBUG
		hdl->rcnt += n;
#endif
	}
	return len - todo;
}

size_t
sio_write(struct sio_hdl *hdl, const void *buf, size_t len)
{
	unsigned int n;
	const unsigned char *data = buf;
	size_t todo = len;

	if (hdl->eof) {
		DPRINTF("sio_write: eof\n");
		return 0;
	}
	if (!hdl->started || !(hdl->mode & SIO_PLAY)) {
		DPRINTF("sio_write: playback not started\n");
		hdl->eof = 1;
		return 0;
	}
	if (todo == 0) {
		DPRINTF("sio_write: zero length write ignored\n");
		return 0;
	}
	if (!sio_wsil(hdl))
		return 0;
	while (todo > 0) {
		n = hdl->ops->write(hdl, data, todo);
		if (n == 0) {
			if (hdl->nbio || hdl->eof)
				break;
			if (!sio_psleep(hdl, POLLOUT))
				break;
			continue;
		}
		data += n;
		todo -= n;
#ifdef DEBUG
		hdl->wcnt += n;
#endif
	}
	return len - todo;
}

int
sio_nfds(struct sio_hdl *hdl)
{
	return hdl->ops->nfds(hdl);
}

int
sio_pollfd(struct sio_hdl *hdl, struct pollfd *pfd, int events)
{
	if (hdl->eof)
		return 0;
	if (!hdl->started)
		events = 0;
	return hdl->ops->pollfd(hdl, pfd, events);
}

int
sio_revents(struct sio_hdl *hdl, struct pollfd *pfd)
{
	int revents;
#ifdef DEBUG
	struct timespec ts0, ts1;

	if (_sndio_debug >= 2)
		clock_gettime(CLOCK_MONOTONIC, &ts0);
#endif
	if (hdl->eof)
		return POLLHUP;
#ifdef DEBUG
	hdl->pollcnt++;
#endif
	revents = hdl->ops->revents(hdl, pfd);
	if (!hdl->started)
		return revents & POLLHUP;
#ifdef DEBUG
	if (_sndio_debug >= 3) {
		clock_gettime(CLOCK_MONOTONIC, &ts1);
		DPRINTF("%09lld: sio_revents: revents = 0x%x, took %lldns\n",
		    1000000000LL * ts0.tv_sec +
		    ts0.tv_nsec - hdl->start_nsec,
		    revents,
		    1000000000LL * (ts1.tv_sec - ts0.tv_sec) +
		    ts1.tv_nsec - ts0.tv_nsec);
	}
#endif
	if ((hdl->mode & SIO_PLAY) && !sio_wsil(hdl))
		revents &= ~POLLOUT;
	if ((hdl->mode & SIO_REC) && !sio_rdrop(hdl))
		revents &= ~POLLIN;
	return revents;
}

int
sio_eof(struct sio_hdl *hdl)
{
	return hdl->eof;
}

void
sio_onmove(struct sio_hdl *hdl, void (*cb)(void *, int), void *addr)
{
	if (hdl->started) {
		DPRINTF("sio_onmove: already started\n");
		hdl->eof = 1;
		return;
	}
	hdl->move_cb = cb;
	hdl->move_addr = addr;
}

#ifdef DEBUG
void
_sio_printpos(struct sio_hdl *hdl)
{	
	struct timespec ts;
	long long rpos, rdiff;
	long long cpos, cdiff;
	long long wpos, wdiff;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	rpos = (hdl->mode & SIO_REC) ? 
	    hdl->rcnt / (hdl->par.bps * hdl->par.rchan) : 0;
	wpos = (hdl->mode & SIO_PLAY) ?
	    hdl->wcnt / (hdl->par.bps * hdl->par.pchan) : 0;

	cdiff = hdl->cpos % hdl->par.round;
	cpos  = hdl->cpos / hdl->par.round;
	if (cdiff > hdl->par.round / 2) {
		cpos++;
		cdiff = cdiff - hdl->par.round;
	}
	rdiff = rpos % hdl->par.round;
	rpos  = rpos / hdl->par.round;
	if (rdiff > hdl->par.round / 2) {
		rpos++;
		rdiff = rdiff - hdl->par.round;
	}
	wdiff = wpos % hdl->par.round;
	wpos  = wpos / hdl->par.round;
	if (wdiff > hdl->par.round / 2) {
		wpos++;
		wdiff = wdiff - hdl->par.round;
	}
	DPRINTF("%011lld: "
	    "clk %+5lld%+5lld, wr %+5lld%+5lld rd: %+5lld%+5lld\n",
	    1000000000LL * ts.tv_sec + ts.tv_nsec - hdl->start_nsec,
	    cpos, cdiff, wpos, wdiff, rpos, rdiff);
}
#endif

void
_sio_onmove_cb(struct sio_hdl *hdl, int delta)
{
#ifdef DEBUG
	hdl->cpos += delta;
	if (_sndio_debug >= 2)
		_sio_printpos(hdl);
#endif
	if (hdl->move_cb)
		hdl->move_cb(hdl->move_addr, delta);
}

int
sio_setvol(struct sio_hdl *hdl, unsigned int ctl)
{
	if (hdl->eof)
		return 0;
	if (!hdl->ops->setvol)
		return 1;
	if (!hdl->ops->setvol(hdl, ctl))
		return 0;
	hdl->ops->getvol(hdl);
	return 1;
}

int
sio_onvol(struct sio_hdl *hdl, void (*cb)(void *, unsigned int), void *addr)
{
	if (hdl->started) {
		DPRINTF("sio_onvol: already started\n");
		hdl->eof = 1;
		return 0;
	}
	if (!hdl->ops->setvol)
		return 0;
	hdl->vol_cb = cb;
	hdl->vol_addr = addr;
	hdl->ops->getvol(hdl);
	return 1;
}

void
_sio_onvol_cb(struct sio_hdl *hdl, unsigned int ctl)
{
	if (hdl->vol_cb)
		hdl->vol_cb(hdl->vol_addr, ctl);
}
