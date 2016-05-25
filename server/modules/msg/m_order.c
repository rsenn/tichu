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
static void m_order(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_order_help[] = {
  "ORDER",
  "",
  "List order of a game",
  NULL
};

static struct msg m_order_msg = {
  "ORDER", 0, 0, MFLG_PLAYER,
  { m_registered, m_order,  m_order },
  m_order_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_order_load(void)
{
  if(msg_register(&m_order_msg) == NULL)
    return -1;

  return 0;
}

void m_order_unload(void)
{
  msg_unregister(&m_order_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "ORDER"                                                           *
 * -------------------------------------------------------------------------- */
static void m_order(struct player *player, int argc, char **argv)
{  
/*  struct game   *game;*/
/*  struct node   *node;*/
  struct player *pptr = NULL;
  int i = 0;
  
  if(player->game == tichu_public)
  {
    player_send(player, "%s FAIL :%s", argv[0], "Du befindest dich nicht in einem Spiel");
    return;
  }
  
  pptr = player;

  do
  {    
    player_send(player, "%s %s %i %s", argv[0], pptr->name, i, image_color_str(pptr->color));
    
    i++;
    if(pptr->gnode.next)
      pptr = pptr->gnode.next->data;
    else 
      pptr = player->game->players.head->data;
    
    
  } while(pptr != player);
  
  
  return;
}
