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
static void m_join(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_join_help[] = {
  "JOIN <channel> [<key>]",
  "",
  "Joins Game.",
  NULL
};

static struct msg m_join_msg = {
  "JOIN", 1, 2, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_join,  m_join },
  m_join_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_join_load(void)
{
  if(msg_register(&m_join_msg) == NULL)
    return -1;

  return 0;
}

void m_join_unload(void)
{
  msg_unregister(&m_join_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "JOIN"                                                           *
 * argv[1] - channel                                                          *
 * argv[2] - key                                                              *
 * -------------------------------------------------------------------------- */
static void m_join(struct player *player, int argc, char **argv)
{  
  struct game *game;
  
  game = game_find_name(argv[1]);
  
  if(!game)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Game/Channel existiert nicht");
    return;
  }
  
  if(game->state == GAME_STATE_GAME)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Das Spiel wurde schon gestartet");
    return;
  }
  
  if(game->players.size >= 4 && argv[1] != "@")
  {
    player_send(player, "%s FAIL :%s", argv[0], "Game ist voll");
    return;
  }
  
  if(dlink_find(&game->banned, player))
  {
    player_send(player, "%s FAIL :%s", argv[0], "Sie wurden von diesem Spiel ausgeschlossen.");
    return;
  }
  
  // if(game->type == GAME_TYPE_Tournier && player->registriert
 // {
 //   player_send(player, "%s FAIL :%s", argv[0], "Game erfordert Registrierung");
 //   return;
 // }
  if(game_has_key(game) && game->key != argv[2])
  {
    player_send(player, "%s FAIL :%s", argv[0], "Falscher Key");
    return;
  }
  
  
  if(argc == 3)
    game_join(game, player, argv[2]);
  else
    game_join(game, player, "");
  
  player_send(player, "%s OK :%s", argv[0], "erfolgreicher Join");
  return;
}
