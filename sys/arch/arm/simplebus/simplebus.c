/* $OpenBSD: simplebus.c,v 1.5 2016/06/12 13:10:06 kettenis Exp $ */
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/malloc.h>

#include <dev/ofw/openfirm.h>

#include <arm/fdt.h>

int simplebus_match(struct device *, void *, void *);
void simplebus_attach(struct device *, struct device *, void *);
int simplebus_detach(struct device *, int);

void simplebus_attach_node(struct device *, int);
int simplebus_bs_map(void *, bus_addr_t, bus_size_t,
		     int , bus_space_handle_t *);

struct simplebus_node {
	struct simplebus_softc	*sn_sc;
	struct bus_space	 sn_bus;
};

struct simplebus_softc {
	struct device		 sc_dev;
	int			 sc_node;
	bus_space_tag_t		 sc_iot;
	bus_dma_tag_t		 sc_dmat;
	struct simplebus_node	*sc_sn;
};

struct cfattach simplebus_ca = {
	sizeof(struct simplebus_softc),
	simplebus_match,
	simplebus_attach,
	simplebus_detach,
	config_activate_children
};

struct cfdriver simplebus_cd = {
	NULL, "simplebus", DV_DULL
};

/*
 * Simplebus is a generic bus with no special casings.
 */
int
simplebus_match(struct device *parent, void *cfdata, void *aux)
{
	struct fdt_attach_args *fa = (struct fdt_attach_args *)aux;

	if (fa->fa_node == 0)
		return (0);

	if (!OF_is_compatible(fa->fa_node, "simple-bus"))
		return (0);

	return (1);
}

void
simplebus_attach(struct device *parent, struct device *self, void *aux)
{
	struct simplebus_softc *sc = (struct simplebus_softc *)self;
	struct fdt_attach_args *fa = (struct fdt_attach_args *)aux;
	struct simplebus_node *sn;
	char name[32];
	int node;

	sc->sc_node = fa->fa_node;
	sc->sc_iot = fa->fa_iot;
	sc->sc_dmat = fa->fa_dmat;

	if (OF_getprop(sc->sc_node, "name", name, sizeof(name)) > 0) {
		name[sizeof(name) - 1] = 0;
		printf(": \"%s\"", name);
	}

	printf("\n");

	/* Allocate a single bus tag for our children. */
	sn = malloc(sizeof(*sn), M_DEVBUF, M_NOWAIT);
	if (sn == NULL)
		return;

	sc->sc_sn = sn;
	sn->sn_sc = sc;
	memcpy(&sn->sn_bus, sc->sc_iot, sizeof(sn->sn_bus));
	sn->sn_bus.bs_cookie = sn;
	sn->sn_bus.bs_map = simplebus_bs_map;

	/* Scan the whole tree. */
	for (node = OF_child(sc->sc_node);
	    node != 0;
	    node = OF_peer(node))
	{
		simplebus_attach_node(self, node);
	}
}


int
simplebus_detach(struct device *self, int flags)
{
	struct simplebus_softc *sc = (struct simplebus_softc *)self;

	free(sc->sc_sn, M_DEVBUF, sizeof(*sc->sc_sn));
	return 0;
}

/*
 * Look for a driver that wants to be attached to this node.
 */
