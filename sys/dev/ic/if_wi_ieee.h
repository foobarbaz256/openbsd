/*	$OpenBSD: if_wi_ieee.h,v 1.2 2001/06/07 04:58:20 mickey Exp $	*/

/*
 * Copyright (c) 1997, 1998, 1999
 *	Bill Paul <wpaul@ctr.columbia.edu>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Bill Paul.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bill Paul AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bill Paul OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	From: if_wavelan_ieee.h,v 1.1 1999/05/05 07:36:50 wpaul Exp $
 */

#ifndef _IF_WI_IEEE_H
#define _IF_WI_IEEE_H

/*
 * This header defines a simple command interface to the FreeBSD
 * WaveLAN/IEEE driver (wi) driver, which is used to set certain
 * device-specific parameters which can't be easily managed through
 * ifconfig(8). No, sysctl(2) is not the answer. I said a _simple_
 * interface, didn't I.
 */

#ifndef SIOCSWAVELAN
#ifdef __FreeBSD__
#define SIOCSWAVELAN	SIOCSIFGENERIC
#else	/* !__FreeBSD__ */
#define SIOCSWAVELAN	SIOCSIFASYNCMAP
#endif	/* __FreeBSD__ */
#endif

#ifndef SIOCGWAVELAN
#ifdef __FreeBSD__
#define SIOCGWAVELAN	SIOCGIFGENERIC
#else	/* !__FreeBSD__ */
#define SIOCGWAVELAN	SIOCGIFASYNCMAP
#endif	/* __FreeBSD__ */
#endif

/*
 * Technically I don't think there's a limit to a record
 * length. The largest record is the one that contains the CIS
 * data, which is 240 words long, so 256 should be a safe
 * value.
 */
#define WI_MAX_DATALEN	512

struct wi_req {
	u_int16_t	wi_len;
	u_int16_t	wi_type;
	u_int16_t	wi_val[WI_MAX_DATALEN];
};

/*
 * Private LTV records (interpreted only by the driver). This is
 * a minor kludge to allow reading the interface statistics from
 * the driver.
 */
#define WI_RID_IFACE_STATS	0x0100
#define WI_RID_MGMT_XMIT	0x0200

struct wi_80211_hdr {
	u_int16_t		frame_ctl;
	u_int16_t		dur_id;
	u_int8_t		addr1[6];
	u_int8_t		addr2[6];
	u_int8_t		addr3[6];
	u_int16_t		seq_ctl;
	u_int8_t		addr4[6];
};

#define WI_FCTL_VERS		0x0002
#define WI_FCTL_FTYPE		0x000C
#define WI_FCTL_STYPE		0x00F0
#define WI_FCTL_TODS		0x0100
#define WI_FCTL_FROMDS		0x0200
#define WI_FCTL_MOREFRAGS	0x0400
#define WI_FCTL_RETRY		0x0800
#define WI_FCTL_PM		0x1000
#define WI_FCTL_MOREDATA	0x2000
#define WI_FCTL_WEP		0x4000
#define WI_FCTL_ORDER		0x8000

#define WI_FTYPE_MGMT		0x0000
#define WI_FTYPE_CTL		0x0004
#define WI_FTYPE_DATA		0x0008

#define WI_STYPE_MGMT_ASREQ	0x0000	/* association request */
#define WI_STYPE_MGMT_ASRESP	0x0010	/* association response */
#define WI_STYPE_MGMT_REASREQ	0x0020	/* reassociation request */
#define WI_STYPE_MGMT_REASRESP	0x0030	/* reassociation response */
#define WI_STYPE_MGMT_PROBEREQ	0x0040	/* probe request */
#define WI_STYPE_MGMT_PROBERESP	0x0050	/* probe response */
#define WI_STYPE_MGMT_BEACON	0x0080	/* beacon */
#define WI_STYPE_MGMT_ATIM	0x0090	/* announcement traffic ind msg */
#define WI_STYPE_MGMT_DISAS	0x00A0	/* disassociation */
#define WI_STYPE_MGMT_AUTH	0x00B0	/* authentication */
#define WI_STYPE_MGMT_DEAUTH	0x00C0	/* deauthentication */

