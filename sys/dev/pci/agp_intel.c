/*	$OpenBSD: agp_intel.c,v 1.13 2009/05/10 14:44:42 oga Exp $	*/
/*	$NetBSD: agp_intel.c,v 1.3 2001/09/15 00:25:00 thorpej Exp $	*/

/*-
 * Copyright (c) 2000 Doug Rabson
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
 *	$FreeBSD: src/sys/pci/agp_intel.c,v 1.4 2001/07/05 21:28:47 jhb Exp $
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/proc.h>
#include <sys/agpio.h>
#include <sys/device.h>
#include <sys/agpio.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>
#include <dev/pci/agpvar.h>
#include <dev/pci/agpreg.h>

#include <machine/bus.h>

struct agp_intel_softc {
	struct device		 dev;
	struct agp_softc	*agpdev;
	struct agp_gatt 	*gatt;
	pci_chipset_tag_t	 isc_pc;
	pcitag_t		 isc_tag;
	bus_addr_t		 isc_apaddr;
	u_int			 aperture_mask;
	enum {
		CHIP_INTEL,
		CHIP_I443,
		CHIP_I840,
		CHIP_I845,
		CHIP_I850,
		CHIP_I865
	}			 chiptype; 
	bus_size_t		 initial_aperture; /* startup aperture size */
};


void	agp_intel_attach(struct device *, struct device *, void *);
int	agp_intel_probe(struct device *, void *, void *);
bus_size_t agp_intel_get_aperture(void *);
int	agp_intel_set_aperture(void *, bus_size_t);
int	agp_intel_bind_page(void *, off_t, bus_addr_t);
int	agp_intel_unbind_page(void *, off_t);
void	agp_intel_flush_tlb(void *);

struct cfattach intelagp_ca = {
	sizeof(struct agp_intel_softc), agp_intel_probe, agp_intel_attach
};

struct cfdriver intelagp_cd = {
	NULL, "intelagp", DV_DULL
};

const struct agp_methods agp_intel_methods = {
	agp_intel_get_aperture,
	agp_intel_bind_page,
	agp_intel_unbind_page,
	agp_intel_flush_tlb,
	/* default enable and memory routines */
};

int
agp_intel_probe(struct device *parent, void *match, void *aux)
{
	struct agp_attach_args	*aa = aux;
	struct pci_attach_args	*pa = aa->aa_pa;

	/* Must be a pchb */
	if (agpbus_probe(aa) == 0)
		return (0);

	switch (PCI_PRODUCT(pa->pa_id)) {
	case PCI_PRODUCT_INTEL_82443LX:
	case PCI_PRODUCT_INTEL_82443BX:
	case PCI_PRODUCT_INTEL_82440BX:
	case PCI_PRODUCT_INTEL_82440BX_AGP:
	case PCI_PRODUCT_INTEL_82815_HB:
	case PCI_PRODUCT_INTEL_82820_HB:
	case PCI_PRODUCT_INTEL_82830M_HB:
	case PCI_PRODUCT_INTEL_82840_HB:
	case PCI_PRODUCT_INTEL_82845_HB:
	case PCI_PRODUCT_INTEL_82845G_HB:
	case PCI_PRODUCT_INTEL_82850_HB:	
	case PCI_PRODUCT_INTEL_82855PM_HB:
	case PCI_PRODUCT_INTEL_82855GM_HB:
	case PCI_PRODUCT_INTEL_82860_HB:
	case PCI_PRODUCT_INTEL_82865G_HB:
	case PCI_PRODUCT_INTEL_82875P_HB:
		return (1);
	}

	return (0);
}

