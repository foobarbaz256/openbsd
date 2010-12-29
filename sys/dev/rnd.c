/*	$OpenBSD: rnd.c,v 1.107 2010/12/29 17:51:48 deraadt Exp $	*/

/*
 * rnd.c -- A strong random number generator
 *
 * Copyright (c) 1996, 1997, 2000-2002 Michael Shalayeff.
 * Copyright (c) 2008 Damien Miller.
 * Copyright Theodore Ts'o, 1994, 1995, 1996, 1997, 1998, 1999.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * the GNU Public License, in which case the provisions of the GPL are
 * required INSTEAD OF the above restrictions.  (This clause is
 * necessary due to a potential bad interaction between the GPL and
 * the restrictions contained in a BSD-style copyright.)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * (now, with legal B.S. out of the way.....)
 *
 * This routine gathers environmental noise from device drivers, etc.,
 * and returns good random numbers, suitable for cryptographic or
 * other use.
 *
 * Theory of operation
 * ===================
 *
 * Computers are very predictable devices.  Hence it is extremely hard
 * to produce truly random numbers on a computer --- as opposed to
 * pseudo-random numbers, which can be easily generated by using an
 * algorithm.  Unfortunately, it is very easy for attackers to guess
 * the sequence of pseudo-random number generators, and for some
 * applications this is not acceptable.  Instead, we must try to
 * gather "environmental noise" from the computer's environment, which
 * must be hard for outside attackers to observe and use to
 * generate random numbers.  In a Unix environment, this is best done
 * from inside the kernel.
 *
 * Sources of randomness from the environment include inter-keyboard
 * timings, inter-interrupt timings from some interrupts, and other
 * events which are both (a) non-deterministic and (b) hard for an
 * outside observer to measure.  Randomness from these sources is
 * added to the "entropy pool", which is mixed using a CRC-like function.
 * This is not cryptographically strong, but it is adequate assuming
 * the randomness is not chosen maliciously, and it is fast enough that
 * the overhead of doing it on every interrupt is very reasonable.
 * As random bytes are mixed into the entropy pool, the routines keep
 * an *estimate* of how many bits of randomness have been stored into
 * the random number generator's internal state.
 *
 * When random bytes are desired, they are obtained by taking the MD5
 * hash of the content of the entropy pool.  The MD5 hash avoids
 * exposing the internal state of the entropy pool.  It is believed to
 * be computationally infeasible to derive any useful information
 * about the input of MD5 from its output.  Even if it is possible to
 * analyze MD5 in some clever way, as long as the amount of data
 * returned from the generator is less than the inherent entropy in
 * the pool, the output data is totally unpredictable.  For this
 * reason, the routine decreases its internal estimate of how many
 * bits of "true randomness" are contained in the entropy pool as it
 * outputs random numbers.
 *
 * If this estimate goes to zero, the routine can still generate
 * random numbers; however, an attacker may (at least in theory) be
 * able to infer the future output of the generator from prior
 * outputs.  This requires successful cryptanalysis of MD5, which is
 * believed to be not feasible, but there is a remote possibility.
 * Nonetheless, these numbers should be useful for the vast majority
 * of purposes.
 *
 * However, this MD5 output is not exported outside the subsystem.  It
 * is next used as input to seed a RC4 stream cipher.  Attempts are
 * made to follow best practice regarding this stream cipher - the first
 * chunk of output is discarded and the cipher is re-seeded from time to
 * time.  This design provides very high amounts of output data from a
 * potentially small entropy base, at high enough speeds to encourage
 * use of random numbers in nearly any situation.
 *
 * The output of this single RC4 engine is then shared amongst many
 * consumers in the kernel and userland via various interfaces:
 * arc4random_buf(), arc4random(), arc4random_uniform(), the set of
 * /dev/random nodes, and the sysctl kern.arandom.
 *
 * Exported interfaces ---- input
 * ==============================
 *
 * The current exported interfaces for gathering environmental noise
 * from the devices are:
 *
 *	void add_true_randomness(int data);
 *	void add_timer_randomness(int data);
 *	void add_mouse_randomness(int mouse_data);
 *	void add_net_randomness(int isr);
 *	void add_tty_randomness(int c);
 *	void add_disk_randomness(int n);
 *	void add_audio_randomness(int n);
 *	void add_video_randomness(int n);
 *
 * add_true_randomness() uses true random number generators present
 * on some cryptographic and system chipsets.  Entropy accounting
 * is not quitable, no timing is done, supplied 32 bits of pure entropy
 * are hashed into the pool plain and blindly, increasing the counter.
 *
 * add_timer_randomness() uses the random driver itselves timing,
 * measuring extract_entropy() and rndioctl() execution times.
 *
 * add_mouse_randomness() uses the mouse interrupt timing, as well as
 * the reported position of the mouse from the hardware.
 *
 * add_net_randomness() times the finishing time of net input.
 *
 * add_tty_randomness() uses the inter-keypress timing, as well as the
 * character as random inputs into the entropy pool.
 *
 * add_disk_randomness() times the finishing time of disk requests as well
 * as feeding both xfer size & time into the entropy pool.
 *
 * add_audio_randomness() times the finishing of audio codec dma
 * requests for both recording and playback, apparently supplies quite
 * a lot of entropy. I'd blame it on low resolution audio clock generators.
 *
 * All of these routines (except for add_true_randomness() of course)
 * try to estimate how many bits of randomness are in a particular
 * randomness source.  They do this by keeping track of the first and
 * second order deltas of the event timings.
 *
 * Acknowledgements:
 * =================
 *
 * Ideas for constructing this random number generator were derived
 * from Pretty Good Privacy's random number generator, and from private
 * discussions with Phil Karn.  Colin Plumb provided a faster random
 * number generator, which speeds up the mixing function of the entropy
 * pool, taken from PGPfone.  Dale Worley has also contributed many
 * useful ideas and suggestions to improve this driver.
 *
 * Any flaws in the design are solely my responsibility, and should
 * not be attributed to the Phil, Colin, or any of the authors of PGP.
 *
 * Further background information on this topic may be obtained from
 * RFC 1750, "Randomness Recommendations for Security", by Donald
 * Eastlake, Steve Crocker, and Jeff Schiller.
 * 
 * Using a RC4 stream cipher as 2nd stage after the MD5 output
 * is the result of work by David Mazieres.
 */

