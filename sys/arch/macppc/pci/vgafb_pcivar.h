/*	$OpenBSD: vgafb_pcivar.h,v 1.6 2012/06/21 10:08:16 mpi Exp $	*/
/*	$NetBSD: vga_pcivar.h,v 1.1 1996/11/19 04:38:36 cgd Exp $	*/

/*
 * Copyright (c) 1995, 1996 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 * 
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND 
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

#define	DEVICE_IS_VGA_PCI(class)					\
	    (((PCI_CLASS(class) == PCI_CLASS_DISPLAY &&			\
	      PCI_SUBCLASS(class) == PCI_SUBCLASS_DISPLAY_VGA) ||	\
	     (PCI_CLASS(class) == PCI_CLASS_DISPLAY &&			\
	      PCI_SUBCLASS(class) == PCI_SUBCLASS_DISPLAY_MISC) ||	\
	     (PCI_CLASS(class) == PCI_CLASS_PREHISTORIC &&		\
	      PCI_SUBCLASS(class) == PCI_SUBCLASS_PREHISTORIC_VGA)) ? 1 : 0)

extern pcitag_t vgafb_pci_console_tag;

/* XXX */
int	vgafb_cnattach(bus_space_tag_t, bus_space_tag_t, int, int);
