/* $Id: ui.h,v 1.33 2005/05/26 10:48:35 smoli Exp $
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

#ifndef UI_H
#define UI_H

#include <libsgui/sgui.h>

/* -------------------------------------------------------------------------- *
 * Das UI Modul regelt die Benutzerinteraktion ausserhalb des Spiels          *
 * -------------------------------------------------------------------------- */
#include "gfxutil.h"

/* -------------------------------------------------------------------------- *
 * Makros fürs Loggen                                                         *
 * -------------------------------------------------------------------------- */
#define ui_log(l, s...) writelog(MOD_UI, (l), s)

#ifdef DEBUG_UI
#define ui_debug(l, s...) ui_log((l), s)
#else
#define ui_debug(l, s...)
#endif

/* -------------------------------------------------------------------------- *
 * Konstanten                                                                 *
 * -------------------------------------------------------------------------- */
#define GRADIENT_DIFF 0x60
#define PREVIEW_FPS 10
#define PREVIEW_INTERVAL (1000 / PREVIEW_FPS)

#define NET_INTERVAL 5

#define UI_DROPDOWN_HEIGHT 128
#define UI_LABEL_WIDTH      96
#define UI_LABEL_HEIGHT     26
#define UI_EDIT_HEIGHT      32
#define UI_COLOR_HEIGHT     32
#define UI_BUTTON_HEIGHT    45
#define UI_TOGGLE_HEIGHT    28
#define UI_ADJUST_HEIGHT    28

#define UI_FADE_DELAY 500

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
extern int         ui_border;
extern Uint8       ui_alpha;
extern int         ui_fade_delay;
extern sgColor     ui_group_color;

// GAME_END Dialog 
extern sgWidget   *ui_game_end_points1;
extern sgWidget   *ui_game_end_points2;

/* -------------------------------------------------------------------------- *
 * Funktionen                                                                 *
 * -------------------------------------------------------------------------- */
int                 ui_loadsounds      (void);
int                 ui_loadmusic       (void);
  
int                 ui_test            (SDL_Surface *screen);

int                 ui_generic_proc    (sgWidget    *widget, 
                                        sgEvent      event);
  
#endif /* UI_H */
