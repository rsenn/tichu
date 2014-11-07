#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * Library headers.                                                           *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/dlink.h>
#include <libchaos/timer.h>
#include <libchaos/hook.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/net.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * Core headers.                                                              *
 * -------------------------------------------------------------------------- */
#include "bot.h"
#include "game.h"
#include "card.h"
#include "player.h"

/* -------------------------------------------------------------------------- *
 * Global variables.                                                          *
 * -------------------------------------------------------------------------- */
int           game_log;
struct sheap  game_heap;
struct sheap  game_invite_heap;
struct list   game_list;
uint32_t      game_id;
uint32_t      game_serial;
uint32_t      game_seed;

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define ROR(v, n) ((v >> ((n) & 0x1f)) | (v << (32 - ((n) & 0x1f))))
#define ROL(v, n) ((v >> ((n) & 0x1f)) | (v << (32 - ((n) & 0x1f))))
/*static*/ uint32_t game_rand(void)
{
  int      it;
  int      i;
  uint32_t ns = timer_mtime;
  
  it = (ns & 0x1f) + 0x20;
  
  for(i = 0; i < it; i++)
  {
    ns = ROL(ns, game_seed);
    
    if(ns & 0x01)
      game_seed -= 0x35014541;
    else
      game_seed += 0x21524110;
      
    ns = ROR(ns, game_seed >> 27);
    
    if(game_seed & 0x02)
      ns ^= game_seed;
    else
      ns -= game_seed;
    game_seed = ROL(game_seed, ns >> 5);
    
    if(ns & 0x04)
      game_seed += ns;
    else
      game_seed ^= ns;
    
    ns = ROL(ns, game_seed >> 13);
    
    if(game_seed & 0x08)
      ns += game_seed;
    else
      game_seed -= ns;
  }

  return ns;
}
#undef ROR
#undef ROL

/* -------------------------------------------------------------------------- *
 * Initialize game heaps and add garbage collect timer.                    *
 * -------------------------------------------------------------------------- */
