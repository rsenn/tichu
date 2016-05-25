/* $Id: ui_config.c,v 1.12 2005/05/26 10:48:35 smoli Exp $
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

#include <libsgui/sgui.h>

/* -------------------------------------------------------------------------- *
 * Das UI-Config Modul regelt die Konfiguration des Clients                   *
 * -------------------------------------------------------------------------- */
#include "client.h"
#include "sound.h"
#include "stack.h"
#include "card.h"
#include "fan.h"
#include "net.h"
#include "ini.h"
#include "ui_sound.h"
#include "ui_config.h"

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
sgWidget    *ui_config_dialog;
sgWidget    *ui_config_ok;
sgWidget    *ui_config_apply;
sgWidget    *ui_config_abort;
sgWidget    *ui_config_tab;

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void ui_config_resize(sgWidget *image, double wh)
{
  int newwidth;
  int newheight;
  
  newwidth = (double)sgImage(image)->body.h * wh;
  newheight = (double)sgImage(image)->body.w / wh;
   
  if(newheight < sgImage(image)->body.h)
  {
    newheight -= sgImage(image)->body.h;
    
    sgSetWidgetSize(image, image->rect.w, image->rect.h + newheight);
    sgSetWidgetPos(image, image->rect.x, image->rect.y - newheight / 2,
                    SG_ALIGN_LEFT|SG_ALIGN_TOP);
  }
  else if(newwidth < sgImage(image)->body.w)
  {
    newwidth -= sgImage(image)->body.w;
    
    sgSetWidgetSize(image, image->rect.w + newwidth, image->rect.h);
    sgSetWidgetPos(image, image->rect.x - newwidth / 2, image->rect.y,
                   SG_ALIGN_LEFT|SG_ALIGN_TOP);

  }
}
  
/* -------------------------------------------------------------------------- *
 * 'event handling' des [config] Dialoges                                     *
 * -------------------------------------------------------------------------- */
int ui_config_proc(sgWidget *widget, sgEvent event)
{
  ui_generic_proc(widget, event);
  
  if(event == SG_EVENT_QUIT || 
    (event == SG_EVENT_CLICK && widget == ui_config_ok))
  {
    sgClearWidgetStatus(ui_config_dialog, SG_RUNNING);
    client_status = CLIENT_MAIN;
    return 1;
  }
    
  /* Wenn der "Übernehmen" Button gedrückt wurde, dann speichern wir die
     Konfiguration. Falls der Videomodus geändert wurde, dann starten wir
     den Dialog neu */
  if(event == SG_EVENT_CLICK && widget == ui_config_apply)
  {
    if(ui_config_save())
      sgClearWidgetStatus(ui_config_dialog, SG_RUNNING);
  }
  
  return 0;  
}

/* -------------------------------------------------------------------------- *
 * Updatet die verschiedenen Vorschau-Teils und den Soundplayer-Status        *
 * -------------------------------------------------------------------------- */
int ui_config_update(int delay)
{
  if(sgSelectedTabGroup(ui_config_tab) == ui_config_card_group)
    ui_config_card_preview(delay);
  
  if(sgSelectedTabGroup(ui_config_tab) == ui_config_fan_group)
    ui_config_fan_preview(delay);
  
  if(sgSelectedTabGroup(ui_config_tab) == ui_config_stack_group)
    ui_config_stack_preview(delay);
  
  if(sgSelectedTabGroup(ui_config_tab) == ui_config_client_group)
    ui_sound_update(delay);
  
  return 0;
  
}

/* -------------------------------------------------------------------------- *
 * Lädt alle Konfigurationsdaten in die Widgets für ui_config                 *
 * -------------------------------------------------------------------------- */
void ui_config_load(void)
{
  ui_sound_load();
  
  ui_config_client_load();
  ui_config_card_load();
  ui_config_fan_load();
  ui_config_stack_load();
}

/* -------------------------------------------------------------------------- *
 * Speichert alle Konfigurationsdaten aus den Widgets von ui_config           *
 * -------------------------------------------------------------------------- */
