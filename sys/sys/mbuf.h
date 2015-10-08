/*	$OpenBSD: mbuf.h,v 1.197 2015/10/08 11:36:15 dlg Exp $	*/
/*	$NetBSD: mbuf.h,v 1.19 1996/02/09 18:25:14 christos Exp $	*/

/*
 * Copyright (c) 1982, 1986, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)mbuf.h	8.5 (Berkeley) 2/19/95
 */

#ifndef _SYS_MBUF_H_
#define _SYS_MBUF_H_

#include <sys/malloc.h>
#include <sys/queue.h>

/*
 * Constants related to network buffer management.
 * MCLBYTES must be no larger than PAGE_SIZE (the software page size) and,
 * on machines that exchange pages of input or output buffers with mbuf
 * clusters (MAPPED_MBUFS), MCLBYTES must also be an integral multiple
 * of the hardware page size.
 */
#define	MSIZE		256		/* size of an mbuf */

/*
 * Mbufs are of a single size, MSIZE, which includes overhead.  An mbuf may
 * add a single "mbuf cluster" of size MCLBYTES, which has no additional
 * overhead and is used instead of the internal data area; this is done when
 * at least MINCLSIZE of data must be stored.
 */

#define	MLEN		(MSIZE - sizeof(struct m_hdr))	/* normal data len */
#define	MHLEN		(MLEN - sizeof(struct pkthdr))	/* data len w/pkthdr */

#define	MAXMCLBYTES	(64 * 1024)		/* largest cluster from the stack */
#define	MINCLSIZE	(MHLEN + MLEN + 1)	/* smallest amount to put in cluster */
#define	M_MAXCOMPRESS	(MHLEN / 2)		/* max amount to copy for compression */

#define	MCLSHIFT	11		/* convert bytes to m_buf clusters */
					/* 2K cluster can hold Ether frame */
#define	MCLBYTES	(1 << MCLSHIFT)	/* size of a m_buf cluster */
#define	MCLOFSET	(MCLBYTES - 1)

/* Packet tags structure */
struct m_tag {
	SLIST_ENTRY(m_tag)	m_tag_link;	/* List of packet tags */
	u_int16_t		m_tag_id;	/* Tag ID */
	u_int16_t		m_tag_len;	/* Length of data */
};

/*
 * Macros for type conversion
 * mtod(m,t) -	convert mbuf pointer to data pointer of correct type
 */
#define	mtod(m,t)	((t)((m)->m_data))

/* header at beginning of each mbuf: */
struct m_hdr {
	struct	mbuf *mh_next;		/* next buffer in chain */
	struct	mbuf *mh_nextpkt;	/* next chain in queue/record */
	caddr_t	mh_data;		/* location of data */
	u_int	mh_len;			/* amount of data in this mbuf */
	short	mh_type;		/* type of data in this mbuf */
	u_short	mh_flags;		/* flags; see below */
};

/* pf stuff */
struct pf_state_key;
struct inpcb;

struct pkthdr_pf {
	struct pf_state_key *statekey;	/* pf stackside statekey */
	struct inpcb	*inp;		/* connected pcb for outgoing packet */
	u_int32_t	 qid;		/* queue id */
	u_int16_t	 tag;		/* tag id */
	u_int8_t	 flags;
	u_int8_t	 routed;
	u_int8_t	 prio;
	u_int8_t	 pad[3];
};

/* pkthdr_pf.flags */
#define	PF_TAG_GENERATED		0x01
#define	PF_TAG_TRANSLATE_LOCALHOST	0x04
#define	PF_TAG_DIVERTED			0x08
#define	PF_TAG_DIVERTED_PACKET		0x10
#define	PF_TAG_REROUTE			0x20
#define	PF_TAG_REFRAGMENTED		0x40	/* refragmented ipv6 packet */
#define	PF_TAG_PROCESSED		0x80	/* packet was checked by pf */

#ifdef _KERNEL
#define MPF_BITS \
    ("\20\1GENERATED\3TRANSLATE_LOCALHOST\4DIVERTED\5DIVERTED_PACKET" \
    "\6REROUTE\7REFRAGMENTED\10PROCESSED")
