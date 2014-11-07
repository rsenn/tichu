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
static void m_players(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_players_help[] = {
  "PLAYERS <channel>",
  "",
  "List all Players in <channel>",
  NULL
};

static struct msg m_players_msg = {
  "PLAYERS", 1, 1, MFLG_PLAYER,
  { m_registered, m_players,  m_players },
  m_players_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_players_load(void)
{
  if(msg_register(&m_players_msg) == NULL)
    return -1;

  return 0;
}

void m_players_unload(void)
{
  msg_unregister(&m_players_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "PLAYERS"                                                           *
 * argv[1] - channel                                                          *
 * -------------------------------------------------------------------------- */
static void m_players(struct player *player, int argc, char **argv)
{  
  struct game   *game;
  struct node   *node;
  struct player *pptr;
  
  game = game_find_name(argv[1]);
  
  if(!game)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Game/Channel existiert nicht");
    return;
  }
  
  dlink_foreach_data(&game->players, node, pptr)
  {
    player_send(player, "%s %s %i %i", 
                argv[0], pptr->name, pptr->team, pptr->accepted);
  }
  
  player_send(player, "%s :Ende der Spielerliste", argv[0]);
  
  return;
}
