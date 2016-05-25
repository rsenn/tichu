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
 * $Id: control.c,v 1.14 2005/01/08 22:18:50 smoli Exp $
 */

#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/syscall.h>
#include <libchaos/queue.h>
#include <libchaos/log.h>
#include <libchaos/net.h>
#include <libchaos/str.h>

#include "servauth.h"
#include "control.h"
#include "commands.h"

#define MAXPARA 64

#ifdef DEBUG
#undef HAVE_SOCKETPAIR
#endif /* DEBUG */

/* -------------------------------------------------------------------------- *
 * control_exec - execute the cmd in av[0]                                    *
 *                                                                            *
 * cptr    - pointer to control connection struct                             *
 * ac      - argument count                                                   *
 * av      - argument vector                                                  *
 * -------------------------------------------------------------------------- */
static int control_exec(control_t *cptr, int ac, char **av)
{
  struct cmd_table *cmdptr;

  if(ac < 1)
    return -1;
  
  cmdptr = command_get(cmds, av[0]);

  if(cmdptr != NULL)
    return cmdptr->func(cptr, ac, av);

  return -1;
}

/* -------------------------------------------------------------------------- *
 * parse a line                                                               *
 * -------------------------------------------------------------------------- */
static int control_parse(control_t *cptr, char *line)
{
  int ac;
  char *av[16];

  ac = strtokenize(line, av, 16);

  if(ac == 0)
    return 0;
  
  return control_exec(cptr, ac, av);
}

/* -------------------------------------------------------------------------- *
 * control fd got readable, handle data                                       *
 * -------------------------------------------------------------------------- */
static void control_readable(int fd, void *ptr)
{
  char buf[1024];
  
  if(io_list[fd].status.closed || io_list[fd].status.err)
    servauth_shutdown();
  
  if(io_list[fd].recvq.lines)
  {
    while(io_gets(fd, buf, 1024) > 0)
    {
      if(control_parse(ptr, buf) == -1)
      {
        servauth_shutdown();
        return;
      }
    }
  }

  return;
}

/* -------------------------------------------------------------------------- *
 * set control connection fd.                                                 *
 * -------------------------------------------------------------------------- */
void control_init(control_t *cptr, int recvfd, int sendfd)
{
  memset(cptr, 0, sizeof(control_t));
  
  cptr->recvfd = recvfd;
  cptr->sendfd = sendfd;
  
  if(sendfd != recvfd) 
  {
    io_queue_control(recvfd, ON, OFF, ON);
    io_queue_control(sendfd, OFF, ON, ON);
  }
  else
  {
    io_queue_control(recvfd, ON, ON, ON);
  }
  
  io_register(recvfd, IO_CB_READ, control_readable, cptr);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int control_send(control_t *cptr, const char *format, ...)
{
  va_list args;
  int ret;

  va_start(args, format);

  ret = io_vputs(cptr->sendfd, format, args);

  va_end(args);
  
  return ret;
}

