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
static void m_team(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_team_help[] = {
  "TEAM <team>",
  "",
  "Chose Team. (1 or 2)",
  NULL
};

static struct msg m_team_msg = {
  "TEAM", 1, 1, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_team,  m_team },
  m_team_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_team_load(void)
{
  if(msg_register(&m_team_msg) == NULL)
    return -1;

  return 0;
}

void m_team_unload(void)
{
  msg_unregister(&m_team_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "TEAM"                                                           *
 * argv[1] -  team                                                            *
 * -------------------------------------------------------------------------- */
static void m_team(struct player *player, int argc, char **argv)
{  
  struct node   *node;
  struct player *pptr;
  int team;
  int count = 0;
  
  team = (uint32_t)strtoul(argv[1], NULL, 10);
  
  if(player->game == tichu_public)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Nicht in einem Game");
    return;
  }
  
  if(player->game->state != GAME_STATE_CHAT)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Teams können zur zeit nicht gewählt werden");
    return;
  }
  
  if(team < 1 || team > 2)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Kein gültiges Team, gültige Teams: 1, 2");
    return;
  }
  
  if(team == player->team)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Bereits in diesem Team");
    return;
  }
  
  
  /* prüfen ob noch platz in diesem Team ist */
  dlink_foreach_data(&player->game->players, node, pptr)
  {
    if(pptr->team == team)
      count++;
  }
  
  /* kein platz mehr */
  if(count >= 2)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Team ist schon voll");
    return;
  }
  
  player->team = team;

  game_send(player->game, ":%s %s OK %s :%s hat sich für Team %i entschieden..", player->name, argv[0], argv[1], player->name, team);
  
}
