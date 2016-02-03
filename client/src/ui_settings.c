/* $Id: ui_settings.c,v 1.6 2005/05/26 10:48:35 smoli Exp $
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
 * Im UI-Settings Dialog kann ein Spiel vorbereitet und gechattet werden      *
 * -------------------------------------------------------------------------- */
#include "client.h"
#include "net.h"
#include "ui_settings.h"

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */

SDL_Surface *ui_symbol_accept[5];
SDL_Surface *ui_symbol_user[5];
SDL_Surface *ui_symbol_icon[5];
SDL_Surface *ui_symbol_team[5];

sgColor   ui_settings_color = { .r=0x00,.g=0x78,.b=0xff };
sgWidget *ui_settings_dialog = NULL;
sgWidget *ui_settings_group_panel;
sgWidget *ui_settings_group_io;
sgWidget *ui_settings_console;
sgWidget *ui_settings_input;
sgWidget *ui_settings_team1;
sgWidget *ui_settings_team2;
sgWidget *ui_settings_accept;
sgWidget *ui_settings_start;
sgWidget *ui_settings_ban;
sgWidget *ui_settings_kick;
sgWidget *ui_settings_type1;
sgWidget *ui_settings_type2;
sgWidget *ui_settings_type3;
sgWidget *ui_settings_players;
sgWidget *ui_settings_label_players;
sgWidget *ui_settings_back;

/* -------------------------------------------------------------------------- *
 * 'event handling' des [settings] Dialoges                                   *
 * -------------------------------------------------------------------------- */
int ui_settings_proc(sgWidget *widget, sgEvent event)
{
  ui_generic_proc(widget, event);

  if(event == SG_EVENT_CLICK)
  {
    if(widget == ui_settings_team1)
    {      
      net_send("TEAM 1");
      sgDisableWidget(widget);
      sgEnableWidget(ui_settings_team2);
    }    

    else if(widget == ui_settings_team2)
    {      
      net_send("TEAM 2");
      sgDisableWidget(widget);
      sgEnableWidget(ui_settings_team1);
    }        

    else if(widget == ui_settings_accept)
    {      
      net_send("ACCEPT");
      sgDisableWidget(widget);
    }    

    else if(widget == ui_settings_start)
    {
      net_send("START");
    }       

    else if(widget == ui_settings_kick)
    {
      sgListboxItem *item = NULL;
      char msg[30] = "KICK \0";            

      item = sgSelectedListboxItem(ui_settings_players);
    
      if(item)
      {
        strcat(msg, item->caption);        
        net_send(msg);        
      }
    }    

    else if(widget == ui_settings_ban)
    {      
      sgListboxItem *item = NULL;
      char msg[30] = "BAN \0";
            
      item = sgSelectedListboxItem(ui_settings_players);
    
      if(item)
      {
        strcat(msg, item->caption);        
        net_send(msg);        
      }
    }        
  
    else if(widget == ui_settings_back)
    {            
      net_send("LEAVE");
            
      sgClearWidgetStatus(ui_settings_dialog, SG_RUNNING);
      client_status = CLIENT_CHAT;
      return 1;
    }
        
  }
  
  /* Return taste wurde gedrückt */  
  else if(event == SG_RETURN)
  {    
    if(widget == ui_settings_input && strcmp(ui_settings_input->caption, ""))
    {
      client_message(ui_settings_input->caption);
    }
  }

  if(event == SG_EVENT_QUIT)
    sgClearWidgetStatus(ui_settings_dialog, SG_RUNNING);
  
  
  return 0;  
}

/* -------------------------------------------------------------------------- *
 * Startet das [settings] User-Interface                                      *
 * -------------------------------------------------------------------------- */
