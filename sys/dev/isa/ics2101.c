/* $NetBSD: ics2101.c,v 1.1 1995/07/19 19:58:49 brezak Exp $ */
/*-
 * Copyright (c) 1995 John T. Kohl.  All Rights Reserved.
 * Copyright (c) 1994 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ken Hornstein.
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
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
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
 *	$Id: ics2101.c,v 1.1.1.1 1995/10/18 08:52:34 deraadt Exp $
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/syslog.h>
#include <sys/device.h>
#include <sys/proc.h>
#include <sys/buf.h>

#include <machine/cpu.h>
#include <machine/pio.h>

#include <sys/audioio.h>
#include <dev/audio_if.h>

#include <dev/isa/isavar.h>
#include <dev/isa/isadmavar.h>

#include <dev/ic/ics2101reg.h>
#include <dev/isa/ics2101var.h>


#define ICS_VALUE	0x01
#define ICS_MUTE	0x02
#define ICS_MUTE_MUTED	0x04

/* convert from [AUDIO_MIN_GAIN,AUDIO_MAX_GAIN] (0,255) to
   [ICSMIX_MAX_ATTN,ICSMIX_MIN_ATTN] (0,127) */

#define cvt_value(val) ((val) >> 1)

/*
 * Program one channel of the ICS mixer
 */


static void
ics2101_mix_doit(sc, chan, side, value, flags)
	struct ics2101_softc *sc;
	unsigned int chan, side, value, flags;
{
	unsigned char flip_left[6] = {0x01, 0x01, 0x01, 0x02, 0x01, 0x02};
	unsigned char flip_right[6] = {0x02, 0x02, 0x02, 0x01, 0x02, 0x01};
	register unsigned char ctrl_addr;
	register unsigned char attn_addr;
	register unsigned char normal;
	int s;

	if (chan < ICSMIX_CHAN_0 || chan > ICSMIX_CHAN_5)
		return;
	if (side != ICSMIX_LEFT && side != ICSMIX_RIGHT)
		return;

	if (flags & ICS_MUTE) {
		value = cvt_value(sc->sc_setting[chan][side]);
		sc->sc_mute[chan][side] = flags & ICS_MUTE_MUTED;
	} else if (flags & ICS_VALUE) {
		sc->sc_setting[chan][side] = value;
		value = cvt_value(value);
		if (value > ICSMIX_MIN_ATTN)
			value = ICSMIX_MIN_ATTN;
	} else
		return;

	ctrl_addr = chan << 3;
	attn_addr = chan << 3;

	if (side == ICSMIX_LEFT) {
		ctrl_addr |= ICSMIX_CTRL_LEFT;
		attn_addr |= ICSMIX_ATTN_LEFT;
		if (sc->sc_mute[chan][side])
			normal = 0x0;
		else if (sc->sc_flags & ICS_FLIP)
			normal = flip_left[chan];
		else
			normal = 0x01;
	} else {
		ctrl_addr |= ICSMIX_CTRL_RIGHT;
		attn_addr |= ICSMIX_ATTN_RIGHT;
		if (sc->sc_mute[chan][side])
			normal = 0x0;
		else if (sc->sc_flags & ICS_FLIP)
			normal = flip_right[chan];
		else
			normal = 0x02;
	}

	s = splaudio();

	outb(sc->sc_selio, ctrl_addr);
	outb(sc->sc_dataio, normal);

	outb(sc->sc_selio, attn_addr);
	outb(sc->sc_dataio, (unsigned char) value);

	splx(s);
}

void
ics2101_mix_mute(sc, chan, side, domute)
	struct ics2101_softc *sc;
	unsigned int chan, side, domute;
{
    ics2101_mix_doit(sc, chan, side, 0,
		     domute ? ICS_MUTE|ICS_MUTE_MUTED : ICS_MUTE);
}

void
ics2101_mix_attenuate(sc, chan, side, value)
	struct ics2101_softc *sc;
	unsigned int chan, side, value;
{
    ics2101_mix_doit(sc, chan, side, value, ICS_VALUE);
}
