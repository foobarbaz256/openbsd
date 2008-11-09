/*	$OpenBSD: agp_ali.c,v 1.6 2008/11/09 15:11:19 oga Exp $	*/
/*	$NetBSD: agp_ali.c,v 1.2 2001/09/15 00:25:00 thorpej Exp $	*/


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
 *	$FreeBSD: src/sys/pci/agp_ali.c,v 1.3 2001/07/05 21:28:46 jhb Exp $
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/lock.h>
#include <sys/agpio.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>
#include <dev/pci/vga_pcivar.h>
#include <dev/pci/agpvar.h>
#include <dev/pci/agpreg.h>

#include <machine/bus.h>

struct agp_ali_softc {
	struct device		 dev;
	struct agp_softc	*agpdev;
	struct agp_gatt		*gatt;
	pci_chipset_tag_t	 asc_pc;
	pcitag_t		 asc_tag;
	bus_size_t		 initial_aperture;
};

void	agp_ali_attach(struct device *, struct device *, void *);
int	agp_ali_probe(struct device *, void *, void *);
bus_size_t agp_ali_get_aperture(void *);
int	agp_ali_set_aperture(void *sc, bus_size_t);
int	agp_ali_bind_page(void *, off_t, bus_addr_t);
int	agp_ali_unbind_page(void *, off_t);
void	agp_ali_flush_tlb(void *);

struct cfattach aliagp_ca = {
        sizeof(struct agp_ali_softc), agp_ali_probe, agp_ali_attach
};

struct cfdriver aliagp_cd = {
	NULL, "aliagp", DV_DULL
};

const struct agp_methods agp_ali_methods = {
	agp_ali_get_aperture,
	agp_ali_bind_page,
	agp_ali_unbind_page,
	agp_ali_flush_tlb,
};

int
agp_ali_probe(struct device *parent, void *match, void *aux)
{
	struct agp_attach_args	*aa = aux;
	struct pci_attach_args	*pa = aa->aa_pa;

	/* Must be a pchb */
	if (agpbus_probe(aa) == 1 && PCI_VENDOR(pa->pa_id) == PCI_VENDOR_ALI)
		return (1);
	return (0);
}

void 
agp_ali_attach(struct device *parent, struct device *self, void *aux)
{
	struct agp_ali_softc	*asc = (struct agp_ali_softc *)self;
	struct agp_gatt		*gatt;
	struct agp_attach_args	*aa = aux;
	struct pci_attach_args	*pa = aa->aa_pa;
	pcireg_t		 reg;

	asc->asc_tag = pa->pa_tag;
	asc->asc_pc = pa->pa_pc;
	asc->initial_aperture = agp_ali_get_aperture(asc);

	for (;;) {
		bus_size_t size = agp_ali_get_aperture(asc);
		gatt = agp_alloc_gatt(pa->pa_dmat, size);
		if (gatt != NULL)
			break;
		/*
		 * almost certainly error allocating contigious dma memory
		 * so reduce aperture so that the gatt size reduces.
		 */
		if (agp_ali_set_aperture(asc, size / 2)) {
			printf("failed to set aperture\n");
			return;
		}
	}
	asc->gatt = gatt;

	/* Install the gatt. */
	reg = pci_conf_read(asc->asc_pc, asc->asc_tag, AGP_ALI_ATTBASE);
	reg = (reg & 0xff) | gatt->ag_physical;
	pci_conf_write(asc->asc_pc, asc->asc_tag, AGP_ALI_ATTBASE, reg);
	
	/* Enable the TLB. */
	reg = pci_conf_read(asc->asc_pc, asc->asc_tag, AGP_ALI_TLBCTRL);
	reg = (reg & ~0xff) | 0x10;
	pci_conf_write(asc->asc_pc, asc->asc_tag, AGP_ALI_TLBCTRL, reg);

	asc->agpdev = (struct agp_softc *)agp_attach_bus(pa, &agp_ali_methods,
	    AGP_APBASE, PCI_MAPREG_TYPE_MEM, &asc->dev);
	return;
}

