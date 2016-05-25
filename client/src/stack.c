/* $Id: stack.c,v 1.77 2005/05/21 08:27:20 smoli Exp $
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

#include <SDL.h>
#include <math.h>

/* -------------------------------------------------------------------------- *
 * Das STACK Modul zeigt den Kartenstapel an und nimmt die gespielten Karten  *
 * entgegen                                                                   *
 * -------------------------------------------------------------------------- */
#include "stack.h"
#include "fan.h"
#include "dnd.h"
#include "client.h"
#include "action.h"
#include "gfxutil.h"
#include "net.h"

/* Status
 * -------------------------------------------------------------------------- */
struct list       stack_list;
int               stack_status;
struct player    *stack_player;

/* Grafikdaten
 * -------------------------------------------------------------------------- */
SDL_Surface      *stack_image;    /* Stapelbild (Kreis mit Pfeil) */
SDL_Surface      *stack_rimage;   /* Eingefärbtes Stapelbild */
SDL_Rect          stack_rect;     /* Rechteck auf das Stapelbild bezogen */
SDL_Surface      *stack_timage;   /* Transformiertes Bild */
SDL_Rect          stack_tirect;   /* Rechteck des transformierten Bildes */
SDL_Surface      *stack_cards;    /* Abgelegte Karten */
SDL_Rect          stack_crect;    /* Rechteck der abgelegten Karten */
SDL_Surface      *stack_tcards;   /* Abgelegte Karten, transformiert */
SDL_Rect          stack_tcrect;   /* Rechteck der transformierten Karten */

/* Stapel-Konfiguration
 * -------------------------------------------------------------------------- */
struct position   stack_pos;
//uint32_t          stack_angle;
float             stack_radius[2];

/* Geometrische Daten
 * -------------------------------------------------------------------------- */
struct range      stack_scale;
struct rotozoom   stack_rotozoom;
struct blend      stack_color;

/* -------------------------------------------------------------------------- *
 * Initialisiert den Kartenstapel                                             *
 * -------------------------------------------------------------------------- */
void stack_init(void)
{
  /* Bild für den Kartenstapel laden (Kreis mit Pfeil) */
  client_load_png(&stack_image, "stack.png");

  /* Kartenstapel oben und zentriert ausrichten */
  stack_rect.x = (client_rect.w - stack_image->w) / 2;
  stack_rect.y = 0;
  stack_rect.w = stack_image->w;
  stack_rect.h = stack_image->h;
  
  /* Rechteck für abgelegte Karten muss etwas grösser sein */
  stack_crect = stack_rect;  
  gfxutil_padrect(&stack_crect, -64);
  
  /* Zentrum des Stacks berechnen */
  stack_pos.x = stack_rect.x + (stack_rect.w / 2);
  stack_pos.y = stack_rect.y + (stack_rect.h / 2);
  
  stack_radius[STACK_OUT] = 200;
  stack_radius[STACK_IN] = 75;

/*  gfxutil_vinit(&stack_org);
  gfxutil_vinit(&stack_pos);
  gfxutil_vinit(&stack_move.end);*/

  stack_set(STACK_DISABLED);
  
  stack_setcolor(gfxutil_white0);
  
  stack_setangle(0);
  stack_setzoom(0.0);
  
  stack_cards = NULL;
  stack_player = NULL;
  stack_status = STACK_DISABLED;
  
  dlink_list_zero(&stack_list);
  
  stack_log(STATUS, "Initialisiert");
}

/* -------------------------------------------------------------------------- *
 * Gibt die Stapel-Ressourcen wieder frei                                     *
 * -------------------------------------------------------------------------- */
void stack_shutdown(void)
{
  client_free_image(stack_image);
  client_free_image(stack_cards);
  client_free_image(stack_timage);
  client_free_image(stack_tcards);
//  client_free_image(stack_sf);
}  

/* -------------------------------------------------------------------------- *
 * Setzt Stapel-Statusflags                                                   *
 * -------------------------------------------------------------------------- */
