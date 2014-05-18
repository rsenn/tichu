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
 * $Id: connect.c,v 1.64 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/connect.h>
#include <libchaos/sauth.h>
#include <libchaos/defs.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/net.h>
#include <libchaos/str.h>

#define CONNECT_LOGLEVEL (cnptr->silent ? L_verbose : L_status)

/* -------------------------------------------------------------------------- *
 * Prototypes                                                                 *
 * -------------------------------------------------------------------------- */

static int  connect_timeout (struct connect *cnptr);
static void connect_read    (int             fd,
                             struct connect *cnptr);
static void connect_write   (int             fd,
                             struct connect *cnptr);
static void connect_sauth   (struct sauth   *saptr,
                             struct connect *cnptr);

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int           connect_log;          /* Log source */
struct sheap  connect_heap;         /* Heap containing connect blocks */
struct list   connect_list;         /* List linking connect blocks */
uint32_t      connect_id;           /* Next id number */
int           connect_dirty;

/* -------------------------------------------------------------------------- *
 * Initialize the connect module.                                             *
 * -------------------------------------------------------------------------- */
void connect_init(void)
{
  /* Get a log source */
  connect_log = log_source_register("connect");

  /* Create connect block heap */
  mem_static_create(&connect_heap, sizeof(struct connect), CONNECT_BLOCK_SIZE);
  mem_static_note(&connect_heap, "connect block heap");

  /* Zero connect block list */
  dlink_list_zero(&connect_list);
  
  connect_dirty = 0;
  
  /* Report success */
  log(connect_log, L_status, "Initialized [connect] module.");
}

/* -------------------------------------------------------------------------- *
 * Shut down the connect module.                                              *
 * -------------------------------------------------------------------------- */
void connect_shutdown(void)
{
  struct connect *cnptr;
  struct connect *next;
  
  /* Report status */
  log(connect_log, L_status, "Shutting down [connect] module...");
  
  /* Remove all connect blocks */
  dlink_foreach_safe(&connect_list, cnptr, next)
  {
    if(cnptr->refcount)
      cnptr->refcount--;
    
    connect_delete(cnptr);
  }
  
  /* Destroy connect block heap */
  mem_static_destroy(&connect_heap);
  
  /* Unregister log source */
  log_source_unregister(connect_log);
}

/* -------------------------------------------------------------------------- *
 * Collect connect block garbage.                                             *
 * -------------------------------------------------------------------------- */
int connect_collect(void)
{
  struct connect *cnptr;
  struct connect *next;
  size_t          n = 0;
  
  if(connect_dirty)
  {
    /* Report verbose */
    log(connect_log, L_verbose, "Doing garbage collect for [connect] module.");
    
    /* Free all connect blocks with a zero refcount */
    dlink_foreach_safe(&connect_list, cnptr, next)
    {
      if(!cnptr->refcount)
      {
        connect_delete(cnptr);
        
        n++;
      }
    }
    
    /* Collect garbage on connect_heap */
    mem_static_collect(&connect_heap);
    
    connect_dirty = 0;
  }

  return 0;
}
  
/* -------------------------------------------------------------------------- *
 * Fill a connect block with default values.                                  *
 *                                                                            *
 * <cnptr>           pointer to connect block                                 *
 * -------------------------------------------------------------------------- */
void connect_default(struct connect *cnptr)
{
  dlink_node_zero(&cnptr->node);
  
  /* Initialise block info */
  cnptr->refcount = 0;
  cnptr->id = 0;
  cnptr->chash = strihash(CONNECT_DEFAULT_ADDR) ^ CONNECT_DEFAULT_PORT;
  cnptr->nhash = strihash(CONNECT_DEFAULT_NAME);
  cnptr->status = CONNECT_IDLE;
  
  /* Zero references */
  cnptr->timer = NULL;
  cnptr->proto = NULL;
  
  /* Internal stuff */
  cnptr->fd = -1;
  cnptr->active = 1;
  cnptr->addr_remote.s_addr = INADDR_LOOPBACK;
  cnptr->addr_local.s_addr = INADDR_ANY;
  cnptr->port_remote = 0;
  cnptr->port_local = 0;
  cnptr->start = 0LLU;
  cnptr->sauth = NULL;
  
  /* External stuff */
  cnptr->timeout = CONNECT_DEFAULT_TIMEOUT;
  cnptr->interval = CONNECT_DEFAULT_INTERVAL;  
  cnptr->autoconn = 0;
  cnptr->ssl = 0;
  cnptr->args = NULL;
  strcpy(cnptr->address, CONNECT_DEFAULT_ADDR);
  strcpy(cnptr->name, CONNECT_DEFAULT_NAME);
}
  
