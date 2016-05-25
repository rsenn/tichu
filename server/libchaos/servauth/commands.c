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
 * $Id: commands.c,v 1.10 2004/12/31 03:39:14 smoli Exp $
 */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/timer.h>
#include <libchaos/queue.h>
#include <libchaos/log.h>
#include <libchaos/net.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include "commands.h"
#include "control.h"
#include "servauth.h"
#include "cache.h"
#include "dns.h"
#include "auth.h"
#include "proxy.h"
#include "query.h"

/* -------------------------------------------------------------------------- *
 * command prototypes                                                         *
 * -------------------------------------------------------------------------- */
static int cmd_dns         (control_t *cptr, int ac, char **av);
static int cmd_auth        (control_t *cptr, int ac, char **av);
static int cmd_dns_forward (control_t *cptr, int ac, char **av);
static int cmd_dns_reverse (control_t *cptr, int ac, char **av);
static int cmd_proxy       (control_t *cptr, int ac, char **av);

/* -------------------------------------------------------------------------- *
 * main commands                                                              *
 * -------------------------------------------------------------------------- */
struct cmd_table cmds[] = {
  { "dns",     &cmd_dns   },
  { "auth",    &cmd_auth  },
  { "proxy",   &cmd_proxy },
  { NULL,      NULL       }
};

/* -------------------------------------------------------------------------- *
 * subcommands for dns                                                        *
 * -------------------------------------------------------------------------- */
struct cmd_table dns_cmds[] = {
  { "reverse", cmd_dns_reverse },
  { "forward", cmd_dns_forward },
  { NULL,      NULL            }
};

/* -------------------------------------------------------------------------- *
 * command_get - find a command in the cmd table                              *
 *                                                                            *
 * cmd_table  - the command table                                             *
 * name       - name of the command to find                                   *
 * -------------------------------------------------------------------------- */
struct cmd_table *command_get(struct cmd_table *cmd_table, const char *name)
{
  struct cmd_table *cmdptr;

  if(!cmd_table || !name)
    return NULL;

  for(cmdptr = cmd_table; cmdptr->name; cmdptr++)
  {
    if(!strcmp(name, cmdptr->name))
      return cmdptr;
  }
  
  return NULL; /* no matches found */
}

/* -------------------------------------------------------------------------- *
 * cmd_dns() - resolve names and addressses                                   *
 *                                                                            *
 * av[0] = "auth"                                                             *
 * av[1] = "id"                                                               *
 * av[2] = "ip"                                                               *
 * av[3] = "src port"                                                         *
 * av[4] = "dst port"                                                         *
 * -------------------------------------------------------------------------- */
static int cmd_auth(control_t *cptr, int ac, char **av)
{

  struct servauth_query *q;
  struct in_addr addr;

  if(ac != 5)
    return -1;

  if(net_aton(av[2], &addr) == 0)
    return -1;

  /* check the cache */
  if(cache_auth_pick(&servauth_authcache, addr, timer_systime))
  {
    /* reply failure */
    control_send(&servauth_control, "auth %s", av[1]);
    return 0;
  }

  /* start query */
  q = query_find(servauth_queries, MAX_QUERIES);

  if(query_set_auth(q, av[1], av[2], av[3], av[4]))
    return -1;

  if(query_start(q, QUERY_AUTH, timer_systime) == -1)
    return -1;

  return 0;
}

/* -------------------------------------------------------------------------- *
 * cmd_dns - resolve names and addresses                                      *
 *                                                                            *
 * av[0] = "dns"                                                              *
 * av[1] = "foward" or "reverse"                                              *
 * av[3] = id                                                                 *
 * av[4] = hostname or ip address                                             *
 * -------------------------------------------------------------------------- */
static int cmd_dns(control_t *cptr, int ac, char **av)
{
  struct cmd_table *cmdptr;

  if(ac < 2)
    return -1;

  cmdptr = command_get(dns_cmds, av[1]);

  if(cmdptr && (cmdptr != (struct cmd_table *) -1))
    return cmdptr->func(cptr, ac, av);

  return 0;
}