int ui_config_save(void)
{
  int ret;
  
  ui_sound_save();
  
  ui_config_client_save();
  ui_config_card_save();
  ui_config_fan_save();
  ui_config_stack_save();
  
  /* Die .ini Datei speichern */
  ini_save(client_ini);
  
  /* ..und dann alle module neu konfigurieren */
  ret = client_configure();
  net_configure(client_ini);
  card_configure(client_ini);
  fan_configure(client_ini);
//  stack_configure(client_ini);

  return ret;
}

/* -------------------------------------------------------------------------- *
 * Initialisiert das gesamte Konfigurations-Interface                         *
 * -------------------------------------------------------------------------- */
int ui_config(SDL_Surface *screen)
{
  SDL_Rect rect_bottom;  
  SDL_Rect rect_back;
  SDL_Rect rect_temp;
  sgColor color;
    
  
  /* Zustand des client's definieren */
  client_status = CLIENT_CONFIG;
  
  /* Dialog erstellen */
  color.r = 0xff;
  color.g = 0x80;
  color.b = 0x00;
  
  ui_config_dialog = sgNewDialog(screen, ui_config_proc, ui_border,
                                 ui_alpha, color, 25);
  
  sgSetWidgetFonts(ui_config_dialog, client_font[0], client_font[1], client_font[2]);
  
  sgSetDialogPattern(ui_config_dialog, client_bgnd);
  sgSetDialogCursorTheme(ui_config_dialog, client_cursor);
  
  /* 16 Pixel Aussenabstand, und unten 36 Pixels für die Buttons */
  rect_temp = client_rect;
  
  sgSubBorder(&rect_temp, 16);
  sgSplitRect(&rect_temp, &rect_bottom, SG_EDGE_BOTTOM, 36);
  
  /* Mit dem erhaltenen Rechteck ein Tab-Widget erstellen */
  ui_config_tab = sgNewTabRect(ui_config_dialog, rect_temp);
  
  /* Purpur Tab für die Client-Konfiguration */
  ui_config_client_group = sgNewGroupFull(ui_config_tab, "Client");
  sgSetWidgetRGB(ui_config_client_group, 192, 0, 192);

  ui_config_client(ui_config_client_group);
  
  /* Hellblaues Tab für die Karten-Konfiguration */
  ui_config_card_group = sgNewGroupFull(ui_config_tab, "Karten");
  sgSetWidgetRGB(ui_config_card_group, 0, 125, 255);

  ui_config_card(ui_config_card_group);
  
  /* Hellgrünes Tab für die Fächer-Konfiguration */
  ui_config_fan_group = sgNewGroupFull(ui_config_tab, "Fächer");
  sgSetWidgetRGB(ui_config_fan_group, 64, 192, 0);
  
  ui_config_fan(ui_config_fan_group);
  
  /* Gelbes Tab für die Stapel-Konfiguration */
  ui_config_stack_group = sgNewGroupFull(ui_config_tab, "Stapel");
  sgSetWidgetRGB(ui_config_stack_group, 255, 225, 0);
  
  ui_config_stack(ui_config_stack_group);
  
  sgSelectTabGroup(ui_config_tab, ui_config_client_group);
  
  /* Buttons */
  sgSplitRect(&rect_bottom, &rect_back, SG_EDGE_RIGHT, rect_bottom.w / 2);
  
  ui_config_ok = sgNewButtonRect(ui_config_dialog, rect_back, "Ok");
  ui_config_apply = sgNewButtonSplitted(ui_config_ok, SG_EDGE_LEFT, ui_config_ok->rect.w * 2 / 3, "Übernehmen");
  ui_config_abort = sgNewButtonSplitted(ui_config_apply, SG_EDGE_LEFT, ui_config_apply->rect.w / 2, "Abbrechen");
  
  sgSetWidgetRGB(ui_config_ok, 64, 192, 0);
  sgSetWidgetRGB(ui_config_apply, 255, 192, 0);
  sgSetWidgetRGB(ui_config_abort, 192, 32, 0);
  
  /* preview timer setzen */
  sgSetDialogTimer(ui_config_dialog, (void *)ui_config_update, PREVIEW_INTERVAL, (void *)0);
  
  sgSetWidgetColor(ui_config_dialog, ui_group_color);
  
  ui_config_load();
  
  /* dialog ausführen */
  sgRunDialog(ui_config_dialog, UI_FADE_DELAY);
  
  ui_config_save();
  
  /* dialog freigeben */
  sgFreeWidget(ui_config_dialog);

  return 0;
}
