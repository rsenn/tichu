/* tichu - pi-networks irc server
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
 * $Id: bot.h,v 1.5 2005/01/10 16:29:24 smoli Exp $
 */

#ifndef BOT_BOT_H
#define BOT_BOT_H

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif /* HAVE_STDINT_H */

#include <stddef.h>

#include <libchaos/defs.h>
/*#include "dlink.h"*/

#define GAME_BLOCK_SIZE      32
#define PLAYER_BLOCK_SIZE     4
#define CARD_BLOCK_SIZE      56

#define BOT_IDLEN      8
#define BOT_CIPHERLEN  8
#define BOT_COOKIELEN  8
#define BOT_USERLEN    16
#define BOT_PROTOLEN   16
#define BOT_KEYLEN     24
#define BOT_NICKLEN    32
#define BOT_PASSWDLEN  32
#define BOT_CLASSLEN   32
#define BOT_INFOLEN    128
#define BOT_GAMELEN    128
#define BOT_TOPICLEN   384
#define BOT_KICKLEN    256
#define BOT_AWAYLEN    256
#define BOT_PREFIXLEN  BOT_NICKLEN + 1 + \
                         BOT_USERLEN + 1 + \
                         BOT_BOT_HOSTLEN

#define BOT_LINELEN    512
#define BOT_BUFSIZE    1024

#define BOT_PATHLEN    64

#define BOT_BOT_HOSTLEN    64
#define BOT_BOT_HOSTIPLEN  16

#define BOT_MODEBUFLEN 64
#define BOT_PARABUFLEN 256

#define BOT_MODESPERLINE 4
#define BOT_MAXGAMES     16
#define BOT_MAXTARGETS   4
#define BOT_MAXBANS      64

#define BOT_STACKSIZE       262144
#define BOT_LINUX_STACKTOP  0xc0000000

extern const char      *bot_package;
extern const char      *bot_version;
extern const char      *bot_release;
extern uint64_t         bot_start;
extern struct dlog     *bot_drain;
extern struct sheap     BOT_BOT_Heap;
extern struct ini      *bot_ini;
extern struct child    *bot_child;
extern struct timer    *bot_game_timer;
extern int              bot_checkinterval;
extern int              bot_log;
extern int              bot_log_in;
extern int              bot_log_out;
extern int              bot_argc;
extern char           **bot_argv;
extern char           **bot_envp;
extern struct game     *bot_game;
extern struct player   *bot_player;
extern char             bot_path[PATH_MAX];
extern struct protocol *bot_protocol;
extern int              bot_connected;
extern struct connect  *bot_connection;
extern int              bot_fd;
extern char             bot_user[33];

typedef enum 
{
   false = 0,
   true = 1 
} bool;

struct support 
{
  struct node node;
  char        name[64];
  char        value[128];
};

struct client;

/* -------------------------------------------------------------------------- *
 * Assemble uptime string                                                     *
 * -------------------------------------------------------------------------- */
extern const char     *bot_uptime       (void);
  
/* -------------------------------------------------------------------------- *
 * Garbage collect.                                                           *
 * -------------------------------------------------------------------------- */
extern void            bot_collect      (void);


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern char           *bot_getname      (void);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int             bot_checkgames   (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            bot_recv         (int fd);
  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            bot_vsend        (const char *format, 
                                         va_list     args);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            bot_send         (const char *format, 
                                         ...);
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            bot_login        (void);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            bot_loadconf     (void);
  
/* -------------------------------------------------------------------------- *
 * Restart the daemon.                                                        *
 * -------------------------------------------------------------------------- */
extern int             bot_restart      (void);

/* -------------------------------------------------------------------------- *
 * Clean things up.                                                           *
 * -------------------------------------------------------------------------- */
extern void            bot_shutdown     (void);

/* -------------------------------------------------------------------------- *
 * Add a new support value                                                    *
 * -------------------------------------------------------------------------- */
extern struct support *bot_support_new  (void);

/* -------------------------------------------------------------------------- *
 * Find a support entry by name                                               *
 * -------------------------------------------------------------------------- */
extern struct support *bot_support_find (const char *name);

/* -------------------------------------------------------------------------- *
 * Unset a support value                                                      *
 * -------------------------------------------------------------------------- */
extern void            bot_support_unset(const char *name);

/* -------------------------------------------------------------------------- *
 * Set a support value                                                        *
 * -------------------------------------------------------------------------- */
extern struct support *bot_support_set  (const char *name, 
                                         const char *value, ...);

/* -------------------------------------------------------------------------- *
 * Show support numeric to a client                                           *
 * -------------------------------------------------------------------------- */
extern void            bot_support_show (struct client *cptr);
  
#endif /* BOT_BOT_H */
