/*
 * Copyright (c) 2001 Sendmail, Inc. and its suppliers.
 *	All rights reserved.
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the sendmail distribution.
 *
 *	$Sendmail: sm_os_unixware.h,v 1.5 2001/09/24 20:34:36 ca Exp $
 */

#define SM_OS_NAME	"unixware"

/* try LLONG tests in libsm/t-types.c? */
#ifndef SM_CONF_TEST_LLONG
# define SM_CONF_TEST_LLONG	0
#endif /* !SM_CONF_TEST_LLONG */

/* needs alarm(), our sleep() otherwise hangs. */
#define SM_CONF_SETITIMER	0

#ifndef SM_CONF_SHM
# define SM_CONF_SHM	1
#endif /* SM_CONF_SHM */

/* size_t seems to be signed */
#define SM_CONF_BROKEN_SIZE_T	1

/* can't set real gid */
#ifndef SM_CONF_CANT_SETRGID
# define SM_CONF_CANT_SETRGID	1
#endif /* SM_CONF_CANT_SETRGID */

