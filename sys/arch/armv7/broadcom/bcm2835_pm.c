/*
 * Copyright (c) 2015 Patrick Wildt <patrick@blueri.se>
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
#include <sys/systm.h>
#include <sys/queue.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/evcount.h>
#include <machine/bus.h>
#include <machine/fdt.h>

#include <dev/ofw/openfirm.h>

/* registers */
#define PM_RSTC		0x1c
#define PM_RSTS		0x20
#define PM_WDOG		0x24

/* bits and bytes */
#define PM_PASSWORD		0x5a000000
#define PM_RSTC_CONFIGMASK	0x00000030
#define PM_RSTC_FULL_RESET	0x00000020
#define PM_RSTC_RESET		0x00000102
#define PM_WDOG_TIMEMASK	0x000fffff

#define HREAD4(sc, reg)							\
	(bus_space_read_4((sc)->sc_iot, (sc)->sc_ioh, (reg)))
#define HWRITE4(sc, reg, val)						\
	bus_space_write_4((sc)->sc_iot, (sc)->sc_ioh, (reg), (val))
#define HSET4(sc, reg, bits)						\
	HWRITE4((sc), (reg), HREAD4((sc), (reg)) | (bits))
#define HCLR4(sc, reg, bits)						\
	HWRITE4((sc), (reg), HREAD4((sc), (reg)) & ~(bits))

struct bcmpm_softc {
	struct device		 sc_dev;
	bus_space_tag_t		 sc_iot;
	bus_space_handle_t	 sc_ioh;
};

struct bcmpm_softc *bcmpm_sc;

int	 bcmpm_match(struct device *, void *, void *);
void	 bcmpm_attach(struct device *, struct device *, void *);
int	 bcmpm_activate(struct device *, int);
int	 bcmpm_wdog_cb(void *, int);
void	 bcmpm_wdog_reset(void);

struct cfattach	bcmpm_ca = {
	sizeof (struct bcmpm_softc), bcmpm_match, bcmpm_attach, NULL,
	bcmpm_activate
};

struct cfdriver bcmpm_cd = {
	NULL, "bcmpm", DV_DULL
};

int
bcmpm_match(struct device *parent, void *cfdata, void *aux)
{
	struct fdt_attach_args *fa = (struct fdt_attach_args *)aux;
	char buffer[128];

	if (fa->fa_node == 0)
		return 0;

	if (!OF_getprop(fa->fa_node, "compatible", buffer,
	    sizeof(buffer)))
		return 0;

	if (!strcmp(buffer, "brcm,bcm2835-pm") ||
	    !strcmp(buffer, "brcm,bcm2835-pm-wdt") ||
	    !strcmp(buffer, "broadcom,bcm2835-pm"))
		return 1;

	return 0;
}

void
bcmpm_attach(struct device *parent, struct device *self, void *args)
{
	struct bcmpm_softc *sc = (struct bcmpm_softc *)self;
	struct fdt_attach_args *fa = args;
	int pnode, inlen, nac, nsc;
	uint64_t addr, size;
	uint32_t buffer[4];

	sc->sc_iot = fa->fa_iot;

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
	sc->sc_ioh = (bus_space_handle_t)(addr >> 32);
	if (bus_space_map(sc->sc_iot, addr, size, 0, &sc->sc_ioh))
		panic("%s: bus_space_map failed!", __func__);

	printf("\n");

	bcmpm_sc = sc;

#ifndef SMALL_KERNEL
	wdog_register(bcmpm_wdog_cb, sc);
#endif
}

int
bcmpm_activate(struct device *self, int act)
{
	switch (act) {
	case DVACT_POWERDOWN:
#ifndef SMALL_KERNEL
		wdog_shutdown(self);
#endif
		break;
	}

	return 0;
}

void
bcmpm_wdog_set(struct bcmpm_softc *sc, uint32_t period)
{
	uint32_t rstc, wdog;

	if (period == 0) {
		HWRITE4(sc, PM_RSTC, PM_RSTC_RESET | PM_PASSWORD);
		return;
	}

	rstc = HREAD4(sc, PM_RSTC) & PM_RSTC_CONFIGMASK;
	rstc |= PM_RSTC_FULL_RESET;
	rstc |= PM_PASSWORD;

	wdog = period & PM_WDOG_TIMEMASK;
	wdog |= PM_PASSWORD;

	HWRITE4(sc, PM_RSTC, wdog);
	HWRITE4(sc, PM_RSTC, rstc);
}

int
bcmpm_wdog_cb(void *self, int period)
{
	struct bcmpm_softc *sc = self;

	bcmpm_wdog_set(sc, period << 16);

	return period;
}

void
bcmpm_wdog_reset(void)
{
	struct bcmpm_softc *sc = bcmpm_sc;
	bcmpm_wdog_set(sc, 10);
	delay(100000);
}
