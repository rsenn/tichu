/* $Id: action.h,v 1.7 2005/05/21 08:27:20 smoli Exp $
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

#ifndef ACTION_H
#define ACTION_H

/* -------------------------------------------------------------------------- *
 * Das ACTION Modul leitet Client-Aktionen weiter an den Server               *
 * -------------------------------------------------------------------------- */
#include "player.h"

/* -------------------------------------------------------------------------- *
 * Makros fürs Loggen                                                         *
 * -------------------------------------------------------------------------- */
#define action_log(l, s...) writelog(MOD_ACTION, (l), s)

#if DEBUG_ACT
#define action_debug(l, s...) action_log((l), s)
#else
#define action_debug(l, s...)
#endif

/* -------------------------------------------------------------------------- *
 * Verbindet auf den Server                                                   *
 * -------------------------------------------------------------------------- */
int  action_connect (void);  

/* -------------------------------------------------------------------------- *
 * Loggt den Spieler ein                                                      *
 * -------------------------------------------------------------------------- */
void action_login   (void);
  
/* -------------------------------------------------------------------------- *
 * Wird aufgerufen wenn der Spieler (eine) Karte(n) spielt                    *
 * -------------------------------------------------------------------------- */
void action_play    (struct list   *cards);

/* -------------------------------------------------------------------------- *
 * Wird aufgerufen wenn der Spieler einem anderen Spieler schupft             *
 * -------------------------------------------------------------------------- */
void action_schupf  (struct player *player);

#endif /* ACTION_H */
