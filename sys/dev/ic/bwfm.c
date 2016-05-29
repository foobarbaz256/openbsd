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

#include <dev/ic/bwfmreg.h>

#ifdef BWFM_DEBUG
#define DPRINTF(x)	do { if (bwfm_debug > 0) printf x; } while (0)
#define DPRINTFN(n, x)	do { if (bwfmm_debug >= (n)) printf x; } while (0)
int bfwm_debug = 1;
#else
#define DPRINTF(x)	do { ; } while (0)
#define DPRINTFN(n, x)	do { ; } while (0)
#endif

struct bwfm_softc {
	struct device		  sc_dev;
	struct sdmmc_function	**sc_sf;
	uint32_t		  sc_bar0;
};

int		 bwfm_match(struct device *, void *, void *);
void		 bwfm_attach(struct device *, struct device *, void *);
void		 bwfm_attach_deferred(struct device *);
int		 bwfm_detach(struct device *, int);

uint8_t		 bwfm_read_1(struct bwfm_softc *, uint32_t);
uint32_t	 bwfm_read_4(struct bwfm_softc *, uint32_t);
void		 bwfm_write_1(struct bwfm_softc *, uint32_t, uint8_t);
void		 bwfm_write_4(struct bwfm_softc *, uint32_t, uint32_t);

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
	sf = saa->sf;

	/* FIXME bus is locked in attach */
	rw_assert_wrlock(&sf->sc->sc_lock);
	rw_exit(&sf->sc->sc_lock);

	/* TODO: set block size */

	/* Enable Function 1. */
	if (sdmmc_io_function_enable(sc->sc_sf[1]) != 0) {
		printf("%s: cannot enable function 1\n", DEVNAME(sc));
		goto err;
	}

	DPRINTF(("%s: F1 signature read @0x18000000=%x\n", DEVNAME(sc),
	    bwfm_read_4(sc, 0x18000000)));

	/* Force PLL off */
	bwfm_write_1(sc, BWFM_SDIO_FUNC1_CHIPCLKCSR,
	    BWFM_SDIO_FUNC1_CHIPCLKCSR_FORCE_HW_CLKREQ_OFF |
	    BWFM_SDIO_FUNC1_CHIPCLKCSR_ALP_AVAIL_REQ);

	/* FIXME re-lock again */
	rw_enter_write(&sf->sc->sc_lock);

	return;

err:
	free(sc->sc_sf, M_DEVBUF, 0);

	/* FIXME re-lock again */
	rw_enter_write(&sf->sc->sc_lock);
}

int
bwfm_detach(struct device *self, int flags)
{
	return 0;
}

uint8_t
bwfm_read_1(struct bwfm_softc *sc, uint32_t addr)
{
	struct sdmmc_function *sf;
	uint8_t rv;

	/*
	 * figure out how to read the register based on address range
	 * 0x00 ~ 0x7FF: function 0 CCCR and FBR
	 * 0x10000 ~ 0x1FFFF: function 1 miscellaneous registers
	 * The rest: function 1 silicon backplane core registers
	 */
	if ((addr & ~0x7ff) == 0)
		sf = sc->sc_sf[0];
	else
		sf = sc->sc_sf[1];

	rw_enter_write(&sf->sc->sc_lock);
	rv = sdmmc_io_read_1(sf, addr);
	rw_exit(&sf->sc->sc_lock);
	return rv;
}

uint32_t
bwfm_read_4(struct bwfm_softc *sc, uint32_t addr)
{
	struct sdmmc_function *sf;
	uint32_t bar0 = addr & ~0x7fff;
	uint32_t rv;

	if (sc->sc_bar0 != bar0) {
		bwfm_write_1(sc, BWFM_SDIO_FUNC1_SBADDRLOW,
		    (bar0 >>  8) & 0x80);
		bwfm_write_1(sc, BWFM_SDIO_FUNC1_SBADDRMID,
		    (bar0 >> 16) & 0xff);
		bwfm_write_1(sc, BWFM_SDIO_FUNC1_SBADDRHIGH,
		    (bar0 >> 24) & 0xff);
		sc->sc_bar0 = bar0;
	}

	addr &= BWFM_SDIO_SB_OFT_ADDR_MASK;
	addr |= BWFM_SDIO_SB_ACCESS_2_4B_FLAG;

	/*
	 * figure out how to read the register based on address range
	 * 0x00 ~ 0x7FF: function 0 CCCR and FBR
	 * 0x10000 ~ 0x1FFFF: function 1 miscellaneous registers
	 * The rest: function 1 silicon backplane core registers
	 */
	if ((addr & ~0x7ff) == 0)
		sf = sc->sc_sf[0];
	else
		sf = sc->sc_sf[1];

	rw_enter_write(&sf->sc->sc_lock);
	rv = sdmmc_io_read_4(sf, addr);
	rw_exit(&sf->sc->sc_lock);
	return rv;
}

void
bwfm_write_1(struct bwfm_softc *sc, uint32_t addr, uint8_t data)
{
	struct sdmmc_function *sf;

	/*
	 * figure out how to read the register based on address range
	 * 0x00 ~ 0x7FF: function 0 CCCR and FBR
	 * 0x10000 ~ 0x1FFFF: function 1 miscellaneous registers
	 * The rest: function 1 silicon backplane core registers
	 */
	if ((addr & ~0x7ff) == 0)
		sf = sc->sc_sf[0];
	else
		sf = sc->sc_sf[1];

	rw_enter_write(&sf->sc->sc_lock);
	sdmmc_io_write_1(sf, addr, data);
	rw_exit(&sf->sc->sc_lock);
}

void
bwfm_write_4(struct bwfm_softc *sc, uint32_t addr, uint32_t data)
{
	struct sdmmc_function *sf;
	uint32_t bar0 = addr & ~0x7fff;

	if (sc->sc_bar0 != bar0) {
		bwfm_write_1(sc, 0x1000A, (bar0 >>  8) & 0x80);
		bwfm_write_1(sc, 0x1000B, (bar0 >> 16) & 0xff);
		bwfm_write_1(sc, 0x1000C, (bar0 >> 24) & 0xff);
		sc->sc_bar0 = bar0;
	}

	addr &= 0x7fff;
	addr |= 0x8000;

	/*
	 * figure out how to read the register based on address range
	 * 0x00 ~ 0x7FF: function 0 CCCR and FBR
	 * 0x10000 ~ 0x1FFFF: function 1 miscellaneous registers
	 * The rest: function 1 silicon backplane core registers
	 */
	if ((addr & ~0x7ff) == 0)
		sf = sc->sc_sf[0];
	else
		sf = sc->sc_sf[1];

	rw_enter_write(&sf->sc->sc_lock);
	sdmmc_io_write_4(sf, addr, data);
	rw_exit(&sf->sc->sc_lock);
}
