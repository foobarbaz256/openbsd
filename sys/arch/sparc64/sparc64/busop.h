/*
 * THIS FILE IS AUTOMATICALLY GENERATED.  DO NOT EDIT.
 */

/*      $OpenBSD: busop.h,v 1.1 2003/02/17 01:29:20 henric Exp $   */

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

/*
 * Implementing u_int16_t
 */


static inline u_int16_t bus_space_read_2(bus_space_tag_t,
    bus_space_handle_t, bus_size_t);
static inline void bus_space_write_2(bus_space_tag_t,
    bus_space_handle_t, bus_size_t, u_int16_t);
void bus_space_read_multi_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    u_int16_t *, bus_size_t);
void bus_space_write_multi_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const u_int16_t *, bus_size_t);
void bus_space_set_multi_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int16_t,
    bus_size_t);
void bus_space_read_region_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    u_int16_t *, bus_size_t);
void bus_space_write_region_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const u_int16_t *, bus_size_t);
void bus_space_set_region_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int16_t,
    bus_size_t);
void bus_space_copy_region_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, bus_space_handle_t, bus_size_t,
    bus_size_t);

static inline
u_int16_t bus_space_read_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o)
{
	u_int16_t r;

	BUS_SPACE_ASSERT(t, h, o, 2);
	r = lduha(h.bh_ptr + o, t->asi);
	BUS_SPACE_TRACE(t, h,
	    ("bsr2(%llx + %llx, %x) -> %4.4x\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->asi,
	    r));
	return (r);
}

static inline
void bus_space_write_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int16_t v)
{
	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bsw2(%llx + %llx, %x) <- %4.4x\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->asi,
	    v));
	stha(h.bh_ptr + o, t->asi, v);
}


/*
 * Implementing u_int32_t
 */


static inline u_int32_t bus_space_read_4(bus_space_tag_t,
    bus_space_handle_t, bus_size_t);
static inline void bus_space_write_4(bus_space_tag_t,
    bus_space_handle_t, bus_size_t, u_int32_t);
void bus_space_read_multi_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    u_int32_t *, bus_size_t);
void bus_space_write_multi_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const u_int32_t *, bus_size_t);
void bus_space_set_multi_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int32_t,
    bus_size_t);
void bus_space_read_region_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    u_int32_t *, bus_size_t);
void bus_space_write_region_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const u_int32_t *, bus_size_t);
void bus_space_set_region_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int32_t,
    bus_size_t);
void bus_space_copy_region_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, bus_space_handle_t, bus_size_t,
    bus_size_t);

static inline
u_int32_t bus_space_read_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o)
{
	u_int32_t r;

	BUS_SPACE_ASSERT(t, h, o, 4);
	r = lduwa(h.bh_ptr + o, t->asi);
	BUS_SPACE_TRACE(t, h,
	    ("bsr4(%llx + %llx, %x) -> %8.8x\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->asi,
	    r));
	return (r);
}

static inline
void bus_space_write_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int32_t v)
{
	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bsw4(%llx + %llx, %x) <- %8.8x\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->asi,
	    v));
	stwa(h.bh_ptr + o, t->asi, v);
}


/*
 * Implementing u_int64_t
 */


static inline u_int64_t bus_space_read_8(bus_space_tag_t,
    bus_space_handle_t, bus_size_t);
static inline void bus_space_write_8(bus_space_tag_t,
    bus_space_handle_t, bus_size_t, u_int64_t);
void bus_space_read_multi_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    u_int64_t *, bus_size_t);
void bus_space_write_multi_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const u_int64_t *, bus_size_t);
void bus_space_set_multi_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int64_t,
    bus_size_t);
void bus_space_read_region_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    u_int64_t *, bus_size_t);
void bus_space_write_region_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const u_int64_t *, bus_size_t);
void bus_space_set_region_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int64_t,
    bus_size_t);
void bus_space_copy_region_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, bus_space_handle_t, bus_size_t,
    bus_size_t);

