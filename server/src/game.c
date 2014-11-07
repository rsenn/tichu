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
#include <tichu/tichu.h>
#include <tichu/player.h>
#include <tichu/combo.h>
#include <tichu/game.h>
#include <tichu/structs.h>
#include <tichu/card.h>
#include <tichu/cnode.h>
#include <tichu/msg.h>

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
static int game_tichu_timer(struct game *game) 
{
  game->tichu_timer = NULL;
  
  /* Spieler informieren */
  game_send(game, "TICHU OK :Niemand hat ein Grosses Tichu angesagt..");
  
  /* zeit zum ansagen ist vorüber */
  game_start(game);
  
  return 1; /* beendet timer */
} 

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static int game_round_timer(struct game *game)
{
  game->round_timer = NULL;
  
  game_round_new(game);
  
  return 1;
}


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define ROR(v, n) ((v >> ((n) & 0x1f)) | (v << (32 - ((n) & 0x1f))))
#define ROL(v, n) ((v >> ((n) & 0x1f)) | (v << (32 - ((n) & 0x1f))))
static uint32_t game_rand(void)
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

/* -------------------------------------------------------------------------- *
 * Create a game.                                                          *
 * -------------------------------------------------------------------------- */
struct game *game_new(const char *name)
{
  struct game *game;
  
  /* allocate, zero and link game struct */
  game = mem_static_alloc(&game_heap);
  
  memset(game, 0, sizeof(struct game));
  
  /* initialize the struct */
  strlcpy(game->name, name, sizeof(game->name));
  
  game->ts         = timer_systime;
  game->hash       = strihash(game->name);
  game->refcount   = 1;
  game->serial     = game_serial++;
  game->modes      = 0LLU;
  game->state      = GAME_STATE_CHAT;
  game->abandoned  = 0;
  game->finished   = 0;
  game->max_points = 170;
  
  dlink_list_zero(&game->players);
  dlink_list_zero(&game->banned);
  dlink_list_zero(&game->cards);
  dlink_list_zero(&game->points);
  
  dlink_add_tail(&game_list, &game->node, game);
  
  debug(game_log, "Created game block: %s", game->name);
  
  return game;
}     
     
/* -------------------------------------------------------------------------- *
 * Delete a game.                                                          *
 * -------------------------------------------------------------------------- */
