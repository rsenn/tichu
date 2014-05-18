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
 * $Id: listen.h,v 1.19 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_LISTEN_H
#define LIB_LISTEN_H

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/mem.h>
#include <libchaos/net.h>
#include <libchaos/dlink.h>

#ifdef HAVE_SSL
#include <libchaos/ssl.h>
#endif /* HAVE_SSL */

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */

#define LISTEN_IDLE        0x00          /* listen block just sitting there */
#define LISTEN_LISTENING   0x01          /* listening for incoming connects */
#define LISTEN_ERROR       0x02          /* error listening */
#define LISTEN_CONNECTION  0x03          /* incoming connection */

/* -------------------------------------------------------------------------- *
 * Listen block structure.                                                    *
 * -------------------------------------------------------------------------- */
#ifdef HAVE_SOCKET_FILTER
struct filter;
#endif /* HAVE_SOCKET_FILTER */  

struct listen {
  struct node         node;                 /* linking node for listen_list */
  uint32_t            id;
  uint32_t            refcount;             /* times this block is referenced */
  uint32_t            lhash;
  uint32_t            nhash;
  
  /* externally initialised */
  uint16_t            port;
  uint16_t            backlog;              /* backlog buffer size */
  int                 ssl;                  /* SSL encryption flag */
  struct protocol    *proto;                /* protocol handler */
  void               *args;                 /* user-defineable arguments */
  char                name[HOSTLEN + 1];    /* user-definable name */
  char                address[HOSTLEN + 1]; /* hostname to listen on */
  char                context[64];

  /* internally initialised */
  int                 fd;
  struct in_addr      addr_local;
  struct in_addr      addr_remote;
  uint16_t            port_local;
  uint16_t            port_remote;
  uint32_t            status;
#ifdef HAVE_SOCKET_FILTER
  struct filter      *filter;
#endif /* HAVE_SOCKET_FILTER */  
#ifdef HAVE_SSL
  struct ssl_context *ctxt;
#endif /* HAVE_SSL */
};

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
extern int           listen_log;
extern struct sheap  listen_heap;       /* heap containing listen blocks */
extern struct list   listen_list;       /* list linking listen blocks */
extern uint32_t      listen_id;
extern int           listen_dirty;

/* -------------------------------------------------------------------------- *
 * Initialize listener heap and add garbage collect timer.                    *
 * -------------------------------------------------------------------------- */
extern void        listen_init         (void);

/* -------------------------------------------------------------------------- *
 * Destroy listener heap and cancel timer.                                    *
 * -------------------------------------------------------------------------- */
extern void        listen_shutdown     (void);

/* -------------------------------------------------------------------------- *
 * Garbage collect                                                            *
 * -------------------------------------------------------------------------- */
extern int         listen_collect      (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void        listen_default      (struct listen  *liptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct listen *listen_add       (const char     *address,
                                        uint16_t        port,
                                        int             backlog,
                                        int             ssl,
                                        const char     *context,
                                        const char     *protocol);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int            listen_update    (struct listen  *liptr,
                                        int             backlog,
                                        int             ssl,
                                        const char     *context,
                                        const char     *protocol);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           listen_delete    (struct listen  *liptr);

/* -------------------------------------------------------------------------- *
 * Loose all references                                                       *
 * -------------------------------------------------------------------------- */
extern void           listen_release   (struct listen  *liptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct listen *listen_pop       (struct listen  *liptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct listen *listen_push      (struct listen **liptrptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct listen *listen_find      (const char     *address,
                                        uint16_t        port);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           listen_set_args  (struct listen  *liptr, 
                                        const void     *argbuf,
                                        size_t          n);
 
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           listen_get_args  (struct listen  *liptr, 
                                        void           *argbuf,
                                        size_t          n);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           listen_set_name  (struct listen  *liptr,
                                        const char     *name);
 
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern const char    *listen_get_name  (struct listen  *liptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct listen *listen_find_name (const char     *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct listen *listen_find_id   (uint32_t        id);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#ifdef HAVE_SOCKET_FILTER
extern int            listen_attach_filter (struct listen *lptr, 
                                            struct filter *fptr);  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int            listen_detach_filter (struct listen *lptr);
#endif /* HAVE_SOCKET_FILTER */  

/* -------------------------------------------------------------------------- *
 * Dump listeners and listen heap.                                            *
 * -------------------------------------------------------------------------- */
extern void           listen_dump      (struct listen  *lptr);
  
#endif /* LIB_LISTEN_H */
