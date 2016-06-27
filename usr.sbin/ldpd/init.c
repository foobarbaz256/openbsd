/*	$OpenBSD: init.c,v 1.30 2016/06/27 19:06:33 renato Exp $ */

/*
 * Copyright (c) 2009 Michele Marchetto <michele@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

#include "ldpd.h"
#include "ldpe.h"
#include "log.h"

static int	gen_init_prms_tlv(struct ibuf *, struct nbr *, uint16_t);
static int	tlv_decode_opt_init_prms(char *, uint16_t);

void
send_init(struct nbr *nbr)
{
	struct ibuf		*buf;
	uint16_t		 size;
	int			 err = 0;

	log_debug("%s: lsr-id %s", __func__, inet_ntoa(nbr->id));

	size = LDP_HDR_SIZE + LDP_MSG_SIZE + SESS_PRMS_SIZE;
	if ((buf = ibuf_open(size)) == NULL)
		fatal(__func__);

	err |= gen_ldp_hdr(buf, size);
	size -= LDP_HDR_SIZE;
	err |= gen_msg_hdr(buf, MSG_TYPE_INIT, size);
	size -= LDP_MSG_SIZE;
	err |= gen_init_prms_tlv(buf, nbr, size);
	if (err) {
		ibuf_free(buf);
		return;
	}

	evbuf_enqueue(&nbr->tcp->wbuf, buf);
}

int
recv_init(struct nbr *nbr, char *buf, uint16_t len)
{
	struct ldp_msg		init;
	struct sess_prms_tlv	sess;
	uint16_t		max_pdu_len;
	int			r;

	log_debug("%s: lsr-id %s", __func__, inet_ntoa(nbr->id));

	memcpy(&init, buf, sizeof(init));
	buf += LDP_MSG_SIZE;
	len -= LDP_MSG_SIZE;

	if (len < SESS_PRMS_SIZE) {
		session_shutdown(nbr, S_BAD_MSG_LEN, init.msgid, init.type);
		return (-1);
	}
	memcpy(&sess, buf, sizeof(sess));
	if (ntohs(sess.keepalive_time) < MIN_KEEPALIVE) {
		session_shutdown(nbr, S_KEEPALIVE_BAD, init.msgid, init.type);
		return (-1);
	}
	if (ntohs(sess.length) != SESS_PRMS_SIZE - TLV_HDR_LEN) {
		session_shutdown(nbr, S_BAD_TLV_LEN, init.msgid, init.type);
		return (-1);
	}
	if (ntohs(sess.proto_version) != LDP_VERSION) {
		session_shutdown(nbr, S_BAD_PROTO_VER, init.msgid, init.type);
		return (-1);
	}
	if (sess.lsr_id != leconf->rtr_id.s_addr ||
	    ntohs(sess.lspace_id) != 0) {
		session_shutdown(nbr, S_NO_HELLO, init.msgid, init.type);
		return (-1);
	}

	buf += SESS_PRMS_SIZE;
	len -= SESS_PRMS_SIZE;

	/* just ignore all optional TLVs for now */
	r = tlv_decode_opt_init_prms(buf, len);
	if (r == -1 || r != len) {
		session_shutdown(nbr, S_BAD_TLV_VAL, init.msgid, init.type);
		return (-1);
	}

	nbr->keepalive = min(nbr_get_keepalive(nbr->af, nbr->id),
	    ntohs(sess.keepalive_time));

	max_pdu_len = ntohs(sess.max_pdu_len);
	/*
	 * RFC 5036 - Section 3.5.3:
	 * "A value of 255 or less specifies the default maximum length of
	 * 4096 octets".
	 */
	if (max_pdu_len <= 255)
		max_pdu_len = LDP_MAX_LEN;
	nbr->max_pdu_len = min(max_pdu_len, LDP_MAX_LEN);

	nbr_fsm(nbr, NBR_EVT_INIT_RCVD);

	return (0);
}

static int
gen_init_prms_tlv(struct ibuf *buf, struct nbr *nbr, uint16_t size)
{
	struct sess_prms_tlv	parms;

	memset(&parms, 0, sizeof(parms));
	parms.type = htons(TLV_TYPE_COMMONSESSION);
	parms.length = htons(size - TLV_HDR_LEN);
	parms.proto_version = htons(LDP_VERSION);
	parms.keepalive_time = htons(nbr_get_keepalive(nbr->af, nbr->id));
	parms.reserved = 0;
	parms.pvlim = 0;
	parms.max_pdu_len = 0;
	parms.lsr_id = nbr->id.s_addr;
	parms.lspace_id = 0;

	return (ibuf_add(buf, &parms, SESS_PRMS_SIZE));
}

static int
tlv_decode_opt_init_prms(char *buf, uint16_t len)
{
	struct tlv	tlv;
	uint16_t	tlv_len;
	int		total = 0;

	 while (len >= sizeof(tlv)) {
		memcpy(&tlv, buf, TLV_HDR_LEN);
		buf += TLV_HDR_LEN;
		len -= TLV_HDR_LEN;
		total += TLV_HDR_LEN;
		tlv_len = ntohs(tlv.length);

		switch (ntohs(tlv.type)) {
		case TLV_TYPE_ATMSESSIONPAR:
			log_warnx("ATM session parameter present");
			return (-1);
		case TLV_TYPE_FRSESSION:
			log_warnx("FR session parameter present");
			return (-1);
		default:
			/* if unknown flag set, ignore TLV */
			if (!(ntohs(tlv.type) & UNKNOWN_FLAG))
				return (-1);
			break;
		}
		buf += tlv_len;
		len -= tlv_len;
		total += tlv_len;
	}

	return (total);
}
