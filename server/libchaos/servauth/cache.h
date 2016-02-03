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
 * $Id: cache.h,v 1.4 2004/01/23 07:46:27 smoli Exp $
 */

#ifndef SERVAUTH_CACHE_H
#define SERVAUTH_CACHE_H

#define CACHE_AUTH_NONE    0
#define CACHE_AUTH_RESET   1
#define CACHE_AUTH_TIMEOUT 2

#define CACHE_DNS_NONE    0
#define CACHE_DNS_FORWARD 1
#define CACHE_DNS_REVERSE 2

#define CACHE_PROXY_NONE     0
#define CACHE_PROXY_TIMEOUT  1
#define CACHE_PROXY_FILTERED 2
#define CACHE_PROXY_CLOSED   3
#define CACHE_PROXY_DENIED   4
#define CACHE_PROXY_NA       5
#define CACHE_PROXY_OPEN     6

/* -------------------------------------------------------------------------- *
 * cache entry for auth replies/timeouts.                                     *
 * -------------------------------------------------------------------------- */
struct cache_entry_auth {
  int            status;
  struct in_addr addr;
  uint64_t       created;
};

/* -------------------------------------------------------------------------- *
 * cache entry for dns replies/timeouts.                                      *
 * -------------------------------------------------------------------------- */
struct cache_entry_dns {
  int            status;
  struct in_addr addr;
  uint64_t       created;
  char           name[HOSTLEN];
};

/* -------------------------------------------------------------------------- *
 * cache entry for proxy replies.                                             *
 * -------------------------------------------------------------------------- */
struct cache_entry_proxy {
  int            status;
  struct in_addr addr;
  uint64_t       created;
  uint16_t       port;
  int            type;
};

/* -------------------------------------------------------------------------- *
 * cache struct for auth replies/timeouts.                                    *
 * -------------------------------------------------------------------------- */
struct cache_auth {
  struct cache_entry_auth *entries;
  uint32_t                 size;
  uint32_t                 free;
};

/* -------------------------------------------------------------------------- *
 * cache struct for dns replies/timeouts.                                     *
 * -------------------------------------------------------------------------- */
struct cache_dns {
  struct cache_entry_dns *entries;
  uint32_t                size;
  uint32_t                free;
};

/* -------------------------------------------------------------------------- *
 * cache struct for proxy shit                                                *
 * -------------------------------------------------------------------------- */
struct cache_proxy {
  struct cache_entry_proxy *entries;
  uint32_t                 size;
  uint32_t                 free;
};

/* -------------------------------------------------------------------------- *
 * generate new cache for timed out and failed auth requests                  *
 * -------------------------------------------------------------------------- */
extern int            cache_auth_new         (struct cache_auth *cache,
                                              uint32_t           size);

/* -------------------------------------------------------------------------- *
 * put a failed auth request into cache.                                      *
 * -------------------------------------------------------------------------- */
extern void           cache_auth_put         (struct cache_auth *cache,
                                              int                status,
                                              struct in_addr     addr,
                                              uint64_t           t);

/* -------------------------------------------------------------------------- *
 * pick a auth query from cache.                                              *
 * -------------------------------------------------------------------------- */
extern int            cache_auth_pick        (struct cache_auth *cache, 
                                              struct in_addr     addr,
                                              uint64_t           t);

/* -------------------------------------------------------------------------- *
 * generate new cache for dns requests                                        *
 * -------------------------------------------------------------------------- */
extern int            cache_dns_new          (struct cache_dns  *cache, 
                                              uint32_t           size);

/* -------------------------------------------------------------------------- *
 * put a dns request into cache.                                              *
 * -------------------------------------------------------------------------- */
extern void           cache_dns_put          (struct cache_dns  *cache,
                                              int                status, 
                                              struct in_addr     addr,
                                              const char        *name, 
                                              uint64_t           t);

/* -------------------------------------------------------------------------- *
 * pick a reverse dns query from cache.                                       *
 * -------------------------------------------------------------------------- */
extern int            cache_dns_pick_reverse (struct cache_dns  *cache, 
                                              struct in_addr     addr,
                                              const char       **namep, 
                                              uint64_t           t);

/* -------------------------------------------------------------------------- *
 * pick a reverse dns query from cache.                                       *
 * -------------------------------------------------------------------------- */
extern struct in_addr cache_dns_pick_forward (struct cache_dns  *cache,
                                              const char        *name, 
                                              uint64_t           t);
/* -------------------------------------------------------------------------- *
 * generate new cache for proxy checks                                        *
 * -------------------------------------------------------------------------- */
extern int            cache_proxy_new        (struct cache_proxy *cache,
                                              uint32_t            size);

/* -------------------------------------------------------------------------- *
 * put a proxy request into cache.                                            *
 * -------------------------------------------------------------------------- */
extern void           cache_proxy_put        (struct cache_proxy *cache,
                                              int                 status,
                                              struct in_addr      addr,
                                              uint16_t            port,
                                              int                 type,
                                              uint64_t            t);

/* -------------------------------------------------------------------------- *
 * pick a proxy query from cache.                                             *
 * -------------------------------------------------------------------------- */
extern int            cache_proxy_pick       (struct cache_proxy *cache, 
                                              struct in_addr      addr,
                                              uint16_t            port,
                                              int                 type,
                                              uint64_t            t);

#endif /* SERVAUTH_CACHE_H */
