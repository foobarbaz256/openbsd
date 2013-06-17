/*
 * Copyright (c) 2004 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* $Id: windlocl.h,v 1.1 2013/06/17 19:11:45 robert Exp $ */

#ifndef _WINDLOCL_H_
#define _WINDLOCL_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <krb5-types.h>
#include <roken.h>

#include "wind.h"
#include "wind_err.h"

int _wind_combining_class(uint32_t);

int _wind_stringprep_testbidi(const uint32_t *, size_t, wind_profile_flags);

int _wind_stringprep_error(const uint32_t, wind_profile_flags);

int _wind_stringprep_prohibited(const uint32_t *, size_t, wind_profile_flags);

int _wind_stringprep_map(const uint32_t *, size_t,
			 uint32_t *, size_t *,
			 wind_profile_flags);

int _wind_stringprep_normalize(const uint32_t *, size_t, uint32_t *, size_t *);

int _wind_ldap_case_exact_attribute(const uint32_t *, size_t,
				    uint32_t *, size_t *);


#endif /* _WINDLOCL_H_ */
