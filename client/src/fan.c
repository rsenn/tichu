/* $Id: fan.c,v 1.88 2005/05/21 08:27:20 smoli Exp $
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

#include <math.h>
#include <SDL.h>

/* -------------------------------------------------------------------------- *
 * Das FAN Modul ist für die Berechnung, das Bewegen und das Anzeigen des     *
 * Kartenfächers zuständig                                                    *
 * -------------------------------------------------------------------------- */
#include "fan.h"
#include "ini.h"
#include "dnd.h"
#include "stack.h"
#include "client.h"

/* -------------------------------------------------------------------------- *
 * Modulstatus                                                                *
 * -------------------------------------------------------------------------- */
struct list       fan_list;              /* Liste aller Karten im Fächer */
struct card      *fan_card;              /* Gewählte Karte */
struct card      *fan_previous;          /* Letzte gewählte Karte */
int               fan_status;
int               fan_target;

/* -------------------------------------------------------------------------- *
 * Geometrie                                                                  *
 * -------------------------------------------------------------------------- */
uint32_t          fan_radius[2];         /* Innerer und äusserer Radius */
float             fan_angle;             /* Winkel zwischen zwei Karten */
struct position   fan_center;            /* Zentrum des Kreisbogens */


uint16_t          fan_wave[FAN_WAVE_WIDTH];
  
/* -------------------------------------------------------------------------- *
 * Konfiguration für den Fächer                                               *
 * -------------------------------------------------------------------------- */

/* fan_config.x            Sollte möglicherweise immer Bildschirmbreite / 2 sein */
#define FAN_DEFAULT_X      (CLIENT_WIDTH / 2)

/* fan_config.y            Sollte zwischen Kartenhöhe / 2 und Bildschirmhöhe -
                           (Kartenhöhe / 2) sein */
#define FAN_DEFAULT_Y      (CLIENT_HEIGHT * 3 / 5)

/* fan_config.raise        Sollte ein Wert von 0 bis Kartenhöhe sein */
#define FAN_DEFAULT_RAISE  25
 
/* fan_config.space        Pixelabstand zwischen den Karten */
#define FAN_DEFAULT_SPACE  32

/* fan_config.bend         Krümmungsquotient des Fächers.
                           0.0 -> keine Krümmung, 1.0 -> maximale */
#define FAN_DEFAULT_BEND   0.5

/* fan_config.mirror       Fächer spiegeln für Linkshänder */
#define FAN_DEFAULT_MIRROR 0  

struct fan_config fan_config;

/* -------------------------------------------------------------------------- *
 * Rechnet den "Wellenberg" auf die Kartenradien, ausgehend von der           *
 * angegebenen Karte und in beide Richtungen gehend                           *
 * -------------------------------------------------------------------------- */
#if BLAHBLAHBLAHBLAHBLAHBLAHBLAHBLAH
static void fan_wavecard(struct card *card)
{
  struct node *forward;
  struct node *backward;
  int i = 0;
  int amplitude = 0;
  
  /* Zuerst alle Karten wieder nach unten schicken, im Falle keine Karte aus-
     gewählt ist und für die Karten die nicht von der Wellenbreite abgedeckt
     werden */
  dlink_foreach(&fan_list, forward)
    card_update(forward->data, CARD_FAN, 0);

  if(!card)
    return;
  
  forward = &card->is[CARD_FAN].node;
  backward = &card->is[CARD_FAN].node;
  
  do
  {
    amplitude = i < FAN_WAVE_WIDTH ? fan_wave[i] : 0;
    
    if(forward)
    {
      if(card_update(forward->data, CARD_FAN, amplitude))
        fan_set(FAN_REDRAW);
      
      forward = forward->next;
    }
    
    if(i && backward)
    {
      if(card_update(backward->data, CARD_FAN, amplitude))
        fan_set(FAN_REDRAW);
        
      backward = backward->prev;
    }

    i++;
    
  } while((forward || backward));
}
#endif
/* -------------------------------------------------------------------------- *
 * Initialisiert den Fächer                                                   *
 * -------------------------------------------------------------------------- */