#endif

/* record/packet header in first mbuf of chain; valid if M_PKTHDR set */
struct	pkthdr {
	void			*ph_cookie;	/* additional data */
	SLIST_HEAD(packet_tags, m_tag) tags;	/* list of packet tags */
	int			 len;		/* total packet length */
	u_int16_t		 tagsset;	/* mtags attached */
	u_int16_t		 flowid;	/* pseudo unique flow id */
	u_int16_t		 csum_flags;	/* checksum flags */
	u_int16_t		 ether_vtag;	/* Ethernet 802.1p+Q vlan tag */
	u_int			 ph_rtableid;	/* routing table id */
	u_int			 ph_ifidx;	/* rcv interface index */
	struct pkthdr_pf	 pf;
};

/* description of external storage mapped into mbuf, valid if M_EXT set */
struct mbuf_ext {
	caddr_t	ext_buf;		/* start of buffer */
					/* free routine if not the usual */
	void	(*ext_free)(caddr_t, u_int, void *);
	void	*ext_arg;
	u_int	ext_size;		/* size of buffer, for ext_free */
	struct mbuf *ext_nextref;
	struct mbuf *ext_prevref;
#ifdef DEBUG
	const char *ext_ofile;
	const char *ext_nfile;
	int ext_oline;
	int ext_nline;
#endif
};

struct mbuf {
	struct	m_hdr m_hdr;
	union {
		struct {
			struct	pkthdr MH_pkthdr;	/* M_PKTHDR set */
			union {
				struct	mbuf_ext MH_ext; /* M_EXT set */
				char	MH_databuf[MHLEN];
			} MH_dat;
		} MH;
		char	M_databuf[MLEN];		/* !M_PKTHDR, !M_EXT */
	} M_dat;
};
#define	m_next		m_hdr.mh_next
#define	m_len		m_hdr.mh_len
#define	m_data		m_hdr.mh_data
#define	m_type		m_hdr.mh_type
#define	m_flags		m_hdr.mh_flags
#define	m_nextpkt	m_hdr.mh_nextpkt
#define	m_pkthdr	M_dat.MH.MH_pkthdr
#define	m_ext		M_dat.MH.MH_dat.MH_ext
#define	m_pktdat	M_dat.MH.MH_dat.MH_databuf
#define	m_dat		M_dat.M_databuf

/* mbuf flags */
#define	M_EXT		0x0001	/* has associated external storage */
#define	M_PKTHDR	0x0002	/* start of record */
#define	M_EOR		0x0004	/* end of record */
#define M_EXTWR		0x0008	/* external storage is writable */
#define	M_PROTO1	0x0010	/* protocol-specific */

/* mbuf pkthdr flags, also in m_flags */
#define M_VLANTAG	0x0020	/* ether_vtag is valid */
#define M_LOOP		0x0040	/* for Mbuf statistics */
#define M_FILDROP	0x0080	/* dropped by bpf filter */
#define M_BCAST		0x0100	/* send/received as link-level broadcast */
#define M_MCAST		0x0200	/* send/received as link-level multicast */
#define M_CONF		0x0400  /* payload was encrypted (ESP-transport) */
#define M_AUTH		0x0800  /* payload was authenticated (AH or ESP auth) */
#define M_TUNNEL	0x1000  /* IP-in-IP added by tunnel mode IPsec */
#define M_ZEROIZE	0x2000  /* Zeroize data part on free */
#define M_COMP		0x4000  /* header was decompressed */
#define M_LINK0		0x8000	/* link layer specific flag */

#ifdef _KERNEL
#define M_BITS \
    ("\20\1M_EXT\2M_PKTHDR\3M_EOR\4M_EXTWR\5M_PROTO1\6M_VLANTAG\7M_LOOP" \
    "\10M_FILDROP\11M_BCAST\12M_MCAST\13M_CONF\14M_AUTH\15M_TUNNEL" \
    "\16M_ZEROIZE\17M_COMP\20M_LINK0")
