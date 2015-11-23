/*	$OpenBSD: pci.c,v 1.3 2015/11/23 13:04:49 reyk Exp $	*/

/*
 * Copyright (c) 2015 Mike Larkin <mlarkin@openbsd.org>
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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>
#include <dev/pci/virtioreg.h>
#include <machine/vmmvar.h>
#include "vmd.h"
#include "pci.h"

struct pci pci;

extern char *__progname;

/* PIC IRQs, assigned to devices in order */
const uint8_t pci_pic_irqs[PCI_MAX_PIC_IRQS] = {3, 5, 9, 10, 11};

/*
 * pci_add_bar
 *
 * Adds a BAR for the PCI device 'id'. On access, 'barfn' will be
 * called, and passed 'cookie' as an identifier.
 *
 * BARs are fixed size, meaning all I/O BARs requested have the 
 * same size and all MMIO BARs have the same size.
 *
 * Parameters:
 *  id: PCI device to add the BAR to (local count, eg if id == 4,
 *      this BAR is to be added to the VM's 5th PCI device)
 *  type: type of the BAR to add (PCI_MAPREG_TYPE_xxx)
 *  barfn: callback function invoked on BAR access
 *  cookie: cookie passed to barfn on access
 *
 * Returns 0 if the BAR was added successfully, 1 otherwise.
 */
int
pci_add_bar(uint8_t id, uint32_t type, void *barfn, void *cookie)
{
	uint8_t bar_reg_idx, bar_ct;

	/* Check id */
	if (id >= pci.pci_dev_ct)
		return (1);

	/* Can only add PCI_MAX_BARS BARs to any device */
	bar_ct = pci.pci_devices[id].pd_bar_ct;
	if (bar_ct >= PCI_MAX_BARS)
		return (1);

	/* Compute BAR address and add */
	bar_reg_idx = (PCI_MAPREG_START + (bar_ct * 4)) / 4;
	if (type == PCI_MAPREG_TYPE_MEM) {
		if (pci.pci_next_mmio_bar >= VMM_PCI_MMIO_BAR_END)
			return (1);

		pci.pci_devices[id].pd_cfg_space[bar_reg_idx] =
		    PCI_MAPREG_MEM_ADDR(pci.pci_next_mmio_bar);
		pci.pci_next_mmio_bar += VMM_PCI_MMIO_BAR_SIZE;
		pci.pci_devices[id].pd_barfunc[bar_ct] = barfn;
		pci.pci_devices[id].pd_bar_cookie[bar_ct] = cookie;
		pci.pci_devices[id].pd_bartype[bar_ct] = PCI_BAR_TYPE_MMIO;
		pci.pci_devices[id].pd_barsize[bar_ct] = VMM_PCI_MMIO_BAR_SIZE;
		pci.pci_devices[id].pd_bar_ct++;
	} else if (type == PCI_MAPREG_TYPE_IO) {
		if (pci.pci_next_io_bar >= VMM_PCI_IO_BAR_END)
			return (1);

		pci.pci_devices[id].pd_cfg_space[bar_reg_idx] =
		     PCI_MAPREG_IO_ADDR(pci.pci_next_io_bar) |
		     PCI_MAPREG_TYPE_IO;
		pci.pci_next_io_bar += VMM_PCI_IO_BAR_SIZE;
		pci.pci_devices[id].pd_barfunc[bar_ct] = barfn;
		pci.pci_devices[id].pd_bar_cookie[bar_ct] = cookie;
		dprintf("%s: adding pci bar cookie for dev %d bar %d = %p",
		    __progname, id, bar_ct, cookie);
		pci.pci_devices[id].pd_bartype[bar_ct] = PCI_BAR_TYPE_IO;
		pci.pci_devices[id].pd_barsize[bar_ct] = VMM_PCI_IO_BAR_SIZE;
		pci.pci_devices[id].pd_bar_ct++;
	}

	return (0);
}

/*
 * pci_add_device
 *
 * Adds a PCI device to the guest VM defined by the supplied parameters.
 *
 * Parameters:
 *  id: the new PCI device ID (0 .. PCI_CONFIG_MAX_DEV)
 *  vid: PCI VID of the new device
 *  pid: PCI PID of the new device
 *  class: PCI 'class' of the new device
 *  subclass: PCI 'subclass' of the new device
 *  subsys_vid: subsystem VID of the new device
 *  subsys_id: subsystem ID of the new device
 *  irq_needed: 1 if an IRQ should be assigned to this PCI device, 0 otherwise
 *  csfunc: PCI config space callback function when the guest VM accesses
 *      CS of this PCI device
 *
 * Return values:
 *  0: the PCI device was added successfully. The PCI device ID is in 'id'.
 *  1: the PCI device addition failed.
 */
int
pci_add_device(uint8_t *id, uint16_t vid, uint16_t pid, uint8_t class,
    uint8_t subclass, uint16_t subsys_vid, uint16_t subsys_id,
    uint8_t irq_needed, pci_cs_fn_t csfunc)
{
	/* Exceeded max devices? */
	if (pci.pci_dev_ct >= PCI_CONFIG_MAX_DEV)
		return (1);

	/* Exceeded max IRQs? */
	/* XXX we could share IRQs ... */
	if (pci.pci_next_pic_irq >= PCI_MAX_PIC_IRQS && irq_needed)
		return (1);

	*id = pci.pci_dev_ct;

	pci.pci_devices[*id].pd_vid = vid;
	pci.pci_devices[*id].pd_did = pid;
	pci.pci_devices[*id].pd_class = class;
	pci.pci_devices[*id].pd_subclass = subclass;
	pci.pci_devices[*id].pd_subsys_vid = subsys_vid;
	pci.pci_devices[*id].pd_subsys_id = subsys_id;

	pci.pci_devices[*id].pd_csfunc = csfunc;

	if (irq_needed) {
		pci.pci_devices[*id].pd_irq =
		    pci_pic_irqs[pci.pci_next_pic_irq];
		pci.pci_devices[*id].pd_int = 1;
		pci.pci_next_pic_irq++;
		dprintf("assigned irq %d to pci dev %d",
		    pci.pci_devices[*id].pd_irq, *id);
	}

	pci.pci_dev_ct ++;

	return (0);
}