int stack_set(int flags)
{
  /* Müssen wir noch flags setzen? */
  if((stack_status & flags) != flags)
  {
    /* WTF? */
    stack_debug(VERBOSE, "Set [%s%s%s%s%s%s ]",
                ((~stack_status) & flags & STACK_LOCKED) ? " LOCKED" : "",
                ((~stack_status) & flags & STACK_HIDDEN) ? " HIDDEN" : "",
                ((~stack_status) & flags & STACK_FROZEN) ? " FROZEN" : "",
                ((~stack_status) & flags & STACK_MOVE) ? " MOVE" : "",
                ((~stack_status) & flags & STACK_RENDER) ? " RENDER" : "",
                ((~stack_status) & flags & STACK_TRANSFORM) ? " TRANSFORM" : "");
    
    /* Flags setzen */
    stack_status |= flags;
    return 1;
  }

  return 0;
}

/* -------------------------------------------------------------------------- *
 * Löscht Fächer-Statusflags                                                  *
 * -------------------------------------------------------------------------- */
int stack_unset(int flags)
{
  /* Müssen wir noch flags löschen? */
  if(stack_status & flags)
  {
    stack_debug(VERBOSE, "Unset [%s%s%s%s%s%s ]",
                (stack_status & flags & STACK_LOCKED) ? " LOCKED" : "",
                (stack_status & flags & STACK_HIDDEN) ? " HIDDEN" : "",
                (stack_status & flags & STACK_FROZEN) ? " FROZEN" : "",
                (stack_status & flags & STACK_MOVE) ? " MOVE" : "",
                (stack_status & flags & STACK_RENDER) ? " RENDER" : "",
                (stack_status & flags & STACK_TRANSFORM) ? " TRANSFORM" : "");
    
    /* Flags löschen */
    stack_status &= ~flags;
    
    return 1;
  }
  
  return 0;
}
 
/* -------------------------------------------------------------------------- *
 * Eine Karte auf den Stapel legen (vielleicht nur temporär)                  *
 * -------------------------------------------------------------------------- */
void stack_add(struct card *card, struct player *player)
{
  /* Spieler setzen */
  card->player = player;  
  
  /* Karte auf die Stapel-Liste */
  dlink_add_tail(&stack_list, &card->is[CARD_STACK].node, card);
}

/* -------------------------------------------------------------------------- *
 * Karte wieder vom Stapel nehmen                                             *
 * -------------------------------------------------------------------------- */
void stack_remove(struct card *card)
{
  dlink_delete(&stack_list, &card->is[CARD_STACK].node);
}

/* -------------------------------------------------------------------------- *
 * Setzt den Spieler der an der Reihe ist                                     *
 * -------------------------------------------------------------------------- */
void stack_actor(struct player *player)
{
  /* Spieler 0 ist auf 180°, also unten mittig */
  float a = 180 - player->angle[PLAYER_OUT];
  
  /* Zeigen wir den Stack das erste mal an für dieses Game? */
  if(stack_player == NULL)
  {
    /* Anfangsfarbe setzen */
    stack_setcolor(gfxutil_white0);
  
    /* Beim Reinzoomen machen wir eine 3/4 Drehung */
    stack_setangle(a - 270);
    stack_setzoom(0.0);
  }
  
  stack_player = player;
  
  /* Stapel in Richtung des Spielers drehen und dabei Überblenden */
  stack_rotate(a, STACK_SPEED);
  stack_zoom(1.0);
  stack_blend(player->blend.now);
  
  /* Wenns nicht der lokale Spieler ist, dann sperren wir
     den Stack für Events, andernfalls geben wir ihn frei */
  (player->index ? stack_set : stack_unset)(STACK_LOCKED);
  
  /* Stack sollte jetzt angezeigt werden */
  stack_unset(STACK_HIDDEN);
  
  stack_debug(INFO, "Spieler #%u (%s) ist dran", player->index, player->name);
}
  
/* -------------------------------------------------------------------------- *
 * Setzt die Stapelfarbe                                                      *
 * -------------------------------------------------------------------------- */
void stack_setcolor(struct color color)
{
  stack_color.start.r = stack_color.now.r = stack_color.end.r = color.r;
  stack_color.start.g = stack_color.now.g = stack_color.end.g = color.g;
  stack_color.start.b = stack_color.now.b = stack_color.end.b = color.b;
  stack_color.start.a = stack_color.now.a = stack_color.end.a = color.a;
}

