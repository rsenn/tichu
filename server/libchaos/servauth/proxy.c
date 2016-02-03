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
 * $Id: proxy.c,v 1.4 2004/12/31 03:39:14 smoli Exp $
 */

#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/timer.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/net.h>
#include <libchaos/str.h>

#include "proxy.h"

extern uint32_t servauth_log;

const char *proxy_types[] = {
  "http", "socks4", "socks5", "wingate", "cisco", NULL
};  

const char *proxy_replies[] = {
  "none", "timeout", "filtered", "closed", "denied", "n/a", "open", NULL
}; 

static int proxy_timeout(struct proxy_check *proxy)
{
  if(proxy->callback)
    proxy->callback(proxy);
  
  if(proxy->timer)
  {
    timer_remove(proxy->timer);
    proxy->timer = NULL;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static uint32_t proxy_seed = 0xdefaced;

#define ROR(v, n) ((v >> ((n) & 0x1f)) | (v << (32 - ((n) & 0x1f))))
#define ROL(v, n) ((v >> ((n) & 0x1f)) | (v << (32 - ((n) & 0x1f))))
static uint32_t proxy_random(void)
{
  int      it;
  int      i;
  uint32_t ns = timer_mtime;
  
  it = (ns & 0x1f) + 0x20;
  
  for(i = 0; i < it; i++)
  {
    ns = ROL(ns, proxy_seed);
    
    if(ns & 0x01)
      proxy_seed -= 0x87e9c96b;
    else
      proxy_seed += 0x9a72e90f;
    
    ns = ROL(ns, proxy_seed >> 21);
    
    if(proxy_seed & 0x04)
      ns ^= proxy_seed;
    else
      ns -= proxy_seed;
    
    proxy_seed = ROL(proxy_seed, ns >> 16);
    
    if(ns & 0x01)
      proxy_seed += ns;
    else
      proxy_seed ^= ns;
    
    ns = ROL(ns, proxy_seed >> 19);
    
    if(proxy_seed & 0x10)
      ns += proxy_seed;
    else
      proxy_seed -= ns;
  }

  return ns;
}
#undef ROR
#undef ROL

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static const char proxy_chars[] = "abcdefghijklmnopqrstuvwxyz";

static const char *proxy_randstr(void)
{
  static char str[10];
  uint32_t    i;
  
  for(i = 0; i < sizeof(str) - 1; i++)
    str[i] = proxy_chars[proxy_random() % sizeof(proxy_chars) - 1];
  
  str[i] = '\0';
  
  return str;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int proxy_send_irc(struct proxy_check *proxy)
{
  io_puts(proxy_fd(proxy), "USER proxy proxy proxy :proxy check");
  io_puts(proxy_fd(proxy), "NICK %s", proxy_randstr());
  io_puts(proxy_fd(proxy), "QUIT");
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int proxy_parse_type(const char *type)
{
  uint32_t i;
  
  for(i = 0; proxy_types[i]; i++)
  {
    if(!stricmp(proxy_types[i], type))
      return i;
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int proxy_parse_reply(const char *reply)
{
  uint32_t i;
  
  for(i = 0; proxy_replies[i]; i++)
  {
    if(!stricmp(proxy_replies[i], reply))
      return i;
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void proxy_event_rd(int fd, struct proxy_check *proxy)
{
  char  buf[512];
  char *ptr;
  int   n;
  
  switch(proxy->type)
  {
    /* check HTTP proxy reply */
    case PROXY_TP_HTTP:
    
      n = io_gets(proxy_fd(proxy), buf, 512);

      if(n > 0 && (ptr = strstr(buf, "HTTP/1")))
      {
        int num;
        
        ptr += 9;
        
        num = strtoul(ptr, &ptr, 10);
        
        if((num == 200 || num / 100 == 5) && num != 501 && strnicmp(&ptr[1], "ok", 2))
          proxy->reply = PROXY_RP_OPEN;
        else
          proxy->reply = PROXY_RP_DENIED;
      }
      else
      {
        proxy->reply = PROXY_RP_NA;
      }    
    
      break;
    
    /* check SOCKS4 proxy reply */
    case PROXY_TP_SOCKS4:
    
      n = io_read(proxy_fd(proxy), buf, 2);
    
      if(n == 2)
      {
        if((unsigned char)buf[1] >= 90)
          proxy->reply = PROXY_RP_OPEN;
        else
          proxy->reply = PROXY_RP_DENIED;
      }
      else
      {
        proxy->reply = PROXY_RP_NA;
      }
    
      break;
    
    /* check SOCKS5 proxy reply */
    case PROXY_TP_SOCKS5:
    
      n = io_read(proxy_fd(proxy), buf, 2);
    
      if(n == 2)
      {
        if((unsigned char)buf[1] == 0x00)
          proxy->reply = PROXY_RP_OPEN;
        else
          proxy->reply = PROXY_RP_DENIED;
      }
      else
      {
        proxy->reply = PROXY_RP_NA;
      }
    
      break;
    
    /* check open WINGATE/CISCO reply */
    case PROXY_TP_WINGATE:
    case PROXY_TP_CISCO:
        
      proxy->reply = PROXY_RP_NA;

      while(io_list[proxy_fd(proxy)].recvq.lines)
      {
        n = io_gets(proxy_fd(proxy), buf, 512);
      
        if(n <= 0)
          break;
    
        if(!strncmp(buf, "PING", 4))
        {
          proxy->reply = PROXY_RP_OPEN;
          break;
        }
      }
    
      if(!io_list[proxy_fd(proxy)].status.closed &&
         !io_list[proxy_fd(proxy)].status.err &&
         (proxy->reply != PROXY_RP_OPEN))
        return;
  
      break;
  }

  if(proxy->callback)
    proxy->callback(proxy);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void proxy_event_wr(int fd, struct proxy_check *proxy)
{
  char buf[10];
  
  io_unregister(fd, IO_CB_WRITE);
  
  switch(proxy->type)
  {
    /* send HTTP proxy request */
    case PROXY_TP_HTTP:
      io_puts(fd, "CONNECT %s:%u HTTP/1.0\r\n\r",
              net_ntoa(proxy->testaddr), (uint32_t)proxy->testport);
    
      io_queue_control(proxy_fd(proxy), ON, OFF, ON);
      break;
    
    /* send SOCKS4 proxy request */
    case PROXY_TP_SOCKS4:
      buf[0] = 0x04;
      buf[1] = 0x01;
      buf[2] = htons(proxy->testport) & 0xff;
      buf[3] = htons(proxy->testport) >> 8;
      buf[4] = (htonl(proxy->testaddr.s_addr) >> 24) & 0xff;
      buf[5] = (htonl(proxy->testaddr.s_addr) >> 16) & 0xff;
      buf[6] = (htonl(proxy->testaddr.s_addr) >> 8) & 0xff;
      buf[7] = htonl(proxy->testaddr.s_addr) & 0xff;
      buf[8] = 0;
    
      io_write(fd, buf, 9);
    
      break;
    
    /* send SOCKS5 proxy request */
    case PROXY_TP_SOCKS5:
      buf[0] = 0x05;
      buf[1] = 0x01;
      buf[2] = 0x00;
      buf[3] = 0x01;
      buf[4] = (htonl(proxy->testaddr.s_addr) >> 24) & 0xff;
      buf[5] = (htonl(proxy->testaddr.s_addr) >> 16) & 0xff;
      buf[6] = (htonl(proxy->testaddr.s_addr) >> 8) & 0xff;
      buf[7] = htonl(proxy->testaddr.s_addr) & 0xff;
      buf[8] = htons(proxy->testport) >> 8;
      buf[9] = htons(proxy->testport) & 0xff;
    
      io_write(fd, buf, 10);

      break;
    
    case PROXY_TP_WINGATE:
    
      io_puts(fd, "%s:%u\r", net_ntoa(proxy->testaddr), (uint32_t)proxy->testport);
      proxy_send_irc(proxy);
      io_queue_control(proxy_fd(proxy), ON, OFF, ON);
      break;
    
    case PROXY_TP_CISCO:
    
      io_puts(fd, "cisco\r");
      io_puts(fd, "telnet %s %u\r", net_ntoa(proxy->testaddr), (uint32_t)proxy->testport);
      proxy_send_irc(proxy);
      io_queue_control(proxy_fd(proxy), ON, OFF, ON);
      break;
  }
  
  io_register(fd, IO_CB_READ, proxy_event_rd, proxy);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void proxy_event_cn(int fd, struct proxy_check *proxy, uint32_t index)
{
  if(io_list[fd].status.closed || io_list[fd].status.err)
  {
    proxy->reply = PROXY_RP_CLOSED;
    
    /* Failed connecting, shut all sockets all return */
    io_shutup(proxy_fd(proxy));
    
    if(proxy->callback)
      proxy->callback(proxy);
  }
  else
  {
    proxy->reply = PROXY_RP_TIMEOUT;
    
    io_register(fd, IO_CB_WRITE, proxy_event_wr, proxy, index);
  }
}

/* -------------------------------------------------------------------------- *
 * zero proxy client struct                                                   *
 * -------------------------------------------------------------------------- */
void proxy_zero(struct proxy_check *proxy)
{
  memset(proxy, 0, sizeof(struct proxy_check));
}

/* -------------------------------------------------------------------------- *
 * close proxy client socket and zero                                          *
 * -------------------------------------------------------------------------- */
void proxy_clear(struct proxy_check *proxy)
{
  io_shutup(proxy_fd(proxy));

  timer_push(&proxy->timer);
  
  proxy_zero(proxy);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int proxy_connect(struct proxy_check *proxy)
{
  proxy->sock = net_socket(AF_INET, SOCK_STREAM) + 1;
  
  if(proxy->sock)
  {
    io_queue_control(proxy_fd(proxy), ON, OFF, OFF);
      
    log(servauth_log, L_status, "connecting to %s:%u", 
        net_ntoa(proxy->addr.sin_addr), (uint32_t)proxy->port);
      
    if(net_connect(proxy_fd(proxy), 
                   proxy->addr.sin_addr, proxy->port,
                   proxy_event_cn, proxy_event_cn, proxy))
      return -1;
    
    timer_push(&proxy->timer);
  
    proxy->deadline = timer_systime + proxy->timeout;
    
    proxy->timer = timer_start(proxy_timeout, PROXY_TIMEOUT, proxy);

    proxy->reply = PROXY_RP_FILTERED;
    
    return 0;
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int proxy_lookup(struct proxy_check *proxy, struct in_addr *addr,
                 uint16_t port, struct in_addr *testaddr, uint16_t testport, 
                 int type, uint64_t t)
{
  proxy->addr.sin_family = AF_INET;
  proxy->addr.sin_addr = *addr;
  proxy->addr.sin_port = htons(port);
  proxy->port = port;
  
  proxy->testaddr = *testaddr;
  proxy->testport = testport;
  proxy->type = type;
  
  return proxy_connect(proxy);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void proxy_set_userarg(struct proxy_check *proxy, void *arg)
{
  proxy->userarg = arg;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void *proxy_get_userarg(struct proxy_check *proxy)
{
  return proxy->userarg;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void proxy_set_callback(struct proxy_check *proxy, proxy_callback_t *cb, 
                       uint64_t timeout)
{
  proxy->callback = cb;
  proxy->timeout = timeout;
}
