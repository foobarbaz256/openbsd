/*	$OpenBSD: script.h,v 1.3 2001/01/29 01:58:46 niklas Exp $	*/

/*-
 * Copyright (c) 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 1993, 1994, 1995, 1996
 *	Keith Bostic.  All rights reserved.
 *
 * See the LICENSE file for redistribution information.
 *
 *	@(#)script.h	10.2 (Berkeley) 3/6/96
 */

struct _script {
	pid_t	 sh_pid;		/* Shell pid. */
	int	 sh_master;		/* Master pty fd. */
	int	 sh_slave;		/* Slave pty fd. */
	char	*sh_prompt;		/* Prompt. */
	size_t	 sh_prompt_len;		/* Prompt length. */
	char	 sh_name[64];		/* Pty name */
#ifdef TIOCGWINSZ
	struct winsize sh_win;		/* Window size. */
#endif
	struct termios sh_term;		/* Terminal information. */
};
