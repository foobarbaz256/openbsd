/*	$OpenBSD: dhcpd.h,v 1.16 2004/02/24 15:35:56 henning Exp $	*/

/*
 * Copyright (c) 2004 Henning Brauer <henning@openbsd.org>
 * Copyright (c) 1995, 1996, 1997, 1998, 1999
 * The Internet Software Consortium.    All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */

#include <sys/types.h>

#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/wait.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <paths.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "dhcp.h"
#include "tree.h"
#include "hash.h"
#include "inet.h"

#define	LOCAL_PORT	68
#define	REMOTE_PORT	67

struct option_data {
	int		 len;
	u_int8_t	*data;
};

struct string_list {
	struct string_list	*next;
	char			*string;
};

struct packet {
	struct dhcp_packet	*raw;
	int			 packet_length;
	int			 packet_type;
	int			 options_valid;
	int			 client_port;
	struct iaddr		 client_addr;
	struct interface_info	*interface;
	struct hardware		*haddr;
	struct option_data	 options[256];
	int			 got_requested_address;
};

struct hardware {
	u_int8_t htype;
	u_int8_t hlen;
	u_int8_t haddr[16];
};

struct client_lease {
	struct client_lease	*next;
	time_t			 expiry, renewal, rebind;
	struct iaddr		 address;
	char			*server_name;
	char			*filename;
	struct string_list	*medium;
	unsigned int		 is_static : 1;
	unsigned int		 is_bootp : 1;
	struct option_data	 options[256];
};

/* Possible states in which the client can be. */
enum dhcp_state {
	S_REBOOTING,
	S_INIT,
	S_SELECTING,
	S_REQUESTING,
	S_BOUND,
	S_RENEWING,
	S_REBINDING
};

struct client_config {
	struct option_data	defaults[256];
	enum {
		ACTION_DEFAULT,
		ACTION_SUPERSEDE,
		ACTION_PREPEND,
		ACTION_APPEND
	} default_actions[256];

	struct option_data	 send_options[256];
	u_int8_t		 required_options[256];
	u_int8_t		 requested_options[256];
	int			 requested_option_count;
	time_t			 timeout;
	time_t			 initial_interval;
	time_t			 retry_interval;
	time_t			 select_interval;
	time_t			 reboot_timeout;
	time_t			 backoff_cutoff;
	struct string_list	*media;
	char			*script_name;
	enum { IGNORE, ACCEPT, PREFER }
				 bootp_policy;
	struct string_list	*medium;
	struct iaddrlist	*reject_list;
};

struct client_state {
	struct client_lease	 *active;
	struct client_lease	 *new;
	struct client_lease	 *offered_leases;
	struct client_lease	 *leases;
	struct client_lease	 *alias;
	enum dhcp_state		  state;
	struct iaddr		  destination;
	u_int32_t		  xid;
	u_int16_t		  secs;
	time_t			  first_sending;
	time_t			  interval;
	struct string_list	 *medium;
	struct dhcp_packet	  packet;
	int			  packet_length;
	struct iaddr		  requested_address;
	struct client_config	 *config;
	char			**scriptEnv;
	int			  scriptEnvsize;
	struct string_list	 *env;
	int			  envc;
};

struct interface_info {
	struct interface_info	*next;
	struct hardware		 hw_address;
	struct in_addr		 primary_address;
	char			 name[IFNAMSIZ];
	int			 rfdesc;
	int			 wfdesc;
	unsigned char		*rbuf;
	size_t			 rbuf_max;
	size_t			 rbuf_offset;
	size_t			 rbuf_len;
	struct ifreq		*ifp;
	u_int32_t		 flags;
#define INTERFACE_REQUESTED 1
#define INTERFACE_AUTOMATIC 2
	struct client_state	*client;
	int			 noifmedia;
	int			 errors;
	int			 dead;
	u_int16_t		 index;
};

struct timeout {
	struct timeout	*next;
	time_t		 when;
	void		 (*func)(void *);
	void		*what;
};

struct protocol {
	struct protocol	*next;
	int fd;
	void (*handler)(struct protocol *);
	void *local;
};

/* Default path to dhcpd config file. */
#define	_PATH_DHCLIENT_CONF	"/etc/dhclient.conf"
#define	_PATH_DHCLIENT_DB	"/var/db/dhclient.leases"
#define	DHCPD_LOG_FACILITY	LOG_DAEMON

#define	MAX_TIME 0x7fffffff
#define	MIN_TIME 0

