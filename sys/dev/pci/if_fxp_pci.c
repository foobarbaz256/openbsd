/*	$OpenBSD: if_fxp_pci.c,v 1.45 2006/01/05 21:34:35 brad Exp $	*/

/*
 * Copyright (c) 1995, David Greenman
 * All rights reserved.
 *
 * Modifications to support NetBSD:
 * Copyright (c) 1997 Jason R. Thorpe.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	Id: if_fxp.c,v 1.55 1998/08/04 08:53:12 dg Exp
 */

/*
 * Intel EtherExpress Pro/100B PCI Fast Ethernet driver
 */

#include "bpfilter.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/syslog.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <net/if_types.h>

#ifdef INET
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#endif

#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/device.h>

#include <netinet/if_ether.h>

#include <machine/cpu.h>
#include <machine/bus.h>
#include <machine/intr.h>

#include <dev/mii/miivar.h>

#include <dev/ic/fxpreg.h>
#include <dev/ic/fxpvar.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>

int fxp_pci_match(struct device *, void *, void *);
void fxp_pci_attach(struct device *, struct device *, void *);

struct cfattach fxp_pci_ca = {
	sizeof(struct fxp_softc), fxp_pci_match, fxp_pci_attach
};

const struct pci_matchid fxp_pci_devices[] = {
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_8255x },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82559 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82559ER },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82562 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82562EH_HPNA_0 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82562EH_HPNA_1 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82562EH_HPNA_2 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VE_0 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VE_1 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VE_2 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VE_3 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VE_4 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VE_5 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VE_6 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VE_7 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VE_8 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_0 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_1 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_2 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_3 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_4 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_5 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_6 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_7 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_8 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_9 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_10 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_11 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_12 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_13 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_14 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_15 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_16 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_17 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_18 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_VM_19 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100_M },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_PRO_100 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82801DB_LAN },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82801E_LAN_1 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82801E_LAN_2 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82801FB_LAN },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82801GB_LAN_2 },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82801FBM_LAN },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82801GB_LAN },
	{ PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82801GB_LAN_2 },
};

int
fxp_pci_match(parent, match, aux)
	struct device *parent;
	void *match;
	void *aux;
{
	return (pci_matchbyid((struct pci_attach_args *)aux, fxp_pci_devices,
	    sizeof(fxp_pci_devices)/sizeof(fxp_pci_devices[0])));
}

void
fxp_pci_attach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct fxp_softc *sc = (struct fxp_softc *)self;
	struct pci_attach_args *pa = aux;
	pci_chipset_tag_t pc = pa->pa_pc;
	pci_intr_handle_t ih;
	const char *chipname = NULL;
	const char *intrstr = NULL;
	bus_size_t iosize;

	if (pci_mapreg_map(pa, FXP_PCI_IOBA, PCI_MAPREG_TYPE_IO, 0,
	    &sc->sc_st, &sc->sc_sh, NULL, &iosize, 0)) {
		printf(": can't map i/o space\n");
		return;
	}
	sc->sc_dmat = pa->pa_dmat;

	sc->sc_revision = PCI_REVISION(pa->pa_class);

	/*
	 * Allocate our interrupt.
	 */
	if (pci_intr_map(pa, &ih)) {
		printf(": couldn't map interrupt\n");
		bus_space_unmap(sc->sc_st, sc->sc_sh, iosize);
		return;
	}

	intrstr = pci_intr_string(pc, ih);
	sc->sc_ih = pci_intr_establish(pc, ih, IPL_NET, fxp_intr, sc,
	    self->dv_xname);
	if (sc->sc_ih == NULL) {
		printf(": couldn't establish interrupt");
		if (intrstr != NULL)
			printf(" at %s", intrstr);
		printf("\n");
		bus_space_unmap(sc->sc_st, sc->sc_sh, iosize);
		return;
	}

	switch (PCI_PRODUCT(pa->pa_id)) {
	case PCI_PRODUCT_INTEL_8255x:
	case PCI_PRODUCT_INTEL_82559:
	case PCI_PRODUCT_INTEL_82559ER:
	{
		chipname = "i82557";
		if (sc->sc_revision >= FXP_REV_82558_A4)
			chipname = "i82558";
		if (sc->sc_revision >= FXP_REV_82559_A0)
			chipname = "i82559";
		if (sc->sc_revision >= FXP_REV_82559S_A)
			chipname = "i82559S";
		if (sc->sc_revision >= FXP_REV_82550)
			chipname = "i82550";
		if (sc->sc_revision >= FXP_REV_82551_E)
			chipname = "i82551";
		break;
	}
		break;
	default:
		chipname = "i82562";
		break;
	}

	if (chipname != NULL)
		printf(", %s", chipname);

	/*
	 * Cards for which we should WRITE TO THE EEPROM
	 * to turn off dynamic standby mode to avoid
	 * a problem where the card will fail to resume when
	 * entering the IDLE state. We use this nasty if statement
	 * and corresponding pci dev numbers directly so that people
	 * know not to add new cards to this unless you are really
	 * certain what you are doing and are not going to end up
	 * killing people's eeproms.
	 */
	if ((PCI_VENDOR(pa->pa_id) == PCI_VENDOR_INTEL) &&
	    (PCI_PRODUCT(pa->pa_id) == 0x2449 || 
	    (PCI_PRODUCT(pa->pa_id) > 0x1030 && 
	    PCI_PRODUCT(pa->pa_id) < 0x1039) || 
	    (PCI_PRODUCT(pa->pa_id) == 0x1229 &&
	    (sc->sc_revision >= 8 && sc->sc_revision <= 16))))
		sc->sc_flags |= FXPF_DISABLE_STANDBY;

	/*
	 * enable PCI Memory Write and Invalidate command
	 */
	if (sc->sc_revision >= FXP_REV_82558_A4)
		if (PCI_CACHELINE(pci_conf_read(pa->pa_pc, pa->pa_tag,
		    PCI_BHLC_REG))) {
			pci_conf_write(pa->pa_pc, pa->pa_tag,
			    PCI_COMMAND_STATUS_REG,
			    PCI_COMMAND_INVALIDATE_ENABLE |
			    pci_conf_read(pa->pa_pc, pa->pa_tag,
			    PCI_COMMAND_STATUS_REG));
			sc->sc_flags |= FXPF_MWI_ENABLE;
		}

	/* Do generic parts of attach. */
	if (fxp_attach_common(sc, intrstr)) {
		/* Failed! */
		pci_intr_disestablish(pc, sc->sc_ih);
		bus_space_unmap(sc->sc_st, sc->sc_sh, iosize);
		return;
	}
}
