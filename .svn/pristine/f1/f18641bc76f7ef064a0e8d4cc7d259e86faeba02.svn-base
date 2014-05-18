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
 * $Id: ssl.h,v 1.15 2005/01/17 19:09:50 smoli Exp $   
 */

#ifndef LIB_SSL_H
#define LIB_SSL_H

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
#include <openssl/ssl.h>
#include <openssl/rand.h>

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
#include <libchaos/log.h>
#include <libchaos/dlink.h>

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
#define SSL_CONTEXT_SERVER 0
#define SSL_CONTEXT_CLIENT 1

#define SSL_RANDOM_SIZE 2048
#define SSL_RANDOM_FILE "/dev/urandom"

#define SSL_READ    1
#define SSL_WRITE   2
#define SSL_ACCEPT  3
#define SSL_CONNECT 4

#define SSL_WRITE_WANTS_READ    1
#define SSL_READ_WANTS_WRITE    2
#define SSL_ACCEPT_WANTS_READ   3
#define SSL_ACCEPT_WANTS_WRITE  4
#define SSL_CONNECT_WANTS_READ  5
#define SSL_CONNECT_WANTS_WRITE 6

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
struct io;

struct ssl_context {
  struct node node;
  uint32_t    id;
  uint32_t    refcount;
  SSL_CTX    *ctxt;
  SSL_METHOD *meth;
  uint32_t    hash;
  int         context;
  char        name[64];
  char        cert[PATH_MAX];
  char        key[PATH_MAX];
  char        ciphers[64];
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int                 ssl_log;
extern struct sheap        ssl_heap;
extern struct list         ssl_list;
extern uint32_t            ssl_id;

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
extern void                ssl_init       (void);

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
extern void                ssl_shutdown   (void);

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
extern void                ssl_seed       (void);

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
extern void                ssl_default    (struct ssl_context  *scptr);

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
extern struct ssl_context *ssl_add        (const char          *name, 
                                           int                  context,
                                           const char          *cert, 
                                           const char          *key,  
                                           const char          *ciphers);  

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
extern int                 ssl_update     (struct ssl_context  *scptr,
                                           const char          *name, 
                                           int                  context,
                                           const char          *cert, 
                                           const char          *key,  
                                           const char          *ciphers);  

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
extern struct ssl_context *ssl_find_name  (const char          *name);

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
extern struct ssl_context *ssl_find_id    (uint32_t             id);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int                 ssl_new        (int                  fd, 
                                           struct ssl_context  *scptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int                 ssl_accept     (int                  fd);    

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int                 ssl_connect    (int                  fd);    

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int                 ssl_read       (int                  fd,
                                           void                *buf,
                                           size_t               n);  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int                 ssl_write      (int                  fd,
                                           const void          *buf,
                                           size_t               n);  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern const char         *ssl_strerror   (int                  err);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void                ssl_close      (int                  fd);  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int                 ssl_handshake  (int                  fd,
                                           struct io           *iofd);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void                ssl_cipher     (int                  fd,
                                           char                *ciphbuf, 
                                           size_t               n);
  
/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
extern struct ssl_context *ssl_pop        (struct ssl_context  *scptr);

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
extern struct ssl_context *ssl_push       (struct ssl_context **scptr);

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
extern void                ssl_delete     (struct ssl_context  *scptr);

/* -------------------------------------------------------------------------- * 
 * -------------------------------------------------------------------------- */
extern void                ssl_dump       (struct ssl_context  *scptr);

#endif /* LIB_SSL_H */
