/*
 * THIS FILE IS AUTOMATICALLY GENERATED.  DO NOT EDIT.
 */

/*      $OpenBSD: busop.c,v 1.1 2003/02/17 01:29:20 henric Exp $   */

/*
 * Copyright (c) 2003 Henric Jungheim
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>

#include <machine/bus.h>

/*
 * Implementing u_int16_t
 */


void
bus_space_read_multi_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    u_int16_t *a, bus_size_t c)
{
	u_int16_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bsrm2(%llx + %llx, %x, %x) ->", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));
	
	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->asi);
	while (--c > 0) {
		u_int16_t r = lduha_asi(h.bh_ptr + o);
		BUS_SPACE_TRACE(t, h, (" %4.4x", r));
		*p++ = r;
	}
	
	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_multi_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const u_int16_t *a, bus_size_t c)
{
	const u_int16_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bswm2(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->asi);
	while (--c > 0) {
		u_int16_t r = *p++;
		BUS_SPACE_TRACE(t, h, (" %4.4x", r));
		stha_asi(h.bh_ptr + o, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_multi_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int16_t v,
    bus_size_t c)
{
	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bssm2(%llx + %llx, %x, %x) <- %4.4x\n", (long long)h.bh_ptr,
	    (long long)o, t->asi, c, v));

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->asi);
	while (--c > 0)
		stha_asi(h.bh_ptr + o, v);
}

void
bus_space_read_region_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    u_int16_t *a, bus_size_t c)
{
	u_int16_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bsrr2(%llx + %llx, %x, %x) <- \n", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));

	asi_set(t->asi);
	for (; c; p++, c--, ptr += 2) {
		u_int16_t r = lduha_asi(ptr);
		BUS_SPACE_TRACE(t, h, (" %4.4x", r));
		*p = r;
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_region_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const u_int16_t *a, bus_size_t c)
{
	const u_int16_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bswr2(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));

	asi_set(t->asi);
	for (; c; p++, c--, ptr += 2) {
		u_int16_t r = *p;
		BUS_SPACE_TRACE(t, h, (" %4.4x", r));
		stha_asi(ptr, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_region_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int16_t v,
    bus_size_t c)
{
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bssr2(%llx + %llx, %x, %x) <- %4.4x\n", (long long)h.bh_ptr,
	    (long long)o, t->asi, c, v));

	asi_set(t->asi);
	for (; c; c--, ptr += 2)
		stha_asi(ptr, v);
}

void
bus_space_copy_region_2(bus_space_tag_t t, bus_space_handle_t h1,
    bus_size_t o1, bus_space_handle_t h2, bus_size_t o2,
    bus_size_t c)
{
	paddr_t ptr1 = h1.bh_ptr + o1;
	paddr_t ptr2 = h2.bh_ptr + o2;

	BUS_SPACE_ASSERT(t, h1, o2, 2);
	BUS_SPACE_ASSERT(t, h2, o2, 2);
	BUS_SPACE_TRACE(t, h1,
	    ("bscr2(%llx + %llx, %llx + %llx, %x, %x) <-> \n",
	    (long long)h1.bh_ptr, (long long)o1,
	    (long long)h2.bh_ptr, (long long)o2,
	    t->asi, c));

	asi_set(t->asi);
        for (; c; c--, ptr1 += 2, ptr2 += 2) {
		u_int16_t r = lduha_asi(ptr2);
		BUS_SPACE_TRACE(t, h1, (" %4.4x", r));
		stha_asi(ptr1, r);
	}
	BUS_SPACE_TRACE(t, h1, ("\n"));
}


/*
 * Implementing u_int32_t
 */


void
bus_space_read_multi_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    u_int32_t *a, bus_size_t c)
{
	u_int32_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bsrm4(%llx + %llx, %x, %x) ->", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));
	
	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->asi);
	while (--c > 0) {
		u_int32_t r = lduwa_asi(h.bh_ptr + o);
		BUS_SPACE_TRACE(t, h, (" %8.8x", r));
		*p++ = r;
	}
	
	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_multi_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const u_int32_t *a, bus_size_t c)
{
	const u_int32_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bswm4(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->asi);
	while (--c > 0) {
		u_int32_t r = *p++;
		BUS_SPACE_TRACE(t, h, (" %8.8x", r));
		stwa_asi(h.bh_ptr + o, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_multi_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int32_t v,
    bus_size_t c)
{
	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bssm4(%llx + %llx, %x, %x) <- %8.8x\n", (long long)h.bh_ptr,
	    (long long)o, t->asi, c, v));

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->asi);
	while (--c > 0)
		stwa_asi(h.bh_ptr + o, v);
}

