/* 	$OpenBSD: if_ocevar.h,v 1.5 2012/11/03 00:23:25 mikeb Exp $	*/

/*-
 * Copyright (C) 2012 Emulex
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the Emulex Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Contact Information:
 * freebsd-drivers@emulex.com
 *
 * Emulex
 * 3333 Susan Street
 * Costa Mesa, CA 92626
 */

/* OCE device driver module component revision informaiton */
#define COMPONENT_REVISION	"4.2.127.0"

/* This should be powers of 2. Like 2,4,8 & 16 */
#define OCE_MAX_RSS		4 /* TODO: 8 */
#define OCE_LEGACY_MODE_RSS	4 /* For BE3 Legacy mode */

#define OCE_MAX_RQ		OCE_MAX_RSS + 1 /* one default queue */
#define OCE_MAX_WQ		8

#define OCE_MAX_EQ		32
#define OCE_MAX_CQ		OCE_MAX_RQ + OCE_MAX_WQ + 1 /* one MCC queue */
#define OCE_MAX_CQ_EQ		8 /* Max CQ that can attached to an EQ */

#define OCE_DEFAULT_EQD		80
#define OCE_RQ_BUF_SIZE		2048
#define OCE_LSO_MAX_SIZE	(64 * 1024)
#define OCE_MAX_JUMBO_FRAME_SIZE 16360
#define OCE_MAX_MTU		(OCE_MAX_JUMBO_FRAME_SIZE -	\
				 ETHER_VLAN_ENCAP_LEN -		\
				 ETHER_HDR_LEN - ETHER_CRC_LEN)

#define OCE_MAX_TX_ELEMENTS	29
#define OCE_MAX_TX_DESC		1024
#define OCE_MAX_TX_SIZE		65535
#define OCE_MAX_RX_SIZE		4096
#define OCE_MAX_RQ_POSTS	255
#define OCE_MAX_RSP_HANDLED	64

#define OCE_RSS_IPV4		0x1
#define OCE_RSS_TCP_IPV4	0x2
#define OCE_RSS_IPV6		0x4
#define OCE_RSS_TCP_IPV6	0x8

/* flow control definitions */
#define OCE_FC_NONE		0x00000000
#define OCE_FC_TX		0x00000001
#define OCE_FC_RX		0x00000002

/* Interface capabilities to give device when creating interface */
#define OCE_CAPAB_FLAGS 	(MBX_RX_IFACE_FLAGS_BROADCAST     | \
				 MBX_RX_IFACE_FLAGS_UNTAGGED      | \
				 MBX_RX_IFACE_FLAGS_PROMISC       | \
				 MBX_RX_IFACE_FLAGS_MCAST_PROMISC)
				/* MBX_RX_IFACE_FLAGS_RSS | \ */
				/* MBX_RX_IFACE_FLAGS_PASS_L3L4_ERR) */

/* Interface capabilities to enable by default (others set dynamically) */
#define OCE_CAPAB_ENABLE	(MBX_RX_IFACE_FLAGS_BROADCAST | \
				 MBX_RX_IFACE_FLAGS_UNTAGGED)
				/* MBX_RX_IFACE_FLAGS_RSS        | \ */
				/* MBX_RX_IFACE_FLAGS_PASS_L3L4_ERR) */

#define BSWAP_8(x)		((x) & 0xff)
#define BSWAP_16(x)		((BSWAP_8(x) << 8) | BSWAP_8((x) >> 8))
#define BSWAP_32(x)		((BSWAP_16(x) << 16) | BSWAP_16((x) >> 16))
#define BSWAP_64(x)		((BSWAP_32(x) << 32) | BSWAP_32((x) >> 32))

#define for_all_wq_queues(sc, wq, i) 	\
		for (i = 0, wq = sc->wq[0]; i < sc->nwqs; i++, wq = sc->wq[i])
#define for_all_rq_queues(sc, rq, i) 	\
		for (i = 0, rq = sc->rq[0]; i < sc->nrqs; i++, rq = sc->rq[i])
#define for_all_eq_queues(sc, eq, i) 	\
		for (i = 0, eq = sc->eq[0]; i < sc->neqs; i++, eq = sc->eq[i])
#define for_all_cq_queues(sc, cq, i) 	\
		for (i = 0, cq = sc->cq[0]; i < sc->ncqs; i++, cq = sc->cq[i])

