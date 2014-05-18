#ifndef SRC_GAME_H
#define SRC_GAME_H

#include "structs.h"

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct game {
  struct node      node;        /* node für die globale game list */
  uint32_t         serial;
  uint32_t         refcount;
  uint32_t         hash;        /* game name hash */
  time_t           ts;          /* game timestamp */
  struct list      players;     /* liste der Spieler */
  struct list      banned;      /* liste der verbannten Spieler */
  uint64_t         modes;
  char             name      [TICHU_GAMELEN + 1];
  char             key       [TICHU_KEYLEN + 1];
  struct player   *founder;
  int              type;
  int              state;
  
  struct list      cards;      /* karten, welche noch nicht verteilt wurden */
  struct player   *tichu;      /* Zeigt auf den Spieler der Tichu angesagt hat */
  int              tichu_big;  /* 1 => grosses Tichu angesagt */
  struct cnode    *x1;         /* wird beim verteilen benötigt um zu ermitteln wer anfängt */
  
  struct list      pricks;     /* liste mit allen stichen die gemacht wurden */
  struct prick    *prick;      /* pointer auf den aktuelen stich */
  struct player   *actor;      /* pointer auf den player, welcher am zug ist */
  
  struct list      points;     /* liste mit den punkten */
  int              max_points; /* maximale anzahl punkte, danach wird spiel beendet */

  struct timer    *tichu_timer; /* timer für tichu ansage */
  struct timer    *round_timer; /* kurzer stop am anfang einer neuen runde */
  
  int              abandoned;  /* anzahl gepasster züge */
  int              finished;   /* anzahl spieler, welche fertig sind */
  int              last_finish; /* schmüdige hilfe, für ausspielrecht vererbung */
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

#define GAME_TICHU_TIME (3 * 1000LLU)
#define GAME_ROUND_TIME (3 * 1000LLU)

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
 * Initialize game heap and add garbage collect timer.                     *
 * -------------------------------------------------------------------------- */
extern void         game_init           (void);

/* -------------------------------------------------------------------------- *
 * Destroy game heap and cancel timer.                                     *
 * -------------------------------------------------------------------------- */
extern void         game_shutdown       (void);

/* -------------------------------------------------------------------------- *
 * Create a game.                                                          *
 * -------------------------------------------------------------------------- */
extern struct game *game_new            (const char    *name);

/* -------------------------------------------------------------------------- *
 * Delete a game.                                                          *
 * -------------------------------------------------------------------------- */
extern void         game_delete         (struct game   *chptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         game_clear          (struct game   *chptr);

/* -------------------------------------------------------------------------- *
 * Loose all references of a game block.                                   *
 * -------------------------------------------------------------------------- */
extern void         game_release        (struct game   *chptr);

/* -------------------------------------------------------------------------- *
 * Find a game by name.                                                    *
 * -------------------------------------------------------------------------- */
extern struct game *game_find_name      (const char    *name);

/* -------------------------------------------------------------------------- *
 * Find a game by id.                                                      *
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
 * -------------------------------------------------------------------------- */
extern void         game_send           (struct game   *game,
                                         const char    *format, 
                                         ...);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void         game_delete_members          (struct game   *game,
                                                  struct list   *list, 
                                                  int            delref);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int             game_welcome              (struct game   *chptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_show_lusers          (struct game   *chptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int             game_can_join             (struct game   *chptr, 
                                                  struct player *player,
                                                  const char    *key);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_message              (struct game   *game,
                                                  struct player *player,
                                                  int            type,
                                                  char          *text);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int             game_join                 (struct game   *game,
                                                   struct player *player, 
                                                   const char    *key);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int             game_leave                (struct game *game, 
                                                  struct player *player);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_remove_player        (struct game *game,
                                                  struct player *player);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_show                 (struct player *player);
  
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

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_start                (struct game *game);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct list    *game_new_list             (struct game *game,
                                                  struct list *list);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_delete_list          (struct game *game,
                                                  struct list *list);
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_schupfen             (struct game *game);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_distribute           (struct game *game);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_distribute_cards     (struct game *game,
                                                  struct list *cards,
                                                  int max);  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_set_order            (struct game *game);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_next_actor          (struct game *game);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_round_new            (struct game *game);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_round_end            (struct game *game);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void            game_end                  (struct game *game,
                                                  int team);

#endif /* SRC_GAME_H */
    