static inline
u_int64_t bus_space_read_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o)
{
	u_int64_t r;

	BUS_SPACE_ASSERT(t, h, o, 8);
	r = ldxa(h.bh_ptr + o, t->asi);
	BUS_SPACE_TRACE(t, h,
	    ("bsr8(%llx + %llx, %x) -> %16.16llx\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->asi,
	    r));
	return (r);
}

static inline
void bus_space_write_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int64_t v)
{
	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bsw8(%llx + %llx, %x) <- %16.16llx\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->asi,
	    v));
	stxa(h.bh_ptr + o, t->asi, v);
}


/*
 * Implementing u_int8_t
 */


static inline u_int8_t bus_space_read_1(bus_space_tag_t,
    bus_space_handle_t, bus_size_t);
static inline void bus_space_write_1(bus_space_tag_t,
    bus_space_handle_t, bus_size_t, u_int8_t);
void bus_space_read_multi_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    u_int8_t *, bus_size_t);
void bus_space_write_multi_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const u_int8_t *, bus_size_t);
void bus_space_set_multi_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int8_t,
    bus_size_t);
void bus_space_read_region_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    u_int8_t *, bus_size_t);
void bus_space_write_region_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const u_int8_t *, bus_size_t);
void bus_space_set_region_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int8_t,
    bus_size_t);
void bus_space_copy_region_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, bus_space_handle_t, bus_size_t,
    bus_size_t);

static inline
u_int8_t bus_space_read_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o)
{
	u_int8_t r;

	BUS_SPACE_ASSERT(t, h, o, 1);
	r = lduba(h.bh_ptr + o, t->asi);
	BUS_SPACE_TRACE(t, h,
	    ("bsr1(%llx + %llx, %x) -> %2.2x\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->asi,
	    r));
	return (r);
}

static inline
void bus_space_write_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int8_t v)
{
	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bsw1(%llx + %llx, %x) <- %2.2x\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->asi,
	    v));
	stba(h.bh_ptr + o, t->asi, v);
}


/*
 * Implementing u_int16_t
 */


static inline u_int16_t bus_space_read_raw_2(bus_space_tag_t,
    bus_space_handle_t, bus_size_t);
static inline void bus_space_write_raw_2(bus_space_tag_t,
    bus_space_handle_t, bus_size_t, u_int16_t);
void bus_space_read_raw_multi_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    void *, size_t);
void bus_space_write_raw_multi_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const void *, size_t);
void bus_space_set_raw_multi_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int16_t,
    size_t);
void bus_space_read_raw_region_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    void *, size_t);
void bus_space_write_raw_region_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const void *, size_t);
void bus_space_set_raw_region_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int16_t,
    size_t);
void bus_space_copy_raw_region_2(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, bus_space_handle_t, bus_size_t,
    size_t);

static inline
u_int16_t bus_space_read_raw_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o)
{
	u_int16_t r;

	BUS_SPACE_ASSERT(t, h, o, 2);
	r = lduha(h.bh_ptr + o, t->sasi);
	BUS_SPACE_TRACE(t, h,
	    ("bsr2(%llx + %llx, %x) -> %4.4x\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->sasi,
	    r));
	return (r);
}

static inline
void bus_space_write_raw_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int16_t v)
{
	BUS_SPACE_ASSERT(t, h, o, 2);
	BUS_SPACE_TRACE(t, h,
	    ("bsw2(%llx + %llx, %x) <- %4.4x\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->sasi,
	    v));
	stha(h.bh_ptr + o, t->sasi, v);
}


/*
 * Implementing u_int32_t
 */


static inline u_int32_t bus_space_read_raw_4(bus_space_tag_t,
    bus_space_handle_t, bus_size_t);
static inline void bus_space_write_raw_4(bus_space_tag_t,
    bus_space_handle_t, bus_size_t, u_int32_t);
void bus_space_read_raw_multi_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    void *, size_t);
void bus_space_write_raw_multi_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const void *, size_t);
void bus_space_set_raw_multi_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int32_t,
    size_t);
void bus_space_read_raw_region_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    void *, size_t);
void bus_space_write_raw_region_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const void *, size_t);
void bus_space_set_raw_region_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int32_t,
    size_t);