#if 0
int
agp_ali_detach(struct agp_softc *sc)
{
	int error;
	pcireg_t reg;
	struct agp_ali_softc *asc = sc->sc_chipc;

	error = agp_generic_detach(sc);
	if (error)
		return (error);

	/* Disable the TLB.. */
	reg = pci_conf_read(sc->sc_pc, sc->sc_pcitag, AGP_ALI_TLBCTRL);
	reg &= ~0xff;
	reg |= 0x90;
	pci_conf_write(sc->sc_pc, sc->sc_pcitag, AGP_ALI_TLBCTRL, reg);

	/* Put the aperture back the way it started. */
	AGP_SET_APERTURE(sc, asc->initial_aperture);
	reg = pci_conf_read(sc->sc_pc, sc->sc_pcitag, AGP_ALI_ATTBASE);
	reg &= 0xff;
	pci_conf_write(sc->sc_pc, sc->sc_pcitag, AGP_ALI_ATTBASE, reg);

	agp_free_gatt(sc, asc->gatt);
	return (0);
}
#endif

#define M 1024*1024

static const u_int32_t agp_ali_table[] = {
	0,			/* 0 - invalid */
	1,			/* 1 - invalid */
	2,			/* 2 - invalid */
	4*M,			/* 3 - invalid */
	8*M,			/* 4 - invalid */
	0,			/* 5 - invalid */
	16*M,			/* 6 - invalid */
	32*M,			/* 7 - invalid */
	64*M,			/* 8 - invalid */
	128*M,			/* 9 - invalid */
	256*M,			/* 10 - invalid */
};
#define agp_ali_table_size (sizeof(agp_ali_table) / sizeof(agp_ali_table[0]))

bus_size_t
agp_ali_get_aperture(void *sc)
{
	struct agp_ali_softc	*asc = sc;
	int			 i;

	/*
	 * The aperture size is derived from the low bits of attbase.
	 * I'm not sure this is correct..
	 */
	i = (int)pci_conf_read(asc->asc_pc, asc->asc_tag,
	    AGP_ALI_ATTBASE) & 0xff;
	if (i >= agp_ali_table_size)
		return (0);
	return (agp_ali_table[i]);
}

int
agp_ali_set_aperture(void *sc, bus_size_t aperture)
{
	struct agp_ali_softc	*asc = sc;
	int			 i;
	pcireg_t		 reg;

	for (i = 0; i < agp_ali_table_size; i++)
		if (agp_ali_table[i] == aperture)
			break;
	if (i == agp_ali_table_size)
		return (EINVAL);

	reg = pci_conf_read(asc->asc_pc, asc->asc_tag, AGP_ALI_ATTBASE);
	reg &= ~0xff;
	reg |= i;
	pci_conf_write(asc->asc_pc, asc->asc_tag, AGP_ALI_ATTBASE, reg);
	return (0);
}

int
agp_ali_bind_page(void *sc, off_t offset, bus_addr_t physical)
{
	struct agp_ali_softc *asc = sc;

	if (offset < 0 || offset >= (asc->gatt->ag_entries << AGP_PAGE_SHIFT))
		return (EINVAL);

	asc->gatt->ag_virtual[offset >> AGP_PAGE_SHIFT] = physical;
	return (0);
}

int
agp_ali_unbind_page(void *sc, off_t offset)
{
	struct agp_ali_softc *asc = sc;

	if (offset < 0 || offset >= (asc->gatt->ag_entries << AGP_PAGE_SHIFT))
		return (EINVAL);

	asc->gatt->ag_virtual[offset >> AGP_PAGE_SHIFT] = 0;
	return (0);
}

void
agp_ali_flush_tlb(void *sc)
{
	struct agp_ali_softc	*asc = sc;
	pcireg_t		reg;

	reg = pci_conf_read(asc->asc_pc, asc->asc_tag, AGP_ALI_TLBCTRL);
	reg &= ~0xff;
	reg |= 0x90;
	pci_conf_write(asc->asc_pc, asc->asc_tag, AGP_ALI_TLBCTRL, reg);
	reg &= ~0xff;
	reg |= 0x10;
	pci_conf_write(asc->asc_pc, asc->asc_tag, AGP_ALI_TLBCTRL, reg);
}