enum {
	PHY_TYPE_CX4_10GB = 0,
	PHY_TYPE_XFP_10GB,
	PHY_TYPE_SFP_1GB,
	PHY_TYPE_SFP_PLUS_10GB,
	PHY_TYPE_KR_10GB,
	PHY_TYPE_KX4_10GB,
	PHY_TYPE_BASET_10GB,
	PHY_TYPE_BASET_1GB,
	PHY_TYPE_BASEX_1GB,
	PHY_TYPE_SGMII,
	PHY_TYPE_DISABLED = 255
};

#define RING_NUM_FREE(_r)	((_r)->nitems - (_r)->nused)

#define OCE_MEM_KVA(_m)		((void *)((_m)->vaddr))

#define OCE_RING_FOREACH(_r, _v, _c)	\
	for ((_v) = oce_ring_first(_r); _c; (_v) = oce_ring_next(_r))

struct oce_packet_desc {
	struct mbuf *		mbuf;
	bus_dmamap_t		map;
	int			nsegs;
};

struct oce_dma_mem {
	bus_dma_tag_t		tag;
	bus_dmamap_t		map;
	bus_dma_segment_t	segs;
	int			nsegs;
	bus_size_t		size;
	caddr_t			vaddr;
	bus_addr_t		paddr;
};

struct oce_ring {
	int			index;
	int			nitems;
	int			nused;
	int			isize;
	struct oce_dma_mem	dma;
};

#define TRUE			1
#define FALSE			0

#define MBX_TIMEOUT_SEC		5

/* size of the packet descriptor array in a transmit queue */
#define OCE_TX_RING_SIZE	512
#define OCE_RX_RING_SIZE	1024
#define OCE_WQ_PACKET_ARRAY_SIZE (OCE_TX_RING_SIZE/2)
#define OCE_RQ_PACKET_ARRAY_SIZE (OCE_RX_RING_SIZE)

struct oce_softc;

enum cq_len {
	CQ_LEN_256  = 256,
	CQ_LEN_512  = 512,
	CQ_LEN_1024 = 1024
};

enum eq_len {
	EQ_LEN_256  = 256,
	EQ_LEN_512  = 512,
	EQ_LEN_1024 = 1024,
	EQ_LEN_2048 = 2048,
	EQ_LEN_4096 = 4096
};

enum eqe_size {
	EQE_SIZE_4  = 4,
	EQE_SIZE_16 = 16
};

enum qtype {
	QTYPE_EQ,
	QTYPE_MQ,
	QTYPE_WQ,
	QTYPE_RQ,
	QTYPE_CQ,
	QTYPE_RSS
};

struct eq_config {
	enum eq_len		q_len;
	enum eqe_size		item_size;
	int			eqd;
};

struct oce_eq {
	struct oce_softc *	sc;
	struct oce_ring *	ring;
	enum qtype		type;
	int			id;

	struct oce_cq *		cq[OCE_MAX_CQ_EQ];
	int			cq_valid;

	struct eq_config	cfg;
};

struct cq_config {
	enum cq_len		q_len;
	int			item_size;
	int			nodelay;
	int			dma_coalescing;
	int			ncoalesce;
	int			eventable;
};

struct oce_cq {
	struct oce_softc *	sc;
	struct oce_ring *	ring;
	enum qtype		type;
	int			id;

	struct oce_eq *		eq;

	struct cq_config 	cfg;

	void			(*cq_intr)(void *);
	void *			cb_arg;
};

struct mq_config {
	int			eqd;
	int			q_len;
};

struct oce_mq {
	struct oce_softc *	sc;
	struct oce_ring *	ring;
	enum qtype		type;
	int			id;

	struct oce_cq *		cq;

	struct mq_config	cfg;
};

struct wq_config {
	int			wq_type;
	int			buf_size;
	int			q_len;
	int			eqd;		/* interrupt delay */
	int			nbufs;
};

struct oce_wq {
	struct oce_softc *	sc;
	struct oce_ring *	ring;
	enum qtype		type;
	int			id;

	bus_dma_tag_t		tag;

	struct oce_cq *		cq;
	struct oce_packet_desc	pckts[OCE_WQ_PACKET_ARRAY_SIZE];

	uint32_t		packets_in;
	uint32_t		packets_out;

	struct wq_config	cfg;
};

struct rq_config {
	int			q_len;
	int			frag_size;
	int			mtu;
	int			if_id;
	int			is_rss_queue;
	int			eqd;
	int			nbufs;
};

struct oce_rq {
	struct oce_softc *	sc;
	struct oce_ring *	ring;
	enum qtype		type;
	int			id;