/*
 * pci_init
 *
 * Initializes the PCI subsystem for the VM by adding a PCI host bridge
 * as the first PCI device.
 */
void
pci_init(void)
{
	uint8_t id;

	bzero(&pci, sizeof(pci));
	pci.pci_next_mmio_bar = VMM_PCI_MMIO_BAR_BASE;
	pci.pci_next_io_bar = VMM_PCI_IO_BAR_BASE;

	if (pci_add_device(&id, PCI_VENDOR_OPENBSD, PCI_PRODUCT_OPENBSD_PCHB,
	    PCI_CLASS_BRIDGE, PCI_SUBCLASS_BRIDGE_HOST,
	    PCI_VENDOR_OPENBSD, 0, 0, NULL)) {
		log_warnx("%s: can't add PCI host bridge", __progname);
		return;
	}
}

void
pci_handle_address_reg(struct vm_run_params *vrp)
{
	union vm_exit *vei = vrp->vrp_exit;

	/*
	 * vei_dir == 0 : out instruction
	 *
	 * The guest wrote to the address register.
	 */
	if (vei->vei.vei_dir == 0) {
		pci.pci_addr_reg = vei->vei.vei_data;
	} else {
		/*
		 * vei_dir == 1 : in instruction
		 *
		 * The guest read the address register/
		 */
		vei->vei.vei_data = pci.pci_addr_reg;
	}
}

uint8_t
pci_handle_io(struct vm_run_params *vrp)
{
	int i, j, k, l;
	uint16_t reg, b_hi, b_lo;
	pci_iobar_fn_t fn;
	union vm_exit *vei = vrp->vrp_exit;
	uint8_t intr;

	k = -1;
	l = -1;
	reg = vei->vei.vei_port;
	intr = 0xFF;

	for (i = 0 ; i < pci.pci_dev_ct ; i++) {
		for (j = 0 ; j < pci.pci_devices[i].pd_bar_ct; j++) {
			b_lo = PCI_MAPREG_IO_ADDR(pci.pci_devices[i].pd_bar[j]);
			b_hi = b_lo + VMM_PCI_IO_BAR_SIZE;
			if (reg >= b_lo && reg < b_hi) {
				if (pci.pci_devices[i].pd_barfunc[j]) {
					k = j;
					l = i;
				}
			}
		}
	}

	if (k >= 0 && l >= 0) {
		fn = (pci_iobar_fn_t)pci.pci_devices[l].pd_barfunc[k];
		if (fn(vei->vei.vei_dir, reg - 
		    PCI_MAPREG_IO_ADDR(pci.pci_devices[l].pd_bar[k]),
		    &vei->vei.vei_data, &intr,
		    pci.pci_devices[l].pd_bar_cookie[k])) {
			log_warnx("%s: pci i/o access function failed",
			    __progname);
		}
	} else {
		log_warnx("%s: no pci i/o function for reg 0x%llx",
		    __progname, (uint64_t)reg);
	}

	if (intr != 0xFF) {
		intr = pci.pci_devices[l].pd_irq;
	}

	return (intr);
}

void
pci_handle_data_reg(struct vm_run_params *vrp)
{
	union vm_exit *vei = vrp->vrp_exit;
	uint8_t b, d, f, o;
	int ret;
	pci_cs_fn_t csfunc;

	/* abort if the address register is wack */
	if (!(pci.pci_addr_reg & PCI_MODE1_ENABLE)) {
		/* if read, return FFs */
		if (vei->vei.vei_dir == 1)
			vei->vei.vei_data = 0xffffffff;
		log_warnx("invalid address register during pci read: "
		    "0x%llx", (uint64_t)pci.pci_addr_reg);
		return;
	}

	b = (pci.pci_addr_reg >> 16) & 0xff;
	d = (pci.pci_addr_reg >> 11) & 0x1f;
	f = (pci.pci_addr_reg >> 8) & 0x7;
	o = (pci.pci_addr_reg & 0xfc);

	csfunc = pci.pci_devices[d].pd_csfunc;
	if (csfunc != NULL) {
		ret = csfunc(vei->vei.vei_dir, (o / 4), &vei->vei.vei_data);
		if (ret)
			log_warnx("cfg space access function failed for "
			    "pci device %d", d);
		return;
	}

	/* No config space function, fallback to default simple r/w impl. */

	/*
	 * vei_dir == 0 : out instruction
	 *
	 * The guest wrote to the config space location denoted by the current
	 * value in the address register.
	 */
	if (vei->vei.vei_dir == 0) {
		if ((o >= 0x10 && o <= 0x24) &&
		    vei->vei.vei_data == 0xffffffff) {
			vei->vei.vei_data = 0xfffff000;
		}
		pci.pci_devices[d].pd_cfg_space[o / 4] = vei->vei.vei_data;
	} else {
		/*
		 * vei_dir == 1 : in instruction
		 *
		 * The guest read from the config space location determined by
		 * the current value in the address register.
		 */
		vei->vei.vei_data = pci.pci_devices[d].pd_cfg_space[o / 4];
	}
}
