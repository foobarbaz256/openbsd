/* $OpenBSD: auth-options.h,v 1.19 2010/04/16 01:47:26 djm Exp $ */

/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

#ifndef AUTH_OPTIONS_H
#define AUTH_OPTIONS_H

/* Linked list of custom environment strings */
struct envstring {
	struct envstring *next;
	char   *s;
};

/* Flags that may be set in authorized_keys options. */
extern int no_port_forwarding_flag;
extern int no_agent_forwarding_flag;
extern int no_x11_forwarding_flag;
extern int no_pty_flag;
extern int no_user_rc;
extern char *forced_command;
extern struct envstring *custom_environment;
extern int forced_tun_device;
extern int key_is_cert_authority;

int	auth_parse_options(struct passwd *, char *, char *, u_long);
void	auth_clear_options(void);
int	auth_cert_options(Key *, struct passwd *);

#endif