#include <sys/param.h>
#include <sys/endian.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/disk.h>
#include <sys/limits.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/fcntl.h>
#include <sys/vnode.h>
#include <sys/timeout.h>
#include <sys/poll.h>
#include <sys/mutex.h>
#include <sys/msgbuf.h>

#include <crypto/md5.h>
#include <crypto/arc4.h>

#include <dev/rndvar.h>

#ifdef	RNDEBUG
int	rnd_debug = 0x0000;
#define	RD_INPUT	0x000f	/* input data */
#define	RD_OUTPUT	0x00f0	/* output data */
#define	RD_WAIT		0x0100	/* sleep/wakeup for good data */
#endif

/*
 * Master random number pool functions
 * -----------------------------------
 */

/*
 * For the purposes of better mixing, we use the CRC-32 polynomial as
 * well to make a twisted Generalized Feedback Shift Register
 *
 * (See M. Matsumoto & Y. Kurita, 1992.  Twisted GFSR generators.  ACM
 * Transactions on Modeling and Computer Simulation 2(3):179-194.
 * Also see M. Matsumoto & Y. Kurita, 1994.  Twisted GFSR generators
 * II.  ACM Transactions on Mdeling and Computer Simulation 4:254-266)
 *
 * Thanks to Colin Plumb for suggesting this.
 *
 * We have not analyzed the resultant polynomial to prove it primitive;
 * in fact it almost certainly isn't.  Nonetheless, the irreducible factors
 * of a random large-degree polynomial over GF(2) are more than large enough
 * that periodicity is not a concern.
 *
 * The input hash is much less sensitive than the output hash.  All
 * we want from it is to be a good non-cryptographic hash -
 * i.e. to not produce collisions when fed "random" data of the sort
 * we expect to see.  As long as the pool state differs for different
 * inputs, we have preserved the input entropy and done a good job.
 * The fact that an intelligent attacker can construct inputs that
 * will produce controlled alterations to the pool's state is not
 * important because we don't consider such inputs to contribute any
 * randomness.  The only property we need with respect to them is that
 * the attacker can't increase his/her knowledge of the pool's state.
 * Since all additions are reversible (knowing the final state and the
 * input, you can reconstruct the initial state), if an attacker has
 * any uncertainty about the initial state, he/she can only shuffle
 * that uncertainty about, but never cause any collisions (which would
 * decrease the uncertainty).
 *
 * The chosen system lets the state of the pool be (essentially) the input
 * modulo the generator polynomial.  Now, for random primitive polynomials,
 * this is a universal class of hash functions, meaning that the chance
 * of a collision is limited by the attacker's knowledge of the generator
 * polynomial, so if it is chosen at random, an attacker can never force
 * a collision.  Here, we use a fixed polynomial, but we *can* assume that
 * ###--> it is unknown to the processes generating the input entropy. <-###
 * Because of this important property, this is a good, collision-resistant
 * hash; hash collisions will occur no more often than chance.
 */

