/* $Id: ui_sound.c,v 1.5 2005/05/22 02:44:34 smoli Exp $
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

#include "client.h"
#include "sound.h"
#include "net.h"
#include "ini.h"
#include "ui_sound.h"

/* -------------------------------------------------------------------------- *
 * GUI Objekte                                                                *
 * -------------------------------------------------------------------------- */
sgWidget    *ui_sound_music;
sgWidget    *ui_sound_fx;
sgWidget    *ui_sound_list;
sgWidget    *ui_sound_next;
sgWidget    *ui_sound_prev;
sgWidget    *ui_sound_play;
sgWidget    *ui_sound_stop;
sgWidget    *ui_sound_title;
sgWidget    *ui_sound_info;
sgWidget    *ui_sound_artist;
sgWidget    *ui_sound_time;
Uint32       ui_sound_elapsed;
Uint32       ui_sound_redraw;

/* -------------------------------------------------------------------------- *
 * Piktogramme für die Knöpfe                                                 *
 * -------------------------------------------------------------------------- */
#define I 0x1   /* invert */
#define D 0x80  /* darken */
#define L 0x7f  /* lighten */

static sgPict ui_sound_pict_play =
{
  16, 16,
  {
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,D,D,D,D,0,D,D,D,0,0,0,0,0,0,
     0,0,L,I,I,D,0,L,I,I,D,0,0,0,0,0,
     0,0,L,I,I,D,0,L,I,I,I,D,0,0,0,0,
     0,0,L,I,I,D,0,L,I,I,I,I,D,0,0,0,
     0,0,L,I,I,D,0,L,I,I,I,I,I,D,0,0,
     0,0,L,I,I,D,0,L,I,I,I,I,I,I,D,0,
     0,0,L,I,I,D,0,L,I,I,I,I,I,I,L,0,
     0,0,L,I,I,D,0,L,I,I,I,I,I,L,0,0,
     0,0,L,I,I,D,0,L,I,I,I,I,L,0,0,0,
     0,0,L,I,I,D,0,L,I,I,I,L,0,0,0,0,
     0,0,L,I,I,D,0,L,I,I,L,0,0,0,0,0,
     0,0,L,L,L,L,0,L,L,L,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  }
};

static sgPict ui_sound_pict_stop =
{
  16, 16,
  {
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,D,D,D,D,D,D,D,D,D,D,D,D,0,0,
     0,0,L,I,I,I,I,I,I,I,I,I,I,D,0,0,
     0,0,L,I,I,I,I,I,I,I,I,I,I,D,0,0,
     0,0,L,I,I,I,I,I,I,I,I,I,I,D,0,0,
     0,0,L,I,I,I,I,I,I,I,I,I,I,D,0,0,
     0,0,L,I,I,I,I,I,I,I,I,I,I,D,0,0,
     0,0,L,I,I,I,I,I,I,I,I,I,I,D,0,0,
     0,0,L,I,I,I,I,I,I,I,I,I,I,D,0,0,
     0,0,L,I,I,I,I,I,I,I,I,I,I,D,0,0,
     0,0,L,I,I,I,I,I,I,I,I,I,I,D,0,0,
     0,0,L,I,I,I,I,I,I,I,I,I,I,D,0,0,
     0,0,L,L,L,L,L,L,L,L,L,L,L,D,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  }
};

static sgPict ui_sound_pict_next =
{
  16, 16,
  {
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,D,D,D,0,0,0,0,0,D,D,D,D,0,0,
     0,0,L,I,I,D,0,0,0,0,L,I,I,D,0,0,
     0,0,L,I,I,I,D,0,0,0,L,I,I,D,0,0,
     0,0,L,I,I,I,I,D,0,0,L,I,I,D,0,0,
     0,0,L,I,I,I,I,I,D,0,L,I,I,D,0,0,
     0,0,L,I,I,I,I,I,I,D,L,I,I,D,0,0,
     0,0,L,I,I,I,I,I,I,L,L,I,I,D,0,0,
     0,0,L,I,I,I,I,I,L,0,L,I,I,D,0,0,
     0,0,L,I,I,I,I,L,0,0,L,I,I,D,0,0,
     0,0,L,I,I,I,L,0,0,0,L,I,I,D,0,0,
     0,0,L,I,I,L,0,0,0,0,L,I,I,D,0,0,
     0,0,L,L,L,0,0,0,0,0,L,L,L,D,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  }
};

