PUSHDIVERT(-1)
#
# Copyright (c) 1998-2000 Sendmail, Inc. and its suppliers.
#	All rights reserved.
# Copyright (c) 1983 Eric P. Allman.  All rights reserved.
# Copyright (c) 1988, 1993
#	The Regents of the University of California.  All rights reserved.
#
# By using this file, you agree to the terms and conditions set
# forth in the LICENSE file which can be found at the top level of
# the sendmail distribution.
#
#

ifdef(`POP_MAILER_PATH',, `define(`POP_MAILER_PATH', /usr/lib/mh/spop)')
_DEFIFNOT(`POP_MAILER_FLAGS', `Penu')
ifdef(`POP_MAILER_ARGS',, `define(`POP_MAILER_ARGS', `pop $u')')

POPDIVERT

####################################
###   POP Mailer specification   ###
####################################

VERSIONID(`$Sendmail: pop.m4,v 8.21 2000/09/02 17:46:43 ca Exp $')

Mpop,		P=POP_MAILER_PATH, F=_MODMF_(CONCAT(`lsDFMq', POP_MAILER_FLAGS), `POP'), S=EnvFromL, R=EnvToL/HdrToL,
		T=DNS/RFC822/X-Unix,
		A=POP_MAILER_ARGS

LOCAL_CONFIG
# POP mailer is a pseudo-domain
CPPOP
