/* $Id: ui_main.c,v 1.2 2005/05/26 10:48:35 smoli Exp $
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
 * Das UI-Main Modul stellt das Hauptmenü dar                                 *
 * -------------------------------------------------------------------------- */
#include "client.h"
#include "ui_main.h"

/* -------------------------------------------------------------------------- *
 * Menü Objekte                                                               *
 * -------------------------------------------------------------------------- */
sgWidget *ui_main_dialog;
sgWidget *ui_main_logo;
sgWidget *ui_main_group;
sgWidget *ui_main_play;
sgWidget *ui_main_test;
sgWidget *ui_main_config;
sgWidget *ui_main_exit;

/* -------------------------------------------------------------------------- *
 * Ereignissbehandlung des Menu-Dialoges                                      *
 * -------------------------------------------------------------------------- */
int ui_main_proc(sgWidget *widget, sgEvent event)
{
  ui_generic_proc(widget, event);
  
  if(event == SG_EVENT_CLICK)
  {
    /* play button */
    if(widget == ui_main_play)
    {
      sgClearWidgetStatus(ui_main_dialog, SG_RUNNING);
      client_status = CLIENT_CHAT;
      return 1;
    }
    
    /* test button */
    if(widget == ui_main_test)
    {
      sgClearWidgetStatus(ui_main_dialog, SG_RUNNING);
      client_status = CLIENT_TEST;
      return 1;
    }
    
    /* config button */
    else if(widget == ui_main_config)
    {
      sgClearWidgetStatus(ui_main_dialog, SG_RUNNING);
      client_status = CLIENT_CONFIG;
      return 1;
    }
  }
  
    /* exit button or quit event */
  if((event == SG_EVENT_CLICK &&
      widget == ui_main_exit) || event == SG_EVENT_QUIT)
  {      
    sgClearWidgetStatus(ui_main_dialog, SG_RUNNING);
      client_status = CLIENT_EXIT;
    return 1;
  }

  return 0;
}

/* -------------------------------------------------------------------------- *
 * Startet das Menu-Interface                                                 *
 * -------------------------------------------------------------------------- */
int ui_main(SDL_Surface *screen)
{
  SDL_Rect rect;
  SDL_Surface *image;
  
  /* zustand des client's definieren */
  client_status = CLIENT_MAIN;
  
  /* create the dialog */
  ui_main_dialog = sgNewDialog(client_screen, ui_main_proc, ui_border, 
                               ui_alpha, ui_group_color, 25);
  
  sgSetDialogPattern(ui_main_dialog, client_bgnd);
  sgSetDialogContrast(ui_main_dialog, client_config.contrast);
  sgSetDialogCursorTheme(ui_main_dialog, client_cursor);
  
  sgSetWidgetFonts(ui_main_dialog, client_font[0], client_font[1], client_font[2]);
  
  /* Grösse des Logos */
  client_load_png(&image, "menu.png");
  
  rect.w = image->w;
  rect.h = image->h;
  
  sgAlignRect(&client_rect, &rect, SG_ALIGN_CENTER|SG_ALIGN_TOP);
  
  ui_main_logo = sgNewImageRect(ui_main_dialog, rect, NULL);
  
  sgSetWidgetBorder(ui_main_logo, 0);
  
  sgSetImageSurface(ui_main_logo, image, SG_ALIGN_CENTER|SG_ALIGN_MIDDLE);
  client_free_image(image);
  
  /* Grösse der Gruppe für das Menu */
  rect.h = ((client_config.height - ui_main_logo->rect.h) & -4) - 2;
  rect.w = (rect.h * 112 / 100);

  sgAlignRect(&client_rect, &rect, SG_ALIGN_CENTER|SG_ALIGN_BOTTOM);
  
  ui_main_group = sgNewGroupRect(ui_main_dialog, rect, "Menu");
  
  /* buttons */
  ui_main_play = sgNewButtonGrouped(ui_main_group, SG_EDGE_TOP, SG_ALIGN_CENTER, 
                                    sgGroup(ui_main_group)->body.w,
                                    (sgGroup(ui_main_group)->body.h) / 4, 
                                    "Spielen");

  ui_main_config = sgNewButtonGrouped(ui_main_group, SG_EDGE_TOP, SG_ALIGN_CENTER, 
                                      sgGroup(ui_main_group)->body.w,
                                      (sgGroup(ui_main_group)->body.h) / 4, 
                                      "Einstellungen");
  
  ui_main_exit = sgNewButtonGrouped(ui_main_group, SG_EDGE_TOP, SG_ALIGN_CENTER,
                                    sgGroup(ui_main_group)->body.w,
                                    (sgGroup(ui_main_group)->body.h) / 4, 
                                    "Verlassen");
  
  ui_main_test = sgNewButtonGrouped(ui_main_group, SG_EDGE_TOP, SG_ALIGN_CENTER,
                                    sgGroup(ui_main_group)->body.w, 
                                    (sgGroup(ui_main_group)->body.h) / 4,
                                    "Engine-Test");
  /* Button-Farben setzen */
  sgSetWidgetRGB(ui_main_play, 0xe0, 0x00, 0x00);
  sgSetWidgetRGB(ui_main_config, 0xff, 0x80, 0x00);
  sgSetWidgetRGB(ui_main_exit, 0xff, 0xe1, 0x00);
  sgSetWidgetRGB(ui_main_test, 0x40, 0xc0, 0x00);

  sgSetWidgetColor(ui_main_group, ui_group_color);
  sgSetWidgetColor(ui_main_dialog, client_bgcolor);
  
  sgRunDialog(ui_main_dialog, UI_FADE_DELAY);
  
  sgFreeWidget(ui_main_dialog);
  
  return 0;
}

