divert(-1)
#
# Copyright (c) 1998, 1999 Sendmail, Inc. and its suppliers.
#	All rights reserved.
# Copyright (c) 1995 Eric P. Allman.  All rights reserved.
# Copyright (c) 1988, 1993
#	The Regents of the University of California.  All rights reserved.
#
# By using this file, you agree to the terms and conditions set
# forth in the LICENSE file which can be found at the top level of
# the sendmail distribution.
#
#

divert(0)
VERSIONID(`$Id: aix2.m4,v 1.1.1.2 2001/01/15 20:52:33 millert Exp $')
define(`LOCAL_MAILER_PATH', /bin/bellmail)dnl
define(`LOCAL_MAILER_ARGS', mail $u)dnl
_DEFIFNOT(`LOCAL_MAILER_FLAGS', `mn9')dnl
define(`confEBINDIR', `/usr/lib')dnl
define(`confTIME_ZONE', `USE_TZ')dnl
