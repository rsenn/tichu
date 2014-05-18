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
 * $Id: httpc.h,v 1.5 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_HTTPC_H
#define LIB_HTTPC_H

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define HTTPC_LINELEN  1024
#define HTTPC_MAX_BUF  (256 * 1024)
#define HTTPC_TIMEOUT  30000LLU
#define HTTPC_INTERVAL 0LLU

#define HTTPC_TYPE_GET  0
#define HTTPC_TYPE_POST 1

#define HTTPC_IDLE       0
#define HTTPC_CONNECTING 1
#define HTTPC_SENT       2
#define HTTPC_HEADER     3
#define HTTPC_DATA       4
#define HTTPC_DONE       5

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct httpc;

typedef void (httpc_cb_t)(struct httpc *, void *, void *, void *, void *);

struct httpc_var {
  struct node node;
  uint32_t    hash;
  char        name[64];
  char        value[128];
};

struct httpc {
  struct node        node;
  uint32_t           id;
  uint32_t           refcount;
  int                fd;
  int                type;
  uint32_t           nhash;
  int                status;
  uint16_t           port;
  int                ssl;
  int                code;
  int                chunked;
  struct connect    *connect;
  httpc_cb_t        *callback;
  struct list        vars;
  struct list        header;
  char              *loc;
  char              *content;
  size_t             content_length;
  char              *data;
  size_t             data_length;
  size_t             chunk_length;
  void              *args[4];
  char               error[32];
  char               protocol[16];
  char               name[32];
  char               host[64];
  char               location[1024];
  char               url[2048];
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           httpc_log;
extern struct sheap  httpc_heap;
extern struct sheap  httpc_var_heap;
extern struct dheap  httpc_dheap;
extern struct list   httpc_list;
extern uint32_t      httpc_id;

/* -------------------------------------------------------------------------- *
 * Initialize httpc heap.                                                     *
 * -------------------------------------------------------------------------- */
extern void          httpc_init      (void);

/* -------------------------------------------------------------------------- *
 * Destroy httpc heap.                                                        *
 * -------------------------------------------------------------------------- */
extern void          httpc_shutdown  (void);

/* -------------------------------------------------------------------------- *
 * Add a httpc.                                                               *
 * -------------------------------------------------------------------------- */
extern struct httpc *httpc_add       (const char    *url);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           httpc_vconnect  (struct httpc  *hcptr,
                                      void          *cb, 
                                      va_list        args);
extern int           httpc_connect   (struct httpc  *hcptr, 
                                      void          *cb, 
                                      ...);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           httpc_url_parse (struct httpc  *hcptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern size_t        httpc_url_encode(char          *to,
                                      const char    *from,
                                      size_t         n, 
                                      int            loc);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           httpc_url_build (struct httpc  *hcptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           httpc_update    (struct httpc  *httpc);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          httpc_clear     (struct httpc  *httpc);

/* -------------------------------------------------------------------------- *
 * Remove a httpc.                                                            *
 * -------------------------------------------------------------------------- */
extern void          httpc_delete    (struct httpc  *httpc);

/* -------------------------------------------------------------------------- *
 * Send HTTP request                                                          *
 * -------------------------------------------------------------------------- */
extern void          httpc_send      (struct httpc  *hcptr);

/* -------------------------------------------------------------------------- *
 * Receive HTTP data                                                          *
 * -------------------------------------------------------------------------- */
extern void          httpc_recv      (int            fd,
                                      struct httpc  *hcptr);
    
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct httpc *httpc_find_name (const char    *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct httpc *httpc_find_id   (uint32_t       id);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          httpc_vset_args (struct httpc  *httpc, 
                                      va_list        args);
extern void          httpc_set_args  (struct httpc  *httpc, 
                                      ...);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          httpc_var_set   (struct httpc  *hcptr, const char *name,
                                      const char    *value, ...);

extern void          httpc_var_vset  (struct httpc  *hcptr, const char *name,
                                      const char    *value, va_list     args);


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          httpc_var_build (struct httpc  *hcptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          httpc_set_name  (struct httpc  *hcptr, 
                                      const char    *name);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct httpc *httpc_pop       (struct httpc  *httpc);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct httpc *httpc_push      (struct httpc **httpcptr);    

/* -------------------------------------------------------------------------- *
 * Dump httpcs.                                                               *
 * -------------------------------------------------------------------------- */
extern void          httpc_dump      (struct httpc  *hcptr);

#endif /* LIB_HTTPC_H */
