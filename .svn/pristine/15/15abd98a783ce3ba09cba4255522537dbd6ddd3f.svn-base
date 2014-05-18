/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
/* layer 1 stuff*/
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/log.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * Core headers                                                               *
 * -------------------------------------------------------------------------- */
/* layer 2 stuff */
#include <tichu/msg.h>
#include <tichu/chars.h>
#include <tichu/card.h>
#include <tichu/player.h>
#include <tichu/game.h>
#include <tichu/cnode.h>

/* -------------------------------------------------------------------------- *
 * Prototypes                                                                 *
 * -------------------------------------------------------------------------- */
static void m_tichu(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_tichu_help[] = {
  "TICHU <gross>",
  "",
  "Tichu ansagen, grosses tichu mit 'tichu 1'",
  NULL
};

static struct msg m_tichu_msg = {
  "TICHU", 1, 1, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_tichu,  m_tichu },
  m_tichu_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_tichu_load(void)
{
  if(msg_register(&m_tichu_msg) == NULL)
    return -1;

  return 0;
}

void m_tichu_unload(void)
{
  msg_unregister(&m_tichu_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "tichu"                                                          *
 * argv[1] -  BIG ?                                                           *
 * -------------------------------------------------------------------------- */
static void m_tichu(struct player *player, int argc, char **argv)
{  
  int tichu;
  
  tichu = (uint32_t)strtoul(argv[1], NULL, 10);
  
  
  if(player->game->state != GAME_STATE_DISTRIBUTING)
  {    
    player_send(player, "%s FAIL :Keine 'Tichu'-Ansage möglich...", argv[0]);
    return;
  }
  
  if(player->game->tichu)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Es hat schon jemand Tichu angesagt");
    return;
  }

  if(tichu < 0 || tichu > 1)
  {
    player_send(player, "%s FAIL :Keine gültige angabe, nur 0 oder 1 möglich..", argv[0]);
    return;
  }
  
  player->game->tichu_big = tichu;
  player->game->tichu = player;  

  game_send(player->game, ":%s %s OK %i :Spieler %s hat ein %s angesagt", 
            player->name, argv[0], tichu, player->name, (tichu?"Grosses Tichu":"Tichu"));
  
  game_start(player->game);
  
  return;
}