void
bus_space_read_region_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    u_int32_t *a, bus_size_t c)
{
	u_int32_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bsrr4(%llx + %llx, %x, %x) <- \n", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));

	asi_set(t->asi);
	for (; c; p++, c--, ptr += 4) {
		u_int32_t r = lduwa_asi(ptr);
		BUS_SPACE_TRACE(t, h, (" %8.8x", r));
		*p = r;
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_region_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const u_int32_t *a, bus_size_t c)
{
	const u_int32_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bswr4(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));

	asi_set(t->asi);
	for (; c; p++, c--, ptr += 4) {
		u_int32_t r = *p;
		BUS_SPACE_TRACE(t, h, (" %8.8x", r));
		stwa_asi(ptr, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_region_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int32_t v,
    bus_size_t c)
{
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bssr4(%llx + %llx, %x, %x) <- %8.8x\n", (long long)h.bh_ptr,
	    (long long)o, t->asi, c, v));

	asi_set(t->asi);
	for (; c; c--, ptr += 4)
		stwa_asi(ptr, v);
}

void
bus_space_copy_region_4(bus_space_tag_t t, bus_space_handle_t h1,
    bus_size_t o1, bus_space_handle_t h2, bus_size_t o2,
    bus_size_t c)
{
	paddr_t ptr1 = h1.bh_ptr + o1;
	paddr_t ptr2 = h2.bh_ptr + o2;

	BUS_SPACE_ASSERT(t, h1, o2, 4);
	BUS_SPACE_ASSERT(t, h2, o2, 4);
	BUS_SPACE_TRACE(t, h1,
	    ("bscr4(%llx + %llx, %llx + %llx, %x, %x) <-> \n",
	    (long long)h1.bh_ptr, (long long)o1,
	    (long long)h2.bh_ptr, (long long)o2,
	    t->asi, c));

	asi_set(t->asi);
        for (; c; c--, ptr1 += 4, ptr2 += 4) {
		u_int32_t r = lduwa_asi(ptr2);
		BUS_SPACE_TRACE(t, h1, (" %8.8x", r));
		stwa_asi(ptr1, r);
	}
	BUS_SPACE_TRACE(t, h1, ("\n"));
}


/*
 * Implementing u_int64_t
 */


void
bus_space_read_multi_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    u_int64_t *a, bus_size_t c)
{
	u_int64_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bsrm8(%llx + %llx, %x, %x) ->", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));
	
	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->asi);
	while (--c > 0) {
		u_int64_t r = ldxa_asi(h.bh_ptr + o);
		BUS_SPACE_TRACE(t, h, (" %16.16llx", r));
		*p++ = r;
	}
	
	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_multi_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const u_int64_t *a, bus_size_t c)
{
	const u_int64_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bswm8(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->asi);
	while (--c > 0) {
		u_int64_t r = *p++;
		BUS_SPACE_TRACE(t, h, (" %16.16llx", r));
		stxa_asi(h.bh_ptr + o, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_multi_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int64_t v,
    bus_size_t c)
{
	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bssm8(%llx + %llx, %x, %x) <- %16.16llx\n", (long long)h.bh_ptr,
	    (long long)o, t->asi, c, v));

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->asi);
	while (--c > 0)
		stxa_asi(h.bh_ptr + o, v);
}

void
bus_space_read_region_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    u_int64_t *a, bus_size_t c)
{
	u_int64_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bsrr8(%llx + %llx, %x, %x) <- \n", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));

	asi_set(t->asi);
	for (; c; p++, c--, ptr += 8) {
		u_int64_t r = ldxa_asi(ptr);
		BUS_SPACE_TRACE(t, h, (" %16.16llx", r));
		*p = r;
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_region_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const u_int64_t *a, bus_size_t c)
{
	const u_int64_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bswr8(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));

	asi_set(t->asi);
	for (; c; p++, c--, ptr += 8) {
		u_int64_t r = *p;
		BUS_SPACE_TRACE(t, h, (" %16.16llx", r));
		stxa_asi(ptr, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_region_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int64_t v,
    bus_size_t c)
{
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bssr8(%llx + %llx, %x, %x) <- %16.16llx\n", (long long)h.bh_ptr,
	    (long long)o, t->asi, c, v));

	asi_set(t->asi);
	for (; c; c--, ptr += 8)
		stxa_asi(ptr, v);
}

