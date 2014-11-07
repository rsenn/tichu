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
 * $Id: sauth.c,v 1.41 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/child.h>
#include <libchaos/timer.h>
#include <libchaos/sauth.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/net.h>
#include <libchaos/str.h>
#include <libchaos/io.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct sheap      sauth_heap;
struct list       sauth_list;
uint32_t          sauth_serial;
struct timer     *sauth_timer;
struct timer     *sauth_rtimer;
struct child     *sauth_child;
int               sauth_log;
int               sauth_fds[2];
char              sauth_readbuf[BUFSIZE];
const char       *sauth_types[] = {
  "http",
  "socks4",
  "socks5",
  "wingate", 
  "cisco",
  NULL
};

const char       *sauth_replies[] = {
  "none",
  "timeout",
  "filtered",
  "closed",
  "denied",
  "n/a", 
  "open", 
  NULL
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static int  sauth_recover(void);
int  sauth_launch(void);
static void sauth_callback(struct sauth *sauth, int type);
static void sauth_read(int fd, void *arg);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static int sauth_timeout(struct sauth *sauth)
{  
  sauth_callback(sauth, SAUTH_TIMEOUT);

  timer_cancel(&sauth->timer);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int sauth_proxy_reply(const char *reply)
{
  uint32_t i;

  for(i = 0; sauth_replies[i]; i++)
  {
    if(!stricmp(sauth_replies[i], reply))
      return i;
  }

  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int sauth_proxy_type(const char *type)
{
  uint32_t i;

  for(i = 0; sauth_types[i]; i++)
  {
    if(!stricmp(sauth_types[i], type))
      return i;
  }

  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void sauth_child_cb(struct child *child)
{
  log(sauth_log, L_status, "sauth child callback %u", sauth_child->status);
  
  switch(sauth_child->status)
  {
    case CHILD_IDLE:
    {
      sauth_launch();
      break;
    }
    case CHILD_DEAD:
    {
      sauth_recover();
      break;
    }
    case CHILD_RUNNING:
    {    
      break;
    }  
  } 
} 

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void sauth_callback(struct sauth *sauth, int type)
{
  const char *what;
  
  sauth->status = type;
  
  if(sauth->status == SAUTH_TIMEDOUT)
    what = "timed out";
  else if(sauth->status == SAUTH_DONE)
    what = "done";
  else
    what = "failed";
  
  timer_cancel(&sauth->timer);

  switch(sauth->type)
  {
    case SAUTH_TYPE_DNSF:
    {
      log(sauth_log, L_verbose, "DNS query (#%u) %s. (%s -> %s)",
          sauth->id, what, sauth->host, 
          (sauth->addr.s_addr == INADDR_ANY ? 
           "NXDOMAIN" : net_ntoa(sauth->addr)));
          
      break;
    }
    case SAUTH_TYPE_DNSR:
    {
      log(sauth_log, L_verbose, "DNS query (#%u) %s. (%s -> %s)",
          sauth->id, what, net_ntoa(sauth->addr),
          (sauth->host[0] ? sauth->host : "NXDOMAIN"));
          
      break;
    }
    case SAUTH_TYPE_AUTH:
    {
      log(sauth_log, L_verbose, "AUTH query (#%u) %s. (%s:%u -> %s)",
          sauth->id, what, net_ntoa(sauth->addr), sauth->local,
          (sauth->ident[0] ? sauth->ident : "<unknown>"));
      break;
    }
    case SAUTH_TYPE_PROXY:
    {
      char buf[32];
      
      net_ntoa_r(sauth->connect, buf);
      
      log(sauth_log, L_verbose, "PROXY check (#%u) %s. (%s:%u -> %s:%u) [%s]: %s",
          sauth->id, what, 
          net_ntoa(sauth->addr), (uint32_t)sauth->remote,
          buf, (uint32_t)sauth->local, sauth_types[sauth->ptype], sauth_replies[sauth->reply]);
      break;
    }
  }
  
  if(sauth->callback)
    sauth->callback(sauth, sauth->args[0], sauth->args[1],
                           sauth->args[2], sauth->args[3]);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static int sauth_parse(char **argv)
{
  int      serial;
  struct sauth *sauth;
  
  if(!stricmp(argv[0], "dns"))
  {
    serial = atoi(argv[2]);
    
    sauth = sauth_find(serial);
    
    if(sauth)
    {
      if(sauth->type == SAUTH_TYPE_DNSF)
      {
        if(!stricmp(argv[1], "forward"))
        {
          if(argv[3])
            net_aton(argv[3], &sauth->addr);
          else
            sauth->addr.s_addr = INADDR_ANY;

          sauth_callback(sauth, SAUTH_DONE);
          
          return 0;
        }
      }
      
      if(sauth->type == SAUTH_TYPE_DNSR)
      {
        if(!stricmp(argv[1], "reverse"))
        {
          if(argv[3])
            strlcpy(sauth->host, argv[3], sizeof(sauth->host));
          else
            sauth->host[0] = '\0';
          
          sauth_callback(sauth, SAUTH_DONE);
          
          return 0;
        }
      }
      
      sauth_callback(sauth, SAUTH_ERROR);

      return -1;
    }
    
    return 0;
  }
  else if(!stricmp(argv[0], "auth"))
  {
    serial = atoi(argv[1]);

    sauth = sauth_find(serial);
    
    if(sauth)
    {
      if(argv[2])
        strlcpy(sauth->ident, argv[2], sizeof(sauth->ident));
      else
        sauth->ident[0] = '\0';
      
      sauth_callback(sauth, SAUTH_DONE);
    }
    
    return 0;
  }
  else if(!stricmp(argv[0], "proxy"))
  {
    serial = atoi(argv[1]);

    sauth = sauth_find(serial);
    
    if(sauth)
    {      
      sauth->reply = sauth_proxy_reply(argv[2]);
      
      sauth_callback(sauth, SAUTH_DONE);
    }
    
    return 0;
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static int sauth_recover(void) 
{
  struct node *node;
  struct node *next;
  
  dlink_foreach_safe(&sauth_list, node, next)
    sauth_callback((struct sauth *)node, SAUTH_ERROR);
   
  if(sauth_rtimer)
  {
    timer_remove(sauth_rtimer);
    sauth_rtimer = NULL;
  }
  
  sauth_fds[0] = -1;
  sauth_fds[1] = -1;
   
  sauth_rtimer = timer_start(sauth_launch, SAUTH_RELAUNCH);
  
  timer_note(sauth_rtimer, "relaunch for sauth child");
  
  sauth_child = NULL;
  
  return 0;
} 

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void sauth_read(int fd, void *ptr)
{
  int   len;
  char *argv[6];
  
  while(io_list[fd].recvq.lines)
  {
    if((len = io_gets(fd, sauth_readbuf, BUFSIZE)) == 0)
      break;

    strtokenize(sauth_readbuf, argv, 5);
    
    if(sauth_parse(argv))
    {
      log(sauth_log, L_warning, "Invalid reply from sauth!");
      
      io_shutup(fd);
    }
  }
  
  if(io_list[fd].status.closed)
  {
     child_kill(sauth_child);
     child_cancel(sauth_child);
     child_push(&sauth_child);
     
     log(sauth_log, L_warning, "sauth channel closed!");
     
     sauth_recover();
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int sauth_launch(void)
{
  if(sauth_child == NULL)
  {
    sauth_child = child_find_name("-sauth");
     
    if(sauth_child == NULL)
    {
      log(sauth_log, L_warning, "No servauth child block found!");
	  
      return 0;
    }
      
    child_set_callback(sauth_child, sauth_child_cb);
      
    if(sauth_child->status != CHILD_RUNNING)
    {
      child_launch(sauth_child);
      
      if(sauth_child->channels[0][PARENT][READ] > -1) 
      {
        sauth_fds[READ] = sauth_child->channels[0][PARENT][READ];
        
#ifdef HAVE_SOCKETPAIR
        sauth_fds[WRITE] = sauth_child->channels[0][PARENT][READ];
        io_queue_control(sauth_fds[READ], ON, ON, ON);
#else        
        sauth_fds[WRITE] = sauth_child->channels[0][PARENT][WRITE];
        io_queue_control(sauth_fds[READ], ON, OFF, ON);
        io_queue_control(sauth_fds[WRITE], OFF, ON, ON);
#endif /* HAVE_SOCKETPAIR */
        
        io_register(sauth_fds[READ], IO_CB_READ, sauth_read, NULL);
      }

      sauth_rtimer = NULL;

      return 1;
    }
 
    return 0;
  }

  sauth_rtimer = NULL;
  
  return 1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static struct sauth *sauth_new(int type)
{
  struct sauth *sauth;
  
  sauth = mem_static_alloc(&sauth_heap);
  
  sauth->id = sauth_serial++;
  sauth->type = type;
  sauth->host[0] = '\0';
  sauth->addr.s_addr = INADDR_ANY;
  sauth->remote = 0;
  sauth->local = 0;
  sauth->refcount = 1;
  
  dlink_add_head(&sauth_list, &sauth->node, sauth);
  
  return sauth;
}

/* -------------------------------------------------------------------------- *
 * Initialize sauth heap and add garbage collect timer.                       *
 * -------------------------------------------------------------------------- */
void sauth_init(void)
{
  sauth_log = log_source_register("sauth");
  
  /* Zero sauth block list */
  dlink_list_zero(&sauth_list);
  
  sauth_serial = 0;
  sauth_fds[0] = -1;
  sauth_fds[1] = -1;
  
  /* Setup sauth heap & timer */
  mem_static_create(&sauth_heap, sizeof(sauth_t), SAUTH_BLOCK_SIZE);
  mem_static_note(&sauth_heap, "sauth query heap");
  
  sauth_rtimer = timer_start(sauth_launch, SAUTH_RELAUNCH);
  
  timer_note(sauth_rtimer, "servauth relaunch timer");
  
  log(sauth_log, L_status, "Initialized [sauth] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy sauth heap and cancel timer.                                       *
 * -------------------------------------------------------------------------- */
void sauth_shutdown(void)
{
  struct node *node;
  struct node *next;
  
  log(sauth_log, L_status, "Shutting down [sauth] module...");

  timer_cancel(&sauth_timer);
  
  /* Remove all sauth blocks */
  dlink_foreach_safe(&sauth_list, node, next)
    sauth_delete((struct sauth *)node);

  mem_static_destroy(&sauth_heap);
  
  log_source_unregister(sauth_log);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sauth_collect(void)
{
  struct node  *node;
  struct sauth *saptr;
  
  dlink_foreach(&sauth_list, node)
  {
    saptr = node->data;
    
    if(!saptr->refcount)
      sauth_delete(saptr);
  }
  
  mem_static_collect(&sauth_heap);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct sauth *sauth_dns_forward(const char *address, void *callback, ...)
{
  struct sauth *sauth;
  va_list       args;
  
  if(!sauth_launch())
    return NULL;
  
  sauth = sauth_new(SAUTH_TYPE_DNSF);

  sauth->callback = callback;
  
  strlcpy(sauth->host, address, sizeof(sauth->host));
  
  va_start(args, callback);
  sauth_vset_args(sauth, args);
  va_end(args);
  
  io_puts(sauth_fds[WRITE], "dns forward %u %s",
          sauth->id, sauth->host);
  
  sauth->timer = timer_start(sauth_timeout, SAUTH_TIMEOUT, sauth);
  
  timer_note(sauth->timer, "sauth timeout (dns forward %s)", address);
  
  log(sauth_log, L_verbose, "Started DNS query for %s (#%u)",
      sauth->host, sauth->id);
  
  return sauth;
}     
     
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct sauth *sauth_dns_reverse(struct in_addr address, void *callback, ...)
{
  struct sauth *sauth;
  va_list       args;
  char          ipbuf[HOSTIPLEN + 1];
  
  if(!sauth_launch())
    return NULL;
  
  sauth = sauth_new(SAUTH_TYPE_DNSR);

  sauth->callback = callback;  
  sauth->addr.s_addr = address.s_addr;
  
  va_start(args, callback);
  sauth_vset_args(sauth, args);
  va_end(args);
  
  net_ntoa_r(sauth->addr, ipbuf);
  
  io_puts(sauth_fds[WRITE], "dns reverse %u %s",
          sauth->id, ipbuf);
  
  sauth->timer = timer_start(sauth_timeout, SAUTH_TIMEOUT, sauth);
  
  timer_note(sauth->timer, "sauth timeout (dns reverse %s)", 
             ipbuf);
  
  log(sauth_log, L_verbose, "Started DNS query for %s (#%u)", 
      ipbuf, sauth->id);
  
  return sauth;
}     
     
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct sauth *sauth_auth(struct in_addr address,  uint16_t local,
                         uint16_t       remote,   void    *callback, ...)
{
  struct sauth *sauth;
  va_list       args;
  
  if(!sauth_launch())
    return NULL;

  sauth = sauth_new(SAUTH_TYPE_AUTH);
  
  sauth->addr.s_addr = address.s_addr;
  sauth->local = local;
  sauth->remote = remote;
  sauth->callback = callback;
  
  va_start(args, callback);
  sauth_vset_args(sauth, args);
  va_end(args);

  io_puts(sauth_fds[WRITE], "auth %u %s %u %u",
          sauth->id, net_ntoa(sauth->addr), sauth->local, sauth->remote);
  
  sauth->timer = timer_start(sauth_timeout, SAUTH_TIMEOUT, sauth);
  
  timer_note(sauth->timer, "sauth timeout (auth %s %u %u)",
             net_ntoa(sauth->addr), sauth->local, sauth->remote);
  
  return sauth;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct sauth *sauth_proxy(int type,
                          struct in_addr remote_addr, uint16_t remote_port,
                          struct in_addr local_addr,  uint16_t local_port,   
                          void          *callback, ...)
{
  struct sauth *sauth;
  va_list       args;
  
  if(!sauth_launch())
    return NULL;

  sauth = sauth_new(SAUTH_TYPE_PROXY);
  
  sauth->addr.s_addr = remote_addr.s_addr;
  sauth->connect.s_addr = local_addr.s_addr;
  sauth->local = local_port;
  sauth->remote = remote_port;
  sauth->callback = callback;
  sauth->ptype = type;
  
  va_start(args, callback);
  sauth_vset_args(sauth, args);
  va_end(args);

  io_puts(sauth_fds[WRITE], "proxy %i %s:%u %s:%u %s",
          sauth->id,
          net_ntoa(sauth->addr), sauth->remote,
          net_ntoa(sauth->connect), sauth->local,
          sauth_types[sauth->ptype]);
  
  sauth->timer = timer_start(sauth_timeout, SAUTH_TIMEOUT, sauth);
  
  timer_note(sauth->timer, "sauth timeout (proxy x %s:%u %s:%u %s)",
             net_ntoa(sauth->addr), sauth->remote,
             net_ntoa(sauth->connect), sauth->local,
             sauth_types[sauth->ptype]);
  
  return sauth;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct sauth *sauth_find(uint32_t id)
{
  struct sauth *sauth;
  struct node  *node;
  
  dlink_foreach(&sauth_list, node)
  {
    sauth = node->data;
    
    if(sauth->id == id)
      return sauth;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct sauth *sauth_pop(struct sauth *sauth)
{
  if(sauth)
  {
    if(!sauth->refcount)
      log(sauth_log, L_warning, "Poping deprecated sauth #%u",
          sauth->id);
    
    sauth->refcount++;
  }
  
  return sauth;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct sauth *sauth_push(struct sauth **sauth)
{
  if(*sauth)
  {
    if((*sauth)->refcount == 0)
    {
      log(sauth_log, L_warning, "Trying to push deprecated sauth #%u",
          (*sauth)->id);
    }
    else
    {
      if(--(*sauth)->refcount == 0)
        sauth_delete(*sauth);
      
      (*sauth) = NULL;
    }
  }
  
  return *sauth;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sauth_delete(struct sauth *sauth)
{
  timer_cancel(&sauth->timer);
  
  dlink_delete(&sauth_list, &sauth->node);
  
  mem_static_free(&sauth_heap, sauth);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sauth_vset_args(struct sauth *sauth, va_list args)
{
  sauth->args[0] = va_arg(args, void *);
  sauth->args[1] = va_arg(args, void *);
  sauth->args[2] = va_arg(args, void *);
  sauth->args[3] = va_arg(args, void *);
}

void sauth_set_args(struct sauth *sauth, ...)
{
  va_list args;
  
  va_start(args, sauth);
  sauth_vset_args(sauth, args);
  va_end(args);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sauth_dump(struct sauth *saptr)
{
  if(saptr == NULL)
  {
    dump(sauth_log, "[================ sauth summary ================]");
    
    dump(sauth_log, "------------------ dns reverse ------------------");

    dlink_foreach(&sauth_list, saptr)
    {
      if(saptr->type == SAUTH_TYPE_DNSR)
        dump(sauth_log, " #%u: [%u] %-20s (%p)",
             saptr->id, saptr->refcount, net_ntoa(saptr->addr), 
             saptr->callback);
    }

    dump(sauth_log, "------------------ dns forward ------------------");

    dlink_foreach(&sauth_list, saptr)
    {
      if(saptr->type == SAUTH_TYPE_DNSF)
        dump(sauth_log, " #%u: [%u] %-20s (%p)",
             saptr->id, saptr->refcount, saptr->host,
             saptr->callback);
    }

    dump(sauth_log, "---------------------- auth ---------------------");

    dlink_foreach(&sauth_list, saptr)
    {
      if(saptr->type == SAUTH_TYPE_AUTH)
        dump(sauth_log, " #%u: [%u] %-20s (%p)",
             saptr->id, saptr->refcount, net_ntoa(saptr->addr),
             saptr->callback);
    }

    dump(sauth_log, "[============= end of sauth summary ============]");
  }
  else
  {
    dump(sauth_log, "[================= sauth dump ==================]");
    
    dump(sauth_log, "         id: #%u", saptr->id);
    dump(sauth_log, "   refcount: %u", saptr->refcount);
    dump(sauth_log, "       type: %s", 
         saptr->type == SAUTH_TYPE_DNSR ? "dns reverse" : 
         saptr->type == SAUTH_TYPE_DNSF ? "dns forward" : 
         saptr->type == SAUTH_TYPE_AUTH ? "auth" : "proxy");
    dump(sauth_log, "     status: %s", 
         saptr->status == SAUTH_DONE ? "done" :
         saptr->status == SAUTH_ERROR ? "error" : "timed out");
    dump(sauth_log, "       host: %s", saptr->host);
    dump(sauth_log, "      ident: %s", saptr->ident);
    dump(sauth_log, "       addr: %s", net_ntoa(saptr->addr));
    dump(sauth_log, "    connect: %s", net_ntoa(saptr->connect));
    dump(sauth_log, "     remote: %u", (uint32_t)saptr->remote);
    dump(sauth_log, "      local: %u", (uint32_t)saptr->local);
    dump(sauth_log, "      timer: %i", saptr->timer ? saptr->timer->id : -1);
    dump(sauth_log, "       args: %p, %p, %p, %p",
         saptr->args[0], saptr->args[1], saptr->args[2], saptr->args[3]);
    dump(sauth_log, "   callback: %p", saptr->callback);
    
    dump(sauth_log, "[============== end of sauth dump ==============]");
  }
}
