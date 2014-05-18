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
 * $Id: msg.h,v 1.5 2004/12/30 14:57:25 smoli Exp $
 */

#ifndef SRC_MSG_H
#define SRC_MSG_H

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/dlink.h>

#include "tichu.h"

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct player;
struct lplayer;

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */

/* Callback types */
#define MSG_UNREGISTERED 0x00         /* for unregistered players */
#define MSG_PLAYER       0x01         /* for registered players */
#define MSG_MEMBER       0x02
#define MSG_OPER         0x03         /* for opers */
#define MSG_LAST         MSG_OPER + 1 /* callback array size */

/* Hashtable size */
#define MSG_HASH_SIZE    32

/* Message flags */
#define MFLG_PLAYER 0x01 /* flood throttling */
#define MFLG_UNREG  0x02 /* available to unregistered players */
#define MFLG_IGNORE 0x04 /* ignore from unregistered players */
#define MFLG_HIDDEN 0x08 /* hidden from everyone */
#define MFLG_OPER   0x10 /* oper-only command */

/* -------------------------------------------------------------------------- *
 * Callback function type for message handlers                                *
 * -------------------------------------------------------------------------- */
typedef void (msg_handler_t)(struct player *, int, char **);

/* -------------------------------------------------------------------------- *
 * Message structure                                                          *
 * -------------------------------------------------------------------------- */
struct msg {
  char          *cmd;
  size_t         args;
  size_t         maxargs;
  size_t         flags;
  msg_handler_t *handlers[MSG_LAST];
  char         **help;
  uint32_t       hash;
  uint32_t       counts[MSG_LAST];
  size_t         bytes;
  uint32_t       id;
  struct node    node;
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int         msg_log;
extern struct list msg_table[MSG_HASH_SIZE];

/* -------------------------------------------------------------------------- *
 * Initialize message heap.                                                   *
 * -------------------------------------------------------------------------- */
extern void        msg_init       (void);

/* -------------------------------------------------------------------------- *
 * Destroy message heap.                                                      *
 * -------------------------------------------------------------------------- */

extern void        msg_shutdown   (void);
/* -------------------------------------------------------------------------- *
 * Find a message.                                                            *
 * -------------------------------------------------------------------------- */
extern struct msg *msg_find       (const char     *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct msg *msg_find_id    (uint32_t        id);

/* -------------------------------------------------------------------------- *
 * Register a message.                                                        *
 * -------------------------------------------------------------------------- */
extern struct msg *msg_register   (struct msg     *msg);

/* -------------------------------------------------------------------------- *
 * Unregister a message.                                                      *
 * -------------------------------------------------------------------------- */
extern void        msg_unregister (struct msg     *msg);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void        m_unregistered (struct player  *pptr,
                                   int             argc, 
                                   char          **argv);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void        m_registered   (struct player  *pptr,
                                   int             argc, 
                                   char          **argv);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void        m_ignore       (struct player  *pptr,
                                   int             argc, 
                                   char          **argv);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void        m_not_oper     (struct player  *pptr,
                                   int             argc, 
                                   char          **argv);

/* -------------------------------------------------------------------------- *
 * Dump message stack.                                                        *
 * -------------------------------------------------------------------------- */
extern void        msg_dump       (struct msg     *mptr);

#endif /* SRC_MSG_H */
