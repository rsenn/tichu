/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/log.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * Core headers                                                               *
 * -------------------------------------------------------------------------- */
#include <tichu/msg.h>
#include <tichu/chars.h>
#include <tichu/player.h>
#include <tichu/game.h>

/* -------------------------------------------------------------------------- *
 * Prototypes                                                                 *
 * -------------------------------------------------------------------------- */
static void m_start(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_start_help[] = {
  "START",
  "",
  "Starts the game.",
  NULL
};

static struct msg m_start_msg = {
  "START", 0, 0, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_start,  m_start },
  m_start_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_start_load(void)
{
  if(msg_register(&m_start_msg) == NULL)
    return -1;

  return 0;
}

void m_start_unload(void)
{
  msg_unregister(&m_start_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "START"                                                         *
 * -------------------------------------------------------------------------- */
static void m_start(struct player *player, int argc, char **argv)
{  
  struct player *pptr;
  struct node   *nptr;
  
  if(player->game == tichu_public)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Public Channel ist kein Spiel");
    return;
  }
      
  if(player->game->founder != player)
  {
    /* Player ist nicht Founder des games */
    player_send(player, "%s FAIL :%s", argv[0], "Nur Founder kann Spiel starten");
    return;
  }
/*  
  if(player->game->players.size != 4)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Es sind noch nicht genügend Spieler vorhanden");
    return;
  }
  */
  dlink_foreach_data(&player->game->players, nptr, pptr)
  {
    if(!pptr->accepted)
    {
      /* nicht alle Spieler haben akzeptiert */
      player_send(player, "%s FAIL :%s", argv[0], "Es haben nicht alle Spieler akzeptiert");
      return;
    }
    
    if(!pptr->team)
    {
      /*teams noch nicht erstellt */
      player_send(player, "%s FAIL :%s", argv[0], "Es sind noch nicht alle Spieler in einem Team");
      return;
    }
  }
  
  /* game starten */
  player->game->state = GAME_STATE_GAME;
  game_send(player->game, "%s OK :%s", argv[0], "Spiel wird gestartet");
  game_distribute(player->game);
  
  return;
}