static sgPict ui_sound_pict_prev =
{
  16, 16,
  {
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,D,D,D,D,0,0,0,0,0,D,D,D,0,0,
     0,0,L,I,I,D,0,0,0,0,D,I,I,D,0,0,
     0,0,L,I,I,D,0,0,0,D,I,I,I,D,0,0,
     0,0,L,I,I,D,0,0,D,I,I,I,I,D,0,0,
     0,0,L,I,I,D,0,D,I,I,I,I,I,D,0,0,
     0,0,L,I,I,D,D,I,I,I,I,I,I,D,0,0,
     0,0,L,I,I,D,L,I,I,I,I,I,I,D,0,0,
     0,0,L,I,I,D,0,L,I,I,I,I,I,D,0,0,
     0,0,L,I,I,D,0,0,L,I,I,I,I,D,0,0,
     0,0,L,I,I,D,0,0,0,L,I,I,I,D,0,0,
     0,0,L,I,I,D,0,0,0,0,L,I,I,D,0,0,
     0,0,L,L,L,D,0,0,0,0,0,L,L,D,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  }
};

/* -------------------------------------------------------------------------- *
 * Ereignissbehandlung des Soundplayers                                       *
 * -------------------------------------------------------------------------- */
int ui_sound_proc(sgWidget *widget, sgEvent event)
{
  ui_generic_proc(widget, event);
  
  /* Button Klicks auf dem Soundtrack-Player checken */
  if(event == SG_EVENT_CLICK)
  {
    if(widget == ui_sound_prev)
    {
      sgListboxItem *prev =
        (sgListboxItem *)sgSelectedListboxItem(ui_sound_list)->node.prev;
      
      if(prev)
      {       
        ui_sound_select(sgListbox(ui_sound_list)->select - 1);
        sgSelectListboxIndex(ui_sound_list, sgListbox(ui_sound_list)->select - 1);
      }
    }
    else if(widget == ui_sound_next)
    {
      sgListboxItem *next =
        (sgListboxItem *)sgSelectedListboxItem(ui_sound_list)->node.next;
      
      if(next)
      {
        ui_sound_select(sgListbox(ui_sound_list)->select + 1);
        sgSelectListboxIndex(ui_sound_list, sgListbox(ui_sound_list)->select + 1);
      }
    }
    else if(widget == ui_sound_play)
    {
      ui_sound_select(sgSelectedListboxIndex(ui_sound_list));
    }
    else if(widget == ui_sound_stop)
    {
      sound_mus_pause();
    }
  }
  
  /* Wenn die Volumen-Schieber geändert wurden, 
     dann stellen wir den Mixer entsprechend ein */
  if(event == SG_EVENT_CHANGE)
  {
    if(widget == ui_sound_music)
      sound_mus_setvol(sgGetAdjustValue(widget, NULL) * 255 / 100);

    if(widget == ui_sound_fx)
      sound_fx_setvol(sgGetAdjustValue(widget, NULL) * 255 / 100);
  }

  return 0;  
}

/* -------------------------------------------------------------------------- *
 * Wählt einen Track aus und spielt ihn ab                                    *
 * -------------------------------------------------------------------------- */
