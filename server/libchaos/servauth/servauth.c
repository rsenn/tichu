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
 * $Id: servauth.c,v 1.24 2005/01/02 04:09:20 slurp Exp $
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <libchaos/defs.h>

#include <libchaos/connect.h>
#include <libchaos/dlink.h>
#include <libchaos/queue.h>
#include <libchaos/timer.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/str.h>
#include <libchaos/io.h>

#include "control.h"
#include "servauth.h"
#include "cache.h"
#include "auth.h"
#include "dns.h"
#include "proxy.h"
#include "query.h"

struct servauth_query servauth_queries[MAX_QUERIES];
struct control        servauth_control;              /* control connection :D */
struct cache_auth     servauth_authcache;
struct cache_dns      servauth_dnscache;
struct cache_proxy    servauth_proxycache;

/*static*/ uint32_t     servauth_log;
static struct dlog *servauth_drain;

/* usage - display usage message */
/*static void sauth_usage(void)
{
  log(F_startup, L_status, PACKAGE_NAME" server auth "PACKAGE_VERSION);
  log(F_startup, L_status, "$Id: servauth.c,v 1.24 2005/01/02 04:09:20 slurp Exp $\n");
  log(F_startup, L_status, "This program is called by "PACKAGE_NAME".");
  log(F_startup, L_status, "It cannot be used on its own.");
  exit(1);
}*/

/* -------------------------------------------------------------------------- *
 * Initialize things.                                                         *
 * -------------------------------------------------------------------------- */
void servauth_init(void)
{
  log_init(0, 0, 0);
  mem_init();
  timer_init();
  queue_init();
  dlink_init();  
  connect_init();

  syscall_signal(SIGPIPE, SIG_IGN);
  
  servauth_log = log_source_register("servauth");

  servauth_drain = log_drain_open("servauth.log", LOG_ALL, L_debug, 1, 1);
  
  log(servauth_log, L_status, "Done initialising ircd library");
}

/* -------------------------------------------------------------------------- *
 * Clean things up.                                                           *
 * -------------------------------------------------------------------------- */
void servauth_shutdown(void)
{
  log(servauth_log, L_status, "Shutting down libircd...");

  connect_shutdown();
  io_shutdown();
  queue_shutdown();
  dlink_shutdown();
  mem_shutdown();
  log_shutdown();
  timer_shutdown();
  
  log_source_unregister(servauth_log);
  
  syscall_exit(0);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void servauth_collect(void)
{
  connect_collect();
  dlink_collect();
  log_collect();
  timer_collect();
  
  log_source_unregister(servauth_log);
  
  exit(0);
}

/* -------------------------------------------------------------------------- *
 * Dump status information.                                                   *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void servauth_dump(void)
{
  debug(servauth_log, "--- sauth complete dump ---");
  
  connect_dump(NULL);
/*  log_dump(NULL);*/
  timer_dump(NULL);
/*  queue_dump(NULL);*/
  dlink_dump();

  debug(servauth_log, "--- end of sauth complete dump ---");
}
#endif /* DEBUG */

/* -------------------------------------------------------------------------- *
 * Loop around some timer stuff and the i/o multiplexer.                      *
 * -------------------------------------------------------------------------- */
void servauth_loop(void)
{
  int ret = 0;
  int64_t *timeout;
  int64_t remain = 0LL;
    
  while(ret >= 0)
  {
    /* Calculate timeout value */
    timeout = timer_timeout();
    
    /* Do I/O multiplexing and event handling */
#ifdef USE_POLL
    ret = io_poll(&remain, timeout);
#else
    ret = io_select(&remain, timeout);
#endif /* USE_SELECT | USE_POLL */
    
    /* Remaining time is 0msecs, we need to run a timer */
    if(remain == 0LL)
      timer_run();
    
    if(timeout)
      timer_drift(*timeout - remain);
    
    io_handle();
  }
}

/* -------------------------------------------------------------------------- *
 * Program entry.                                                             *
 * -------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
  int i, j;
  int recvfd = 0, sendfd = 1;
  struct in_addr localhost = { htonl(INADDR_LOOPBACK) };

  /* get control connection */
  if(argc >= 2)
    recvfd = strtol(argv[1], NULL, 10);
  
  if(argc >= 3)
    sendfd = strtol(argv[2], NULL, 10);
    
  io_init_except(recvfd, sendfd, -1);
    
  if(recvfd == sendfd)
  {
    /* socketpair() */
    io_new(recvfd, FD_SOCKET);
  }
  else
  {
    /* 2x pipe() */
    io_new(recvfd, FD_PIPE);
    io_new(sendfd, FD_PIPE);
  }

  /* initialize library */
  servauth_init();

  /* Make sure we are running under hybrid.. */
/*  if(strncmp(argv[0], "-sauth", 6))
    sauth_usage();*/

  /* clear argv */
  for(i = 1; i < argc; i++)
    for(j = 0; argv[i][j]; j++)
      argv[i][j] = '\0';

  /* set file descriptor to nonblocking mode */
/*  io_nonblock(recvfd);
  io_nonblock(sendfd);*/

  /* initialize dns client */
  dns_init();
  dns_updatedb();

  /* initialize caches */
  cache_auth_new(&servauth_authcache, CACHE_AUTH_SIZE);
  cache_dns_new(&servauth_dnscache, CACHE_DNS_SIZE);
  cache_proxy_new(&servauth_proxycache, CACHE_PROXY_SIZE);

  cache_dns_put(&servauth_dnscache, CACHE_DNS_REVERSE, localhost, "localhost", (uint64_t)-1LL);
  
  /* initialize control connection */
  control_init(&servauth_control, recvfd, sendfd);
  
  /* clear query structs */
  memset(servauth_queries, 0, sizeof(servauth_queries));

  /* enter io loop */
  servauth_loop();

  /* NOTREACHED */
  return(0);
}
