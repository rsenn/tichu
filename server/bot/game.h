#ifndef BOT_GAME_H
#define BOT_GAME_H

#include "bot.h"

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct game {
  struct node      node;        /* node für die globale game list */
  uint32_t         serial;
  uint32_t         refcount;
  uint32_t         hash;        /* game name hash */
  int              player_count;
  struct list      players;     /* liste der Spieler */
  struct list      cards;   
  char             name      [BOT_GAMELEN + 1];
  struct player   *founder;
  int              type;
  int              state;
  int              team;
  int              accepted;
  int              gschupft;
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define GAME_HASH_SIZE 16

#define GAME_PRIVMSG 0
#define GAME_NOTICE 0

#define GAME_TYPE_FUN      0
#define GAME_TYPE_LIGA     1
#define GAME_TYPE_TOURNIER 2

#define GAME_STATE_CHAT         0
#define GAME_STATE_DISTRIBUTING 1
#define GAME_STATE_SCHUPFEN     2
#define GAME_STATE_GAME         3

#define GAME_TICHU_TIME (10 * 1000LLU)

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define game_is_member(game, player)  (dlink_find(&game->players, player) != NULL)

#define game_is_banned(game, player)  (dlink_find(&game->banned, player) != NULL)

#define game_has_key(game)            (game->key[0] != '\0')

#define game_is_valid(name)           ((name) && \
                                        (*(name) == '#'))

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int          game_log;
extern struct sheap game_heap;
extern struct sheap game_invite_heap;
extern struct list  game_list;
extern uint32_t     game_serial;

/* -------------------------------------------------------------------------- *
 * Initialize game heap and add garbage collect timer.                        * 
 * -------------------------------------------------------------------------- */
extern void         game_init           (void);

/* -------------------------------------------------------------------------- *
 * Destroy game heap and cancel timer.                                        *
 * -------------------------------------------------------------------------- */
extern void         game_shutdown       (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         game_clear          (void);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         game_update         (struct game   *game,
                                         int            players, 
                                         int            type);
  
/* -------------------------------------------------------------------------- *
 * Join a game.                                                               *
 * -------------------------------------------------------------------------- */
extern void         game_join           (struct game   *game);
                    
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         game_check_team     (struct game   *game);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         game_check_accept   (struct game   *game);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int          game_check_play     (struct game   *game);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         game_check_schupfe  (struct game *game);
  
/* -------------------------------------------------------------------------- *
 * Create a game.                                                             *
 * -------------------------------------------------------------------------- */
extern struct game *game_new            (const char    *name,
                                         int            players,
                                         int            type);

/* -------------------------------------------------------------------------- *
 * Delete a game.                                                             *
 * -------------------------------------------------------------------------- */
extern void         game_delete         (struct game   *chptr);

/* -------------------------------------------------------------------------- *
 * Loose all references of a game block.                                      *
 * -------------------------------------------------------------------------- */
extern void         game_release        (struct game   *chptr);

/* -------------------------------------------------------------------------- *
 * Find a game by name.                                                       *
 * -------------------------------------------------------------------------- */
extern struct game *game_find_name      (const char    *name);

/* -------------------------------------------------------------------------- *
 * Find a game by id.                                                         *
 * -------------------------------------------------------------------------- */
extern struct game *game_find_id        (uint32_t       id);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct game *game_find_warn      (struct client *player, 
                                         const char    *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         game_set_name       (struct game   *chptr,
                                         const char    *name);

/* -------------------------------------------------------------------------- *
 * Get a reference to an game block                                           *
 * -------------------------------------------------------------------------- */
extern struct game    *game_pop                  (struct game   *chptr);  

/* -------------------------------------------------------------------------- *
 * Push back a reference to a game block                                      *
 * -------------------------------------------------------------------------- */
extern struct game    *game_push                 (struct game  **chptrptr);  

/* -------------------------------------------------------------------------- *
 * Dump games and game heap.                                                  *
 * -------------------------------------------------------------------------- */
extern void            game_dump                 (struct game   *game);

#endif /* BOT_GAME_H */
    
