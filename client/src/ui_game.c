/* $Id: ui_game.c,v 1.4 2005/05/26 10:48:35 smoli Exp $
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

#include <string.h>
#include <libsgui/sgui.h>

/* -------------------------------------------------------------------------- *
 * Erstellt und verwaltet das ganze Benutzerinterface während des Spiels      *
 * -------------------------------------------------------------------------- */
#include "net.h"
#include "fan.h"
#include "card.h"
#include "game.h"
#include "ui_game.h"
#include "ui_sound.h"

#define GAME_RAISE_SPEED 1000

#define CALC_FPS 1
#define DRAW_FPS 0

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
sgWidget    *ui_game_dialog;
sgWidget    *ui_game_status_group;
sgWidget    *ui_game_chat_group;
sgWidget    *ui_game_menu_group;
sgWidget    *ui_game_sound_group;
sgWidget    *ui_game_chat_console;
sgWidget    *ui_game_chat_input;
sgWidget    *ui_game_button_sort;
sgWidget    *ui_game_button_abandon;
sgWidget    *ui_game_status_actor;
sgWidget    *ui_game_actor;
sgWidget    *ui_game_status_spacer;
sgWidget    *ui_game_status_points1;
sgWidget    *ui_game_status_points2;
sgWidget    *ui_game_status_points1;
sgWidget    *ui_game_status_points2;
sgWidget    *ui_game_status_players;
sgWidget    *ui_game_status_player[3];
sgWidget    *ui_game_image_player[3];
sgWidget    *ui_game_image_spacer[2];
sgWidget    *ui_game_button_abort;
sgColor      ui_game_color;
sgWidget    *ui_game_dialog_tichu;
SDL_Rect     ui_game_dialog_tichu_rect;
sgWidget    *ui_game_group_tichu;
//sgWidget    *ui_game_button_tichu;
sgWidget    *ui_game_button_tichu_big;

/*Uint8        game_alpha;
int          game_border = 5;
Uint32       game_ticks;
Uint32       game_lastupdate;
SDL_Surface *ui_game_ui;
SDL_Surface *ui_game_bgnd;
double       game_lastfps = 0.0;
double       game_fps = 0.0;
int          game_status = 0;
sgColor      game_color;*/

/* -------------------------------------------------------------------------- *
 * Lokale Variabeln                                                           *
 * -------------------------------------------------------------------------- */

static sgWidget *ui_game_button_menu;
static sgWidget *ui_game_button_chat;
static sgWidget *ui_game_button_sound;

static sgWidget *ui_game_end_dialog;
static sgWidget *ui_game_end_group;
static sgWidget *ui_game_end_team1;
static sgWidget *ui_game_end_team2;
static sgWidget *ui_game_end_exit;
static sgColor   ui_game_end_color;

/* -------------------------------------------------------------------------- *
 * Zeichnet den Game-Dialog (ohne Hintergrund, da dieser gezeichnet werden    *
 * muss bevor die Blitting-Routinen der Engine aufgerufen werden)             *
 * -------------------------------------------------------------------------- */
static int ui_game_dialog_blit(sgWidget *dialog, SDL_Surface *surface, 
                               Sint16 x, Sint16 y)
{
  sgWidget *child;
  
  /* Alle Widgets im Dialog blitten */
  sgForeachUp(&dialog->list, child)
    if(child->methods.blit)
      child->methods.blit(child, surface, 0, 0);  
  
  /* Redraw Flags löschen */
  sgClearWidgetStatus(dialog, SG_REDRAW_NEEDED);
  
  return 1;
}

/* -------------------------------------------------------------------------- *
 * Behandelt Dialog-Ereignisse                                                *
 * -------------------------------------------------------------------------- */