/*
 * Stirring polynomials over GF(2) for various pool sizes. Used in
 * add_entropy_words() below.
 * 
 * The polynomial terms are chosen to be evenly spaced (minimum RMS
 * distance from evenly spaced; except for the last tap, which is 1 to
 * get the twisting happening as fast as possible.
 *
 * The reultant polynomial is:
 *   2^POOLWORDS + 2^POOL_TAP1 + 2^POOL_TAP2 + 2^POOL_TAP3 + 2^POOL_TAP4 + 1
 */
#define POOLBITS	(POOLWORDS*32)
#define POOLBYTES	(POOLWORDS*4)
#define POOLMASK	(POOLWORDS - 1)
#if POOLWORDS == 2048
#define	POOL_TAP1	1638
#define	POOL_TAP2	1231
#define	POOL_TAP3	819
#define	POOL_TAP4	411
#elif POOLWORDS == 1024	/* also (819, 616, 410, 207, 2) */
#define	POOL_TAP1	817
#define	POOL_TAP2	615
#define	POOL_TAP3	412
#define	POOL_TAP4	204
#elif POOLWORDS == 512	/* also (409,307,206,102,2), (409,309,205,103,2) */
#define	POOL_TAP1	411
#define	POOL_TAP2	308
#define	POOL_TAP3	208
#define	POOL_TAP4	104
#elif POOLWORDS == 256
#define	POOL_TAP1	205
#define	POOL_TAP2	155
#define	POOL_TAP3	101
#define	POOL_TAP4	52
#elif POOLWORDS == 128	/* also (103, 78, 51, 27, 2) */
#define	POOL_TAP1	103
#define	POOL_TAP2	76
#define	POOL_TAP3	51
#define	POOL_TAP4	25
#elif POOLWORDS == 64
#define	POOL_TAP1	52
#define	POOL_TAP2	39
#define	POOL_TAP3	26
#define	POOL_TAP4	14
#elif POOLWORDS == 32
#define	POOL_TAP1	26
#define	POOL_TAP2	20
#define	POOL_TAP3	14
#define	POOL_TAP4	7
#else
#error No primitive polynomial available for chosen POOLWORDS
#endif

static void dequeue_randomness(void *);

/* Master kernel random number pool. */
struct random_bucket {
	u_int	add_ptr;
	u_int	entropy_count;
	u_char	input_rotate;
	u_int32_t pool[POOLWORDS];
	u_int	asleep;
	u_int	tmo;
};
struct random_bucket random_state;
struct mutex rndlock;

/*
 * This function adds a byte into the entropy pool.  It does not
 * update the entropy estimate.  The caller must do this if appropriate.
 *
 * The pool is stirred with a polynomial of degree POOLWORDS over GF(2);
 * see POOL_TAP[1-4] above
 *
 * Rotate the input word by a changing number of bits, to help assure
 * that all bits in the entropy get toggled.  Otherwise, if the pool
 * is consistently feed small numbers (such as keyboard scan codes)
 * then the upper bits of the entropy pool will frequently remain
 * untouched.
 */
