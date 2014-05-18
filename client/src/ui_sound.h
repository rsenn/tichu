/* $Id: ui_sound.h,v 1.3 2005/05/21 22:36:09 smoli Exp $
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

#ifndef UI_SOUND_H
#define UI_SOUND_H

#include <libsgui/sgui.h>

/* -------------------------------------------------------------------------- *
 * Das UI-Sound Modul stellt einen Sound-Player bereit                        *
 * -------------------------------------------------------------------------- */
#include "ui.h"

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
extern sgWidget *ui_sound_music;
extern sgWidget *ui_sound_fx;
extern sgWidget *ui_sound_list;
extern sgWidget *ui_sound_next;
extern sgWidget *ui_sound_prev;
extern sgWidget *ui_sound_play;
extern sgWidget *ui_sound_stop;
 
/* -------------------------------------------------------------------------- *
 * Funktionen                                                                 *
 * -------------------------------------------------------------------------- */
int              ui_sound_proc   (sgWidget *widget, 
                                  sgEvent   event);
void             ui_sound_load   (void);
void             ui_sound_save   (void);
void             ui_sound_select (int       i);
void             ui_sound_show   (void);
void             ui_sound_update (Uint32    delay);
  
void             ui_sound        (sgWidget *group);

#endif /* UI_SOUND_H */
