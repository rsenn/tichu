/* $Id: player.c,v 1.66 2005/05/21 08:27:20 smoli Exp $
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
#include <math.h>
#include <SDL.h>

/* -------------------------------------------------------------------------- *
 * Das PLAYER Modul verwaltet die Spieler welche am aktuellen Spiel           *
 * teilnehmen                                                                 *
 *                                                                            *
 * Es ist ausserdem für das "Schupfen" (das Austauschen von Karten unter den  *
 * Spielern) verantwortlich indem es den Kreis, der die Karten ähnlich dem    *
 * Stapel entgegennimmt, für jeden Spieler anzeigt.                           *
 * -------------------------------------------------------------------------- */

#include "player.h"
#include "action.h"
#include "stack.h"
#include "game.h"
#include "fan.h"
#include "dnd.h"
#include "net.h"

/* -------------------------------------------------------------------------- *
 * Globale Variabeln                                                          *
 * -------------------------------------------------------------------------- */
struct list            player_list;      /* Liste der Spieler */
float                  player_radius[2]; /* Innerer und äusserer Radius */
int                    player_status;    /* Status des Spieler-Moduls */
struct player         *player_hilite;    /* Ausgewählter Spieler */
struct player         *player_local;     /* Lokaler Spieler */
SDL_Surface           *player_image;     /* Bild mit Kreis */

/* -------------------------------------------------------------------------- *
 * Initialisiert die Spielerverwaltung zu Begin eines Spiels                  *
 * -------------------------------------------------------------------------- */
void player_init(void)
{
  /* Spieler-Liste initialisieren */
  dlink_list_zero(&player_list);
  
  /* Surface für den Schupf-Kreis laden */
  player_image = NULL;
  
  client_load_png(&player_image, "circle.png");
  
  /* Radius des äusseren Kreisbogens, welcher um den Screen verläuft */
  player_radius[PLAYER_OUT] = stack_pos.x + gfxutil_dist(CARD_WIDTH, CARD_HEIGHT);
  
  /* Radius des inneren Kreisbogens, welcher das gleiche Zentrum hat
     wie der Kreisbogen des Fächers und auf welchem die Schupf-Kreise 
     angezeigt werden */
  player_radius[PLAYER_IN] = fan_radius[0] +
                            (fan_center.y - fan_radius[0]) / 3 + 
                             gfxutil_dist(CARD_WIDTH, CARD_HEIGHT) / 2;
  
  player_status = PLAYER_REDRAW|PLAYER_FROZEN;
}

/* -------------------------------------------------------------------------- *
 * Fährt die Spielerverwaltung runter                                         *
 * -------------------------------------------------------------------------- */
void player_shutdown(void)
{
  struct player *player;
  struct node   *node;
  
  /* Alle Spieler löschen */
  dlink_foreach_safe(&player_list, player, node)
    player_delete(player);
  
  /* Surfaces freigeben */
  client_free_image(player_image);
}

/* -------------------------------------------------------------------------- *
 * Erstellt einen neuen Spieler.                                              *
 *                                                                            *
 * 'index' ist die Position des Spielers. Der lokale Spieler hat immer einen  *
 * index von 0, gegen den Uhrzeigersinn werden dann die Spieler bis 3 durch-  *
 * nummeriert.                                                                *
 * -------------------------------------------------------------------------- */
struct player *player_new(const char *name, int index, struct color tint)
{
  struct player *player;
  
  /* Neue Spielerstruktur erstellen und initialisieren */
  player = malloc(sizeof(struct player));
  
  client_strlcpy(player->name, name, sizeof(player->name));
  
  player->index = index;
  player->status = 0;
  player->sf = NULL;
  player->tf = NULL;

  dlink_list_zero(&player->cards);
  
  player_setcolor(player, tint);

  dlink_add_tail(&player_list, &player->node, player);
  
  /* Setze den lokalen Spieler wenn der index 0 ist */
  if(index == 0)
    player_local = player;
  
  /* ...andernfalls brauchen wir ein Surface für den 
     Kreis, welches wir mit der Spielerfarbe einfärben */
  else
    player->status |= PLAYER_RENDER|PLAYER_TRANSFORM;
  
  player_debug(VERBOSE, "New [ index = %i, color = %s ] (%s)",
               player->index,
               gfxutil_strcolor(player->blend.now),
               player->name);
  
  return player;
}

/* -------------------------------------------------------------------------- *
 * Löscht den Spieler                                                         *
 * -------------------------------------------------------------------------- */
void player_delete(struct player *player)
{
  client_free_image(player->sf);
  
  dlink_delete(&player_list, &player->node);
  
  free(player);
}

