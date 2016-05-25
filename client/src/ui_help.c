/* $Id: ui_help.c,v 1.3 2005/05/26 10:48:35 smoli Exp $
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
 * Zeigt Hilfe-Dialoge in den verschiedenen User-Interfaces an                *
 * -------------------------------------------------------------------------- */
#include "client.h"
#include "ui.h"

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
sgWidget *ui_help_box;
sgWidget *ui_help_back;
sgWidget *ui_help_group;
sgWidget *ui_help_dialog;
sgColor   ui_help_color = { 0xff, 0xc0, 0x80 };  

/* -------------------------------------------------------------------------- *
 * Ereignissbehandlung des Help-Dialoges                                      *
 * -------------------------------------------------------------------------- */
int ui_help_proc(sgWidget *widget, sgEvent event)
{
  if(event == SG_EVENT_CLICK)
  {    
    /* 'help back' Button wurde gedrückt */
    if(widget == ui_help_back)
      sgClearWidgetStatus(ui_help_dialog, SG_RUNNING);
  }
  
  if(event == SG_EVENT_QUIT)
    sgClearWidgetStatus(ui_help_dialog, SG_RUNNING);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Startet einen Hilfe-Dialog                                                 *
 * -------------------------------------------------------------------------- */
int ui_help(SDL_Surface *screen)
{
  SDL_Rect rect;
    
  rect.w = 600;
  rect.h = 400;
  
  sgAlignRect(&client_rect, &rect, SG_ALIGN_CENTER|SG_ALIGN_MIDDLE);
  
  /* Create a dialog */
  ui_help_dialog = sgNewDialog(client_screen, ui_help_proc, ui_border,
                               ui_alpha, ui_help_color, 25);

  sgSetWidgetFonts(ui_help_dialog, client_font[0], client_font[1], client_font[2]);

  sgSetDialogPattern(ui_help_dialog, client_bgnd);
  sgSetDialogCursorTheme(ui_help_dialog, client_cursor);
  
  ui_help_group = sgNewGroupFull(ui_help_dialog, "Hilfe");
  
  ui_help_back =
    sgNewButtonGrouped(ui_help_group, SG_EDGE_BOTTOM,
                       SG_ALIGN_CENTER|SG_ALIGN_BOTTOM, 
                       sgGroup(ui_help_group)->body.w, 40, "Zurück");  
  ui_help_box = 
    sgNewConsoleGrouped(ui_help_group, SG_EDGE_BOTTOM, 
                        SG_ALIGN_CENTER|SG_ALIGN_TOP, sgGroup(ui_help_group)->body.w,
                        sgGroup(ui_help_group)->body.h, NULL);
  
  
  switch(client_status)
  {    
    case CLIENT_MAIN:
      break;
    case CLIENT_CONFIG:
      break;
    case CLIENT_CHAT:
//      sgAddConsoleText(ui_help_box, HELP_CHAT);
      break;
    case CLIENT_SETTINGS:
      break;
    case CLIENT_GAME:
      break; 
  }
                       
  sgScrollConsole(ui_help_box, 65535, 1, 1);
  
  sgRunDialog(ui_help_dialog, UI_FADE_DELAY);
  
  sgFreeWidget(ui_help_dialog);
  
  return 0;
}