/* -------------------------------------------------------------------------- *
 * Add a new connect block if we got a valid address and a valid protocol     *
 * handler. Initialise all the user-supplied (externally initialised) and the *
 * internal stuff. If the autoconn flag is set the connect will be initiated  *
 * immediately.                                                               *
 *                                                                            *
 * <address>           A valid hostname or address                            *
 * <port>              A valid port                                           *
 * <pptr>              Pointer to a protocol block                            *
 * <timeout>           Connect timeout in msecs                               *
 * <interval>          Connect interval for autoconnect                       *
 * <autoconn>          Flag for autoconnect                                   *
 * <ssl>               SSL connection?                                        *
 *                                                                            *
 * Will return a pointer to the connect block or NULL on failure.             *
 * -------------------------------------------------------------------------- */
struct connect *connect_add(const char      *address,  uint16_t    port,
                            struct protocol *pptr,     uint64_t    timeout,
                            uint64_t         interval, int         autoconn,
                            int              ssl,      const char *context)
{
  struct connect *cnptr;
  
  /* Check if we got a protocol */
  if(pptr == NULL)
  {
    log(connect_log, L_warning, "No protocol");
    return NULL;
  }
  
  /* Allocate connect block */
  cnptr = mem_static_alloc(&connect_heap);
  
  strlcpy(cnptr->address, address, sizeof(cnptr->address));
  
  /* Externally initialised stuff */
  if(!net_aton(address, &cnptr->addr_remote))
  {
    cnptr->addr_remote.s_addr = INADDR_ANY;
    cnptr->resolve = 1;
  }
  else
  {
    cnptr->resolve = 0;
  }
    
  cnptr->port_remote = port;
  cnptr->proto = net_pop(pptr);
  cnptr->autoconn = autoconn;
  cnptr->interval = interval;
  cnptr->timeout = timeout;
  cnptr->args = NULL;
  cnptr->active = 1;
  cnptr->silent = 0;
  
  cnptr->ssl = 0;
  
  if(context)
  {
    strlcpy(cnptr->context, context, sizeof(cnptr->context));
#ifdef HAVE_SSL
    if((cnptr->ctxt = ssl_find_name(cnptr->context)))
      cnptr->ssl = ssl;
    else
      log(connect_log, L_warning, "Could not find SSL context '%s'.",
          cnptr->context);
#endif /* HAVE_SSL */
  }
  
  /* Internally initialised stuff */
  cnptr->fd = -1;
  cnptr->status = CONNECT_IDLE;
  cnptr->id = connect_id++;
  cnptr->refcount = 1;
  cnptr->addr_local.s_addr = INADDR_ANY;
  cnptr->port_local = 0;
  
  snprintf(cnptr->name, sizeof(cnptr->name), "%s:%u", address, port);  
  
  /* Add to connect list */
  dlink_add_tail(&connect_list, &cnptr->node, connect);
 
  log(connect_log, L_debug, "Added connect block: %s", cnptr->name);
  
  /* If we're auto-connecting then initiate it now */
  if(cnptr->autoconn && cnptr->interval)
  {
    /* Add timer for the retry */
    cnptr->timer = timer_start(connect_start, cnptr->interval, cnptr);
    
    timer_note(cnptr->timer, "connect timer for %s", cnptr->name);
      
    /* Inform about the retry */
    log(connect_log, CONNECT_LOGLEVEL, "Initiating connect to %s in %llu msecs.",
        cnptr->name, cnptr->interval);
  }
  
  return cnptr;
}     
     
/* -------------------------------------------------------------------------- *
 * Update the externally initialised stuff of a connect block.                *
 *                                                                            *
 * <cnptr>             The connect block to update                            *
 * <address>           A valid hostname or address                            *
 * <port>              A valid port                                           *
 * <pptr>              Pointer to a protocol block                            *
 * <timeout>           Connect timeout in msecs                               *
 * <interval>          Connect interval for autoconnect                       *
 * <autoconn>          Flag for autoconnect                                   *
 * <ssl>               SSL connection?                                        *
 * -------------------------------------------------------------------------- */