void
simplebus_attach_node(struct device *self, int node)
{
	struct simplebus_softc	*sc = (struct simplebus_softc *)self;
	struct fdt_attach_args	 fa;
	char			 buffer[128];
	int			 len;

	if (!OF_getprop(node, "compatible", buffer, sizeof(buffer)))
		return;

	if (OF_getprop(node, "status", buffer, sizeof(buffer)))
		if (!strcmp(buffer, "disabled"))
			return;

	memset(&fa, 0, sizeof(fa));
	fa.fa_name = "";
	fa.fa_node = node;
	fa.fa_iot = &sc->sc_sn->sn_bus;
	fa.fa_dmat = sc->sc_dmat;

	len = OF_getproplen(node, "reg");
	if (len > 0 && (len % sizeof(uint32_t)) == 0) {
		fa.fa_reg = malloc(len, M_DEVBUF, M_WAITOK);
		fa.fa_nreg = len / sizeof(uint32_t);

		OF_getpropintarray(node, "reg", fa.fa_reg, len);
	}

	len = OF_getproplen(node, "interrupts");
	if (len > 0 && (len % sizeof(uint32_t)) == 0) {
		fa.fa_intr = malloc(len, M_DEVBUF, M_WAITOK);
		fa.fa_nintr = len / sizeof(uint32_t);

		OF_getpropintarray(node, "interrupts", fa.fa_intr, len);
	}

	/* TODO: attach the device's clocks first? */

	config_found(self, &fa, NULL);

	free(fa.fa_reg, M_DEVBUF, fa.fa_nreg * sizeof(uint32_t));
	free(fa.fa_intr, M_DEVBUF, fa.fa_nintr * sizeof(uint32_t));
}

/*
 * Translate memory address if needed.
 */
int
simplebus_bs_map(void *t, bus_addr_t bpa, bus_size_t size,
		 int flag, bus_space_handle_t *bshp)
{
	struct simplebus_node *sn = (struct simplebus_node *)t;
	int bus, parent, pac, psc, ac, sc, rlen, rone;
	uint64_t addr, rfrom, rto, rsize;
	uint32_t *range, *ranges;

	addr = bpa;
	bus = sn->sn_sc->sc_node;
	parent = OF_parent(bus);
	if (parent == 0)
		return bus_space_map(sn->sn_sc->sc_iot, addr, size, flag, bshp);

	rlen = OF_getproplen(bus, "ranges") / sizeof(uint32_t);
	if (rlen < 0)
		return EINVAL;
	if (rlen == 0)
		return bus_space_map(sn->sn_sc->sc_iot, addr, size, flag, bshp);

	range = ranges = malloc(rlen * sizeof(uint32_t), M_DEVBUF, M_NOWAIT);
	if (ranges == NULL)
		return ENOMEM;

	OF_getpropintarray(bus, "ranges", ranges, rlen);

	pac = OF_getpropint(parent, "#address-cells", 0);
	if (pac <= 0 || pac > 2)
		goto invalid;

	psc = OF_getpropint(parent, "#size-cells", 0);
	if (psc <= 0 || psc > 2)
		goto invalid;

	ac = OF_getpropint(bus, "#address-cells", pac);
	if (ac <= 0 || ac > 2)
		goto invalid;

	sc = OF_getpropint(bus, "#size-cells", psc);
	if (sc <= 0 || sc > 2)
		goto invalid;

	/* Size of one range entry in 32-bits. */
	rone = pac + ac + sc;

	/* For each range. */
	for (range = ranges; rlen >= rone; rlen -= rone, range += rone) {
		/* Extract from and size, so we can see if we fit. */
		rfrom = range[0];
		if (ac == 2)
			rfrom = (rfrom << 32) + range[1];
		rsize = range[ac + pac];
		if (sc == 2)
			rsize = (rsize << 32) + range[ac + pac + 1];

		/* Try next, if we're not in the range. */
		if (addr < rfrom || (addr + size) > (rfrom + rsize))
			continue;

		/* All good, extract to address and translate. */
		rto = range[ac];
		if (pac == 2)
			rto = (rto << 32) + range[ac + 1];

		addr -= rfrom;
		addr += rto;

		free(ranges, M_DEVBUF, rlen * sizeof(int));
		return bus_space_map(sn->sn_sc->sc_iot, addr, size, flag, bshp);
	}

	free(ranges, M_DEVBUF, rlen * sizeof(int));
	return ESRCH;

invalid:
	free(ranges, M_DEVBUF, rlen * sizeof(int));
	return EINVAL;
}