struct wi_mgmt_hdr {
	u_int16_t		frame_ctl;
	u_int16_t		duration;
	u_int8_t		dst_addr[6];
	u_int8_t		src_addr[6];
	u_int8_t		bssid[6];
	u_int16_t		seq_ctl;
};

struct wi_counters {
	u_int32_t		wi_tx_unicast_frames;
	u_int32_t		wi_tx_multicast_frames;
	u_int32_t		wi_tx_fragments;
	u_int32_t		wi_tx_unicast_octets;
	u_int32_t		wi_tx_multicast_octets;
	u_int32_t		wi_tx_deferred_xmits;
	u_int32_t		wi_tx_single_retries;
	u_int32_t		wi_tx_multi_retries;
	u_int32_t		wi_tx_retry_limit;
	u_int32_t		wi_tx_discards;
	u_int32_t		wi_rx_unicast_frames;
	u_int32_t		wi_rx_multicast_frames;
	u_int32_t		wi_rx_fragments;
	u_int32_t		wi_rx_unicast_octets;
	u_int32_t		wi_rx_multicast_octets;
	u_int32_t		wi_rx_fcs_errors;
	u_int32_t		wi_rx_discards_nobuf;
	u_int32_t		wi_tx_discards_wrong_sa;
	u_int32_t		wi_rx_WEP_cant_decrypt;
	u_int32_t		wi_rx_msg_in_msg_frags;
	u_int32_t		wi_rx_msg_in_bad_msg_frags;
};

/*
 * These are all the LTV record types that we can read or write
 * from the WaveLAN. Not all of them are temendously useful, but I
 * list as many as I know about here for completeness.
 */

#define WI_RID_DNLD_BUF		0xFD01
#define WI_RID_MEMSZ		0xFD02
#define WI_RID_DOMAINS		0xFD11
#define WI_RID_CIS		0xFD13
#define WI_RID_COMMQUAL		0xFD43
#define WI_RID_SCALETHRESH	0xFD46
#define WI_RID_PCF		0xFD87

/*
 * Network parameters, static configuration entities.
 */
#define	WI_RID_PORTTYPE		0xFC00 /* Connection control characteristics */
#define	WI_RID_MAC_NODE		0xFC01 /* MAC address of this station */
#define	WI_RID_DESIRED_SSID	0xFC02 /* Service Set ID for connection */
#define	WI_RID_OWN_CHNL		0xFC03 /* Comm channel for BSS creation */
#define	WI_RID_OWN_SSID		0xFC04 /* IBSS creation ID */
#define	WI_RID_OWN_ATIM_WIN	0xFC05 /* ATIM window time for IBSS creation */
#define	WI_RID_SYSTEM_SCALE	0xFC06 /* scale that specifies AP density */
#define	WI_RID_MAX_DATALEN	0xFC07 /* Max len of MAC frame body data */
#define	WI_RID_MAC_WDS		0xFC08 /* MAC addr of corresponding WDS node */
#define	WI_RID_PM_ENABLED	0xFC09 /* ESS power management enable */
#define	WI_RID_PM_EPS		0xFC0A /* PM EPS/PS mode */
#define	WI_RID_MCAST_RX		0xFC0B /* ESS PM mcast reception */
#define	WI_RID_MAX_SLEEP	0xFC0C /* max sleep time for ESS PM */
#define	WI_RID_HOLDOVER		0xFC0D /* holdover time for ESS PM */
#define	WI_RID_NODENAME		0xFC0E /* ID name of this node for diag */
#define	WI_RID_DTIM_PERIOD	0xFC10 /* beacon interval between DTIMs */
#define	WI_RID_WDS_ADDR1	0xFC11 /* port 1 MAC of WDS link node */
#define	WI_RID_WDS_ADDR2	0xFC12 /* port 1 MAC of WDS link node */
#define	WI_RID_WDS_ADDR3	0xFC13 /* port 1 MAC of WDS link node */
#define	WI_RID_WDS_ADDR4	0xFC14 /* port 1 MAC of WDS link node */
#define	WI_RID_WDS_ADDR5	0xFC15 /* port 1 MAC of WDS link node */
#define	WI_RID_WDS_ADDR6	0xFC16 /* port 1 MAC of WDS link node */
#define	WI_RID_MCAST_PM_BUF	0xFC17 /* PM buffering of mcast */
#define	WI_RID_ENCRYPTION	0xFC20 /* enable/disable WEP */
#define	WI_RID_AUTHTYPE		0xFC21 /* specify authentication type */
#define	WI_RID_P2_TX_CRYPT_KEY	0xFC23
#define	WI_RID_P2_CRYPT_KEY0	0xFC24
#define	WI_RID_P2_CRYPT_KEY1	0xFC25
#define	WI_RID_MICROWAVE_OVEN	0xFC25
#define	WI_RID_P2_CRYPT_KEY2	0xFC26
#define	WI_RID_P2_CRYPT_KEY3	0xFC27
#define	WI_RID_P2_ENCRYPTION	0xFC28
#define	WI_RID_WEP_MAPTABLE	0xFC29
#define	WI_RID_AUTH_CNTL	0xFC2A
#define	WI_RID_ROAMING_MODE	0xFC2D
#define	WI_RID_BASIC_RATE	0xFCB3
#define	WI_RID_SUPPORT_RATE	0xFCB4

