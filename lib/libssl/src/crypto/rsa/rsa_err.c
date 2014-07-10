/* $OpenBSD: rsa_err.c,v 1.15 2014/07/10 22:45:57 jsing Exp $ */
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

#include <openssl/opensslconf.h>

#include <openssl/err.h>
#include <openssl/rsa.h>

/* BEGIN ERROR CODES */
#ifndef OPENSSL_NO_ERR

#define ERR_FUNC(func) ERR_PACK(ERR_LIB_RSA,func,0)
#define ERR_REASON(reason) ERR_PACK(ERR_LIB_RSA,0,reason)

static ERR_STRING_DATA RSA_str_functs[] = {
	{ERR_FUNC(RSA_F_CHECK_PADDING_MD),	"CHECK_PADDING_MD"},
	{ERR_FUNC(RSA_F_DO_RSA_PRINT),	"DO_RSA_PRINT"},
	{ERR_FUNC(RSA_F_INT_RSA_VERIFY),	"INT_RSA_VERIFY"},
	{ERR_FUNC(RSA_F_MEMORY_LOCK),	"MEMORY_LOCK"},
	{ERR_FUNC(RSA_F_OLD_RSA_PRIV_DECODE),	"OLD_RSA_PRIV_DECODE"},
	{ERR_FUNC(RSA_F_PKEY_RSA_CTRL),	"PKEY_RSA_CTRL"},
	{ERR_FUNC(RSA_F_PKEY_RSA_CTRL_STR),	"PKEY_RSA_CTRL_STR"},
	{ERR_FUNC(RSA_F_PKEY_RSA_SIGN),	"PKEY_RSA_SIGN"},
	{ERR_FUNC(RSA_F_PKEY_RSA_VERIFY),	"PKEY_RSA_VERIFY"},
	{ERR_FUNC(RSA_F_PKEY_RSA_VERIFYRECOVER),	"PKEY_RSA_VERIFYRECOVER"},
	{ERR_FUNC(RSA_F_RSA_BUILTIN_KEYGEN),	"RSA_BUILTIN_KEYGEN"},
	{ERR_FUNC(RSA_F_RSA_CHECK_KEY),	"RSA_check_key"},
	{ERR_FUNC(RSA_F_RSA_EAY_PRIVATE_DECRYPT),	"RSA_EAY_PRIVATE_DECRYPT"},
	{ERR_FUNC(RSA_F_RSA_EAY_PRIVATE_ENCRYPT),	"RSA_EAY_PRIVATE_ENCRYPT"},
	{ERR_FUNC(RSA_F_RSA_EAY_PUBLIC_DECRYPT),	"RSA_EAY_PUBLIC_DECRYPT"},
	{ERR_FUNC(RSA_F_RSA_EAY_PUBLIC_ENCRYPT),	"RSA_EAY_PUBLIC_ENCRYPT"},
	{ERR_FUNC(RSA_F_RSA_GENERATE_KEY),	"RSA_generate_key"},
	{ERR_FUNC(RSA_F_RSA_GENERATE_KEY_EX),	"RSA_generate_key_ex"},
	{ERR_FUNC(RSA_F_RSA_ITEM_VERIFY),	"RSA_ITEM_VERIFY"},
	{ERR_FUNC(RSA_F_RSA_MEMORY_LOCK),	"RSA_memory_lock"},
	{ERR_FUNC(RSA_F_RSA_NEW_METHOD),	"RSA_new_method"},
	{ERR_FUNC(RSA_F_RSA_PADDING_ADD_NONE),	"RSA_padding_add_none"},
	{ERR_FUNC(RSA_F_RSA_PADDING_ADD_PKCS1_OAEP),	"RSA_padding_add_PKCS1_OAEP"},
	{ERR_FUNC(RSA_F_RSA_PADDING_ADD_PKCS1_PSS),	"RSA_padding_add_PKCS1_PSS"},
	{ERR_FUNC(RSA_F_RSA_PADDING_ADD_PKCS1_PSS_MGF1),	"RSA_padding_add_PKCS1_PSS_mgf1"},
	{ERR_FUNC(RSA_F_RSA_PADDING_ADD_PKCS1_TYPE_1),	"RSA_padding_add_PKCS1_type_1"},
	{ERR_FUNC(RSA_F_RSA_PADDING_ADD_PKCS1_TYPE_2),	"RSA_padding_add_PKCS1_type_2"},
	{ERR_FUNC(RSA_F_RSA_PADDING_ADD_SSLV23),	"RSA_padding_add_SSLv23"},
	{ERR_FUNC(RSA_F_RSA_PADDING_ADD_X931),	"RSA_padding_add_X931"},
	{ERR_FUNC(RSA_F_RSA_PADDING_CHECK_NONE),	"RSA_padding_check_none"},
	{ERR_FUNC(RSA_F_RSA_PADDING_CHECK_PKCS1_OAEP),	"RSA_padding_check_PKCS1_OAEP"},
	{ERR_FUNC(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_1),	"RSA_padding_check_PKCS1_type_1"},
	{ERR_FUNC(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_2),	"RSA_padding_check_PKCS1_type_2"},
	{ERR_FUNC(RSA_F_RSA_PADDING_CHECK_SSLV23),	"RSA_padding_check_SSLv23"},
	{ERR_FUNC(RSA_F_RSA_PADDING_CHECK_X931),	"RSA_padding_check_X931"},
	{ERR_FUNC(RSA_F_RSA_PRINT),	"RSA_print"},
	{ERR_FUNC(RSA_F_RSA_PRINT_FP),	"RSA_print_fp"},
	{ERR_FUNC(RSA_F_RSA_PRIVATE_DECRYPT),	"RSA_private_decrypt"},
	{ERR_FUNC(RSA_F_RSA_PRIVATE_ENCRYPT),	"RSA_private_encrypt"},
	{ERR_FUNC(RSA_F_RSA_PRIV_DECODE),	"RSA_PRIV_DECODE"},
	{ERR_FUNC(RSA_F_RSA_PRIV_ENCODE),	"RSA_PRIV_ENCODE"},
	{ERR_FUNC(RSA_F_RSA_PUBLIC_DECRYPT),	"RSA_public_decrypt"},
	{ERR_FUNC(RSA_F_RSA_PUBLIC_ENCRYPT),	"RSA_public_encrypt"},
	{ERR_FUNC(RSA_F_RSA_PUB_DECODE),	"RSA_PUB_DECODE"},
	{ERR_FUNC(RSA_F_RSA_SETUP_BLINDING),	"RSA_setup_blinding"},
	{ERR_FUNC(RSA_F_RSA_SIGN),	"RSA_sign"},
	{ERR_FUNC(RSA_F_RSA_SIGN_ASN1_OCTET_STRING),	"RSA_sign_ASN1_OCTET_STRING"},
	{ERR_FUNC(RSA_F_RSA_VERIFY),	"RSA_verify"},
	{ERR_FUNC(RSA_F_RSA_VERIFY_ASN1_OCTET_STRING),	"RSA_verify_ASN1_OCTET_STRING"},
	{ERR_FUNC(RSA_F_RSA_VERIFY_PKCS1_PSS),	"RSA_verify_PKCS1_PSS"},
	{ERR_FUNC(RSA_F_RSA_VERIFY_PKCS1_PSS_MGF1),	"RSA_verify_PKCS1_PSS_mgf1"},
	{0, NULL}
};

