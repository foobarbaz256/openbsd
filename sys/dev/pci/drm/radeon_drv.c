/* radeon_drv.c -- ATI Radeon driver -*- linux-c -*-
 * Created: Wed Feb 14 17:10:04 2001 by gareth@valinux.com
 */
/*-
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Gareth Hughes <gareth@valinux.com>
 *
 */

#include "drmP.h"
#include "drm.h"
#include "radeon_drm.h"
#include "radeon_drv.h"
#include "drm_pciids.h"

void	radeon_configure(struct drm_device *);

int radeon_no_wb;

/* drv_PCI_IDs comes from drm_pciids.h, generated from drm_pciids.txt. */
static drm_pci_id_list_t radeon_pciidlist[] = {
	radeon_PCI_IDS
};

void
radeon_configure(struct drm_device *dev)
{
	dev->driver.buf_priv_size	= sizeof(drm_radeon_buf_priv_t);
	dev->driver.load		= radeon_driver_load;
	dev->driver.unload		= radeon_driver_unload;
	dev->driver.firstopen		= radeon_driver_firstopen;
	dev->driver.open		= radeon_driver_open;
	dev->driver.preclose		= radeon_driver_preclose;
	dev->driver.postclose		= radeon_driver_postclose;
	dev->driver.lastclose		= radeon_driver_lastclose;
	dev->driver.get_vblank_counter	= radeon_get_vblank_counter;
	dev->driver.enable_vblank	= radeon_enable_vblank;
	dev->driver.disable_vblank	= radeon_disable_vblank;
	dev->driver.irq_preinstall	= radeon_driver_irq_preinstall;
	dev->driver.irq_postinstall	= radeon_driver_irq_postinstall;
	dev->driver.irq_uninstall	= radeon_driver_irq_uninstall;
	dev->driver.irq_handler		= radeon_driver_irq_handler;
	dev->driver.dma_ioctl		= radeon_cp_buffers;

	dev->driver.ioctls		= radeon_ioctls;
	dev->driver.max_ioctl		= radeon_max_ioctl;

	dev->driver.name		= DRIVER_NAME;
	dev->driver.desc		= DRIVER_DESC;
	dev->driver.date		= DRIVER_DATE;
	dev->driver.major		= DRIVER_MAJOR;
	dev->driver.minor		= DRIVER_MINOR;
	dev->driver.patchlevel		= DRIVER_PATCHLEVEL;

	dev->driver.use_agp		= 1;
	dev->driver.use_mtrr		= 1;
	dev->driver.use_pci_dma		= 1;
	dev->driver.use_sg		= 1;
	dev->driver.use_dma		= 1;
	dev->driver.use_irq		= 1;
	dev->driver.use_vbl_irq		= 1;
}

#ifdef __FreeBSD__
static int
radeon_probe(device_t dev)
{
	return drm_probe(dev, radeon_pciidlist);
}

static int
radeon_attach(device_t nbdev)
{
	struct drm_device *dev = device_get_softc(nbdev);

	bzero(dev, sizeof(struct drm_device));
	radeon_configure(dev);
	return drm_attach(nbdev, radeon_pciidlist);
}

static device_method_t radeon_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		radeon_probe),
	DEVMETHOD(device_attach,	radeon_attach),
	DEVMETHOD(device_detach,	drm_detach),

	{ 0, 0 }
};

static driver_t radeon_driver = {
	"drm",
	radeon_methods,
	sizeof(struct drm_device)
};

extern devclass_t drm_devclass;
#if __FreeBSD_version >= 700010
DRIVER_MODULE(radeon, vgapci, radeon_driver, drm_devclass, 0, 0);
#else
DRIVER_MODULE(radeon, pci, radeon_driver, drm_devclass, 0, 0);
#endif
MODULE_DEPEND(radeon, drm, 1, 1, 1);

#elif defined(__NetBSD__) || defined(__OpenBSD__)

int	radeondrm_probe(struct device *, void *, void *);
void	radeondrm_attach(struct device *, struct device *, void *);

int
#if defined(__OpenBSD__)
radeondrm_probe(struct device *parent, void *match, void *aux)
#else
radeondrm_probe(struct device *parent, struct cfdata *match, void *aux)
#endif
{
	return drm_probe((struct pci_attach_args *)aux, radeon_pciidlist);
}

void
radeondrm_attach(struct device *parent, struct device *self, void *aux)
{
	struct pci_attach_args *pa = aux;
	struct drm_device *dev = (struct drm_device *)self;

	radeon_configure(dev);
	return drm_attach(parent, self, pa, radeon_pciidlist);
}

#if defined(__OpenBSD__)
struct cfattach radeondrm_ca = {
        sizeof (struct drm_device), radeondrm_probe, radeondrm_attach, 
	drm_detach, drm_activate
}; 

struct cfdriver radeondrm_cd = {
	NULL, "radeondrm", DV_DULL
}; 
#else
#ifdef _LKM
CFDRIVER_DECL(radeondrm, DV_TTY, NULL);
#else
CFATTACH_DECL(radeondrm, sizeof(struct drm_device), radeondrm_probe,
    radeondrm_attach, drm_detach, drm_activate);
#endif
#endif

#endif
