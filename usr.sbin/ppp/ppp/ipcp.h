/*
 *	    Written by Toshiharu OHNO (tony-o@iij.ad.jp)
 *
 *   Copyright (C) 1993, Internet Initiative Japan, Inc. All rights reserverd.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the Internet Initiative Japan.  The name of the
 * IIJ may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $OpenBSD: ipcp.h,v 1.9 2000/03/19 10:33:33 brian Exp $
 *
 *	TODO:
 */

#define	IPCP_MAXCODE	CODE_CODEREJ

#define	TY_IPADDRS	1
#define	TY_COMPPROTO	2
#define	TY_IPADDR	3

/* Domain NameServer and NetBIOS NameServer options */

#define TY_PRIMARY_DNS		129
#define TY_PRIMARY_NBNS		130
#define TY_SECONDARY_DNS	131
#define TY_SECONDARY_NBNS	132
#define TY_ADJUST_NS		119 /* subtract from NS val for REJECT bit */

struct sticky_route;

struct in_range {
  struct in_addr ipaddr;
  struct in_addr mask;
  int width;
};

struct port_range {
  unsigned nports;		/* How many ports */
  unsigned maxports;		/* How many allocated (malloc) ports */
  u_short *port;		/* The actual ports */
};

struct ipcp {
  struct fsm fsm;			/* The finite state machine */

  struct {
    struct {
      int slots;			/* Maximum VJ slots */
      unsigned slotcomp : 1;		/* Slot compression */
      unsigned neg : 2;			/* VJ negotiation */
    } vj;

    struct in_range  my_range;		/* MYADDR spec */
    struct in_addr   netmask;		/* Iface netmask (unused by most OSs) */
    struct in_range  peer_range;	/* HISADDR spec */
    struct iplist    peer_list;		/* Ranges of HISADDR values */

    u_long sendpipe;			/* route sendpipe size */
    u_long recvpipe;			/* route recvpipe size */

    struct in_addr   TriggerAddress;	/* Address to suggest in REQ */
    unsigned HaveTriggerAddress : 1;	/* Trigger address specified */

    struct {
      struct in_addr dns[2];		/* DNS addresses offered */
      unsigned dns_neg : 2;		/* dns negotiation */
      struct in_addr nbns[2];		/* NetBIOS NS addresses offered */
    } ns;

    struct {
      struct port_range tcp, udp;	/* The range of urgent ports */
    } urgent;

    struct fsm_retry fsm;	/* How often/frequently to resend requests */
  } cfg;

  struct {
    struct slcompress cslc;		/* VJ state */
    struct slstat slstat;		/* VJ statistics */
  } vj;

  struct {
    unsigned resolver : 1;		/* Found resolv.conf ? */
    unsigned writable : 1;		/* Can write resolv.conf ? */
    struct in_addr dns[2];		/* Current DNS addresses */
    char *resolv;			/* Contents of resolv.conf */
    char *resolv_nons;			/* Contents of resolv.conf without ns */
  } ns;

  struct sticky_route *route;		/* List of dynamic routes */

  unsigned heis1172 : 1;		/* True if he is speaking rfc1172 */

  struct in_addr peer_ip;		/* IP address he's willing to use */
  u_int32_t peer_compproto;		/* VJ params he's willing to use */

  struct in_addr ifmask;		/* Interface netmask */

  struct in_addr my_ip;			/* IP address I'm willing to use */
  u_int32_t my_compproto;		/* VJ params I'm willing to use */

  struct in_addr dns[2];		/* DNSs to REQ/ACK */

  u_int32_t peer_reject;		/* Request codes rejected by peer */
  u_int32_t my_reject;			/* Request codes I have rejected */

  struct pppThroughput throughput;	/* throughput statistics */
  struct mqueue Queue[3];		/* Output packet queues */
};

#define fsm2ipcp(fp) (fp->proto == PROTO_IPCP ? (struct ipcp *)fp : NULL)
#define IPCP_QUEUES(ipcp) (sizeof ipcp->Queue / sizeof ipcp->Queue[0])

struct bundle;
struct link;
struct cmdargs;

extern void ipcp_Init(struct ipcp *, struct bundle *, struct link *,
                      const struct fsm_parent *);
extern void ipcp_Destroy(struct ipcp *);
extern void ipcp_Setup(struct ipcp *, u_int32_t);
extern void ipcp_SetLink(struct ipcp *, struct link *);

extern int  ipcp_Show(struct cmdargs const *);
extern struct mbuf *ipcp_Input(struct bundle *, struct link *, struct mbuf *);
extern void ipcp_AddInOctets(struct ipcp *, int);
extern void ipcp_AddOutOctets(struct ipcp *, int);
extern int  ipcp_UseHisIPaddr(struct bundle *, struct in_addr);
extern int  ipcp_UseHisaddr(struct bundle *, const char *, int);
extern int  ipcp_vjset(struct cmdargs const *);
extern void ipcp_CleanInterface(struct ipcp *);
extern int  ipcp_InterfaceUp(struct ipcp *);
extern int  ipcp_IsUrgentPort(struct port_range *, u_short, u_short);
extern void ipcp_AddUrgentPort(struct port_range *, u_short);
extern void ipcp_RemoveUrgentPort(struct port_range *, u_short);
extern void ipcp_ClearUrgentPorts(struct port_range *);
extern struct in_addr addr2mask(struct in_addr);
extern int ipcp_WriteDNS(struct ipcp *);
extern void ipcp_RestoreDNS(struct ipcp *);
extern void ipcp_LoadDNS(struct ipcp *);

#define ipcp_IsUrgentTcpPort(ipcp, p1, p2) \
          ipcp_IsUrgentPort(&(ipcp)->cfg.urgent.tcp, p1, p2)
#define ipcp_IsUrgentUdpPort(ipcp, p1, p2) \
          ipcp_IsUrgentPort(&(ipcp)->cfg.urgent.udp, p1, p2)
#define ipcp_AddUrgentTcpPort(ipcp, p) \
          ipcp_AddUrgentPort(&(ipcp)->cfg.urgent.tcp, p)
#define ipcp_AddUrgentUdpPort(ipcp, p) \
          ipcp_AddUrgentPort(&(ipcp)->cfg.urgent.udp, p)
#define ipcp_RemoveUrgentTcpPort(ipcp, p) \
          ipcp_RemoveUrgentPort(&(ipcp)->cfg.urgent.tcp, p)
#define ipcp_RemoveUrgentUdpPort(ipcp, p) \
          ipcp_RemoveUrgentPort(&(ipcp)->cfg.urgent.udp, p)
#define ipcp_ClearUrgentTcpPorts(ipcp) \
          ipcp_ClearUrgentPorts(&(ipcp)->cfg.urgent.tcp)
#define ipcp_ClearUrgentUdpPorts(ipcp) \
          ipcp_ClearUrgentPorts(&(ipcp)->cfg.urgent.udp)