void
agp_intel_attach(struct device *parent, struct device *self, void *aux)
{
	struct agp_intel_softc	*isc = (struct agp_intel_softc *)self;
	struct agp_attach_args	*aa = aux;
	struct pci_attach_args	*pa = aa->aa_pa;
	struct agp_gatt		*gatt;
	pcireg_t		 reg;
	u_int32_t		 value;

	isc->isc_pc = pa->pa_pc;
	isc->isc_tag = pa->pa_tag;

	switch (PCI_PRODUCT(pa->pa_id)) {
	case PCI_PRODUCT_INTEL_82443LX:
	case PCI_PRODUCT_INTEL_82443BX:
	case PCI_PRODUCT_INTEL_82440BX:
	case PCI_PRODUCT_INTEL_82440BX_AGP:
		isc->chiptype = CHIP_I443;
		break;
	case PCI_PRODUCT_INTEL_82830M_HB:
	case PCI_PRODUCT_INTEL_82840_HB:
		isc->chiptype = CHIP_I840;
		break;
	case PCI_PRODUCT_INTEL_82845_HB:
	case PCI_PRODUCT_INTEL_82845G_HB:
	case PCI_PRODUCT_INTEL_82855PM_HB:
		isc->chiptype = CHIP_I845;
		break;
	case PCI_PRODUCT_INTEL_82850_HB:
		isc->chiptype = CHIP_I850;
		break;
	case PCI_PRODUCT_INTEL_82865G_HB:
	case PCI_PRODUCT_INTEL_82875P_HB:
		isc->chiptype = CHIP_I865;
		break;
	default:
		isc->chiptype = CHIP_INTEL;
	}

	if (pci_mapreg_info(pa->pa_pc, pa->pa_tag, AGP_APBASE,
	    PCI_MAPREG_TYPE_MEM, &isc->isc_apaddr, NULL, NULL) != 0) {
		printf(": can't get aperture info\n");
		return;
	}

	/* Determine maximum supported aperture size. */
	value = pci_conf_read(pa->pa_pc, pa->pa_tag, AGP_INTEL_APSIZE);
	pci_conf_write(pa->pa_pc, pa->pa_tag, AGP_INTEL_APSIZE, APSIZE_MASK);
	isc->aperture_mask = pci_conf_read(pa->pa_pc, pa->pa_tag,
		AGP_INTEL_APSIZE) & APSIZE_MASK;
	pci_conf_write(pa->pa_pc, pa->pa_tag, AGP_INTEL_APSIZE, value);
	isc->initial_aperture = agp_intel_get_aperture(isc);

	for (;;) {
		bus_size_t size = agp_intel_get_aperture(isc);
		gatt = agp_alloc_gatt(pa->pa_dmat, size);
		if (gatt != NULL)
			break;

		/*
		 * almost certainly error allocating contigious dma memory
		 * so reduce aperture so that the gatt size reduces.
		 */
		if (agp_intel_set_aperture(isc, size / 2)) {
			printf(": failed to set aperture\n");
			return;
		}
	}
	isc->gatt = gatt;

	/* Install the gatt. */
	pci_conf_write(pa->pa_pc, pa->pa_tag, AGP_INTEL_ATTBASE,
	    gatt->ag_physical);
	
	/* Enable the GLTB and setup the control register. */
	switch (isc->chiptype) {
	case CHIP_I443:
		pci_conf_write(isc->isc_pc, isc->isc_tag, AGP_INTEL_AGPCTRL,
		    AGPCTRL_AGPRSE | AGPCTRL_GTLB);
		break;
	default:
		pci_conf_write(isc->isc_pc, isc->isc_tag, AGP_INTEL_AGPCTRL,
		    pci_conf_read(isc->isc_pc, isc->isc_tag,
		    AGP_INTEL_AGPCTRL) | AGPCTRL_GTLB);
	}

	/* Enable things, clear errors etc. */
	switch (isc->chiptype) {
	case CHIP_I845:
	case CHIP_I865:
		reg = pci_conf_read(pa->pa_pc, pa->pa_tag, AGP_I840_MCHCFG);
		reg |= MCHCFG_AAGN;
		pci_conf_write(pa->pa_pc, pa->pa_tag, AGP_I840_MCHCFG, reg);
		break;
	case CHIP_I840:
	case CHIP_I850:
		reg = pci_conf_read(pa->pa_pc, pa->pa_tag, AGP_INTEL_AGPCMD);
		reg |= AGPCMD_AGPEN;
		pci_conf_write(pa->pa_pc, pa->pa_tag, AGP_INTEL_AGPCMD,
		    reg);
		reg = pci_conf_read(pa->pa_pc, pa->pa_tag, AGP_I840_MCHCFG);
		reg |= MCHCFG_AAGN;
		pci_conf_write(pa->pa_pc, pa->pa_tag, AGP_I840_MCHCFG,
		    reg);
		break;
	default:
		reg = pci_conf_read(pa->pa_pc, pa->pa_tag, AGP_INTEL_NBXCFG);
		reg &= ~NBXCFG_APAE;
		reg |=  NBXCFG_AAGN;
		pci_conf_write(pa->pa_pc, pa->pa_tag, AGP_INTEL_NBXCFG, reg);
	}

	/* Clear Error status */
	switch (isc->chiptype) {
	case CHIP_I840:
		pci_conf_write(pa->pa_pc, pa->pa_tag,
		    AGP_INTEL_I8XX_ERRSTS, 0xc000);
		break;
	case CHIP_I845:
	case CHIP_I850:
	case CHIP_I865:
		pci_conf_write(isc->isc_pc, isc->isc_tag,
		    AGP_INTEL_I8XX_ERRSTS, 0x00ff);
		break;

	default:
		pci_conf_write(pa->pa_pc, pa->pa_tag, AGP_INTEL_ERRSTS, 0x70);
	}
	
	isc->agpdev = (struct agp_softc *)agp_attach_bus(pa, &agp_intel_methods,
	    isc->isc_apaddr, &isc->dev);
	return;
}

