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
 * $Id: dns.c,v 1.24 2004/12/31 03:39:14 smoli Exp $
 */

#define BSD_COMP

#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/syscall.h>
#include <libchaos/timer.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/net.h>
#include <libchaos/str.h>

#include "dns.h"
#include "control.h"
#include "servauth.h"

#ifndef EPROTO
#define EPROTO EPROTONOSUPPORT
#endif

#define DNS_LINEBUF_SIZE 256

#define DNS_ST_ERROR     -1
#define DNS_ST_IDLE       0
#define DNS_ST_START      1      /* not yet connecting
(connect on pre_poll/pre_select) */
#define DNS_ST_CONNECTING 2      /* waiting for connection */
#define DNS_ST_SENT       3      /* query sent, waiting for response */
#define DNS_ST_DONE       4

/* Prototypes for private functions */

static struct dns_server *dns_add_ns(const char *name);
static struct dns_server *dns_get_ns(int af);
static int dns_read_conf(const char *filename);
static int dns_error(int error);
#ifdef HAVE_IPV6
static void dns_fallback(struct dns_resolver *res);
#endif /* HAVE_IPV6 */
static int  dns_socket(struct dns_resolver *res, int type);
static int  dns_bind(struct dns_resolver *res);
static int  dns_connect(struct dns_resolver *res);
static int  dns_send(struct dns_resolver *res);
static void dns_event_rd(int fd, void *ptr);
static void dns_event_cn(int fd, void *ptr);
static int  dns_start(struct dns_resolver *res, int type);
static int  dns_transmit_start(struct dns_resolver *res, const uint8_t *domain,
                               size_t len, const char *type);

/* -------------------------------------------------------------------------
   Used when converting IPv6 addresses to PTRs
   ------------------------------------------------------------------------- */
#ifdef HAVE_IPV6
static const uint8_t dns_hexchars[] = "0123456789abcdef";
#endif /* HAVE_IPV6 */
/* -------------------------------------------------------------------------
   Error messages
   ------------------------------------------------------------------------- */
static const char *dns_errstrs[] = {
  "Success",
  "No domain name servers available",
  "Underlying syscall error",
  "Timed out",
  "Server failed",
  "Insufficient memory",
  "Fallback to IPv4 sockets",
  "Receive error",
  "Send error",
  "Could not connect"
};

/* -------------------------------------------------------------------------
   Used for global respective per-instance and per-server statistics 
   ------------------------------------------------------------------------- */
struct dns_stat {
  uint32_t errors;           /* socket errors */
  uint32_t queries;          /* queries sent */
  uint32_t responses;        /* responses received */
  uint32_t sendb;            /* bytes sent */
  uint32_t recvb;            /* bytes received */
};

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */
struct dns_server {
  int             af;
  struct in_addr  addr;       /* Will contain IPv4 address when af = AF_INET
                                 or af = AF_INET6 and it is a mapped address */
#ifdef HAVE_IPV6
  struct in6_addr addr6;      /* Will contain IPv4 mapped address
                                 when af = AF_INET */
#endif /* HAVE_IPV6 */
  time_t          reactivate; /* When this is set, the server has been
                                 deactivated because requests timed out.
                                 Will contain the time of reactivation. */
  struct dns_stat stat;       /* Per server statistics */
};

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */
struct dns_options {
  int         ipv6_support;
  int         ipv6_sock;
  const char *config;
  const char *env;
  int         connect;
  int         timeout;
  int         retries;
  int         paranoid;
  int         rr_time;
  int         rr_uses;        /* Times this instance was used */
  int         bind_high;      /* Upper border of source port range */
  int         bind_low;       /* Lower border of source port range */
};

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */
struct dns_instance {
  struct dns_server    servers[MAX_SERVERS];
  uint32_t             server_count;
  struct dns_stat      stats;
  struct dns_options   options;
  struct dns_server   *ns;
  struct dns_resolver *res;
  struct dns_resolver *cur;
  uint64_t             deadline;
  int                  __dns__errno__;
  int                  error;
};

/* -------------------------------------------------------------------------
   Non-reentrant (default) instance 
   ------------------------------------------------------------------------- */
struct dns_instance dns;
#define inst (&dns)

/*****************************************************************************/
/**                           Utility functions                             **/
/*****************************************************************************/

#ifdef HAVE_IPV6
static void dns_fallback(struct dns_resolver *res)
{
}
#endif /* HAVE_IPV6 */

/* -------------------------------------------------------------------------
   This multi-purpose function can generate 2 types of domain name pointers
   ------------------------------------------------------------------------- */

static size_t dns_dn_ptr(uint8_t *dst, int af, void *ip)
{
/* -------------------------------------------------------------------------
   Convert IPv4 address to domain style
  
   192.168.100.1 -> 1.100.168.192.in-addr.arpa
     
   Returns number of bytes written to *dst.
   ------------------------------------------------------------------------- */

  if(af == AF_INET)
  {
    int i;
    uint8_t *p = dst;
    uint8_t *addrp = (uint8_t *)&((struct in_addr *)ip)->s_addr;
    uint8_t tmp;

    /* Somewhat optimized (but still sucking divisions :P) */
    for(i = 3; i >= 0; i--)
    {
      tmp = addrp[i];

      /* 3 Chars */
      if(tmp > 99)
      {
        *p++ = 3;
        *p++ = '0' + (tmp / 100);
        *p++ = '0' + (tmp % 100 / 10);
        *p++ = '0' + (tmp % 10);
        /* 2 Chars */
      }
      else if(tmp > 9)
      {
        *p++ = 2;
        *p++ = '0' + (tmp % 100 / 10);
        *p++ = '0' + (tmp % 10);
        /* 1 Char */
      }
      else
      {
        *p++ = 1;
        *p++ = '0' + (tmp % 10);
      }
    }

    memcpy(p, "\7in-addr\4arpa", 14);

    return p - dst + 14;
  }
  
/* -------------------------------------------------------------------------
   Convert IPv6 address to domain style
     
                        4321:0:1:2:3:4:567:89ab
                                  ->
   b.a.9.8.7.6.5.0.4.0.0.0.3.0.0.0.2.0.0.0.1.0.0.0.0.0.0.0.1.2.3.4.ip6.int.
     
   Returns number of bytes written to *dst.
   ------------------------------------------------------------------------- */

#ifdef HAVE_IPV6
  else if(af == AF_INET6)
  {
    int i;
    struct in6_addr *addr = ip;
    uint8_t *addrp;

    for(i = 3; i >= 0; i--)
    {
      addrp = (uint8_t *)&addr->s6_addr32[i];

#ifdef _BIG_ENDIAN
      *dst++ = 1; *dst++ = dns_hexchars[ addr->s6_addr32[i]        & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 4)  & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 8)  & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 12) & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 16) & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 20) & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 24) & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 28) & 0x0f];
