/* $Id: game.c,v 1.75 2005/05/23 12:52:38 smoli Exp $
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
 * Das Game-Modul übernimmt die Initialisation und auf Aufräumen der Engine,  *
 * sowie das Zusammenbringen der anderen Engine-Module in der Game-Schleife   *
 * -------------------------------------------------------------------------- */
#include "client.h"
#include "card.h"
#include "stack.h"
#include "dlink.h"
#include "fan.h"
#include "dnd.h"
#include "game.h"
#include "player.h"
#include "net.h"
#include "ui_game.h"

#define GAME_RAISE_SPEED 1000

#define CALC_FPS 1
#define DRAW_FPS 0

int          game_border = 5;
Uint32       game_lastupdate;
double       game_lastfps = 0.0;
double       game_fps = 0.0;
sgColor      game_color;


Uint32              game_ticks;       /* Zeitpunkt der aktuellen Iteration */

/* -------------------------------------------------------------------------- *
 * Lokale Variabeln                                                           *
 * -------------------------------------------------------------------------- */
static int          game_status;      /* Status des Spiels */
//static SDL_Surface *game_bgnd;        /* Hintergrundbild */

/* -------------------------------------------------------------------------- *
 * Initialisiert das Spiel                                                    *
 * -------------------------------------------------------------------------- */
void game_init(void)
{
  /* Benutzerinterface erstellen */
  ui_game(client_screen);
  
  /* Game-Status setzen */
  game_status = GAME_REDRAW|GAME_RUN;
  
  /* Liste der Spieler anfordern */
  net_send("ORDER");
  
  /* Initiale Updatezeit */
  game_lastupdate = SDL_GetTicks();
}

/* -------------------------------------------------------------------------- *
 * Beendet den Spielmodus                                                     *
 * -------------------------------------------------------------------------- */
void game_shutdown(void)
{
  /* Kontrolle über den Konsolenpuffer an das Client-Modul zurückgeben */
  client_free(client_buffer);
  client_buffer = sgGetConsoleText(ui_game_chat_console);

  /* Das Benutzerinterface wieder freigeben */
  sgFreeWidget(ui_game_dialog);
  ui_game_chat_console = NULL;
  ui_game_dialog = NULL;
  
  /* Rien ne va plus */
  game_status = 0;  
}  

/* -------------------------------------------------------------------------- *
 * Setzt Game-Statusflags                                                     *
 * -------------------------------------------------------------------------- */
int game_set(int flags)
{
  /* Müssen wir noch Flags setzen? */
  if((game_status & flags) != flags)
  {
    /* Flags setzen */
    game_status |= flags;

    card_debug(DETAILS, "Set [%s%s%s%s%s%s ]",
               (game_status & GAME_RUN) ? " RUN" : "",
               (game_status & GAME_REDRAW_DIALOG) ? " DIALOG" : "",
               (game_status & GAME_REDRAW_FAN) ? " FAN" : "",
               (game_status & GAME_STACK) ? " STACK" : "",
               (game_status & GAME_PLAYER) ? " PLAYER" : "",
               (game_status & GAME_DND) ? " DND" : "");
    return 1;
  }

  return 0;
}

/* -------------------------------------------------------------------------- *
 * Löscht Game-Statusflags                                                    *
 * -------------------------------------------------------------------------- */
int game_unset(int flags)
{
  /* Müssen wir noch Flags setzen? */
  if(game_status & flags)
  {
    /* Flags löschen */
    game_status &= ~flags;

    card_debug(DETAILS, "Unset [%s%s%s%s%s%s ]",
               (game_status & GAME_RUN) ? " RUN" : "",
               (game_status & GAME_REDRAW_DIALOG) ? " DIALOG" : "",
               (game_status & GAME_REDRAW_FAN) ? " FAN" : "",
               (game_status & GAME_STACK) ? " STACK" : "",
               (game_status & GAME_PLAYER) ? " PLAYER" : "",
               (game_status & GAME_DND) ? " DND" : "");
    return 1;
  }

  return 0;
}

/* -------------------------------------------------------------------------- *
 * Wird per Iteration der Game-Schleife aufgerufen und updated Zeitabhängige  *
 * Dinge (wie z.B. FPS-Counter)                                               *
 * -------------------------------------------------------------------------- */
