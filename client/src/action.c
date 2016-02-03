/* $Id: action.c,v 1.19 2005/05/21 08:27:20 smoli Exp $
 * -------------------------------------------------------------------------- *
 *  .___.    .                                                                *
 *    |  * _.|_ . .        Portabler, SDL-basierender Client für das          *
 *    |  |(_.[ )(_|             Multiplayer-Kartenspiel Tichu.                *
 *  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . *
 *                                                                            *
 *               (c) 2004-2005 by Martin Zangger, Roman Senn                  *
 *                                                                            *
 *    Dieses Programm ist freie Software. Sie können es unter den Bedingungen *
 * der GNU General Public License, wie von der Free Software Foundation ver-  *
 * öffentlicht, weitergeben und/oder modifizieren, entweder gemäss Version 2  *
 * der Lizenz oder (nach Ihrer Option) jeder späteren Version.                *
 *                                                                            *
 *    Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, dass es  *
 * Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, sogar ohne die  *
 * implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BE-    *
 * STIMMTEN ZWECK. Details finden Sie in der GNU General Public License.      *
 *                                                                            *
 *    Sie sollten eine Kopie der GNU General Public License zusammen mit      *
 * diesem Programm erhalten haben. Falls nicht, schreiben Sie an die Free     *
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA          *
 * 02111-1307, USA.                                                           *
 * -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- *
 * Das ACTION Modul leitet Client-Aktionen weiter an den Server               *
 * -------------------------------------------------------------------------- */
#include "action.h"
#include "client.h"
#include "net.h"

/* -------------------------------------------------------------------------- *
 * Verbindet auf den Server                                                   *
 * -------------------------------------------------------------------------- */
int action_connect(void)
{
  /* Konfiguration aus der "net" Sektion lesen */
  net_configure(client_ini);
  
  action_log(STATUS, "Serveradresse %s auflösen...", net_host);
  
  /* DNS Lookup versuchen */
  if(net_resolve())
  {
    action_log(STATUS, "Server %s nicht gefunden!", net_host);
    return -1;
  }
  
  /* Wenn erfolgreich -> Verbindung initiieren */
  action_log(STATUS, "Verbinde nach %s:%u...", net_ip, net_port);
  
  if(net_connect())
  {
    action_log(STATUS, "Verbindung fehlgeschlagen!");
    return -1;
  }
  
  return 0;
}  
  
/* -------------------------------------------------------------------------- *
 * Loggt den Spieler ein                                                      *
 * -------------------------------------------------------------------------- */
void action_login(void)
{
  /* Client-Konfiguration laden */
  client_configure();

  /* Karten-Konfiguration laden (Farbton) */
  card_configure(client_ini);
  
  action_log(STATUS, "Verbunden... Einloggen als Benutzer %s (%s)",
             client_config.user, client_config.info);
  
  /* Login command an Server */
  net_send("LOGIN %s %s :%s", client_config.user, 
           client_config.pass[0] ? client_config.pass : "-", client_config.info);
  
  /* Farbton setzen */
  net_send("COLOR %s", gfxutil_strcolor(card_tint));
  
  /* Game list von Server anfordern */
  net_send("GAMES");
}  
  
/* -------------------------------------------------------------------------- *
 * Wird aufgerufen wenn der Spieler (eine) Karte(n) spielt                    *
 * -------------------------------------------------------------------------- */
void action_play(struct list *cards)
{
  struct node *node;
  struct card *card;
  char *cardstr;
  int n;

  /* ...dazu müssen wir aber Karten haben :) */
  if(cards->size)
  {
    /* Kartenstring zusammensetzen */
    cardstr = alloca(cards->size * 3 + 1);
    n = 0;
    
    dlink_foreach_data(cards, node, card)
    {
      /* Immer nur 2 Zeichen pro Karte */
      cardstr[n++] = ' ';
      cardstr[n++] = card->name[0];
      cardstr[n++] = card->name[1];
      cardstr[n] = '\0';
    }

    /* ..und dann an den Server schicken */
    action_debug(INFO, "Play [%s ]", cardstr);

    net_send("PLAYCARDS%s", cardstr);
  }  
}

/* -------------------------------------------------------------------------- *
 * Wird aufgerufen wenn der Spieler einem anderen Spieler schupft             *
 * -------------------------------------------------------------------------- */
void action_schupf(struct player *player)
{
  struct card *card;
  struct node *node;
  char *cardstr;
  int n;

  /* Uhm, also eigentlich sollte diese Liste immer 1 Karte beinhalten */
  if(player->cards.size)
  {
    /* Kartenstring zusammensetzen, falls mal mehr als 1 Karte geschupft 
       werden müssen (Bei einem anderen Spiel, z.B. "Arschlöchlen") */
    cardstr = alloca(player->cards.size * 3 + 1);
    n = 0;
    
    dlink_foreach_data(&player->cards, node, card)
    {
      /* Immer nur 2 Zeichen pro Karte */
      cardstr[n++] = ' ';
      cardstr[n++] = card->name[0];
      cardstr[n++] = card->name[1];
      cardstr[n] = '\0';
    }
    
    /* Karte an den Server senden.. */
    action_debug(INFO, "Schupf [ player = %s, cards =%s ]", player->name, cardstr);

    net_send("SCHUPFE %s%s", player->name, cardstr);
  }
  
  /* Kartenliste des Spielers löschen, damit wir sie für die Karten 
     die wir nächstens geschupft bekommen wiederverwenden können */
  dlink_list_zero(&player->cards);
  
  /* Schupfungen durchgeführt */
  player->status |= PLAYER_SENT;
}

