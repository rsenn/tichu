/************************************************************************
 *   IRC - Internet Relay Chat, servauth/commands.h
 *
 *   Copyright (C) 2003 by Roman Senn <smoli@paranoya.ch>
 * 
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   $Id: commands.h,v 1.1 2003/08/15 02:09:20 smolie Exp $
 */

#ifndef SERVAUTH_COMMANDS_H
#define SERVAUTH_COMMANDS_H

struct control;

typedef int (cmd_t)(struct control *, int, char **);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct cmd_table {
  char    *name; /* name of command */
  cmd_t   *func; /* function to call */
};

extern struct cmd_table cmds[];
extern struct cmd_table dns_cmds[];

/* -------------------------------------------------------------------------- *
 * find a command in the cmd table                                            *
 *                                                                            *
 * cmd_table  - the command table                                             *
 * name       - name of the command to find                                   *
 * -------------------------------------------------------------------------- */
extern struct cmd_table *command_get (struct cmd_table *cmd_table, 
                                      const char       *name);

#endif /* SERVAUTH_COMMANDS_H */