void ui_sound_select(int i)
{
  struct sound_mus *music;
  sgListboxItem *item;
  
  if((item = sgGetListboxItem(ui_sound_list, i)))
  {
    music = item->value;
  
    /* Infos zum aktuellen Track setzen */
    sgSetWidgetCaption(ui_sound_title, music->title);
    sgSetWidgetCaption(ui_sound_info, music->infostr);
    sgSetWidgetCaption(ui_sound_artist, music->artist);
    
    /* Wiedergabe starten */
    sound_mus_play(music);
    
    sgSelectListboxItem(ui_sound_list, item);
  }
}

/* -------------------------------------------------------------------------- *
 * Zeigt den Zeitzähler und so an                                             *
 * -------------------------------------------------------------------------- */
void ui_sound_show(void)
{
  char timebuf[32];
  Uint32 seconds;
  
  seconds = ui_sound_elapsed / 1000;
  
  snprintf(timebuf, sizeof(timebuf), "%02u:%02u",
           (seconds % 3600) / 60,
           (seconds % 60));
  
  sgSetWidgetCaption(ui_sound_time, timebuf);
}

/* -------------------------------------------------------------------------- *
 * Updatet den Zeitzähler                                                     *
 * -------------------------------------------------------------------------- */
void ui_sound_update(Uint32 delay)
{
  Uint32 now = SDL_GetTicks();
  
  sound_update();
  
  /* Huh? Kein Musikstück? */
  if(sound_music == NULL)
    return;
  
  /* Im Pause Modus den neuen "Anfang" berechnen */
  if(sound_status & SOUND_PAUSED)
  {    
    sound_music->start = now - ui_sound_elapsed;
  }
  /* Wenn wir am Abspielen sind, dann den Zähler inkrementieren */
  else if(sound_status & SOUND_PLAYING)
  {
    Uint32 elapsed;
    
    elapsed = now - sound_music->start;
    
    /* Hat Sekunde geändert? */
    if(elapsed / 1000 != ui_sound_elapsed / 1000)
      ui_sound_redraw++;

    ui_sound_elapsed = elapsed;
  }

  /* Neu anzeigen falls nötig */
  if(ui_sound_redraw)
  {
    ui_sound_show();
    ui_sound_redraw++;
  }
}

/* -------------------------------------------------------------------------- *
 * Lädt alle Konfigurationsdaten in die Widgets für ui_sound                 *
 * -------------------------------------------------------------------------- */
void ui_sound_load(void)
{
  /* Stuff für den Playlist loop */
  struct sound_mus *mus;
  int i = 0;
  
  /* Aktuelle Lautstärke setzen */
  sgSetAdjustValue(ui_sound_music, sound_mus_getvol() * 100 / 255);
  sgSetAdjustValue(ui_sound_fx, sound_fx_getvol() * 100 / 255);
  
  /* Alle Soundtracks in die Playliste laden */
  dlink_foreach(&sound_playlist, mus)
  {
    char buf[256];
    
    snprintf(buf, sizeof(buf), "%i. %s", ++i, mus->title);
    sgAddListboxItem(ui_sound_list, buf, (void *)mus);
  }  
}

/* -------------------------------------------------------------------------- *
 * Speichert alle Konfigurationsdaten aus den Widgets von ui_sound           *
 * -------------------------------------------------------------------------- */
void ui_sound_save(void)
{
  sound_save(client_ini);
}

/* -------------------------------------------------------------------------- *
 * Erstellt die Widgets des Soundplayers                                      *
 * -------------------------------------------------------------------------- */
