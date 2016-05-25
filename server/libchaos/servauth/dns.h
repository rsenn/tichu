/* chaosircd - pi-networks irc server
 *              
 * Copyright (C) 2003  Roman Senn <smoli@paranoya.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *     
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *     
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * $Id: dns.h,v 1.3 2003/08/28 09:09:18 smoli Exp $
 */

#ifndef SERVAUTH_DNS_H
#define SERVAUTH_DNS_H


/* -------------------------------------------------------------------------
   DNS return values
   ------------------------------------------------------------------------- */

#define DNS_EDONE      1
#define DNS_ESUCCESS   0
#define DNS_ENOSERVER -1
#define DNS_ESYSCALL  -2
#define DNS_ETIMEOUT  -3
#define DNS_ESERVFAIL -4
#define DNS_EMEMORY   -5
#define DNS_EFALLBACK -6
#define DNS_ERECV     -7
#define DNS_ESEND     -8
#define DNS_ECONNECT  -9

/* -------------------------------------------------------------------------
   DNS status values
   ------------------------------------------------------------------------- */
#define DNS_ST_TIMEOUT   -2
#define DNS_ST_ERROR     -1
#define DNS_ST_IDLE       0
#define DNS_ST_START      1      /* not yet connecting
(connect on pre_poll/pre_select) */
#define DNS_ST_CONNECTING 2      /* waiting for connection */
#define DNS_ST_SENT       3      /* query sent, waiting for response */
#define DNS_ST_DONE       4

/* -------------------------------------------------------------------------
   DNS query/response types
   ------------------------------------------------------------------------- */
#define DNS_C_IN    "\0\1"
#define DNS_C_ANY   "\0\377"

#define DNS_T_A     "\0\1"     /* Domain to IPv4 address */
#define DNS_T_NS    "\0\2"     /* Get nameservers of the domain */
#define DNS_T_CNAME "\0\5"     /* Get cname of the domain */
#define DNS_T_SOA   "\0\6"
#define DNS_T_PTR   "\0\14"    /* PTR (reverse lookup) */
#define DNS_T_HINFO "\0\15"
#define DNS_T_MX    "\0\17"    /* Get mail exchanger */
#define DNS_T_TXT   "\0\20"
#define DNS_T_RP    "\0\21"
#define DNS_T_SIG   "\0\30"    /* Get signature record */
#define DNS_T_KEY   "\0\31"    /* Get key record */
#define DNS_T_AAAA  "\0\34"    /* Domain to IPv6 address */
#define DNS_T_AXFR  "\0\374"
#define DNS_T_ANY   "\0\377"

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

#define MAX_SERVERS 16

/* length of domain name pointers */
#define DNS_NAME4_DOMAIN 31
#define DNS_NAME6_DOMAIN (4 * 16 + 10)

#define IP4_FMT 20

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

struct dns_resolver;
typedef void (dns_callback_t)(struct dns_resolver *res);

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

struct dns_instance;

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */
struct dns_resolver
{
  struct dns_resolver *next;
  struct dns_resolver *prev;
  struct dns_instance *instance;
  int                  status;
  int                  sock; /* actually file descriptor + 1 */
  uint64_t             deadline;
  uint64_t             timeout;
  struct dns_server   *ns;
  struct dns_server   *fns;  /* 1st nameserver */
  uint8_t             *query;
  uint8_t             *reply;
  size_t               querylen;
  size_t               replylen;
  size_t               addrindex; /* how many addresses already fetched */
  int                  stype;     /* socket type. SOCK_DGRAM or SOCK_STREAM */
  int                  retries;
  char                 type[2];
  dns_callback_t      *callback;
  struct timer        *timer;
  void                *userarg;
};

/* -----------------------------------------------------------------
   DNS options 
  
   Type Option            Description
   -----------------------------------------------------------------
   bool DNS_IPV6_SUPPORT  Compiled with IPv6 support (read-only).
                          This flag is for binary compatiblity.
   
   bool DNS_IPV6_SOCK     IPv6 address/socket support. Is hardcoded
                          to 0 if DNS_IPV6_SUPPORT is 0. Will be
                          cleared if an IPv6 socket operation returned
                          EAFNOSUPPORT or EPROTONOSUPPORT (IPv4 fall-
                          back). After a fallback all servers without
                          an IPv4 address will be ignored.
                          You can set this to 0 only.
   
   str  DNS_CONFIG        Config file, defaults to "/etc/resolv.conf"
   
   str  DNS_ENV           Environment variable, defaults to "DNSCACHE"
  
   int  DNS_CONNECT       Timeout for socket connection,
                          defaults to 5 seconds.
  
   int  DNS_TIMEOUT       Timeout for DNS query, defaults to 10 seconds.
   
   int  DNS_RETRIES       Number of retries. 
                          -1 == try each server once.
                          -2 == never give up.
   
   int  DNS_PARANOID      Will not give up after receving NXDOMAIN
                          until each server in the list has been tried.
   
   int  DNS_RR_TIME       Will reread DNS_CONFIG and DNS_ENV every n
                          seconds.
                          0 == disable
   
   int  DNS_RR_USES       Will reread DNS_CONFIG and DNS_ENV every n
                          queries.
 
   int  DNS_BIND_LOW      Bottom of random source port range.
   int  DNS_BIND_HIGH     Top of random source port range.
 
   -----------------------------------------------------------------
  
   String options:
  
     void dns_set_option(int, const char *);
     void dns_get_option(int, const char **);
   
   Numeric options:
   
     void dns_set_option(int, int);
     void dns_get_option(int, int *);
   
   Examples:
   
     dns_set_option(DNS_CONFIG, "/etc/dns.conf");
     dns_get_option(DNS_TIMEOUT, &timeout);
 
   ----------------------------------------------------------------- */

