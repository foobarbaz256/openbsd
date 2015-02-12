/*	$OpenBSD: octdwctwo.c,v 1.2 2015/02/12 00:23:58 uebayasi Exp $	*/

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

#include <machine/intr.h>
#include <machine/bus.h>
#include <machine/octeonreg.h>
#include <machine/octeonvar.h>

#include <dev/usb/usb.h>
#include <dev/usb/usbdi.h>
#include <dev/usb/usbdivar.h>
#include <dev/usb/usb_mem.h>
#include <dev/usb/usb_quirks.h>

#include <octeon/dev/usb_port.h>
#include <octeon/dev/iobusvar.h>
#include <octeon/dev/cn30xxusbnvar.h>
#include <octeon/dev/cn30xxusbcvar.h>
#include <octeon/dev/cn30xxusbnreg.h>
#include <octeon/dev/cn30xxusbcreg.h>

#include <dev/usb/dwc2/dwc2var.h>
#include <dev/usb/dwc2/dwc2.h>
#include <dev/usb/dwc2/dwc2_core.h>

struct octdwctwo_softc {
	struct dwc2_softc	sc_dwc2;
#if 0
	/* USBN bus space */
	bus_space_tag_t		sc_bust;
	bus_space_handle_t	sc_regh;
#endif
	void			*sc_ih;
};

int			octdwctwo_match(struct device *, void *, void *);
void			octdwctwo_attach(struct device *, struct device *,
			    void *);
void			octdwctwo_attach_deferred(struct device *);

const struct cfattach octdwctwo_ca = {
	sizeof(struct octdwctwo_softc), octdwctwo_match, octdwctwo_attach,
};

struct cfdriver dwctwo_cd = {
	NULL, "dwctwo", DV_DULL
};

static struct dwc2_core_params octdwctwo_params = {
	.otg_cap = 2,
	.otg_ver = 0,
	.dma_enable = 1,
	.dma_desc_enable = 0,
	.speed = 0,
	.enable_dynamic_fifo = 1,
	.en_multiple_tx_fifo = 0,
	.host_rx_fifo_size = 128/*XXX*/,
	.host_nperio_tx_fifo_size = 128/*XXX*/,
	.host_perio_tx_fifo_size = 128/*XXX*/,
	.max_transfer_size = 65535,
	.max_packet_count = 511,
	.host_channels = 8,
	.phy_type = 1,
	.phy_utmi_width = 16,
	.phy_ulpi_ddr = 0,
	.phy_ulpi_ext_vbus = 0,
	.i2c_enable = 0,
	.ulpi_fs_ls = 0,
	.host_support_fs_ls_low_power = 0,
	.host_ls_low_power_phy_clk = 0,
	.ts_dline = 0,
	.reload_ctl = 0,
	.ahbcfg = 0x7,
	.uframe_sched = 1,
};

int
octdwctwo_match(struct device *parent, void *match, void *aux)
{
	return (1);
}

void
octdwctwo_attach(struct device *parent, struct device *self, void *aux)
{
	struct octdwctwo_softc *sc = (struct octdwctwo_softc *)self;
	struct iobus_attach_args *aa = aux;
	int rc;

	sc->sc_dwc2.sc_iot = aa->aa_bust;
	sc->sc_dwc2.sc_bus.pipe_size = sizeof(struct usbd_pipe);
	sc->sc_dwc2.sc_bus.dmatag = aa->aa_dmat;
	sc->sc_dwc2.sc_params = &octdwctwo_params;

	rc = bus_space_map(aa->aa_bust, USBC_BASE, USBC_SIZE,
	    0, &sc->sc_dwc2.sc_ioh);
	KASSERT(rc == 0);

#if 0
	sc->sc_bust = aa->aa_bust;
	rc = bus_space_map(aa->aa_bust, USBN_2_BASE, USBN_2_SIZE,
	    0, &sc->sc_regh);
	KASSERT(rc == 0);
#endif

	sc->sc_ih = octeon_intr_establish(CIU_INT_USB, IPL_USB, dwc2_intr,
	    (void *)&sc->sc_dwc2, sc->sc_dwc2.sc_bus.bdev.dv_xname);
	KASSERT(sc->sc_ih != NULL);

	printf("\n");

	config_defer(self, octdwctwo_attach_deferred);
}

void
octdwctwo_attach_deferred(struct device *self)
{
	struct octdwctwo_softc *sc = (struct octdwctwo_softc *)self;
	int error;

	error = dwc2_init(&sc->sc_dwc2);
	if (error != 0)
		return;
	sc->sc_dwc2.sc_child = config_found(&sc->sc_dwc2.sc_bus.bdev,
	    &sc->sc_dwc2.sc_bus, usbctlprint);
}