void game_init(void)
{
  game_log = log_source_register("game");

  game_id = 0;
  game_serial = 0;
  
  dlink_list_zero(&game_list);
  
  mem_static_create(&game_heap, sizeof(struct game),
                    GAME_BLOCK_SIZE);
  mem_static_note(&game_heap, "game heap");
  
  game_seed = timer_mtime;

  log(game_log, L_status, "Initialised [game] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy game heaps and cancel timer.                                    *
 * -------------------------------------------------------------------------- */
void game_shutdown(void)
{
  struct game *game;
  struct game *next;
  
  log(game_log, L_status, "Shutting down [game] module...");
  
  dlink_foreach_safe(&game_list, game, next)
    game_delete(game);

  mem_static_destroy(&game_heap);
  
  log_source_unregister(game_log);
}

void game_clear(void) 
{
  struct node *next;
  struct game *game;
  
  dlink_foreach_safe(&game_list, game, next)
    game_delete(game);
} 

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_update(struct game *game, int players, int type)
{
  game->player_count = players;
  game->type = type;
  game->serial = game_serial;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_join(struct game *game)
{
  bot_send("JOIN %s", game->name);
  
  bot_game = game;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_check_team(struct game *game)
{
  int team1, team2;
  struct player *player;
  
  /* entscheide mich für ein team */
  if(!game->team)
  {
    team1 = 0;
    team2 = 0;
    
    dlink_foreach(&player_list, player)
    {
      if(player->team == 1)
        team1++;
      if(player->team == 2)
        team2++;
    }
    
    if(team1 || team2)
    {
      if(team2 < team1)
        game->team = 2;
      else
        game->team = 1;
      
      bot_send("TEAM %i", game->team);
    }
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_check_accept(struct game *game)
{
  /* erst akzeptieren wenn 4 spieler im game und teams gewählt */
  if(!game->accepted && game->team)
  {
    if(player_list.size >= 4)
    {
      bot_send("ACCEPT");
      game->accepted = 1;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_check_schupfe(struct game *game)
{
  struct player *player;
  
  if(!game->gschupft)
  {
    dlink_foreach(&player_list, player)
      if(player != bot_player)
        player_schupf(bot_player, player, (player->team != game->team) ?
                      CARD_LAME : CARD_LEET);
    
    game->gschupft = 1;      
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int game_check_play(struct game *game)
{
  struct list list;
  struct card *card;
  
  dlink_list_zero(&list);
  
  log(card_log, L_status, "$$$$$$=========> have %i card(s) left", bot_player->cards.size);
  
  /* ausspielen? */
  if(game->cards.size == 0)
  {
    struct card *card = card_by_mode(&bot_player->cards, CARD_LAME);
    
    dlink_add_tail(&list, dlink_node_new(), card);
    
    player_playcards(bot_player, &list);
    
    player_sendcards();
  }
  else
  {
    int value;
    struct list tmplist;
    
    dlink_list_zero(&tmplist);
    
    card = game->cards.head->data;
    value = card->value;
    
    for(;;)
    {
      dlink_destroy(&tmplist);

      log(game_log, L_verbose, "searching a card with value higher than: %i", value);
      
      if(!(card = card_higher(&bot_player->cards, value)))
      {
        log(game_log, L_verbose, "not found");
        break;
      }
      
      value = card->value;
      
      card_by_value(&bot_player->cards, &tmplist, value, game->cards.size);
      
      log(game_log, L_verbose, "found %u cards", tmplist.size);
      
      if(tmplist.size >= game->cards.size)
      {
        player_playcards(bot_player, &tmplist);
        player_sendcards();
        break;
      }
    }
    
    if(!tmplist.size)
    {
      log(game_log, L_status, "kann nicht spielen, muss passen..");
      bot_send("ABANDON");
    }
    
    dlink_destroy(&tmplist);
  }
  
  dlink_destroy(&list);
  
  return 1;
}

/* -------------------------------------------------------------------------- *
 * Create a game.                                                             *
 * -------------------------------------------------------------------------- */
struct game *game_new(const char *name, int players, int type)
{
  struct game *game;
  
  /* allocate, zero and link game struct */
  game = mem_static_alloc(&game_heap);
  
  memset(game, 0, sizeof(struct game));
  
  /* initialize the struct */
  strlcpy(game->name, name, sizeof(game->name));
  
  game->hash     = strihash(game->name);
  game->refcount = 1;
  game->serial   = game_serial;
  game->type     = type;
  game->team     = 0;
  game->accepted = 0;
  
  dlink_list_zero(&game->players);
  dlink_list_zero(&game->cards);
    
  dlink_add_tail(&game_list, &game->node, game);
  
  debug(game_log, "Created game block: %s", game->name);
  
  return game;
}     
     
/* -------------------------------------------------------------------------- *
 * Delete a game.                                                          *
 * -------------------------------------------------------------------------- */
void game_delete(struct game *game)
{
  debug(game_log, "Destroying game block: %s", game->name);
  
  dlink_delete(&game_list, &game->node);
  
  game_release(game);
  
  mem_static_free(&game_heap, game);
}

/* -------------------------------------------------------------------------- *
 * Loose all references of a game block.                                   *
 * -------------------------------------------------------------------------- */
void game_release(struct game *game)
{
  
  dlink_list_zero(&game->players);
  dlink_list_zero(&game->cards);
} 

/* -------------------------------------------------------------------------- *
 * Find a game by name.                                                    *
 * -------------------------------------------------------------------------- */
struct game *game_find_name(const char *name)
{
  struct game *game;
  struct node    *node;
  uint32_t        hash;
  
  hash = strihash(name);

  dlink_foreach(&game_list, node)
  {
    game = node->data;
    
    if(hash == game->hash)
    {
      if(!stricmp(game->name, name))
        return game;
    }
  }
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Find a game by id.                                                         *
 * -------------------------------------------------------------------------- */
struct game *game_find_id(uint32_t id)
{
  struct game *game;
  
  dlink_foreach(&game_list, game)
  {
    if(game->serial == id)
      return game;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Get a reference to a game block                                         *
 * -------------------------------------------------------------------------- */
struct game *game_pop(struct game *game)
{
  if(game)
  {
    if(!game->refcount)
    {
      debug(game_log, "Poping deprecated game %s",
          game->name);
    }
    
    game->refcount++;
  }
  
  return game;
}

/* -------------------------------------------------------------------------- *
 * Push back a reference to a game block                                   *
 * -------------------------------------------------------------------------- */
struct game *game_push(struct game **gameptr)
{
  if(*gameptr)
  {
    if((*gameptr)->refcount == 0)
    {
      debug(game_log, "Trying to push deprecated game %s",
          (*gameptr)->name);
    }
    else
    {
      if(--(*gameptr)->refcount == 0)
        game_release(*gameptr);
      
      (*gameptr) = NULL;
    }
  }
      
  return *gameptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_dump(struct game *game)
{
  if(game == NULL)
  {
    struct node *node;
    
    dump(game_log, "[============== game summary ===============]");
    
    dlink_foreach_data(&game_list, node, game)
      dump(game_log, " #%03u: [%u] %-20s (%u players)",
            game->serial,
            game->refcount,
            game->name,
            game->players.size);
    
    dump(game_log, "[========== end of game summary ============]");
  }
  else
  {
    dump(game_log, "[============== game dump ===============]");
    dump(game_log, "           serial: %u", game->serial);
    dump(game_log, "         refcount: %u", game->refcount);
    dump(game_log, "             hash: %p", game->hash);
    dump(game_log, "             name: %s", game->name);
    dump(game_log, "          players: %u nodes", game->players.size);
    dump(game_log, "[========== end of game dump ============]");    
  }
}