/* The actual options */
#define DNS_IPV6_SUPPORT 0
#define DNS_IPV6_SOCK    1
#define DNS_CONFIG       2
#define DNS_ENV          3
#define DNS_CONNECT      4
/*#define DNS_TIMEOUT      5*/
#define DNS_RETRIES      6
#define DNS_PARANOID     7
#define DNS_RR_TIME      8
#define DNS_RR_USES      9
#define DNS_BIND_LOW    10
#define DNS_BIND_HIGH   11

/* Defaults */
#ifdef HAVE_IPV6
#define DNS_DEFAULT_IPV6_SUPPORT 1
#define DNS_DEFAULT_IPV6_SOCK    1
#else
#define DNS_DEFAULT_IPV6_SUPPORT 0
#define DNS_DEFAULT_IPV6_SOCK    0
#endif /* HAVE_IPV6 */
#define DNS_DEFAULT_CONFIG    "/etc/resolv.conf" /* Nameserver config */
#define DNS_DEFAULT_ENV       "DNSCACHE"      /* Additional server from env */
#define DNS_DEFAULT_CONNECT   5               /* 5 seconds connect timeout */
#define DNS_DEFAULT_TIMEOUT   10              /* 10 seconds timeout */
#define DNS_DEFAULT_RETRIES   -1              /* Try each server once */
#define DNS_DEFAULT_PARANOID  0               /* Dont be paranoid! */
#define DNS_DEFAULT_RR_TIME   600             /* Reread every 10 minutes */
#define DNS_DEFAULT_RR_USES   25              /* Reread after 25 uses */
#define DNS_DEFAULT_BIND_LOW  1024
#define DNS_DEFAULT_BIND_HIGH 65535

/* -------------------------------------------------------------------------
   Some helpful macros
   ------------------------------------------------------------------------- */

#define dns_is_idle(dns) (!((struct dns_resolver *)(dns))->sock)
#define dns_is_busy(dns)  (((struct dns_resolver *)(dns))->sock)
#define dns_get_fd(dns)   (((struct dns_resolver *)(dns))->sock - 1)

/* needed on SunOS */
#ifndef s6_addr32
#define s6_addr32 _S6_un._S6_u32
#endif /* s6_addr32 */

/* get the IPv4 part of an IPv6 mapped address expects
   struct in6_addr and writes to a struct in_addr. */
#ifdef IN6_V4MAPPED_TO_INADDR
/* SunOS */
# define IN6_MAPV4 IN6_V4MAPPED_TO_INADDR
#else
/* On Linux and maybe others */
# define IN6_MAPV4(in6, in4) ((in4)->s_addr = (in6)->s6_addr32[3])
#endif

#ifdef IN6_INADDR_TO_V4MAPPED
/* SunOS */
# define IN4_MAPV6 IN6_INADDR_TO_V4MAPPED
#elif defined s6_addr32
/* On Linux and maybe others */
# define IN4_MAPV6(in4, in6) \
  do { \
    (in6)->s6_addr32[3] = (in4)->s_addr; \
    (in6)->s6_addr32[2] = htonl(0x0000ffff); \
    (in6)->s6_addr32[1] = 0; \
    (in6)->s6_addr32[0] = 0; \
  } while(0);
#else
/* oooops */
# error "in_addr to in6_addr conversion macro missing."
#endif

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

extern void                 dns_init         (void);
extern int                  dns_updatedb     (void);
extern void                 dns_dump         (void);
extern int                  dns_errno        (void);
extern const char          *dns_errstr       (void);
extern void                 dns_vset_option  (int                  opt, 
                                              va_list              arg);
extern void                 dns_vget_option  (int                  opt, 
                                              va_list              arg);
extern void                *dns_get_userarg  (struct dns_resolver *res);
extern void                 dns_set_userarg  (struct dns_resolver *res,
                                              void                *userarg);
extern int                  dns_get_status   (struct dns_resolver *res);
extern void                 dns_clear        (struct dns_resolver *res);
extern void                 dns_zero         (struct dns_resolver *res);
extern struct dns_resolver *dns_new          (void);
extern void                 dns_free         (struct dns_resolver *res);
extern void                 dns_set_callback (struct dns_resolver *res,
                                              dns_callback_t      *callback, 
                                              uint64_t             timeout);
extern void                 dns_forall_begin (void);
extern struct dns_resolver *dns_forall_next  (void);
extern int                  dns_name_lookup  (struct dns_resolver *res,
                                              int                  af,
                                              const char          *name, 
                                              uint64_t             t);
extern int                  dns_ptr_lookup   (struct dns_resolver *res,
                                              int                  af, 
                                              void                *ip, 
                                              uint64_t             t);
extern int                  dns_get_addr     (struct dns_resolver *res, 
                                              int                  af, 
                                              void                *addr);
extern int                  dns_get_addr_x   (struct dns_resolver *res,
                                              int                  af, 
                                              void                *addr,
                                              size_t               nelem);
extern int                  dns_get_name     (struct dns_resolver *res,
                                              char                *buf, 
                                              size_t               n);
extern char                *dns_dup_name     (struct dns_resolver *res);

#endif /* SERVAUTH_DNS_H */