/* -------------------------------------------------------------------------- *
 * cmd_dns_forward - resolve names                                            *
 *                                                                            *
 * av[0] = "dns"                                                              *
 * av[1] = "forward"                                                          *
 * av[2] = id                                                                 *
 * av[3] = hostname                                                           *
 * -------------------------------------------------------------------------- */
static int cmd_dns_forward(control_t *cptr, int ac, char **av)
{
  struct servauth_query *q;
#if 0
  struct in_addr         addr;
#endif  
  int                    ret;

  if(ac != 4)
    return -1;

#if 0
  /* check the cache */
  addr = cache_dns_pick_forward(&servauth_dnscache, av[3], timer_systime);

  /* reply cached response */
  if(addr.s_addr != INADDR_ANY)
  {
    control_send(cptr, "dns forward %s %s", av[2], net_ntoa(addr));
    return 0;
  }
#endif

  q = query_find(servauth_queries, MAX_QUERIES);

  if(query_set_host(q, av[2], av[3]))
    return -1;

  if((ret = query_start(q, QUERY_DNS_FORWARD, timer_systime)) == -1)
    return -1;

  return 0;
}

/* -------------------------------------------------------------------------- *
 * cmd_dns_reverse - resolve addresses                                        *
 *                                                                            *
 * av[0] = "dns"                                                              *
 * av[1] = "reverse"                                                          *
 * av[2] = id                                                                 *
 * av[3] = ip address                                                         *
 * -------------------------------------------------------------------------- */
static int cmd_dns_reverse(control_t *cptr, int ac, char **av)
{
  struct servauth_query *q;
  int ret;
  const char *name;
  struct in_addr addr;

  if(ac != 4)
    return -1;

  if(net_aton(av[3], &addr) == 0)
    return -1;
  
  /* check the cache */
  if(cache_dns_pick_reverse(&servauth_dnscache, addr, &name, timer_systime))
  {
    /* reply cached response */
    if(name)
      control_send(cptr, "dns reverse %s %s", av[2], name);
    else
      control_send(cptr, "dns reverse %s", av[2]);

    return 0;
  }

  q = query_find(servauth_queries, MAX_QUERIES);

  if(query_set_addr(q, av[2], av[3]))
    return -1;

  if((ret = query_start(q, QUERY_DNS_REVERSE, timer_systime)) == -1)
    return -1;

  return 0;
}

/* -------------------------------------------------------------------------- *
 * cmd_proxy() - detect proxies                                               *
 *                                                                            *
 * av[0] = "proxy"                                                            *
 * av[1] = "id"                                                               *
 * av[2] = "ip:port"                                                          *
 * av[3] = "testaddr:testport"                                                *
 * av[4] = "type"                                                             *
 * -------------------------------------------------------------------------- */
static int cmd_proxy(control_t *cptr, int ac, char **av)
{
  struct servauth_query *q;
  struct in_addr         addr;
  uint16_t               port;
  int                    type;
  int                    status;
  char                  *ptr;
  char                   remote[32];

  if(ac != 5)
    return -1;

  strlcpy(remote, av[2], sizeof(remote));
  
  if((ptr = strchr(remote, ':')) == NULL)
    return -1;
  
  *ptr++ = '\0';
  
  if(net_aton(remote, &addr) == 0)
    return -1;

  port = (uint16_t)strtoul(ptr, NULL, 10);
  type = proxy_parse_type(av[4]);
  
  /* check the cache */
  if((status = cache_proxy_pick(&servauth_proxycache, addr, port, type, timer_systime)))
  {
    control_send(&servauth_control, "proxy %s %s", av[1], proxy_replies[status]);
    
    return 0;
  }

  /* start query */
  q = query_find(servauth_queries, MAX_QUERIES);

  if(query_set_proxy(q, av[1], av[2], av[3], av[4]))
    return -1;

  if(query_start(q, QUERY_PROXY, timer_systime) == -1)
    return -1;

  return 0;
}