void fan_init(void)
{
  int i;
  
  dlink_list_zero(&fan_list);

  fan_previous = NULL;
  fan_card = 0;
//  fan_rect = client_rect;
//  fan_tint = tint;
  
  /* Einen "Wellenberg" generieren, welcher um die Karten hevorzuheben.
     Dieser wird auf den Radius der gehiliteten Karte und den darum liegenden
     angewandt. */
  for(i = 0; i < FAN_WAVE_SAMPLES; i++)
  {
    fan_wave[i] = (int)((1 - sin((M_PI / 2 * i / FAN_WAVE_SAMPLES))) * 65535);
    
    fan_debug(DETAILS, "Welle[%u] = %i", i, fan_wave[i]);
  }
  
  
  /* Am Anfang alles neu zeichnen */
  fan_set(FAN_LOCKED|FAN_HIDDEN);
  fan_target = CARD_PLAYER;
  
  fan_geometry();
}

/* -------------------------------------------------------------------------- *
 * Gibt die Fächer-Ressourcen wieder frei                                     *
 * -------------------------------------------------------------------------- */
void fan_shutdown(void)
{
  struct card *card;
  struct node *node;
  
  dlink_foreach_data(&fan_list, node, card)
    card_clean(card);
  
  dlink_list_zero(&fan_list);
}  

/* -------------------------------------------------------------------------- *
 * Liest die Konfiguration des Fächers aus der [fan] Sektion der angegebenen  *
 * ini-Datei                                                                  *
 * -------------------------------------------------------------------------- */
void fan_configure(struct ini *ini)
{
  /* Zuerst mal alles aus dem .ini File laden */
  ini_section(ini, "fan");
  
  fan_config.x = ini_getlong_default(ini, "x", FAN_DEFAULT_X);
  fan_config.y = ini_getlong_default(ini, "y", FAN_DEFAULT_Y);
  
  fan_config.raise = ini_getulong_default(ini, "raise", FAN_DEFAULT_RAISE);
  fan_config.spacing = ini_getulong_default(ini, "spacing", FAN_DEFAULT_SPACE);
  
  fan_config.bend = ini_getdouble_default(ini, "bend", FAN_DEFAULT_BEND);
  fan_config.mirror = ini_getlong_default(ini, "mirror", FAN_DEFAULT_MIRROR);
  
  /* Dann mal überprüfen ob wir "gesunde" Werte haben, 
     andernfalls Defaults setzen */
  if(fan_config.x < 0 || fan_config.x >= client_config.width)
    ini_putlong(ini, "x", (fan_config.x = FAN_DEFAULT_X));
  
  if(fan_config.y < CARD_HEIGHT / 2 ||
     fan_config.y >= client_config.height - CARD_HEIGHT / 2)
    ini_putlong(ini, "y", (fan_config.y = FAN_DEFAULT_Y));
  
  if(fan_config.raise > CARD_HEIGHT)
    ini_putulong(ini, "raise", (fan_config.raise = FAN_DEFAULT_RAISE));
  
  if(fan_config.spacing > CARD_WIDTH)
    ini_putulong(ini, "spacing", (fan_config.spacing = FAN_DEFAULT_SPACE));

  /* Biegungsfaktor sollte nie 0.0 (Ein total gerader Fächer ist nicht
     möglich) und auch nicht zu klein sein */
  if(fan_config.bend < 0.01 || fan_config.bend > 1.0)
    ini_putdouble(ini, "bend", (fan_config.bend = FAN_DEFAULT_BEND));  
}

/* -------------------------------------------------------------------------- *
 * Berechnet die Fächergeometrie                                              *
 * -------------------------------------------------------------------------- */
void fan_geometry(void)
{
  /* Die Radien errechnen sich aus dem Biegungsfaktor */
  fan_radius[0] = (float)(client_config.height - fan_config.y) / fan_config.bend * 0.5;
  fan_radius[1] = fan_radius[0] + fan_config.raise;
  
  /* Dann wissen wir auch das Zentrum des Kreisbogens */
  fan_center.x = fan_config.x;
  fan_center.y = fan_config.y + fan_radius[0];
  
  /* Aus dem gewünschten Pixelabstand gewinnen wir einen 
     Winkel mithilfe der umgekehrten Umfangsformel */
  fan_angle = 180.0 * (float)fan_config.spacing / ((float)fan_radius[0] * M_PI);
  
  /* Umkehrung für Linkshänder (lol) */
  if(fan_config.mirror)
    fan_angle *= -1;
}

/* -------------------------------------------------------------------------- *
 * Rendert eine Vorschau des Fächers auf eine Surface                         *
 * -------------------------------------------------------------------------- */
