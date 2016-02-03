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
 * $Id: query.c,v 1.10 2004/12/31 03:39:14 smoli Exp $
 */

#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/timer.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/net.h>
#include <libchaos/str.h>

#include "control.h"
#include "servauth.h"
#include "cache.h"
#include "auth.h"
#include "dns.h"
#include "proxy.h"
#include "query.h"

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void query_dns_forward_done(struct dns_resolver *res)
{
  struct servauth_query *q;
  
  q = dns_get_userarg(res);
  
  q->remote_addr.s_addr = INADDR_ANY;
  
  dns_get_addr(res, AF_INET, &q->remote_addr);
  
  if(q->remote_addr.s_addr != INADDR_ANY)
  {
    /* reply address */
    control_send(&servauth_control, "dns forward %s %s", q->id, net_ntoa(q->remote_addr));
  }
  else
  {
    /* reply failure */
    control_send(&servauth_control, "dns forward %s", q->id);
  }
  
  query_free(q);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void query_dns_reverse_done(struct dns_resolver *res)
{
  struct servauth_query *q;
  
  q = dns_get_userarg(res);
  
  q->host = dns_dup_name(res);
  
  if(q->host != NULL)
  {
    /* Reply name */
    control_send(&servauth_control, "dns reverse %s %s", q->id, q->host);
  }
  else
  {
    /* Reply failure */
    control_send(&servauth_control, "dns reverse %s", q->id);
  }
  
  /* Add to cache */
  cache_dns_put(&servauth_dnscache, CACHE_DNS_REVERSE, 
                q->remote_addr, q->host, timer_systime);
  
  query_free(q);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void query_auth_done(struct auth_client *auth)
{
  struct servauth_query *q;
  
  q = auth_get_userarg(auth);
  
  if(auth->reply[0])
    q->ident = strdup(auth->reply);
  else
    q->ident = NULL;
  
  if(q->ident != NULL)
  {    
    /* reply username */
    control_send(&servauth_control, "auth %s %s", q->id, q->ident);
  }
  else
  {
    int status;
    
    /* reply failure */
    control_send(&servauth_control, "auth %s", q->id);
    
    status = (q->auth.status == AUTH_ST_TIMEOUT ?
              CACHE_AUTH_TIMEOUT : CACHE_AUTH_RESET);
    
    /* add to cache */
    cache_auth_put(&servauth_authcache, status, q->remote_addr, timer_systime);
  }

  query_free(q);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void query_proxy_done(struct proxy_check *proxy)
{
  struct servauth_query *q;
  
  q = proxy_get_userarg(proxy);
  
  /* reply  */
  control_send(&servauth_control, "proxy %s %s", q->id, proxy_replies[q->proxy.reply]);
  
  /* add to cache */
  cache_proxy_put(&servauth_proxycache, q->proxy.reply, q->remote_addr, q->remote_port, q->ptype, timer_systime);

  query_free(q);
}

/* -------------------------------------------------------------------------- *
 * find a free query structure                                                *
 * -------------------------------------------------------------------------- */
struct servauth_query *query_find(struct servauth_query *q, size_t queries)
{
  size_t i;

  for(i = 0; i < queries; i++)
    if(query_is_free(&q[i]))
      return &q[i];

  return NULL;
}

/* -------------------------------------------------------------------------- *
 * zero-initialise a query structure                                          *
 * -------------------------------------------------------------------------- */
void query_zero(struct servauth_query *q)
{
  memset(q, 0, sizeof(struct servauth_query));
}

/* -------------------------------------------------------------------------- *
 * set values necessary for an authentication query                           *
 * -------------------------------------------------------------------------- */
int query_set_auth(struct servauth_query *q, const char *id, 
                   const char *ip, const char *l, const char *r)
{
  /* convert address */
  if(net_aton(ip, &q->remote_addr) <= 0)
    return -1;

  q->id = strdup(id);

  /* set the ports */
  q->local_port = (uint16_t)strtol(l, NULL, 10);
  q->remote_port = (uint16_t)strtol(r, NULL, 10);

  return 0;
}

/* -------------------------------------------------------------------------- *
 * set values necessary for a proxy query                                     *
 * -------------------------------------------------------------------------- */
int query_set_proxy(struct servauth_query *q, const char *id, 
                    const char *r, const char *l, const char *t)
{
  char *ptr;
  char  addrbuf[32];
  
  q->id = strdup(id);

  /* parse remote address:port */
  strlcpy(addrbuf, r, sizeof(addrbuf));
  
  if((ptr = strchr(addrbuf, ':')))
    *ptr++ = '\0';
  
  if(net_aton(addrbuf, &q->remote_addr) <= 0)
    return -1;

  q->remote_port = strtoul(ptr, NULL, 10);
  
  /* parse local address:port */
  strlcpy(addrbuf, l, sizeof(addrbuf));
  
  if((ptr = strchr(addrbuf, ':')))
    *ptr++ = '\0';
  
  if(net_aton(addrbuf, &q->local_addr) <= 0)
    return -1;
  
  q->local_port = strtoul(ptr, NULL, 10);
    
  q->ptype = proxy_parse_type(t);
  
  if(q->ptype == -1)
    return -1;

  return 0;
}

/* -------------------------------------------------------------------------- *
 * set query host for dns forward lookups                                     *
 * -------------------------------------------------------------------------- */
int query_set_host(struct servauth_query *q, const char *id, const char *host)
{
  q->id = strdup(id);
  q->host = strdup(host);

  return 0;
}

/* -------------------------------------------------------------------------- *
 * set address for dns reverse lookups                                        *
 * -------------------------------------------------------------------------- */
int query_set_addr(struct servauth_query *q, const char *id, const char *addr)
{
  q->id = strdup(id);

  return net_aton(addr, &q->remote_addr) <= 0;
}

/* -------------------------------------------------------------------------- *
 * free a query structure                                                     *
 * -------------------------------------------------------------------------- */
void query_free(struct servauth_query *q)
{
  dns_clear(&q->res);
  auth_clear(&q->auth);
  proxy_clear(&q->proxy);

  free(q->id);
  free(q->ident);
  free(q->host);
  free(q->username);
  free(q->password);

  query_zero(q);
}

/* -------------------------------------------------------------------------- *
 * start a DNS or AUTH query                                                  *
 * -------------------------------------------------------------------------- */
int query_start(struct servauth_query *q, int type, uint64_t t)
{
  int ret = -1;

  if(q != NULL)
  {
    q->type = type;

    /* start reverse lookup */
    if(type & QUERY_DNS_REVERSE)
    {
      dns_zero(&q->res);
      dns_set_userarg(&q->res, q);
      dns_set_callback(&q->res, query_dns_reverse_done, DNS_TIMEOUT);
      
      ret = dns_ptr_lookup(&q->res, AF_INET, &q->remote_addr, t);
    }

    /* start forward lookup */
    if(type & QUERY_DNS_FORWARD)
    {
      dns_zero(&q->res);
      dns_set_userarg(&q->res, q);
      dns_set_callback(&q->res, query_dns_forward_done, DNS_TIMEOUT);
      
      ret = dns_name_lookup(&q->res, AF_INET, q->host, t);
    }

    /* start auth lookup */
    if(type & QUERY_AUTH)
    {
      auth_zero(&q->auth);
      auth_set_userarg(&q->auth, q);
      auth_set_callback(&q->auth, query_auth_done, AUTH_TIMEOUT);
      
      ret = auth_lookup(&q->auth, 
                        &q->remote_addr,
                         q->local_port, 
                         q->remote_port, t);
    }

    /* start proxy lookup */
    if(type & QUERY_PROXY)
    {
      proxy_zero(&q->proxy);
      proxy_set_userarg(&q->proxy, q);
      proxy_set_callback(&q->proxy, query_proxy_done, PROXY_TIMEOUT);

      ret = proxy_lookup(&q->proxy, &q->remote_addr, q->remote_port, &q->local_addr, q->local_port, q->ptype, t);
    }
  }

  /* return status */
  return ret;
}
