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
#include <tichu/card.h>
#include <tichu/player.h>
#include <tichu/game.h>
#include <tichu/cnode.h>

/* -------------------------------------------------------------------------- *
 * Prototypes                                                                 *
 * -------------------------------------------------------------------------- */
static void m_cards(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_cards_help[] = {
  "cards",
  "",
  "List all cards",
  NULL
};

static struct msg m_cards_msg = {
  "CARDS", 0, 0, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_cards,  m_cards },
  m_cards_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_cards_load(void)
{
  if(msg_register(&m_cards_msg) == NULL)
    return -1;

  return 0;
}

void m_cards_unload(void)
{
  msg_unregister(&m_cards_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "cards"                                                           *
 * -------------------------------------------------------------------------- */
static void m_cards(struct player *player, int argc, char **argv)
{  
  struct node   *node;
  struct cnode  *cnode;
  
  combo_sort_list(&player->cards);
  
  dlink_foreach_data(&player->cards, node, cnode)
  {
    player_send(player, "%s %s", argv[0], cnode->card->name);
  }
  
  return;
}