/*
 * Network parameters, dynamic configuration entities
 */
#define WI_RID_MCAST_LIST	0xFC80 /* list of multicast addrs */
#define WI_RID_CREATE_IBSS	0xFC81 /* create IBSS */
#define WI_RID_FRAG_THRESH	0xFC82 /* frag len, unicast msg xmit */
#define WI_RID_RTS_THRESH	0xFC83 /* frame len for RTS/CTS handshake */
#define WI_RID_TX_RATE		0xFC84 /* data rate for message xmit */
#define WI_RID_PROMISC		0xFC85 /* enable promisc mode */
#define WI_RID_FRAG_THRESH0	0xFC90
#define WI_RID_FRAG_THRESH1	0xFC91
#define WI_RID_FRAG_THRESH2	0xFC92
#define WI_RID_FRAG_THRESH3	0xFC93
#define WI_RID_FRAG_THRESH4	0xFC94
#define WI_RID_FRAG_THRESH5	0xFC95
#define WI_RID_FRAG_THRESH6	0xFC96
#define WI_RID_RTS_THRESH0	0xFC97
#define WI_RID_RTS_THRESH1	0xFC98
#define WI_RID_RTS_THRESH2	0xFC99
#define WI_RID_RTS_THRESH3	0xFC9A
#define WI_RID_RTS_THRESH4	0xFC9B
#define WI_RID_RTS_THRESH5	0xFC9C
#define WI_RID_RTS_THRESH6	0xFC9D
#define WI_RID_TX_RATE0		0xFC9E
#define WI_RID_TX_RATE1		0xFC9F
#define WI_RID_TX_RATE2		0xFCA0
#define WI_RID_TX_RATE3		0xFCA1
#define WI_RID_TX_RATE4		0xFCA2
#define WI_RID_TX_RATE5		0xFCA3
#define WI_RID_TX_RATE6		0xFCA4
#define WI_RID_DEFLT_CRYPT_KEYS	0xFCB0
#define WI_RID_TX_CRYPT_KEY	0xFCB1
#define WI_RID_TICK_TIME	0xFCE0
#define WI_RID_SCAN_REQ		0xFCE1
#define WI_RID_JOIN_REQ		0xFCE2

struct wi_key {
	u_int16_t		wi_keylen;
	u_int8_t		wi_keydat[14];
};

struct wi_ltv_keys {
	u_int16_t		wi_len;
	u_int16_t		wi_type;
	struct wi_key		wi_keys[4];
};

/*
 * NIC information
 */
