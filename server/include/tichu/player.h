#ifndef SRC_PLAYER_H
#define SRC_PLAYER_H

#include <libchaos/image.h>

#include <tichu/tichu.h>

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */
#define PLAYER_UNKNOWN 0x00
#define PLAYER_USER    0x01
#define PLAYER_MEMBER  0x02
#define PLAYER_OPER    0x03

#define PLAYER_PLUGDATA_SAUTH  0
#define PLAYER_PLUGDATA_COOKIE 1
#define PLAYER_PLUGDATA_MFLOOD 2
#define PLAYER_PLUGDATA_CFLOOD 3

/* -------------------------------------------------------------------------- *
 * Types                                                                      *
 * -------------------------------------------------------------------------- */
struct player {
  struct node     node;          /* node for lclient_list */
  struct node     tnode;         /* node for typed lists */
  struct node     gnode;         /* node for game->players */
  struct node     bnode;         /* node for game->banned */
  uint32_t        serial;
  uint32_t        refcount;      /* how many times this block is referenced */
  uint32_t        type;          /* lclient type */
  uint32_t        hash;
  struct game    *game;
  struct class   *class;
  struct listen  *listen;        /* listener reference */
  int             fds[2];        /* file descriptors for data connection */
  struct in_addr  addr_local;    /* local address */
  struct in_addr  addr_remote;   /* remote address */
  uint16_t        port_local;    /* local port */
  uint16_t        port_remote;   /* remote port */
  struct timer   *ptimer;        /* ping timer */
  
  int             db_id;         /* id des Benutzer datensatz's in der db */
  char            *password;     /* passwort des Benutzers */
  
  struct list     cards;         /* liste der Karten welche der Spieler besitzt */
  struct list     stack;
  
  /* stuff für's game */
  int             accepted;      /* 1 -> wenn der spieler akzeptiert hat*/
  struct list     schupfen;      /* die karten welche der spieler schupfen möchte */
  int             finished;      /* 1 -> wenn der spieler alle karten gespielt hat */
  int             team;          /* team '1' oder team '2' */

  struct color    color;
  
  /* statistics */
  uint32_t        recvb;
  uint32_t        sendb;
  uint32_t        recvk;
  uint32_t        sendk;
  uint32_t        recvm;
  uint32_t        sendm;
  
  /* capabilities bit-field */
  uint64_t        caps;
  uint32_t        ts;
  int64_t         lag;
  uint64_t        ping;
  int             shut;          /* don't read (for flood throttling etc.) */
  
  /* string fields */
  char            name   [TICHU_NICKLEN   + 1];
  char            host   [TICHU_HOSTLEN   + 1];
  char            hostip [TICHU_HOSTIPLEN + 1];
  char            pass   [TICHU_PASSWDLEN + 1];
  char            info   [TICHU_INFOLEN   + 1];
  
  void           *plugdata[32];
};

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
extern struct sheap    player_heap;     /* heap for player_t */
extern int             player_log;      /* player log source */
extern uint32_t        player_serial;
extern struct list     player_list;     /* list with all of them */
extern struct list     player_lists[4]; /* unreg, clients, servers, opers */
extern unsigned long   player_recvb;
extern unsigned long   player_sendb;
extern unsigned long   player_recvm;
extern unsigned long   player_sendm;

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define player_is_unknown(x) (((struct player *)(x))->type == PLAYER_UNKNOWN)
#define player_is_user(x)    (((struct player *)(x))->type == PLAYER_USER)
#define player_is_member(x)  (((struct player *)(x))->type == PLAYER_MEMBER)
#define player_is_oper(x)    (((struct player *)(x))->type == PLAYER_OPER)

/* -------------------------------------------------------------------------- *
 * Initialize player module                                                  *
 * -------------------------------------------------------------------------- */
extern void            player_init         (void);

/* -------------------------------------------------------------------------- *
 * Shutdown player module                                                    *
 * -------------------------------------------------------------------------- */
extern void            player_shutdown     (void);

/* -------------------------------------------------------------------------- *
 * Garbage collect player data                                               *
 * -------------------------------------------------------------------------- */
extern void            player_collect      (void);

/* -------------------------------------------------------------------------- *
 * A 32-bit PRNG for the ping cookies                                         *
 * -------------------------------------------------------------------------- */
extern uint32_t        player_random       (void);  

/* -------------------------------------------------------------------------- *
 * Create a new player block                                                 *
 * -------------------------------------------------------------------------- */
extern struct player  *player_new           (int              fd);

/* -------------------------------------------------------------------------- *
 * Delete a  player block                                                    *
 * -------------------------------------------------------------------------- */
extern void            player_delete       (struct player  *player);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            player_clear        (struct player  *player);
  
/* -------------------------------------------------------------------------- *
 * Loose all references of an player block                                    *
 * -------------------------------------------------------------------------- */
extern void            player_release      (struct player  *player);

/* -------------------------------------------------------------------------- *
 * Get a reference to an player block                                        *
 * -------------------------------------------------------------------------- */
extern struct player  *player_pop          (struct player   *player);

/* -------------------------------------------------------------------------- *
 * Push back a reference to an player block                                  *
 * -------------------------------------------------------------------------- */
extern struct player  *player_push         (struct player  **playerptr);

/* -------------------------------------------------------------------------- *
 * Set the type of an player and move it to the appropriate list             *
 * -------------------------------------------------------------------------- */