/* -------------------------------------------------------------------------- *
 * Findet einen Spieler nach Namen                                            *
 * -------------------------------------------------------------------------- */
struct player *player_find(const char *name)
{
  struct node *node;
  struct player *player;
  
  dlink_foreach_data(&player_list, node, player)
  {
    if(!strcmp(player->name, name))
      return player;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Berechnet inneren und äusseren Winkel eines Gegen- oder Mitspielers        *
 * anhand seines Indexes.                                                     *
 *                                                                            *
 * Der äussere Winkel legt fest aus welcher Richtung die Karten des Spielers  *
 * erscheinen.                                                                *
 * Der 'innere' Winkel hingegen die Position des Schupf-Kreises auf dem Kreis *
 * bogen.                                                                     *
 * -------------------------------------------------------------------------- */
void player_calc(struct player *player)
{
  float extent;
  float step;
  float angle;
  
  /* Umfang des Bogens berechnen */
  extent = player_radius[0] * M_PI * 2;
  angle = (client_config.width - player_image->w) * 2 * 360 / extent;
  
  step = angle / player_list.size;
  
  player->angle[0] = (angle / 2) - (step * player->index);

  /* Innere Position */
  player->pos[0].x = fan_center.x + sin(player->angle[0] * M_PI / 180) * player_radius[0];
  player->pos[0].y = fan_center.y - cos(player->angle[0] * M_PI / 180) * player_radius[0];
  
  /* Schrittweite auf dem Umkreis für den äusseren Winkel */
  extent = 360.0;
  step = extent / player_list.size;

  /* Winkel 0° ist 6 Uhr, Player 0 ist Winkel 0°, 
     Drehung gegen den Uhrzeigersinn */
  player->angle[1] = player->index * step;

  if(player->angle[1] > 180)
    player->angle[1] -= 360;
  
  /* Äussere Position */
  player->pos[1].x = stack_pos.x + sin(player->angle[1] * M_PI / 180) * player_radius[1];
  player->pos[1].y = stack_pos.y + cos(player->angle[1] * M_PI / 180) * player_radius[1];  
  
  player_debug(VERBOSE, "Calc [ winkel innen =%4i, aussen =%4i ] (%s)",
               (int)player->angle[0], (int)player->angle[1], player->name);
}

/* -------------------------------------------------------------------------- *
 * Setzt die Spielerfarbe                                                     *
 * -------------------------------------------------------------------------- */
void player_setcolor(struct player *player, struct color tint)
{
  player->blend.start = player->blend.now = player->blend.end = tint;
}

/* -------------------------------------------------------------------------- *
 * Fadet den Alpha-Layer des Spielerkreises                                   *
 * -------------------------------------------------------------------------- */
void player_fade(struct player *player, uint8_t alpha, uint32_t t)
{
  /* Zu erreichenden Alphawert setzen */
  player->rotozoom.end = player->rotozoom.start = player->rotozoom.now;  
  player->rotozoom.end.a = ((float)alpha) / 255.0;
  
  /* Loslegen damit */
  gfxutil_scaletime(&player->scale, game_ticks, t);
  
  player_status &= ~PLAYER_FROZEN;
  
  player_debug(VERBOSE, "Fade [ f = %3u, t = %3u, ms = %umsec ] (%s)",
               (int)(player->rotozoom.start.a * 255), 
               (int)(player->rotozoom.end.a * 255), t, player->name);
}

/* -------------------------------------------------------------------------- *
 * Setzt initiale Position eines Mit- oder Gegenspielers.                     *
 * -------------------------------------------------------------------------- */
void player_setpos(struct player *player, int out)
{
  /* Bewegungsvektor setzen */
  player->move.start =
  player->move.now =
  player->move.end = player->pos[out];
  
  /* Winkel */
  player->rotozoom.start.r =
  player->rotozoom.now.r =
  player->rotozoom.end.r = (out ? -1 : 1) * player->angle[out];
  
  /* Zoomfaktor ist immer 1.0 */
  player->rotozoom.start.z =
  player->rotozoom.now.z =
  player->rotozoom.end.z = 1.0;

  /* Alpha anfangs 0.0 */
  player->rotozoom.start.a = 
  player->rotozoom.now.a =
  player->rotozoom.end.a = 0.0;
}

/* -------------------------------------------------------------------------- *
 * Schiebt Mit- oder Gegenspieler an die innere oder äussere Position         *
 * Gibt die Relative (noch zurückzulegende) Distanz zurück                    *
 * -------------------------------------------------------------------------- */
struct position player_move(struct player *player, int out, uint32_t t)
{
  struct position rel;
  
  /* Ausgangspunkt ist aktuelle Position */
  player->move.start = player->move.end = player->move.now;
  player->rotozoom.start = player->rotozoom.end = player->rotozoom.now;

  /* Zielposition setzen */
  player->move.end.x = player->pos[out].x;
  player->move.end.y = player->pos[out].y;
  
  /* Immer inner Winkel (Keine Rotation während Bewegung) */
  player->rotozoom.end.r = player->angle[out];
  
  /* Start+Endalpha bei einer Bewegung immer gleich (Opaque) */
  player->rotozoom.start.a =
  player->rotozoom.end.a = 1.0;

  /* Zeitskala */
  gfxutil_scaletime(&player->scale, game_ticks, t);

  player_status &= ~PLAYER_FROZEN;

  player_debug(INFO, "Spieler %s nach %i|%i (Winkel %3i) schieben in %umsec",
               player->name, player->move.end.x, 
               player->move.end.y, (int)player->rotozoom.end.r);
  
  rel.x = player->move.end.x - player->move.start.x;
  rel.y = player->move.end.y - player->move.start.y;
  
  return rel;
}

/* -------------------------------------------------------------------------- *
 * Stellt fest ob sich die angegebende Position innerhalb des Schupf-Kreises  *
 * des angegebenen Spielers befindet                                          *
 * -------------------------------------------------------------------------- */
int player_isat(struct player *player, int x, int y)
{
  /* Innerhalb des Rechtecks? */
  if(gfxutil_matchrect(&player->tr, x, y) && player->tf)
  {
    struct color color;
    
    x -= player->tr.x;
    y -= player->tr.y;

    /* Wenn innerhalb des Rechtecks, dann Alpha-Layer überprüfen */
    color = gfxutil_getpixel(player->tf, x, y);
    
    if(color.a)
      return 1;
  }

  return 0;
}

/* -------------------------------------------------------------------------- *
 * Gibt den Spieler an der angegebenen Position zurück, oder NULL falls dort  *
 * keiner ist.                                                                *
 * -------------------------------------------------------------------------- */
struct player *player_at(int x, int y)
{
  struct player *player;
  
  dlink_foreach(&player_list, player)
  {
    if(player_isat(player, x, y))
      return player;
  }
  

  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Setzt das Zielrechteck                                                     *
 * -------------------------------------------------------------------------- */
void player_center(struct player *player)
{
  player->sr.w = player->sf->w;
  player->sr.h = player->sf->h;
  player->tr.w = player->tf->w;
  player->tr.h = player->tf->h;
  
  gfxutil_centerxy(&player->tr, player->move.now.x, player->move.now.y);
}

/* -------------------------------------------------------------------------- *
 * Setzt ein Statusflag                                                       *
 * -------------------------------------------------------------------------- */
void player_set(struct player *player, int flags)
{
  /* Aktionen nur ausführen wenn das Flag nicht schon gesetzt ist */
  if((player->status & flags) == 0)
  {
    player_debug(DETAILS, "Set [%s%s%s%s%s ] (%s%s%s%s%s )",
                 ((~player->status) & flags & PLAYER_HILITE) ? " HILITE" : "",
                 ((~player->status) & flags & PLAYER_RENDER) ? " RENDER" : "",
                 ((~player->status) & flags & PLAYER_TRANSFORM) ? " TRANSFORM" : "",
                 ((~player->status) & flags & PLAYER_SCHUPF) ? " SCHUPF" : "",
                 ((~player->status) & flags & PLAYER_SENT) ? " SENT" : "",
                 ((player->status | flags) & PLAYER_HILITE) ? " HILITE" : "",
                 ((player->status | flags) & PLAYER_RENDER) ? " RENDER" : "",
                 ((player->status | flags) & PLAYER_TRANSFORM) ? " TRANSFORM" : "",
                 ((player->status | flags) & PLAYER_SCHUPF) ? " SCHUPF" : "",
                 ((player->status | flags) & PLAYER_SENT) ? " SENT" : "");

    player_status |= PLAYER_REDRAW;
    player->status |= flags|PLAYER_RENDER;
  }
}
  
/* -------------------------------------------------------------------------- *
 * Löscht ein Statusflag                                                      *
 * -------------------------------------------------------------------------- */
void player_unset(struct player *player, int status)
{
  /* Aktionen nur ausführen wenn das Flag gesetzt ist */
  if(player->status & status)
  {
    player_status |= PLAYER_REDRAW;
    player->status &= ~status;
    player->status |= PLAYER_RENDER;
    
    if(status & PLAYER_HILITE)
      player_debug(INFO, "Spieler %s abgewählt", player->name);
  }
}  

/* -------------------------------------------------------------------------- *
 * Alle Spieler abwählen                                                      *
 * -------------------------------------------------------------------------- */
void player_deselect(void)
{
  struct player *player;
  
  dlink_foreach(&player_list, player)
    if(player->status & PLAYER_HILITE)
      player_unset(player, PLAYER_HILITE);  
}

/* -------------------------------------------------------------------------- *
 * Legt Karten auf dem Schupf-Kreis ab und schiebt diesen zusammen mit der    *
 * Karte aus dem Bild                                                         *
 * -------------------------------------------------------------------------- */
void player_drop(struct player *player)
{
  struct node *node;
  struct node *next;
  struct card *card;

  player_debug(INFO, "Karten ablegen (%s)", player->name);
      
  /* Gedraggte Karten durchgehen */
  dlink_foreach_safe_data(&dnd_list, node, next, card)
  {
    struct position move;
    
    /* Von der Fächer und DnD-Liste nehmen */
    fan_remove(card);
    dnd_remove(card);
    
    /* Auf die Spieler-Liste adden */
    dlink_add(&player->cards, &card->is[CARD_PLAYER].node, card);

    /* Ziel des Schupfkreises wieder auf äussere Position zurücksetzen */
    move = player_move(player, PLAYER_OUT, PLAYER_MDELAY);
    
    /* Aktuellen Dragvektor der Karte übernehmen */
    card_setvector(card, CARD_PLAYER, 
                   card->is[CARD_DND].move.now,
                   card->is[CARD_DND].rotozoom.now);

    /* Karte ans selbe Ziel schieben wie den Spieler */
    card_translate(card, CARD_PLAYER,
                   card->is[CARD_PLAYER].move.start.x + move.x,
                   card->is[CARD_PLAYER].move.start.y + move.y);

    card_blend(card, gfxutil_white, PLAYER_MDELAY / 2);
    
    /* ...in der selben Zeit wie der Spieler */
    card_setrange(card, CARD_PLAYER, player->scale.start, player->scale.end);
    
    player->status &= ~PLAYER_SCHUPF;
    
    /* Farbe des Spielers setzen */
//    card->tint = player->blend.now;
    card->status |= CARD_RENDER;
  }
  
  /* Fächer neu berechnen und aktivieren */
  fan_calc();
  
  /* Players redrawen */
  player_status &= ~PLAYER_FROZEN;
  player_status |= PLAYER_REDRAW;
}

/* -------------------------------------------------------------------------- *
 * Generiert einen eingefärbten Kreis in der Spielerfarbe                     *
 * -------------------------------------------------------------------------- */
void player_render(struct player *player)
{
  struct color c;
  
  /* 32-bit transparentes Surface erstellen wenn nötig */
  if(player->sf == NULL)
    player->sf = SDL_CreateRGBSurface(SDL_HWSURFACE, 
                                          player_image->w, player_image->h,
                                          32, RMASK, GMASK, BMASK, AMASK);
  c = *(struct color *)&player->blend.now;
  
  c.a = (player->status & PLAYER_HILITE) ? 255 : 128;
 
  player_debug(DETAILS, 
               "Render [ rot = %3u, grün = %3u, blau = %3u, alpha = %3u ] (%s)",
               c.r, c.g, c.b, c.a, player->name);
  
  /* Gefärbtes blitten */
  gfxutil_tint(player_image, NULL, player->sf, NULL, c);
  
  sgDrawTextOutline(client_font[1], player->sf, NULL,
                    SG_ALIGN_CENTER|SG_ALIGN_MIDDLE, player->name);
  
  player->status &= ~PLAYER_RENDER;
  player->status |= PLAYER_TRANSFORM;
}
  
/* -------------------------------------------------------------------------- *
 * Transformiert den Spielerkreis                                             *
 * -------------------------------------------------------------------------- */
void player_transform(struct player *player)
{
  player_debug(VERBOSE, "Transform [ blend = %3u, winkel = %4i° ] (%s)",
               (int)(player->rotozoom.now.a * 255), 
               (int)player->rotozoom.now.r, player->name);
  
  /* Da ist bereits ein Surface? */
  if(player->tf)
    SDL_FreeSurface(player->tf);
  
  /* Transformation ausführen */
  player->tf = gfxutil_rotozoom(player->sf,
                                player->rotozoom.now.r,
                                player->rotozoom.now.z,
                                player->rotozoom.now.a * 255);
  
  player->status &= ~PLAYER_TRANSFORM;
}

/* -------------------------------------------------------------------------- *
 * Zeichnet einen Schupfkreis und die dazugehörigen Karten auf den Screen     *
 * -------------------------------------------------------------------------- */
void player_blit(struct player *player, SDL_Surface *surface)
{
  SDL_Rect drect;
  struct node *node;
  
  /* Den (transformierten) Kreis auf den Screen zeichnen */
  drect = player->tr;
  SDL_BlitSurface(player->tf, NULL, surface, &drect);
  
  /* Und dann die darüberliegenden Karten */
  dlink_foreach(&player->cards, node)
    card_blit(node->data, CARD_PLAYER, surface);
 
  player_debug(DETAILS, "Blit [ x =%4i, y =%4i, w =%4u, h =%4u ] (%s)",
               drect.x, drect.y, drect.w, drect.h, player->name);
}

/* -------------------------------------------------------------------------- *
 * Alle Player (Schupfkreise und Karten) neu zeichnen                         *
 * -------------------------------------------------------------------------- */
void player_redraw(SDL_Surface *surface)
{
  struct player *player;
  
  /* Nicht zeichnen wenn versteckt */
  if(player_status & PLAYER_HIDDEN)
    return;
  
  /* Spieler neu rendern falls nötig */
  dlink_foreach(&player_list, player)
  {
    if(player->status & PLAYER_RENDER)
      player_render(player);
  }

  /* ...neu transformieren falls nötig */
  dlink_foreach(&player_list, player)
  {
    if(player->status & PLAYER_TRANSFORM)
      player_transform(player);
  }
  
  /* Dann alle Spieler neu blitten */
  dlink_foreach(&player_list, player)
  {
    if(player->index)
    {
      player_center(player);
      player_blit(player, surface);
    }
  }
  
  player_status &= ~PLAYER_REDRAW;
}

/* -------------------------------------------------------------------------- *
 * Beginnt mit dem Schupfen. Alle Winkel und Positionen werden erst berechnet *
 * -------------------------------------------------------------------------- */
void player_start(void)
{
  struct player *player;

  /* Durch die Playerliste gehen */
  dlink_foreach(&player_list, player)
  {
    /* Winkel kalkulieren */
    player_calc(player);
    
    /* Lokaler Spieler wird nicht angezeigt */
    if(player->index == 0)
      continue;

    /* Positionen und Farbe kalkulieren */
    player_setpos(player, PLAYER_IN);
    player_fade(player, 255, PLAYER_FDELAY);
    
    player->status |= PLAYER_SCHUPF;
  }
  
  /* Entsperren und neu zeichnen */
  player_status &= ~PLAYER_FROZEN;
  player_status |= PLAYER_REDRAW;
}

/* -------------------------------------------------------------------------- *
 * Zurückschupfen.                                                            *
 * -------------------------------------------------------------------------- */
void player_return(struct player *player, struct card *card)
{
  dlink_add_tail(&player->cards, &card->is[CARD_PLAYER].node, card);
  
  /* Karte auf äussere Position */
  card_setpos(card, CARD_PLAYER, player->move.now.x, player->move.now.y);
  card_setangle(card, CARD_PLAYER, player->angle[PLAYER_IN]);
  
  /* Karte und Spielerkreis nach innen schieben */
  player_move(player, PLAYER_IN, PLAYER_MDELAY);  
  card_translate(card, CARD_PLAYER, player->move.end.x, player->move.end.y);
  
  card_setcolor(card, player->blend.start);
  card_blend(card, player->blend.end, PLAYER_MDELAY * 2);
  
  card_render(card);
  /* Karte synchron zum Kreis */
  card->is[CARD_PLAYER].scale = player->scale;
  
  player_status &= ~(PLAYER_FROZEN|PLAYER_HIDDEN);
  player_status |= PLAYER_RETURN;
}

/* -------------------------------------------------------------------------- *
 * Berechnet Winkel der gedraggten Karte anhand den Distanzverhältnissen der  *
 * verschiedenen Schupfkreisen                                                *
 *                                                                            *
 * Bisschen hacky und ungenau das Teil hier, aber tut seinen Zweck            *
 * -------------------------------------------------------------------------- */
float player_dragangle(int x, int y, int sdist, float sangle)
{
  struct player *player, *p = NULL ;
  uint32_t total = 65535 / (sdist + 1);
  uint32_t dist;
  float angle = 0.0;
  uint32_t near;
  int n = 0;
  
  /* Zuerst alle Näherungswerte zusammenrechnen */
  dlink_foreach(&player_list, player)
  {
    if((player->status & PLAYER_SCHUPF) == 0) 
      continue;
    
    dist = gfxutil_dist(x - player->move.now.x, y - player->move.now.y) + 1;
    near = 65535 / (dist + 1);
    total += near;
    p = player;
    n++;
  }

  /* Die etwas einfachere Variante wenn wir nur noch einem
     Spieler schupfen müssen */
  if(n == 1)
  {
    dist = gfxutil_dist(x - p->pos[PLAYER_IN].x, y - p->pos[PLAYER_OUT].y);
    total = dist + sdist;
    
//    player_dump(p);
    
    angle = ((sangle * (float)dist) + (p->rotozoom.end.r * (float)sdist)) / total;
  }
  else
  {
    /* Ansonsten für jeden Player das Verhältnis seiner [Nähe zur Drag-Position]
       zu der [totalen Nähe] mit dessen Winkel multiplizieren, damit wir
       also einen Zwischenwert aller Playerwinkel und dem Ausganswinkel erhalten */
    dlink_foreach(&player_list, player)
    {
      if((player->status & PLAYER_SCHUPF) == 0) 
        continue;
      
      dist = gfxutil_dist(x - player->move.now.x, y - player->move.now.y) + 1;
      near = 65535 / (dist + 1);
      angle += player->rotozoom.end.r * (float)near / (float)total;
    }
  
    near = 65535 / (sdist + 1);
    angle += sangle * (float)near / (float)total;
  }

  player_debug(DETAILS, "Drag & Drop Winkel %.3f", angle);
  
  return angle;
}

/* -------------------------------------------------------------------------- *
 * Behandelt Mausgeklicke                                                     *
 * -------------------------------------------------------------------------- */
int player_button(uint8_t type, uint8_t button, uint8_t state, 
                  int16_t x, int16_t y)
{
  /* Wurde die linke Maustaste losgelassen? */
  if(!(state & SDL_BUTTON_LMASK) &&
     (button == SDL_BUTTON_LEFT))
  {
    /* Über einem Kreis? */
    if(player_hilite)
    {
      player_drop(player_hilite);
      player_deselect();
      dnd_stop();
      return 1;
    }
 }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Behandelt Mausbewegungen                                                   *
 * -------------------------------------------------------------------------- */
int player_motion(uint8_t type, uint8_t state, int16_t x, int16_t y,
                  int16_t xrel, int16_t yrel)
{
  struct player *player = player_at(x, y);

  if(fan_dragging())
  {
    /* Wir sind im Dragging-Modus, nun prüfen wir ob 
       an der aktuellen Position ein Schupfkreis ist,
       welcher noch eine Karte braucht */
    /* Wenn wir uns über dem Fächer befinden, welcher eine 
       höhere Priorität hat, dann ignorieren wir die Bewegung hier */
    if(player && player->index && (player->status & PLAYER_SCHUPF) && !fan_at(x, y))
    {
      /* Sonst: Kreis Hiliten */
      if(player_hilite != player)
      {
        player_deselect();
        player_set(player, PLAYER_HILITE);
        player_hilite = player;
      }
      
      /* ...und dann Fächer neu zeichnen */
      fan_set(FAN_REDRAW);
    }
  }
  
  if(!fan_dragging() || player == NULL)
  {
    player_deselect();
    player_hilite = NULL;
  }
  
  return player_status;
}

/* -------------------------------------------------------------------------- *
 * Behandelt allgemeine Events und leitet sie weiter and Subroutinen          *
 * -------------------------------------------------------------------------- */
int player_event(SDL_Event *event)
{
  /* Player wurde gesperrt - keine Events behandeln */
  if(player_status & PLAYER_LOCKED)
    return player_status;
  
  switch(event->type)
  {
    case SDL_MOUSEMOTION:
    {
      return player_motion(event->motion.type,
                           event->motion.state,
                           event->motion.x, event->motion.y,
                           event->motion.xrel, event->motion.yrel);
      break;
    }
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
    {
      return player_button(event->button.type,
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
      client_shutdown();
    }
  }
  
  return 1;
}

/* -------------------------------------------------------------------------- *
 * Checkt ob jedem Spieler geschupft wurde.                                   *
 * -------------------------------------------------------------------------- */
int player_check(void)
{
  int done = 1;
  struct player *player;

  /* Tja dann mal die Players durchgehen */
  dlink_foreach(&player_list, player)
  {
    if(!player->index)
      continue;

    /* Schupfung noch nicht gesendet, wir sind also noch nicht durch! */
    if((player->status & PLAYER_SENT) == 0)
    {
      /* Endposition erreicht? */
      if(player->move.now.x == player->pos[PLAYER_OUT].x &&
         player->move.now.y == player->pos[PLAYER_OUT].y)
      {
        action_schupf(player);
        continue;
      }
      
      done = 0;
    }
  }
    
  return done;
}

/* -------------------------------------------------------------------------- *
 * Karte eines Gegen- oder Mitspielers entgegennehmen und in Fächer adden     *
 * -------------------------------------------------------------------------- */
void player_receive(struct player *player)
{
  struct node *node;
  struct card *card = NULL;
  struct range scale;
  int n = 0;
  float x = 0.0, y = 0.0;

  player_fade(player, 0, PLAYER_FDELAY);

  /* Karten in den Fächer adden und dann von der 
     aktuellen Position aus verschieben */
  dlink_foreach_data(&player->cards, node, card)
  {
    fan_add(card);

    card_setvector(card, CARD_FAN, player->move.now, player->rotozoom.now);

    n++;
  }

  /* Fächer neu kalkulieren */
  fan_calc();
  fan_unset(FAN_DISABLED);
  
  /* Zentrum der Karten errechnen */
  dlink_foreach_data(&player->cards, node, card)
  {
    x += (float)card->is[CARD_FAN].move.end.x / (float)n;
    y += (float)card->is[CARD_FAN].move.end.y / (float)n;
    scale = card->is[CARD_FAN].scale;
  }

  /* Schupfkreis auch dort hin schieben */
  player->move.start = player->move.now;
  player->move.end.x = x;
  player->move.end.y = y;
  player->scale = scale;
  
  /* Karten vom Spieler unlinken, sollten jetzt nur noch im Fächer sein */
  dlink_list_zero(&player->cards);

  fan_unset(FAN_LOCKED);
  fan_set(FAN_REDRAW);
}

/* -------------------------------------------------------------------------- *
 * Spielerpositionen und -transformationen aktualisieren und dabei die        *
 * nötigen Redraw-Flags setzen                                                *
 * -------------------------------------------------------------------------- */
int player_update(uint32_t t)
{
  struct player *player;
  struct card *card;
  struct node *node;
  int done = 0;
  
  /* Nicht aktualisieren wenn eingefroren */
  if(player_status & PLAYER_FROZEN)
    return player_status;
  
  dlink_foreach(&player_list, player)
  {
    if(player->index)
    {
      /* Die neuen Parameter werden erst temporär hier gespeichert, weil
         wir sie dann mit den alten vergleichen um den Redraw-Status zu
         aktualisieren */
      struct position move;
      struct transform rotozoom;
      struct color blend;
      uint16_t scale16;
    
      /* Zeitskala updaten */
      done += gfxutil_scaleadd(&player->scale, t);
        
      /* 16-bit Skalenwert generieren... */
      scale16 = gfxutil_scale16(&player->scale);
      
      /* ...und damit die Translation, Transformation und das Blending updaten */
      gfxutil_translate(&move, player->move.start, player->move.end, scale16);
      gfxutil_transform(&rotozoom, player->rotozoom.start, player->rotozoom.end, scale16);
      gfxutil_blend(&blend, player->blend.start, player->blend.end, scale16);
      
      /* Karten mit dem Kreis mitbewegen */
      dlink_foreach_data(&player->cards, node, card)
        if(card_update(card, CARD_PLAYER, t) & CARD_REDRAW)
          player_status |= PLAYER_REDRAW;

      /* Hat Blending geändert? */
      if(gfxutil_diffcolor(blend, player->blend.now))
        player->status |= PLAYER_RENDER;

      /* Hat Transformation geändert? */
      if(gfxutil_difftransform(rotozoom, player->rotozoom.now))
        player->status |= PLAYER_TRANSFORM;
      
      /* Hat Position geändert? */
      if(gfxutil_diffposition(move, player->move.now))
        player->status |= PLAYER_MOVE;
      
      /* Hat irgendwas geändert, müssen wir neu zeichnen */
      if(player->status & (PLAYER_MOVE|PLAYER_TRANSFORM|PLAYER_RENDER))
        player_status |= PLAYER_REDRAW;
      
      /* Die Werte aktualisieren */
      player->move.now = move;
      player->rotozoom.now = rotozoom;
      player->blend.now = blend;
    }
  }
  
  /* Alle Bewegungen abgeschlossen? */
  if(done == player_list.size - 1)
  {
    player_status |= PLAYER_FROZEN;
    player_debug(INFO, "Spielermodul wieder eingefroren");
  }
  
  /* Schupfen oder Schupfungen entgegennehmen?  */
  if((player_status & PLAYER_RETURN) == 0)
  {
    /* Fertig mit Schupfen? */
    if(player_check())
    {
      player_debug(INFO, "Lokaler Spieler hat fertig geschupft!");
      
      player_status |= PLAYER_FROZEN|PLAYER_HIDDEN;
      fan_target = CARD_STACK;
      
#ifdef DEBUG
      /* DEBUG STUFF! */
      if(!net_socket)
      {
        /* Wenn nicht connected, also im Engine Test, dann diese Karten zurückschupfen: */
        player_return(player_list.tail->data, card_get("p2"));
        player_return(player_list.tail->prev->data, card_get("p3"));
        player_return(player_list.tail->prev->prev->data, card_get("p4"));
      }      
#endif /* DEBUG */
    }
  }
  else
  {
    /* Wenn der Schupfkreis an der Endposition ist, 
       dann ausblenden und Karte in den Fächer schieben. */
    struct player *player;
    int done = 1;
    int count = 0; /* Karten die noch in den Fächer müssen */
    
    dlink_foreach(&player_list, player)
    {
      /* Innere Position erreicht, aber wir gehen noch weiter :) */
      if(player->move.now.x == player->pos[PLAYER_IN].x &&
         player->move.now.y == player->pos[PLAYER_IN].y)
      {
        /* Da sind noch Karten die in den Fächer müssen, 
           also schicken wir diese auf Ihren Weg */
        if(player->cards.size)
          player_receive(player);
      }
      
      count += player->cards.size;
      
      /* Solange Schupfkreise noch sichtbar sind warten wir noch */
      if(player->blend.now.a)
        done = 0;
    }
    
    if((stack_status & STACK_HIDDEN))
      player_debug(DETAILS, "Stack versteckt, count = %i", count);
    
    /* Der Stack kann jetzt angezeigt werden sobald alle Karten unterwegs sind */
    if(count == 0 && (stack_status & STACK_HIDDEN))
    {

//      stack_start();
      stack_actor(player_list.head->data);
    }
    
    if(done)
    {
      /* Für Insider :) */
      player_debug(INFO, "Die Kompanie hat fertig geschupft!");
      
      /* Spielerzeugs können wir jetzt sperren, 
         brauchen wir erst beim nächsten Spiel wieder */
      player_status |= PLAYER_LOCKED|PLAYER_HIDDEN|PLAYER_FROZEN;
    }
  }
  
  return player_status;
}

/* -------------------------------------------------------------------------- *
 * Playerstruktur ausgeben                                                    *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void player_dump(struct player *player)
{
  struct node *node;
  
  player_debug(INFO, "-- player ------------------------------------------");

  if(player->sf)
    player_debug(INFO, "sf     = %ux%u", 
                 player->sf->w, player->sf->h);
  else
    player_debug(INFO, "sf     = NULL");
  
  if(player->tf)
    player_debug(INFO, "tf     = %ux%u",
                 player->tf->w, player->tf->h);
  else
    player_debug(INFO, "tf     = NULL");
  
  player_debug(INFO, "sr  = %i,%i %ux%u",
               player->sr.x, player->sr.y,
               player->sr.w, player->sr.h);
  
  player_debug(INFO, "tr  = %i,%i %ux%u",
               player->tr.x, player->tr.y,
               player->tr.w, player->tr.h);
  
  player_debug(INFO, "name   = %s", 
               player->name);
  
  player_debug(INFO, "index  = %i",
               player->index);
  
  player_debug(INFO, "status =%s%s%s",
               (player->status & PLAYER_HILITE) ? " HILITE" : "",
               (player->status & PLAYER_RENDER) ? " RENDER" : "",
               (player->status & PLAYER_SCHUPF) ? " SCHUPF" : "");
  
  player_debug(INFO, "tint   = #%02X%02X%02X%02X",
               player->blend.now.r, player->blend.now.g, player->blend.now.b, player->blend.now.a);
  
  dlink_foreach(&player->cards, node)
    card_dump(node->data, CARD_PLAYER);
  
  player_debug(INFO, 
               "move.start = { x = %i, y = %i, z = %.3f }",
               player->move.start.x, player->move.start.y);
  
  player_debug(INFO, 
               "move.now   = { x = %i, y = %i, z = %.3f }",
               player->move.now.x, player->move.now.y);
  
  player_debug(INFO, 
               "move.end   = { x = %i, y = %i, z = %.3f }",
               player->move.end.x, player->move.end.y);
  
  player_debug(INFO, "-- end of player -----------------------------------");

}
#endif /* DEBUG */