static void
add_entropy_words(const u_int32_t *buf, u_int n)
{
	/* derived from IEEE 802.3 CRC-32 */
	static const u_int32_t twist_table[8] = {
		0x00000000, 0x3b6e20c8, 0x76dc4190, 0x4db26158,
		0xedb88320, 0xd6d6a3e8, 0x9b64c2b0, 0xa00ae278
	};

	for (; n--; buf++) {
		u_int32_t w = (*buf << random_state.input_rotate) |
		    (*buf >> (32 - random_state.input_rotate));
		u_int i = random_state.add_ptr =
		    (random_state.add_ptr - 1) & POOLMASK;
		/*
		 * Normally, we add 7 bits of rotation to the pool.
		 * At the beginning of the pool, add an extra 7 bits
		 * rotation, so that successive passes spread the
		 * input bits across the pool evenly.
		 */
		random_state.input_rotate =
		    (random_state.input_rotate + (i ? 7 : 14)) & 31;

		/* XOR pool contents corresponding to polynomial terms */
		w ^= random_state.pool[(i + POOL_TAP1) & POOLMASK] ^
		     random_state.pool[(i + POOL_TAP2) & POOLMASK] ^
		     random_state.pool[(i + POOL_TAP3) & POOLMASK] ^
		     random_state.pool[(i + POOL_TAP4) & POOLMASK] ^
		     random_state.pool[(i + 1) & POOLMASK] ^
		     random_state.pool[i]; /* + 2^POOLWORDS */

		random_state.pool[i] = (w >> 3) ^ twist_table[w & 7];
	}
}

/*
 * This function extracts randomness from the entropy pool, and
 * returns it in a buffer.  This function computes how many remaining
 * bits of entropy are left in the pool, but it does not restrict the
 * number of bytes that are actually obtained.
 */
static void
extract_entropy(u_int8_t *buf, int nbytes)
{
	struct random_bucket *rs = &random_state;
	u_char buffer[16];
	MD5_CTX tmp;
	u_int i;

	add_timer_randomness(nbytes);

	while (nbytes) {
		if (nbytes < sizeof(buffer) / 2)
			i = nbytes;
		else
			i = sizeof(buffer) / 2;

		/* Hash the pool to get the output */
		MD5Init(&tmp);
		mtx_enter(&rndlock);
		MD5Update(&tmp, (u_int8_t*)rs->pool, sizeof(rs->pool));
		if (rs->entropy_count / 8 > i)
			rs->entropy_count -= i * 8;
		else
			rs->entropy_count = 0;
		mtx_leave(&rndlock);
		MD5Final(buffer, &tmp);

		/*
		 * In case the hash function has some recognizable
		 * output pattern, we fold it in half.
		 */
		buffer[0] ^= buffer[15];
		buffer[1] ^= buffer[14];
		buffer[2] ^= buffer[13];
		buffer[3] ^= buffer[12];
		buffer[4] ^= buffer[11];
		buffer[5] ^= buffer[10];
		buffer[6] ^= buffer[ 9];
		buffer[7] ^= buffer[ 8];

		/* Copy data to destination buffer */
		bcopy(buffer, buf, i);
		nbytes -= i;
		buf += i;

		/* Modify pool so next hash will produce different results */
		add_timer_randomness(nbytes);
		dequeue_randomness(&random_state);
	}

	/* Wipe data from memory */
	bzero(&tmp, sizeof(tmp));
	bzero(buffer, sizeof(buffer));
}

/*
 * Kernel-side entropy crediting API and handling of entropy-bearing events
 * ------------------------------------------------------------------------
 */

/* pIII/333 reported to have some drops w/ these numbers */
#define QEVLEN (1024 / sizeof(struct rand_event))
#define QEVSLOW (QEVLEN * 3 / 4) /* yet another 0.75 for 60-minutes hour /-; */
#define QEVSBITS 10

/* There is one of these per entropy source */
struct timer_rand_state {
	u_int	last_time;
	u_int	last_delta;
	u_int	last_delta2;
	u_int	dont_count_entropy : 1;
	u_int	max_entropy : 1;
};

struct rand_event {
	struct timer_rand_state *re_state;
	u_int re_nbits;
	u_int re_time;
	u_int re_val;
};

struct timer_rand_state rnd_states[RND_SRC_NUM];
struct rand_event rnd_event_space[QEVLEN];
struct rand_event *rnd_event_head = rnd_event_space;
struct rand_event *rnd_event_tail = rnd_event_space;
struct timeout rnd_timeout;
struct selinfo rnd_rsel;
struct rndstats rndstats;
int rnd_attached;

/* must be called at a proper spl, returns ptr to the next event */
static __inline struct rand_event *
rnd_get(void)
{
	struct rand_event *p = rnd_event_tail;

	if (p == rnd_event_head)
		return NULL;

	if (p + 1 >= &rnd_event_space[QEVLEN])
		rnd_event_tail = rnd_event_space;
	else
		rnd_event_tail++;

	return p;
}

