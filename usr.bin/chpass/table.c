/*	$OpenBSD: table.c,v 1.5 2003/06/03 02:56:06 millert Exp $	*/
/*	$NetBSD: table.c,v 1.3 1995/03/26 04:55:34 glass Exp $	*/

/*-
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
#if 0
static char sccsid[] = "@(#)table.c	8.3 (Berkeley) 4/2/94";
#else
static char rcsid[] = "$OpenBSD: table.c,v 1.5 2003/06/03 02:56:06 millert Exp $";
#endif
#endif /* not lint */

#include <sys/types.h>
#include <stddef.h>
#include "chpass.h"

char e1[] = ": ";
char e2[] = ":,";

ENTRY list[] = {
	{ "login",		p_login,  1,   5, e1,   },
	{ "encrypted password",	p_passwd, 1,   8, e1,   },
	{ "uid",		p_uid,    1,   3, e1,   },
	{ "gid",		p_gid,    1,   3, e1,   },
	{ "class",		p_class,  1,   5, e1,   },
	{ "change",		p_change, 1,   6, NULL, },
	{ "expire",		p_expire, 1,   6, NULL, },
	{ "full name",		p_gecos,  0,   9, e2,   },
	{ "office phone",	p_gecos,  0,  12, e2,   },
	{ "home phone",		p_gecos,  0,  10, e2,   },
	{ "office location",	p_gecos,  0,  15, e2,   },
	{ "home directory",	p_hdir,   1,  14, e1,   },
	{ "shell",		p_shell,  0,   5, e1,   },
	{ NULL, 0, },
};
