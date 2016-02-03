/* $Id: ui_settings.h,v 1.2 2005/05/21 10:09:34 smoli Exp $
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

#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include <libsgui/sgui.h>

/* -------------------------------------------------------------------------- *
 * Im UI-Settings Dialog kann ein Spiel vorbereitet und gechattet werden      *
 * -------------------------------------------------------------------------- */
#include "ui.h"

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
extern SDL_Surface *ui_symbol_accept[5];
extern SDL_Surface *ui_symbol_user[5];
extern SDL_Surface *ui_symbol_icon[5];
extern SDL_Surface *ui_symbol_team[5];

extern sgWidget    *ui_settings_dialog;
extern sgWidget    *ui_settings_group_panel;
extern sgWidget    *ui_settings_group_io;
extern sgWidget    *ui_settings_console;
extern sgWidget    *ui_settings_input;
extern sgWidget    *ui_settings_team1;
extern sgWidget    *ui_settings_team2;
extern sgWidget    *ui_settings_accept;
extern sgWidget    *ui_settings_start;
extern sgWidget    *ui_settings_ban;
extern sgWidget    *ui_settings_kick;
extern sgWidget    *ui_settings_type1;
extern sgWidget    *ui_settings_type2;
extern sgWidget    *ui_settings_type3;
extern sgWidget    *ui_settings_players;
extern sgWidget    *ui_settings_label_players;
extern sgWidget    *ui_settings_back;

/* -------------------------------------------------------------------------- *
 * Funktionen                                                                 *
 * -------------------------------------------------------------------------- */
int                 ui_settings_proc (sgWidget    *widget, 
                                      sgEvent      event);
int                 ui_settings      (SDL_Surface *screen);

#endif /* UI_SETTINGS_H */