int game_update(Uint32 diff)
{
  int    ret = 0;
  
  /* Update fps counter */
#if CALC_FPS
  double fps;
  
  if(SDL_GetTicks() - game_lastupdate > 2500)
  {
    fps = ((1000 / diff) + game_fps) / 2;
    
    if((int)fps != (int)game_fps)
    {
      ret = 1;
    }
    
    game_lastfps = game_fps;
    game_fps = fps;
    
    game_lastupdate = SDL_GetTicks();
  }
#endif /* CALC_FPS */
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * Blitted alle Surfaces auf den Screen                                       *
 * -------------------------------------------------------------------------- */
void game_redraw(SDL_Surface *surface)
{
#if DRAW_FPS
  char fps[64];
#endif /* SHOW_FPS */

  if((game_status & GAME_REDRAW) == 0)
    return;

  sgRedrawWidget(ui_game_dialog);
  
  /* Screen löschen */
  SDL_BlitSurface(ui_game_dialog->face.frame, NULL, client_screen, NULL);

  stack_redraw(surface);
  player_redraw(surface);
  fan_redraw(surface);
  dnd_redraw(surface);

  /* Dialog auf Screen zeichnen */
  sgBlitDialog(ui_game_dialog, surface);
//  sgResetDialogRedraw(ui_game_dialog);

  if(ui_game_dialog_tichu)
  {
    sgBlitDialog(ui_game_dialog_tichu, surface);
    //sgResetDialogRedraw(ui_game_dialog_tichu);
  }  
    
#if DRAW_FPS
  sprintf(fps, "%u", (unsigned int)game_fps);
  
  sgDrawTextOutline(client_font_bold, surface, NULL, SG_ALIGN_BOTTOM|SG_ALIGN_LEFT, fps);
#endif /* SHOW_FPS */
  
  SDL_Flip(surface);

  game_status &= ~GAME_REDRAW;
}

/* -------------------------------------------------------------------------- *
 * Hauptloop im Game-Modus                                                    *
 * -------------------------------------------------------------------------- */
int game_loop(SDL_Surface *screen)
{
  SDL_Event event;
  Uint32 diff = 0;
  Uint32 current;
  
  game_ticks = SDL_GetTicks();
                
  while(game_status)
  {
    game_status &= ~GAME_REDRAW;
    
    /* Events abfragen */
    while(SDL_PollEvent(&event))
    {
      /* Ereignisse behandeln */
      if(ui_game_dialog_tichu)
        if(sgHandleDialogEvent(ui_game_dialog_tichu, &event))
          game_status |= GAME_REDRAW_DIALOG;

      if(sgHandleDialogEvent(ui_game_dialog, &event))
        game_status |= GAME_REDRAW_DIALOG;

      if(dnd_event(&event) & DND_REDRAW) game_status |= GAME_DND;
      if(fan_event(&event) & FAN_REDRAW) game_status |= GAME_REDRAW_FAN;
      if(stack_event(&event) & STACK_REDRAW) game_status |= GAME_STACK;
      if(player_event(&event) & PLAYER_REDRAW) game_status |= GAME_PLAYER;
      
      /* Nach Quit-Event checken */
      switch(event.type)
      {
        case SDL_QUIT: 
          game_status = 0; break;
        case SDL_KEYUP: 
          if(event.key.keysym.sym == SDLK_ESCAPE) game_status = 0; break;
        default: break;
      }
    }
    
    /* Wenn Zeit vergangen ist seit der letzten 
       Iteration dann führen wir Bewegungen aus */
    if(diff)
    {
#ifdef NEVER_DO_THAT_SILLY_THING_AGAIN
      client_log(LOG_DEBUG, 
                 "+++ GAME: Letzte Iteration vor %umsecs, Positionen aktualisieren...", diff);
#endif /* NEVER_DO_THAT_SILLY_THING_AGAIN */

      /* Positionen updaten */
      if(ui_game_dialog_tichu)
      {        
        if(ui_game_dialog_tichu->status & SG_REDRAW_NEEDED)
          game_status |= GAME_REDRAW;
      }
      
      if(ui_game_dialog->status & SG_REDRAW_NEEDED)
        game_status |= GAME_REDRAW;
      
      if(game_update(diff) & GAME_REDRAW) game_status |= GAME_REDRAW_DIALOG;
      if(dnd_update(diff) & DND_REDRAW) game_status |= GAME_DND;
      if(fan_update(diff) & FAN_REDRAW) game_status |= GAME_REDRAW_FAN;
      if(stack_update(diff) & STACK_REDRAW) game_status |= GAME_STACK;
      if(player_update(diff) & PLAYER_REDRAW) game_status |= GAME_PLAYER;      
    }

    /* Zeichnen oder pausieren */
    if(diff && (game_status & GAME_REDRAW))
    {
      /* Info ausgeben */
      game_debug(INFO, "Redraw [%s%s%s%s%s ] @ %.1ffps",
                 (game_status & GAME_REDRAW_DIALOG) ? " DIALOG" : "",
                 (game_status & GAME_DND) ? " DND" : "", 
                 (game_status & GAME_REDRAW_FAN) ? " FAN" : "", 
                 (game_status & GAME_PLAYER) ? " PLAYER" : "",
                 (game_status & GAME_STACK) ? " STACK" : "",
                 (float)1000 / (float)diff);

      game_redraw(screen);
      net_poll(0, 0);
    }
    else
    {
      game_debug(DETAILS, "Kein Redraw, nur Net checken...");

      net_poll(0, 1000 / GAME_FPS);
    }

    /* Dauer des Zeichnens ausrechnen */
    current = SDL_GetTicks();
    diff = SDL_GetTicks() - game_ticks;
    game_ticks = current;
  }
  
  return 0;
}