void ui_sound(sgWidget *group)
{
  /* Ereignissbehandlungsroutine für den Player setzen */
  sgSetWidgetProc(group, ui_sound_proc);
  
  /* Labels für Titel/Info */
  ui_sound_title = sgNewLabelGrouped(group, SG_EDGE_TOP, SG_ALIGN_LEFT,
                                     sgGroup(group)->splitted.w, UI_LABEL_HEIGHT,
                                     SG_ALIGN_LEFT, NULL);
  ui_sound_info = sgNewLabelGrouped(group, SG_EDGE_TOP, SG_ALIGN_LEFT,
                                    sgGroup(group)->splitted.w, UI_LABEL_HEIGHT,
                                    SG_ALIGN_LEFT, NULL);
  
  /* Eine Listbox für die Playliste erstellen */
  ui_sound_list = sgNewListboxGrouped(group, SG_EDGE_RIGHT, SG_ALIGN_TOP,
                                      sgGroup(group)->splitted.w * 9 / 18,
                                      sgGroup(group)->splitted.h,
                                      0, "Playlist");

  /* Label für Artist */
  ui_sound_artist = sgNewLabelGrouped(group, SG_EDGE_TOP, SG_ALIGN_LEFT,
                                      sgGroup(group)->splitted.w, UI_LABEL_HEIGHT, 
                                      SG_ALIGN_LEFT, NULL);
  
  /* Regler für die Lautstärke */
  ui_sound_fx = sgNewAdjustGrouped(group, SG_EDGE_BOTTOM, SG_ALIGN_LEFT,
                                   sgGroup(group)->splitted.w, UI_ADJUST_HEIGHT,
                                   0, 100, "F/X");
  ui_sound_music = sgNewAdjustGrouped(group, SG_EDGE_BOTTOM, SG_ALIGN_LEFT,
                                      sgGroup(group)->splitted.w, UI_ADJUST_HEIGHT,
                                      0, 100, "Musik");
  
  /* Bedienung des Players */
  ui_sound_stop = sgNewButtonGrouped(group, SG_EDGE_BOTTOM, SG_ALIGN_LEFT,
                                     sgGroup(group)->splitted.w,
                                     32 * 2, NULL);
  
  ui_sound_prev = sgNewButtonSplitted(ui_sound_stop, SG_EDGE_BOTTOM,
                                      ui_sound_stop->rect.h / 2, NULL);
  
  ui_sound_play = sgNewButtonSplitted(ui_sound_stop, SG_EDGE_RIGHT,
                                      ui_sound_stop->rect.w / 3, NULL);
  ui_sound_next = sgNewButtonSplitted(ui_sound_prev, SG_EDGE_RIGHT,
                                      ui_sound_prev->rect.w / 2, NULL);

  sgSetAdjustFormat(ui_sound_music, "%.0lf%%");
  sgSetAdjustFormat(ui_sound_fx, "%.0lf%%");
  
  /* Zeitanzeige */
  ui_sound_time = sgNewLabelGrouped(group, SG_EDGE_TOP, SG_ALIGN_LEFT,
                                    sgGroup(group)->splitted.w, sgGroup(group)->splitted.h,
                                    SG_ALIGN_CENTER|SG_ALIGN_MIDDLE, "Zeit");

  sgSetWidgetFont(ui_sound_time, SG_FONT_NORMAL, ui_sound_time->font[SG_FONT_BOLD]);
  /*
  sound_mus_setvol(sgGetAdjustValue(ui_sound_music, NULL));
  sound_fx_setvol(sgGetAdjustValue(ui_sound_fx, NULL));
  */
  
  sgNewLabelSplitted(ui_sound_title, SG_EDGE_LEFT, 56,
                     SG_ALIGN_RIGHT, "Titel:");
  
  sgNewLabelSplitted(ui_sound_info, SG_EDGE_LEFT, 56,
                     SG_ALIGN_RIGHT, "Info:");
  
  sgNewLabelSplitted(ui_sound_artist, SG_EDGE_LEFT, 56,
                     SG_ALIGN_RIGHT, "Artist:");
  
  sgSetButtonPict(ui_sound_stop, &ui_sound_pict_stop);  
  sgSetButtonPict(ui_sound_play, &ui_sound_pict_play);
  sgSetButtonPict(ui_sound_next, &ui_sound_pict_next);  
  sgSetButtonPict(ui_sound_prev, &ui_sound_pict_prev);
  
  /* Kein Sound verfügbar? -> Sound-UI disablen */
  if(!(sound_status & SOUND_AVAIL))
    sgDisableWidget(group);
}