/* must be called at a proper spl, returns next available item */
static __inline struct rand_event *
rnd_put(void)
{
	struct rand_event *p = rnd_event_head + 1;

	if (p >= &rnd_event_space[QEVLEN])
		p = rnd_event_space;

	if (p == rnd_event_tail)
		return NULL;

	return rnd_event_head = p;
}

/* must be called at a proper spl, returns number of items in the queue */
static __inline int
rnd_qlen(void)
{
	int len = rnd_event_head - rnd_event_tail;
	return (len < 0)? -len : len;
}

/*
 * This function adds entropy to the entropy pool by using timing
 * delays.  It uses the timer_rand_state structure to make an estimate
 * of how many bits of entropy this call has added to the pool.
 *
 * The number "val" is also added to the pool - it should somehow describe
 * the type of event which just happened.  Currently the values of 0-255
 * are for keyboard scan codes, 256 and upwards - for interrupts.
 * On the i386, this is assumed to be at most 16 bits, and the high bits
 * are used for a high-resolution timer.
 */
void
enqueue_randomness(int state, int val)
{
	struct timer_rand_state *p;
	struct rand_event *rep;
	struct timespec	tv;
	u_int	time, nbits;

#ifdef DIAGNOSTIC
	if (state < 0 || state >= RND_SRC_NUM)
		return;
#endif

	p = &rnd_states[state];
	val += state << 13;

	if (!rnd_attached) {
		if ((rep = rnd_put()) == NULL) {
			rndstats.rnd_drops++;
			return;
		}

		rep->re_state = &rnd_states[RND_SRC_TIMER];
		rep->re_nbits = 0;
		rep->re_time = 0;
		rep->re_time = val;
		return;
	}

	nanotime(&tv);
	time = (tv.tv_nsec >> 10) + (tv.tv_sec << 20);
	nbits = 0;

	/*
	 * Calculate the number of bits of randomness that we probably
	 * added.  We take into account the first and second order
	 * deltas in order to make our estimate.
	 */
	if (!p->dont_count_entropy) {
		int	delta, delta2, delta3;
		delta  = time   - p->last_time;
		delta2 = delta  - p->last_delta;
		delta3 = delta2 - p->last_delta2;

		if (delta < 0) delta = -delta;
		if (delta2 < 0) delta2 = -delta2;
		if (delta3 < 0) delta3 = -delta3;
		if (delta > delta2) delta = delta2;
		if (delta > delta3) delta = delta3;
		delta3 = delta >>= 1;
		/*
		 * delta &= 0xfff;
		 * we don't do it since our time sheet is different from linux
		 */

		if (delta & 0xffff0000) {
			nbits = 16;
			delta >>= 16;
		}
		if (delta & 0xff00) {
			nbits += 8;
			delta >>= 8;
		}
		if (delta & 0xf0) {
			nbits += 4;
			delta >>= 4;
		}
		if (delta & 0xc) {
			nbits += 2;
			delta >>= 2;
		}
		if (delta & 2) {
			nbits += 1;
			delta >>= 1;
		}
		if (delta & 1)
			nbits++;

		/*
		 * the logic is to drop low-entropy entries,
		 * in hope for dequeuing to be more randomfull
		 */
		if (rnd_qlen() > QEVSLOW && nbits < QEVSBITS) {
			rndstats.rnd_drople++;
			return;
		}
		p->last_time = time;
		p->last_delta  = delta3;
		p->last_delta2 = delta2;
	} else if (p->max_entropy)
		nbits = 8 * sizeof(val) - 1;

	/* given the multi-order delta logic above, this should never happen */
	if (nbits >= 32)
		return;

	mtx_enter(&rndlock);
	if ((rep = rnd_put()) == NULL) {
		rndstats.rnd_drops++;
		mtx_leave(&rndlock);
		return;
	}

	rep->re_state = p;
	rep->re_nbits = nbits;
	rep->re_time = tv.tv_nsec ^ (tv.tv_sec << 20);
	rep->re_val = val;

	rndstats.rnd_enqs++;
	rndstats.rnd_ed[nbits]++;
	rndstats.rnd_sc[state]++;
	rndstats.rnd_sb[state] += nbits;

	if (rnd_qlen() > QEVSLOW/2 && !random_state.tmo) {
		random_state.tmo++;
		timeout_add(&rnd_timeout, 1);
	}
	mtx_leave(&rndlock);
}