#endif

/* flags copied when copying m_pkthdr */
#define	M_COPYFLAGS	(M_PKTHDR|M_EOR|M_PROTO1|M_BCAST|M_MCAST|M_CONF|M_COMP|\
			 M_AUTH|M_LOOP|M_TUNNEL|M_LINK0|M_VLANTAG|M_FILDROP|\
			 M_ZEROIZE)

/* Checksumming flags */
#define	M_IPV4_CSUM_OUT		0x0001	/* IPv4 checksum needed */
#define	M_TCP_CSUM_OUT		0x0002	/* TCP checksum needed */
#define	M_UDP_CSUM_OUT		0x0004	/* UDP checksum needed */
#define	M_IPV4_CSUM_IN_OK	0x0008	/* IPv4 checksum verified */
#define	M_IPV4_CSUM_IN_BAD	0x0010	/* IPv4 checksum bad */
#define	M_TCP_CSUM_IN_OK	0x0020	/* TCP checksum verified */
#define	M_TCP_CSUM_IN_BAD	0x0040	/* TCP checksum bad */
#define	M_UDP_CSUM_IN_OK	0x0080	/* UDP checksum verified */
#define	M_UDP_CSUM_IN_BAD	0x0100	/* UDP checksum bad */
#define	M_ICMP_CSUM_OUT		0x0200	/* ICMP/ICMPv6 checksum needed */
#define	M_ICMP_CSUM_IN_OK	0x0400	/* ICMP/ICMPv6 checksum verified */
#define	M_ICMP_CSUM_IN_BAD	0x0800	/* ICMP/ICMPv6 checksum bad */

#ifdef _KERNEL
#define MCS_BITS \
    ("\20\1IPV4_CSUM_OUT\2TCP_CSUM_OUT\3UDP_CSUM_OUT\4IPV4_CSUM_IN_OK" \
    "\5IPV4_CSUM_IN_BAD\6TCP_CSUM_IN_OK\7TCP_CSUM_IN_BAD\10UDP_CSUM_IN_OK" \
    "\11UDP_CSUM_IN_BAD\12ICMP_CSUM_OUT\13ICMP_CSUM_IN_OK\14ICMP_CSUM_IN_BAD")
#endif

/* mbuf types */
#define	MT_FREE		0	/* should be on free list */
#define	MT_DATA		1	/* dynamic (data) allocation */
#define	MT_HEADER	2	/* packet header */
#define	MT_SONAME	3	/* socket name */
#define	MT_SOOPTS	4	/* socket options */
#define	MT_FTABLE	5	/* fragment reassembly header */
#define	MT_CONTROL	6	/* extra-data protocol message */
#define	MT_OOBDATA	7	/* expedited data  */

/* flowid field */
#define M_FLOWID_VALID	0x8000	/* is the flowid set */
#define M_FLOWID_MASK	0x7fff	/* flow id to map to path */

/* flags to m_get/MGET */
#define	M_DONTWAIT	M_NOWAIT
#define	M_WAIT		M_WAITOK

/*
 * mbuf allocation/deallocation macros:
 *
 *	MGET(struct mbuf *m, int how, int type)
 * allocates an mbuf and initializes it to contain internal data.
 *
 *	MGETHDR(struct mbuf *m, int how, int type)
 * allocates an mbuf and initializes it to contain a packet header
 * and internal data.
 */
#define MGET(m, how, type) m = m_get((how), (type))

#define MGETHDR(m, how, type) m = m_gethdr((how), (type))

/*
 * Macros for tracking external storage associated with an mbuf.
 */
#ifdef DEBUG
#define MCLREFDEBUGN(m, file, line) do {				\
		(m)->m_ext.ext_nfile = (file);				\
		(m)->m_ext.ext_nline = (line);				\
	} while (/* CONSTCOND */ 0)
#define MCLREFDEBUGO(m, file, line) do {				\
		(m)->m_ext.ext_ofile = (file);				\
		(m)->m_ext.ext_oline = (line);				\
	} while (/* CONSTCOND */ 0)
