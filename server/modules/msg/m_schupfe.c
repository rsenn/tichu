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
static void m_schupfe(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_schupfe_help[] = {
  "SCHUPFE <target> <card>",
  "",
  "Schupfe <card> an <target>",
  NULL
};

static struct msg m_schupfe_msg = {
  "SCHUPFE", 2, 2, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_schupfe,  m_schupfe },
  m_schupfe_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_schupfe_load(void)
{
  if(msg_register(&m_schupfe_msg) == NULL)
    return -1;

  return 0;
}

void m_schupfe_unload(void)
{
  msg_unregister(&m_schupfe_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "schupfe"                                                        *
 * argv[1] -  target                                                          *
 * argv[2] -  card                                                            *
 * -------------------------------------------------------------------------- */
static void m_schupfe(struct player *player, int argc, char **argv)
{  
  struct node   *node;
  struct cnode  *cnode;
  struct card   *card;
  struct player *target;
  
  /* ist schupfen denn angesagt? */
  if(player->game == NULL || player->game->state != GAME_STATE_SCHUPFEN)
  {
    player_send(player, "%s FAIL :Zur Zeit kein Schupfen möglich...", argv[0]);
    return;
  }
  
  /* prüfen ob noch schupfungen nötig sind */
  if(player->schupfen.size >= (player->game->players.size - 1))
  {
    player_send(player, "%s FAIL :%s", argv[0], "Allen Spielern schon geschupft");
    return;
  }
  
  /* prüfen ob dem Spieler "target" schon geschupft wurde */
  dlink_foreach_data(&player->schupfen, node, cnode)
  {
    if(!strcmp(cnode->player->name,argv[1]))
    {
      player_send(player, "%s FAIL :%s", argv[0], "Diesem Spieler bereits geschupft");
      return;
    }
  }
  
  /* prüfen ob sich der spieler "target" in dem game befindet und ob target nicht der spieler selbst ist*/
  if(!(target = player_find_name(argv[1])) ||  target->game != player->game || target == player)
  {
    player_send(player, "%s FAIL :Der Spieler %s ist nicht gütlitg.", argv[0], argv[1]);    
    return;
  }

  /* prüfen ob der spieler die Karte bestitzt */
  if(!(card = card_find_name(&player->cards, argv[2])))
  {
    player_send(player, "%s FAIL :Karte %s ist unzulässig", argv[0], argv[2]);
    return;
  }
  
  /*
   * Vorgang wenn eine Karte geschupft wird:
   * -> haben die Karte und der Spieler allen prüfungen stand gehalten
   * -> wird die Karte aus der player->card list herausgenommen
   * -> und in die player->schupfen list gelinkt
   * 
   * in der Funktion game_schupfen wird dann
   * -> die karte aus player->schupfen
   * -> nach target->cards gelinkt
   */
  cnode            = cnode_find_by_card(card, &player->cards);
  cnode->player    = target;
  
  cnode_unlink(cnode, &player->cards);
  cnode_link(cnode, &player->schupfen);
   
  player_send(player, "%s OK :%s", argv[0], "Die Schupfung ist geglück ;)");

  // wird in game_schupfen erledigt, darf zu diesem zeit punkt noch nicht mitgeteilt werden
  /*  player_send(target, ":%s %s OK %s %s :%s schupft %s an %s",
              player->name, argv[0], target->name, card->name,
              player->name, card->name, target->name);
  */
  game_schupfen(player->game);
  
  return;
}
