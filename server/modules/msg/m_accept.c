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
static void m_accept(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_accept_help[] = {
  "ACCEPT",
  "",
  "Accepts the game.",
  NULL
};

static struct msg m_accept_msg = {
  "ACCEPT", 0, 0, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_accept,  m_accept },
  m_accept_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_accept_load(void)
{
  if(msg_register(&m_accept_msg) == NULL)
    return -1;

  return 0;
}

void m_accept_unload(void)
{
  msg_unregister(&m_accept_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "ACCEPT"                                                         *
 * -------------------------------------------------------------------------- */
static void m_accept(struct player *player, int argc, char **argv)
{  
  
  if(player->game == tichu_public)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Nicht in einem Game");
    return;
  }
  
  if(player->accepted)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Sie haben bereits akzeptiert");
    return;
  }
  
  player->accepted = 1;
  game_send(player->game, ":%s %s OK :%s hat die Spielbedingungen akzeptiert", player->name, argv[0], player->name);
  
}
