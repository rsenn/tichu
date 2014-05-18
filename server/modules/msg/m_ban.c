/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/dlink.h>
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
static void m_ban(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_ban_help[] = {
  "ban <player>",
  "",
  "ban a player from game",
  NULL
};

static struct msg m_ban_msg = {
  "BAN", 1, 1, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_ban,  m_ban },
  m_ban_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_ban_load(void)
{
  if(msg_register(&m_ban_msg) == NULL)
    return -1;

  return 0;
}

void m_ban_unload(void)
{
  msg_unregister(&m_ban_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "BAN"                                                            *
 * argv[1] -  player                                                          *
 * -------------------------------------------------------------------------- */
static void m_ban(struct player *player, int argc, char **argv)
{  
  struct player *pptr;
  
  if(player->game == tichu_public)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Vergiss es");
    return;
  }
  
  if(player->game->founder != player)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Bannen kann nur der Gamefounder");
    return;
  }
  
  if(!(pptr = player_find_name(argv[1])))
  {
    player_send(player, "%s FAIL :Kontte den User '%s' nicht finden.", argv[0], argv[1]);
    return;
  }

  if(game_is_banned(player->game, pptr))
  {
    player_send(player, "%s FAIL :Der Spieler '%s' ist bereits verbannt...", argv[0], argv[1]);
    return;
  }

  /*if(!game_is_member(player->game, pptr))
  {
    player_send(player, "%s FAIL :Der User (%s) befindet sich nicht in diesem Game", argv[0], argv[1]);
    return;
  }*/
  
  /* dann verbannen wir den mistkerl */
  dlink_add_head(&player->game->banned, &pptr->bnode, pptr);

  player_send(player, "%s OK :Der Spieler '%s' wurde von unserer Runde ausgeschlossen...", argv[0], argv[1]);
  
}