static void
dequeue_randomness(void *v)
{
	struct random_bucket *rs = v;
	struct rand_event *rep;
	u_int32_t buf[2];
	u_int nbits;

	timeout_del(&rnd_timeout);
	rndstats.rnd_deqs++;

	mtx_enter(&rndlock);
	while ((rep = rnd_get())) {

		buf[0] = rep->re_time;
		buf[1] = rep->re_val;
		nbits = rep->re_nbits;
		mtx_leave(&rndlock);

		add_entropy_words(buf, 2);

		rndstats.rnd_total += nbits;
		rs->entropy_count += nbits;
		if (rs->entropy_count > POOLBITS)
			rs->entropy_count = POOLBITS;

		if (rs->asleep && rs->entropy_count > 8) {
#ifdef	RNDEBUG
			if (rnd_debug & RD_WAIT)
				printf("rnd: wakeup[%u]{%u}\n",
				    rs->asleep,
				    rs->entropy_count);
#endif
			rs->asleep--;
			wakeup((void *)&rs->asleep);
			selwakeup(&rnd_rsel);
		}

		mtx_enter(&rndlock);
	}

	rs->tmo = 0;
	mtx_leave(&rndlock);
}

/*
 * Exported kernel CPRNG API: arc4random() and friends
 * ---------------------------------------------------
 */

/*
 * Maximum number of bytes to serve directly from the main arc4random
 * pool. Larger requests are served from discrete arc4 instances keyed
 * from the main pool.
 */
#define ARC4_MAIN_MAX_BYTES	2048

/*
 * Key size (in bytes) for arc4 instances setup to serve requests larger
 * than ARC4_MAIN_MAX_BYTES.
 */
#define ARC4_SUB_KEY_BYTES	(256 / 8)

struct timeout arc4_timeout;
struct rc4_ctx arc4random_state;
int arc4random_initialized;
u_long arc4random_count = 0;

/*
 * This function returns some number of good random numbers but is quite
 * slow. Please use arc4random_buf() instead unless very high quality
 * randomness is required.
 * XXX: rename this
 */
void
get_random_bytes(void *buf, size_t nbytes)
{
	extract_entropy((u_int8_t *) buf, nbytes);
	rndstats.rnd_used += nbytes * 8;
}

static void
arc4_stir(void)
{
	u_int8_t buf[256];
	int len;

	nanotime((struct timespec *) buf);
	len = sizeof(buf) - sizeof(struct timespec);
	get_random_bytes(buf + sizeof (struct timespec), len);
	len += sizeof(struct timespec);

	mtx_enter(&rndlock);
	if (rndstats.arc4_nstirs > 0)
		rc4_crypt(&arc4random_state, buf, buf, sizeof(buf));

	rc4_keysetup(&arc4random_state, buf, sizeof(buf));
	arc4random_count = 0;
	rndstats.arc4_stirs += len;
	rndstats.arc4_nstirs++;

	/*
	 * Throw away the first N words of output, as suggested in the
	 * paper "Weaknesses in the Key Scheduling Algorithm of RC4"
	 * by Fluher, Mantin, and Shamir.  (N = 256 in our case.)
	 */
	rc4_skip(&arc4random_state, 256 * 4);
	mtx_leave(&rndlock);
}

/*
 * called by timeout to mark arc4 for stirring,
 * actual stirring happens on any access attempt.
 */
static void
arc4_reinit(void *v)
{
	arc4random_initialized = 0;
}

static void
arc4maybeinit(void)
{

	if (!arc4random_initialized) {
#ifdef DIAGNOSTIC
		if (!rnd_attached)
			panic("arc4maybeinit: premature");
#endif
		arc4random_initialized++;
		arc4_stir();
		/* 10 minutes, per dm@'s suggestion */
		timeout_add_sec(&arc4_timeout, 10 * 60);
	}
}