void bus_space_copy_raw_region_4(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, bus_space_handle_t, bus_size_t,
    size_t);

static inline
u_int32_t bus_space_read_raw_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o)
{
	u_int32_t r;

	BUS_SPACE_ASSERT(t, h, o, 4);
	r = lduwa(h.bh_ptr + o, t->sasi);
	BUS_SPACE_TRACE(t, h,
	    ("bsr4(%llx + %llx, %x) -> %8.8x\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->sasi,
	    r));
	return (r);
}

static inline
void bus_space_write_raw_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int32_t v)
{
	BUS_SPACE_ASSERT(t, h, o, 4);
	BUS_SPACE_TRACE(t, h,
	    ("bsw4(%llx + %llx, %x) <- %8.8x\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->sasi,
	    v));
	stwa(h.bh_ptr + o, t->sasi, v);
}


/*
 * Implementing u_int64_t
 */


static inline u_int64_t bus_space_read_raw_8(bus_space_tag_t,
    bus_space_handle_t, bus_size_t);
static inline void bus_space_write_raw_8(bus_space_tag_t,
    bus_space_handle_t, bus_size_t, u_int64_t);
void bus_space_read_raw_multi_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    void *, size_t);
void bus_space_write_raw_multi_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const void *, size_t);
void bus_space_set_raw_multi_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int64_t,
    size_t);
void bus_space_read_raw_region_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    void *, size_t);
void bus_space_write_raw_region_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const void *, size_t);
void bus_space_set_raw_region_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int64_t,
    size_t);
void bus_space_copy_raw_region_8(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, bus_space_handle_t, bus_size_t,
    size_t);

static inline
u_int64_t bus_space_read_raw_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o)
{
	u_int64_t r;

	BUS_SPACE_ASSERT(t, h, o, 8);
	r = ldxa(h.bh_ptr + o, t->sasi);
	BUS_SPACE_TRACE(t, h,
	    ("bsr8(%llx + %llx, %x) -> %16.16llx\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->sasi,
	    r));
	return (r);
}

static inline
void bus_space_write_raw_8(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int64_t v)
{
	BUS_SPACE_ASSERT(t, h, o, 8);
	BUS_SPACE_TRACE(t, h,
	    ("bsw8(%llx + %llx, %x) <- %16.16llx\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->sasi,
	    v));
	stxa(h.bh_ptr + o, t->sasi, v);
}


/*
 * Implementing u_int8_t
 */


static inline u_int8_t bus_space_read_raw_1(bus_space_tag_t,
    bus_space_handle_t, bus_size_t);
static inline void bus_space_write_raw_1(bus_space_tag_t,
    bus_space_handle_t, bus_size_t, u_int8_t);
void bus_space_read_raw_multi_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    void *, size_t);
void bus_space_write_raw_multi_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const void *, size_t);
void bus_space_set_raw_multi_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int8_t,
    size_t);
void bus_space_read_raw_region_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    void *, size_t);
void bus_space_write_raw_region_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t,
    const void *, size_t);
void bus_space_set_raw_region_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, u_int8_t,
    size_t);
void bus_space_copy_raw_region_1(bus_space_tag_t, bus_space_handle_t,
    bus_size_t, bus_space_handle_t, bus_size_t,
    size_t);

static inline
u_int8_t bus_space_read_raw_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o)
{
	u_int8_t r;

	BUS_SPACE_ASSERT(t, h, o, 1);
	r = lduba(h.bh_ptr + o, t->sasi);
	BUS_SPACE_TRACE(t, h,
	    ("bsr1(%llx + %llx, %x) -> %2.2x\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->sasi,
	    r));
	return (r);
}

static inline
void bus_space_write_raw_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int8_t v)
{
	BUS_SPACE_ASSERT(t, h, o, 1);
	BUS_SPACE_TRACE(t, h,
	    ("bsw1(%llx + %llx, %x) <- %2.2x\n",
	    (long long)h.bh_ptr,
	    (long long)o,
	    t->sasi,
	    v));
	stba(h.bh_ptr + o, t->sasi, v);
}