#if 0
int
agp_intel_detach(struct agp_softc *sc)
{
	int error;
	pcireg_t reg;
	struct agp_intel_softc *isc = sc->sc_chipc;

	error = agp_generic_detach(sc);
	if (error)
		return (error);

	/* XXX i845/i855PM/i840/i850E */
	reg = pci_conf_read(sc->sc_pc, sc->sc_tag, AGP_INTEL_NBXCFG);
	reg &= ~(1 << 9);
	printf("%s: set NBXCFG to %x\n", __FUNCTION__, reg);
	pci_conf_write(sc->sc_pc, sc->sc_tag, AGP_INTEL_NBXCFG, reg);
	pci_conf_write(sc->sc_pc, sc->sc_tag, AGP_INTEL_ATTBASE, 0);
	AGP_SET_APERTURE(sc, isc->initial_aperture);
	agp_free_gatt(sc, isc->gatt);

	return (0);
}
#endif

bus_size_t
agp_intel_get_aperture(void *sc)
{
	struct agp_intel_softc *isc = sc;
	bus_size_t apsize;

	apsize = pci_conf_read(isc->isc_pc, isc->isc_tag,
	    AGP_INTEL_APSIZE) & isc->aperture_mask;

	/*
	 * The size is determined by the number of low bits of
	 * register APBASE which are forced to zero. The low 22 bits
	 * are always forced to zero and each zero bit in the apsize
	 * field just read forces the corresponding bit in the 27:22
	 * to be zero. We calculate the aperture size accordingly.
	 */
	return ((((apsize ^ isc->aperture_mask) << 22) | ((1 << 22) - 1)) + 1);
}

int
agp_intel_set_aperture(void *sc, bus_size_t aperture)
{
	struct agp_intel_softc *isc = sc;
	bus_size_t apsize;

	/*
	 * Reverse the magic from get_aperture.
	 */
	apsize = ((aperture - 1) >> 22) ^ isc->aperture_mask;

	/*
	 * Double check for sanity.
	 */
	if ((((apsize ^ isc->aperture_mask) << 22) |
	    ((1 << 22) - 1)) + 1 != aperture)
		return (EINVAL);

	pci_conf_write(isc->isc_pc, isc->isc_tag, AGP_INTEL_APSIZE, apsize);

	return (0);
}

int
agp_intel_bind_page(void *sc, off_t offset, bus_addr_t physical)
{
	struct agp_intel_softc *isc = sc;

	if (offset < 0 || offset >= (isc->gatt->ag_entries << AGP_PAGE_SHIFT))
		return (EINVAL);

	isc->gatt->ag_virtual[offset >> AGP_PAGE_SHIFT] = physical | 0x17;
	return (0);
}

int
agp_intel_unbind_page(void *sc, off_t offset)
{
	struct agp_intel_softc *isc = sc;

	if (offset < 0 || offset >= (isc->gatt->ag_entries << AGP_PAGE_SHIFT))
		return (EINVAL);

	isc->gatt->ag_virtual[offset >> AGP_PAGE_SHIFT] = 0;
	return (0);
}

void
agp_intel_flush_tlb(void *sc)
{
	struct agp_intel_softc *isc = sc;
	pcireg_t reg;

	switch (isc->chiptype) {
	case CHIP_I865:
	case CHIP_I850:
	case CHIP_I845:
	case CHIP_I840:
	case CHIP_I443:
		reg = pci_conf_read(isc->isc_pc, isc->isc_tag,
		    AGP_INTEL_AGPCTRL);
		reg &= ~AGPCTRL_GTLB;
		pci_conf_write(isc->isc_pc, isc->isc_tag,
		    AGP_INTEL_AGPCTRL, reg);
		pci_conf_write(isc->isc_pc, isc->isc_tag, AGP_INTEL_AGPCTRL,
		    reg | AGPCTRL_GTLB);
		break;
	default: /* XXX */
		pci_conf_write(isc->isc_pc, isc->isc_tag, AGP_INTEL_AGPCTRL,
		    0x2200);
		pci_conf_write(isc->isc_pc, isc->isc_tag, AGP_INTEL_AGPCTRL,
		    0x2280);
	}
}
