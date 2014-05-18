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
#include <tichu/combo.h>
#include <tichu/structs.h>
#include <tichu/game.h>
#include <tichu/cnode.h>

/* -------------------------------------------------------------------------- *
 * Prototypes                                                                 *
 * -------------------------------------------------------------------------- */
static void m_abandon(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_abandon_help[] = {
  "ABANDON",
  "",
  "Passen",
  NULL
};

static struct msg m_abandon_msg = {
  "ABANDON", 0, 0, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_abandon,  m_abandon },
  m_abandon_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_abandon_load(void)
{
  if(msg_register(&m_abandon_msg) == NULL)
    return -1;

  return 0;
}

void m_abandon_unload(void)
{
  msg_unregister(&m_abandon_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "abandon"                                                        *
 * -------------------------------------------------------------------------- */
static void m_abandon(struct player *player, int argc, char **argv)
{  
  struct node   *node;
  struct node   *node2;
  struct cnode  *cnode;
  struct turn   *turn;
  
  /* Ist der Spieler überhaupt in einem spiel? */
  if(player->game == tichu_public)
  {
    player_send(player, "%s FAIL :Spieler ist in einem Spiel", argv[0], argv[1]);
    return;
  }        
  
  /* Ist der Spieler überhaupt an der reihe ? */
  if(player != player->game->actor)
  {
    player_send(player, "%s FAIL :Spieler ist nicht an der Reihe", argv[0], argv[1]);
    return;
  }    
  
  /* 
   * einen "lehren" turn in den stich linken
   */
  turn = turn_new(player->game, player, COMBO_ABANDON, 0);

  game_send(player->game, ":%s ABANDON :%s hat gepasst",
            player->name, player->name);
  
  /* wenn weniger als 2 spieler gepasst haben, normal weiter machen */
  if(player->game->abandoned < (2 - player->game->finished + player->game->last_finish))
  {
    /* festhalten, dass der spieler gepasst hat */
    player->game->abandoned++;
    /* nächster Spieler ist am zug */
    game_next_actor(player->game);
  }
  else
  {
    /* 3 Spieler haben nach einander gepasst d.h. das heisst es gibt einen neuen stich */
    struct prick *prick;
    
    /* anzahl passungen auf 0 setzten */
    player->game->abandoned = 0;
    
    prick = prick_new(player->game);
    player->game->prick = prick;
        
    game_send(player->game, "PRICK :Neue Runde...");
    game_next_actor(player->game);
  }
 
  log(player_log, L_status, "nächster Spieler %s ", player->game->actor->name);
  
  /* debug */
  dlink_foreach_data(&player->game->prick->turns, node, turn)
  {
    log(player_log, L_status, "------ Zug von %s ----", turn->player->name);
    
    dlink_foreach_data(&turn->cards, node2, cnode)
    {
      log(player_log, L_status, "-  Karte %s", cnode->card->name);
    }
    
    log(player_log, L_status, "------ Zug ende von %s ----", turn->player->name);
  }
  
  return;
}