extern void            player_set_type    (struct player   *player, 
                                           uint32_t         type);

/* -------------------------------------------------------------------------- *
 * Set the name of an player block                                           *
 * -------------------------------------------------------------------------- */
extern void            player_set_name    (struct player   *player, 
                                           const char      *name);

/* -------------------------------------------------------------------------- *
 * Accept a local client                                                      *
 *                                                                            *
 * <fd>                     - filedescriptor of the new connection            *
 *                            (may be invalid?)                               *
 * <listen>                 - the corresponding listen{} block                *
 * -------------------------------------------------------------------------- */
extern void            player_accept      (int              fd,
                                           struct listen   *listen);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
/*extern void            player_connect   (int              fd, 
                                           struct connect  *connect);*/

/* -------------------------------------------------------------------------- *
 * Read data from a local connection and process it                           *
 * -------------------------------------------------------------------------- */
extern void            player_recv        (int              fd,
                                           struct player   *player);

/* -------------------------------------------------------------------------- *
 * Read a line from queue and process it                                      *
 * -------------------------------------------------------------------------- */
extern void            player_process     (int              fd, 
                                           struct player   *player);  

/* -------------------------------------------------------------------------- *
 * Parse the prefix and the command                                           *
 * -------------------------------------------------------------------------- */
extern void            player_parse       (struct player   *player, 
                                           char            *s,
                                           size_t           n);

/* -------------------------------------------------------------------------- *
 * Decide whether a message is numeric or not and call the appropriate        *
 * message handler.                                                           *
 * -------------------------------------------------------------------------- */
extern void            player_message      (struct player  *client,
                                            char          **argv, 
                                            char           *arg,
                                            size_t          n);

/* -------------------------------------------------------------------------- *
 * Process a numeric message                                                  *
 * -------------------------------------------------------------------------- */
extern void            player_numeric      (struct player  *player,
                                            char          **argv, 
                                            char           *arg);

/* -------------------------------------------------------------------------- *
 * Process a command                                                          *
 * -------------------------------------------------------------------------- */
extern void            player_command      (struct player  *player,
                                            char          **argv, 
                                            char           *arg,
                                            size_t          n);

/* -------------------------------------------------------------------------- *
 * Parse the prefix and find the appropriate client                           *
 * -------------------------------------------------------------------------- */
extern struct player  *player_prefix       (struct player  *player,
                                            const char     *pfx);
  

/* -------------------------------------------------------------------------- *
 * Exit a local client and leave him an error message if he has registered.   *
 * -------------------------------------------------------------------------- */
extern void            player_vexit        (struct player  *player,
                                            char           *format, 
                                            va_list         args);

extern int             player_exit         (struct player  *player,
                                            char           *format, 
                                            ...);

/* -------------------------------------------------------------------------- *
 * Update client message/byte counters                                        *
 * -------------------------------------------------------------------------- */
extern void            player_update_recvb (struct player  *player,
                                            size_t          n);

extern void            player_update_sendb (struct player  *player, 
                                             size_t         n);

/* -------------------------------------------------------------------------- *
 * Send a line to a local client                                              *
 * -------------------------------------------------------------------------- */
extern void            player_vsend        (struct player  *player,
                                            const char     *format, 
                                            va_list         args); 

extern void            player_send         (struct player  *player,
                                            const char     *format, 
                                            ...);  

/* -------------------------------------------------------------------------- *
 * Send a line to a client list but one                                       *
 * -------------------------------------------------------------------------- */
extern void            player_vsend_list   (struct player  *one,
                                            struct list    *list,
                                            const char     *format, 
                                            va_list         args); 

extern void            player_send_list    (struct player  *one,
                                            struct list    *list,
                                            const char     *format, 
                                            ...);

/* -------------------------------------------------------------------------- *
 * Check for valid USER/NICK and start handshake                              *
 * -------------------------------------------------------------------------- */
extern void            player_handshake    (struct player  *player);

/* -------------------------------------------------------------------------- *
 * Register a local client to the global client pool                          *
 * -------------------------------------------------------------------------- */
extern int             player_register     (struct player  *player);

/* -------------------------------------------------------------------------- *
 * Check if we got a PONG, if not exit the client otherwise send another PING *
 * -------------------------------------------------------------------------- */
extern int             player_ping         (struct player  *player);
  
/* -------------------------------------------------------------------------- *
 * USER/NICK has been sent but not yet validated                              *
 * -------------------------------------------------------------------------- */
extern void            player_login        (struct player  *player);
  
/* -------------------------------------------------------------------------- *
 * Send welcome messages to the client                                        *
 * -------------------------------------------------------------------------- */
extern void            player_welcome      (struct player  *player);

/* -------------------------------------------------------------------------- *
 * Find a player by its id                                                   *
 * -------------------------------------------------------------------------- */
extern struct player *player_find_id       (int              id);

/* -------------------------------------------------------------------------- *
 * Find a player by its name                                                 *
 * -------------------------------------------------------------------------- */
extern struct player *player_find_name     (const char      *name);

/* -------------------------------------------------------------------------- *
 * Find a player by its name in the database                                  *
 * -------------------------------------------------------------------------- */
extern struct player *player_find_sql      (struct player   *player,
                                            const char      *name);

/* -------------------------------------------------------------------------- *
 * Dump players and player heap.                                            *
 * -------------------------------------------------------------------------- */
extern void            player_dump         (struct player   *player);
  
#endif /* SRC_player_H */