#else
#define MCLREFDEBUGN(m, file, line)
#define MCLREFDEBUGO(m, file, line)
#endif

#define	MCLISREFERENCED(m)	((m)->m_ext.ext_nextref != (m))

#define	MCLADDREFERENCE(o, n)	m_extref((o), (n))

#define	MCLINITREFERENCE(m)	do {					\
		(m)->m_ext.ext_prevref = (m);				\
		(m)->m_ext.ext_nextref = (m);				\
		MCLREFDEBUGO((m), __FILE__, __LINE__);			\
		MCLREFDEBUGN((m), NULL, 0);				\
	} while (/* CONSTCOND */ 0)

/*
 * Macros for mbuf external storage.
 *
 * MEXTADD adds pre-allocated external storage to
 * a normal mbuf; the flag M_EXT is set.
 *
 * MCLGET allocates and adds an mbuf cluster to a normal mbuf;
 * the flag M_EXT is set upon success.
 */
#define	MEXTADD(m, buf, size, mflags, free, arg) do {			\
	(m)->m_data = (m)->m_ext.ext_buf = (caddr_t)(buf);		\
	(m)->m_flags |= M_EXT | (mflags & M_EXTWR);			\
	(m)->m_ext.ext_size = (size);					\
	(m)->m_ext.ext_free = (free);					\
	(m)->m_ext.ext_arg = (arg);					\
	MCLINITREFERENCE(m);						\
} while (/* CONSTCOND */ 0)

#define MCLGET(m, how) (void) m_clget((m), (how), MCLBYTES)
#define MCLGETI(m, how, ifp, l) m_clget((m), (how), (l))

/*
 * Move just m_pkthdr from from to to,
 * remove M_PKTHDR and clean flags/tags for from.
 */
#define M_MOVE_HDR(to, from) do {					\
	(to)->m_pkthdr = (from)->m_pkthdr;				\
	(from)->m_flags &= ~M_PKTHDR;					\
	SLIST_INIT(&(from)->m_pkthdr.tags);				\
} while (/* CONSTCOND */ 0)

/*
 * MOVE mbuf pkthdr from from to to.
 * from must have M_PKTHDR set, and to must be empty.
 */
#define	M_MOVE_PKTHDR(to, from) do {					\
	(to)->m_flags = ((to)->m_flags & (M_EXT | M_EXTWR));		\
	(to)->m_flags |= (from)->m_flags & M_COPYFLAGS;			\
	M_MOVE_HDR((to), (from));					\
	if (((to)->m_flags & M_EXT) == 0)				\
		(to)->m_data = (to)->m_pktdat;				\
} while (/* CONSTCOND */ 0)

/*
 * Set the m_data pointer of a newly-allocated mbuf (m_get/MGET) to place
 * an object of the specified size at the end of the mbuf, longword aligned.
 */
#define	M_ALIGN(m, len) \
	(m)->m_data += (MLEN - (len)) &~ (sizeof(long) - 1)
/*
 * As above, for mbufs allocated with m_gethdr/MGETHDR
 * or initialized by M_MOVE_PKTHDR.
 */
#define	MH_ALIGN(m, len) \
	(m)->m_data += (MHLEN - (len)) &~ (sizeof(long) - 1)

/*
 * Determine if an mbuf's data area is read-only. This is true for
 * non-cluster external storage and for clusters that are being
 * referenced by more than one mbuf.
 */
#define	M_READONLY(m)							\
	(((m)->m_flags & M_EXT) != 0 &&					\
	  (((m)->m_flags & M_EXTWR) == 0 || MCLISREFERENCED(m)))

/*
 * Compute the amount of space available
 * before the current start of data in an mbuf.
 */
#define	M_LEADINGSPACE(m) m_leadingspace(m)

/*
 * Compute the amount of space available
 * after the end of data in an mbuf.
 */
#define	M_TRAILINGSPACE(m) m_trailingspace(m)

