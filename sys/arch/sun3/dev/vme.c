/*	$NetBSD: vme.c,v 1.3 1996/03/26 15:16:19 gwr Exp $	*/

/*
 * Copyright (c) 1994 Gordon W. Ross
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Gordon Ross
 * 4. The name of the Author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>

#include <machine/autoconf.h>
/* #include <machine/vme.h> */

static int  vmes_match __P((struct device *, void *, void *));
static int  vmel_match __P((struct device *, void *, void *));

static void vme_attach __P((struct device *, struct device *, void *));

struct cfattach vmes_ca = {
	sizeof(struct device), vmes_match, vme_attach
};

struct cfdriver vmes_cd = {
	NULL, "vmes", DV_DULL
};

struct cfattach vmel_ca = {
	sizeof(struct device), vmel_match, vme_attach
};

struct cfdriver vmel_cd = {
	NULL, "vmel", DV_DULL
};


/* Does this machine have a VME bus? */
extern int cpu_has_vme;

static int
vmes_match(parent, vcf, aux)
	struct device *parent;
	void *vcf, *aux;
{
	struct confargs *ca = aux;

	if (ca->ca_bustype != BUS_VME16)
		return (0);
	return (cpu_has_vme);
}

static int
vmel_match(parent, vcf, aux)
	struct device *parent;
	void *vcf, *aux;
{
	struct confargs *ca = aux;

	if (ca->ca_bustype != BUS_VME32)
		return (0);
	return (cpu_has_vme);
}

static void
vme_attach(parent, self, args)
	struct device *parent;
	struct device *self;
	void *args;
{
	printf("\n");

	/* We know ca_bustype == BUS_VMExx */
	(void) config_search(bus_scan, self, args);
}