void
randomattach(void)
{
	if (rnd_attached) {
#ifdef RNDEBUG
		printf("random: second attach\n");
#endif
		return;
	}

	timeout_set(&rnd_timeout, dequeue_randomness, &random_state);
	timeout_set(&arc4_timeout, arc4_reinit, NULL);

	random_state.add_ptr = 0;
	random_state.entropy_count = 0;
	rnd_states[RND_SRC_TIMER].dont_count_entropy = 1;
	rnd_states[RND_SRC_TRUE].dont_count_entropy = 1;
	rnd_states[RND_SRC_TRUE].max_entropy = 1;

	mtx_init(&rndlock, IPL_HIGH);
	arc4_reinit(NULL);

	if (msgbufp && msgbufp->msg_magic == MSG_MAGIC)
		add_entropy_words((u_int32_t *)msgbufp->msg_bufc,
		    msgbufp->msg_bufs / sizeof(u_int32_t));
	rnd_attached = 1;
}

/* Return one word of randomness from an RC4 generator */
u_int32_t
arc4random(void)
{
	u_int32_t ret;

	arc4maybeinit();
	mtx_enter(&rndlock);
	rc4_getbytes(&arc4random_state, (u_char*)&ret, sizeof(ret));
	rndstats.arc4_reads += sizeof(ret);
	arc4random_count += sizeof(ret);
	mtx_leave(&rndlock);
	return ret;
}

/*
 * Return a "large" buffer of randomness using an independantly-keyed RC4
 * generator.
 */
static void
arc4random_buf_large(void *buf, size_t n)
{
	u_char lbuf[ARC4_SUB_KEY_BYTES];
	struct rc4_ctx lctx;

	mtx_enter(&rndlock);
	rc4_getbytes(&arc4random_state, lbuf, sizeof(lbuf));
	rndstats.arc4_reads += n;
	arc4random_count += sizeof(lbuf);
	mtx_leave(&rndlock);

	rc4_keysetup(&lctx, lbuf, sizeof(lbuf));
	rc4_skip(&lctx, 256 * 4);
	rc4_getbytes(&lctx, (u_char*)buf, n);
	bzero(lbuf, sizeof(lbuf));
	bzero(&lctx, sizeof(lctx));
}

/*
 * Fill a buffer of arbitrary length with RC4-derived randomness.
 */
void
arc4random_buf(void *buf, size_t n)
{
	arc4maybeinit();

	/* Satisfy large requests via an independent ARC4 instance */
	if (n > ARC4_MAIN_MAX_BYTES) {
		arc4random_buf_large(buf, n);
		return;
	}

	mtx_enter(&rndlock);
	rc4_getbytes(&arc4random_state, (u_char*)buf, n);
	rndstats.arc4_reads += n;
	arc4random_count += n;
	mtx_leave(&rndlock);
}

/*
 * Calculate a uniformly distributed random number less than upper_bound
 * avoiding "modulo bias".
 *
 * Uniformity is achieved by generating new random numbers until the one
 * returned is outside the range [0, 2**32 % upper_bound).  This
 * guarantees the selected random number will be inside
 * [2**32 % upper_bound, 2**32) which maps back to [0, upper_bound)
 * after reduction modulo upper_bound.
 */
u_int32_t
arc4random_uniform(u_int32_t upper_bound)
{
	u_int32_t r, min;

	if (upper_bound < 2)
		return 0;

#if (ULONG_MAX > 0xffffffffUL)
	min = 0x100000000UL % upper_bound;
#else
	/* Calculate (2**32 % upper_bound) avoiding 64-bit math */
	if (upper_bound > 0x80000000)
		min = 1 + ~upper_bound;		/* 2**32 - upper_bound */
	else {
		/* (2**32 - x) % x == 2**32 % x when x <= 2**31 */
		min = ((0xffffffff - upper_bound) + 1) % upper_bound;
	}
#endif

	/*
	 * This could theoretically loop forever but each retry has
	 * p > 0.5 (worst case, usually far better) of selecting a
	 * number inside the range we need, so it should rarely need
	 * to re-roll.
	 */
	for (;;) {
		r = arc4random();
		if (r >= min)
			break;
	}

	return r % upper_bound;
}

/*
 * random, srandom, urandom, arandom char devices
 * -------------------------------------------------------
 */

void filt_rndrdetach(struct knote *kn);
int filt_rndread(struct knote *kn, long hint);

struct filterops rndread_filtops =
	{ 1, NULL, filt_rndrdetach, filt_rndread};

void filt_rndwdetach(struct knote *kn);
int filt_rndwrite(struct knote *kn, long hint);

struct filterops rndwrite_filtops =
	{ 1, NULL, filt_rndwdetach, filt_rndwrite};