void
bus_space_copy_region_8(bus_space_tag_t t, bus_space_handle_t h1,
    bus_size_t o1, bus_space_handle_t h2, bus_size_t o2,
    bus_size_t c)
{
	paddr_t ptr1 = h1.bh_ptr + o1;
	paddr_t ptr2 = h2.bh_ptr + o2;

	BUS_SPACE_ASSERT(t, h1, o2, 8);
	BUS_SPACE_ASSERT(t, h2, o2, 8);
	BUS_SPACE_TRACE(t, h1,
	    ("bscr8(%llx + %llx, %llx + %llx, %x, %x) <-> \n",
	    (long long)h1.bh_ptr, (long long)o1,
	    (long long)h2.bh_ptr, (long long)o2,
	    t->asi, c));

	asi_set(t->asi);
        for (; c; c--, ptr1 += 8, ptr2 += 8) {
		u_int64_t r = ldxa_asi(ptr2);
		BUS_SPACE_TRACE(t, h1, (" %16.16llx", r));
		stxa_asi(ptr1, r);
	}
	BUS_SPACE_TRACE(t, h1, ("\n"));
}


/*
 * Implementing u_int8_t
 */


void
bus_space_read_multi_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    u_int8_t *a, bus_size_t c)
{
	u_int8_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bsrm1(%llx + %llx, %x, %x) ->", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));
	
	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->asi);
	while (--c > 0) {
		u_int8_t r = lduba_asi(h.bh_ptr + o);
		BUS_SPACE_TRACE(t, h, (" %2.2x", r));
		*p++ = r;
	}
	
	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_multi_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const u_int8_t *a, bus_size_t c)
{
	const u_int8_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bswm1(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->asi);
	while (--c > 0) {
		u_int8_t r = *p++;
		BUS_SPACE_TRACE(t, h, (" %2.2x", r));
		stba_asi(h.bh_ptr + o, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_multi_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int8_t v,
    bus_size_t c)
{
	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bssm1(%llx + %llx, %x, %x) <- %2.2x\n", (long long)h.bh_ptr,
	    (long long)o, t->asi, c, v));

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->asi);
	while (--c > 0)
		stba_asi(h.bh_ptr + o, v);
}

void
bus_space_read_region_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    u_int8_t *a, bus_size_t c)
{
	u_int8_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bsrr1(%llx + %llx, %x, %x) <- \n", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));

	asi_set(t->asi);
	for (; c; p++, c--, ptr += 1) {
		u_int8_t r = lduba_asi(ptr);
		BUS_SPACE_TRACE(t, h, (" %2.2x", r));
		*p = r;
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_region_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const u_int8_t *a, bus_size_t c)
{
	const u_int8_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bswr1(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->asi, c));

	asi_set(t->asi);
	for (; c; p++, c--, ptr += 1) {
		u_int8_t r = *p;
		BUS_SPACE_TRACE(t, h, (" %2.2x", r));
		stba_asi(ptr, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_region_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int8_t v,
    bus_size_t c)
{
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bssr1(%llx + %llx, %x, %x) <- %2.2x\n", (long long)h.bh_ptr,
	    (long long)o, t->asi, c, v));

	asi_set(t->asi);
	for (; c; c--, ptr += 1)
		stba_asi(ptr, v);
}

void
bus_space_copy_region_1(bus_space_tag_t t, bus_space_handle_t h1,
    bus_size_t o1, bus_space_handle_t h2, bus_size_t o2,
    bus_size_t c)
{
	paddr_t ptr1 = h1.bh_ptr + o1;
	paddr_t ptr2 = h2.bh_ptr + o2;

	BUS_SPACE_ASSERT(t, h1, o2, 1);
	BUS_SPACE_ASSERT(t, h2, o2, 1);
	BUS_SPACE_TRACE(t, h1,
	    ("bscr1(%llx + %llx, %llx + %llx, %x, %x) <-> \n",
	    (long long)h1.bh_ptr, (long long)o1,
	    (long long)h2.bh_ptr, (long long)o2,
	    t->asi, c));

	asi_set(t->asi);
        for (; c; c--, ptr1 += 1, ptr2 += 1) {
		u_int8_t r = lduba_asi(ptr2);
		BUS_SPACE_TRACE(t, h1, (" %2.2x", r));
		stba_asi(ptr1, r);
	}
	BUS_SPACE_TRACE(t, h1, ("\n"));
}


/*
 * Implementing u_int16_t
 */


