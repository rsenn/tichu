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
 * $Id: tichu.h,v 1.14 2005/02/03 17:13:37 slurp Exp $
 */

#ifndef SRC_TICHU_H
#define SRC_TICHU_H

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif /* HAVE_STDINT_H */

#include <stddef.h>

#include <libchaos/defs.h>
/*#include "dlink.h"*/

#define PLAYER_BLOCK_SIZE    64
#define LPLAYER_BLOCK_SIZE   32
#define USER_BLOCK_SIZE      32
#define CARD_BLOCK_SIZE      56
#define MSG_BLOCK_SIZE       32
#define USERMODE_BLOCK_SIZE  16
#define GAME_BLOCK_SIZE      32
#define CHANMODE_BLOCK_SIZE 128
#define BAN_BLOCK_SIZE      128
#define CLASS_BLOCK_SIZE      8 
#define OPER_BLOCK_SIZE       8
#define SERVER_BLOCK_SIZE    16
#define CNODE_BLOCK_SIZE     64
#define TURN_BLOCK_SIZE      64
#define PRICK_BLOCK_SIZE     64
#define POINTS_BLOCK_SIZE    64
#define SUPPORT_BLOCK_SIZE   32
#define STATS_BLOCK_SIZE     16
#define SERVICE_BLOCK_SIZE   16
#define COMBO_BLOCK_SIZE     16


#define TICHU_IDLEN      8
#define TICHU_CIPHERLEN  8
#define TICHU_COOKIELEN  8
#define TICHU_USERLEN    16
#define TICHU_PROTOLEN   16
#define TICHU_KEYLEN     24
#define TICHU_NICKLEN    32
#define TICHU_PASSWDLEN  32
#define TICHU_CLASSLEN   32
#define TICHU_INFOLEN    128
#define TICHU_GAMELEN    128
#define TICHU_TOPICLEN   384
#define TICHU_KICKLEN    256
#define TICHU_AWAYLEN    256
#define TICHU_PREFIXLEN  TICHU_NICKLEN + 1 + \
                         TICHU_USERLEN + 1 + \
                         TICHU_HOSTLEN

#define TICHU_LINELEN    512
#define TICHU_BUFSIZE    1024

#define TICHU_PATHLEN    64

#define TICHU_HOSTLEN    64
#define TICHU_HOSTIPLEN  16

#define TICHU_MODEBUFLEN 64
#define TICHU_PARABUFLEN 256

#define TICHU_MODESPERLINE 4
#define TICHU_MAXGAMES     16
#define TICHU_MAXTARGETS   4
#define TICHU_MAXBANS      64

#define TICHU_STACKSIZE       262144
#define TICHU_LINUX_STACKTOP  0xc0000000

extern int          tichu_log;
extern int          tichu_log_in;
extern int          tichu_log_out;
extern const char  *tichu_package;
extern const char  *tichu_version;
extern const char  *tichu_release;
extern struct game *tichu_public;
extern struct db   *tichu_db;
typedef enum { 
   false = 0,
   true = 1 
} bool;

struct support {
  struct node node;
  char        name[64];
  char        value[128];
};

struct client;

/* -------------------------------------------------------------------------- *
 * Assemble uptime string                                                     *
 * -------------------------------------------------------------------------- */
extern const char     *tichu_uptime       (void);
  
/* -------------------------------------------------------------------------- *
 * Garbage collect.                                                           *
 * -------------------------------------------------------------------------- */
extern void            tichu_collect      (void);

/* -------------------------------------------------------------------------- *
 * Restart the daemon.                                                        *
 * -------------------------------------------------------------------------- */
extern int             tichu_restart      (void);

/* -------------------------------------------------------------------------- *
 * Clean things up.                                                           *
 * -------------------------------------------------------------------------- */
extern void            tichu_shutdown     (void);

/* -------------------------------------------------------------------------- *
 * Add a new support value                                                    *
 * -------------------------------------------------------------------------- */
extern struct support *tichu_support_new  (void);

/* -------------------------------------------------------------------------- *
 * Find a support entry by name                                               *
 * -------------------------------------------------------------------------- */
extern struct support *tichu_support_find (const char *name);

/* -------------------------------------------------------------------------- *
 * Unset a support value                                                      *
 * -------------------------------------------------------------------------- */
extern void            tichu_support_unset(const char *name);

/* -------------------------------------------------------------------------- *
 * Set a support value                                                        *
 * -------------------------------------------------------------------------- */
extern struct support *tichu_support_set  (const char *name, 
                                          const char *value, ...);

/* -------------------------------------------------------------------------- *
 * Show support numeric to a client                                           *
 * -------------------------------------------------------------------------- */
extern void            tichu_support_show (struct client *cptr);
  
#endif /* SRC_TICHU_H */
