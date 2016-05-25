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
static void m_games(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_games_help[] = {
  "GAMES",
  "",
  "List all games",
  NULL
};

static struct msg m_games_msg = {
  "GAMES", 0, 0, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_games,  m_games },
  m_games_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_games_load(void)
{
  if(msg_register(&m_games_msg) == NULL)
    return -1;

  return 0;
}

void m_games_unload(void)
{
  msg_unregister(&m_games_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "games"                                                           *
 * -------------------------------------------------------------------------- */
static void m_games(struct player *player, int argc, char **argv)
{  
  struct node   *node;
  struct game   *gptr;
  
  dlink_foreach_data(&game_list, node, gptr)
  {
    player_send(player, "%s %s %i %i", argv[0], gptr->name, gptr->players.size, gptr->type);
  }
  
  player_send(player, "%s :Ende der Game-Liste", argv[0]);
  
  return;
}