/* -------------------------------------------------------------------------- *
 * Überblendet die Stapelfarbe                                                *
 * -------------------------------------------------------------------------- */
void stack_blend(struct color color)
{
  /* Von der aktuellen Farbe ausgehend */
  stack_color.end = stack_color.start = stack_color.now;
  
  /* Zielfarbe setzen */
  stack_color.end.r = color.r;
  stack_color.end.g = color.g;
  stack_color.end.b = color.b;
  stack_color.end.a = 255;
   
  stack_debug(INFO, "Blend [ r = %3u, g = %3u, b = %3u, a = %3u ]",
              color.r, color.g, color.b, color.a);
}

/* -------------------------------------------------------------------------- *
 * Setzt den Winkel des Stapels                                               *
 * -------------------------------------------------------------------------- */
void stack_setangle(int r)
{
  stack_rotozoom.now.r = (float)(r % 360);
  stack_rotozoom.start = stack_rotozoom.end = stack_rotozoom.now;
}

/* -------------------------------------------------------------------------- *
 * Setzt den Zoomfaktor des Stapels                                           *
 * -------------------------------------------------------------------------- */
void stack_setzoom(float z)
{
  stack_rotozoom.now.z = z;
  stack_rotozoom.start = stack_rotozoom.end = stack_rotozoom.now;
}

/* -------------------------------------------------------------------------- *
 * Rotiert den Stapel an die angegebene Position mit einer Geschwindigkeit    *
 * von v° pro Sekunde                                                         *
 * -------------------------------------------------------------------------- */
void stack_rotate(int a, uint32_t v)
{
  int sa = (int)stack_rotozoom.now.r % 360;
  int da = a % 360;
  uint32_t t;
  
  /* Immer im Gegenuhrzeigersinn und niemals mehr als 360° drehen */
  if(sa < da) sa += 360;
  if(da > sa) da -= 360;

  stack_debug(INFO, "Rotate [ sa = %3i, da = %3i, v = %4u ]",
              (int)sa, (int)da, v);

  /* Anfangs- und Endwinkel setzen */
  stack_rotozoom.end = stack_rotozoom.start = stack_rotozoom.now;
  
  stack_rotozoom.start.r = sa;
  stack_rotozoom.end.r = da;
  
  /* Zeit für den Vorgang ausrechnen */
  t = (sa - da) * 1000 / v;

  gfxutil_scaletime(&stack_scale, SDL_GetTicks(), t);
  
  /* GO GO GO! Because we set start time to current! :) */
  stack_unset(STACK_FROZEN);
}

/* -------------------------------------------------------------------------- *
 * Zoomt den Stapel während einer Rotation                                    *
 * -------------------------------------------------------------------------- */
void stack_zoom(float z)
{
  /* Muss nach stack_rotate() aufgerufen werden, dafür müssen wir nur noch den
     Zielfaktor setzen. Vielleicht sollte dies nicht eine einzelne Funktion sein,
     aber wir wollen ja nicht schon wieder mein Konzept durcheinander bringen :) */
  stack_rotozoom.end.z = z;

  stack_debug(VERBOSE, "Zoom [ z = %.3f ]", z);
}

/* -------------------------------------------------------------------------- *
 * Findet per Rechteck-Überlappung und Pixel-Test heraus ob sich an der       *
 * angegebenen Position tatsächlich der Stapel befindet                       *
 * -------------------------------------------------------------------------- */
