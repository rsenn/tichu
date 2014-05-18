/* $Id: ui.c,v 1.144 2005/05/23 02:45:24 smoli Exp $
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>

/* POSIX directory stuff */  
#ifndef _WIN32
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#endif

#include <libsgui/sgui.h>
#include <libsgui/png.h>

#include "sound.h"
#include "ini.h"
#include "net.h"

#include "fan.h"
#include "dnd.h"
#include "action.h"
#include "stack.h"
#include "card.h"
#include "player.h"

#include "client.h"
#include "game.h"
#include "ui.h"


/* -------------------------------------------------------------------------- *
 * Globale Variablen                                                          *
 * -------------------------------------------------------------------------- */
int         ui_border = 4;
Uint8       ui_alpha = 128;
int         ui_fade_delay = 512;
sgColor     ui_group_color = { .r=0x7f,.g=0x7f,.b=0x7f };

struct sound_fx *ui_fx_button;
struct sound_fx *ui_fx_zap;
struct sound_fx *ui_fx_scroll;
struct sound_fx *ui_fx_select;

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */

// GAME_END 
sgColor   ui_game_end_color = { .r=0xb2,.g=0xfb,.b=0x0d };
sgWidget *ui_game_end_dialog = NULL;
sgWidget *ui_game_end_group;
sgWidget *ui_game_end_team1;
sgWidget *ui_game_end_team2;
sgWidget *ui_game_end_points1;
sgWidget *ui_game_end_points2;
sgWidget *ui_game_end_exit;

/* -------------------------------------------------------------------------- *
 * Lädt die Sounds für den Client                                             *
 * -------------------------------------------------------------------------- */
int ui_loadsounds(void)
{
  ui_fx_zap = sound_fx_open("zap.wav");
  ui_fx_button = sound_fx_open("rs.wav");
  ui_fx_scroll = sound_fx_open("click.wav");
  ui_fx_select = sound_fx_open("select.wav");

  return 0;
}
/*
int ui_loadmusic(void)
{
  DIR           *d;
  struct dirent *de;
  size_t         len;
  char           filename[PATH_MAX];
   
  if((d = opendir("sounds")))
  {              
    while((de = readdir(d)))
    {            
      if((len = strlen(de->d_name)) > 3)
      {                      
        if(!strcmp(&de->d_name[len - 3], ".xm"))
        {                    
          strcpy(filename, de->d_name);
          
          ui_debug(INFO, "Lade Soundtrack: %s", filename);
          
          dlink_add_head(&client_playlist, dlink_node_new(), strdup(filename));
          sound_mus_load(filename);
        }          
      }      
    }
  }  
  return 0;
}*/


/* -------------------------------------------------------------------------- *
 * Allgemeines 'event handling'                                               *
 * -------------------------------------------------------------------------- */
int ui_generic_proc(sgWidget *widget, sgEvent event)
{
  if(event == SG_BUTTON_DOWN)
    sound_fx_play(ui_fx_button);
  
  if(event == SG_SCROLL || event == SG_EVENT_CHANGE)
    sound_fx_play(ui_fx_scroll);
  
  if(event == SG_SEL_CHANGE)
    sound_fx_play(ui_fx_select);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Testet das Game User-Interface (temporär)                                  *
 * -------------------------------------------------------------------------- */
int ui_test(SDL_Surface *screen)
{
  struct color tint;
  
  /* zustand des client's definieren */
  client_status = CLIENT_TEST;

  fan_configure(client_ini);
  card_configure(client_ini);

  fan_init();
  dnd_init();
  stack_init();
  player_init();
  game_init();
  
  player_new("codemutz", 0, card_tint);
  
  tint.r = 0xff; tint.g = 0x80; tint.b = 0x00;
  
  player_new("testmutz", 1, tint);
  
  tint.r = 0xff; tint.g = 0x00; tint.b = 0x00;
  
  player_new("testdude", 2, tint);
  
  tint.r = 0x80; tint.g = 0xff; tint.b = 0x00;
  
  player_new("testbabe", 3, tint);
  
  fan_add(card_new("j5"));
  fan_add(card_new("p5"));
  fan_add(card_new("s5"));
  fan_add(card_new("t5"));
  fan_add(card_new("j6"));
  fan_add(card_new("p6"));
  fan_add(card_new("s6"));
  fan_add(card_new("t6"));
  fan_add(card_new("j7"));
  fan_add(card_new("p7"));
  fan_add(card_new("s7"));
  fan_add(card_new("t7"));
  fan_add(card_new("j8"));
/*  fan_add(card_new("p8"));
  fan_add(card_new("s8"));
  fan_add(card_new("t8"));
  fan_add(card_new("j9"));
  fan_add(card_new("p9"));
  fan_add(card_new("s9"));
  fan_add(card_new("t9"));
  fan_add(card_new("j0"));
  fan_add(card_new("p0"));
  fan_add(card_new("s0"));
  fan_add(card_new("t0"));
  fan_add(card_new("jj"));
  fan_add(card_new("pj"));
  fan_add(card_new("sj"));
  fan_add(card_new("tj"));*/
  fan_start();
  player_start();
  
  game_loop(screen);
  
  player_shutdown();
  game_shutdown();
  fan_shutdown();
  dnd_shutdown();
  stack_shutdown();
  
  return 0;
}
