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
static void m_kick(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_kick_help[] = {
  "KICK <player>",
  "",
  "Kick a player out of a game",
  NULL
};

static struct msg m_kick_msg = {
  "KICK", 1, 1, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_kick,  m_kick },
  m_kick_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_kick_load(void)
{
  if(msg_register(&m_kick_msg) == NULL)
    return -1;

  return 0;
}

void m_kick_unload(void)
{
  msg_unregister(&m_kick_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "KICK"                                                           *
 * argv[1] -  player                                                          *
 * -------------------------------------------------------------------------- */
static void m_kick(struct player *player, int argc, char **argv)
{  
  struct player *pptr;
  
  if(player->game == tichu_public)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Vergiss es");
    return;
  }
  
  if(player->game->founder != player)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Kicken kann nur der Gamefounder");
    return;
  }
  
  if(!(pptr = player_find_name(argv[1])))
  {
    player_send(player, "%s FAIL :Kontte den User (%s) nicht finden.", argv[0], argv[1]);
    return;
  }
  
  if(!game_is_member(player->game, pptr))
  {
    player_send(player, "%s FAIL :Der User (%s) befindet sich nicht in diesem Game", argv[0], argv[1]);
    return;
  }
  
  /* dann schmeisen wir den mistkerl mal aus dem game */
  game_join(tichu_public, pptr, "");

  player_send(player, "%s OK :Der Spieler (%s) wurde 'entfernt'", argv[0], argv[1]);
  
}