static ERR_STRING_DATA RSA_str_reasons[] = {
	{ERR_REASON(RSA_R_ALGORITHM_MISMATCH)    , "algorithm mismatch"},
	{ERR_REASON(RSA_R_BAD_E_VALUE)           , "bad e value"},
	{ERR_REASON(RSA_R_BAD_FIXED_HEADER_DECRYPT), "bad fixed header decrypt"},
	{ERR_REASON(RSA_R_BAD_PAD_BYTE_COUNT)    , "bad pad byte count"},
	{ERR_REASON(RSA_R_BAD_SIGNATURE)         , "bad signature"},
	{ERR_REASON(RSA_R_BLOCK_TYPE_IS_NOT_01)  , "block type is not 01"},
	{ERR_REASON(RSA_R_BLOCK_TYPE_IS_NOT_02)  , "block type is not 02"},
	{ERR_REASON(RSA_R_DATA_GREATER_THAN_MOD_LEN), "data greater than mod len"},
	{ERR_REASON(RSA_R_DATA_TOO_LARGE)        , "data too large"},
	{ERR_REASON(RSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE), "data too large for key size"},
	{ERR_REASON(RSA_R_DATA_TOO_LARGE_FOR_MODULUS), "data too large for modulus"},
	{ERR_REASON(RSA_R_DATA_TOO_SMALL)        , "data too small"},
	{ERR_REASON(RSA_R_DATA_TOO_SMALL_FOR_KEY_SIZE), "data too small for key size"},
	{ERR_REASON(RSA_R_DIGEST_TOO_BIG_FOR_RSA_KEY), "digest too big for rsa key"},
	{ERR_REASON(RSA_R_DMP1_NOT_CONGRUENT_TO_D), "dmp1 not congruent to d"},
	{ERR_REASON(RSA_R_DMQ1_NOT_CONGRUENT_TO_D), "dmq1 not congruent to d"},
	{ERR_REASON(RSA_R_D_E_NOT_CONGRUENT_TO_1), "d e not congruent to 1"},
	{ERR_REASON(RSA_R_FIRST_OCTET_INVALID)   , "first octet invalid"},
	{ERR_REASON(RSA_R_ILLEGAL_OR_UNSUPPORTED_PADDING_MODE), "illegal or unsupported padding mode"},
	{ERR_REASON(RSA_R_INVALID_DIGEST_LENGTH) , "invalid digest length"},
	{ERR_REASON(RSA_R_INVALID_HEADER)        , "invalid header"},
	{ERR_REASON(RSA_R_INVALID_KEYBITS)       , "invalid keybits"},
	{ERR_REASON(RSA_R_INVALID_MESSAGE_LENGTH), "invalid message length"},
	{ERR_REASON(RSA_R_INVALID_MGF1_MD)       , "invalid mgf1 md"},
	{ERR_REASON(RSA_R_INVALID_PADDING)       , "invalid padding"},
	{ERR_REASON(RSA_R_INVALID_PADDING_MODE)  , "invalid padding mode"},
	{ERR_REASON(RSA_R_INVALID_PSS_PARAMETERS), "invalid pss parameters"},
	{ERR_REASON(RSA_R_INVALID_PSS_SALTLEN)   , "invalid pss saltlen"},
	{ERR_REASON(RSA_R_INVALID_SALT_LENGTH)   , "invalid salt length"},
	{ERR_REASON(RSA_R_INVALID_TRAILER)       , "invalid trailer"},
	{ERR_REASON(RSA_R_INVALID_X931_DIGEST)   , "invalid x931 digest"},
	{ERR_REASON(RSA_R_IQMP_NOT_INVERSE_OF_Q) , "iqmp not inverse of q"},
	{ERR_REASON(RSA_R_KEY_SIZE_TOO_SMALL)    , "key size too small"},
	{ERR_REASON(RSA_R_LAST_OCTET_INVALID)    , "last octet invalid"},
	{ERR_REASON(RSA_R_MODULUS_TOO_LARGE)     , "modulus too large"},
	{ERR_REASON(RSA_R_NON_FIPS_RSA_METHOD)   , "non fips rsa method"},
	{ERR_REASON(RSA_R_NO_PUBLIC_EXPONENT)    , "no public exponent"},
	{ERR_REASON(RSA_R_NULL_BEFORE_BLOCK_MISSING), "null before block missing"},
	{ERR_REASON(RSA_R_N_DOES_NOT_EQUAL_P_Q)  , "n does not equal p q"},
	{ERR_REASON(RSA_R_OAEP_DECODING_ERROR)   , "oaep decoding error"},
	{ERR_REASON(RSA_R_OPERATION_NOT_ALLOWED_IN_FIPS_MODE), "operation not allowed in fips mode"},
	{ERR_REASON(RSA_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE), "operation not supported for this keytype"},
	{ERR_REASON(RSA_R_PADDING_CHECK_FAILED)  , "padding check failed"},
	{ERR_REASON(RSA_R_P_NOT_PRIME)           , "p not prime"},
	{ERR_REASON(RSA_R_Q_NOT_PRIME)           , "q not prime"},
	{ERR_REASON(RSA_R_RSA_OPERATIONS_NOT_SUPPORTED), "rsa operations not supported"},
	{ERR_REASON(RSA_R_SLEN_CHECK_FAILED)     , "salt length check failed"},
	{ERR_REASON(RSA_R_SLEN_RECOVERY_FAILED)  , "salt length recovery failed"},
	{ERR_REASON(RSA_R_SSLV3_ROLLBACK_ATTACK) , "sslv3 rollback attack"},
	{ERR_REASON(RSA_R_THE_ASN1_OBJECT_IDENTIFIER_IS_NOT_KNOWN_FOR_THIS_MD), "the asn1 object identifier is not known for this md"},
	{ERR_REASON(RSA_R_UNKNOWN_ALGORITHM_TYPE), "unknown algorithm type"},
	{ERR_REASON(RSA_R_UNKNOWN_MASK_DIGEST)   , "unknown mask digest"},
	{ERR_REASON(RSA_R_UNKNOWN_PADDING_TYPE)  , "unknown padding type"},
	{ERR_REASON(RSA_R_UNKNOWN_PSS_DIGEST)    , "unknown pss digest"},
	{ERR_REASON(RSA_R_UNSUPPORTED_MASK_ALGORITHM), "unsupported mask algorithm"},
	{ERR_REASON(RSA_R_UNSUPPORTED_MASK_PARAMETER), "unsupported mask parameter"},
	{ERR_REASON(RSA_R_UNSUPPORTED_SIGNATURE_TYPE), "unsupported signature type"},
	{ERR_REASON(RSA_R_VALUE_MISSING)         , "value missing"},
	{ERR_REASON(RSA_R_WRONG_SIGNATURE_LENGTH), "wrong signature length"},
	{0, NULL}
};

#endif

void
ERR_load_RSA_strings(void)
{
#ifndef OPENSSL_NO_ERR
	if (ERR_func_error_string(RSA_str_functs[0].error) == NULL) {
		ERR_load_strings(0, RSA_str_functs);
		ERR_load_strings(0, RSA_str_reasons);
	}
#endif
}