#else
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 24) & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 28) & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 16) & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 20) & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 8)  & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 12) & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[ addr->s6_addr32[i]        & 0x0f];
      *dst++ = 1; *dst++ = dns_hexchars[(addr->s6_addr32[i] >> 4)  & 0x0f];
#endif
    }

    memcpy(dst, "\3ip6\3int", 9);

    return 4 * 16 + 9; /* Hello DJB! ;) */
  }
#endif /* HAVE_IPV6 */

  return 0;
}

/* -------------------------------------------------------------------------
   The following 2 functions are endian-independant.
   They just act like a memcpy() with a size of 2
   on systems where network byte order == host byte order.
   ------------------------------------------------------------------------- */

static inline void dns_uint16_unpack(const uint8_t *in, uint16_t *out)
{
  *out = ((uint16_t)(in[0] << 8)) | in[1];
}

static inline void dns_uint16_pack(uint8_t *out, uint16_t in)
{
  out[0] = in >> 8;
  out[1] = in & 255;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

static size_t dns_packet_copy(const uint8_t *buf, size_t len,
                              size_t pos, uint8_t *out, size_t outlen)
{
  while(outlen--)
  {
    if(pos >= len) return 0;

    *out++ = buf[pos++];
  }

  return pos;
}

/* -------------------------------------------------------------------------
   [buf] points to the start address of a valid domain style name.
   [buf + return value] will point to past the name.
   ------------------------------------------------------------------------- */

static size_t dns_packet_skipname(const uint8_t *buf, size_t len, size_t pos)
{
  uint8_t ch;

  while(pos < len)
  {
    ch = buf[pos++];

    if(ch >= 192)
      return pos + 1;

    if(ch >= 64)
      break;

    if(ch == 0)
      return pos;

    pos += ch;
  }

  return 0;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

static size_t dns_packet_getname(const uint8_t *buf, size_t len, size_t pos,
                                 uint8_t *domain, size_t n)
{
  uint32_t loop = 0;
  uint32_t state = 0;
  uint32_t firstcompress = 0;
  uint32_t where;
  uint8_t  ch;
  uint8_t  name[256];
  size_t   namelen = 0;

  for(;;)
  {
    if(pos >= len)
      goto proto;

    ch = buf[pos++];

    if(++loop >= 1000)
      goto proto;

    if(state)
    {
      if(namelen > sizeof(name) - 1)
        goto proto;

      name[namelen++] = ch;
      state--;
    }
    else
    {
      while(ch >= 192)
      {
        where = ch;
        where -= 192;
        where <<= 8;

        if(pos >= len)
          goto proto;

        ch = buf[pos++];

        if(!firstcompress) firstcompress = pos;

        pos = where + ch;

        if(pos >= len)
          goto proto;

        ch = buf[pos++];

        if(++loop >= 1000)
          goto proto;
      }

      if(ch >= 64)
        goto proto;

      if(namelen > sizeof(name) - 1)
        goto proto;

      name[namelen++] = ch;

      if(!ch)
        break;

      state = ch;
    }
  }

  if(n)
    strlcpy((char *)domain, (char *)name, n - 1);
  else
    *domain = 0;

  if(firstcompress)
    return firstcompress;

  return pos;

proto:
  syscall_errno = EPROTO;
  return 0;
}

/* -------------------------------------------------------------------------
   Gets the length of a domain in DNS style format
   ------------------------------------------------------------------------- */

static __inline__ size_t dns_dn_len(const uint8_t *domain)
{
  const uint8_t *p;
  uint8_t c;

  p = domain;

  while((c = *p++))
    p += c;

  return p - domain;
}

/* -------------------------------------------------------------------------
   Similar to memcmp() but case insensitive
   ------------------------------------------------------------------------- */

static int dns_cmp(const uint8_t *s1, const uint8_t *s2, size_t len)
{
  register uint8_t c1;
  register uint8_t c2;

  while(len--)
  {
    c1 = *s1++;
    c2 = *s2++;

    if(c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
    if(c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';

    if(c1 != c2)
      return 1;
  }

  return 0;
}

/* -------------------------------------------------------------------------
   Compares two names in domain style
   ------------------------------------------------------------------------- */

static int dns_dn_cmp(const uint8_t *domain1, const uint8_t *domain2)
{
  uint32_t len;

  len = dns_dn_len(domain1);

  if(len != dns_dn_len(domain2))
    return 0;

  if(dns_cmp(domain1, domain2, len))
    return 0;

  return 1;
}

/* -------------------------------------------------------------------------
   Convert domain style to dotted.
   ------------------------------------------------------------------------- */

static size_t dns_dn_to_dot(char *out, const uint8_t *d, size_t n)
{

  char ch;
  char ch2;
  int idx = 0;

  if(!n)
    return 0;

  if(!*d)
  {
    *out = '\0';
    return 0;
  }

  for(;idx < n;)
  {
    ch = *d++;

    while(ch--)
    {
      ch2 = *d++;

      if((ch2 >= 'A') && (ch2 <= 'Z'))
        ch2 += 0x20;

      if(((ch2 >= 'a') && (ch2 <= 'z')) ||
         ((ch2 >= '0') && (ch2 <= '9')) ||
          (ch2 == '-') || (ch2 == '_'))
      {
        out[idx++] = ch2;

        if(idx == n)
          break;
      }
    }

    if(!*d || idx == n)
      break;

    out[idx++] = '.';
  }

  out[idx] = '\0';

  return idx;
}

/* -------------------------------------------------------------------------
   Convert dotted style to domain.
   ------------------------------------------------------------------------- */

static size_t dns_dot_to_dn(uint8_t *out, const char *d, size_t n)
{
  char     label[63];
  uint32_t labellen = 0; /* <= sizeof label */
  char     name[255];
  uint32_t namelen = 0; /* <= sizeof name */
  char     ch;

  while(*d)
  {
    ch = *d++;

    if(ch == '.')
    {
      if(labellen)
      {
        if(namelen + labellen + 1 > sizeof(name))
          return 0;

        name[namelen++] = labellen;
        memcpy(&name[namelen], label, labellen);
        namelen += labellen;
        labellen = 0;
      }

      continue;
    }

    /* convert octal shit */
    if(ch == '\\')
    {
      if(!n--)
        break;

      ch = *d++;

      if((ch >= '0') && (ch <= '7'))
      {
        ch -= '0';

        if(n && (*d >= '0') && (*d <= '7'))
        {
          ch <<= 3;
          ch += *d - '0';
          d++; n--;

          if(n && (*d >= '0') && (*d <= '7'))
          {
            ch <<= 3;
            ch += *d - '0';
            d++; n--;
          }
        }
      }
    }

    if(labellen >= sizeof(label))
      return 0;

    if(namelen > n)
      break;

    label[labellen++] = ch;
  }

  if(labellen)
  {
    if(namelen + labellen + 1 > sizeof(name)) return 0;
    name[namelen++] = labellen;
    memcpy(&name[namelen], label, labellen);
    namelen += labellen;
    labellen = 0;
  }

  if(namelen + 1 > sizeof(name)) return 0;
  name[namelen++] = '\0';

  memcpy(out, name, n);

  return namelen;
}

/****************************************************************************/
/**                         Database functions                             **/
/****************************************************************************/

/* -------------------------------------------------------------------------
   Add a nameserver to the DB
   
   Name must be a valid IPv4 or (when compiled with IPv6) IPv6 address.
   
   When dns_server->af is set to AF_INET, then at least
   dns_server->addr will be valid.
   When dns_server->af is set to AF_INET6, then at least
   dns_server->addr6 will be valid.
   
   Unused dns_server structs are always marked with
   INADDR_ANY respective in6addr_any.
   
   Special cases (when compiled with HAVE_IPV6):
    - If its an IPv4 to IPv6 mapped address, both members 
      (addr & addr6) of the dns_server struct will be valid.
    - If its localhost both members (addr & addr6)
      of the dns_server struct will be valid.
    - If ipv6_sock in options struct is zero then IPv6
      addresses other than IPv4 mapped ones will be invalid.
 
   Returns NULL on error or a pointer to a valid dns_server struct.
   If the nameserver already exists a pointer to it will be returned.
   ------------------------------------------------------------------------- */

static struct dns_server *dns_add_ns(const char *name)
{
  int i, af = 0;
  struct in_addr addr;
  struct dns_server *ret = NULL;
#ifdef HAVE_IPV6
  struct in6_addr addr6;
#endif /* HAVE_IPV6 */

  /* Convert address */
  /*#ifdef HAVE_INET_PTON
    if(inet_pton(AF_INET, name, &addr) > 0)
  #else*/
  if(net_aton(name, &addr) > 0)
    /*#endif HAVE_INET_PTON */
    af = AF_INET;
  else
    addr.s_addr = INADDR_ANY;

#ifdef HAVE_IPV6
  if(net_pton(AF_INET6, name, &addr6) > 0)
  {
    /* When we don't support ipv6 sockets only
       accept it when its mapped or localhost. */
    if(!inst->options.ipv6_sock)
    {
      af = AF_INET;

      if(IN6_IS_ADDR_V4MAPPED(&addr6))
        IN6_MAPV4(&addr6, &addr);
      else if(!memcmp(&addr6, &in6addr_loopback, sizeof(struct in6_addr)))
        addr.s_addr = htonl(INADDR_LOOPBACK);
      else
        /* Ahwww!! Not mapped or localhost, we have an error */
        return NULL;
    }
    else
      af = AF_INET6;
  }
  else
    memcpy(&addr6, &in6addr_any, sizeof(struct in6_addr));
#endif /* HAVE_IPV6 */

  if(!af)
    return NULL;

  /* Find a free dns_server struct */
  for(i = 0; i < MAX_SERVERS; i++)
  {
    /* If not yet found a free place, look for one */
    if(ret == NULL && inst->servers[i].af == 0)
    {
      ret = &inst->servers[i];
      break;
    }

    /* Does this nameserver already exist? */
    if(af == AF_INET && inst->servers[i].addr.s_addr == addr.s_addr)
      return NULL;

#ifdef HAVE_IPV6
    if(af == AF_INET6 &&
        !memcmp(&inst->servers[i].addr6, &addr6, sizeof(struct in6_addr)))
      return NULL;
#endif /* HAVE_IPV6 */

  }

  if(!ret)
    return NULL;

  /* Found a free one, fill it */
  if(af == AF_INET)
  {
    ret->addr.s_addr = addr.s_addr;

#ifdef HAVE_IPV6
    /* If its loopback, then copy IPv6 loopback address */
    if(ntohl(addr.s_addr) == INADDR_LOOPBACK)
      memcpy(&ret->addr6, &in6addr_loopback, sizeof(struct in6_addr));
    /* Otherwise just map it to IPv6 */
    else
      IN4_MAPV6(&addr, &ret->addr6);
  }

  if(af == AF_INET6)
  {
    /* Copy IPv6 to the DNS server struct */
    memcpy(&ret->addr6, &addr6, sizeof(struct in6_addr));

    /* If its a mapped address, then copy it to IPv4 address */
    if(IN6_IS_ADDR_V4MAPPED(&addr6))
    {
      af = AF_INET;
      IN6_MAPV4(&addr6, &ret->addr);
    }

    /* If its loopback, then copy it to IPv4 address */
    if(!memcmp(&ret->addr6, &in6addr_loopback, sizeof(struct in6_addr)))
      ret->addr.s_addr = htonl(INADDR_LOOPBACK);
#endif /* HAVE_IPV6 */

  }

  /* Copy address family to the DNS server struct */
  ret->af = af;
  inst->server_count++;

  return ret;
}

/* -------------------------------------------------------------------------
   Clears a nameserver by setting it to INADDR_ANY respective in6addr_any
   ------------------------------------------------------------------------- */

void dns_clear_ns(struct dns_server *ns)
{
  ns->af = 0;
  ns->addr.s_addr = INADDR_ANY;

#ifdef HAVE_IPV6
  memcpy(&ns->addr6, &in6addr_any, sizeof(struct in6_addr));
#endif /* HAVE_IPV6 */
}

/* -------------------------------------------------------------------------
   Get next DNS server.
 
   When af == AF_INET then the next server with a valid IPv4 address
   will be returned.
   When af == AF_INET6 then the next server with a valid IPv6 address
   will be returned.
   When ns is NULL then the first valid DNS server will be returned.
   ------------------------------------------------------------------------- */

static struct dns_server *dns_get_ns(int af)
{
  struct dns_server *ret = NULL;
  int i;

  for(i = 0; i < MAX_SERVERS; i++)
  {
    if(af == AF_INET && inst->servers[i].addr.s_addr != INADDR_ANY)
    {
      ret = &inst->servers[i];
      break;
    }
  }
  /* WAAAAAAAARNING */
  ret = &inst->servers[0];

  return ret;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */
static int dns_timeout(void *arg)
{
  struct dns_resolver *res = arg;
  
  if(res->callback)
    res->callback(arg);
  
  if(res->timer)
  {
    timer_remove(res->timer);
    res->timer = NULL;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------
   Tries to get nameservers from supplied file.
   Each nameserver has to be on its own line.
   Supports IPv4 and IPv6 (if compiled with IPv6) addresses.
   
   Returns number of nameservers added.
   ------------------------------------------------------------------------- */

static void dns_read_line(int fd, void *arg)
{
  char buf[DNS_LINEBUF_SIZE];
  int ret;
  char *p;
  size_t i = 0;

  for(EVER)
  {
    ret = io_gets(fd, buf, DNS_LINEBUF_SIZE);
    
    if(ret <= 0)
      break;
    
    while(isspace(buf[i]))
      i++;
    
    if(!strncmp("nameserver", &buf[i], 10))
    {
      i += 10;
      
      while(isspace(buf[i]))
        i++;
      
      if(!isdigit(buf[i]))
        return;

      p = strchr(&buf[i], '\n');
      
      if(p)
        *p = '\0';
      
      dns_add_ns(&buf[i]);
    }
  }
  
  io_close(fd);
}

static int dns_read_conf(const char *filename)
{
  int fd;

  fd = io_open(filename, O_RDONLY);

  if(fd == -1)
    return -1;

  io_queue_control(fd, ON, OFF, ON);

  io_register(fd, IO_CB_READ, dns_read_line, NULL);

  return 1;
}

/* -------------------------------------------------------------------------
   Check for next re-read of config file, env vars and stuff.
   If deadline has been exceeded there then do it!
   
   Returns positive value if DB was updated (number of servers in DB)
   otherwise 0.
   ------------------------------------------------------------------------- */

int dns_updatedb()
{
  int ret = 0;

  inst->server_count = 0;

  /* Config file was supplied, read it */
  if(inst->options.config)
    ret += dns_read_conf(inst->options.config);

  /* Add localhost if there's no valid server */
  if(!ret)
#ifdef HAVE_IPV6
    if(dns_add_ns("::1"))
#else
    if(dns_add_ns("127.0.0.1"))
#endif /* HAVE_IPV6 */
      ret++;

  return ret;
}

/*****************************************************************************/
/**                           Instance managing                             **/
/*****************************************************************************/

/* -------------------------------------------------------------------------
   Initialize a DNS instance.
   
   Zeroes the instance.
   Removes (zeroes) all dns_servers.
   Initializes options to their defaults.   
   ------------------------------------------------------------------------- */

void dns_init()
{
  int i;

  /* Clear the DNS instance */
  memset(inst, 0, sizeof(struct dns_instance));

  /* Set all dns_server structs to INADDR_ANY */
  for(i = 0; i < MAX_SERVERS; i++)
    dns_clear_ns(&inst->servers[i]);

  /* Set options to a sane default */
  inst->options.ipv6_support = DNS_DEFAULT_IPV6_SUPPORT;
  inst->options.ipv6_sock    = DNS_DEFAULT_IPV6_SOCK;
  inst->options.config       = DNS_DEFAULT_CONFIG;
  inst->options.env          = DNS_DEFAULT_ENV;
  inst->options.timeout      = DNS_DEFAULT_TIMEOUT;
  inst->options.retries      = DNS_DEFAULT_RETRIES;
  inst->options.paranoid     = DNS_DEFAULT_PARANOID;
  inst->options.rr_time      = DNS_DEFAULT_RR_TIME;
  inst->options.rr_uses      = DNS_DEFAULT_RR_USES;
  inst->options.bind_low     = DNS_DEFAULT_BIND_LOW;
  inst->options.bind_high    = DNS_DEFAULT_BIND_HIGH;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

static void dns_set_syscall_errno()
{
  dns.stats.errors++;
  dns.__dns__errno__ = syscall_errno;
  syscall_errno = 0;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

int dns_syscall_errno()
{
  return inst->__dns__errno__;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

const char *dns_errstr()
{
  return dns_errstrs[0 - inst->error];
}

/* -------------------------------------------------------------------------
   -1 = error has been set, 0 = retry
   ------------------------------------------------------------------------- */

static int dns_error(int error)
{
  dns_set_syscall_errno();

  inst->error = error;

  return error;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

void dns_vset_option(int opt, va_list arg)
{
  switch(opt)
  {
      case DNS_IPV6_SOCK:
      if(va_arg(arg, int) == 0)
        inst->options.ipv6_sock = 0;
      break;
      case DNS_CONFIG:
      inst->options.config    = va_arg(arg, const char *); break;
      case DNS_ENV:
      inst->options.env       = va_arg(arg, const char *); break;
      case DNS_CONNECT:
      inst->options.connect   = va_arg(arg, int); break;
      case DNS_TIMEOUT:
      inst->options.timeout   = va_arg(arg, int); break;
      case DNS_RETRIES:
      inst->options.retries   = va_arg(arg, int); break;
      case DNS_PARANOID:
      inst->options.paranoid  = va_arg(arg, int); break;
      case DNS_RR_TIME:
      inst->options.rr_time   = va_arg(arg, int); break;
      case DNS_RR_USES:
      inst->options.rr_uses   = va_arg(arg, int); break;
      case DNS_BIND_LOW:
      inst->options.bind_low  = va_arg(arg, int); break;
      case DNS_BIND_HIGH:
      inst->options.bind_high = va_arg(arg, int); break;
  }
}

void dns_set_option(int opt, ...)
{
  va_list arg;

  va_start(arg, opt);

  dns_vset_option(opt, arg);

  va_end(arg);
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

void dns_vget_option(int opt, va_list arg)
{
  const char **cp;
  int *ip;

  switch(opt)
  {
      case DNS_IPV6_SOCK:
      ip = va_arg(arg, int *);
      if(ip) *ip = inst->options.ipv6_sock;
      break;
      case DNS_CONFIG:
      cp = va_arg(arg, const char **);
      if(cp) *cp = inst->options.config;
      break;
      case DNS_ENV:
      cp = va_arg(arg, const char **);
      if(cp) *cp = inst->options.env;
      break;
      case DNS_CONNECT:
      ip = va_arg(arg, int *);
      if(ip) *ip = inst->options.connect;
      break;
      case DNS_TIMEOUT:
      ip = va_arg(arg, int *);
      if(ip) *ip = inst->options.timeout;
      break;
      case DNS_RETRIES:
      ip = va_arg(arg, int *);
      if(ip) *ip = inst->options.retries;
      break;
      case DNS_PARANOID:
      ip = va_arg(arg, int *);
      if(ip) *ip = inst->options.paranoid;
      break;
      case DNS_RR_TIME:
      ip = va_arg(arg, int *);
      if(ip) *ip = inst->options.rr_time;
      break;
      case DNS_RR_USES:
      ip = va_arg(arg, int *);
      if(ip) *ip = inst->options.rr_uses;
      break;
      case DNS_BIND_HIGH:
      ip = va_arg(arg, int *);
      if(ip) *ip = inst->options.bind_high;
      break;
      case DNS_BIND_LOW:
      ip = va_arg(arg, int *);
      if(ip) *ip = inst->options.bind_low;
      break;
  }
}

void dns_get_option(int opt, ...)
{
  va_list arg;

  va_start(arg, opt);

  dns_vget_option(opt, arg);

  va_end(arg);
}

/*****************************************************************************/
/**                   Packet assembly and parsing functions                 **/
/*****************************************************************************/

/* -------------------------------------------------------------------------
   Checks whether the response matches the query
   ------------------------------------------------------------------------- */

static int dns_irrelevant(struct dns_resolver *res, uint8_t *buf, size_t len)
{
  uint8_t  header[12];
  uint8_t  domain[256];
  uint32_t pos;

  res->addrindex = 0;

  pos = dns_packet_copy(buf, len, 0, header, 12);

  if(pos == 0)
    return 1;

  if(memcmp(header, &res->query[2], 2))
    return 1;

  if(header[4] != 0)
    return 1;

  if(header[5] != 1)
    return 1;

  pos = dns_packet_getname(buf, len, pos, domain, sizeof(domain));

  if(pos == 0)
    return 1;

  if(!dns_dn_cmp(domain, &res->query[14]))
    return 1;

  pos = dns_packet_copy(buf, len, pos, header, 4);

  if(pos == 0)
    return 1;

  if(memcmp(header, res->type, 2))
    return 1;

  if(memcmp(&header[2], DNS_C_IN, 2))
    return 1;

  return 0;
}

/* -------------------------------------------------------------------------
   Extracts a name from a response packet
   ------------------------------------------------------------------------- */

static int dns_extract_name(struct dns_resolver *res, char *out, size_t len)
{
  uint32_t pos;
  uint8_t  header[12];
  uint16_t numans;
  uint16_t datalen;
  uint8_t  name[256];

  pos = dns_packet_copy(res->reply, res->replylen, 0, header, 12);

  if(!pos)
    return -1;

  dns_uint16_unpack(&header[6], &numans);

  pos = dns_packet_skipname(res->reply, res->replylen, pos);

  if(!pos)
    return -1;

  pos += 4;

  while(numans--)
  {
    pos = dns_packet_skipname(res->reply, res->replylen, pos);

    if(!pos)
      return -1;

    pos = dns_packet_copy(res->reply, res->replylen, pos, header, 10);

    if(!pos)
      return -1;

    dns_uint16_unpack(&header[8], &datalen);

    if(!memcmp(&header[0], DNS_T_PTR, 2))
      if(!memcmp(&header[2], DNS_C_IN, 2))
      {
        if(!dns_packet_getname(res->reply, res->replylen, pos, name, 255))
          return -1;

        if(!dns_dn_to_dot(out, name, len))
          return -1;

        return 1;
      }

    pos += datalen;
  }

  return 0;
}

/* -------------------------------------------------------------------------
   Extracts n'th address from a response packet
   ------------------------------------------------------------------------- */

static int dns_extract_addr(struct dns_resolver *res, 
                            int af, void *addr, size_t n)
{
  uint32_t pos;
  uint8_t header[12];
  uint16_t numans;
  uint16_t len;
  size_t count = 0;

  pos = dns_packet_copy(res->reply, res->replylen, 0, header, 12);

  if(!pos)
    return -1;

  dns_uint16_unpack(&header[6], &numans);

  pos = dns_packet_skipname(res->reply, res->replylen, pos);

  if(!pos)
    return -1;

  pos += 4;

  if(n >= numans)
    return 0;

  while(numans--)
  {
    pos = dns_packet_skipname(res->reply, res->replylen, pos);

    if(!pos)
      return 0;

    pos = dns_packet_copy(res->reply, res->replylen, pos, header, 10);

    if(!pos)
      return 0;

    dns_uint16_unpack(&header[8], &len);

    if(!memcmp(&header[0], DNS_T_A, 2) &&
        !memcmp(&header[2], DNS_C_IN, 2) &&
        len == sizeof(struct in_addr))
    {

      if(!dns_packet_copy(res->reply, res->replylen, pos, header,
                          sizeof(struct in_addr)))
        return 0;

      if(count == n)
      {
        *(uint32_t *)addr = *(uint32_t *)header;
        return 1;
      }

      count++;
    }
#ifdef HAVE_IPV6
    if(!memcmp(&header[0], DNS_T_AAAA, 2) && !memcmp(&header[2], DNS_C_IN, 2) &&
        len == sizeof(struct in6_addr))
    {
      if(!dns_packet_copy(res->reply, res->replylen, pos, header,
                          sizeof(struct in6_addr)))
        return 0;

      if(count == n)
      {
        *(struct in6_addr *)addr = *(struct in6_addr *)header;
        return 1;
      }

      count++;
    }
#endif /* HAVE_IPV6 */

    pos += len;
  }

  return 0;
}

/*****************************************************************************/
/**                             Network functions                           **/
/*****************************************************************************/

/* -------------------------------------------------------------------------
   Server wants TCP?
   ------------------------------------------------------------------------- */

static int dns_want_tcp(const uint8_t *buf, size_t len)
{
  uint8_t header[12];

  if(!dns_packet_copy(buf, len, 0, header, 12))
    return 1;

  if(header[2] & 0x02)
    return 1;

  return 0;
}

/* -------------------------------------------------------------------------
   Server failed?
   ------------------------------------------------------------------------- */

static int dns_fail(const uint8_t *buf, size_t len)
{
  uint8_t  header[12];
  uint32_t rcode;

  if(!dns_packet_copy(buf, len, 0, header, 12))
    return 1;

  rcode = header[3] & 0x0f;

  if(rcode && (rcode != 3))
    return 1;

  return 0;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

static int dns_transmit_start(struct dns_resolver *res, const uint8_t *domain,
                              size_t len, const char *type)
{
  int stype;

  /* Allocate some space for the query */
  res->querylen = len + 18;
  res->query = malloc(res->querylen);

  if(res->query == NULL)
    return dns_error(DNS_EMEMORY);

  /* Set length in header */
  dns_uint16_pack(res->query, len + 16);

  /* Always do recursive query */
  memcpy(&res->query[2], "\0\0\1\0\0\1\0\0\0\0\0\0", 12);

  /* Copy domain */
  memcpy(&res->query[14], domain, len);

  /* Copy query type to packet and resolver struct */
  res->type[0] = res->query[len + 14] = type[0];
  res->type[1] = res->query[len + 15] = type[1];

  /* DNS_C_IN */
  res->query[len + 16] = 0x00;
  res->query[len + 17] = 0x01;

  /* Update statistics */
  inst->stats.queries++;

  /* Get the first server */
#ifdef HAVE_IPV6
  if(inst->options.ipv6_sock)
    res->ns = dns_get_ns(AF_INET6);
  else
#else
    res->ns = dns_get_ns(AF_INET);
#endif /* HAVE_IPV6 */

    /* Error, no nameserver */
    if(res->ns == NULL)
      return DNS_ENOSERVER;

  /* If the packet is any bigger than
     512 bytes start a TCP transmission */
  if(len + 16 > 512)
    stype = SOCK_STREAM;
  else
    stype = SOCK_DGRAM;

  return dns_start(res, stype);
}

/*****************************************************************************/
/**                             I/O functions                               **/
/*****************************************************************************/

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */
static void dns_close(struct dns_resolver *res)
{
  if(res->sock)
  {
    io_close(res->sock - 1);
    res->sock = 0;
  }
}

/* -------------------------------------------------------------------------
   Bah, not really I/O
   ------------------------------------------------------------------------- */

static void dns_free_query(struct dns_resolver *res)
{
  if(res->query)
  {
    free(res->query);
    res->query = NULL;
    res->querylen = 0;
  }
}

static void dns_free_reply(struct dns_resolver *res)
{
  if(res->reply)
  {
    free(res->reply);
    res->reply = NULL;
    res->replylen = 0;
  }
}

void dns_clear(struct dns_resolver *res)
{
  dns_close(res);

  dns_free_query(res);
  dns_free_reply(res);

  timer_push(&res->timer);
  
  dns_zero(res);
}

/* -------------------------------------------------------------------------
   Create IPv4 or IPv6 socket depending on the current server.  
   Will do IPv4 fallback on error.
   ------------------------------------------------------------------------- */

static int dns_socket(struct dns_resolver *res, int type)
{
  int fd = -1;

  if(res->ns == NULL)
    return dns_error(DNS_ENOSERVER);

  if(res->ns->af == AF_INET)
  {
    fd = net_socket(PF_INET, type);

    if(fd == -1)
      return dns_error(DNS_ESYSCALL);

    res->stype = type;
    res->sock = fd + 1;

    return 0;

  }
#ifdef HAVE_IPV6
  else if(!inst->options.ipv6_sock && res->ns->af == AF_INET6)
  {
    int flags;
    
    fd = net_socket(PF_INET, type);

    if(fd == -1)
    {
      if(syscall_errno == EAFNOSUPPORT ||
          syscall_errno == EPROTONOSUPPORT)
      {
        dns_fallback(res);
        return dns_error(DNS_EFALLBACK);
      }

      return dns_error(DNS_ESYSCALL);
    }

    flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);

    res->stype = type;
    res->sock = fd + 1;

    return 0;
  }
#endif /* HAVE_IPV6 */

  syscall_errno = EAFNOSUPPORT;

  return dns_error(DNS_ESYSCALL);
}

/* -------------------------------------------------------------------------
   When we are connecting to 127.0.0.1 respective ::1 we will bind to 
   localhost otherwise we'll bind to INADDR_ANY respective in6_addr.
   ------------------------------------------------------------------------- */

static int dns_bind(struct dns_resolver *res)
{
  int low;
  int high;
  int port;

  if(res->ns == NULL)
    return dns_error(DNS_ESYSCALL);

  if(res->ns->af == AF_INET)
  {
    struct in_addr addr;

    low = inst->options.bind_low;
    high = inst->options.bind_high;
    port = low + rand() % (high - low);

    if(res->ns->addr.s_addr == INADDR_LOOPBACK)
      addr.s_addr = INADDR_LOOPBACK;
    else
      addr.s_addr = INADDR_ANY;
    
    return net_bind(res->sock - 1, addr, port);
#ifdef HAVE_IPV6
  }
  else if(!inst->options.ipv6_sock && res->ns->af == AF_INET6)
  {
    struct sockaddr_in6 local;

    low = inst->options.bind_low;
    high = inst->options.bind_high;
    port = low + rand() % (high - low);

    local.sin6_family = AF_INET6;
    local.sin6_port = htons(port);

    if(!memcmp(&res->ns->addr6, &in6addr_loopback, sizeof(struct in6_addr)))
      memcpy(&local.sin6_addr, &in6addr_loopback, sizeof(struct in6_addr));
    else
      memcpy(&local.sin6_addr, &in6addr_any, sizeof(struct in6_addr));

    if(bind(res->sock - 1, (struct sockaddr *)&local, sizeof(local)) == -1)
      return dns_error(DNS_ESYSCALL);

    return 0;
#endif /* HAVE_IPV6 */

  }

  syscall_errno = EAFNOSUPPORT;

  return dns_error(DNS_ESYSCALL);
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */
static int dns_connect(struct dns_resolver *res)
{
  if(res->ns->af == AF_INET)
  {
    struct sockaddr_in local;
    void *cb;

    local.sin_family = AF_INET;
    
    cb = (res->stype == SOCK_STREAM ? &dns_event_cn : NULL);
    
    return net_connect(res->sock - 1, res->ns->addr, 53, cb, cb, res);
  }
#ifdef HAVE_IPV6
  else if(!inst->options.ipv6_sock && res->ns->af == AF_INET6)
  {
    struct sockaddr_in6 local;

    local.sin6_family = AF_INET6;
    local.sin6_port   = htons(53);
    memcpy(&local.sin6_addr, &res->ns->addr6, sizeof(struct in6_addr));

    if(connect(res->sock-1, (struct sockaddr *)&local, sizeof(local)) == -1)
      return dns_error(DNS_ESYSCALL);

    return 0;
  }
#endif /* HAVE_IPV6 */

  syscall_errno = EAFNOSUPPORT;

  return dns_error(DNS_ESYSCALL);
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */
static int dns_send(struct dns_resolver *res)
{
  /* Just in case a send() on an UDP socket would block we treat
     it like a TCP socket waiting for a connection.
     (Shouldn't happen but you never know) */
  if(io_write(res->sock - 1, &res->query[2], res->querylen - 2) == -1)
  {
    if(syscall_errno == EAGAIN || syscall_errno == EWOULDBLOCK)
    {
      syscall_errno = 0;
      return 0;
    }

    return dns_error(DNS_ESEND);
  }

  res->status = DNS_ST_SENT;

  return 0;
}

/* -------------------------------------------------------------------------
   When socket gets writeable, send out pending stuff and change status
   to DNS_ST_SENT 
   ------------------------------------------------------------------------- */
static void dns_event_cn(int fd, void *ptr)
{
  struct dns_resolver *res = ptr;  
  
/*  if(!io_list[fd].status.connected)
  {
    if(res->callback)
      res->callback(ptr);
  }
  else*/
  {
    if(dns_send(res) == -1)
    {
      dns_start(res, res->stype);
      
      return;
    }
    
    res->status = DNS_ST_SENT;
  }
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

static void dns_event_rd(int fd, void *ptr)
{
  struct dns_resolver *res = ptr;
  
  if(res->stype == SOCK_DGRAM)
  {
    uint8_t buf[513];
    int     ret;

    ret = recv(res->sock - 1, buf, sizeof(buf), 0);

    if(ret <= 0)
    {
      dns_error(DNS_ERECV);
      
      return;
    }

    if(ret > sizeof(buf) - 1 || dns_irrelevant(res, buf, ret))
    {
      dns_error(DNS_ESERVFAIL);
      
      return;
    }

    if(dns_want_tcp(buf, ret))
    {
      dns_start(res, SOCK_STREAM);
      
      return;
    }

    if(dns_fail(buf, ret))
      dns_close(res);

    res->replylen = ret;

    if(res->reply)
      free(res->reply);

    res->reply = malloc(ret);

    if(res->reply == NULL)
    {
      dns_error(DNS_EMEMORY);
      
      return;
    }

    memcpy(res->reply, buf, ret);

    dns_free_query(res);

    res->status = DNS_ST_DONE;

    if(res->callback)
      res->callback(res);
    
    return;

    /*dns_start(res, res->stype);*/
  }

  return;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

static int dns_start(struct dns_resolver *res, int type)
{
  int i;
  int err;
  int af;

  res->query[2] = rand() & 0xff;
  res->query[3] = rand() & 0xff;
  
#ifdef HAVE_IPV6
  af = inst->options.ipv6_sock ? AF_INET6 : AF_INET;
#else
  af = AF_INET;
#endif /* HAVE_IPV6 */
  
  res->ns = dns_get_ns(af);

  if(res->ns == NULL)
    return dns_error(DNS_ENOSERVER);
again:
  err = dns_socket(res, type);

  /* Fallback happened, just get another socket */
  if(err == DNS_EFALLBACK)
    goto again;

  if(err != DNS_ESUCCESS)
    return err;

  /* Bind the socket to a random local port (UDP only) */
  if(type == SOCK_DGRAM)
  {
    for(i = 0; i < 10; i++)
      if(dns_bind(res) == DNS_ESUCCESS)
        break;

    if(i == 10)
      return dns_error(DNS_ESYSCALL);
  }

  res->status = DNS_ST_START;

  dns_connect(res);

  if(res->stype == SOCK_DGRAM)
  {
    if(res->status != DNS_ST_SENT)
      dns_send(res);
    
    io_register(res->sock - 1, IO_CB_READ, dns_event_rd, res);
  }
  
  return DNS_ESUCCESS;
}

/*****************************************************************************
 *                           PUBLIC INTERFACE                                *
 *****************************************************************************/

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */
void dns_zero(struct dns_resolver *res)
{
  memset(res, 0, sizeof(struct dns_resolver));
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */
int dns_get_status(struct dns_resolver *res)
{
  return res->status;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */
void *dns_get_userarg(struct dns_resolver *res)
{
  return res->userarg;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */
void dns_set_userarg(struct dns_resolver *res, void *userarg)
{
  res->userarg = userarg;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */
void dns_set_callback(struct dns_resolver *res, dns_callback_t *callback,
                      uint64_t timeout)
{
  res->timeout = timeout;
  res->callback = callback;
  res->timer = timer_start(dns_timeout, timeout, res);
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

struct dns_resolver *dns_new()
{
  struct dns_resolver *res;
  struct dns_resolver *r;

  res = malloc(sizeof(struct dns_resolver));

  if(res == NULL)
    return NULL;

  dns_zero(res);

  res->next = NULL;

  if(inst->res == NULL)
  {
    inst->res = res;
    res->prev = NULL;
  }
  else
  {
    for(r = inst->res; r->next; r = r->next);

    r->next = res;
    res->prev = r;
  }

  res->instance = inst;

  return res;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

void dns_free(struct dns_resolver *res)
{
  if(res->prev == NULL)
  {
    inst->res = res->next;

    if(inst->res != NULL)
      inst->res->prev = NULL;

  }
  else
  {
    res->prev->next = res->next;

    if(res->next != NULL)
      res->next->prev = res->prev;
  }

  dns_clear(res);
  free(res);
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

void dns_forall_begin()
{
  inst->cur = inst->res;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

struct dns_resolver *dns_forall_next()
{
  struct dns_resolver *ret;

  ret = inst->cur;

  if(ret != NULL)
    inst->cur = ret->next;

  return ret;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

/*int dns_prepare(struct dns_resolver *res)
{
  int ret;

  if(*timeout > (res->deadline - t))
    *timeout = (res->deadline - t);

  if(res->sock)
  {
    if(res->sock - 1 > *hsck)
      *hsck = res->sock - 1;

    FD_SET(res->sock - 1, rfds);

    if(res->status == DNS_ST_START)
    {
      if((ret = dns_connect(res)) != DNS_ESUCCESS)
        return ret;

      if(res->stype == SOCK_DGRAM && res->status != DNS_ST_SENT)
      {
        if((ret = dns_send(res)) != DNS_ESUCCESS)
          return ret;

        res->status = DNS_ST_SENT;
      }
      else
      {
        FD_SET(res->sock - 1, wfds);
        res->status = DNS_ST_CONNECTING;
      }
    }

    if(res->status == DNS_ST_CONNECTING)
      FD_SET(res->sock - 1, wfds);

    return DNS_ESUCCESS;
  }

  return DNS_ESUCCESS;
}*/

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

/*int dns_post_select(struct dns_resolver *res, const fd_set *rfds,
                    const fd_set *wfds, time_t t)
{
  if(res->sock)
  {
    if(FD_ISSET(res->sock - 1, rfds))
    {
      return dns_event_rd(res);

    }
    else if(FD_ISSET(res->sock - 1, wfds))
    {
      return dns_event_wr(res);

    }
    else
    {
      if(res->status == DNS_ST_CONNECTING)
        syscall_errno = DNS_ECONNECT;
      if(res->status == DNS_ST_SENT)
        syscall_errno = DNS_ETIMEOUT;

      dns_close(res);

      dns_set_syscall_errno();
      return dns_start(res, res->stype);
    }
  }

  if(t >= res->deadline)
  {
    res->status = DNS_ST_TIMEOUT;
    return 1;
  }

  return DNS_EDONE;
}*/

/*****************************************************************************/
/**                         Lookup functions                                **/
/*****************************************************************************/

/* -------------------------------------------------------------------------
   Start a DNS_T_PTR lookup.
   Valid address families (af) are AF_INET and AF_INET6.
   (Corresponding to struct in_addr respective in6_addr for ip)
 
   Returns:  1 when lookup is done (caching not yet implemented).
             0 when lookup is in progress.
            -1 on error
   ------------------------------------------------------------------------- */

int dns_ptr_lookup(struct dns_resolver *res, int af, void *ip, uint64_t t)
{
  uint8_t ptr[256];
  size_t  len;

  inst->stats.queries++;

  /* Build a domain name pointer */
  len = dns_dn_ptr(ptr, af, ip);

  res->deadline = t + DNS_TIMEOUT;

  /* Start the transmission */
  return dns_transmit_start(res, ptr, len, DNS_T_PTR);
}

/* -------------------------------------------------------------------------
   Start a DNS_T_A[AAA] lookup.
   Valid address families (af) are AF_INET and AF_INET6.
   (Corresponding to struct in_addr respective in6_addr for ip)
 
   Returns:  1 when lookup is done (caching not yet implemented).
             0 when lookup is in progress.
            -1 on error
   ------------------------------------------------------------------------- */

int dns_name_lookup(struct dns_resolver *res, int af, 
                    const char *name, uint64_t t)
{
  uint8_t domain[256];
  size_t len;
  const char *type;

  inst->stats.queries++;

  type = (af == AF_INET ? DNS_T_A : DNS_T_AAAA);

  len = dns_dot_to_dn(domain, name, 256);

  res->deadline = t + DNS_TIMEOUT;

  /* Start the transmission */
  return dns_transmit_start(res, domain, len, type);
}

/*****************************************************************************/
/**                      Data retrieval functions                           **/
/*****************************************************************************/

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

int dns_get_name(struct dns_resolver *res, char *buf, size_t n)
{
  return dns_extract_name(res, buf, n);
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

char *dns_dup_name(struct dns_resolver *res)
{
  char name[256];
  char *ret = NULL;

  if(dns_extract_name(res, name, 256) == 1)
    ret = strdup(name);

  return ret;
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

int dns_get_addr(struct dns_resolver *res, int af, void *addr)
{
  return dns_extract_addr(res, af, addr, res->addrindex++);
}

/* -------------------------------------------------------------------------
   ------------------------------------------------------------------------- */

int dns_get_addr_x(struct dns_resolver *res, int af, void *addr, size_t nelem)
{
  size_t n;

  for(n = 0; n < nelem; n++)
  {
    if(dns_extract_addr(res, af, addr, res->addrindex++) <= 0)
      return n;
  }

  return 0;
}

