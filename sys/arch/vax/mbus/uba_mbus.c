/*	$OpenBSD: uba_mbus.c,v 1.4 2015/02/01 13:18:32 miod Exp $	*/

/*
 * Copyright (c) 2008 Miodrag Vallat.
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
/*
 * Copyright (c) 1996 Jonathan Stone.
 * Copyright (c) 1994, 1996 Ludd, University of Lule}, Sweden.
 * Copyright (c) 1982, 1986 The Regents of the University of California.
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
 *	@(#)uba.c	7.10 (Berkeley) 12/16/90
 *	@(#)autoconf.c	7.20 (Berkeley) 5/9/91
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>

#include <machine/bus.h>
#include <machine/cpu.h>
#include <machine/mtpr.h>
#include <machine/sgmap.h>

#include <vax/mbus/mbusreg.h>
#include <vax/mbus/mbusvar.h>

#include <arch/vax/qbus/ubavar.h>
#include <arch/vax/uba/uba_common.h>
#include <arch/vax/uba/ubareg.h>

#define	QBASIZE	(8192 * VAX_NBPG)

void	uba_mbus_attach(struct device *, struct device *, void *);
int	uba_mbus_match(struct device *, void *, void *);

const struct cfattach uba_mbus_ca = {
	sizeof(struct uba_vsoftc), uba_mbus_match, uba_mbus_attach
};

void	uba_mbus_beforescan(struct uba_softc*);
void	uba_mbus_init(struct uba_softc*);

extern	struct vax_bus_space vax_mem_bus_space;

int
uba_mbus_match(struct device *parent, void *vcf, void *aux)
{
	struct mbus_attach_args *maa = (struct mbus_attach_args *)aux;

	/*
	 * There can only be one QBus adapter (because it uses range-mapped
	 * MBus I/O), and it has to be in slot zero for connectivity reasons.
	 */
	if (maa->maa_mid != 0)
		return 0;

	if (maa->maa_class == CLASS_BA && maa->maa_interface == INTERFACE_FBIC)
		return 1;

	return 0;
}

void
uba_mbus_attach(struct device *parent, struct device *self, void *aux)
{
	struct mbus_attach_args *maa = (struct mbus_attach_args *)aux;
	struct uba_vsoftc *sc = (void *)self;
	paddr_t modaddr;
	vaddr_t fbic;

	printf(": Q22\n");

	/*
	 * Configure M-Bus I/O range.
	 *
	 * This will map the sgmap at 2008xxxx (QBAMAP), and the doorbell
	 * registers at 2000xxxx (QIOPAGE).
	 */
	modaddr = MBUS_SLOT_BASE(maa->maa_mid);
	fbic = vax_map_physmem(modaddr + FBIC_BASE, 1);
	if (fbic == 0) {
		printf("%s: can't setup M-bus range register\n",
		    self->dv_xname);
		return;
	}
	*(uint32_t *)(fbic + FBIC_RANGE) =
	    (HOST_TO_MBUS(QBAMAP & RANGE_MATCH)) | RANGE_ENABLE |
	    ((QBAMAP ^ QIOPAGE) >> 16);
	vax_unmap_physmem(fbic, 1);

	/*
	 * There is nothing special to do to enable interrupt routing;
	 * the CQBIC will route Q-bus interrupts to the C-bus, and
	 * mbus(4) has already configured our FBIC interrupt registers
	 * to route C-bus interrupts to the M-bus (whether they are
	 * generated by the FBIC or by the Q-bus), which will make them
	 * visible to the processor.
	 *
	 * Note that we do not enable the boards' FBIC memory error
	 * interrupt yet.
	 */

	/*
	 * Fill in bus specific data.
	 */
	sc->uv_sc.uh_beforescan = uba_mbus_beforescan;
	sc->uv_sc.uh_ubainit = uba_mbus_init;
	sc->uv_sc.uh_iot = &vax_mem_bus_space;
	sc->uv_sc.uh_dmat = &sc->uv_dmat;

	/*
	 * Fill in variables used by the sgmap system.
	 */
	sc->uv_size = QBASIZE;	/* Size in bytes of Qbus space */
	sc->uv_addr = QBAMAP;	/* Physical address of map registers */

	uba_dma_init(sc);
	uba_attach(&sc->uv_sc, QIOPAGE);
}

/*
 * Called when the CQBIC is set up; to enable DMA access from
 * Q-bus devices to main memory.
 */
void
uba_mbus_beforescan(sc)
	struct uba_softc *sc;
{
	/*
	 * Writing to the doorbell registers causes a bus error and
	 * a machine check.  Don't do this for now, the built-in
	 * tape drive still works without this.
	bus_space_write_2(sc->uh_iot, sc->uh_ioh, QIPCR, Q_LMEAE);
	 */
}

void
uba_mbus_init(sc)
	struct uba_softc *sc;
{
	DELAY(500000);
	uba_mbus_beforescan(sc);
}
