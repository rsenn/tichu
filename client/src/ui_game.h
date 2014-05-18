/* $Id: ui_game.h,v 1.2 2005/05/23 12:52:38 smoli Exp $
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

#ifndef UI_GAME_H
#define UI_GAME_H

#include <libsgui/sgui.h>

/* -------------------------------------------------------------------------- *
 * Das UI-Game Modul verwaltet das GUI im Game-Modus                          *
 * -------------------------------------------------------------------------- */
#include "ui.h"

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
extern sgWidget *ui_game_dialog;
extern sgWidget *ui_game_chat_group;
extern sgWidget *ui_game_menu_group;
extern sgWidget *ui_game_chat_console;
extern sgWidget *ui_game_chat_input;
extern sgWidget *ui_game_button_sort;
extern sgWidget *ui_game_button_abandon;

extern sgWidget *ui_game_status_actor;
extern sgWidget *ui_game_actor;
extern sgWidget *ui_game_status_spacer;
extern sgWidget *ui_game_status_group;
extern sgWidget *ui_game_status_points1;
extern sgWidget *ui_game_status_points2;
extern sgWidget *ui_game_status_points1;
extern sgWidget *ui_game_status_points2;
extern sgWidget *ui_game_status_players;
extern sgWidget *ui_game_status_player[3];
extern sgWidget *ui_game_image_player[3];
extern sgWidget *ui_game_image_spacer[2];
extern sgWidget *ui_game_button_abort;
extern sgColor   ui_game_color;
extern sgWidget *ui_game_dialog_tichu;
extern SDL_Rect  ui_game_dialog_tichu_rect;
extern sgWidget *ui_game_group_tichu;
extern sgWidget *ui_game_button_tichu_big;


/* -------------------------------------------------------------------------- *
 * Funktionen                                                                 *
 * -------------------------------------------------------------------------- */
int              ui_game_proc        (sgWidget    *widget, 
                                      sgEvent      event);
void             ui_game_tichu_end   (void);
void             ui_game_tichu_start (void);
int              ui_game_loop        (SDL_Surface *screen);
void             ui_game_redraw      (SDL_Surface *surface);
int              ui_game_update      (Uint32       diff);


void             ui_game             (SDL_Surface *screen);
void             ui_game_end         (SDL_Surface *screen);

void             ui_game_status      (sgWidget    *group);
void             ui_game_chat        (sgWidget    *group);
    

#endif /* UI_GAME_H */
