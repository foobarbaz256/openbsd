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
#include <arm/cpufunc.h>

#include <dev/ofw/openfirm.h>

#include <dev/sdmmc/sdhcreg.h>
#include <dev/sdmmc/sdhcvar.h>
#include <dev/sdmmc/sdmmcvar.h>

struct bcm_sdhc_softc {
	struct device		 sc_dev;
	bus_space_tag_t		 sc_iot;
	bus_space_handle_t	 sc_ioh;
	void			*sc_ih;
};

int	 bcm_sdhc_match(struct device *, void *, void *);
void	 bcm_sdhc_attach(struct device *, struct device *, void *);

struct cfattach	bcmsdhc_ca = {
	sizeof (struct bcm_sdhc_softc), bcm_sdhc_match, bcm_sdhc_attach
};

struct cfdriver bcmsdhc_cd = {
	NULL, "bcmsdhc", DV_DULL
};

int
bcm_sdhc_match(struct device *parent, void *cfdata, void *aux)
{
	struct fdt_attach_args *fa = (struct fdt_attach_args *)aux;
	char buffer[128];

	if (fa->fa_node == 0)
		return 0;

	if (!OF_getprop(fa->fa_node, "compatible", buffer,
	    sizeof(buffer)))
		return 0;

	if (!strcmp(buffer, "brcm,bcm2835-mmc") ||
	    !strcmp(buffer, "brcm,bcm2835-sdhost") ||
	    !strcmp(buffer, "brcm,bcm2835-sdhci") ||
	    !strcmp(buffer, "broadcom,bcm2835-sdhci"))
		return 1;

	return 0;
}

void
bcm_sdhc_attach(struct device *parent, struct device *self, void *args)
{
	struct bcm_sdhc_softc *sc = (struct bcm_sdhc_softc *)self;
	struct fdt_attach_args *fa = args;
	struct sdhc_softc *ssc;
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

	if ((ssc = (struct sdhc_softc *)config_found(self, NULL, NULL)) == NULL)
		goto mem;

	ssc->sc_flags = 0;
	ssc->sc_flags |= SDHC_F_32BIT_ACCESS;
	ssc->sc_flags |= SDHC_F_NO_HS_BIT;
	/* TODO: dynamically extract clock base */
	ssc->sc_clkbase = 250000; /* Default to 250MHz */

	/* TODO: Fetch frequency from FDT. */

	/* Allocate an array big enough to hold all the possible hosts */
	ssc->sc_host = mallocarray(1, sizeof(struct sdhc_host *),
	    M_DEVBUF, M_WAITOK);

	sc->sc_ih = arm_intr_establish_fdt(fa->fa_node, IPL_SDMMC, sdhc_intr,
	    ssc, ssc->sc_dev.dv_xname);
	if (sc->sc_ih == NULL) {
		printf("%s: unable to establish interrupt\n",
		    ssc->sc_dev.dv_xname);
		goto mem;
	}

	if (sdhc_host_found(ssc, sc->sc_iot, sc->sc_ioh, size, 0,
	    SDHC_VOLTAGE_SUPP_3_3V | SDHC_HIGH_SPEED_SUPP |
	    (SDHC_MAX_BLK_LEN_1024 << SDHC_MAX_BLK_LEN_SHIFT)) != 0) {
		/* XXX: sc->sc_host leak */
		printf("%s: can't initialize host\n", ssc->sc_dev.dv_xname);
		goto intr;
	}

	return;

intr:
	arm_intr_disestablish(sc->sc_ih);
	sc->sc_ih = NULL;
mem:
	bus_space_unmap(sc->sc_iot, sc->sc_ioh, size);
}

int	sdhc_bcm_match(struct device *, void *, void *);
void	sdhc_bcm_attach(struct device *, struct device *, void *);

struct cfattach sdhc_bcm_ca = {
	sizeof (struct sdhc_softc), sdhc_bcm_match, sdhc_bcm_attach
};

int
sdhc_bcm_match(struct device *parent, void *v, void *aux)
{
	return 1;
}

void
sdhc_bcm_attach(struct device *parent, struct device *self, void *aux)
{
	printf("\n");
}
