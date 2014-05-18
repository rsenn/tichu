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
 * $Id: net.h,v 1.16 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_NET_H
#define LIB_NET_H

#include <libchaos/defs.h>
#include <libchaos/io.h>

/*#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>*/

#define NET_SERVER 0
#define NET_CLIENT 1

struct listen;

/* protocol handler type */
typedef void (net_callback_t)(int, void *listenerorconnect, void *arg);

typedef struct protocol {
  struct node     node;
  uint32_t        id;
  int             refcount;
  uint32_t        hash;
  int             type;
  char            name[PROTOLEN + 1];
  net_callback_t *handler;
} protocol_t;

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           net_log;
extern struct sheap  net_heap;
extern struct timer *net_timer;
extern struct list   net_list;
extern uint32_t      net_id;

/* -------------------------------------------------------------------------- *
 * Convert a short from host to network byteorder                              *
 * -------------------------------------------------------------------------- */
static inline uint16_t net_htons(uint16_t n)
{
  union {
    uint8_t c[2];
    uint16_t i;
  } u;
  
  u.c[0] = (n >> 8) & 0xff;
  u.c[1] =  n       & 0xff;
  
  return u.i;
}

/* -------------------------------------------------------------------------- *
 * Convert a short from network to host byteorder                             *
 * -------------------------------------------------------------------------- */
static inline uint16_t net_ntohs(uint16_t n)
{
  union {
    uint16_t i;
    uint8_t c[2];
  } u;
  
  u.i = n;
  
  return ((uint16_t)(u.c[0] << 8)) | ((uint16_t)u.c[1]);
}

/* -------------------------------------------------------------------------- *
 * Convert a long from host to network byteorder                              *
 * -------------------------------------------------------------------------- */
static inline uint32_t net_htonl(uint32_t n)
{
  union {
    uint8_t c[4];
    uint32_t i;
  } u;
  
  u.c[0] = (n >> 24) & 0xff;
  u.c[1] = (n >> 16) & 0xff;
  u.c[2] = (n >>  8) & 0xff;
  u.c[3] =  n        & 0xff;
  
  return u.i;
}

/* -------------------------------------------------------------------------- *
 * Convert a long from network to host byteorder                              *
 * -------------------------------------------------------------------------- */
static inline uint32_t net_ntohl(uint32_t n)
{
  union {
    uint32_t i;
    uint8_t c[4];
  } u;
  
  u.i = n;
  
  return (u.c[0] << 24) |
         (u.c[1] << 16) |
         (u.c[2] <<  8) | 
          u.c[3];
}

/* -------------------------------------------------------------------------- *
 * Convert from network address to string (re-entrant). (AF_INET)             *
 * -------------------------------------------------------------------------- */
extern char       *net_ntoa_r   (struct in_addr  in, 
                                 char           *buf);

/* -------------------------------------------------------------------------- *
 * Convert from network address to string. (AF_INET)                          *
 * -------------------------------------------------------------------------- */
extern char       *net_ntoa     (struct in_addr  in);

/* -------------------------------------------------------------------------- *
 * Convert from string to network address. (AF_INET)                          *
 * -------------------------------------------------------------------------- */
extern int         net_aton     (const char     *cp, 
                                 struct in_addr *inp);

/* -------------------------------------------------------------------------- *
 * Convert from network address to string. (AF_INET and AF_INET6)             *
 * -------------------------------------------------------------------------- */
extern const char *inet_ntop    (int             af,
                                 const void     *cp,
                                 char           *buf, 
                                 size_t          len);

/* -------------------------------------------------------------------------- *
 * Convert from string to network address. (AF_INET and AF_INET6)             *
 * -------------------------------------------------------------------------- */
extern int         net_pton     (int             af,
                                 const char     *cp,
                                 void           *buf);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#ifdef __GNUC__

extern inline char *net_ntoa    (struct in_addr in)
{
  static char buf[16];
  return net_ntoa_r(in, buf);
}

#endif /* __GNUC__ */

/* -------------------------------------------------------------------------- *
 * Initialize protocol heap.                                                  *
 * -------------------------------------------------------------------------- */
extern void             net_init       (void);
  
/* -------------------------------------------------------------------------- *
 * Destroy protocol heap.                                                     *
 * -------------------------------------------------------------------------- */
extern void             net_shutdown   (void);

/* -------------------------------------------------------------------------- *
 * Find a protocol.                                                           *
 * -------------------------------------------------------------------------- */
extern struct protocol *net_find       (int             type,
                                        const char     *name);

extern struct protocol *net_find_id    (uint32_t        id);

/* -------------------------------------------------------------------------- *
 * Register a protocol.                                                       *
 * -------------------------------------------------------------------------- */
extern struct protocol *net_register   (int             type,
                                        const char     *name,
                                        void           *handler);

/* -------------------------------------------------------------------------- *
 * Unregister a protocol.                                                     *
 * -------------------------------------------------------------------------- */
extern void            *net_unregister (int               type,
                                        const char       *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct protocol *net_pop        (struct protocol  *pptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct protocol *net_push       (struct protocol **pptrptr);


/* -------------------------------------------------------------------------- *
 * Create a streaming socket with desired protocol family and type            *
 * (SOCK_DGRAM or SOCK_STREAM)                                                *
 * -------------------------------------------------------------------------- */
extern int              net_socket     (int             pf, 
                                        int             type);

/* -------------------------------------------------------------------------- *
 * Bind a socket to a specified address/port                                  *
 * -------------------------------------------------------------------------- */
extern int              net_bind       (int             fd,
                                        struct in_addr  addr,
                                        uint16_t        port);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int              net_vconnect   (int             fd,
                                        struct in_addr  addr,
                                        uint16_t        port,
                                        void           *cb_rd,
                                        void           *cb_wr,
                                        va_list         args);

extern int              net_connect    (int             fd,
                                        struct in_addr  addr,
                                        uint16_t        port,
                                        void           *cb_rd,
                                        void           *cb_wr,
                                        ...);

/* -------------------------------------------------------------------------- *
 * Listen for incoming connections and register read callback.                *
 * -------------------------------------------------------------------------- */
extern int              net_vlisten    (int             fd,
                                        int             backlog,
                                        void           *callback,
                                        va_list         args);

extern int              net_listen     (int             fd,
                                        int             backlog,
                                        void           *callback,
                                        ...);

/* -------------------------------------------------------------------------- *
 * Accept a pending connection.                                               *
 * -------------------------------------------------------------------------- */
extern int              net_accept     (int             fd);

/* -------------------------------------------------------------------------- *
 * Dump protocol stack.                                                       *
 * -------------------------------------------------------------------------- */
extern void             net_dump       (struct protocol  *nptr);

#endif /* LIB_NET_H */
