/*	$OpenBSD: conf.c,v 1.5 2005/05/03 13:18:05 tom Exp $	*/

/*
 * Copyright (c) 2004 Tom Cosgrove
 * Copyright (c) 1996 Michael Shalayeff
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <libsa.h>
#include <lib/libsa/ufs.h>
#include <lib/libsa/cd9660.h>
#ifdef notdef
#include <lib/libsa/fat.h>
#include <lib/libsa/nfs.h>
#include <lib/libsa/tftp.h>
#include <lib/libsa/netif.h>
#endif
#include <lib/libsa/unixdev.h>
#include <biosdev.h>
#include <dev/cons.h>
#include "debug.h"

const char version[] = "1.04";
int	debug = 1;

#undef _TEST


void (*sa_cleanup)(void) = NULL;


void (*i386_probe1[])(void) = {
	ps2probe, gateA20on, debug_init, cninit, apmprobe,
	pciprobe, /* smpprobe, */ memprobe
};
void (*i386_probe2[])(void) = {
	diskprobe, cdprobe
};

struct i386_boot_probes probe_list[] = {
	{ "probing", i386_probe1, NENTS(i386_probe1) },
	{ "disk",    i386_probe2, NENTS(i386_probe2) },
};
int nibprobes = NENTS(probe_list);

struct fs_ops file_system[] = {
	{ ufs_open,    ufs_close,    ufs_read,    ufs_write,    ufs_seek,
	  ufs_stat,    ufs_readdir    },
	{ cd9660_open, cd9660_close, cd9660_read, cd9660_write, cd9660_seek,
	  cd9660_stat, cd9660_readdir },
#ifdef notdef
	{ tftp_open,   tftp_close,   tftp_read,   tftp_write,   tftp_seek,
	  tftp_stat,   tftp_readdir   },
	{ nfs_open,    nfs_close,    nfs_read,    nfs_write,    nfs_seek,
	  nfs_stat,    nfs_readdir    },
	{ fat_open,    fat_close,    fat_read,    fat_write,    fat_seek,
	  fat_stat,    fat_readdir    },
#endif
#ifdef _TEST
	{ null_open,   null_close,   null_read,   null_write,   null_seek,
	  null_stat,   null_readdir   }
#endif
};
int nfsys = NENTS(file_system);

struct devsw	devsw[] = {
#ifdef _TEST
	{ "UNIX", unixstrategy, unixopen, unixclose, unixioctl },
#else
	{ "BIOS", biosstrategy, biosopen, biosclose, biosioctl },
#endif
};
int ndevs = NENTS(devsw);

struct consdev constab[] = {
#ifdef _TEST
	{ unix_probe, unix_init, unix_getc, unix_putc },
#else
	{ pc_probe, pc_init, pc_getc, pc_putc },
	{ com_probe, com_init, com_getc, com_putc },
#endif
	{ NULL }
};
struct consdev *cn_tab = constab;
