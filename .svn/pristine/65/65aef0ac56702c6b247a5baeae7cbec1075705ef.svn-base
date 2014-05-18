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
static void m_create(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_create_help[] = {
  "CREATE <gametyp> [<key>]",
  "",
  "Create Game with type <gametyp> and Key <key>.",
  NULL
};

static struct msg m_create_msg = {
  "CREATE", 1, 2, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_create,  m_create },
  m_create_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_create_load(void)
{
  if(msg_register(&m_create_msg) == NULL)
    return -1;

  return 0;
}

void m_create_unload(void)
{
  msg_unregister(&m_create_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "CREATE"                                                         *
 * argv[1] - gametyp                                                          *
 * argv[2] - key                                                              *
 * -------------------------------------------------------------------------- */
static void m_create(struct player *player, int argc, char **argv)
{  
  struct game   *game;
  struct player *pptr;
  struct node   *node;
  char           name [TICHU_GAMELEN + 1];
  
  memset(name, '\0', TICHU_GAMELEN + 1);
  
  if(player->game != tichu_public)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Zur zeit noch in einem Game");
    return;
  }
  
  if((uint32_t)strtoul(argv[1], NULL, 10) != GAME_TYPE_FUN && player->type == PLAYER_USER)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Spieltyp erfordert Registrierung");
    return;
  }
  
  /* erstelle ein neues Spiel */  
  *name = '@';
  strncpy(name + 1, player->name, sizeof(player->name));
 
  game = game_new(name);
  
  /* initialize some stuff in the game struct */
  game->type    = (uint32_t)strtoul(argv[1], NULL, 10);
  game->founder = player; 
  game->state   = GAME_STATE_CHAT;
  
  /* neuen channel joinen, alter wird automatisch verlassen */
  if(argc == 3)
    game_join(game, player, argv[2]);
  else
    game_join(game, player, '\0');  

  /* game-founder akzeptiert automatisch */
  player->accepted = 1;
  
  player_send(player, "%s OK :%s", argv[0], "Neues Game erstellt");
  
  /* Information an ALLE spieler senden */
  dlink_foreach_data(&player_list, node, pptr)
  {
    player_send(pptr, ":%s %s :%s hat ein neues Spiel erstellt", player->name, argv[0], player->name);
  }
  
  return;
}
