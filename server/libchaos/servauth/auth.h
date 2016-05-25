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
 * $Id: auth.h,v 1.3 2004/01/23 07:46:27 smoli Exp $
 */

#ifndef SERVAUTH_AUTH_H
#define SERVAUTH_AUTH_H

#define AUTH_TIMEOUT   10000 /* msecs timeout */

#define AUTH_ST_IDLE       0
#define AUTH_ST_CONNECTING 1
#define AUTH_ST_SENT       2
#define AUTH_ST_DONE       3
#define AUTH_ST_ERROR     -1
#define AUTH_ST_TIMEOUT   -2

struct auth_client;

typedef void (auth_callback_t)(struct auth_client *);

struct auth_client {
  uint64_t           deadline;
  int                sock;
  struct sockaddr_in addr;
  int                status;
  char               reply[32];
  char               recvbuf[512];
  uint32_t           pos;
  uint16_t           local_port;
  uint16_t           remote_port;
  uint64_t           timeout;
  void              *userarg;
  struct timer      *timer;
  auth_callback_t   *callback;
};

#define auth_fd(auth)       ((auth)->sock - 1)
#define auth_hfd(auth, hfd) { \
  if(auth_fd(auth) > (hfd)) \
    (hfd) = auth_fd(auth); \
}

#define auth_is_idle(auth) (!((struct auth_client *)(auth))->sock)
#define auth_is_busy(auth)  (((struct auth_client *)(auth))->sock)

#define auth_is_ident_char(c) (((c) >= 'A' && (c) <= 'Z') || \
                               ((c) >= 'a' && (c) <= 'z') || \
                               ((c) >= '0' && (c) <= '9') || \
                               (c) == '-' || (c) == '_' || \
                               (c) == '.')

extern void  auth_zero         (struct auth_client *auth);
extern void  auth_clear        (struct auth_client *auth);
extern int   auth_lookup       (struct auth_client *auth, 
                                struct in_addr     *addr,
                                uint16_t            local_port,
                                uint16_t            remote_port, 
                                uint64_t            t);
extern void  auth_set_userarg  (struct auth_client *auth, 
                                void               *arg);
extern void *auth_get_userarg  (struct auth_client *auth);
extern void  auth_set_callback (struct auth_client *auth, 
                                auth_callback_t    *cb,
                                uint64_t            timeout);
      
#endif /* SERVAUTH_AUTH_H */