/*
 * Arrange to prepend space of size plen to mbuf m.
 * If a new mbuf must be allocated, how specifies whether to wait.
 * If how is M_DONTWAIT and allocation fails, the original mbuf chain
 * is freed and m is set to NULL.
 */
#define	M_PREPEND(m, plen, how) \
		(m) = m_prepend((m), (plen), (how))

/* length to m_copy to copy all */
#define	M_COPYALL	1000000000

/*
 * Mbuf statistics.
 * For statistics related to mbuf and cluster allocations, see also the
 * pool headers (mbpool and mclpool).
 */
struct mbstat {
	u_long	_m_spare;	/* formerly m_mbufs */
	u_long	_m_spare1;	/* formerly m_clusters */
	u_long	_m_spare2;	/* spare field */
	u_long	_m_spare3;	/* formely m_clfree - free clusters */
	u_long	m_drops;	/* times failed to find space */
	u_long	m_wait;		/* times waited for space */
	u_long	m_drain;	/* times drained protocols for space */
	u_short	m_mtypes[256];	/* type specific mbuf allocations */
};

#ifdef	_KERNEL

extern	struct mbstat mbstat;
extern	int nmbclust;			/* limit on the # of clusters */
extern	int mblowat;			/* mbuf low water mark */
extern	int mcllowat;			/* mbuf cluster low water mark */
extern	int max_linkhdr;		/* largest link-level header */
extern	int max_protohdr;		/* largest protocol header */
extern	int max_hdr;			/* largest link+protocol header */

void	mbinit(void);
struct	mbuf *m_copym2(struct mbuf *, int, int, int);
struct	mbuf *m_copym(struct mbuf *, int, int, int);
struct	mbuf *m_free(struct mbuf *);
struct	mbuf *m_get(int, int);
struct	mbuf *m_getclr(int, int);
struct	mbuf *m_gethdr(int, int);
struct	mbuf *m_inithdr(struct mbuf *);
int	      m_defrag(struct mbuf *, int);
struct	mbuf *m_prepend(struct mbuf *, int, int);
struct	mbuf *m_pulldown(struct mbuf *, int, int, int *);
struct	mbuf *m_pullup(struct mbuf *, int);
struct	mbuf *m_split(struct mbuf *, int, int);
struct  mbuf *m_inject(struct mbuf *, int, int, int);
struct  mbuf *m_getptr(struct mbuf *, int, int *);
int	m_leadingspace(struct mbuf *);
int	m_trailingspace(struct mbuf *);
struct mbuf *m_clget(struct mbuf *, int, u_int);
void	m_extref(struct mbuf *, struct mbuf *);
void	m_extfree_pool(caddr_t, u_int, void *);
void	m_adj(struct mbuf *, int);
int	m_copyback(struct mbuf *, int, int, const void *, int);
void	m_freem(struct mbuf *);
void	m_reclaim(void *, int);
void	m_copydata(struct mbuf *, int, int, caddr_t);
void	m_cat(struct mbuf *, struct mbuf *);
struct mbuf *m_devget(char *, int, int);
int	m_apply(struct mbuf *, int, int,
	    int (*)(caddr_t, caddr_t, unsigned int), caddr_t);
int	m_dup_pkthdr(struct mbuf *, struct mbuf *, int);

/* Packet tag routines */
struct m_tag *m_tag_get(int, int, int);
void	m_tag_prepend(struct mbuf *, struct m_tag *);
void	m_tag_delete(struct mbuf *, struct m_tag *);
void	m_tag_delete_chain(struct mbuf *);
struct m_tag *m_tag_find(struct mbuf *, int, struct m_tag *);
struct m_tag *m_tag_copy(struct m_tag *, int);
int	m_tag_copy_chain(struct mbuf *, struct mbuf *, int);
void	m_tag_init(struct mbuf *);
struct m_tag *m_tag_first(struct mbuf *);
struct m_tag *m_tag_next(struct mbuf *, struct m_tag *);