#define WI_RID_FIRM_ID		0xFD02 /* Primary func firmware ID. */
#define WI_RID_PRI_SUP_RANGE	0xFD03 /* primary supplier compatibility */
#define WI_RID_CIF_ACT_RANGE	0xFD04 /* controller sup. compatibility */
#define WI_RID_SERIALNO		0xFD0A /* card serial number */
#define WI_RID_CARD_ID		0xFD0B /* card identification */
#define WI_RID_MFI_SUP_RANGE	0xFD0C /* modem supplier compatibility */
#define WI_RID_CFI_SUP_RANGE	0xFD0D /* controller sup. compatibility */
#define WI_RID_CHANNEL_LIST	0xFD10 /* allowd comm. frequencies. */
#define WI_RID_REG_DOMAINS	0xFD11 /* list of intendted regulatory doms */
#define WI_RID_TEMP_TYPE	0xFD12 /* hw temp range code */
#define WI_RID_CIS		0xFD13 /* PC card info struct */
#define WI_RID_STA_IDENEITY	0xFD20 /* station funcs firmware ident */
#define WI_RID_STA_SUP_RANGE	0xFD21 /* station supplier compat */
#define WI_RID_MFI_ACT_RANGE	0xFD22
#define WI_RID_CFI_ACT_RANGE	0xFD33

/*
 * MAC information
 */
#define WI_RID_PORT_STAT	0xFD40 /* actual MAC port con control stat */
#define WI_RID_CURRENT_SSID	0xFD41 /* ID of actually connected SS */
#define WI_RID_CURRENT_BSSID	0xFD42 /* ID of actually connected BSS */
#define WI_RID_COMMS_QUALITY	0xFD43 /* quality of BSS connection */
#define WI_RID_CUR_TX_RATE	0xFD44 /* current TX rate */
#define WI_RID_OWN_BEACON_INT	0xFD45 /* beacon xmit time for BSS creation */
#define WI_RID_CUR_SCALE_THRESH	0xFD46 /* actual system scane thresh setting */
#define WI_RID_PROT_RESP_TIME	0xFD47 /* time to wait for resp to req msg */
#define WI_RID_SHORT_RTR_LIM	0xFD48 /* max tx attempts for short frames */
#define WI_RID_LONG_RTS_LIM	0xFD49 /* max tx attempts for long frames */
#define WI_RID_MAX_TX_LIFE	0xFD4A /* max tx frame handling duration */
#define WI_RID_MAX_RX_LIFE	0xFD4B /* max rx frame handling duration */
#define WI_RID_CF_POLL		0xFD4C /* contention free pollable ind */
#define WI_RID_AUTH_ALGS	0xFD4D /* auth algorithms available */
#define WI_RID_AUTH_TYPE	0xFD4E /* availanle auth types */
#define WI_RID_WEP_AVAIL	0xFD4F /* WEP privacy option available */
#define WI_RID_CUR_TX_RATE1	0xFD80
#define WI_RID_CUR_TX_RATE2	0xFD81
#define WI_RID_CUR_TX_RATE3	0xFD82
#define WI_RID_CUR_TX_RATE4	0xFD83
#define WI_RID_CUR_TX_RATE5	0xFD84
#define WI_RID_CUR_TX_RATE6	0xFD85
#define WI_RID_OWN_MAC		0xFD86 /* unique local MAC addr */
#define WI_RID_PCI_INFO		0xFD87 /* point coordination func cap */

/*
 * Modem information
 */
#define WI_RID_PHY_TYPE		0xFDC0 /* phys layer type indication */
#define WI_RID_CURRENT_CHAN	0xFDC1 /* current frequency */
#define WI_RID_PWR_STATE	0xFDC2 /* pwr consumption status */
#define WI_RID_CCA_MODE		0xFDC3 /* clear chan assess mode indication */
#define WI_RID_CCA_TIME		0xFDC4 /* clear chan assess time */
#define WI_RID_MAC_PROC_DELAY	0xFDC5 /* MAC processing delay time */
#define WI_RID_DATA_RATES	0xFDC6 /* supported data rates */

#endif
