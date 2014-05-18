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
 * $Id: query.h,v 1.3 2004/01/23 07:46:28 smoli Exp $
 */

#ifndef SERVAUTH_QUERY_H
#define SERVAUTH_QUERY_H

#define QUERY_DNS_FORWARD 1
#define QUERY_DNS_REVERSE 2
#define QUERY_AUTH        4
#define QUERY_PROXY       8

struct servauth_query {
  int                  done;
  int                  type;
  char                *id;
  char                *host;
  char                *ident;
  struct in_addr       remote_addr;
  struct in_addr       local_addr;
  uint16_t             remote_port;
  uint16_t             local_port;
  uint16_t             port;
  char                *username;
  char                *password;
  int                  ptype;
  struct dns_resolver  res;
  struct auth_client   auth;
  struct proxy_check   proxy;
};

#define query_is_free(x) ((x)->id == NULL)
#define query_is_busy(x) ((x)->id != NULL)

extern struct servauth_query *query_find        (struct servauth_query *q, 
                                                 size_t                 queries);

extern void                   query_zero        (struct servauth_query *q);

extern void                   query_free        (struct servauth_query *q);

extern int                    query_set_auth    (struct servauth_query *q,
                                                 const char            *id, 
                                                 const char            *ip,
                                                 const char            *l, 
                                                 const char            *r);

extern int                    query_set_proxy   (struct servauth_query *q,
                                                 const char            *id, 
                                                 const char            *ip,
                                                 const char            *t,
                                                 const char            *p);

extern int                    query_set_host    (struct servauth_query *q,
                                                 const char            *id, 
                                                 const char            *host);

extern int                    query_set_addr    (struct servauth_query *q,
                                                 const char            *id, 
                                                 const char            *addr);

extern int                    query_start       (struct servauth_query *q,
                                                 int                    type,
                                                 uint64_t               t);

extern int                    query_pre_select  (struct servauth_query *q,
                                                 int                   *hsck, 
                                                 fd_set                *rfds,
                                                 fd_set                *wfds,
                                                 uint64_t              *timeout,
                                                 uint64_t               t);

extern int                    query_post_select (struct servauth_query *q,
                                                 const fd_set          *rfds,
                                                 const fd_set          *wfds,
                                                 uint64_t               t);
#endif /* SERVAUTH_QUERY_H */
