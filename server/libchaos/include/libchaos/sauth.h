/* chaosircd - pi-networks irc server
 *              
 * Copyright (C) 2003-2005  Roman Senn <smoli@paranoya.ch>
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
 * $Id: sauth.h,v 1.13 2005/04/15 06:16:29 smoli Exp $
 */

#ifndef LIB_SAUTH_H
#define LIB_SAUTH_H

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>
#include <libchaos/timer.h>
#include <libchaos/net.h>

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */

#define SAUTH_DONE       0x00
#define SAUTH_ERROR      0x01
#define SAUTH_TIMEDOUT   0x02

#define SAUTH_TYPE_DNSF   0x00
#define SAUTH_TYPE_DNSR   0x01
#define SAUTH_TYPE_AUTH   0x02
#define SAUTH_TYPE_PROXY  0x03

#define SAUTH_TIMEOUT    20000
#define SAUTH_RELAUNCH   15000        /* 15 secs relaunch interval */

#define SAUTH_PROXY_TIMEOUT  1
#define SAUTH_PROXY_FILTERED 2
#define SAUTH_PROXY_CLOSED   3
#define SAUTH_PROXY_DENIED   4
#define SAUTH_PROXY_NA       5
#define SAUTH_PROXY_OPEN     6

#define PROXY_HTTP    0
#define PROXY_SOCKS4  1
#define PROXY_SOCKS5  2
#define PROXY_WINGATE 3
#define PROXY_CISCO   4

/* -------------------------------------------------------------------------- *
 * Types                                                                      *
 * -------------------------------------------------------------------------- */

struct sauth;
typedef void (sauth_callback_t)(struct sauth *, void *, void *, void *, void *);

/* -------------------------------------------------------------------------- *
 * Sauth block structure.                                                     *
 * -------------------------------------------------------------------------- */
typedef struct sauth {
  struct node       node;        /* linking node for sauth block list */  
  uint32_t          id;
  uint32_t          refcount;
  
  /* externally initialised */
  int               type;
  int               status;
  char              host[HOSTLEN + 1];
  char              ident[USERLEN + 1];
  struct in_addr    addr;
  struct in_addr    connect;
  uint16_t          remote;
  uint16_t          local;
  struct timer     *timer;
  void             *args[4];
  int               reply;
  int               ptype;
  sauth_callback_t *callback;
} sauth_t;

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct sheap  sauth_heap;
extern struct list   sauth_list;
extern uint32_t      sauth_serial;
extern struct timer *sauth_timer;
extern struct timer *sauth_rtimer;
extern struct child *sauth_child;
extern int           sauth_log;
extern int           sauth_fds[2];
extern char          sauth_readbuf[BUFSIZE];
extern const char   *sauth_types[];
extern const char   *sauth_replies[];
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          sauth_init         (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          sauth_shutdown     (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          sauth_collect      (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct sauth *sauth_dns_forward  (const char    *address,
                                         void          *callback, 
                                         ...);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct sauth *sauth_dns_reverse  (struct in_addr address,
                                         void          *callback, 
                                         ...);  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           sauth_launch       (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct sauth *sauth_auth         (struct in_addr address,  
                                         uint16_t       local,
                                         uint16_t       remote,
                                         void          *callback,
                                         ...);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct sauth *sauth_proxy        (int            type,
                                         struct in_addr remote_addr, 
                                         uint16_t       remote_port,
                                         struct in_addr local_addr,  
                                         uint16_t       local_port,
                                         void          *callback, 
                                         ...);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           sauth_proxy_reply  (const char    *reply);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           sauth_proxy_type   (const char    *type);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          sauth_delete       (struct sauth  *sauth);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct sauth *sauth_find         (uint32_t       id);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct sauth *sauth_pop          (struct sauth  *sauth);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct sauth *sauth_push         (struct sauth **sauth);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          sauth_cancel       (struct sauth  *sauth);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          sauth_vset_args    (struct sauth  *sauth,
                                         va_list        args);

extern void          sauth_set_args     (sauth_t    *sauth, 
                                         ...);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          sauth_dump         (struct sauth  *saptr);
  
#endif /* LIB_SAUTH_H */