void
bus_space_read_raw_multi_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    void *a, size_t c)
{
	u_int16_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bsrm2(%llx + %llx, %x, %x) ->", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int16_t);
	
	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->sasi);
	while (--c > 0) {
		u_int16_t r = lduha_asi(h.bh_ptr + o);
		BUS_SPACE_TRACE(t, h, (" %4.4x", r));
		*p++ = r;
	}
	
	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_raw_multi_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const void *a, size_t c)
{
	const u_int16_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bswm2(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int16_t);

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->sasi);
	while (--c > 0) {
		u_int16_t r = *p++;
		BUS_SPACE_TRACE(t, h, (" %4.4x", r));
		stha_asi(h.bh_ptr + o, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_raw_multi_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int16_t v,
    size_t c)
{
	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bssm2(%llx + %llx, %x, %x) <- %4.4x\n", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c, v));
	c /= sizeof(u_int16_t);

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->sasi);
	while (--c > 0)
		stha_asi(h.bh_ptr + o, v);
}

void
bus_space_read_raw_region_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    void *a, size_t c)
{
	u_int16_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bsrr2(%llx + %llx, %x, %x) <- \n", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int16_t);

	asi_set(t->sasi);
	for (; c; p++, c--, ptr += 2) {
		u_int16_t r = lduha_asi(ptr);
		BUS_SPACE_TRACE(t, h, (" %4.4x", r));
		*p = r;
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_raw_region_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const void *a, size_t c)
{
	const u_int16_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bswr2(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int16_t);

	asi_set(t->sasi);
	for (; c; p++, c--, ptr += 2) {
		u_int16_t r = *p;
		BUS_SPACE_TRACE(t, h, (" %4.4x", r));
		stha_asi(ptr, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_raw_region_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int16_t v,
    size_t c)
{
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bssr2(%llx + %llx, %x, %x) <- %4.4x\n", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c, v));
	c /= sizeof(u_int16_t);

	asi_set(t->sasi);
	for (; c; c--, ptr += 2)
		stha_asi(ptr, v);
}

void
bus_space_copy_raw_region_2(bus_space_tag_t t, bus_space_handle_t h1,
    bus_size_t o1, bus_space_handle_t h2, bus_size_t o2,
    size_t c)
{
	paddr_t ptr1 = h1.bh_ptr + o1;
	paddr_t ptr2 = h2.bh_ptr + o2;

	BUS_SPACE_ASSERT(t, h1, o2, 2);
	BUS_SPACE_ASSERT(t, h2, o2, 2);
	BUS_SPACE_TRACE(t, h1,
	    ("bscr2(%llx + %llx, %llx + %llx, %x, %x) <-> \n",
	    (long long)h1.bh_ptr, (long long)o1,
	    (long long)h2.bh_ptr, (long long)o2,
	    t->sasi, c));
	c /= sizeof(u_int16_t);

	asi_set(t->sasi);
        for (; c; c--, ptr1 += 2, ptr2 += 2) {
		u_int16_t r = lduha_asi(ptr2);
		BUS_SPACE_TRACE(t, h1, (" %4.4x", r));
		stha_asi(ptr1, r);
	}
	BUS_SPACE_TRACE(t, h1, ("\n"));
}


/*
 * Implementing u_int32_t
 */


void
bus_space_read_raw_multi_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    void *a, size_t c)
{
	u_int32_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bsrm4(%llx + %llx, %x, %x) ->", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int32_t);
	
	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->sasi);
	while (--c > 0) {
		u_int32_t r = lduwa_asi(h.bh_ptr + o);
		BUS_SPACE_TRACE(t, h, (" %8.8x", r));
		*p++ = r;
	}
	
	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_raw_multi_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const void *a, size_t c)
{
	const u_int32_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bswm4(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int32_t);

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->sasi);
	while (--c > 0) {
		u_int32_t r = *p++;
		BUS_SPACE_TRACE(t, h, (" %8.8x", r));
		stwa_asi(h.bh_ptr + o, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_raw_multi_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int32_t v,
    size_t c)
{
	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bssm4(%llx + %llx, %x, %x) <- %8.8x\n", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c, v));
	c /= sizeof(u_int32_t);

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->sasi);
	while (--c > 0)
		stwa_asi(h.bh_ptr + o, v);
}

void
bus_space_read_raw_region_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    void *a, size_t c)
{
	u_int32_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bsrr4(%llx + %llx, %x, %x) <- \n", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int32_t);

	asi_set(t->sasi);
	for (; c; p++, c--, ptr += 4) {
		u_int32_t r = lduwa_asi(ptr);
		BUS_SPACE_TRACE(t, h, (" %8.8x", r));
		*p = r;
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_raw_region_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const void *a, size_t c)
{
	const u_int32_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bswr4(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int32_t);

	asi_set(t->sasi);
	for (; c; p++, c--, ptr += 4) {
		u_int32_t r = *p;
		BUS_SPACE_TRACE(t, h, (" %8.8x", r));
		stwa_asi(ptr, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_raw_region_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int32_t v,
    size_t c)
{
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bssr4(%llx + %llx, %x, %x) <- %8.8x\n", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c, v));
	c /= sizeof(u_int32_t);

	asi_set(t->sasi);
	for (; c; c--, ptr += 4)
		stwa_asi(ptr, v);
}

void
bus_space_copy_raw_region_4(bus_space_tag_t t, bus_space_handle_t h1,
    bus_size_t o1, bus_space_handle_t h2, bus_size_t o2,
    size_t c)
{
	paddr_t ptr1 = h1.bh_ptr + o1;
	paddr_t ptr2 = h2.bh_ptr + o2;

	BUS_SPACE_ASSERT(t, h1, o2, 4);
	BUS_SPACE_ASSERT(t, h2, o2, 4);
	BUS_SPACE_TRACE(t, h1,
	    ("bscr4(%llx + %llx, %llx + %llx, %x, %x) <-> \n",
	    (long long)h1.bh_ptr, (long long)o1,
	    (long long)h2.bh_ptr, (long long)o2,
	    t->sasi, c));
	c /= sizeof(u_int32_t);

	asi_set(t->sasi);
        for (; c; c--, ptr1 += 4, ptr2 += 4) {
		u_int32_t r = lduwa_asi(ptr2);
		BUS_SPACE_TRACE(t, h1, (" %8.8x", r));
		stwa_asi(ptr1, r);
	}
	BUS_SPACE_TRACE(t, h1, ("\n"));
}


/*
 * Implementing u_int64_t
 */


void
bus_space_read_raw_multi_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    void *a, size_t c)
{
	u_int64_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bsrm8(%llx + %llx, %x, %x) ->", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int64_t);
	
	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->sasi);
	while (--c > 0) {
		u_int64_t r = ldxa_asi(h.bh_ptr + o);
		BUS_SPACE_TRACE(t, h, (" %16.16llx", r));
		*p++ = r;
	}
	
	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_raw_multi_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const void *a, size_t c)
{
	const u_int64_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bswm8(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int64_t);

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->sasi);
	while (--c > 0) {
		u_int64_t r = *p++;
		BUS_SPACE_TRACE(t, h, (" %16.16llx", r));
		stxa_asi(h.bh_ptr + o, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_raw_multi_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int64_t v,
    size_t c)
{
	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bssm8(%llx + %llx, %x, %x) <- %16.16llx\n", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c, v));
	c /= sizeof(u_int64_t);

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->sasi);
	while (--c > 0)
		stxa_asi(h.bh_ptr + o, v);
}

void
bus_space_read_raw_region_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    void *a, size_t c)
{
	u_int64_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bsrr8(%llx + %llx, %x, %x) <- \n", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int64_t);

	asi_set(t->sasi);
	for (; c; p++, c--, ptr += 8) {
		u_int64_t r = ldxa_asi(ptr);
		BUS_SPACE_TRACE(t, h, (" %16.16llx", r));
		*p = r;
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_raw_region_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const void *a, size_t c)
{
	const u_int64_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bswr8(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int64_t);

	asi_set(t->sasi);
	for (; c; p++, c--, ptr += 8) {
		u_int64_t r = *p;
		BUS_SPACE_TRACE(t, h, (" %16.16llx", r));
		stxa_asi(ptr, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_raw_region_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int64_t v,
    size_t c)
{
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bssr8(%llx + %llx, %x, %x) <- %16.16llx\n", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c, v));
	c /= sizeof(u_int64_t);

	asi_set(t->sasi);
	for (; c; c--, ptr += 8)
		stxa_asi(ptr, v);
}

void
bus_space_copy_raw_region_8(bus_space_tag_t t, bus_space_handle_t h1,
    bus_size_t o1, bus_space_handle_t h2, bus_size_t o2,
    size_t c)
{
	paddr_t ptr1 = h1.bh_ptr + o1;
	paddr_t ptr2 = h2.bh_ptr + o2;

	BUS_SPACE_ASSERT(t, h1, o2, 8);
	BUS_SPACE_ASSERT(t, h2, o2, 8);
	BUS_SPACE_TRACE(t, h1,
	    ("bscr8(%llx + %llx, %llx + %llx, %x, %x) <-> \n",
	    (long long)h1.bh_ptr, (long long)o1,
	    (long long)h2.bh_ptr, (long long)o2,
	    t->sasi, c));
	c /= sizeof(u_int64_t);

	asi_set(t->sasi);
        for (; c; c--, ptr1 += 8, ptr2 += 8) {
		u_int64_t r = ldxa_asi(ptr2);
		BUS_SPACE_TRACE(t, h1, (" %16.16llx", r));
		stxa_asi(ptr1, r);
	}
	BUS_SPACE_TRACE(t, h1, ("\n"));
}


/*
 * Implementing u_int8_t
 */


void
bus_space_read_raw_multi_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    void *a, size_t c)
{
	u_int8_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bsrm1(%llx + %llx, %x, %x) ->", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int8_t);
	
	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->sasi);
	while (--c > 0) {
		u_int8_t r = lduba_asi(h.bh_ptr + o);
		BUS_SPACE_TRACE(t, h, (" %2.2x", r));
		*p++ = r;
	}
	
	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_raw_multi_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const void *a, size_t c)
{
	const u_int8_t *p = a;

	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bswm1(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int8_t);

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->sasi);
	while (--c > 0) {
		u_int8_t r = *p++;
		BUS_SPACE_TRACE(t, h, (" %2.2x", r));
		stba_asi(h.bh_ptr + o, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_raw_multi_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int8_t v,
    size_t c)
{
	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bssm1(%llx + %llx, %x, %x) <- %2.2x\n", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c, v));
	c /= sizeof(u_int8_t);

	++c;  /* Looping on "--c" is slightly faster than on "c--" */
	asi_set(t->sasi);
	while (--c > 0)
		stba_asi(h.bh_ptr + o, v);
}

void
bus_space_read_raw_region_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    void *a, size_t c)
{
	u_int8_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bsrr1(%llx + %llx, %x, %x) <- \n", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int8_t);

	asi_set(t->sasi);
	for (; c; p++, c--, ptr += 1) {
		u_int8_t r = lduba_asi(ptr);
		BUS_SPACE_TRACE(t, h, (" %2.2x", r));
		*p = r;
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_write_raw_region_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o,
    const void *a, size_t c)
{
	const u_int8_t *p = a;
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bswr1(%llx + %llx, %x, %x) <-", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c));
	c /= sizeof(u_int8_t);

	asi_set(t->sasi);
	for (; c; p++, c--, ptr += 1) {
		u_int8_t r = *p;
		BUS_SPACE_TRACE(t, h, (" %2.2x", r));
		stba_asi(ptr, r);
	}

	BUS_SPACE_TRACE(t, h, ("\n"));
}

void
bus_space_set_raw_region_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int8_t v,
    size_t c)
{
	paddr_t ptr = h.bh_ptr + o;

	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bssr1(%llx + %llx, %x, %x) <- %2.2x\n", (long long)h.bh_ptr,
	    (long long)o, t->sasi, c, v));
	c /= sizeof(u_int8_t);

	asi_set(t->sasi);
	for (; c; c--, ptr += 1)
		stba_asi(ptr, v);
}

void
bus_space_copy_raw_region_1(bus_space_tag_t t, bus_space_handle_t h1,
    bus_size_t o1, bus_space_handle_t h2, bus_size_t o2,
    size_t c)
{
	paddr_t ptr1 = h1.bh_ptr + o1;
	paddr_t ptr2 = h2.bh_ptr + o2;

	BUS_SPACE_ASSERT(t, h1, o2, 1);
	BUS_SPACE_ASSERT(t, h2, o2, 1);
	BUS_SPACE_TRACE(t, h1,
	    ("bscr1(%llx + %llx, %llx + %llx, %x, %x) <-> \n",
	    (long long)h1.bh_ptr, (long long)o1,
	    (long long)h2.bh_ptr, (long long)o2,
	    t->sasi, c));
	c /= sizeof(u_int8_t);

	asi_set(t->sasi);
        for (; c; c--, ptr1 += 1, ptr2 += 1) {
		u_int8_t r = lduba_asi(ptr2);
		BUS_SPACE_TRACE(t, h1, (" %2.2x", r));
		stba_asi(ptr1, r);
	}
	BUS_SPACE_TRACE(t, h1, ("\n"));
}


