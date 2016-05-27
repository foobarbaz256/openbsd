/* $OpenBSD$ */
/*
 * Copyright (c) 2016 Patrick Wildt <patrick@blueri.se>
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
 * Copyright (c) 2010 Broadcom Corporation
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/queue.h>

#include <dev/sdmmc/sdmmcvar.h>

int	 bwfm_match(struct device *, void *, void *);
void	 bwfm_attach(struct device *, struct device *, void *);
void	 bwfm_attach_deferred(struct device *);
int	 bwfm_detach(struct device *, int);

struct bwfm_softc {
	struct device		  sc_dev;
	struct sdmmc_function	**sc_sf;
};

struct cfattach bwfm_ca = {
	sizeof(struct bwfm_softc),
	bwfm_match,
	bwfm_attach,
	bwfm_detach,
};

struct cfdriver bwfm_cd = {
	NULL, "bwfm", DV_DULL
};

int
bwfm_match(struct device *parent, void *match, void *aux)
{
	struct sdmmc_attach_args *saa = aux;
	struct sdmmc_function *sf = saa->sf;
	struct sdmmc_cis *cis;

	/* Not SDIO. */
	if (sf == NULL)
		return 0;

	/* Look for Broadcom 4334. */
	cis = &sf->sc->sc_fn0->cis;
	if (cis->manufacturer != 0x02d0 || cis->product != 0x4334)
		return 0;

	/* We need both functions, but ... */
	if (sf->sc->sc_function_count <= 1)
		return 0;

	/* ... only attach for one. */
	if (sf->number != 1)
		return 0;

	return 1;
}

void
bwfm_attach(struct device *parent, struct device *self, void *aux)
{
	struct bwfm_softc *sc = (struct bwfm_softc *)self;
	struct sdmmc_attach_args *saa = aux;
	struct sdmmc_function *sf = saa->sf;

	printf("\n");

	sc->sc_sf = mallocarray(sf->sc->sc_function_count + 1,
	    sizeof(struct sdmmc_function *), M_DEVBUF, M_WAITOK);

	if (sc->sc_sf == NULL)
		return;

	/* Copy all function pointers. */
	SIMPLEQ_FOREACH(sf, &saa->sf->sc->sf_head, sf_list) {
		sc->sc_sf[sf->number] = sf;
	}

	config_defer(self, bwfm_attach_deferred);
}

void
bwfm_attach_deferred(struct device *self)
{
	struct bwfm_softc *sc = (struct bwfm_softc *)self;

	/* TODO: set block size */

	/* Enable Function 1. */
	if (sdmmc_io_function_enable(sc->sc_sf[1]) != 0) {
		printf("%s: cannot enable function 1\n", DEVNAME(sc));
		goto err;
	}

	return;

err:
	free(sc->sc_sf, M_DEVBUF, 0);
}

int
bwfm_detach(struct device *self, int flags)
{
	return 0;
}
