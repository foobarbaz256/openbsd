/* $OpenBSD: dh_err.c,v 1.14 2014/06/12 15:49:28 deraadt Exp $ */
/* ====================================================================
 * Copyright (c) 1999-2011 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

/* NOTE: this file was auto generated by the mkerr.pl script: any changes
 * made to it will be overwritten when the script next updates this file,
 * only reason strings will be preserved.
 */

#include <stdio.h>
#include <openssl/err.h>
#include <openssl/dh.h>

/* BEGIN ERROR CODES */
#ifndef OPENSSL_NO_ERR

#define ERR_FUNC(func) ERR_PACK(ERR_LIB_DH,func,0)
#define ERR_REASON(reason) ERR_PACK(ERR_LIB_DH,0,reason)

static ERR_STRING_DATA DH_str_functs[]=
	{
{ERR_FUNC(DH_F_COMPUTE_KEY),	"COMPUTE_KEY"},
{ERR_FUNC(DH_F_DHPARAMS_PRINT_FP),	"DHparams_print_fp"},
{ERR_FUNC(DH_F_DH_BUILTIN_GENPARAMS),	"DH_BUILTIN_GENPARAMS"},
{ERR_FUNC(DH_F_DH_COMPUTE_KEY),	"DH_compute_key"},
{ERR_FUNC(DH_F_DH_GENERATE_KEY),	"DH_generate_key"},
{ERR_FUNC(DH_F_DH_GENERATE_PARAMETERS_EX),	"DH_generate_parameters_ex"},
{ERR_FUNC(DH_F_DH_NEW_METHOD),	"DH_new_method"},
{ERR_FUNC(DH_F_DH_PARAM_DECODE),	"DH_PARAM_DECODE"},
{ERR_FUNC(DH_F_DH_PRIV_DECODE),	"DH_PRIV_DECODE"},
{ERR_FUNC(DH_F_DH_PRIV_ENCODE),	"DH_PRIV_ENCODE"},
{ERR_FUNC(DH_F_DH_PUB_DECODE),	"DH_PUB_DECODE"},
{ERR_FUNC(DH_F_DH_PUB_ENCODE),	"DH_PUB_ENCODE"},
{ERR_FUNC(DH_F_DO_DH_PRINT),	"DO_DH_PRINT"},
{ERR_FUNC(DH_F_GENERATE_KEY),	"GENERATE_KEY"},
{ERR_FUNC(DH_F_GENERATE_PARAMETERS),	"GENERATE_PARAMETERS"},
{ERR_FUNC(DH_F_PKEY_DH_DERIVE),	"PKEY_DH_DERIVE"},
{ERR_FUNC(DH_F_PKEY_DH_KEYGEN),	"PKEY_DH_KEYGEN"},
{0,NULL}
	};

static ERR_STRING_DATA DH_str_reasons[]=
	{
{ERR_REASON(DH_R_BAD_GENERATOR)          ,"bad generator"},
{ERR_REASON(DH_R_BN_DECODE_ERROR)        ,"bn decode error"},
{ERR_REASON(DH_R_BN_ERROR)               ,"bn error"},
{ERR_REASON(DH_R_DECODE_ERROR)           ,"decode error"},
{ERR_REASON(DH_R_INVALID_PUBKEY)         ,"invalid public key"},
{ERR_REASON(DH_R_KEYS_NOT_SET)           ,"keys not set"},
{ERR_REASON(DH_R_KEY_SIZE_TOO_SMALL)     ,"key size too small"},
{ERR_REASON(DH_R_MODULUS_TOO_LARGE)      ,"modulus too large"},
{ERR_REASON(DH_R_NON_FIPS_METHOD)        ,"non fips method"},
{ERR_REASON(DH_R_NO_PARAMETERS_SET)      ,"no parameters set"},
{ERR_REASON(DH_R_NO_PRIVATE_VALUE)       ,"no private value"},
{ERR_REASON(DH_R_PARAMETER_ENCODING_ERROR),"parameter encoding error"},
{0,NULL}
	};

#endif

void ERR_load_DH_strings(void)
	{
#ifndef OPENSSL_NO_ERR

	if (ERR_func_error_string(DH_str_functs[0].error) == NULL)
		{
		ERR_load_strings(0,DH_str_functs);
		ERR_load_strings(0,DH_str_reasons);
		}
#endif
	}