int ui_game_proc(sgWidget *widget, sgEvent event)
{
//  client_log(LOG_DEBUG, "game dialog event");
  
  if(event == SG_RETURN)
  {
    ui_debug(INFO, "Dialog RETURN Event");
  
    if(widget == ui_game_chat_input && ui_game_chat_input->caption[0])
      client_message(ui_game_chat_input->caption);
  }
  
  if(event == SG_EVENT_CLICK)
  {
//    client_log(LOG_DEBUG, "click");
    
    if(widget == ui_game_button_abort)
    {
      game_unset(GAME_RUN);
      client_status = CLIENT_CHAT;
    }
    else if(widget == ui_game_button_sort)
    {      
      fan_sort();
    }
    else if(widget == ui_game_button_abandon)
    {
      net_send("ABANDON");
    }
    
  }

  return 0;
}

/* -------------------------------------------------------------------------- *
 * Initialisiert das Game-Benutzerinterface                                   *
 * -------------------------------------------------------------------------- */
void ui_game(SDL_Surface *screen)
{
  SDL_Rect rect;
  SDL_Rect chatrect;
  SDL_Rect menurect;
  SDL_Rect statusrect;
  SDL_Rect soundrect;
  
  /* Im Game-Modus haben die Widgets die Spielerfarbe */
  ui_game_color.r = card_tint.r;
  ui_game_color.g = card_tint.g;
  ui_game_color.b = card_tint.b;
  
  /* Einen Dialog erstellen, und die Fonts, den Hintergrund und die Cursors
     setzen */
  ui_game_dialog =
    sgNewDialog(screen, ui_game_proc, ui_border, ui_alpha, ui_game_color, 50);
    
  sgSetDialogCursorTheme(ui_game_dialog, client_cursor);
  sgSetDialogContrast(ui_game_dialog, client_config.contrast);
  sgSetDialogPattern(ui_game_dialog, client_bgnd);
  sgSetWidgetFonts(ui_game_dialog, client_font[0],
                   client_font[1], client_font[2]);
  
  /* Custom Blitting-Methode für diesen Dialog, da der Hintergrund separat
     gezeichnet werden muss */
  ui_game_dialog->methods.blit = ui_game_dialog_blit;
  
  /* groups */
  rect = ui_game_dialog->rect;  
  sgSplitRect(&rect, &statusrect, SG_EDGE_BOTTOM, 48);
  
  chatrect.x = 100;
  chatrect.y = 100;
  chatrect.w = 400;
  chatrect.h = 200;  
  
  /* Position und Grösse des Soundplayers */
  soundrect.w = 400;
  soundrect.h = 200;
  
  soundrect.x = 400;
  soundrect.y = 300;
  
  /* Position und Grösse des Menus */
  menurect.w = 400;
  menurect.h = 200;
  
  sgAlignRect(&client_rect, &menurect, SG_ALIGN_LEFT|SG_ALIGN_BOTTOM);
  
  menurect.y -= statusrect.h;
  
  /* Gruppenboxen erstellen */
  ui_game_status_group = sgNewGroupRect(ui_game_dialog, statusrect, NULL);
  ui_game_chat_group = sgNewGroupRect(ui_game_dialog, chatrect, "Chat");
  ui_game_menu_group = sgNewGroupRect(ui_game_dialog, menurect, "Menü");
  ui_game_sound_group = sgNewGroupRect(ui_game_dialog, soundrect, "Sound");
  
  /* Sound UI-erstellen */
  ui_sound(ui_game_sound_group);
  
  /* Widgets in der Chatgruppe */
  ui_game_chat_input = 
    sgNewInputGrouped(ui_game_chat_group, SG_EDGE_BOTTOM, SG_ALIGN_RIGHT, 
                      sgGroup(ui_game_chat_group)->splitted.w, 34, NULL);

  ui_game_chat_console =
    sgNewConsoleGrouped(ui_game_chat_group, SG_EDGE_BOTTOM, SG_ALIGN_RIGHT,  
                        sgGroup(ui_game_chat_group)->splitted.w,
                        sgGroup(ui_game_chat_group)->splitted.h, NULL);

  /* Konsolenpuffer in das Widget laden */
  sgAddConsoleText(ui_game_chat_console, client_buffer);

  ui_game_status(ui_game_status_group);
    
  /* menu stuff */
  ui_game_button_abort = 
    sgNewButtonGrouped(ui_game_menu_group, SG_EDGE_BOTTOM, SG_ALIGN_CENTER,
                       sgGroup(ui_game_menu_group)->body.w, 34, "Zurück");

  ui_game_status_players = 
    sgNewLabelGrouped(ui_game_menu_group, SG_EDGE_TOP, SG_ALIGN_LEFT, 
                      sgGroup(ui_game_menu_group)->body.w, 40, SG_ALIGN_LEFT, 
                      "Spieler Anordnung:");
  
  /* reihenfolge entspricht derjenigen von ORDER */
  ui_game_status_player[2] = 
    sgNewLabelGrouped(ui_game_menu_group, SG_EDGE_TOP,
                      SG_ALIGN_CENTER, sgGroup(ui_game_menu_group)->body.w/2,
                      30, SG_ALIGN_LEFT, "");
  
  ui_game_status_player[3] =
    sgNewLabelGrouped(ui_game_menu_group, SG_EDGE_TOP,
                      SG_ALIGN_LEFT, sgGroup(ui_game_menu_group)->body.w,
                      30, SG_ALIGN_LEFT, "");
  
  ui_game_status_player[1] = 
    sgNewLabelSplitted(ui_game_status_player[3], SG_EDGE_RIGHT,
                       sgGroup(ui_game_menu_group)->body.w/2, SG_ALIGN_LEFT, "");
  
  ui_game_status_player[0] = 
    sgNewLabelGrouped(ui_game_menu_group, SG_EDGE_TOP,
                      SG_ALIGN_CENTER, sgGroup(ui_game_menu_group)->body.w/2,
                      30, SG_ALIGN_LEFT, "");

  /* set fonts */
  sgSetWidgetFont(ui_game_status_player[0], SG_FONT_BOLD, client_font[2]);
  sgSetWidgetFont(ui_game_status_player[1], SG_FONT_BOLD, client_font[2]);
  sgSetWidgetFont(ui_game_status_player[2], SG_FONT_BOLD, client_font[2]);
  sgSetWidgetFont(ui_game_status_player[3], SG_FONT_BOLD, client_font[2]);
     
  
  /*
  ui_game_button_sort2 = sgNewButtonSplitted(ui_game_dialog, ui_game_button_sort1,
                      SG_EDGE_RIGHT, ui_game_group_sort->body.w/2, "Farbe");
  */
//  ui_game_toggle = sgNewToggleGrouped(ui_game_dialog, ui_game_menu_group, SG_EDGE_BOTTOM,
//                                   SG_ALIGN_CENTER, 100, 50, "Sortieren nach Farbe");  
  sgSetWidgetColor(ui_game_status_group, ui_group_color);
  sgSetWidgetColor(ui_game_chat_group, ui_group_color);
  sgSetWidgetColor(ui_game_menu_group, ui_group_color);
  sgSetWidgetColor(ui_game_sound_group, ui_group_color);
  

  sgSetWidgetColor(ui_game_dialog, client_bgcolor);
}

