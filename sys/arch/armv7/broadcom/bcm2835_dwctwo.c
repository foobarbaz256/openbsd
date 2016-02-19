/*	$OpenBSD: octdwctwo.c,v 1.4 2015/02/14 06:46:03 uebayasi Exp $	*/

/*
 * Copyright (c) 2015 Masao Uebayashi <uebayasi@tombiinc.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
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
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/pool.h>
#include <sys/kthread.h>

#include <machine/intr.h>
#include <machine/bus.h>
#include <machine/fdt.h>

#include <dev/ofw/openfirm.h>

#include <dev/usb/usb.h>
#include <dev/usb/usbdi.h>
#include <dev/usb/usbdivar.h>
#include <dev/usb/usb_mem.h>
#include <dev/usb/usb_quirks.h>

#include <dev/usb/dwc2/dwc2var.h>
#include <dev/usb/dwc2/dwc2.h>
#include <dev/usb/dwc2/dwc2_core.h>

struct bcm_dwctwo_softc {
	struct dwc2_softc	sc_dwc2;
	struct arm32_bus_dma_tag sc_dmat;
	struct arm32_dma_range	sc_dmarange[1];

	void			*sc_ih;
};

int			bcm_dwctwo_match(struct device *, void *, void *);
void			bcm_dwctwo_attach(struct device *, struct device *,
			    void *);
void			bcm_dwctwo_deferred(void *);

const struct cfattach bcmdwctwo_ca = {
	sizeof(struct bcm_dwctwo_softc), bcm_dwctwo_match, bcm_dwctwo_attach,
};

struct cfdriver dwctwo_cd = {
	NULL, "dwctwo", DV_DULL
};

static struct dwc2_core_params bcm_dwctwo_params = {
	.otg_cap			= 0,	/* HNP/SRP capable */
	.otg_ver			= 0,	/* 1.3 */
	.dma_enable			= 1,
	.dma_desc_enable		= 0,
	.speed				= 0,	/* High Speed */
	.enable_dynamic_fifo		= 1,
	.en_multiple_tx_fifo		= 1,
	.host_rx_fifo_size		= 774,	/* 774 DWORDs */
	.host_nperio_tx_fifo_size	= 256,	/* 256 DWORDs */
	.host_perio_tx_fifo_size	= 512,	/* 512 DWORDs */
	.max_transfer_size		= 65535,
	.max_packet_count		= 511,
	.host_channels			= 8,
	.phy_type			= 1,	/* UTMI */
	.phy_utmi_width			= 8,	/* 8 bits */
	.phy_ulpi_ddr			= 0,	/* Single */
	.phy_ulpi_ext_vbus		= 0,
	.i2c_enable			= 0,
	.ulpi_fs_ls			= 0,
	.host_support_fs_ls_low_power	= 0,
	.host_ls_low_power_phy_clk	= 0,	/* 48 MHz */
	.ts_dline			= 0,
	.reload_ctl			= 0,
	.ahbcfg				= 0x10,
	.uframe_sched			= 1,
};

int
bcm_dwctwo_match(struct device *parent, void *match, void *aux)
{
	struct fdt_attach_args *fa = (struct fdt_attach_args *)aux;
	char buffer[128];

	if (fa->fa_node == 0)
		return 0;

	if (!OF_getprop(fa->fa_node, "compatible", buffer,
	    sizeof(buffer)))
		return 0;

	if (!strcmp(buffer, "brcm,bcm2708-usb") ||
	    !strcmp(buffer, "brcm,bcm2835-usb") ||
	    !strcmp(buffer, "broadcom,bcm2835-usb"))
		return 1;

	return 0;
}

void
bcm_dwctwo_attach(struct device *parent, struct device *self, void *aux)
{
	struct bcm_dwctwo_softc *sc = (struct bcm_dwctwo_softc *)self;
	struct fdt_attach_args *fa = aux;
	int pnode, inlen, nac, nsc;
	uint64_t addr, size;
	uint32_t buffer[4];
	extern int physmem;

	printf("\n");

	memcpy(&sc->sc_dmat, fa->fa_dmat, sizeof(sc->sc_dmat));
	sc->sc_dmarange[0].dr_sysbase = 0;
	sc->sc_dmarange[0].dr_busbase = 0xC0000000;
	sc->sc_dmarange[0].dr_len = physmem * PAGE_SIZE;
	sc->sc_dmat._ranges = sc->sc_dmarange;
	sc->sc_dmat._nranges = 1;

	sc->sc_dwc2.sc_iot = fa->fa_iot;
	sc->sc_dwc2.sc_bus.pipe_size = sizeof(struct usbd_pipe);
	sc->sc_dwc2.sc_bus.dmatag = &sc->sc_dmat;
	sc->sc_dwc2.sc_params = &bcm_dwctwo_params;

	if ((pnode = OF_parent(fa->fa_node)) == 0)
		panic("%s: cannot get device tree parent", __func__);

	inlen = OF_getprop(pnode, "#address-cells", buffer,
	    sizeof(buffer));
	if (inlen != sizeof(uint32_t))
		panic("%s: cannot get address cells", __func__);
	nac = betoh32(buffer[0]);

	inlen = OF_getprop(pnode, "#size-cells", buffer,
	    sizeof(buffer));
	if (inlen != sizeof(uint32_t))
		panic("%s: cannot get size cells", __func__);
	nsc = betoh32(buffer[0]);

	inlen = OF_getprop(fa->fa_node, "reg", buffer,
	    sizeof(buffer));
	if (inlen < (nac+nsc) * sizeof(uint32_t))
		panic("%s: cannot extract reg", __func__);

	addr = betoh32(buffer[0]);
	if (nac == 2)
		addr = betoh32(buffer[1]);

	size = betoh32(buffer[nac]);
	if (nsc == 2)
		size = betoh32(buffer[nac + 1]);

	/* XXX: try to cope with 64-byte bus addresses */
	sc->sc_dwc2.sc_ioh = (bus_space_handle_t)(addr >> 32);
	if (bus_space_map(fa->fa_iot, addr, size, 0,
	    &sc->sc_dwc2.sc_ioh))
		panic("%s: bus_space_map failed!", __func__);

	sc->sc_ih = arm_intr_establish_fdt_idx(fa->fa_node, 1, IPL_SCHED,
	    dwc2_intr, (void *)&sc->sc_dwc2, sc->sc_dwc2.sc_bus.bdev.dv_xname);
	if (sc->sc_ih == NULL)
		panic("%s: intr_establish failed!", __func__);

	kthread_create_deferred(bcm_dwctwo_deferred, sc);
}

void
bcm_dwctwo_deferred(void *self)
{
	struct bcm_dwctwo_softc *sc = (struct bcm_dwctwo_softc *)self;
	int rc;

	rc = dwc2_init(&sc->sc_dwc2);
	if (rc != 0)
		return;

	sc->sc_dwc2.sc_child = config_found(&sc->sc_dwc2.sc_bus.bdev,
	    &sc->sc_dwc2.sc_bus, usbctlprint);
}