int stack_isat(int x, int y)
{
  /* Ist eine Surface da und die Position innerhalb des Stapelrechtecks? */
  if(stack_timage && gfxutil_matchrect(&stack_tirect, x, y))
  {
    struct color color;
    
    /* Position relativ zum Stapelsurface */
    x -= stack_tirect.x;
    y -= stack_tirect.y;
    
    /* Kollisionstest */
    color = gfxutil_getpixel(stack_timage, x, y);
    
    if(color.a)
    {
      stack_debug(INFO, "Pixelmaske @ %u|%u", x, y);
      return 1;
    }
    
    return 0;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Blittet das Stack-Bild nach stack_sf und färbt es dabei ein                *
 * -------------------------------------------------------------------------- */
void stack_render(void)
{
  stack_debug(VERBOSE, "Render [ r = %3u, g = %3u, b = %3u, a = %3u ]",
              stack_color.now.r, stack_color.now.g,
              stack_color.now.b, stack_color.now.a);
  
  /* Neue Surface falls noch keine da */
  if(stack_rimage == NULL)
    stack_rimage = SDL_CreateRGBSurface(SDL_SWSURFACE, 
                                        stack_image->w, stack_image->h,
                                        32, RMASK, GMASK, BMASK, AMASK);
  
  /* Gefärbtes Kopieren mit 100% Opazität */
  gfxutil_tint(stack_image, NULL, stack_rimage, NULL, stack_color.now);
  
  /* Stack gerendert, aber jetzt müssen wir noch neu transformieren */
  stack_status &= ~STACK_RENDER;
  stack_status |= STACK_TRANSFORM;
}

/* -------------------------------------------------------------------------- *
 * Rotiert und zoomt den Stapel nach den aktuellen Werten                     *
 * -------------------------------------------------------------------------- */
void stack_transform(void)
{
  stack_debug(VERBOSE, "Transform [ angle = %3i, zoom = %.3f ]",
              (int)stack_rotozoom.now.r, stack_rotozoom.now.z);
  
  /* Alte Surface löschen */
  if(stack_timage) SDL_FreeSurface(stack_timage);
  if(stack_tcards) SDL_FreeSurface(stack_tcards);
  
  /* Surfaces transformieren */
  stack_timage = gfxutil_rotozoom(stack_rimage, stack_rotozoom.now.r, stack_rotozoom.now.z, 255);
  
  if(stack_cards)
    stack_tcards = gfxutil_rotozoom(stack_cards, stack_rotozoom.now.r, stack_rotozoom.now.z, 255);

  stack_status &= ~STACK_TRANSFORM;
}

/* -------------------------------------------------------------------------- *
 * Updatet die Werte für Zoom, Rotation und Farbe des Kartenstapels für die   *
 * aktuelle Zeit                                                              *
 * -------------------------------------------------------------------------- */
int stack_update(uint32_t t)
{
  if((stack_status & STACK_FROZEN) == 0)
  {
    /* Die neuen Parameter werden erst temporär hier gespeichert, weil
       wir sie dann mit den alten vergleichen um den Redraw-Status zu
       aktualisieren */
    struct transform rotozoom;
    struct color blend;
    uint16_t scale16;
    struct node *node;
    int cstatus; /* Kartenstatus */
    
    /* Ist das Ende der ZeitSKAla erreicht, so 
       frieren wir den Stapel wieder ein :) */
    if(gfxutil_scaleadd(&stack_scale, t))
      stack_set(STACK_FROZEN);

    /* 16-bit Skalenwert generieren... */
    scale16 = gfxutil_scale16(&stack_scale);
    
    /* ...und damit die Transformation und das Blending updaten */
    gfxutil_transform(&rotozoom, stack_rotozoom.start, stack_rotozoom.end, scale16);
    gfxutil_blend(&blend, stack_color.start, stack_color.end, scale16);
    
    /* Hat Transformation geändert? */
    if(gfxutil_difftransform(rotozoom, stack_rotozoom.now))
    {
      stack_debug(DETAILS, "Transform -> %.3f (%.3f)", rotozoom.r, rotozoom.a);
      stack_status |= STACK_TRANSFORM;
    }
    
    /* Hat Blending geändert? */
    if(gfxutil_diffcolor(blend, stack_color.now))
    {
      stack_debug(DETAILS, "Blend -> %02x%02x%02x%02x",
                  blend.r, blend.g, blend.b, blend.a);
      
      stack_status |= STACK_RENDER;
    }
    
    /* Karten updaten */
    cstatus = CARD_FROZEN;
    
    dlink_foreach(&stack_list, node)
    {
      int status;
      status = card_update(node->data, CARD_STACK, t);
      
      if(!(status & CARD_FROZEN))
        stack_unset(STACK_FROZEN);
      
      if(status & CARD_MOVE)
        stack_status |= STACK_MOVE;
      
      cstatus &= status;
    }
    
    /* Wenn Karten in der Stapel-Liste sind, deren Bewegung
       aber jetzt abgeschlossen ist, dann nehmen wir sie
       entgegen und "Brennen" Sie auf das Stapel-Surface, 
       womit sie dann aus dem Spiel sind */
    if(stack_list.size && (cstatus & CARD_FROZEN))
      stack_accept();
    
    if(stack_status & STACK_FROZEN)
      stack_debug(INFO, "Bewegung fertig -> eingefroren");
  
    /* Die Werte aktualisieren */
    stack_rotozoom.now = rotozoom;
    stack_color.now = blend;
  }
  
  return stack_status;
}

/* -------------------------------------------------------------------------- *
 * Kalkuliert die Positionen und Winkel aller Karten und rotiert die Bilder   *
 * dementsprechend                                                            *
 *                                                                            *
 * 'angle' gibt den Startwinkel an, 0° für den lokalen Spieler, 90° für den   *
 * nächsten Spieler                                                           *
 * -------------------------------------------------------------------------- */
void stack_calc(struct player *player, uint32_t speed)
{
  struct card *card;
  struct node *node;
  float angle;
  float extent;
  
  angle = player->angle[PLAYER_OUT];
  
  /* Bogenumfang */
  extent = ((stack_list.size - 1) * STACK_ANGLE);
  
  /* Von - (Bogenumfang / 2) bis (Bogenumfang / 2) */
  angle -= (extent / 2);

  stack_debug(INFO, "Calc [ extent = %3i°, start = %3i°, end = %3i° ]",
              (int)extent, (int)angle, -(int)angle);
  
  dlink_foreach_data(&stack_list, node, card)
  {
    if(card->player == player)
    {
      float x, y;
      
      /* Relative Position zum Stack */
      x = stack_pos.x - sin(angle * M_PI / 180) * stack_radius[STACK_IN];
      y = stack_pos.y - cos(angle * M_PI / 180) * stack_radius[STACK_IN];
      
      /* Karte in die Stapelposition bringen */
      card_move(card, CARD_STACK, x, y, speed);
      card_rotate(card, CARD_STACK, angle);
      
      angle += STACK_ANGLE;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * Karte für das endgültige Ablegen auf die Surface vorbereiten               *
 * -------------------------------------------------------------------------- */
void stack_recalc(struct card *card)
{
  float angle;
  int x, y;
  
  /* Endgültigen Winkel setzen */
  
  
  
  /* card:      0°  stack: 0°   (player #0) result: 0° */
  /* card:    -90°  stack: 90°  (player #1) result: 0° */
  /* card: +/-180°  stack: 180° (player #2) result: 0/360° */
  /* card:     90°  stack: 270° (player #3) result: 0/360° */
  angle = stack_rotozoom.now.r - (180 - card->is[CARD_STACK].rotozoom.now.r);
  
  stack_debug(INFO, "Stack winkel: %i, Kartenwinkel: %i Final: %i",
              (int)stack_rotozoom.now.r,
              (int)card->is[CARD_STACK].rotozoom.now.r,
              (int)angle);
              
  card_setangle(card, CARD_STACK, angle);
  
  /* Relative Position zum Stack */
  x = stack_pos.x - sin(angle * M_PI / 180) * stack_radius[STACK_IN];
  y = stack_pos.y + cos(angle * M_PI / 180) * stack_radius[STACK_IN];
  
  /* Karte in Position bringen */
  card_setpos(card, CARD_STACK, x, y);
}

/* -------------------------------------------------------------------------- *
 * Behandelt das Ablegen von Karten auf dem Stack                             *
 * -------------------------------------------------------------------------- */
void stack_drop(void)
{
  struct node *node;
  struct node *next;
  struct card *card;
  
  /* Karten spielen */
  action_play(&dnd_list);
  
  dlink_foreach_safe_data(&dnd_list, node, next, card)
  {
    /* Position und Winkel von DnD übernehmen */
    card->is[CARD_STACK].move.now = card->is[CARD_DND].move.now;
    card->is[CARD_STACK].rotozoom.now = card->is[CARD_DND].rotozoom.now;
#ifdef DEBUG
    card_dump(card, CARD_STACK);
    card_dump(card, CARD_DND);
#endif /* DEBUG */
    /* Richtung Stackposition schieben */
    card_move(card, CARD_STACK, 
              card->is[CARD_STACK].move.end.x,
              card->is[CARD_STACK].move.end.y, DND_SPEED);
    
    card_rotate(card, CARD_STACK, 
                card->is[CARD_STACK].rotozoom.end.r);
    
    /* Von der DnD-Liste nehmen */
    dlink_delete(&dnd_list, &card->is[CARD_DND].node);
//    dnd_remove(card);
    
    /* Und der Fan-Liste */
   //  dlink_delete(&fan_list, &card->is[CARD_FAN].node);
  }
  
  /* DnD und Fan Freezen und Locken */
  fan_set(FAN_LOCKED|FAN_FROZEN);
  dnd_set(DND_LOCKED|DND_FROZEN);
  
  stack_debug(INFO, "drop");
  
  stack_set(STACK_LOCKED);
  stack_unset(STACK_FROZEN);
  
#ifdef DEBUG
  if(client_status == CLIENT_TEST)
    stack_refuse();
#endif /* DEBUG */
}

/* -------------------------------------------------------------------------- *
 * Weist Karten zurück                                                        *
 * -------------------------------------------------------------------------- */
void stack_refuse(void)
{
  /* Karten wieder dem Drag & Drop übergeben und dieses stoppen */
/*  dlink_foreach_data(&stack_list, node, card)
  {
    dlink_add_head(&dnd_list, &card->is[CARD_DND].node, card);
    dlink_add_head(&fan_list, &card->is[CARD_FAN].node, card);
  }*/
  
  stack_debug(INFO, "Refuse");
  
  fan_unset(FAN_LOCKED);
  
  dnd_stop();
}

/* -------------------------------------------------------------------------- *
 * Nimmt die Karten definitiv entgegen und zeichnet sie auf das Surface für   *
 * die abgelegten Karten                                                      *
 * -------------------------------------------------------------------------- */
void stack_accept(void)
{
  struct node *node;
  struct node *next;
  struct card *card;
  
/*  stack_debug(INFO, "Karten entgegennehmen (Winkel %i), Rect = %i, %i, %i, %i",
              stack_angle,
              stack_rect.x,
              stack_rect.y,
              stack_rect.w,
              stack_rect.h);*/
  
  /* Surface erstellen, falls noch keine da */
  if(stack_cards == NULL)
  {
    stack_cards = SDL_CreateRGBSurface(SDL_SWSURFACE, 
                                       stack_crect.w, stack_crect.h,
                                       32, RMASK, GMASK, BMASK, AMASK);

    SDL_FillRect(stack_cards, NULL, 0);
  }

  /* Karten auf Surface blitten ein freigeben */
  dlink_foreach_safe_data(&stack_list, node, next, card)
  {
    /* Aber nur die Karten die ihre Bewegung abgeschlossen haben */
    if(card->is[CARD_STACK].status & CARD_FROZEN)
    {
      SDL_Rect rect;
      
      /* Kartenwinkel neu berechnen, da diese anders 
         transformiert sein muss wenn Sie abgelegt ist */
      stack_recalc(card);
      
      /* Karte neu transformieren falls nötig */
      if(card->is[CARD_STACK].status & CARD_TRANSFORM)
        card_transform(card, CARD_STACK);
      
      /* Auf die neue Position zentrieren */
      card_center(card, CARD_STACK);
      
      rect = card->is[CARD_STACK].rect;
      
      rect.x -= stack_crect.x;
      rect.y -= stack_crect.y;
      
      gfxutil_blit(card->is[CARD_STACK].surface, NULL,
                   stack_cards, &rect, 255, NULL);
      
      stack_debug(INFO, "Karte fixiert auf %i|%i",
                  card->is[CARD_STACK].move.now.x,
                  card->is[CARD_STACK].move.now.y);
      
      /* Kartendaten freigeben, Struktur aber beibehalten */
      card_clean(card);
      
      stack_remove(card);
      
      /* Aus dem Fächer nehmen und diesen neu kalkulieren */
//      fan_remove(card);
//      
      fan_calc();
    }
  }
  
  stack_set(STACK_REDRAW|STACK_TRANSFORM);
  
  fan_unset(FAN_LOCKED|FAN_FROZEN);
  
  dnd_stop();
  
  /* Netzwerk wieder aktivieren */
  net_unset(NET_SHUTUP);
}
  
/* -------------------------------------------------------------------------- *
 * Schiebt Karten aus Position von anderem Spieler in Richtung Stapel         *
 *                                                                            *
 * Winkel wird aus player_index und player_count berechnet,                   *
 * player_index 0 ist der lokale Spieler                                      *
 * -------------------------------------------------------------------------- */
void stack_appear(struct list *cards, struct player *player, int radius)
{
  struct node *node;
  struct card *card;
  
  /* Startpositionen und Winkel setzen */
  dlink_foreach_data(&stack_list, node, card)
  {
    if(card->player == player)
    {
      card_setpos(card, CARD_STACK, player->pos[PLAYER_OUT].x, player->pos[PLAYER_OUT].y);
      card_setangle(card, CARD_STACK, player->angle[PLAYER_OUT]);
      
      stack_debug(INFO, "Appear %s", card->name);
    }
  }
  
  /* Stapelpositionen durchkalkulieren (setzt Endposition) */
  stack_calc(player, CARD_MOVESPEED);
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int stack_motion(Uint8 type, Uint8 state, Uint16 x, Uint16 y,
                 Sint16 xrel, Sint16 yrel)
{
//  int isat;

  /* Maus zeigt auf Stapel und Fächer liegt nicht dazwischen */
//  isat = stack_isat(x, y) && !fan_at(x, y);
  
/*  if(isat)
    client_log(LOG_DEBUG, "--- STK: Mausbewegung innerhalb");
  else
    client_log(LOG_DEBUG, "--- STK: Mausbewegung ausserhalb");*/
  
  /* Falls Karten über den Stapel gedraggt werden aktivieren wir
     den Hover-Modus */
#if 0
/*  if(fan_dragging() && isat)
  {
    if((stack_status & STACK_HOVER) == 0)
      stack_hover();
  }
  else
  {
    if((stack_status & STACK_HOVER))
      stack_unhover();
  }*/
#endif
  return stack_status;
}
 
/* -------------------------------------------------------------------------- *
 * Behandelt Mausgeklicke                                                     *
 * -------------------------------------------------------------------------- */
int stack_button(uint8_t type, uint8_t button, uint8_t state, int16_t x, int16_t y)
{
/*  struct node *node;
  struct node *next;
  struct card *card;*/  
  
  /* Wurde die linke Maustaste losgelassen? */
  if(button == SDL_BUTTON_LEFT && !(state & SDL_BUTTON_LMASK))
  {
    stack_debug(INFO, "Maus über dem Stapel losgelassen");
    
    /* Sind Karten in der Stapel-Liste? */
    if(stack_list.size)
    {
      /* Wenn wir über dem Stapel sind, dann leiten wir das Ablegen der Karten ein */
      if(stack_isat(x, y))
        stack_drop();
      /* ..andernfalls den Stop des Drag & Drop */
      else
        dnd_stop();
    }
    
  }

  return stack_status;
} 

/* -------------------------------------------------------------------------- *
 * Behandelt allgemeine Events und leitet sie weiter and Subroutinen          *
 * -------------------------------------------------------------------------- */
int stack_event(SDL_Event *event)
{
  if(stack_status & STACK_LOCKED)
    return stack_status;
  
  switch(event->type)
  {
    case SDL_MOUSEMOTION:
    {
      return stack_motion(event->motion.type, 
                          event->motion.state,
                          event->motion.x, event->motion.y,
                          event->motion.xrel, event->motion.yrel);
      break;
    }
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
    {
      return stack_button(event->button.type, 
                          event->button.button, 
                          event->button.state,
                          event->button.x, event->button.y);
      break;
    }
    case SDL_KEYUP:
    {
      if(event->key.keysym.sym != SDLK_ESCAPE)
        break;
    }
    case SDL_QUIT:
    {
      return 0;
    }
  }
  
  return 1;
}

/* -------------------------------------------------------------------------- *
 * Setzt das Zielrechteck                                                     *
 * -------------------------------------------------------------------------- */
void stack_center(SDL_Rect *irect, SDL_Rect *crect)
{
  if(irect)
  {
    irect->w = stack_timage->w;
    irect->h = stack_timage->h;
    
    gfxutil_centerxy(irect, stack_pos.x, stack_pos.y);
    stack_tirect = *irect;
  }

  if(crect)
  {
    crect->w = stack_tcards->w;
    crect->h = stack_tcards->h;
    
    gfxutil_centerxy(crect, stack_pos.x, stack_pos.y);
    stack_tcrect = *crect;
  }
}

/* -------------------------------------------------------------------------- *
 * Stapel auf den Screen blitten...                                           *
 * -------------------------------------------------------------------------- */
void stack_blit(SDL_Surface *surface)
{
  SDL_Rect irect, crect;
  struct card *card;
  struct node *node;
  
  stack_center((stack_timage ? &irect : NULL), 
               (stack_tcards ? &crect : NULL));
  
  if(stack_timage)
    SDL_BlitSurface(stack_timage, NULL, surface, &irect);
  
  if(stack_tcards)
    SDL_BlitSurface(stack_tcards, NULL, surface, &crect);

  dlink_foreach_data(&stack_list, node, card)
    card_blit(card, CARD_STACK, surface);
}

/* -------------------------------------------------------------------------- *
 * Stapel neu zeichnen...                                                     *
 * -------------------------------------------------------------------------- */
void stack_redraw(SDL_Surface *surface)
{
  if(stack_status & STACK_HIDDEN)
    return;
 
  /* Neu rendern? */
  if((stack_status & STACK_RENDER) || stack_rimage == NULL)
    stack_render();
  
  /* Neu transformieren? */
  if((stack_status & STACK_TRANSFORM) || stack_timage == NULL)
    stack_transform();
  
  /* Schlussendlich auf Screen blitten */
  stack_blit(surface);

  /* Redraw flags resetten */
  stack_unset(STACK_REDRAW);
}

/* -------------------------------------------------------------------------- *
 * Löscht den Stapel                                                          *
 * -------------------------------------------------------------------------- */
void stack_clear(void)
{
  stack_debug(INFO, "Lösche Kartenstapel");
  
  dlink_list_zero(&stack_list);
  
  if(stack_cards)
    SDL_FillRect(stack_cards, NULL, 0);
  
  stack_status |= STACK_MOVE;
}

/* -------------------------------------------------------------------------- *
 * Stapeldaten ausgeben                                                       *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void stack_dump(void)
{
  stack_debug(INFO,
              "----------------------------------------------------");
  
  if(stack_rimage)
    stack_debug(INFO, "rimage = %ux%u",
                stack_rimage->w, stack_rimage->h);
  else
    stack_debug(INFO, "sf     = NULL");
  
  if(stack_timage)
    stack_debug(INFO, "timage = %ux%u",
                stack_timage->w, stack_timage->h);
  else
    stack_debug(INFO, "timage = NULL");
  
  stack_debug(INFO, "rect   = %i,%i %ux%u",
              stack_rect.x, stack_rect.y,
              stack_rect.w, stack_rect.h);
  
  stack_debug(INFO, "trect  = %i,%i %ux%u",
              stack_tirect.x, stack_tirect.y,
              stack_tirect.w, stack_tirect.h);
  
/*  stack_debug(INFO, "pos   = { x = %i, y = %i, z = %.3f, a = %.3f, t = %u }",
              stack_move.now.x, stack_move.now.y,
              stack_rotozoom.now.z, stack_rotozoom.now.a,
              stack_range.now);
  
  stack_debug(INFO, "org   = { x = %i, y = %i, z = %.3f, a = %.3f, t = %u }",
              stack_move.start.x, stack_move.start.y,
              stack_rotozoom.start.z, stack_rotozoom.start.a,
              stack_move.start.t);
  
  stack_debug(INFO, "dst   = { x = %i, y = %i, z = %.3f, a = %.3f, t = %u }",
              stack_move.end.x, stack_move.end.y,
              stack_rotozoom.end.z, stack_rotozoom.end.a,
              stack_range.end);*/
}
#endif /* DEBUG */
