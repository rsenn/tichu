/* $Id: ui_config.h,v 1.2 2005/05/21 10:09:34 smoli Exp $
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

#ifndef UI_CONFIG_H
#define UI_CONFIG_H

#include <libsgui/sgui.h>

/* -------------------------------------------------------------------------- *
 * Das UI-Config Modul regelt die Konfiguration des Clients                   *
 * -------------------------------------------------------------------------- */
#include "ui.h"

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
extern sgWidget    *ui_config_dialog;
extern sgWidget    *ui_config_ok;
extern sgWidget    *ui_config_apply;
extern sgWidget    *ui_config_abort;
extern sgWidget    *ui_config_tab;        

/* -------------------------------------------------------------------------- *
 * Konfigurationsdialog                                                       *
 * -------------------------------------------------------------------------- */
int                 ui_config_proc    (sgWidget    *widget, 
                                       sgEvent      event);
int                 ui_config_preview (int          delay);
void                ui_config_load    (void);
int                 ui_config_save    (void);
void                ui_config_resize  (sgWidget    *image, 
                                       double       wh);
int                 ui_config         (SDL_Surface *screen);
  
/* -------------------------------------------------------------------------- *
 * Client-Konfiguration                                                       *
 * -------------------------------------------------------------------------- */
extern sgWidget    *ui_config_client_group;
extern sgWidget    *ui_config_client_user;
extern sgWidget    *ui_config_client_pass;
extern sgWidget    *ui_config_client_host;
extern sgWidget    *ui_config_client_port;
extern sgWidget    *ui_config_client_fullscreen;
extern sgWidget    *ui_config_client_grab;
extern sgWidget    *ui_config_client_resolution;
extern sgWidget    *ui_config_client_cursor;
extern sgWidget    *ui_config_client_pattern;
extern sgWidget    *ui_config_client_hue;
extern sgWidget    *ui_config_client_saturation;
extern sgWidget    *ui_config_client_value;
extern sgWidget    *ui_config_client_contrast;
extern sgWidget    *ui_config_client_music;
extern sgWidget    *ui_config_client_fx;
extern sgWidget    *ui_config_client_playlist;
extern sgWidget    *ui_config_client_next;
extern sgWidget    *ui_config_client_prev;
extern sgWidget    *ui_config_client_play;
extern sgWidget    *ui_config_client_stop;

void                ui_config_client_load (void);
void                ui_config_client_save (void);
void                ui_config_client      (sgWidget *group);
int                 ui_config_client_proc (sgWidget *widget, 
                                           sgEvent   event);

/* -------------------------------------------------------------------------- *
 * Kartenkonfiguration                                                        *
 * -------------------------------------------------------------------------- */
extern sgWidget    *ui_config_card_group;
extern sgWidget    *ui_config_card_hue;
extern sgWidget    *ui_config_card_saturation;
extern sgWidget    *ui_config_card_value;
extern sgWidget    *ui_config_card_border;
extern sgWidget    *ui_config_card_antialias;
extern sgWidget    *ui_config_card_zoom;
extern Uint32       ui_config_card_counter;
extern Uint32       ui_config_card_last;
extern double       ui_config_card_angle;
extern struct card *ui_config_card_card;
extern SDL_Surface *ui_config_card_surface;
extern SDL_Rect     ui_config_card_preview_rect;
extern sgWidget    *ui_config_card_image;

void                ui_config_card_show    (void);
int                 ui_config_card_preview (int       delay);
void                ui_config_card_load    (void);
void                ui_config_card_save    (void);
int                 ui_config_card_proc    (sgWidget *widget, 
                                            sgEvent   event);
void                ui_config_card         (sgWidget *group);        

/* -------------------------------------------------------------------------- *
 * Fächer-Konfiguration                                                       *
 * -------------------------------------------------------------------------- */
extern sgWidget    *ui_config_fan_group;
extern sgWidget    *ui_config_fan_angle;
extern sgWidget    *ui_config_fan_offset;
extern sgWidget    *ui_config_fan_image;
extern SDL_Rect     ui_config_fan_rect;
extern Uint32       ui_config_fan_last;
extern int          ui_config_fan_redraw;

void                ui_config_fan_preview (int       delay);
void                ui_config_fan_load    (void);
void                ui_config_fan_save    (void);
int                 ui_config_fan_proc    (sgWidget *widget, 
                                           sgEvent   event);
void                ui_config_fan         (sgWidget *group);
          
/* -------------------------------------------------------------------------- *
 * Stapel-Konfiguration                                                       *
 * -------------------------------------------------------------------------- */
extern sgWidget    *ui_config_stack_group;

void                ui_config_stack_preview (int       delay);
void                ui_config_stack_load    (void);
void                ui_config_stack_save    (void);
int                 ui_config_stack_proc    (sgWidget *widget, 
                                             sgEvent   event);
void                ui_config_stack         (sgWidget *group);

#endif /* UI_CONFIG_H */
