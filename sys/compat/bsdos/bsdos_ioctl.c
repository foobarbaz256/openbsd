/*	$OpenBSD: bsdos_ioctl.c,v 1.3 2003/06/03 01:52:41 millert Exp $	*/

/*
 * Copyright (c) 1999 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND TODD C. MILLER DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL TODD C. MILLER BE LIABLE
 * FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/mount.h>

#include <sys/syscallargs.h>

#include <compat/bsdos/bsdos_syscallargs.h>
#include <compat/bsdos/bsdos_ioctl.h>

#include <compat/ossaudio/ossaudio.h>
#include <compat/ossaudio/ossaudiovar.h>

#include <compat/common/compat_util.h>

static void bsdos_to_oss(struct bsdos_sys_ioctl_args *, struct oss_sys_ioctl_args *);

/*
 * BSD/OS and OSS have different values for IOC_*.  Also,
 * sizeof(bsdos_audio_buf_info) != sizeof(oss_audio_buf_info) which
 * is encoded in OSS_SNDCTL_DSP_GETOSPACE and OSS_SNDCTL_DSP_GETISPACE.
 */
static void
bsdos_to_oss(bap, oap)
	struct bsdos_sys_ioctl_args *bap;
	struct oss_sys_ioctl_args *oap;
{
	u_long bcom, ocom;

	bcom = SCARG(bap, com);
	ocom = bcom & ~BSDOS_IOC_DIRMASK;
	switch (bcom & BSDOS_IOC_DIRMASK) {
	case BSDOS_IOC_VOID:
		ocom |= OSS_IOC_VOID;
		break;
	case BSDOS_IOC_OUT:
		if (bcom == BSDOS_SNDCTL_DSP_GETOSPACE)
			ocom = OSS_SNDCTL_DSP_GETOSPACE;
		else if (bcom == BSDOS_SNDCTL_DSP_GETISPACE)
			ocom = OSS_SNDCTL_DSP_GETISPACE;
		else
			ocom |= OSS_IOC_OUT;
		break;
	case BSDOS_IOC_IN:
		ocom |= OSS_IOC_IN;
		break;
	case BSDOS_IOC_INOUT:
		ocom |= OSS_IOC_INOUT;
		break;
	}
	SCARG(oap, fd) = SCARG(bap, fd);
	SCARG(oap, com) = ocom;
	SCARG(oap, data) = SCARG(bap, data);
}

int
bsdos_sys_ioctl(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct bsdos_sys_ioctl_args /* {
		syscallarg(int) fd;
		syscallarg(u_long) com;
		syscallarg(caddr_t) data;
	} */ *uap = v;
        struct oss_sys_ioctl_args ap;

	/*
	 * XXX should support 'T' timer ioctl's
	 * XXX also /dev/sequencer and /dev/patmgr#
	 */
	switch (BSDOS_IOCGROUP(SCARG(uap, com))) {
	case 'M':
		bsdos_to_oss(uap, &ap);
		return (oss_ioctl_mixer(p, &ap, retval));
	case 'Q':
		bsdos_to_oss(uap, &ap);
		return (oss_ioctl_sequencer(p, &ap, retval));
	case 'P':
		bsdos_to_oss(uap, &ap);
		/*
		 * Special handling since the BSD/OS audio_buf_info
		 * struct lacks a fragstotal member.
		 */
		if (SCARG(uap, com) == BSDOS_SNDCTL_DSP_GETOSPACE ||
		    SCARG(uap, com) == BSDOS_SNDCTL_DSP_GETISPACE)
		{
			struct oss_audio_buf_info oss_buf, *oss_bufp;
			struct bsdos_audio_buf_info bsdos_buf;
			caddr_t sg = stackgap_init(p->p_emul);
			int error;

			oss_bufp = stackgap_alloc(&sg, sizeof(*oss_bufp));
			SCARG(&ap, data) = (void *) oss_bufp;
			error = oss_ioctl_audio(p, &ap, retval);
			if (error)
				return (error);
			error = copyin(oss_bufp, &oss_buf, sizeof(oss_buf));
			if (error)
				return (error);
			bsdos_buf.fragments = oss_buf.fragstotal;
			bsdos_buf.fragsize = oss_buf.fragsize;
			bsdos_buf.bytes = oss_buf.bytes;
			error = copyout(&bsdos_buf, SCARG(uap, data),
			    sizeof(bsdos_buf));
			if (error)
				return (error);
		} else
			return (oss_ioctl_audio(p, &ap, retval));
	}
	return (sys_ioctl(p, uap, retval));
}