struct selinfo rnd_wsel;

int
randomopen(dev_t dev, int flag, int mode, struct proc *p)
{
	return (minor(dev) < RND_NODEV ? 0 : ENXIO);
}

int
randomclose(dev_t dev, int flag, int mode, struct proc *p)
{
	return 0;
}

int
randomread(dev_t dev, struct uio *uio, int ioflag)
{
	int		ret = 0;
	u_int32_t 	*buf;

	if (uio->uio_resid == 0)
		return 0;

	buf = malloc(POOLBYTES, M_TEMP, M_WAITOK);

	while (!ret && uio->uio_resid > 0) {
		int	n = min(POOLBYTES, uio->uio_resid);

		switch(minor(dev)) {
		case RND_RND:
			ret = EIO;	/* no chip -- error */
			break;
		case RND_SRND:
		case RND_URND:
		case RND_ARND_OLD:
		case RND_ARND:
			arc4random_buf(buf, n);
			break;
		default:
			ret = ENXIO;
		}
		if (n != 0 && ret == 0) {
			ret = uiomove((caddr_t)buf, n, uio);
			if (!ret && uio->uio_resid > 0)
				yield();
		}
	}

	free(buf, M_TEMP);
	return ret;
}

int
randompoll(dev_t dev, int events, struct proc *p)
{
	int revents;

	revents = events & (POLLOUT | POLLWRNORM);	/* always writable */
	if (events & (POLLIN | POLLRDNORM)) {
		revents |= events & (POLLIN | POLLRDNORM);
	}

	return (revents);
}

int
randomkqfilter(dev_t dev, struct knote *kn)
{
	struct klist *klist;

	switch (kn->kn_filter) {
	case EVFILT_READ:
		klist = &rnd_rsel.si_note;
		kn->kn_fop = &rndread_filtops;
		break;
	case EVFILT_WRITE:
		klist = &rnd_wsel.si_note;
		kn->kn_fop = &rndwrite_filtops;
		break;
	default:
		return (1);
	}
	kn->kn_hook = (void *)&random_state;

	mtx_enter(&rndlock);
	SLIST_INSERT_HEAD(klist, kn, kn_selnext);
	mtx_leave(&rndlock);

	return (0);
}

void
filt_rndrdetach(struct knote *kn)
{
	mtx_enter(&rndlock);
	SLIST_REMOVE(&rnd_rsel.si_note, kn, knote, kn_selnext);
	mtx_leave(&rndlock);
}

int
filt_rndread(struct knote *kn, long hint)
{
	struct random_bucket *rs = (struct random_bucket *)kn->kn_hook;

	kn->kn_data = (int)rs->entropy_count;
	return rs->entropy_count > 0;
}

void
filt_rndwdetach(struct knote *kn)
{
	mtx_enter(&rndlock);
	SLIST_REMOVE(&rnd_wsel.si_note, kn, knote, kn_selnext);
	mtx_leave(&rndlock);
}

int
filt_rndwrite(struct knote *kn, long hint)
{
	return (1);
}

int
randomwrite(dev_t dev, struct uio *uio, int flags)
{
	int		ret = 0, newdata = 0;
	u_int32_t	*buf;

	if (minor(dev) == RND_RND)
		return ENXIO;

	if (uio->uio_resid == 0)
		return 0;

	buf = malloc(POOLBYTES, M_TEMP, M_WAITOK);

	while (!ret && uio->uio_resid > 0) {
		u_int	n = min(POOLBYTES, uio->uio_resid);

		ret = uiomove(buf, n, uio);
		if (ret)
			break;
		while (n % sizeof(u_int32_t))
			((u_int8_t *) buf)[n++] = 0;
		add_entropy_words(buf, n / 4);
		newdata = 1;
	}

	if (newdata) {
		mtx_enter(&rndlock);
		arc4random_initialized = 0;
		mtx_leave(&rndlock);
	}

	free(buf, M_TEMP);
	return ret;
}

int
randomioctl(dev_t dev, u_long cmd, caddr_t data, int flag, struct proc *p)
{
	int	ret = 0;

	switch (cmd) {
	case FIOASYNC:
		/* rnd has no async flag in softc so this is really a no-op. */
		break;

	case FIONBIO:
		/* Handled in the upper FS layer. */
		break;

	default:
		ret = ENOTTY;
	}

	return ret;
}
