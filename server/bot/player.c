#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * System headers.                                                            *
 * -------------------------------------------------------------------------- */
#include <string.h>

/* -------------------------------------------------------------------------- *
 * Library headers.                                                           *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/timer.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
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
int           player_log;
struct sheap  player_heap;
struct sheap  player_invite_heap;
struct list   player_list;
uint32_t      player_id;
uint32_t      player_serial;
uint32_t      player_seed;

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define ROR(v, n) ((v >> ((n) & 0x1f)) | (v << (32 - ((n) & 0x1f))))
#define ROL(v, n) ((v >> ((n) & 0x1f)) | (v << (32 - ((n) & 0x1f))))
/*static*/ uint32_t player_rand(void)
{
  int      it;
  int      i;
  uint32_t ns = timer_mtime;
  
  it = (ns & 0x1f) + 0x20;
  
  for(i = 0; i < it; i++)
  {
    ns = ROL(ns, player_seed);
    
    if(ns & 0x01)
      player_seed -= 0x35014541;
    else
      player_seed += 0x21524110;
      
    ns = ROR(ns, player_seed >> 27);
    
    if(player_seed & 0x02)
      ns ^= player_seed;
    else
      ns -= player_seed;
    player_seed = ROL(player_seed, ns >> 5);
    
    if(ns & 0x04)
      player_seed += ns;
    else
      player_seed ^= ns;
    
    ns = ROL(ns, player_seed >> 13);
    
    if(player_seed & 0x08)
      ns += player_seed;
    else
      player_seed -= ns;
  }

  return ns;
}
#undef ROR
#undef ROL

/* -------------------------------------------------------------------------- *
 * Initialize game heaps and add garbage collect timer.                       *
 * -------------------------------------------------------------------------- */
void player_init(void)
{
  player_log = log_source_register("game");

  player_id = 0;
  player_serial = 0;
  
  dlink_list_zero(&player_list);
  
  mem_static_create(&player_heap, sizeof(struct player),
                    PLAYER_BLOCK_SIZE);
  mem_static_note(&player_heap, "game heap");
  
  player_seed = timer_mtime;

  log(player_log, L_status, "Initialised [player] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy game heaps and cancel timer.                                       *
 * -------------------------------------------------------------------------- */
void player_shutdown(void)
{
  struct player *player;
  struct node *next;
  
  log(player_log, L_status, "Shutting down [player] module...");

  dlink_foreach_safe(&player_list, player, next)
    player_delete(player);

  mem_static_destroy(&player_heap);
  
  log_source_unregister(player_log);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void player_clear(void) 
{
  struct player *player;
  struct node *next;
  
  dlink_foreach_safe(&player_list, player, next)
    player_delete(player);
} 

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void player_update(struct player *game, int team, int accepted)
{
  game->team = team;
  game->accepted = accepted;
  game->serial = player_serial;
}

/* -------------------------------------------------------------------------- *
 * Create a game.                                                             *
 * -------------------------------------------------------------------------- */
struct player *player_new(const char *name, int team, int accepted)
{
  struct player *player;
  
  /* allocate, zero and link game struct */
  player = mem_static_alloc(&player_heap);
  
  memset(player, 0, sizeof(struct player));
  
  /* initialize the struct */
  strlcpy(player->name, name, sizeof(player->name));
  
  player->hash     = strihash(player->name);
  player->refcount = 1;
  player->serial   = player_serial;
  player->team     = team;
  player->accepted = accepted;
  
  dlink_list_zero(&player->cards);
  
  dlink_add_tail(&player_list, &player->node, player);
  
  debug(player_log, "Created player block: %s", player->name);
  
  return player;
}     
     
/* -------------------------------------------------------------------------- *
 * Delete a player.                                                           *
 * -------------------------------------------------------------------------- */
void player_delete(struct player *player)
{
  debug(player_log, "Destroying player block: %s", player->name);
  
  dlink_delete(&player_list, &player->node);
  
  player_release(player);
  
  mem_static_free(&player_heap, player);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void player_schupf(struct player *from, struct player *to, int mode)
{
  struct card *card;

  if((card = card_by_mode(&from->cards, mode)))
  {
    dlink_find_delete(&from->cards, card);
    dlink_add_tail(&to->cards, &card->pnode, card);
    
    log(player_log, L_status, "Karte %s geschupft an %s.", 
        card->name, to->name);
    
    bot_send("SCHUPFE %s %s", to->name, card->name);
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void player_playcards(struct player *player, struct list *cards)
{
  struct node *node;
  struct card *card;
  
  dlink_list_zero(&bot_game->cards);
  
  dlink_foreach_data(cards, node, card)
  {
    dlink_find_delete(&player->cards, card);
    
    dlink_add_head(&bot_game->cards, &card->gnode, card);
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void player_sendcards(void)
{
  struct node *node;
  struct card *card;
  char cardbuf[1024];
  
  cardbuf[0] = '\0';
  
  dlink_foreach_data(&bot_game->cards, node, card)
  {
    strcat(cardbuf, " ");
    strcat(cardbuf, card->name);
  }
  
  if(cardbuf[0])
    bot_send("PLAYCARDS%s", cardbuf);
  else
    bot_send("ABANDON");
}

/* -------------------------------------------------------------------------- *
 * Loose all references of a player block.                                    *
 * -------------------------------------------------------------------------- */
void player_release(struct player *player)
{
} 

/* -------------------------------------------------------------------------- *
 * Find a player by name.                                                     *
 * -------------------------------------------------------------------------- */
struct player *player_find_name(const char *name)
{
  struct player *player;
  struct node    *node;
  uint32_t        hash;
  
  hash = strihash(name);

  dlink_foreach(&player_list, node)
  {
    player = node->data;
    
    if(hash == player->hash)
    {
      if(!stricmp(player->name, name))
        return player;
    }
  }
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Find a player by id.                                                       *
 * -------------------------------------------------------------------------- */
struct player *player_find_id(uint32_t id)
{
  struct player *player;
  
  dlink_foreach(&player_list, player)
  {
    if(player->serial == id)
      return player;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Get a reference to a player block                                          *
 * -------------------------------------------------------------------------- */
struct player *player_pop(struct player *player)
{
  if(player)
  {
    if(!player->refcount)
    {
      debug(player_log, "Poping deprecated player %s",
          player->name);
    }
    
    player->refcount++;
  }
  
  return player;
}

/* -------------------------------------------------------------------------- *
 * Push back a reference to a player block                                   *
 * -------------------------------------------------------------------------- */
struct player *player_push(struct player **playerptr)
{
  if(*playerptr)
  {
    if((*playerptr)->refcount == 0)
    {
      debug(player_log, "Trying to push deprecated player %s",
          (*playerptr)->name);
    }
    else
    {
      if(--(*playerptr)->refcount == 0)
        player_release(*playerptr);
      
      (*playerptr) = NULL;
    }
  }
      
  return *playerptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void player_dump(struct player *player)
{
  if(player == NULL)
  {
    struct node *node;
    
    dump(player_log, "[============== player summary ===============]");
    
    dlink_foreach_data(&player_list, node, player)
      dump(player_log, " #%03u: [%u] %-20s",
           player->serial,
           player->refcount,
           player->name);
    
    dump(player_log, "[========== end of player summary ============]");
  }
  else
  {
    dump(player_log, "[============== player dump ===============]");
    dump(player_log, "           serial: %u", player->serial);
    dump(player_log, "         refcount: %u", player->refcount);
    dump(player_log, "             hash: %p", player->hash);
    dump(player_log, "             name: %s", player->name);
    dump(player_log, "[========== end of player dump ============]");    
  }
}

