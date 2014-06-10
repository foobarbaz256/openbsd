/* crypto/engine/tb_ecdh.c */
/* ====================================================================
 * Copyright 2002 Sun Microsystems, Inc. ALL RIGHTS RESERVED.
 *
 * The Elliptic Curve Public-Key Crypto Library (ECC Code) included
 * herein is developed by SUN MICROSYSTEMS, INC., and is contributed
 * to the OpenSSL project.
 *
 * The ECC Code is licensed pursuant to the OpenSSL open source
 * license provided below.
 *
 * The ECDH engine software is originally written by Nils Gura and
 * Douglas Stebila of Sun Microsystems Laboratories.
 *
 */
/* ====================================================================
 * Copyright (c) 2000-2002 The OpenSSL Project.  All rights reserved.
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
 *    licensing@OpenSSL.org.
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

#include "eng_int.h"

/* If this symbol is defined then ENGINE_get_default_ECDH(), the function that is
 * used by ECDH to hook in implementation code and cache defaults (etc), will
 * display brief debugging summaries to stderr with the 'nid'. */
/* #define ENGINE_ECDH_DEBUG */

static ENGINE_TABLE *ecdh_table = NULL;
static const int dummy_nid = 1;

void
ENGINE_unregister_ECDH(ENGINE *e)
{
	engine_table_unregister(&ecdh_table, e);
}

static void
engine_unregister_all_ECDH(void)
{
	engine_table_cleanup(&ecdh_table);
}

int
ENGINE_register_ECDH(ENGINE *e)
{
	if (e->ecdh_meth)
		return engine_table_register(&ecdh_table,
		    engine_unregister_all_ECDH, e, &dummy_nid, 1, 0);
	return 1;
}

void
ENGINE_register_all_ECDH(void)
{
	ENGINE *e;

	for (e = ENGINE_get_first(); e; e = ENGINE_get_next(e))
		ENGINE_register_ECDH(e);
}

int
ENGINE_set_default_ECDH(ENGINE *e)
{
	if (e->ecdh_meth)
		return engine_table_register(&ecdh_table,
		    engine_unregister_all_ECDH, e, &dummy_nid, 1, 1);
	return 1;
}

/* Exposed API function to get a functional reference from the implementation
 * table (ie. try to get a functional reference from the tabled structural
 * references). */
ENGINE *
ENGINE_get_default_ECDH(void)
{
	return engine_table_select(&ecdh_table, dummy_nid);
}

/* Obtains an ECDH implementation from an ENGINE functional reference */
const ECDH_METHOD *
ENGINE_get_ECDH(const ENGINE *e)
{
	return e->ecdh_meth;
}

/* Sets an ECDH implementation in an ENGINE structure */
int
ENGINE_set_ECDH(ENGINE *e, const ECDH_METHOD *ecdh_meth)
{
	e->ecdh_meth = ecdh_meth;
	return 1;
}