int ui_settings(SDL_Surface *screen)
{
  SDL_Rect rect_panel;
  SDL_Rect rect_io;
  
  /* zustand des client's definieren */
  client_status = CLIENT_SETTINGS;
  
  /* Symbole laden */
  client_load_icons("accept.png", ui_symbol_accept, 2);
  client_load_icons("team.png", ui_symbol_team, 2);
  client_load_icons("icon.png", ui_symbol_icon, 3);
  client_load_icons("num.png",  ui_symbol_user, 4);
  
  /* Create a dialog */
  ui_settings_dialog = sgNewDialog(client_screen, ui_settings_proc, ui_border,
                                   ui_alpha, ui_settings_color, 25);
  
  sgSetWidgetFonts(ui_settings_dialog, client_font[0], client_font[1], client_font[2]);
  
  sgSetDialogPattern(ui_settings_dialog, client_bgnd);
  sgSetDialogCursorTheme(ui_settings_dialog, client_cursor);
  
  /* Split dialog area into rects */
  rect_io = client_rect;
  sgSubBorder(&rect_io, ui_settings_dialog->border);  
  sgSplitRect(&rect_io, &rect_panel, SG_EDGE_LEFT, 300);
  
  
  ui_settings_group_io = sgNewGroupRect(ui_settings_dialog, rect_io, "Chat");
  
  ui_settings_group_panel = sgNewGroupRect(ui_settings_dialog, rect_panel, "Spieleinstellungen");
  
  /* Hintergrundbild */
  sgLoadGroupImage(ui_settings_group_io, "settings.png", SG_ALIGN_CENTER);
  
  
  /* console + input */
  ui_settings_input =
    sgNewInputGrouped(ui_settings_group_io, SG_EDGE_BOTTOM, 0, 
                      sgGroup(ui_settings_group_io)->body.w, 34, NULL);

//FIXME  sgSetDialogInput(ui_settings_dialog, ui_settings_input);
  
  ui_settings_console = 
    sgNewConsoleGrouped(ui_settings_group_io, SG_EDGE_TOP, 0,
                        sgGroup(ui_settings_group_io)->body.w,
                        sgGroup(ui_settings_group_io)->body.h,
                        NULL);
  
  sgAddConsoleText(ui_settings_console, client_buffer);
  client_free(client_buffer);

  /* listboxe */
  ui_settings_players =
    sgNewListboxGrouped(ui_settings_group_panel, SG_EDGE_TOP, 0, 
                        sgGroup(ui_settings_group_panel)->body.w, 230,
                        2, "Spieler");
  /* spacer */
  sgPadRect(&sgGroup(ui_settings_group_panel)->body, SG_EDGE_TOP, 20);
  
  /* buttons */
  ui_settings_team1 =
    sgNewButtonGrouped(ui_settings_group_panel, SG_EDGE_TOP, SG_ALIGN_LEFT,
                       sgGroup(ui_settings_group_panel)->body.w, 34, "Team 1");
  ui_settings_team2 =
    sgNewButtonSplitted(ui_settings_team1, SG_EDGE_RIGHT, 
                        sgGroup(ui_settings_group_panel)->body.w/2, "Team 2");
  
  ui_settings_accept =
    sgNewButtonGrouped(ui_settings_group_panel, SG_EDGE_TOP, SG_ALIGN_LEFT,
                       sgGroup(ui_settings_group_panel)->body.w, 34, "Akzeptieren");
  ui_settings_start  =
    sgNewButtonSplitted(ui_settings_accept, SG_EDGE_RIGHT, 
                        sgGroup(ui_settings_group_panel)->body.w/2, "Starten");
  
  ui_settings_ban  =
    sgNewButtonGrouped(ui_settings_group_panel, 
                       SG_EDGE_TOP, SG_ALIGN_LEFT,
                       sgGroup(ui_settings_group_panel)->body.w, 34, "Ban");
  ui_settings_kick =
    sgNewButtonSplitted(ui_settings_ban, SG_EDGE_RIGHT, 
                        sgGroup(ui_settings_group_panel)->body.w/2, "Kick");
  
  ui_settings_back =
    sgNewButtonGrouped(ui_settings_group_panel,
                       SG_EDGE_BOTTOM, 0,
                       sgGroup(ui_settings_group_panel)->body.w, 34, "Zurück");
  
  sgSetDialogTimer(ui_settings_dialog, (void *)net_poll, NET_INTERVAL, (void *)0);
  
  /* bestimmte Buttons sind nur für den game founder Aktiviert*/
  if(strcmp(&client_target[1], client_config.user))
  {        
    sgDisableWidget(ui_settings_start);
    sgDisableWidget(ui_settings_kick);
    sgDisableWidget(ui_settings_ban);
  }
  else
    sgDisableWidget(ui_settings_accept);
  
  /* players list für das game anfordern */
  net_send("PLAYERS %s", client_target);

  /* Konsolen inhalt anzeigen ??? */
//  sgForceDialogRedraw(ui_settings_dialog, NULL);
  
  sgRunDialog(ui_settings_dialog, UI_FADE_DELAY);
  
  client_buffer = sgGetConsoleText(ui_settings_console);
  
  sgFreeWidget(ui_settings_dialog);
  
  return 0;
}