SDL_Surface *fan_preview(SDL_Surface *bgnd, SDL_Rect rect, struct list *cards)
{
  SDL_Surface *fan;
  SDL_Surface *preview;
  float zoom;
  struct node *node;
  
  fan_init();
  
  /* Karten in den Fächer laden */
  dlink_foreach(cards, node)
    fan_add(node->data);
    
  /* Fächer berechnen */
  fan_calc();
  fan_start();

  /* Fächer zeichnen */
  fan = SDL_CreateRGBSurface(SDL_SWSURFACE, client_rect.w, client_rect.h,
                             32, RMASK, GMASK, BMASK, AMASK);

  SDL_FillRect(fan, NULL, RMASK|GMASK|BMASK|AMASK);
  SDL_BlitSurface(bgnd, NULL, fan, NULL);
  
  fan_redraw(fan);
  fan_shutdown();
  
  zoom = (double)rect.w / (double)client_rect.w;
  
  preview = gfxutil_rotozoom(fan, 0.0, zoom, 255);
  SDL_FreeSurface(fan);

  return preview;
}

/* -------------------------------------------------------------------------- *
 * Setzt Fächer-Statusflags                                                   *
 * -------------------------------------------------------------------------- */
int fan_set(int flags)
{
  /* Müssen wir noch flags setzen? */
  if((fan_status & flags) != flags)
  {
    /* Flags die gesetzt werden und endgültige Flags anzeigen */
    fan_debug(VERBOSE, "Set [%s%s%s%s ] (%s%s%s%s )",
              ((~fan_status) & flags & FAN_REDRAW) ? " REDRAW" : "",
              ((~fan_status) & flags & FAN_LOCKED) ? " LOCKED" : "",
              ((~fan_status) & flags & FAN_HIDDEN) ? " HIDDEN" : "",
              ((~fan_status) & flags & FAN_DRAG) ? " DRAG" : "",
              ((fan_status | flags) & FAN_REDRAW) ? " REDRAW" : "",
              ((fan_status | flags) & FAN_LOCKED) ? " LOCKED" : "",
              ((fan_status | flags) & FAN_HIDDEN) ? " HIDDEN" : "",
              ((fan_status | flags) & FAN_DRAG) ? " DRAG" : "");
    
    /* Flags setzen */
    fan_status |= flags;

    return 1;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Löscht Fächer-Statusflags                                                  *
 * -------------------------------------------------------------------------- */
int fan_unset(int flags)
{
  /* Müssen wir noch Flags löschen? */
  if(fan_status & flags)
  {
    /* Flags die gelöscht werden und endgültige Flags anzeigen */
    fan_debug(VERBOSE, "Unset [%s%s%s%s ] (%s%s%s%s )",
              (fan_status & flags & FAN_REDRAW) ? " REDRAW" : "",
              (fan_status & flags & FAN_LOCKED) ? " LOCKED" : "",
              (fan_status & flags & FAN_HIDDEN) ? " HIDDEN" : "",
              (fan_status & flags & FAN_DRAG) ? " DRAG" : "",
              (fan_status & (~flags) & FAN_REDRAW) ? " REDRAW" : "",
              (fan_status & (~flags) & FAN_LOCKED) ? " LOCKED" : "",
              (fan_status & (~flags) & FAN_HIDDEN) ? " HIDDEN" : "",
              (fan_status & (~flags) & FAN_DRAG) ? " DRAG" : "");

    /* Flags löschen */
    fan_status &= ~flags;
    
    return 1;
  }
  
  return 0;
}  
  
/* -------------------------------------------------------------------------- *
 * Fügt eine Karte hinzu                                                      *
 * -------------------------------------------------------------------------- */
void fan_add(struct card *card)
{
  /* Vielleicht baut der Server Scheisse: */
  if(dlink_find(&fan_list, card))
    fan_log(ERROR, "Karte %s bereits im Fächer!", card->name);
  
  /* Am Ende der Liste hinzufügen */
  dlink_add_tail(&fan_list, &card->is[CARD_FAN].node, card);
}

/* -------------------------------------------------------------------------- *
 * Nimmt eine Karte weg                                                       *
 * -------------------------------------------------------------------------- */
void fan_remove(struct card *card)
{
  dlink_delete(&fan_list, &card->is[CARD_FAN].node);
  fan_calc();
  fan_set(FAN_REDRAW);
}
  
/* -------------------------------------------------------------------------- *
 * Startet mit dem Anzeigen des Fächers, nachdem die initialen Karten hinzu-  *
 * gefügt wurden                                                              *
 * -------------------------------------------------------------------------- */
void fan_start(void)
{
  struct card *card;
  struct node *node;
  
  fan_calc();
  
  /* Anfangspositionen und Winkel setzen */
  dlink_foreach_data(&fan_list, node, card)
  {
    /* Initialier Winkel ist der endgültige Winkel */
    card->is[CARD_FAN].rotozoom.start = 
    card->is[CARD_FAN].rotozoom.now = card->is[CARD_FAN].rotozoom.end;
    
    /* Vom Stapel aus Richtung Fächer schieben */
    card->is[CARD_FAN].move.start = stack_pos;
    
    /* Position fürs Preview setzen */
    card->is[CARD_FAN].move.now = card->is[CARD_FAN].move.end;
  }
  
  fan_unset(FAN_DISABLED);
}

/* -------------------------------------------------------------------------- *
 * Gibt die Karte zurück auf welche x/y zeigt                                 *
 * -------------------------------------------------------------------------- */
struct card *fan_at(int x, int y)
{
  struct card *card;
  struct node *node;
  
  /* Von oben nach unten durch die Fächer-Liste gehen */
  dlink_foreach_up_data(&fan_list, node, card)
  {
    /* Achja, und im Dragging-Modus sind nur die 
       nicht-ausgewählten Maus-Sensitiv! */
    if(fan_dragging() && card_selected(card))
      continue;    
    
    if(card_isat(card, CARD_FAN, x, y))
      return card;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Kalkuliert die Positionen und Winkel aller Karten und rotiert die Bilder   *
 * dementsprechend                                                            *
 * -------------------------------------------------------------------------- */
void fan_calc(void)
{
  struct card *card;
  struct node *node;
  float angle;
  float extent;
  int x;
  int y;
  
  extent = ((fan_list.size - 1) * fan_angle);
  angle = - (extent / 2);

  /* Die Kartenpositionen kalkulieren und Karten dorthin schieben */
  dlink_foreach_up_data(&fan_list, node, card)
  {
    x = fan_center.x + (sin(angle * M_PI / 180) * fan_radius[0]);
    y = fan_center.y - (cos(angle * M_PI / 180) * fan_radius[0]);
    
    card_setvector(card, CARD_FAN,
                   card->is[CARD_FAN].move.now, 
                   card->is[CARD_FAN].rotozoom.now);
    
    card_move(card, CARD_FAN, x, y, FAN_MSPEED);
    card_rotate(card, CARD_FAN, angle);
    
    fan_debug(INFO, "Calc [ x = %3i, y = %3i, r = %3i ] (%s)",
              x, y, (int)angle, card->name);
    
    angle += fan_angle;
  }
    
  /* Fächer muss neu gezeichnet werden */
  fan_set(FAN_REDRAW);
}

/* -------------------------------------------------------------------------- *
 * Ausgewählte Karten zählen                                                  *
 * -------------------------------------------------------------------------- */
int fan_selected(void)
{
  struct node *node;
  struct card *card;
  int i = 0;
  
  dlink_foreach_data(&fan_list, node, card)
    if(card_selected(card))
      i++;
  
  return i;
}

/* -------------------------------------------------------------------------- *
 * Alle Karten abwählen                                                       *
 * -------------------------------------------------------------------------- */
int fan_deselect(void)
{
  struct node *node;
  struct card *card;
  int i = 0;
  
  dlink_foreach_data(&fan_list, node, card)
  {
    /* Ausgewählte Karten müssen neu gerendert werden */
    if(card_selected(card))
    {
      card_unset(card, CARD_SEL);
      i++;
    }
  }
  
  return i;
}

/* -------------------------------------------------------------------------- *
 * Fächer zeichnen                                                            *
 * -------------------------------------------------------------------------- */
void fan_redraw(SDL_Surface *surface)
{
  struct card *card;
  struct node *node;
  
  if(fan_status & FAN_HIDDEN)
    return;
  
  fan_debug(DETAILS, "Redraw");
  
  dlink_foreach_data(&fan_list, node, card)
  {
    /* Wenn am draggen und die Karte ausgewählt, dann die DND 
       Instanz der Karte zeichnen, ansonsten die FAN-Instanz */
    card_blit(card, 
            ((card_selected(card) && fan_dragging()) ? CARD_DND : CARD_FAN), 
              surface);
  }

  fan_unset(FAN_REDRAW);
}
  
/* -------------------------------------------------------------------------- *
 * Checkt ob eine Karte an der aktuellen Position ist und Hilited diese ge-   *
 * gegebenenfalls.                                                            *
 * -------------------------------------------------------------------------- */
int fan_hilite(int x, int y)
{
  struct card *card;
  
  /* Über welcher Karte befindet sich die Maus? */
  card = fan_at(x, y);
  
  /* Neues hilite setzen */
  fan_previous = fan_card;
  fan_card = card;

  /* Hat hilite geändert? */
  if(fan_card != fan_previous)
  {
    /* Wenn eine andere Karte ausgewählt ist, dann diese hervorheben
       (Amplitude) */
//    fan_wavecard(fan_card);

    if(fan_card)
    {
      if(card_set(fan_card, CARD_HILITE))
        fan_set(FAN_REDRAW);
      
#ifdef MASSIVE
      client_log(LOG_DEBUG, "\\|/ FAN: Neue Hilite-Karte: %s", 
                 fan_card->name);
#endif /* DEBUG */
    }
#ifdef MASSIVE
    else
    {
      client_log(LOG_DEBUG, "\\|/ FAN: Keine Hilite-Karte mehr",
                 fan_card->name);
    }
#endif /* DEBUG */
    
    /* War vorher schon eine Karte ausgewählt, müssen wir diese abwählen */
    if(fan_previous)
    {
      if(card_unset(fan_previous, CARD_HILITE))
        fan_set(FAN_REDRAW);
    }
  }
  
  return fan_status;
}

/* -------------------------------------------------------------------------- *
 * Behandelt Mausbewegungen                                                   *
 * -------------------------------------------------------------------------- */
int fan_motion(uint8_t type, uint8_t state, int16_t x, int16_t y,
               int16_t xrel, int16_t yrel)
{
  /* Highlited Karte updaten wenn am Draggen oder Maustaste nicht gedrückt */
  if(fan_dragging() || !(state & SDL_BUTTON_LMASK))
    fan_hilite(x, y);
  
  if(!fan_dragging())
  {
    /* Bewegung über einer Karte bei gleichzeitigem Drücken der Maustaste? */
    if(fan_card && (state & SDL_BUTTON_LMASK))
    {
      /* Wenn keine Karte ausgewählt ist, dann wählen wir die Aktuelle aus */
      if(!fan_selected())
        card_set(fan_card, CARD_SEL);
      
      /* Wenn die aktuelle Karte ausgewählt ist, dann probieren wir den DnD
         Modus zu starten */
      if(card_selected(fan_card) && dnd_start(CARD_FAN, fan_target, x, y))
      {
        struct node *node;
        
        /* Wenn der Modus aktiviert ist, dann linken wir die ausgewählten 
           Karten in die DnD- und die Stack-Liste */
        if(fan_target == CARD_STACK)
        {
          /* Wenn nicht im Schupf-Modus, dann Stapel vorbereiten */
          dlink_foreach(&fan_list, node) if(card_selected(node->data))
            stack_add(node->data, player_local);
          
          /* Stapelpositionen vorkalkulieren */
          stack_calc(player_local, 0);
        }

        dlink_foreach(&fan_list, node) if(card_selected(node->data))
          dnd_add(node->data);
        
        fan_set(FAN_DRAG);
      }
    }
  }
  
  return fan_status;
}

/* -------------------------------------------------------------------------- *
 * Behandelt Mausgeklicke                                                     *
 * -------------------------------------------------------------------------- */
int fan_button(uint8_t type, uint8_t button, uint8_t state, int16_t x, int16_t y)
{
  /* Karten auswählen funktioniert nur wenn nicht im DnD Modus */
  if(!fan_dragging())
  {
    /* Wurde Linke Maustaste losgelassen? */
    if(button == SDL_BUTTON_LEFT && !(state & SDL_BUTTON_LMASK))
    {
      /* Wenn eine Karte den Fokus hat, dann wird sie an-/abgewählt */
      if(fan_card)
      {
        if(card_selected(fan_card))
        {
          if(card_unset(fan_card, CARD_SEL))
            fan_set(FAN_REDRAW);
        }
        else
        {
          /* Wenn am Schupfen, dann zuerst alle Karten abwählen */
          if(fan_target == CARD_PLAYER)
            fan_deselect();
          
          if(card_set(fan_card, CARD_SEL))
            fan_set(FAN_REDRAW);
        }
      }
    }
    
    /* Rechte Maustaste wurde losgelassen? */
    if(button == SDL_BUTTON_RIGHT && !(state & SDL_BUTTON_RMASK))
    {
      /* Alle Karten abwählen, und falls welche abgewählt wurden dann Redraw */
      if(fan_deselect())
      {
        fan_set(FAN_REDRAW);

        fan_debug(INFO, "Karten abgewählt...");
      }
    }
  }
  else
  {
    /* Wurde Linke Maustaste losgelassen? */
    if(button == SDL_BUTTON_LEFT && !(state & SDL_BUTTON_LMASK))
    {
      dnd_stop();
    }
    
  }

  return fan_status;
}
           
/* -------------------------------------------------------------------------- *
 * Behandelt Events die den Fächer betreffen                                  *
 * -------------------------------------------------------------------------- */
int fan_event(SDL_Event *event)
{
  if(fan_status & FAN_LOCKED)
    return fan_status;
  
  switch(event->type)
  {
    case SDL_MOUSEMOTION:
    {
      return fan_motion(event->motion.type, 
                        event->motion.state, 
                        event->motion.x, event->motion.y,
                        event->motion.xrel, event->motion.yrel);
    }
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
    {
      return fan_button(event->button.type, 
                        event->button.button,
                        event->button.state,
                        event->button.x, event->button.y);
    }
  }
  
  return fan_status;
}

/* -------------------------------------------------------------------------- *
 * Updatet Kartenpositionen und so                                            *
 * -------------------------------------------------------------------------- */
int fan_update(uint32_t t)
{
  struct node *node;

  /* Im DnD Modus müssen wir checken wann dieser vorüber ist... */
#if 0
  if(fan_dragging())
  {
    /* ...und dies ist er wenn die linke Maustaste nicht gedrückt ist
          und dnd_stop() uns sagt dass die Karten an ihrem angestammten
          Platz zurück sind */
    Uint8 buttons = SDL_GetMouseState(NULL, NULL);
    
    if((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) == 0)
    {
      if(dnd_stop())
      {
/*        if(fan_target == CARD_STACK)
        {
          dlink_foreach(&fan_list, node)
            if(card_selected(node->data))
              stack_remove(node->data);
        }*/

        fan_unset(FAN_DRAG);
        fan_set(FAN_REDRAW);
      }
    }    
  }
#endif
    
  /* Falls nicht eingefroren: Kartenpositionen aktualisieren */
  if((fan_status & FAN_FROZEN) == 0)
  {
    int astatus = CARD_FROZEN;
    int ostatus = 0;
    
    dlink_foreach(&fan_list, node)
    {
      int status;
      
      status = card_update(node->data, CARD_FAN, t);
      astatus &= status;
      ostatus |= status;
    }
    
    /* Irgend eine Karte muss neu gezeichnet werden */
    if(ostatus & CARD_REDRAW)
      fan_set(FAN_REDRAW);
    
    /* Alle Karten wieder eingefroren -> Fächer einfrieren */
    if(astatus & CARD_FROZEN)
    {
      fan_debug(INFO, "Fächer wieder eingefroren");
      fan_set(FAN_FROZEN);
    }
  }
  
  return fan_status;
}

/* -------------------------------------------------------------------------- *
 * Sortiert den Fächer                                                        *
 * -------------------------------------------------------------------------- */
void fan_sort(void)
{  
  struct node *node1;
  struct node *node2;
  struct card *card = NULL;
  struct card *array[fan_list.size];
  int i, j, size;
  
  /* backup list->size in size */
  size = fan_list.size;
  
  /* array erstellen */
  i = 0;
  
  dlink_foreach_safe_data(&fan_list, node1, node2, card)
  {    
    array[i] = card;
    i++;
    dlink_delete(&fan_list, &card->is[CARD_FAN].node);
  }
    
  /* array sortieren */
  for (i = 1; i < size; i++)
    for (j = size - 1; j >= i; j--)
      if (array[j]->value < array[j - 1]->value)
      {        
        card = array[j];
        array[j] = array[j - 1];
        array[j - 1] = card;
      }
  
  /* list erstellen */
  for(i = size-1; i >= 0; i--)
   // dlink_add_head(&fan_list, &card->n[CARD_FAN], card);    
   fan_add(array[i]);
  
  /* fächer neu berechnen */
  fan_calc();
  
}