/* External definitions... */

/* options.c */
void parse_options(struct packet *);
void parse_option_buffer(struct packet *, unsigned char *, int);
int cons_options(struct packet *, struct dhcp_packet *, int,
    struct tree_cache **, int, int, int, u_int8_t *, int);
int store_options(unsigned char *, int, struct tree_cache **,
    unsigned char *, int, int, int, int);
char *pretty_print_option(unsigned int,
    unsigned char *, int, int, int);
void do_packet(struct interface_info *, struct dhcp_packet *,
    int, unsigned int, struct iaddr, struct hardware *);

/* errwarn.c */
extern int warnings_occurred;
void error(char *, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
int warn(char *, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
int note(char *, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
int debug(char *, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
int parse_warn(char *, ...) __attribute__ ((__format__ (__printf__, 1, 2)));

/* conflex.c */
extern int lexline, lexchar;
extern char *token_line, *tlname;
extern char comments[4096];
extern int comment_index;
extern int eol_token;
void new_parse(char *);
int next_token(char **, FILE *);
int peek_token(char **, FILE *);

/* parse.c */
void skip_to_semi(FILE *);
int parse_semi(FILE *);
char *parse_string(FILE *);
char *parse_host_name(FILE *);
int parse_ip_addr(FILE *, struct iaddr *);
void parse_hardware_param(FILE *, struct hardware *);
void parse_lease_time(FILE *, time_t *);
unsigned char *parse_numeric_aggregate(FILE *, unsigned char *, int *,
    int, int, int);
void convert_num(unsigned char *, char *, int, int);
time_t parse_date(FILE *);

/* tree.c */
pair cons(caddr_t, pair);
struct tree_cache *tree_cache(struct tree *);
struct tree *tree_host_lookup(char *);
struct dns_host_entry *enter_dns_host(char *);
struct tree *tree_const(unsigned char *, int);
struct tree *tree_concat(struct tree *, struct tree *);
struct tree *tree_limit(struct tree *, int);
int tree_evaluate(struct tree_cache *);

/* alloc.c */
void *dmalloc(int, char *);
void dfree(void *, char *);
struct tree *new_tree(char *);
struct tree_cache *new_tree_cache(char *);
struct hash_table *new_hash_table(int, char *);
struct hash_bucket *new_hash_bucket(char *);
struct string_list *new_string_list(size_t size, char * name);
void free_hash_bucket(struct hash_bucket *, char *);
void free_hash_table(struct hash_table *, char *);
void free_tree_cache(struct tree_cache *, char *);
void free_tree(struct tree *, char *);
void free_string_list(struct string_list *, char *);

/* print.c */
char *print_hw_addr(int, int, unsigned char *);
void dump_raw(unsigned char *, int);
void dump_packet(struct packet *);
void hash_dump(struct hash_table *);

/* bpf.c */
int if_register_bpf(struct interface_info *);
void if_reinitialize_send(struct interface_info *);
void if_register_send(struct interface_info *);
ssize_t send_packet(struct interface_info *,
    struct packet *, struct dhcp_packet *, size_t, struct in_addr,
    struct sockaddr_in *, struct hardware *);
void if_reinitialize_receive(struct interface_info *);
void if_register_receive(struct interface_info *);
ssize_t receive_packet(struct interface_info *, unsigned char *, size_t,
    struct sockaddr_in *, struct hardware *);
int can_unicast_without_arp(void);
int can_receive_unicast_unconfigured(struct interface_info *);
void maybe_setup_fallback(void);

/* dispatch.c */
extern struct interface_info *interfaces,
    *dummy_interfaces, *fallback_interface;
extern struct protocol *protocols;
extern int quiet_interface_discovery;
extern void (*bootp_packet_handler)(struct interface_info *,
    struct dhcp_packet *, int, unsigned int, struct iaddr, struct hardware *);
extern struct timeout *timeouts;
void discover_interfaces(void);
void reinitialize_interfaces(void);
void dispatch(void);
void got_one(struct protocol *);
void add_timeout(time_t, void (*)(void *), void *);
void cancel_timeout(void (*)(void *), void *);
void add_protocol(char *, int, void (*)(struct protocol *), void *);
void remove_protocol(struct protocol *);
int interface_link_status(char *);

/* hash.c */
struct hash_table *new_hash(void);
void add_hash(struct hash_table *, unsigned char *, int, unsigned char *);
void delete_hash_entry(struct hash_table *, unsigned char *, int);
unsigned char *hash_lookup(struct hash_table *, unsigned char *, int);

/* tables.c */
extern struct option dhcp_options[256];
extern unsigned char dhcp_option_default_priority_list[];
extern int sizeof_dhcp_option_default_priority_list;
extern char *hardware_types[256];
extern struct hash_table universe_hash;
extern struct universe dhcp_universe;
void initialize_universes(void);

/* convert.c */
u_int32_t getULong(unsigned char *);
int32_t getLong(unsigned char *);
u_int16_t getUShort(unsigned char *);
int16_t getShort(unsigned char *);
void putULong(unsigned char *, u_int32_t);
void putLong(unsigned char *, int32_t);
void putUShort(unsigned char *, unsigned int);
void putShort(unsigned char *, int);

/* inet.c */
struct iaddr subnet_number(struct iaddr, struct iaddr);
struct iaddr ip_addr(struct iaddr, struct iaddr, u_int32_t);
struct iaddr broadcast_addr(struct iaddr, struct iaddr);
u_int32_t host_addr(struct iaddr, struct iaddr);
int addr_eq(struct iaddr, struct iaddr);
char *piaddr(struct iaddr);

/* dhclient.c */
extern char *path_dhclient_conf;
extern char *path_dhclient_db;
extern time_t cur_time;
extern int log_priority;
extern int log_perror;

extern struct client_config top_level_config;

void dhcpoffer(struct packet *);
void dhcpack(struct packet *);
void dhcpnak(struct packet *);

void send_discover(void *);
void send_request(void *);
void send_release(void *);
void send_decline(void *);

void state_reboot(void *);
void state_init(void *);
void state_selecting(void *);
void state_requesting(void *);
void state_bound(void *);
void state_panic(void *);

void bind_lease(struct interface_info *);

void make_discover(struct interface_info *, struct client_lease *);
void make_request(struct interface_info *, struct client_lease *);
void make_decline(struct interface_info *, struct client_lease *);
void make_release(struct interface_info *, struct client_lease *);

void free_client_lease(struct client_lease *);
void rewrite_client_leases(void);
void write_client_lease(struct interface_info *, struct client_lease *, int);

void script_init(struct interface_info *, char *, struct string_list *);
void script_write_params(struct interface_info *,
    char *, struct client_lease *);
int script_go(struct interface_info *);
void client_envadd(struct client_state *,
    const char *, const char *, const char *, ...);
void script_set_env(struct client_state *, const char *, const char *,
    const char *);
void script_flush_env(struct client_state *);
int dhcp_option_ev_name(char *, size_t, struct option *);

struct client_lease *packet_to_lease(struct packet *);
void go_daemon(void);
void client_location_changed(void);

void bootp(struct packet *);
void dhcp(struct packet *);
void cleanup(void);

/* packet.c */
u_int32_t checksum(unsigned char *, unsigned, u_int32_t);
u_int32_t wrapsum(u_int32_t);
void assemble_hw_header(struct interface_info *, unsigned char *,
    int *, struct hardware *);
void assemble_udp_ip_header(struct interface_info *, unsigned char *,
    int *, u_int32_t, u_int32_t, unsigned int, unsigned char *, int);
ssize_t decode_hw_header(struct interface_info *, unsigned char *,
    int, struct hardware *);
ssize_t decode_udp_ip_header(struct interface_info *, unsigned char *,
    int, struct sockaddr_in *, unsigned char *, int);

/* ethernet.c */
void assemble_ethernet_header(struct interface_info *, unsigned char *,
    int *, struct hardware *);
ssize_t decode_ethernet_header(struct interface_info *, unsigned char *,
    int, struct hardware *);

/* clparse.c */
int read_client_conf(void);
void read_client_leases(void);
void parse_client_statement(FILE *, struct interface_info *,
    struct client_config *);
int parse_X(FILE *, u_int8_t *, int);
int parse_option_list(FILE *, u_int8_t *);
void parse_interface_declaration(FILE *, struct client_config *);
struct interface_info *interface_or_dummy(char *);
void make_client_state(struct interface_info *);
void make_client_config(struct interface_info *, struct client_config *);
void parse_client_lease_statement(FILE *, int);
void parse_client_lease_declaration(FILE *, struct client_lease *,
    struct interface_info **);
struct option *parse_option_decl(FILE *, struct option_data *);
void parse_string_list(FILE *, struct string_list **, int);
void parse_reject_statement(FILE *, struct client_config *);