/* Packet tag types */
#define PACKET_TAG_IPSEC_IN_DONE	0x0001  /* IPsec applied, in */
#define PACKET_TAG_IPSEC_OUT_DONE	0x0002  /* IPsec applied, out */
#define PACKET_TAG_GIF			0x0040  /* GIF processing done */
#define PACKET_TAG_GRE			0x0080  /* GRE processing done */
#define PACKET_TAG_DLT			0x0100 /* data link layer type */
#define PACKET_TAG_PF_DIVERT		0x0200 /* pf(4) diverted packet */
#define PACKET_TAG_PF_REASSEMBLED	0x0800 /* pf reassembled ipv6 packet */
#define PACKET_TAG_SRCROUTE		0x1000 /* IPv4 source routing options */
#define PACKET_TAG_TUNNEL		0x2000	/* Tunnel endpoint address */

#define MTAG_BITS \
    ("\20\1IPSEC_IN_DONE\2IPSEC_OUT_DONE\3IPSEC_IN_CRYPTO_DONE" \
    "\4IPSEC_OUT_CRYPTO_NEEDED\5IPSEC_PENDING_TDB\6BRIDGE\7GIF\10GRE\11DLT" \
    "\12PF_DIVERT\14PF_REASSEMBLED\15SRCROUTE\16TUNNEL")

/*
 * Maximum tag payload length (that is excluding the m_tag structure).
 * Please make sure to update this value when increasing the payload
 * length for an existing packet tag type or when adding a new one that
 * has payload larger than the value below.
 */
#define PACKET_TAG_MAXSIZE		52

/*
 * mbuf lists
 */

#include <sys/mutex.h>

struct mbuf_list {
	struct mbuf		*ml_head;
	struct mbuf		*ml_tail;
	u_int			ml_len;
};

#define MBUF_LIST_INITIALIZER() { NULL, NULL, 0 }

void			ml_init(struct mbuf_list *);
void			ml_enqueue(struct mbuf_list *, struct mbuf *);
struct mbuf *		ml_dequeue(struct mbuf_list *);
void			ml_requeue(struct mbuf_list *, struct mbuf *);
struct mbuf *		ml_dechain(struct mbuf_list *);
struct mbuf *		ml_filter(struct mbuf_list *,
			    int (*)(void *, const struct mbuf *), void *);

#define	ml_len(_ml)		((_ml)->ml_len)
#define	ml_empty(_ml)		((_ml)->ml_len == 0)

#define MBUF_LIST_FOREACH(_ml, _m) \
	for ((_m) = (_ml)->ml_head; (_m) != NULL; (_m) = (_m)->m_nextpkt)

/*
 * mbuf queues
 */

struct mbuf_queue {
	struct mutex		mq_mtx;
	struct mbuf_list	mq_list;
	u_int			mq_maxlen;
	u_int			mq_drops;
};

#define MBUF_QUEUE_INITIALIZER(_maxlen, _ipl) \
    { MUTEX_INITIALIZER(_ipl), MBUF_LIST_INITIALIZER(), (_maxlen), 0 }

void			mq_init(struct mbuf_queue *, u_int, int);
int			mq_enqueue(struct mbuf_queue *, struct mbuf *);
struct mbuf *		mq_dequeue(struct mbuf_queue *);
int			mq_requeue(struct mbuf_queue *, struct mbuf *);
int			mq_enlist(struct mbuf_queue *, struct mbuf_list *);
void			mq_delist(struct mbuf_queue *, struct mbuf_list *);
struct mbuf *		mq_dechain(struct mbuf_queue *);
struct mbuf *		mq_filter(struct mbuf_queue *,
			    int (*)(void *, const struct mbuf *), void *);

#define	mq_len(_mq)		ml_len(&(_mq)->mq_list)
#define	mq_empty(_mq)		ml_empty(&(_mq)->mq_list)
#define	mq_drops(_mq)		((_mq)->mq_drops)
#define	mq_set_maxlen(_mq, _l)	((_mq)->mq_maxlen = (_l))

#endif /* _KERNEL */
#endif /* _SYS_MBUF_H_ */
