/*	$OpenBSD: if_fxp_cardbus.c,v 1.3 2001/08/09 21:12:51 jason Exp $ */
/*	$NetBSD: if_fxp_cardbus.c,v 1.12 2000/05/08 18:23:36 thorpej Exp $	*/

/*
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Johan Danielsson.
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
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * CardBus front-end for the Intel i82557 family of Ethernet chips.
 */

#include "bpfilter.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/device.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/if_media.h>

#include <machine/endian.h>

#if NBPFILTER > 0
#include <net/bpf.h>
#endif

#ifdef INET
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#endif

#include <machine/bus.h>
#include <machine/intr.h>

#include <dev/mii/miivar.h>

#include <dev/ic/fxpreg.h>
#include <dev/ic/fxpvar.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>

#include <dev/cardbus/cardbusvar.h>
#include <dev/cardbus/cardbusdevs.h>

int fxp_cardbus_match __P((struct device *, void *, void *));
void fxp_cardbus_attach __P((struct device *, struct device *, void *));
int fxp_cardbus_detach __P((struct device * self, int flags));
void fxp_cardbus_setup __P((struct fxp_softc * sc));

struct fxp_cardbus_softc {
	struct fxp_softc sc;
	cardbus_devfunc_t ct;
	pcireg_t base0_reg;
	pcireg_t base1_reg;
	bus_size_t size;
};

struct cfattach fxp_cardbus_ca = {
	sizeof(struct fxp_cardbus_softc), fxp_cardbus_match, fxp_cardbus_attach,
	    fxp_cardbus_detach
};

#ifdef CBB_DEBUG
#define DPRINTF(X) printf X
#else
#define DPRINTF(X)
#endif

int
fxp_cardbus_match(parent, match, aux)
	struct device *parent;
	void *match;
	void *aux;
{
	struct cardbus_attach_args *ca = aux;

	if (CARDBUS_VENDOR(ca->ca_id) == CARDBUS_VENDOR_INTEL &&
	    CARDBUS_PRODUCT(ca->ca_id) == CARDBUS_PRODUCT_INTEL_82557)
		return (1);

	return (0);
}

void
fxp_cardbus_attach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	static const char thisfunc[] = "fxp_cardbus_attach";

	char intrstr[16];
	struct fxp_softc *sc = (struct fxp_softc *) self;
	struct fxp_cardbus_softc *csc = (struct fxp_cardbus_softc *) self;
	struct cardbus_attach_args *ca = aux;
	struct cardbus_softc *psc =
	    (struct cardbus_softc *)sc->sc_dev.dv_parent;
	cardbus_chipset_tag_t cc = psc->sc_cc;
	cardbus_function_tag_t cf = psc->sc_cf;
	bus_space_tag_t iot, memt;
	bus_space_handle_t ioh, memh;
	u_int8_t enaddr[6];

	bus_addr_t adr;
	bus_size_t size;

	csc->ct = ca->ca_ct;

	/*
         * Map control/status registers.
         */
	if (Cardbus_mapreg_map(csc->ct, CARDBUS_BASE1_REG,
	    PCI_MAPREG_TYPE_IO, 0, &iot, &ioh, &adr, &size) == 0) {
		csc->base1_reg = adr | 1;
		sc->sc_st = iot;
		sc->sc_sh = ioh;
		csc->size = size;
	} else if (Cardbus_mapreg_map(csc->ct, CARDBUS_BASE0_REG,
	    PCI_MAPREG_TYPE_MEM | PCI_MAPREG_MEM_TYPE_32BIT,
	    0, &memt, &memh, &adr, &size) == 0) {
		csc->base0_reg = adr;
		sc->sc_st = memt;
		sc->sc_sh = memh;
		csc->size = size;
	} else
		panic("%s: failed to allocate mem and io space", thisfunc);

	if (ca->ca_cis.cis1_info[0] && ca->ca_cis.cis1_info[1])
		printf(": %s %s", ca->ca_cis.cis1_info[0],
		    ca->ca_cis.cis1_info[1]);
	else
		printf("\n");

	sc->sc_dmat = ca->ca_dmat;
#if 0
	sc->sc_enable = fxp_cardbus_enable;
	sc->sc_disable = fxp_cardbus_disable;
	sc->sc_enabled = 0;
#endif

	Cardbus_function_enable(csc->ct);

	fxp_cardbus_setup(sc);

	/* Map and establish the interrupt. */
	sc->sc_ih = cardbus_intr_establish(cc, cf, psc->sc_intrline, IPL_NET,
	    fxp_intr, sc);
	if (NULL == sc->sc_ih) {
		printf(": couldn't establish interrupt");
		printf("at %d\n", ca->ca_intrline);
		return;
	}
	snprintf(intrstr, sizeof(intrstr), "irq %d", ca->ca_intrline);

	fxp_attach_common(sc, enaddr, intrstr);
}

void
fxp_cardbus_setup(struct fxp_softc * sc)
{
	struct fxp_cardbus_softc *csc = (struct fxp_cardbus_softc *) sc;
	struct cardbus_softc *psc =
	    (struct cardbus_softc *) sc->sc_dev.dv_parent;
	cardbus_chipset_tag_t cc = psc->sc_cc;
	cardbus_function_tag_t cf = psc->sc_cf;
	pcireg_t command;

	cardbustag_t tag = cardbus_make_tag(cc, cf, csc->ct->ct_bus,
	    csc->ct->ct_dev, csc->ct->ct_func);

	command = Cardbus_conf_read(csc->ct, tag, CARDBUS_COMMAND_STATUS_REG);
	if (csc->base0_reg) {
		Cardbus_conf_write(csc->ct, tag,
		    CARDBUS_BASE0_REG, csc->base0_reg);
		(cf->cardbus_ctrl) (cc, CARDBUS_MEM_ENABLE);
		command |= CARDBUS_COMMAND_MEM_ENABLE |
		    CARDBUS_COMMAND_MASTER_ENABLE;
	} else if (csc->base1_reg) {
		Cardbus_conf_write(csc->ct, tag,
		    CARDBUS_BASE1_REG, csc->base1_reg);
		(cf->cardbus_ctrl) (cc, CARDBUS_IO_ENABLE);
		command |= (CARDBUS_COMMAND_IO_ENABLE |
		    CARDBUS_COMMAND_MASTER_ENABLE);
	}

	(cf->cardbus_ctrl) (cc, CARDBUS_BM_ENABLE);

	/* enable the card */
	Cardbus_conf_write(csc->ct, tag, CARDBUS_COMMAND_STATUS_REG, command);
}

int
fxp_cardbus_detach(self, flags)
	struct device *self;
	int flags;
{
	struct fxp_softc *sc = (struct fxp_softc *) self;
	struct fxp_cardbus_softc *csc = (struct fxp_cardbus_softc *) self;
	struct cardbus_devfunc *ct = csc->ct;
	int rv, reg;

#ifdef DIAGNOSTIC
	if (ct == NULL)
		panic("%s: data structure lacks\n", sc->sc_dev.dv_xname);
#endif

	rv = fxp_detach(sc);
	if (rv == 0) {
		/*
		 * Unhook the interrupt handler.
		 */
		cardbus_intr_disestablish(ct->ct_cc, ct->ct_cf, sc->sc_ih);

		/*
		 * release bus space and close window
		 */
		if (csc->base0_reg)
			reg = CARDBUS_BASE0_REG;
		else
			reg = CARDBUS_BASE1_REG;
		Cardbus_mapreg_unmap(ct, reg, sc->sc_st, sc->sc_sh, csc->size);
	}
	return (rv);
}
