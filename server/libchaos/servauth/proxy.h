/* chaosircd - pi-networks irc server
 *              
 * Copyright (C) 2003  Roman Senn <smoli@paranoya.ch>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 * 
 * $Id: proxy.h,v 1.2 2004/01/23 07:46:28 smoli Exp $
 */

#ifndef SERVAUTH_PROXY_H
#define SERVAUTH_PROXY_H

#define PROXY_TIMEOUT   10000 /* msecs timeout */

#define PROXY_ST_IDLE       0
#define PROXY_ST_CONNECTING 1
#define PROXY_ST_SENT       2
#define PROXY_ST_DONE       3
#define PROXY_ST_ERROR     -1
#define PROXY_ST_TIMEOUT   -2

#define PROXY_TP_HTTP    0
#define PROXY_TP_SOCKS4  1
#define PROXY_TP_SOCKS5  2
#define PROXY_TP_WINGATE 3
#define PROXY_TP_CISCO   4

#define PROXY_RP_NONE     0
#define PROXY_RP_TIMEOUT  1
#define PROXY_RP_FILTERED 2
#define PROXY_RP_CLOSED   3
#define PROXY_RP_DENIED   4
#define PROXY_RP_NA       5
#define PROXY_RP_OPEN     6


struct proxy_check;

typedef void (proxy_callback_t)(struct proxy_check *);

struct proxy_check {
  uint64_t           deadline;
  int                status;
  int                sock;
  struct sockaddr_in addr;
  uint32_t           pos;
  uint16_t           port;
  uint64_t           timeout;
  struct in_addr     testaddr;
  uint16_t           testport;
  void              *userarg;
  struct timer      *timer;
  proxy_callback_t  *callback;
  int                type;
  int                reply;
};

#define proxy_fd(proxy)       ((proxy)->sock - 1)
#define proxy_hfd(proxy, hfd) { \
  if(proxy_fd(proxy) > (hfd)) \
    (hfd) = proxy_fd(proxy); \
}

#define proxy_is_idle(proxy) (!((struct proxy_check *)(proxy))->sock)
#define proxy_is_busy(proxy)  (((struct proxy_check *)(proxy))->sock)

extern const char *proxy_types[];
extern const char *proxy_replies[];

extern int proxy_parse_type(const char *type);
extern int proxy_parse_reply(const char *reply);
 
extern void  proxy_zero         (struct proxy_check *proxy);
extern void  proxy_clear        (struct proxy_check *proxy);
extern int   proxy_connect      (struct proxy_check *proxy);
extern int   proxy_lookup       (struct proxy_check *proxy, 
                                 struct in_addr     *addr,
                                 uint16_t            port,
                                 struct in_addr     *testaddr,
                                 uint16_t            testport,
                                 int                 type,
                                 uint64_t            t);
extern void  proxy_set_userarg  (struct proxy_check *proxy, 
                                 void               *arg);
extern void *proxy_get_userarg  (struct proxy_check *proxy);
extern void  proxy_set_callback (struct proxy_check *proxy, 
                                 proxy_callback_t   *cb,
                                 uint64_t            timeout);
#endif /* SERVAUTH_PROXY_H */