	bus_dma_tag_t		tag;

	struct oce_cq *		cq;
	struct oce_packet_desc	pckts[OCE_RQ_PACKET_ARRAY_SIZE];

	uint32_t		packets_in;
	uint32_t		packets_out;
	uint32_t		pending;

	uint32_t		rss_cpuid;

#ifdef OCE_LRO
	struct lro_ctrl		lro;
	int			lro_pkts_queued;
#endif

	struct rq_config	cfg;
};

struct link_status {
	uint8_t			physical_port;
	uint8_t			mac_duplex;
	uint8_t			mac_speed;
	uint8_t			mac_fault;
	uint8_t			mgmt_mac_duplex;
	uint8_t			mgmt_mac_speed;
	uint16_t		qos_link_speed;
	uint32_t		logical_link_status;
} __packed;

#define OCE_F_BE2		0x00000001
#define OCE_F_BE3		0x00000002
#define OCE_F_BE3_NATIVE	0x00000004
#define OCE_F_XE201		0x00000008
#define OCE_F_RESET_RQD		0x00000100
#define OCE_F_MBOX_ENDIAN_RQD	0x00000200

struct oce_softc {
	struct device		dev;

	uint32_t		flags;

	struct pci_attach_args	pa;

	bus_space_tag_t		cfg_iot;
	bus_space_handle_t	cfg_ioh;
	bus_size_t		cfg_size;

	bus_space_tag_t		csr_iot;
	bus_space_handle_t	csr_ioh;
	bus_size_t		csr_size;

	bus_space_tag_t		db_iot;
	bus_space_handle_t	db_ioh;
	bus_size_t		db_size;

	struct arpcom		arpcom;
	struct ifmedia		media;
	int			link_active;
	uint8_t			link_status;
	uint8_t			link_speed;
	uint8_t			duplex;
	uint32_t		qos_link_speed;

	struct oce_dma_mem	bsmbx;

	uint32_t		port_id;
	uint32_t		function_mode;

	struct oce_wq *		wq[OCE_MAX_WQ];	/* TX work queues */
	struct oce_rq *		rq[OCE_MAX_RQ];	/* RX work queues */
	struct oce_cq *		cq[OCE_MAX_CQ];	/* Completion queues */
	struct oce_eq *		eq[OCE_MAX_EQ];	/* Event queues */
	struct oce_mq *		mq;		/* Mailbox queue */

	ushort			neqs;
	ushort			ncqs;
	ushort			nrqs;
	ushort			nwqs;
	ushort			intr_count;
	ushort			tx_ring_size;
	ushort			rx_ring_size;
	ushort			rq_frag_size;
	ushort			rss_enable;

	uint32_t		if_id;		/* interface ID */
	uint32_t		nifs;		/* number of adapter interfaces, 0 or 1 */
	uint32_t		pmac_id;	/* PMAC id */

	char			macaddr[ETHER_ADDR_LEN];

	uint32_t		if_cap_flags;

	uint32_t		flow_control;

	int			be3_native;
	uint32_t		pvid;

	uint64_t		rx_errors;
	uint64_t		tx_errors;

	struct timeout		timer;
	struct timeout		rxrefill;
};

#define IS_BE(sc)		ISSET((sc)->flags, OCE_F_BE2 | OCE_F_BE3)
#define IS_XE201(sc)		ISSET((sc)->flags, OCE_F_XE201)

#define oce_dma_sync(d, f) \
	bus_dmamap_sync((d)->tag, (d)->map, 0, (d)->map->dm_mapsize, f)

#define DW_SWAP(x, l)

#define ADDR_HI(x)		((uint32_t)((uint64_t)(x) >> 32))
#define ADDR_LO(x)		((uint32_t)((uint64_t)(x) & 0xffffffff))

#define IFCAP_HWCSUM \
	(IFCAP_CSUM_IPv4 | IFCAP_CSUM_TCPv4 | IFCAP_CSUM_UDPv4)
#define IF_LRO_ENABLED(ifp)	ISSET((ifp)->if_capabilities, IFCAP_LRO)
#define IF_LSO_ENABLED(ifp)	ISSET((ifp)->if_capabilities, IFCAP_TSO4)
#define IF_CSUM_ENABLED(ifp)	ISSET((ifp)->if_capabilities, IFCAP_HWCSUM)

static inline int
ilog2(unsigned int v)
{
	int r = 0;

	while (v >>= 1)
		r++;
	return (r);
}
