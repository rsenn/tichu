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
 * $Id: control.h,v 1.3 2004/12/31 03:39:14 smoli Exp $
 */

#ifndef SERVAUTH_CONTROL_H
#define SERVAUTH_CONTROL_H

#include <libchaos/queue.h>

typedef struct control {
  int        recvfd;
  int        sendfd;
  char      *name;
} control_t;

/* -------------------------------------------------------------------------- *
 * set control connection fd.                                                 *
 * -------------------------------------------------------------------------- */
extern void control_init        (control_t *cptr, 
                                 int        recvfd, 
                                 int        sendfd);
  
extern void control_zero        (control_t *cptr);

extern void control_set_fd      (control_t *cptr, 
                                 int        recvfd, 
                                 int        sendfd);

extern int  control_pre_select  (control_t *cptr, 
                                 int       *hsck,
                                 fd_set    *rfds, 
                                 fd_set    *wfds);

extern int  control_post_select (control_t    *cptr, 
                                 const fd_set *rfds,
                                 const fd_set *wfds);

extern int  control_send        (control_t    *cptr,
                                 const char   *format, 
                                 ...);

#endif /* SERVAUTH_CONTROL_H */
