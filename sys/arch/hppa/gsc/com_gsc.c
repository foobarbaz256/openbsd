/*	$OpenBSD: com_gsc.c,v 1.19 2004/04/07 18:24:19 mickey Exp $	*/

/*
 * Copyright (c) 1998-2003 Michael Shalayeff
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR HIS RELATIVES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF MIND, USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/tty.h>

#include <machine/bus.h>
#include <machine/intr.h>
#include <machine/iomod.h>
#include <machine/autoconf.h>

#include <dev/ic/comreg.h>
#include <dev/ic/comvar.h>

#include <hppa/dev/cpudevs.h>
#include <hppa/gsc/gscbusvar.h>

struct com_gsc_regs {
	u_int8_t reset;
};

int	com_gsc_probe(struct device *, void *, void *);
void	com_gsc_attach(struct device *, struct device *, void *);

struct cfattach com_gsc_ca = {
	sizeof(struct com_softc), com_gsc_probe, com_gsc_attach
};

int
com_gsc_probe(parent, match, aux)
	struct device *parent;
	void *match, *aux;
{
	struct gsc_attach_args *ga = aux;

	if (ga->ga_type.iodc_type != HPPA_TYPE_FIO ||
	    (ga->ga_type.iodc_sv_model != HPPA_FIO_GRS232 &&
	     ga->ga_type.iodc_sv_model != HPPA_FIO_RS232 &&
	     ga->ga_type.iodc_sv_model != HPPA_FIO_GRJ16))
		return (0);

	return (1);
	/* HOZER comprobe1(ga->ga_iot, ga->ga_hpa + IOMOD_DEVOFFSET); */
}

void
com_gsc_attach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct com_softc *sc = (void *)self;
	struct gsc_attach_args *ga = aux;
	struct com_gsc_regs *regs;
	int rj16 = 0;

	sc->sc_frequency = COM_FREQ;
	sc->sc_iot = ga->ga_iot;
	sc->sc_iobase = (bus_addr_t)ga->ga_hpa;
	if (ga->ga_type.iodc_sv_model != HPPA_FIO_GRJ16)
		sc->sc_iobase += IOMOD_DEVOFFSET;
	else
		rj16++;

	if (sc->sc_iobase == CONADDR)
		sc->sc_ioh = comconsioh;
	else if (bus_space_map(sc->sc_iot, sc->sc_iobase, COM_NPORTS,
	    0, &sc->sc_ioh)) {
		printf(": cannot map io space\n");
		return;
	}

	regs = (struct com_gsc_regs *)ga->ga_hpa;
	if (!rj16 && sc->sc_iobase != CONADDR) {
		/*regs->reset = 0xd0;*/
		DELAY(1000);
	}

	com_attach_subr(sc);

	sc->sc_ih = gsc_intr_establish((struct gsc_softc *)parent,
	    ga->ga_irq, IPL_TTY, comintr, sc, sc->sc_dev.dv_xname);
}