/* -------------------------------------------------------------------------- *
 * Erstellt den Statusbalken                                                  *
 * -------------------------------------------------------------------------- */
void ui_game_status(sgWidget *group)
{
  /* Den grossen Font nicht benutzen */
  sgSetWidgetFonts(group, client_font[0], client_font[0], client_font[2]);
  
  /* Buttons */
  ui_game_button_menu = 
    sgNewButtonGrouped(group, SG_EDGE_LEFT, SG_ALIGN_MIDDLE, 
                       80, sgGroup(group)->splitted.h, "Menü");
  
  ui_game_button_chat =
    sgNewButtonGrouped(group, SG_EDGE_LEFT, SG_ALIGN_MIDDLE, 
                       80, sgGroup(group)->splitted.h, "Chat");
  
  ui_game_button_sound = 
    sgNewButtonGrouped(group, SG_EDGE_LEFT, SG_ALIGN_MIDDLE, 
                       80, sgGroup(group)->splitted.h, "Sound");
  
  /* Drachenbild für den Menu-Button */
  sgLoadButtonImage(ui_game_button_menu, "menu-16.png",
                    SG_EDGE_LEFT, SG_ALIGN_MIDDLE);
  
  /* Sprechblase für den Chat-Button */
  sgLoadButtonImage(ui_game_button_chat, "chat-16.png",
                    SG_EDGE_LEFT, SG_ALIGN_MIDDLE);
  
  /* Ultra-Krasser Harddiskplayer, damit wir im Trend sind :) */
  sgLoadButtonImage(ui_game_button_sound, "sound-16.png",
                    SG_EDGE_LEFT, SG_ALIGN_MIDDLE);
  
  /* Statusanzeige */
  ui_game_status_actor = 
    sgNewLabelGrouped(group, SG_EDGE_LEFT, SG_ALIGN_LEFT,
                      sgTextWidth(ui_game_dialog->font[1], "Am Zug: "), 40,
                      SG_ALIGN_LEFT, "Am Zug: ");
  
  ui_game_actor = 
    sgNewLabelGrouped(group, SG_EDGE_LEFT, SG_ALIGN_LEFT,
                      sgTextWidth(ui_game_dialog->font[1], " ")*25, 40, 
                      SG_ALIGN_LEFT, "");
  
  ui_game_status_points1 = 
    sgNewLabelGrouped(group, SG_EDGE_LEFT, SG_ALIGN_LEFT,
                      sgTextWidth(ui_game_dialog->font[1], "Team 1: "), 40,
                      SG_ALIGN_LEFT, "Team 1: ");
  
  ui_game_status_points1 = 
    sgNewLabelGrouped(group, SG_EDGE_LEFT, SG_ALIGN_LEFT,
                      sgTextWidth(ui_game_dialog->font[1], "0")*5, 40,
                      SG_ALIGN_LEFT, "");
  
  ui_game_status_points2 = 
    sgNewLabelGrouped(group, SG_EDGE_LEFT, SG_ALIGN_LEFT,
                      sgTextWidth(ui_game_dialog->font[1], "Team 2: "), 40,
                      SG_ALIGN_LEFT, "Team 2: ");
  
  ui_game_status_points2 = 
    sgNewLabelGrouped(group, SG_EDGE_LEFT, SG_ALIGN_LEFT,
                      sgTextWidth(ui_game_dialog->font[1], "0")*5, 40,
                      SG_ALIGN_LEFT, "");

  ui_game_button_abandon = 
    sgNewButtonGrouped(group, SG_EDGE_RIGHT, SG_ALIGN_CENTER, 
                       150, 34, "Passen");
}
  
