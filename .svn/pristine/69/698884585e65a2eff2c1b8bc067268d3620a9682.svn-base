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
static void m_msg(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_msg_help[] = {
  "MSG <target> :<msg>",
  "",
   "Send <msg> to <target>.",
  NULL
};

static struct msg m_msg_msg = {
  "MSG", 2, 2, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_msg, m_msg },
  m_msg_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_msg_load(void)
{
  if(msg_register(&m_msg_msg) == NULL)
    return -1;

  return 0;
}

void m_msg_unload(void)
{
  msg_unregister(&m_msg_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "MSG"                                                            *
 * argv[1] - target                                                           *
 * argv[2] - msg                                                              *
 * -------------------------------------------------------------------------- */
static void m_msg(struct player *player, int argc, char **argv)
{  
  struct player *target;
  struct game *game;
      
  if(*argv[1] == '@')
  {
    /* Message an Game */
    game = game_find_name(argv[1]);
    
    /* an public * /
    if(strlen(argv[1]) == 1)
      game = tichu_public;
    else
      game = game_find_name(argv[1]);
    */ 
    
    if(!game)
      player_send(player, "%s FAIL :%s", argv[0], "Keinen gültigen channel angegeben");
    else
      game_send(game, ":%s %s %s :%s", player->name, argv[0], argv[1], argv[2]);
    
  }
  else
  {
    /* Message an Player */
    target = player_find_name(argv[1]);
  
    if(!target)
    {
      /* error: kein gültiges target */
      char *error = "Kein gültiges target angegeben";
      player_send(player, "%s FAIL :%s", argv[0], error);
    
    }
    else
    {
      /* ok: message versenden */
      player_send(target, ":%s %s %s :%s", player->name, argv[0], argv[1], argv[2]);
      player_send(player, ":%s %s %s :%s", player->name, argv[0], argv[1], argv[2]);
    }
  
  }
  
    
}
