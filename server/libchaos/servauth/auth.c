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
 * $Id: auth.c,v 1.20 2004/12/31 03:39:14 smoli Exp $
 */

#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/timer.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/net.h>
#include <libchaos/str.h>

#include "auth.h"


static int auth_timeout(struct auth_client *auth)
{
  auth->recvbuf[0] = '\0';
  
  io_shutup(auth_fd(auth));
  
  if(auth->callback)
    auth->callback(auth);
  
  if(auth->timer)
  {
    timer_remove(auth->timer);
    auth->timer = NULL;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * builds and sends auth request to socket                                    *
 * -------------------------------------------------------------------------- */
static int auth_send(struct auth_client *auth)
{
  int ret;

  /* <local port> , <remote port> */
  ret = io_puts(auth_fd(auth), "%u , %u\r",
                auth->local_port, auth->remote_port);

  /* everything was sent, update client state */
  if(ret > 0)
  {
    auth->status = AUTH_ST_SENT;
    return 0;
  }

  return -1;
}

/* -------------------------------------------------------------------------- *
 * parse auth reply                                                           *
 * -------------------------------------------------------------------------- */
static int auth_parse(struct auth_client *auth)
{
  char *p;
  uint32_t i = 0;

  /* we're only interested in the last field */
  p = strrchr(auth->recvbuf, ':');

  if(*++p != ' ')
  {
    /* get not more than 31 valid chars */
    for(i = 0; *p && i < 31; i++)
    {
      if(!auth_is_ident_char(*p))
        break;
      
      auth->reply[i] = *p++;
    }
  }

  /* null terminate reply */
  auth->reply[i] = '\0';

  if(auth->callback)
    auth->callback(auth);

  return 1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void auth_event_rd(int fd, void *ptr)
{
  struct auth_client *auth = ptr;
  
  auth->recvbuf[0] = '\0';
  
  if(io_gets(fd, auth->recvbuf, sizeof(auth->recvbuf)) > 0)
  {
    auth_parse(ptr);
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void auth_event_cn(int fd, void *ptr)
{
  struct auth_client *auth = ptr;
  int64_t timeout;
  
  if(io_list[fd].status.closed || io_list[fd].status.err)
  {
    if(auth->callback)
      auth->callback(auth);
  }
  else
  {
    timeout = timer_systime - auth->deadline;
  
    if(timeout < 0LL)
      timeout = 1LL;
    
    io_register(fd, IO_CB_READ, auth_event_rd, ptr);
  }
}

/* -------------------------------------------------------------------------- *
 * zero authentication client struct                                          *
 * -------------------------------------------------------------------------- */
void auth_zero(struct auth_client *auth)
{
  memset(auth, 0, sizeof(struct auth_client));
}

/* -------------------------------------------------------------------------- *
 * close auth client socket and zero                                          *
 * -------------------------------------------------------------------------- */
void auth_clear(struct auth_client *auth)
{
  if(auth->sock)
    io_shutup(auth_fd(auth));

  timer_push(&auth->timer);
  
  auth_zero(auth);
}

/* -------------------------------------------------------------------------- *
 * start authentication client                                                *
 *                                                                            *
 * addr        - address to connect to (port is always 113)                   *
 * local_port  - port on this server                                          *
 * remote_port - port on the client                                           *
 * -------------------------------------------------------------------------- */
int auth_lookup(struct auth_client *auth, struct in_addr *addr,
                uint16_t local_port, uint16_t remote_port, uint64_t t)
{
  auth->sock = net_socket(AF_INET, SOCK_STREAM) + 1;
  
  if(auth->sock)
  {
    auth->local_port = local_port;
    auth->remote_port = remote_port;
    auth->deadline = timer_systime + t;
    
    io_queue_control(auth->sock - 1, ON, ON, OFF);
    
    auth_send(auth);
    
    if(net_connect(auth->sock - 1, *addr, 113, auth_event_cn, auth_event_cn, auth))
      return -1;
    
    auth->timer = timer_start(auth_timeout, AUTH_TIMEOUT, auth);
    
    return 0;
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void auth_set_userarg(struct auth_client *auth, void *arg)
{
  auth->userarg = arg;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void *auth_get_userarg(struct auth_client *auth)
{
  return auth->userarg;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void auth_set_callback(struct auth_client *auth, auth_callback_t *cb, 
                       uint64_t timeout)
{
  auth->callback = cb;
  auth->timeout = timeout;
}
