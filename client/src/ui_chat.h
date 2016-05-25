/* $Id: ui_chat.h,v 1.2 2005/05/21 10:09:34 smoli Exp $
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

#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <libsgui/sgui.h>

/* -------------------------------------------------------------------------- *
 * Das UI-Chat Modul regelt das Verbinden auf den Server, Chatten, Anzeigen   *
 * der Spieler und Spiele, sowie das eröffnen eines Spiels                    *
 * -------------------------------------------------------------------------- */
#include "gfxutil.h"
#include "ui.h"

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
extern sgWidget *ui_chat_dialog;
extern sgWidget *ui_chat_console;
extern sgWidget *ui_chat_connect;
extern sgWidget *ui_chat_create;
extern sgWidget *ui_chat_players;
extern sgWidget *ui_chat_games;

/* -------------------------------------------------------------------------- *
 * Funktionen                                                                 *
 * -------------------------------------------------------------------------- */
int              ui_chat_proc (sgWidget    *widget, 
                               sgEvent      event);
int              ui_chat      (SDL_Surface *screen);

/* -------------------------------------------------------------------------- *
 * Help Texte                                                                 *
 * -------------------------------------------------------------------------- */

#define HELP_CHAT "  _____ _      _                     _   _      _\n" \
  " |_   _(_) ___| |__  _   _          | | | | ___| |_ __\n" \
    "   | | | |/ __| '_ \\| | | |  _____  | |_| |/ _ \\ | '_ \\\n" \
    "   | | | | (__| | | | |_| | |_____| |  _  |  __/ | |_) |\n" \
    "   |_| |_|\\___|_| |_|\\__,_|         |_| |_|\\___|_| .__/\n" \
    "                                                 |_|\n"\
    "\n"\
    "\n"\
    "Herrzlich Willkomen bei der Tichu-Hilfe\n"\
    "---------------------------------------\n"\
    "  1. Einleitung\n"\
    "  2. Spile\n"\
    "  3. Spieler\n"\
    "  4. Neues Spiel\n"\
    "  5. Verbinden / Trennen\n"\
    "  6. Chat-Anzeige\n"\
    "  7. Chat-Eingabezeile\n"\
    "\n"\
    "\n"\
    "\n"\
    "1. Einleitung\n"\
    "-----------------------------------------------------------------\n"\
    " Die Hilfe soll Ihnen im umgang mit diesem Dialog weiterhelfen.\n"\
    " Untenstehend werden die einzelnen Elemente des Dialoges genauer\n"\
    " beschrieben, doch zuerst eine Kurzfassung für alle ungeduldigen.\n"\
    "\n"\
    " Kurzfassung:\n"\
    " Als erstes müssen Sie eine Verbindung zum Tichu-Server herstellen,\n"\
    " klicken Sie dazu auf den Verbinden-Button.\n"\
    " Ist die Verbindung hergestellt sehen Sie auf der rechten Seite\n"\
    " des Fensters nun die aktuell verfügbahren Spiele sowie die Spiler\n"\
    " Sie können nun gleich einem Spiel beitreten und Spielen :)\n"\
    "\n"\
    "\n"\
    "2. Spiele\n"\
    "-----------------------------------------------------------------\n"\
    " In der oberen rechten Ecke des Dialogs ist die 'Spiele-Anzeige'\n"\
    " Hier sehen Sie immer die Aktuell verfügbahren Spiele, sowie\n"\
    " den Spieltyp des Spiels gekennzeichnet durch ein Symbol(Wir\n"\
    " unterscheiden zwischen Fun-, Liga- und Turnier-Spiel).\n"\
    " Rechts neben dem Symbol sehen sie die Anzahl Spieler, welche\n"\
    " sich bereits in diesem Spiel befinden(pro Spiel maximal 4 Spieler).\n"\
    "\n"\
    "\n"\
    "3. Spieler\n"\
    "-----------------------------------------------------------------\n"\
    " Diese Anzeige befindet sich gleich unter den Spielen.\n"\
    " Hier sind alle Spieler aufgelistet welche sich noch nicht in einem\n"\
    " Spiel befinden.\n"\
    " \n"\
    " Sie können sich mit diesen Spielern unterhalten und sich gegebenfalls\n"\
    " für ein Spiel verabreden. (siehe 'Chat-Anzeige/Eingabezeile')\n"\
    "\n"\
    "\n"\
    "4. Neues Spiel\n"\
    "-----------------------------------------------------------------\n"\
    " Sie können natürlich auch selbst ein Spiel eröffnen,\n"\
    " klicken Sie dazu auf diesen Button.\n"\
    " Es wird sich dann ein neuer Dialog öffnen, weitere Hilfe erhalten,\n"\
    " Sie dann im nächsten Fenster.\n"\
    "\n"\
    "\n"\
    "5. Chat-Anzeige\n"\
    "-----------------------------------------------------------------\n"\
    " In dieser Anzeige sehen sie sämtliche Chat-Nachrichten\n"\
    "   <Spieler> Dies ist die Nachricht\n"\
    " Zeilen die mit drei Sternen (*** Nachricht) beginnen, sind\n"\
    " Informationen vom Server, die man im Allgemeinen ignorieren kann\n"\
    " Zusätzlich werden noch die Anfragen und Antworten an und von dem\n"\
    " Server ausgegeben. \n"\
    "\n"\
    "\n"\



#endif /* UI_CHAT_H */