int connect_update(struct connect *cnptr,    const char      *address,
                   uint16_t        port,     struct protocol *pptr,   
                   uint64_t        timeout,  uint64_t         interval,
                   int             autoconn, int              ssl,
                   const char     *context)
{
  /* Check if we got a protocol */
  if(pptr == NULL)
  {
    /* Maybe protocol changed */
    if(cnptr->proto != pptr)
    {
      net_push(&cnptr->proto);
      cnptr->proto = net_pop(pptr);
    }

    log(connect_log, L_warning, "No protocol");
    
    return -1;
  }
  
  /* Only update the following two if they're valid */
  if(address)
  {
    if(!net_aton(address, &cnptr->addr_remote))
    {
      cnptr->addr_remote.s_addr = INADDR_ANY;
      cnptr->resolve = 1;
    }
    else
    {
      cnptr->resolve = 0;
    }
    
    cnptr->port_remote = port;
  }
    
  /* Always update these */  
  cnptr->autoconn = autoconn;
  cnptr->interval = interval;
  cnptr->timeout = timeout;
  cnptr->ssl = 0;
  
  if(context)
  {
    strlcpy(cnptr->context, context, sizeof(cnptr->context));
#ifdef HAVE_SSL
    if((cnptr->ctxt = ssl_find_name(cnptr->context)))
      cnptr->ssl = ssl;
    else
      log(connect_log, L_warning, "Could not find SSL context '%s'.",
          cnptr->context);
#endif /* HAVE_SSL */
  }
  
  /* Update the name if none has been set */
  if(cnptr->name[0] == '\0')
     snprintf(cnptr->name, sizeof(cnptr->name), "%s:%u", address, port);    
  
  log(connect_log, L_status, "Updated connect block: %s", cnptr->name);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Remove and free a connect block.                                           *
 * -------------------------------------------------------------------------- */
void connect_delete(struct connect *cnptr)
{
  log(connect_log, L_status, "Deleting connect block: %s", cnptr->name);
  
  if(cnptr->args)
    free(cnptr->args);
    
  /* Cancels timers, shuts down sockets and stuff */
  connect_cancel(cnptr);
  
  /* Remove from the list and free */
  dlink_delete(&connect_list, &cnptr->node);
  
  mem_static_free(&connect_heap, cnptr);
}

 /* -------------------------------------------------------------------------- *
  * Loose all references                                                       *
  * -------------------------------------------------------------------------- */
void connect_release(struct connect *cnptr)
{  
  io_push(&cnptr->fd);
  net_push(&cnptr->proto);
  
  if(cnptr->timer)
  {
    timer_remove(cnptr->timer);
    cnptr->timer = NULL;
  }
  
  connect_dirty = 1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct connect *connect_pop(struct connect *cnptr)
{
  if(cnptr)
  {
    if(!cnptr->refcount)
      log(connect_log, L_warning, "Poping deprecated connect: %s",
          cnptr->name);
    
    cnptr->refcount++;
  }

  return cnptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct connect *connect_push(struct connect **cnptrptr)
{
  if(*cnptrptr)
  {
    if(!(*cnptrptr)->refcount)
    {
      log(connect_log, L_warning, "Trying to push deprecated connect %s",
          (*cnptrptr)->name);
    }
    else
    {
      if(--(*cnptrptr)->refcount == 0)
        connect_release(*cnptrptr);
    }
    
    (*cnptrptr) = NULL;
  }
  
  return *cnptrptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int connect_start(struct connect *cnptr)
{
  if(cnptr->status == CONNECT_IDLE)
  {
    cnptr->active = 1;
    
    if(cnptr->timer)
    {
      timer_remove(cnptr->timer);
      cnptr->timer = NULL;
    }
    
    if(cnptr->resolve)
      return connect_resolve(cnptr);
    else
      return connect_initiate(cnptr);
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int connect_resolve(struct connect *cnptr)
{
  log(connect_log, CONNECT_LOGLEVEL, "Looking up %s...", cnptr->address);
  
  cnptr->sauth = sauth_dns_forward(cnptr->address, connect_sauth, cnptr);
  
  cnptr->status = CONNECT_RESOLVING;
  
  if(cnptr->timeout && cnptr->timer == NULL)
  {
    cnptr->timer = timer_start(connect_timeout, cnptr->timeout, cnptr);
    
    timer_note(cnptr->timer, "Connect timeout for %s",
               cnptr->name);
  }

  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int connect_initiate(struct connect *cnptr)
{
  if(cnptr->fd != -1)
    return -1;
  
  cnptr->fd = net_socket(AF_INET, SOCK_STREAM);
  cnptr->start = timer_mtime;
   
  if(net_connect(cnptr->fd, cnptr->addr_remote,
                 cnptr->port_remote, connect_read, connect_write, cnptr))
  {
    io_shutup(cnptr->fd);
    
    cnptr->proto->handler(cnptr->fd, cnptr, cnptr->args);
    cnptr->fd = -1;
    cnptr->status = CONNECT_ERROR;
    
    log(connect_log, CONNECT_LOGLEVEL, "Connect to %s failed: %s",
        cnptr->name, syscall_strerror(syscall_errno));
    
    connect_retry(cnptr);
    
    return -1;
  }
  else
  {
    log(connect_log, CONNECT_LOGLEVEL, "Initiated connect to %s [%s:%u]",
        cnptr->name, cnptr->address, (uint32_t)cnptr->port_remote);

    cnptr->status = CONNECT_CONNECTING;
    
    io_queue_control(cnptr->fd, ON, ON, ON);

    if(cnptr->timeout && cnptr->timer == NULL)
    {
      cnptr->timer = timer_start(connect_timeout, cnptr->timeout, cnptr);
      
      timer_note(cnptr->timer, "Connect timeout for %s",
                 cnptr->name);
    }
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void connect_cancel(struct connect *cnptr)
{
  if(cnptr->fd > -1)
  {
    io_shutup(cnptr->fd);
    cnptr->fd = -1;
  }

  if(cnptr->timer)
  {
    timer_remove(cnptr->timer);
    cnptr->timer = NULL;
  }
  
  if(cnptr->sauth)
  {
    sauth_delete(cnptr->sauth);
    cnptr->sauth = NULL;
  }
  
  cnptr->active = 0;
  cnptr->start = timer_mtime;
  cnptr->status = CONNECT_IDLE;
}
  
/* -------------------------------------------------------------------------- *
 * If the connect block has an interval value then schedule a connection      *
 * retry. The retry will occurr in the remaining time from last connection    *
 * initiation (cnptr->start) 'til next interval (cnptr->start + interval)     *
 * Returns 0 if a retry was scheduled.                                        *
 * -------------------------------------------------------------------------- */
int connect_retry(struct connect *cnptr)
{
  int64_t delta;
  
  if(cnptr->interval && cnptr->fd == -1 && cnptr->timer == NULL && cnptr->active)
  {
    cnptr->status = 0;
    
    /* Calculate remaining time */
    delta = cnptr->interval - (timer_mtime - cnptr->start);
    
    if(delta < 0LL)
      delta = 0LL;
    
    /* Add timer for the retry */
    cnptr->timer = timer_start(connect_start, delta, cnptr);
   
    timer_note(cnptr->timer, "connect retry for %s", cnptr->name);
    
    /* Inform about the retry */
    if(delta)
      log(connect_log, CONNECT_LOGLEVEL, "Retrying connect to %s in %llu msecs.", 
          cnptr->name, delta);
    
    return 0;
  }
  
  /* There was no interval */
  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void connect_set_arg(struct connect *cnptr, void *arg)
{
  cnptr->args = arg;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void connect_set_args(struct connect *cnptr, const void *argbuf, size_t n)
{
  if(!cnptr->args)
    cnptr->args = malloc(n);
  
  memcpy(cnptr->args, argbuf, n);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void *connect_get_args(struct connect *cnptr)
{
  return cnptr->args;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void connect_set_name(struct connect *cnptr, const char *name)
{
  strlcpy(cnptr->name, name, sizeof(cnptr->name));
  
  cnptr->nhash = strihash(cnptr->name);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
const char *connect_get_name(struct connect *cnptr)
{
  return cnptr->name;
}  
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct connect *connect_find_name(const char *name)
{
  struct connect *cnptr;
  uint32_t        nhash;
  
  nhash = strihash(name);
  
  dlink_foreach(&connect_list, cnptr)
  {
    if(cnptr->nhash == nhash)
    {
      if(!stricmp(cnptr->name, name))
        return cnptr;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct connect *connect_find_id(uint32_t id)
{
  struct connect *cnptr;
  
  dlink_foreach(&connect_list, cnptr)
  {
    if(cnptr->id == id)
      return cnptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Connection has timed out. Warn about it, call the connect callback, close  *
 * the underlying socket and schedule a retry if there is a connection        *
 * interval.                                                                  *
 *                                                                            *
 * <connect>              - pointer to connect block                          *
 *                                                                            *
 * Always returns 1 due to timer cancellation.                                *
 * -------------------------------------------------------------------------- */
static int connect_timeout(struct connect *cnptr)
{
  log(connect_log, L_warning, "Connection to %s timed out.", cnptr->name);
  
  /* Connect callback */
  if(cnptr->proto)
    cnptr->proto->handler(cnptr->fd, cnptr, cnptr->args);

  /* Close underlying socket */
  if(cnptr->fd != -1)
  {
    io_shutup(cnptr->fd);
  
    cnptr->fd = -1;
  }

  if(cnptr->sauth)
  {
    sauth_delete(cnptr->sauth);
    cnptr->sauth = NULL;
  }
  
  if(cnptr->timer)
  {
    timer_remove(cnptr->timer);
    cnptr->timer = NULL;
  }
  
  /* Schedule retry */
  connect_retry(cnptr);
  
  /* Timer done */
  return 0;
}

/* -------------------------------------------------------------------------- *
 * The socket of the connect block got readable while connecting. This most   *
 * likely means that there was an error on the connection. If so spit out a   *
 * warning, call the connect callback, close underlying socket, cancel the    *
 * timeout timer and finally schedule a retry.                                *
 * -------------------------------------------------------------------------- */
static void connect_read(int fd, struct connect *cnptr)
{
  /* Uh, something went wrong *sigh* */
  if(cnptr->fd < 0)
    return;
  
  /* Socket has been closed by I/O code */
  if(io_list[fd].status.closed || io_list[fd].status.err)
  {
    log(connect_log, L_warning, "Failed connecting to %s: %s",
        cnptr->name, syscall_strerror(io_list[fd].error));
    
    /* Connect callback */
    if(cnptr->proto)
      cnptr->proto->handler(cnptr->fd, cnptr, cnptr->args);
    
    /* Close underlying socket */
    io_shutup(fd);
  
    cnptr->fd = -1;
   
    cnptr->status = CONNECT_ERROR;

    /* Cancel the timeout timer */
    if(cnptr->timer)
    {
      timer_remove(cnptr->timer);
      cnptr->timer = NULL;
    }

    /* Schedule a retry */
    connect_retry(cnptr);
  }
}

/* -------------------------------------------------------------------------- *
 * The socket of the connect block got writeable while connecting. This most  *
 * likely means that the connection was successfully established.             *
 * Maybe there was an error, if so call the readable handler above.           *
 * Else cancel the timeout timer, report success and call the connection      *
 * callback.                                                                  *
 * -------------------------------------------------------------------------- */
static void connect_write(int fd, struct connect *cnptr)
{
  /* Uh, something went wrong */
  if(cnptr->fd < 0)
    return;
  
  if(cnptr->status == CONNECT_CONNECTING)
  {
    /* Socket has been closed by I/O code, call the readable handler */
    if(io_list[fd].status.closed)
    {
      connect_read(fd, cnptr);
      return;
    }
    
#ifdef HAVE_SSL
    if(cnptr->ssl && io_list[fd].callbacks[IO_CB_READ] == (void *)connect_read)
    {
      if(ssl_new(fd, cnptr->ctxt))
      {
        cnptr->status = CONNECT_ERROR;
        io_close(fd);
        connect_retry(cnptr);
        return;
      }
      
      if(ssl_connect(fd))
      {
        log(connect_log, L_warning, "SSL handshake error on %s: %s",
            cnptr->name, ssl_strerror(fd));
        
        ssl_close(fd);
        io_close(fd);
        connect_retry(cnptr);
        return;
      }
      
      if(io_list[fd].sslstate)
      {
        io_list[fd].callbacks[IO_CB_READ] = io_list[fd].callbacks[IO_CB_WRITE];
        return;
      }
    }
#endif /* HAVE_SSL */
    
/*    io_list[fd].status.connecting = 0;*/
    
    /* Cancel the timeout timer */
    if(cnptr->timer)
    {
      timer_remove(cnptr->timer);
      cnptr->timer = NULL;
    }

    /* Report success */
    log(connect_log, L_warning, "Connection to %s succeeded",
        cnptr->name);
    
    cnptr->status = CONNECT_DONE;
    
    /* Call the connection callback */
    if(cnptr->proto)
      cnptr->proto->handler(cnptr->fd, cnptr, cnptr->args);

    cnptr->fd = -1;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void connect_sauth(struct sauth *saptr, struct connect *cnptr)
{
  if(saptr->status == SAUTH_DONE)
  {
    cnptr->addr_remote = saptr->addr;
    
    log(connect_log, CONNECT_LOGLEVEL, "DNS lookup for %s done: %s",
        cnptr->address, net_ntoa(cnptr->addr_remote));
    
    connect_initiate(cnptr);
    
    return;
  }
  
  if(saptr->status == SAUTH_ERROR)
  {
    log(connect_log, L_warning, "Failed resolving %s.", cnptr->address);
  }
  else if(saptr->status == SAUTH_TIMEDOUT)
  {
    log(connect_log, L_warning, "Timed out while resolving %s.", cnptr->address);
  }
}

/* -------------------------------------------------------------------------- *
 * Dump connects and connect heap.                                            *
 * -------------------------------------------------------------------------- */
void connect_dump(struct connect *cnptr)
{
  if(cnptr == NULL)
  {
    dump(connect_log, "[============== connect summary ===============]");
    
    dlink_foreach(&connect_list, cnptr)
    {
      dump(connect_log, " #%u: [%u] %-20s {%s:%u}",
            cnptr->id, cnptr->refcount, cnptr->name, 
            net_ntoa(cnptr->addr_remote), cnptr->port_remote);
    }
    
    dump(connect_log, "[=========== end of connect summary ===========]");
  }
  else
  {
    dump(connect_log, "[============== connect dump ===============]");
    dump(connect_log, "         id: #%u", cnptr->id);
    dump(connect_log, "   refcount: %u", cnptr->refcount);
    dump(connect_log, "      chash: %p", cnptr->chash);
    dump(connect_log, "      nhash: %p", cnptr->nhash);
    dump(connect_log, "     status: %u",
         cnptr->status == CONNECT_DONE ? "done" :
         cnptr->status == CONNECT_ERROR ? "error" :
         cnptr->status == CONNECT_TIMEOUT ? "timeout" :
         cnptr->status == CONNECT_CONNECTING ? "connecting" :
         cnptr->status == CONNECT_RESOLVING ? "resolving" : "idle");
    dump(connect_log, "      timer: %i", cnptr->timer ? cnptr->timer->id : -1);
    dump(connect_log, "      proto: %s", 
          cnptr->proto ? cnptr->proto->name : "(null)");
    dump(connect_log, "         fd: %i", cnptr->fd);
    dump(connect_log, "     active: %i", cnptr->active);
    dump(connect_log, "     silent: %i", cnptr->silent);
    dump(connect_log, "     remote: %s:%u", 
          net_ntoa(cnptr->addr_remote), cnptr->port_remote);
    dump(connect_log, "      local: %s:%u", 
          net_ntoa(cnptr->addr_local), cnptr->port_local);
    dump(connect_log, "      start: %llu", cnptr->start);
    dump(connect_log, "    timeout: %llu", cnptr->timeout);
    dump(connect_log, "   interval: %llu", cnptr->interval);
    dump(connect_log, "   autoconn: %s", cnptr->autoconn ? "on" : "off");
    dump(connect_log, "        ssl: %s", cnptr->ssl ? "yes" : "no");
    dump(connect_log, "       args: %p", cnptr->args);
    dump(connect_log, "    address: %s", cnptr->address);
    dump(connect_log, "       name: %s", cnptr->name);
    dump(connect_log, "[=========== end of connect dump ===========]");
  }  
}