/* -------------------------------------------------------------------------- *
 * Erstellt die Chat-Gruppe                                                   *
 * -------------------------------------------------------------------------- */
void ui_game_chat(sgWidget *group)
{
  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void ui_game_tichu_start(void)
{  
  SDL_Rect rect;
  
  rect.w = 250;
  rect.h = 100;    

  sgAlignRect(&client_rect, &rect, SG_ALIGN_CENTER|SG_ALIGN_MIDDLE);
 
  sgSubBorder(&rect, 4);
  
  ui_game_dialog_tichu = 
    sgNewDialog(client_screen, ui_game_proc, ui_border,
                ui_alpha, ui_game_color, 25);
  
  sgSetWidgetFonts(ui_game_dialog_tichu, client_font[0], 
                   client_font[1], client_font[2]);

  /* tichu dialog */
  rect.x = 0;
  rect.y = 0;
  
  ui_game_group_tichu = 
    sgNewGroupRect(ui_game_dialog_tichu, rect,
                   "Grosses Tichu ansagen?");
  
  ui_game_button_tichu_big = 
    sgNewButtonGrouped(ui_game_group_tichu, SG_EDGE_TOP,
                       SG_ALIGN_MIDDLE|SG_ALIGN_CENTER, 220, 40,
                       "Na klar doch!");

/*  ui_game_button_tichu = sgNewButtonGrouped(ui_game_dialog_tichu, ui_game_group_tichu, SG_EDGE_LEFT,
                                        SG_ALIGN_MIDDLE, rect.w, 50,
                                        "Tichu!");
    
 ui_game_button_tichu_big = sgNewButtonSplitted(ui_game_dialog_tichu, ui_game_button_tichu, SG_EDGE_RIGHT,
                                              rect.w/2, "Grosses Tichu!");
  */
//  sgDrawGroups(ui_game_dialog_tichu);
  sgRedrawDialog(ui_game_dialog_tichu);
  

  game_set(GAME_REDRAW_DIALOG);
  
  return;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void ui_game_tichu_end(void)
{
  
  sgFreeWidget(ui_game_dialog_tichu);
  ui_game_dialog_tichu = NULL;
  
  return;
}

/* -------------------------------------------------------------------------- *
 * 'event handling' des [game end] Dialoges                                   * 
 * -------------------------------------------------------------------------- */
int ui_game_end_proc(sgWidget *widget, sgEvent event)
{
  ui_generic_proc(widget, event);
  
  if(event == SG_EVENT_CLICK)
  {
    /* exit button */
    if(widget == ui_game_end_exit)
    {
      sgClearWidgetStatus(ui_game_end_dialog, SG_RUNNING);
      return 1;
    }
    
  } 
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Startet das [game end] User-Interface                                      *
 * -------------------------------------------------------------------------- */
void ui_game_end(SDL_Surface *screen)
{
  SDL_Rect rect;
  
  /* zustand des client's definieren */
  client_status = CLIENT_GAME_END;
  
  rect.w = 300;
  rect.h = 200;
  
  sgAlignRect(&client_rect, &rect, SG_ALIGN_CENTER|SG_ALIGN_MIDDLE);
  
  /* create the dialog */
  ui_game_end_dialog = 
    sgNewDialog(client_screen, ui_game_end_proc, ui_border,
                ui_alpha, ui_game_end_color, 25);
  
  sgSetWidgetFonts(ui_game_end_dialog, client_font[0], 
                   client_font[1], client_font[2]);
  
  ui_game_end_group = sgNewGroupFull(ui_game_end_dialog, "Spiel beendet!");
  
  /* Labels */
  ui_game_end_team1 =
    sgNewLabelGrouped(ui_game_end_group,
                      SG_EDGE_TOP, SG_ALIGN_LEFT|SG_ALIGN_TOP,
                      sgGroup(ui_game_end_group)->body.w,
                      30, SG_ALIGN_LEFT, "Punkte Team 1:");

  ui_game_end_team2 =
    sgNewLabelGrouped(ui_game_end_group,
                      SG_EDGE_TOP, SG_ALIGN_LEFT|SG_ALIGN_TOP,
                      sgGroup(ui_game_end_group)->body.w,
                      60, SG_ALIGN_LEFT, "Punkte Team 2:");

  ui_game_end_points1 =
    sgNewLabelSplitted(ui_game_end_team1, SG_EDGE_RIGHT,
                       sgGroup(ui_game_end_group)->body.w / 4, SG_ALIGN_LEFT, "");

  ui_game_end_points2 =
    sgNewLabelSplitted(ui_game_end_team2, SG_EDGE_RIGHT, 
                       sgGroup(ui_game_end_group)->body.w / 4, SG_ALIGN_LEFT, "");
  
  /* back button */
  ui_game_end_exit =
    sgNewButtonGrouped(ui_game_end_group,
                       SG_EDGE_BOTTOM, 0,
                       sgGroup(ui_game_end_group)->body.w, 34, "Zurück");


  sgSetDialogTimer(ui_game_end_dialog, (void *)net_poll, NET_INTERVAL, (void *)0);

  sgRunDialog(ui_game_end_dialog, UI_FADE_DELAY);

  sgFreeWidget(ui_game_end_dialog);
}