void game_delete(struct game *game)
{
  if(game->tichu_timer)
    timer_push(&game->tichu_timer);
  
  debug(game_log, "Destroying game block: %s", game->name);
  
  dlink_delete(&game_list, &game->node);
  
  game_clear(game);
  game_release(game);
  
  mem_static_free(&game_heap, game);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_clear(struct game *game) 
{
  struct node *node;
  struct node *next;
  struct cnode *cnode;
  struct player *player;
  struct node   *node1, *node2, *node3, *node4;
  struct prick  *prick;
  struct turn   *turn;
  
  dlink_foreach_data(&game->players, node, player)
    player_clear(player);
  
  dlink_foreach_safe_data(&game->cards, next, node, cnode)
    cnode_delete(cnode);
  
  /* in alle turns jedes pricks gehen, und die karten in eine temp-liste linken */
  dlink_foreach_safe_data(&game->pricks, node1, node2, prick)
  {    
    dlink_foreach_safe_data(&prick->turns, node3, node4, turn)      
      /* den turn deleten */
      turn_delete(turn);
    
    /* den prick deleten */
    prick_delete(prick);
  }
   
}

/* -------------------------------------------------------------------------- *
 * Loose all references of a game block.                                   *
 * -------------------------------------------------------------------------- */
void game_release(struct game *game)
{
  dlink_list_zero(&game->players);
  dlink_list_zero(&game->banned);
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
 * -------------------------------------------------------------------------- */
void game_vsend(struct game *game, const char *format, va_list args)
{
  struct player   *player;
  struct node     *node;
  struct node     *next;
  struct fqueue    multi;
  size_t           n;
  char             buf[TICHU_LINELEN + 1];

  n = vsnprintf(buf, sizeof(buf) - 2, format, args);  
  
  buf[n++] = '\r';
  buf[n++] = '\n';
  
  io_multi_start(&multi);
  
  io_multi_write(&multi, buf, n);
  
  dlink_foreach_safe_data(&game->players, node, next, player)
  {
    if(player->fds[1] < 0 || player->fds[1] >= MAX_FDS)
      continue;
    
    io_multi_link(&multi, player->fds[1]);
    player_update_sendb(player, n);
    buf[n - 2] = '\0';
//    log(tichu_log_out, L_status, "To %s: %s", player->name, buf);    
  }
        
  io_multi_end(&multi);
}

void game_send(struct game *game, const char  *format, ...)
{
  va_list args;
  
  va_start(args, format);
  game_vsend(game, format, args);
  va_end(args);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int game_join(struct game *game, struct player *player, const char *key)
{

  if(game_is_member(game, player))
    return -1;
  
  if(player->game)
    game_leave(player->game, player);
    
  /* game->players.head        - Kopf-Knoten der playerliste
   * game->players.tail        - Schwanz-Knoten der playerliste
   * game->players.size        - Grösse der playerliste
   * 
   * player->gnode.data        - Referenz auf Daten des Knotens
   * player->gnode.prev        - Referenz auf vorherigen Knoten
   * player->gnode.next        - Referenz auf nächsten Knoten
   * 
   * 
   * dlink_add_head - Verlinkt den Knoten player->gnode auf die Liste
   *                  game->players und verknüpft den Knoten (player->gnode.data) mit
   *                  dem player 
   * 
   */
  
  dlink_add_head(&game->players, &player->gnode, player);
  player->game = game;

  if(game->founder)
  {    
    game_send(game, ":%s JOIN %s :%s hat das Spiel von %s betreten.",
              player->name, game->name, player->name, game->founder->name);
  }
  else
  {
    /*public channel */
    game_send(game, ":%s JOIN %s :%s hat den Public Channel betreten.",
              player->name, game->name, player->name);
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int game_leave(struct game *game, struct player *player)
{
  if(game_is_member(game, player))
  {
    if(player->game->founder == player)
    {
      /* wenn der founder den channel verlässt, wrid channel geschlossen */
      struct player *pptr;
      struct node   *node;
      struct node   *nptr;
      
      /* Clients informieren, dass der channel geschlossen wurde */
      dlink_foreach_data(&player_list, node, pptr)
        player_send(pptr, ":%s LEAVE %s :Das Spiel von %s wurde geschlossen", 
                    player->name, game->name, player->name);      
      

      /* channel verlassen */
      dlink_foreach_safe_data(&game->players, node, nptr, pptr)
      {
        game_remove_player(game, pptr);
        game_join(tichu_public, pptr, "");
      }
      
      game_delete(game);
      
      return 0;
    }
    else
    {
      game_remove_player(game, player);      
      
      return 0;
    }
  }
  
  return -1;
}            

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_remove_player(struct game *game, struct player *player)
{
  if(game->founder)
  {    
    game_send(game, ":%s LEAVE %s :%s hat das Spiel von %s verlassen.",
              player->name, game->name, player->name, game->founder->name);
    game->founder = NULL;
  }  
  else
    /*public channel */
    game_send(game, ":%s LEAVE %s :%s hat den Public Channel verlassen.",
              player->name, game->name, player->name);
  
  dlink_delete(&game->players, &player->gnode);
  
  dlink_list_zero(&player->cards);
  
  player->game     = NULL;
  player->accepted = 0;
  player->finished = 0;
  player->team     = 0;              
}    

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_show(struct player *player)
{
  /*  struct game *game;
  
   player_send(player, numeric_format(RPL_LISTSTART),
              player_me->name, player->name);
  
  dlink_foreach(&game_list, game)
  {
    if(hooks_call(game_show, HOOK_DEFAULT, player, game))
      continue;
    
    player_send(player, numeric_format(RPL_LIST),
                player_me->name, player->name,
                game->name, game->chanusers.size,
                game->topic);
  }

  player_send(player, numeric_format(RPL_LISTEND),
              player_me->name, player->name);*/
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
    dump(game_log, "               ts: %lu", game->ts);
    dump(game_log, "          players: %u nodes", game->players.size);
    dump(game_log, "           banned: %u nodes", game->banned.size);
    dump(game_log, "[========== end of game dump ============]");    
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_distribute(struct game *game)
{
  struct cnode  *cnode;
  struct card   *card;
  struct player *player;
  
  /* status setzen */
  game->state = GAME_STATE_DISTRIBUTING;
    
  /* Reihenfolge definieren */
  game_set_order(game);
  
  /* Karten für das Spiel laden */
  dlink_foreach(&card_list, card)
  {
    cnode = cnode_new(game, card);    
    cnode_link(cnode, &game->cards);
  }

  /* lame */
  dlink_foreach_data(&game->players, cnode, player)
  {
    player_clear(player);
    
    log(game_log, L_status, "player %s has %u cards", player->name, player->cards.size);
  }
  
  /* Karte mit welcher begonnen wird markieren */
  game->x1 = cnode_find_by_name("x1", &game->cards);
    
  /* Karten verteilen und Spieler informieren*/
  game_distribute_cards(game, &game->cards, 8);
  game_send(game, "TICHU START :Ansagen nun möglich..");
  
  game->tichu_timer = timer_start(game_tichu_timer, GAME_TICHU_TIME, game);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_start(struct game *game)
{
/*  struct cnode  *cnode;
  struct card   *card;*/
  struct prick *prick;  

  /* status setzen */
    game->state = GAME_STATE_SCHUPFEN;
    
//  game->state = GAME_STATE_GAME;
      
  /* restliche Karten verteilen und Spieler informieren*/
  game_distribute_cards(game, &game->cards, 14);
  game_send(game, "CARDS OK :Alle Karten sind verteilt");
  
  /* neuen stich erstellen... */
  prick = prick_new(game);
  
  /* 
   * pointer auf den aktuellen stich in die games struct speichern 
   * Der spieler welcher, auspielen muss in game->actor
   */
  game->prick  = prick;

  if(game->x1->player)
  {    
    game->actor = game->x1->player;
//    game_send(game, "ACTOR %s", game->x1->player->name);
  } 
  else
  {
    /* sollte eigentlich nie passieren, aber bei debuging zwecken werden unter umständen 
     nicht alle karten verteilt */
//    game_next_actor(game);
    game->actor = game->players.head->data;
  }  
    
  debug(game_log, "  Spieler %s ist an der Reihe", game->actor->name);

}

/* -------------------------------------------------------------------------- *
 *  *game  - pointer auf das Spiel                                            *
 *  *cards - Liste mit allen Karten                                           *
 *   max   - Maximale Anzahl Karten welche jeder Spieler haben soll           *
 * -------------------------------------------------------------------------- */
void game_distribute_cards(struct game *game, struct list *cards, int max)
{
  struct player *player;
  struct node   *node;
  struct cnode  *cnode;
  uint32_t       n;

  /* Karten reihum an alle Spieler verteilen */
  while(cards->size && ((struct player *)game->players.head->data)->cards.size < max)
  {
    /* Reihum duch die Players gehen bis keine Karten mehr da sind */
    dlink_foreach_data(&game->players, node, player)
    {
      // huch...
      if(cards->size == 0)
        return;
      
      /* Zufällig eine Karte ziehen */
      n = game_rand() % cards->size;
      
      cnode = dlink_index(cards, n)->data;
      
      cnode_unlink(cnode, cards);
      cnode_link(cnode, &player->cards);
      cnode->player = player;
      
      /* Spieler über den erhalt der karte informieren */
      player_send(player, "CARDS %s :Karte %s erhalten", 
                  cnode->card->name, cnode->card->name);

    }
  }
}

/* -------------------------------------------------------------------------- *
 * Die 'Schupfungen' werden in player->schupfen gespeichert und, wenn alle    *
 * Spieler ihre 'Schupfungen' gespeichert haben, wird diese Funktion          *
 * ausgeführt.                                                                *
 * -------------------------------------------------------------------------- */
void game_schupfen(struct game *game)
{
  struct node   *node;
  struct node   *node2;
  struct node   *node3;
  struct cnode  *cnode;
  struct player *player;
  
  /* zuerst prüfen ob alle ihre schupfungen gemacht haben */
  dlink_foreach_data(&game->players, node, player)
  {
    if(player->schupfen.size < (game->players.size - 1))
      return;
  }
  
  debug(game_log, " Schupfungen werden ausgeführt...");
  
  dlink_foreach_data(&game->players, node, player)
  {
    dlink_foreach_safe_data(&player->schupfen, node2, node3, cnode)
    {
      /* karte aus der player->cards unlinek */
      cnode_unlink(cnode, &player->schupfen);

      /* karte in die player->cards lite des target's linken */            
      cnode_link(cnode, &cnode->player->cards); 
      
      /* mitteilung an den Spieler der die Karte erhalten hat */
      player_send(cnode->player, ":%s SCHUPFE %s :Karte %s von Spieler %s erhalten.", 
                  player->name, cnode->card->name, cnode->card->name, player->name);      
    }
  }  
  
  game_send(game, "SCHUPFE OK END :Die Schupfungen sind beendet...");
  
  /* alles war ok, dann können wir nun mit dem Spiel beginnen */
  game->state = GAME_STATE_GAME;  
  
  dlink_foreach_data(&game->players, node, player)
    player_send(player, "ACTOR %s", game->actor->name);
  
  return;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_set_order(struct game *game)
{
  struct node   *node, *nptr;
  
  if(game->players.size < 4)
    return;
  
  /* es dürfen nicht 2 personen vom selben team nebeneinander 'sitzen' */
  dlink_foreach_safe(&game->players, node, nptr)
  {
    if(nptr)
    {      
      if(((struct player *)node->data)->team == ((struct player *)nptr->data)->team)
      {
        if(nptr->next)
          dlink_swap(&game->players, nptr, nptr->next);
        else
          dlink_swap(&game->players, nptr, game->players.head);        
      }      
    }    
  }
  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_next_actor(struct game *game)
{
  struct node *nptr;
  
  /* node auf aktuellen spieler */
  nptr = &game->actor->gnode;
  
  do
  {
    if(nptr == game->players.tail)   /* wenn wir am ende sind,     */
      nptr = game->players.head;     /* gehen wir zum anfang ;D    */
    else                             /* sonst,                     */
      nptr = nptr->next;             /* einfach zum nächsten       */
   
    if(!nptr)
      break;
    
    /* alles solange wiederholen bis wir einen spieler gefunden haben, der noch nicht fertig ist */
  } while(((struct player *)nptr->data)->finished != 0);
   
  game->actor = nptr->data;
  
  /* Spieler Informieren */
  game_send(game, "ACTOR %s", game->actor->name);
  
  return;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void game_round_end(struct game *game)
{
  struct node   *node1, *node2, *node3, *node4, *node5, *node6;
  struct node   *node;
  struct prick  *prick;
  struct turn   *turn;
  struct cnode  *cnode = NULL;
  struct points *points;
  struct player *player = NULL;
  
  int points1 = 0;
  int points2 = 0;
  int count;
  
  log(player_log, L_status, " Punkte werden berrechnet....");
  
  /* Spieler informieren */
  game_send(game, "ROUND :Runde ist zu Ende...");
  
  /*
   * Punkte auswerten
   * ----------------
   * 
   * -> Wurde das Spiel regulär beendet, werden die Punkte gewöhnlih nach ihrem
   *    Wert zusammengezählt und gut geschrieben.
   * -> Haben beide Spieler des selben Teams nacheinander beendet, kann dieser vorgang
   *    ausgelassen werden. => das Team erhält 200 Punkte, das andere 0 Punkte.
   */
  
  log(player_log, L_status, "------ karten ----");
  log(player_log, L_status, "------ %i ----", game->cards.size);  
  
  /* in alle turns jedes pricks gehen, und die karten in eine temp-liste linken */  
  dlink_foreach_safe_data(&game->pricks, node1, node2, prick)
  {
    dlink_foreach_safe_data(&prick->turns, node3, node4, turn)
    {
      /* die cards von turn->cards <-> cards linken */
      dlink_foreach_safe_data(&turn->cards, node5, node6, cnode)
      {        
        cnode_unlink(cnode, &turn->cards);
        log(player_log, L_status, "------ unlinking %s ----", cnode->card->name);
        
        if(game->finished == 3)
        {            
          /* letzer spieler gibt seine stiche an den ersten */
          if(!turn->player->finished)
          {
            dlink_foreach_data(&game->players, node, player)
              if(player->finished == 1)
                break;
            
            if(player->team == 1)
              points1 += cnode->card->points;
            else if(player->team == 2)
              points2 += cnode->card->points;
          }
          else
          {                        
            /* punkte zusammen zählen */
            if(turn->player->team == 1)
              points1 += cnode->card->points;
            else if(turn->player->team == 2)
              points2 += cnode->card->points;
          }
        }
        
        log(player_log, L_status, "------ linking %s ----", cnode->card->name);          
        cnode_link(cnode, &game->cards);
      }     
      /* den turn deleten */
//      turn_delete(turn);
    }
    /* den prick deleten */
//    prick_delete(prick);
  }  
  

  if(game->finished == 2)
  {
    dlink_foreach_data(&game->players, node1, player)
    {
      if(player->finished)
        break;
    }
    if(player->team == 1)
      points1 = 200;
    else
      points2 = 200;
  }

  
  /* Hand karten der Spieler zurücknehmen, und gegebenfalls zählen */
  dlink_foreach_data(&game->players, node, player)
  {
    if(player->finished)
      continue;
    
    /* die cards von player->cards <-> cards linken */
    dlink_foreach_safe_data(&player->cards, node1, node2, cnode)
    {        
      cnode_unlink(cnode, &player->cards);  
             
      /* handkarten gehen an die gegner */
      if(game->finished == 3)
      {
        /* punkte zusammen zählen */
        if(player->team == 1)
          points2 += cnode->card->points;
        else if(player->team == 2)
          points1 += cnode->card->points;
      }
      
      cnode_link(cnode, &game->cards);
    }
    
  }
  
  log(player_log, L_status, "------ karten ----");
  log(player_log, L_status, "------ %i ----", game->cards.size);  

  
  /* die punkte noch in game->points... */
  points = points_new(game);
  
  points->team1 = points1;
  points->team2 = points2;

//  game_send(game, "DEBUG : Points team1: %i, Points team2 :%i", points1, points2);

  
  /* gesamtzahl punkte */
  points1 = 0;
  points2 = 0;
  dlink_foreach_data(&game->points, node1, points)
  {
    points1 += points->team1;
    points2 += points->team2;
  }  
  
  game_send(game, "POINTS 1 %i :Punkte des ersten Teams", points1);
  game_send(game, "POINTS 2 %i :Punkte des zweiten Teams", points2);

  /* debug */
  log(player_log, L_status, "------ PUNKTE ----");  
  count = 1;
  dlink_foreach_data(&game->points, node1, points)
  {
    log(player_log, L_status, " Runde %i", count);
    log(player_log, L_status, " -- Team 1 -> %i", points->team1);
    log(player_log, L_status, " -- Team 2 -> %i", points->team2);
    
    count++;
  }
    
  
  /* Neue Runde spielen ? */
  if(points1 < game->max_points && points2 < game->max_points)
  {
    /* ja */
    /* timer setzten zum 'grosses tichu ansagen' danach wird game_start aufgreufen */
    game->round_timer = timer_start(game_round_timer, GAME_ROUND_TIME, game);
  }
  else
  {    /* nein */
    if(points1 >= game->max_points)      
      game_end(game, 1);
    else
      game_end(game, 2);
  }
  
}

void game_round_new(struct game *game)
{
  struct cnode *x1;
  struct node  *node;
  struct player *player;
//  int count;
  
  /* Spieler daten zurücksetzten */
  game->finished = 0;
  game->abandoned = 0;
  
  game->tichu     = NULL;
  game->tichu_big = 0;

  dlink_foreach_data(&game->players, node, player)
  {
    player->finished = 0;
  }
  
  /* lame */
  dlink_foreach_data(&game->players, x1, player)
  {        
    log(game_log, L_status, "player %s has %u cards", player->name, player->cards.size);
    player_clear(player);
    log(game_log, L_status, "player %s has %u cards", player->name, player->cards.size);
  }  
  
  /* Status setzten */
  game->state = GAME_STATE_DISTRIBUTING;

  /* karte markieren */
  x1 = cnode_find_by_name("x1", &game->cards);
  
  /* Karten verteilen und Spieler informieren*/
  game_distribute_cards(game, &game->cards, 8);
  game_send(game, "TICHU START :Ansagen nun möglich..");
  
  /* timer setzten zum 'grosses tichu ansagen' danach wird game_start aufgreufen */
  game->tichu_timer = timer_start(game_tichu_timer, GAME_TICHU_TIME, game);  
}

void game_end(struct game *game, int team)
{
  int points1;
  int points2;
  struct node *node1;
  struct points *points;
  /*
   * Spiel ist zu ende, team <team> hat die maximal punktzahl erreicht
   */
  game_send(game, "END :Team%i hat gewonnen...", team);
  
  /* gesamtzahl punkte */
  points1 = 0;
  points2 = 0;
  dlink_foreach_data(&game->points, node1, points)
  {
    points1 += points->team1;
    points2 += points->team2;
  }  
  
  game_send(game, "POINTS 1 %i :Punkte des ersten Teams", points1);
  game_send(game, "POINTS 2 %i :Punkte des zweiten Teams", points2);
        
  /* 
   * wenn der founder das spiel verlässt,
   * wird das spiel geschlossen und alles schön aufgeräumt...
   */
  game_leave(game, game->founder);
  
  
}

