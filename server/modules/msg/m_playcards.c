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
#include <tichu/structs.h>
#include <tichu/combo.h>

/* -------------------------------------------------------------------------- *
 * Prototypes                                                                 *
 * -------------------------------------------------------------------------- */
static void m_playcards(struct player *player, int argc, char **argv);

/* -------------------------------------------------------------------------- *
 * Message entries                                                            *
 * -------------------------------------------------------------------------- */
static char *m_playcards_help[] = {
  "PLAYCARDS <card> [<card>, <card>, ..]",
  "",
  "play cards",
  NULL
};

static struct msg m_playcards_msg = {
  "PLAYCARDS", 1, 14, MFLG_PLAYER | MFLG_UNREG,
  { m_registered, m_playcards,  m_playcards },
  m_playcards_help
};

/* -------------------------------------------------------------------------- *
 * Module hooks                                                               *
 * -------------------------------------------------------------------------- */
int m_playcards_load(void)
{
  if(msg_register(&m_playcards_msg) == NULL)
    return -1;

  return 0;
}

void m_playcards_unload(void)
{
  msg_unregister(&m_playcards_msg);
}

/* -------------------------------------------------------------------------- *
 * argv[0] - "playcards"                                                      *
 * argv[1] -  card 1                                                          *
 * argv[*] -  card *                                                          *
 * -------------------------------------------------------------------------- */
static void m_playcards(struct player *player, int argc, char **argv)
{  
  struct node   *node, *node2;
  struct cnode  *cnode;
  struct turn   *turn;
  struct combo  *combo;
  struct list    list;
  int            i;
  char           cardstr[argc - 1];    
  
  cardstr[0] = '\0';
  dlink_list_zero(&list);
  
  /* 
   * NOTE:
   * Ob der Spieler an der reihe ist wird in combo.c:combo_check() geprüft, 
   * da dies beim spielen einer Bombe keine rolle spielt...
   */
  
  if(player->game == tichu_public)
  {    
    player_send(player, "%s FAIL :eventuell zuerst ein Spiel joinen/eröffnen?", argv[0]);
    return;
  }
  
  /* befinden wir uns denn schon im spiel? */
  if(player->game->state != GAME_STATE_GAME)
  {
    player_send(player, "%s FAIL :Es ist noch nicht an der Zeit, Karten zu spielen...", argv[0]);
    return;    
  }
  
  /* prüfen ob der spieler die Karte(n) bestitzt und falls ja, diese gleich in eine temporäre liste linken */
  for(i = 1; i <= argc; i++)
  {
    cnode = cnode_find_by_name(argv[i], &player->cards);
    
    if(!cnode)
    {
      player_send(player, "%s FAIL :Die Karte %s besitzt du nicht..", argv[0], argv[i]);
      
      /* wenn der zug ungülltig ist, müssen wir die karten welche in die temporäre liste gelinkt wurden, wieder dem spieler zuweisen */
      dlink_foreach_safe_data(&list, node, node2, cnode)
      {
        cnode_unlink(cnode, &list);
        cnode_link(cnode, &player->cards);
      }          
      return;
    }
    
    cnode_unlink(cnode, &player->cards);
    cnode_link(cnode, &list);
    
  }
  
    
  /*
   * KOMBINATION PRÜFEN
   */
  if(!(combo = combo_check(player, &list)))
  {    
    /* wenn der zug ungülltig ist, müssen wir die karten welche in die temporäre liste gelinkt wurden, wieder dem spieler zuweisen */
    dlink_foreach_safe_data(&list, node, node2, cnode)
    {
      cnode_unlink(cnode, &list);
      cnode_link(cnode, &player->cards);
    }          
    return;
  }
  else
  {
    /* zug speichern */
    turn = turn_new(player->game, player, combo->type, combo->value);

    /* gespielte karten in den zug linken */
    dlink_foreach_safe_data(&list, node, node2, cnode)
    {      
      cnode_unlink(cnode, &list);
      cnode_link(cnode, &turn->cards);
            
      strcat(cardstr, " ");
      strcat(cardstr, cnode->card->name);
    }        
    
    /* spieler informieren */
    game_send(player->game, ":%s %s%s :%s hat die Karten%s gespielt.",
              player->name, argv[0], cardstr, player->name, cardstr);
    
    player->game->last_finish = 0;
    /* hat der Spieler gerade seine letzte Karte gespielt? */
    if(player->cards.size == 0)
    {
      player->game->finished++;
      player->game->last_finish = 1;
      player->finished = player->game->finished;
      
      /* spieler informieren */
      game_send(player->game, ":%s %s OK :%s hat alle Karten gespielt.", 
                player->name, argv[0], player->name);
      
      
      /* sind beide spieler eines teams nacheinander fertig, 
       * wird das spiel beendet, und das team erhält 200 punkte */
      if(player->game->finished == 2)
      {
        struct player *pptr;
        
        dlink_foreach_data(&player->game->players, node, pptr) {          
          if(pptr->finished  == player->game->finished -1) {            
            if(player->team == pptr->team) {              
              game_round_end(player->game);
              return;
            }
          }
        }
        
      }      

      if(player->game->finished >= player->game->players.size -1)
      {
        /* 3 Spieler sind fertig, runde ist damit zu ende */
        game_round_end(player->game);
        return;
      }      
    }
    
    /* anzahl passungen auf 0 setzten */
    player->game->abandoned = 0;
    
    /*
     * NOTE: Achtung, Hund benötigt (sonder)wurst
     */
    if(combo->type == COMBO_HUND)
    {
      /* Neuen Stich und auspielen darf der gegebenüber */      
      struct prick *prick;
      struct node *nptr;
            
      /* 
       * nächsten Spieler ermitteln
       * 
       * mit dem aufruf von game_next_actor wird dann alles korrekt....
       */
      nptr = &player->game->actor->gnode;
      
      if(nptr == player->game->players.tail)   /* wenn wir am ende sind,     */
        nptr = player->game->players.head;     /* gehen wir zum anfang ;D    */
      else                                     /* sonst,                     */
        nptr = nptr->next;                     /* einfach zum nächsten       */
      
      player->game->actor = nptr->data;      
      
      prick = prick_new(player->game);
      player->game->prick = prick;
      
      game_send(player->game, "PRICK :Neue Stich...");
    }    
    
    /* Der nächste Spieler ist am Zug */    
    game_next_actor(player->game);        
  }  
    
  /* combo struct wieder freigeben */
  combo_delete(combo);
  
  /* debug */
  dlink_foreach_data(&player->game->prick->turns, node, turn)
  {
    log(player_log, L_status, "------ Zug von %s ----", turn->player->name);
    
    dlink_foreach_data(&turn->cards, node2, cnode)
    {
      log(player_log, L_status, "-  Karte %s, (points %i, value %i)", cnode->card->name, cnode->card->points, cnode->value);
    }
    
    log(player_log, L_status, "------ Zug ende von %s ----", turn->player->name);
  }
  
  log(player_log, L_status, "nächster Spieler %s ", player->game->actor->name);

  return;
}
