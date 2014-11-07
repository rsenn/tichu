/* $Id: ui_chat.c,v 1.9 2005/05/26 10:48:35 smoli Exp $
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
 * Das UI-Chat Modul regelt das Verbinden auf den Server, Chatten, Anzeigen   *
 * der Spieler und Spiele, sowie das eröffnen eines Spiels                    *
 * -------------------------------------------------------------------------- */
#include "client.h"
#include "action.h"
#include "net.h"
#include "ui_chat.h"
#include "ui_help.h"

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
sgColor     ui_chat_color = { 0xc0, 0x20, 00 };
sgWidget   *ui_chat_dialog;
sgWidget   *ui_chat_console;
sgWidget   *ui_chat_connect;
sgWidget   *ui_chat_create;
sgWidget   *ui_chat_players;
sgWidget   *ui_chat_games;
sgWidget   *ui_chat_back;
sgWidget   *ui_chat_help;
sgWidget   *ui_chat_input;
sgWidget   *ui_chat_group_io;
sgWidget   *ui_chat_group_panel;

/* -------------------------------------------------------------------------- *
 * Ereignissbehandlung des Chat-Dialogs                                       *
 * -------------------------------------------------------------------------- */
int ui_chat_proc(sgWidget *widget, sgEvent event)
{  
  ui_generic_proc(widget, event);
      
  /* 'back' Button wurde gedrückt */
  if(event == SG_EVENT_QUIT || (event == SG_EVENT_CLICK && widget == ui_chat_back))
  {      
    sgClearWidgetStatus(ui_chat_dialog, SG_RUNNING);
    client_status = CLIENT_MAIN;      
    return 1;
  }    
    
  /* Button wurde gedrückt */
  if(event == SG_EVENT_CLICK)
  {            
    /* 'create' Button wurde gedrückt */
    if(widget == ui_chat_create)
    {      
      net_send("CREATE 0");
    }    

    /* 'help' Button wurde gedrückt */
    else if(widget == ui_chat_help)
    { 
      sgDisableWidget(ui_chat_dialog);
      sgBlitDialog(ui_chat_dialog, client_screen);
      
      ui_help(client_screen);
      
      sgEnableWidget(ui_chat_dialog);
    }
    
    /* 'connect' Button wurde gedrückt */
    else if(widget == ui_chat_connect)
    {
      if(!(net_status & NET_CONNECTED))
      {
        /* Versuche zu verbinden.. */
        if(action_connect())
          return 0;
        
        /* Einloggen... */
        action_login();

        /* UI Status ändern */
        sgSetWidgetCaption(widget, "Trennen");
        
        sgEnableWidget(ui_chat_create);
      }      
      else        
      {        
        net_send("LOGOUT");
        
        net_close();
        
        ui_log(STATUS, "Verbindung getrennt!");
        
        /* games und players list löschen */
        sgClearListbox(ui_chat_players);
        sgClearListbox(ui_chat_games);
        
        sgDisableWidget(ui_chat_create);
        
        sgSetWidgetCaption(widget, "Verbinden");
      }      
    }       
  }
  
  /* Doppelklick */
  if(event == SG_SEL_CLICK)
  {
    if(widget == ui_chat_games)
    {
      /* nicht joinen, wenn der user schon in dem channel ist */
      if(strcmp(sgSelectedListboxItem(widget)->value, client_target))
        net_send("JOIN %s %s", sgSelectedListboxItem(widget)->value, "");
    }
  }
  
  /* Return-Taste wurde gedrückt */
  if(event == SG_RETURN)
  {    
    if(widget == ui_chat_input && ui_chat_input->caption[0])
    {
      client_message(ui_chat_input->caption);
    }
  }
    
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Startet den Chat-Dialog                                                    *
 * -------------------------------------------------------------------------- */
int ui_chat(SDL_Surface *screen)
{
  SDL_Rect rect_io;
  SDL_Rect rect_panel;
  
  /* zustand des client's definieren */
  client_status = CLIENT_CHAT;

  /* Create a dialog */
  ui_chat_dialog = sgNewDialog(screen, ui_chat_proc, ui_border,
                               ui_alpha, ui_chat_color, 25);
  
  sgSetWidgetFonts(ui_chat_dialog, client_font[0], client_font[1], client_font[2]);
  
  sgSetDialogPattern(ui_chat_dialog, client_bgnd);
  sgSetDialogCursorTheme(ui_chat_dialog, client_cursor);
  
  /* Split dialog area into rects */
  rect_io = client_rect;  
  sgSubBorder(&rect_io, ui_chat_dialog->border);  
  sgSplitRect(&rect_io, &rect_panel, SG_EDGE_RIGHT, 300);
    
  /* groups */
  ui_chat_group_io = sgNewGroupRect(ui_chat_dialog, rect_io, "Chat");
  
  
  ui_chat_group_panel = sgNewGroupRect(ui_chat_dialog, rect_panel, "Spielverwaltung");
  
  /* background picture */
  sgLoadGroupImage(ui_chat_group_io, "chat.png", SG_ALIGN_CENTER);
  
  /* console + input */
  ui_chat_input   = 
    sgNewInputGrouped(ui_chat_group_io, SG_EDGE_BOTTOM,
                      SG_ALIGN_LEFT|SG_ALIGN_BOTTOM, 
                      sgGroup(ui_chat_group_io)->body.w,
                      34, NULL);
  
  ui_chat_console =
    sgNewConsoleGrouped(ui_chat_group_io, SG_EDGE_BOTTOM,
                        SG_ALIGN_LEFT|SG_ALIGN_TOP,
                        sgGroup(ui_chat_group_io)->body.w,
                        sgGroup(ui_chat_group_io)->splitted.h,
                        NULL);
  
  /* add backuped text */
  sgAddConsoleText(ui_chat_console, client_buffer);
//  client_free(&client_buffer);
  sgScrollConsole(ui_chat_console, 65535, 1, 1);
  
  /* set default dialog input */
//  sgSetDialogInput(ui_chat_dialog, ui_chat_input);
  
  /* buttons */
  ui_chat_connect =
    sgNewButtonGrouped(ui_chat_group_panel, SG_EDGE_BOTTOM,
                       SG_ALIGN_LEFT|SG_ALIGN_TOP, 
                       sgGroup(ui_chat_group_panel)->body.w,
                       34,
                       (net_status & NET_CONNECTED) ? "Trennen" : "Verbinden");
  ui_chat_back =
    sgNewButtonSplitted(ui_chat_connect, 
                        SG_EDGE_RIGHT, sgGroup(ui_chat_group_panel)->body.w/2, "Zurück");
    
  ui_chat_create  =
    sgNewButtonGrouped(ui_chat_group_panel, SG_EDGE_BOTTOM,
                       SG_ALIGN_LEFT|SG_ALIGN_TOP,
                       sgGroup(ui_chat_group_panel)->body.w,
                       34, 
                       "Neues Spiel");  
  ui_chat_help =
    sgNewButtonSplitted(ui_chat_create, 
                        SG_EDGE_RIGHT, sgGroup(ui_chat_group_panel)->body.w/2, "Hilfe");
   /* listboxes */
  ui_chat_games   =
    sgNewListboxGrouped(ui_chat_group_panel, SG_EDGE_TOP,
                        SG_ALIGN_RIGHT|SG_ALIGN_TOP,
                        sgGroup(ui_chat_group_panel)->splitted.w,
                        sgGroup(ui_chat_group_panel)->splitted.h / 2, 2,
                        "Spiele");
  
  ui_chat_players =
    sgNewListboxGrouped(ui_chat_group_panel, SG_EDGE_TOP,
                        SG_ALIGN_RIGHT|SG_ALIGN_TOP, 
                        sgGroup(ui_chat_group_panel)->splitted.w,
                        sgGroup(ui_chat_group_panel)->splitted.h, 1, 
                        "Spieler");

  sgSetDialogTimer(ui_chat_dialog, (void *)net_poll, NET_INTERVAL, (void *)0);
  
  if(net_status & NET_CONNECTED)
  {
    net_send("GAMES");
    net_send("PLAYERS %s", client_target);
  }
  else 
  {
    sgDisableWidget(ui_chat_create);
  }
    

  sgSetWidgetColor(ui_chat_dialog, client_bgcolor);

  sgSetWidgetColor(ui_chat_group_panel, ui_group_color);
  sgSetWidgetColor(ui_chat_group_io, ui_group_color);

  sgSetWidgetColor(ui_chat_console, ui_chat_color);
  
  sgRunDialog(ui_chat_dialog, UI_FADE_DELAY);
  
  client_buffer = sgGetConsoleText(ui_chat_console);
  
  sgFreeWidget(ui_chat_dialog);
  
  return 0;
}

