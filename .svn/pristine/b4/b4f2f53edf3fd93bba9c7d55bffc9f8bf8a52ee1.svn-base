/* $Id: card.c,v 1.44 2005/05/21 08:27:20 smoli Exp $
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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>

/* -------------------------------------------------------------------------- *
 * Das [cards] Modul verwaltet alle Karten                                    *
 * -------------------------------------------------------------------------- */
#include "ini.h"
#include "card.h"
#include "game.h"
#include "client.h"
#include "gfxutil.h"

/* -------------------------------------------------------------------------- *
 * Globale Variabeln                                                          *
 * -------------------------------------------------------------------------- */
struct list            card_list;         /* Kartenliste */
struct color           card_tint;         /* Farbe und Alpha */
const char            *card_mods[CARD_MODS] = 
{ 
  "FAN", "STACK", "PLAYER", "DND" 
};

/* -------------------------------------------------------------------------- *
 * Modul-Internes                                                             *
 * -------------------------------------------------------------------------- */
static struct ini     *card_ini;          /* INI file mit karteneigenschaften */
static SDL_Rect        card_mrect;        /* rechteck für den rahmen */
static SDL_Surface    *card_image;        /* surface mit allen karten */
static SDL_Surface    *card_margin;       /* rahmen für selektierte karten */

/* -------------------------------------------------------------------------- *
 * Konfiguration für die Karten                                               *
 * -------------------------------------------------------------------------- */

/* card_config.border           Default Blur-Rate */
#define CARD_DEFAULT_BORDER     128

/* card_config.color            Default Farbtönung */
#define CARD_DEFAULT_COLOR      gfxutil_defhsv

/* card_config.alpha            Default Transparenz */
#define CARD_DEFAULT_ALPHA      128

/* card_config.antialias        Anti-Aliasing */
#define CARD_DEFAULT_ANTIALIAS  1

/* card_config.zoom             Zoomfaktor */
#define CARD_DEFAULT_ZOOM       1.0

/* card_config.image            Bild mit den Karten */
#define CARD_DEFAULT_IMAGE      "cards-alpha.png"

/* card_config.ini              .ini Datei mit Karteninfos */
#define CARD_DEFAULT_INI        "cards.ini"

struct card_config card_config;

/* -------------------------------------------------------------------------- *
 * Initialisiert die Kartenverwaltung                                         *
 * -------------------------------------------------------------------------- */
int card_init(void)
{
  /* Liste initialisieren */
  dlink_list_zero(&card_list);
  
  /* .ini Datei mit Kartendaten öffnen */
  if(!(card_ini = ini_open(card_config.ini, INI_READ)))
  {
    card_log(ERROR, "Konnte Kartendaten nicht laden!");
    return -1;
  }

  /* Bild mit den Karten laden */
  client_load_png(&card_image, card_config.image);
  
  if(card_image->format->BitsPerPixel != 32 ||
     card_image->format->Amask == 0)
    card_log(ERROR, "%s muss ein 32-bit Bild mit Alpha-Kanal sein!", 
             card_config.image);
  
  card_log(STATUS, "%s wurde geladen (%ux%u @ %ubpp)",
           card_config.image, card_image->w, card_image->h,
           card_image->format->BitsPerPixel);
  
  /* Position des Rahmens innerhalb des Bilds */
  /* Letzte Spalte im Bild */
  card_mrect.x = ((card_image->w / CARD_WIDTH) - 1) * CARD_WIDTH;
  card_mrect.y = 0;
  card_mrect.w = CARD_WIDTH;
  card_mrect.h = CARD_HEIGHT;

  /* Surface für Rahmen erstellen */
  card_margin = SDL_CreateRGBSurface(SDL_SWSURFACE, IMAGE_WIDTH, IMAGE_HEIGHT,
                                     32, RMASK, GMASK, BMASK, AMASK);
  gfxutil_copy(card_image, &card_mrect, card_margin, NULL);  
  
  /* Nicht sehr Random, da dieses Modul relativ früh initialisiert wird */
  srand(SDL_GetTicks());
  
  card_log(STATUS, "%u Karten bereit zum Laden...", card_ini->nsections);
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Gibt alle Daten im Zusammenhang mit der Kartenverwaltung wieder frei       *
 * -------------------------------------------------------------------------- */
void card_shutdown()
{
  struct node *next;
  struct card *card;
  
  /* Surfaces freigeben */
  client_free_image(card_margin);
  client_free_image(card_image);
  
  /* Alle Karten freigeben */
  dlink_foreach_safe(&card_list, card, next)
    card_delete(card);

  if(card_ini)
    ini_close(card_ini);
}

/* -------------------------------------------------------------------------- *
 * Liest die Kartenkonfiguration [card] Sektion der angegebenen ini-Datei     *
 * -------------------------------------------------------------------------- */
void card_configure(struct ini *ini)
{
  ini_section(ini, "card");
  
  /* Farbton der Karten des lokalen Spielers */
  card_config.color =
    ini_gethsv_default(ini, "color", CARD_DEFAULT_COLOR);
  
  /* Kartentransparenz */
  card_config.alpha =
    ini_getlong_default(ini, "alpha", CARD_DEFAULT_ALPHA);
  
  /* Rahmendicke für ausgewählte Karten, Blur-Rate von 0..255 */
  card_config.border = 
    ini_getulong_default(ini, "border", CARD_DEFAULT_BORDER);
  
  /* Anti-Aliasing */
  card_config.antialias = 
    ini_getlong_default(ini, "antialias", CARD_DEFAULT_ANTIALIAS);
  
  /* Zoom-Faktor */
  card_config.zoom =
    ini_getdouble_default(ini, "zoom", CARD_DEFAULT_ZOOM);
  
  /* Kartendaten */
  card_config.image = ini_gets_default(ini, "image", CARD_DEFAULT_IMAGE);
  card_config.ini = ini_gets_default(ini, "ini", CARD_DEFAULT_INI);
  
  
  card_tint = gfxutil_hsv2rgb(card_config.color);
  card_tint.a = card_config.alpha;
}

/* -------------------------------------------------------------------------- *
 * Rendert eine Vorschau einer Karte auf eine Surface                         *
 * -------------------------------------------------------------------------- */
SDL_Surface *card_preview(SDL_Rect rect, struct card *card, float angle)
{
  SDL_Surface *preview;
  
  card_setcolor(card, card_tint);
  
//  card->status |= CARD_SEL;
  card_render(card);
  
  preview = gfxutil_rotozoom(card->surface, angle, card_config.zoom, 255);
  
//  card_clean(card);
  
  return preview;  
}
  
/* -------------------------------------------------------------------------- *
 * Eine Kartenstruktur anhand einer .ini Sektion erstellen                    *
 * -------------------------------------------------------------------------- */
struct card *card_new(const char *name)
{
  struct card *card;
  int col, row;
  int i;

  /* Nach der entsprechenden Sektion in der .ini Datei suchen */
  if(ini_section(card_ini, name))
  {
    card_log(ERROR, "Karte %s nicht gefunden", name);
    return NULL;
  }
  
  /* Speicher reservieren */
  card = malloc(sizeof(struct card));

  /* Auf 0 initialisieren */
  memset(card, 0, sizeof(struct card));
  
  card_setcolor(card, card_tint);
  
  /* Namen setzen */
  client_strlcpy(card->name, name, sizeof(card->name));
  
  card->value = ini_getlong(card_ini, "value");

  /* Rechteck der Karte innerhalb card_image berechnen */
  col = ini_getlong(card_ini, "x");
  row = ini_getlong(card_ini, "y");

  card->rect.x = col * CARD_WIDTH;
  card->rect.y = row * CARD_HEIGHT;
  card->rect.w = CARD_WIDTH;
  card->rect.h = CARD_HEIGHT;

  /* Zoom und Alpha initialisieren */
  for(i = 0; i < CARD_MODS; i++)
  {
    card_setzoom(card, i, card_config.zoom);
    card_setalpha(card, i, 1.0);
  }
  
  card_debug(INFO, "Karte %s wurde geladen [%u|%u]", 
             name, col, row);

  /* Auf die globale Kartenliste linken */
  dlink_add_head(&card_list, &card->node, card);
  
  return card;
}

/* -------------------------------------------------------------------------- *
 * Gibt alle mit einer Karte assozierten Daten frei                           *
 * -------------------------------------------------------------------------- */
void card_clean(struct card *card)
{
  int i;
  
  /* Surfaces löschen */
  client_free_image(card->surface);
  
  for(i = 0; i < CARD_MODS; i++)
    card_freeinst(card, i);
}

/* -------------------------------------------------------------------------- *
 * Gibt eine Karte frei                                                       *
 * -------------------------------------------------------------------------- */
void card_delete(struct card *card)
{  
  dlink_delete(&card_list, &card->node);
  
  card_clean(card);
  free(card);
}

/* -------------------------------------------------------------------------- *
 * Die Instanz-Surface freigeben, aber nur wenn Sie in keiner anderen Instanz *
 * Benutzt wird                                                               *
 * -------------------------------------------------------------------------- */
void card_freeinst(struct card *card, int mod)
{
  int i;
  
  for(i = 0; i < CARD_MODS; i++)
  {
    if(i == mod)
      continue;
    
    if(card->is[i].surface == card->is[mod].surface)
    {
      card->is[mod].surface = NULL;
      return;
    }
  }

  client_free_image(card->is[mod].surface);
}

/* -------------------------------------------------------------------------- *
 * Aktuelle Position und Transformation einer Instanz übernehmen              *
 * -------------------------------------------------------------------------- */
void card_copyinst(struct card *card, int from, int to)
{
  card->is[to].move = card->is[from].move;
  card->is[to].rotozoom = card->is[from].rotozoom;
  card->is[to].surface = card->is[from].surface;
}

/* -------------------------------------------------------------------------- *
 * Findet eine Karte nach dem Namen                                           *
 * -------------------------------------------------------------------------- */
struct card *card_find(const char *name)
{
  struct card *card;
  
  dlink_foreach(&card_list, card)    
  {
    if(!strncmp(card->name, name, 3))
       return card;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Karte suchen und laden falls nicht gefunden                                *
 * -------------------------------------------------------------------------- */
struct card *card_get(const char *name)
{
  struct card *card;
  
  if((card = card_find(name)))
    return card;
  
  return card_new(name);
}

/* -------------------------------------------------------------------------- *
 * Findet eine zufällige Karte welche, falls angegeben, noch nicht auf der    *
 * angegebenen Liste ist                                                      *
 * -------------------------------------------------------------------------- */
struct card *card_random(struct list *list)
{
  struct section *section;
  struct card *card;
  
  for(;;)
  {
    /* Eine zufällige ini-Sektion wählen */
    section = (struct section *)
      dlink_index(&card_ini->sections, rand() % card_ini->sections.size);
    
    /* Die erste Sektion ist normalerweise leer */
    if(section->name == NULL)
      continue;
    
    /* Die korrespondierende Karte laden */
    card = card_get(section->name);
    
    /* Wenn eine Liste angegeben wurde, dann jetzt checken ob 
       die zufällige Karte bereits auf dieser Liste ist */
    if(list && dlink_find(list, card))
      continue;
  
    return card;
  }
}

/* -------------------------------------------------------------------------- *
 * Setzt Karten-Statusflags                                                   *
 * -------------------------------------------------------------------------- */
int card_set(struct card *card, int flags)
{
  /* Müssen wir noch flags setzen? */
  if((card->status & flags) != flags)
  {
    /* Neu rendern wenn hilite oder selected geändert hat */
    if((card->status | (flags & (CARD_SEL|CARD_HILITE))) != card->status)
      card->status |= CARD_RENDER;
    
    /* Flags setzen */
    card->status |= flags;
    
    card_debug(DETAILS, "Set [%s%s%s ]",
               (card->status & CARD_HILITE) ? " HILITE" : "",
               (card->status & CARD_SEL) ? " SEL" : "",
               (card->status & CARD_RENDER) ? " RENDER" : "",
               card->name);
    return 1;
  }
  
  return 0;
}
  
/* -------------------------------------------------------------------------- *
 * Löscht Karten-Statusflags                                                  *
 * -------------------------------------------------------------------------- */
int card_unset(struct card *card, int flags)
{
  /* Müssen wir noch flags löschen? */
  if(card->status & flags)
  {
    /* Neu rendern wenn hilite oder selected geändert hat */
    if((card->status & ~(flags & (CARD_SEL|CARD_HILITE))) != card->status)
      card->status |= CARD_RENDER;
    
    /* Flags löschen */
    card->status &= ~flags;
    
    card_debug(DETAILS, "Unset [%s%s%s ] (%s)",
               (card->status & CARD_HILITE) ? " HILITE" : "",
               (card->status & CARD_SEL) ? " SEL" : "",
               (card->status & CARD_RENDER) ? " RENDER" : "",
               card->name);
    return 1;
  }
  
  return 0;
}
  
/* -------------------------------------------------------------------------- *
 * Setzt die Kartenposition                                                   *
 * -------------------------------------------------------------------------- */
int card_setpos(struct card *card, int mod, int16_t x, int16_t y)
{
  struct position move;
  
  move.x = x;
  move.y = y;
  
  /* Redrawen wenn Position geändert hat */
  if(gfxutil_diffposition(card->is[mod].move.now, move))
    card->is[mod].status |= CARD_MOVE;    
  
  card->is[mod].move.start =
  card->is[mod].move.now =
  card->is[mod].move.end = move;
  
  card_debug(VERBOSE, "Set pos [ x = %3i, y = %3i ] (%s)",
             move.x, move.y, card->name);
  
  return card->is[mod].status;
}

/* -------------------------------------------------------------------------- *
 * Setzt den aktuellen Winkel                                                 *
 * -------------------------------------------------------------------------- */
int card_setangle(struct card *card, int mod, float angle)
{
  struct transform rotozoom;
  
  rotozoom = card->is[mod].rotozoom.now;
  rotozoom.r = angle;
  
  /* Retransformieren wenn Winkel geändert hat */
  if(gfxutil_difftransform(card->is[mod].rotozoom.now, rotozoom))
    card->is[mod].status |= CARD_TRANSFORM;
  
  /* Neuen Winkel setzen */
  card->is[mod].rotozoom.start =
  card->is[mod].rotozoom.now =
  card->is[mod].rotozoom.end = rotozoom;
  
  card_debug(VERBOSE, "Set angle [ r = %.3f ] (%s)",
             rotozoom.r, card->name);
  
  /* Status zurückgeben */
  return card->is[mod].status;
}

/* -------------------------------------------------------------------------- *
 * Setzt den aktuellen Zoomfaktor                                             *
 * -------------------------------------------------------------------------- */
int card_setzoom(struct card *card, int mod, float zoom)
{
  struct transform rotozoom;
  
  rotozoom = card->is[mod].rotozoom.now;
  rotozoom.z = zoom;
  
  /* Retransformieren wenn Zoom geändert hat */
  if(gfxutil_difftransform(card->is[mod].rotozoom.now, rotozoom))
    card->is[mod].status |= CARD_TRANSFORM;
  
  /* Neues Zoom setzen */
  card->is[mod].rotozoom.start =
  card->is[mod].rotozoom.now =
  card->is[mod].rotozoom.end = rotozoom;
  
  card_debug(DETAILS, "Set zoom [ z = %.3f ] (%s)",
             card->is[mod].rotozoom.now.z, card->name);
  
  /* Status zurückgeben */
  return card->is[mod].status;
}

/* -------------------------------------------------------------------------- *
 * Setzt den aktuellen Alphawert                                              *
 * -------------------------------------------------------------------------- */
int card_setalpha(struct card *card, int mod, float alpha)
{
  struct transform rotozoom;
  
  rotozoom = card->is[mod].rotozoom.now;
  rotozoom.a = alpha;
  
  /* Retransformieren wenn Alpha geändert hat */
  if(gfxutil_difftransform(card->is[mod].rotozoom.now, rotozoom))
    card->is[mod].status |= CARD_TRANSFORM;
  
  /* Neues Alpha setzen */
  card->is[mod].rotozoom.start =
  card->is[mod].rotozoom.now =
  card->is[mod].rotozoom.end = rotozoom;
  
  card_debug(DETAILS, "Set alpha [ a = %.3f ] (%s)",
             card->is[mod].rotozoom.now.a, card->name);
  
  /* Status zurückgeben */
  return card->is[mod].status;
}

/* -------------------------------------------------------------------------- *
 * Setzt Kartenfarbe                                                          *
 * -------------------------------------------------------------------------- */
int card_setcolor(struct card *card, struct color color)
{
  /* Neu Rendern wenn Farbe geändert hat */
  if(gfxutil_diffcolor(card->blend.now, color))
    card->status |= CARD_RENDER;
  
  card->blend.start =
  card->blend.now =
  card->blend.end = color;
  
  card_debug(DETAILS, "Set color [ #%02x%02x%02x%02x ] (%)",
             color.r, color.g, color.b, color.a, card->name);
  
  /* Status zurückgeben */
  return card->status;
}

/* -------------------------------------------------------------------------- *
 * Setzt Kartenposition und Winkel                                            *
 * -------------------------------------------------------------------------- */
int card_setvector(struct card *card, int mod,
                   struct position move, struct transform rotozoom)
{
  /* Redrawen wenn Position geändert hat */
  if(gfxutil_diffposition(card->is[mod].move.now, move))
    card->is[mod].status |= CARD_MOVE;
  
  /* Retransformieren? */
  if(gfxutil_difftransform(card->is[mod].rotozoom.now, rotozoom))
    card->is[mod].status |= CARD_TRANSFORM;
  
  card->is[mod].move.start = 
  card->is[mod].move.now = 
  card->is[mod].move.end = move;
  
  card->is[mod].rotozoom.start = 
  card->is[mod].rotozoom.now = 
  card->is[mod].rotozoom.end = rotozoom;
  
  card_debug(VERBOSE, "Set vector [ x = %3i, y = %3i, r = %4i, z = %.3f, a = %.3f ] (%s)",
             card->is[mod].move.now.x, card->is[mod].move.now.y,
             (int)card->is[mod].rotozoom.now.r,
             card->is[mod].rotozoom.now.z,
             card->is[mod].rotozoom.now.a,
             card->name);
  
  return card->is[mod].status;
}

/* -------------------------------------------------------------------------- *
 * Setzt die Zielposition einer Karte.                                        *
 * Gibt die noch zurückzulegende Distanz in Pixel zurück.                     *
 * -------------------------------------------------------------------------- */
uint32_t card_translate(struct card *card, int mod, int16_t x, int16_t y)
{
  int dx, dy;
  uint32_t dist;
  
  /* Deltas ausrechnen */
  dx = card->is[mod].move.start.x - x;
  dy = card->is[mod].move.start.y - y;
  
  /* Aus den Deltas quadratisch die Distanz ausrechnen */
  dist = gfxutil_dist(dx, dy);
  
  /* Neue Zielposition setzen */
  card->is[mod].move.end.x = x;
  card->is[mod].move.end.y = y;
  
  card_debug(VERBOSE, "Translate [ mod = %-5s, x = %3i, y = %3i ] (%s)",
             card_mods[mod], card->is[mod].move.end.x, card->is[mod].move.end.y, 
             card->name);

  card->is[mod].status &= ~CARD_FROZEN;
  
  return dist;
}

/* -------------------------------------------------------------------------- *
 * Schiebt die Karte von der aktuellen Position an die angegebene Position    *
 * mit einer Geschwindigkeit von 'v' Pixeln pro Sekunde (0 = sofort)          *
 * -------------------------------------------------------------------------- */
uint32_t card_move(struct card *card, int mod, int16_t x, int16_t y, uint32_t v)
{
  uint32_t d;
  uint32_t t = 0;
  
  /* Von der aktuellen Position ausgehend */
  card->is[mod].move.start = card->is[mod].move.now;
  
  /* Translation initieren, gibt die Distanz zurück */
  d = card_translate(card, mod, x, y);
  
  /* Zeit für die Translation setzen */
  if(v == 0)
    card_setpos(card, mod, x, y);
  else
    t = d * 1000 / v;

  gfxutil_scaletime(&card->is[mod].scale, game_ticks, t);

  card_debug(INFO, "Move [ mod = %-5s, x = %3i, y = %3i, v = %4u, dist = %u ] (%s)",
             card_mods[mod],
             card->is[mod].move.end.x, card->is[mod].move.end.y, 
             v, d, card->name);
  return d;
}

/* -------------------------------------------------------------------------- *
 * Setzt den Winkel, welchen die Karte erreichen soll                         *
 * -------------------------------------------------------------------------- */
int card_rotate(struct card *card, int mod, float angle)
{
  if(card->is[mod].scale.end == card->is[mod].scale.start)
    return card_setangle(card, mod, angle);
  
  card->is[mod].rotozoom.end.r = angle;
  
  card_debug(INFO, "Rot. [ mod = %-5s, a = %3i° ] (%s)",
             card_mods[mod],
             (int)card->is[mod].rotozoom.end.r, card->name);

  card->is[mod].status &= ~CARD_FROZEN;
  
  return card->is[mod].status;
}

/* -------------------------------------------------------------------------- *
 * Setzt den Zoomwert, welchen die Karte innerhalb von 't' Millisekunden      *
 * erreichen soll.                                                            *
 * -------------------------------------------------------------------------- */
int card_zoom(struct card *card, int mod, float zoom, uint32_t t)
{
/*  if(card->is[mod].scale.end == card->is[mod].scale.start)
    return card_setzoom(card, mod, zoom);*/
  
  card->is[mod].rotozoom.end.z = zoom;
  
  card_debug(INFO, "Zoom [ mod = %-5s, z = %.3f ] (%s)",
             card_mods[mod],
             card->is[mod].rotozoom.end.z, card->name);
  
  card->is[mod].status &= ~CARD_FROZEN;
  
  return card->is[mod].status;
}

/* -------------------------------------------------------------------------- *
 * Setzt den Farbwert, welchen die Karte innerhalb von 't' Millisekunden      *
 * erreichen soll.                                                            *
 * -------------------------------------------------------------------------- */
int card_blend(struct card *card, struct color color, uint32_t t)
{
  card->blend.end = color;

  gfxutil_scaletime(&card->scale, game_ticks, t);
  
  card_debug(INFO, "Blend [ r = %3u, g = %3u, b = %3u, a = %3u ] (%s)",
             color.r, color.g, color.b, color.a, card->name);
  
  card->status &= ~CARD_FROZEN;
  
  return card->status;
}

/* -------------------------------------------------------------------------- *
 * Setzt die Zeitspanne oder Mausdistanz innerhalb der die Bewegung statt-    *
 * findet                                                                     *
 * -------------------------------------------------------------------------- */
void card_setrange(struct card *card, int mod, uint32_t start, uint32_t end)
{
  card->is[mod].scale.start = 
  card->is[mod].scale.now = start;
  card->is[mod].scale.end = end;
  
  card_debug(INFO, "Set range [ mod = %s, start = %5u, now = %5u, end = %5u ] (%s)",
             card_mods[mod],
             card->is[mod].scale.start, 
             card->is[mod].scale.now, 
             card->is[mod].scale.end, card->name);
}

/* -------------------------------------------------------------------------- *
 * Ist die Karte an der angegebenen Position?                                 *
 * -------------------------------------------------------------------------- */
int card_isat(struct card *card, int mod, int16_t x, int16_t y)
{
  /* Innerhalb des Kartenrechtecks? */
  if(gfxutil_matchrect(&card->is[mod].rect, x, y))
  {
    struct color color;
    
    /* Hat das Pixel an der entsprechenden Position einen Alpha-Wert? */
    color = gfxutil_getpixel(card->is[CARD_FAN].surface,
                             x - card->is[CARD_FAN].rect.x,
                             y - card->is[CARD_FAN].rect.y);
    if(color.a)
      return 1;
  }

  return 0;
}

/* -------------------------------------------------------------------------- *
 * Updated Kartenpositionen und Winkel                                        *
 *                                                                            *
 * 't' ist die aktuelle Position/Zeit                                         *
 * -------------------------------------------------------------------------- */
int card_update(struct card *card, int mod, uint32_t t)
{
  struct color blend;
  struct position move;
  struct transform rotozoom;
  uint16_t bscale16;
  uint16_t iscale16;
  int done = 0;

  if(card->is[mod].status & CARD_FROZEN)
    return card->status|card->is[mod].status;
  
  /* Zeitskalen setzen (nicht adden, da diese Funktion
     mehrfach, für jede Instanz aufgerufen werden könnte) */
  done = gfxutil_scaleset(&card->scale, game_ticks);
  done &= gfxutil_scaleset(&card->is[mod].scale, game_ticks);
  
  /* 16-bit Skalenwerte generieren... */
  bscale16 = gfxutil_scale16(&card->scale);
  iscale16 = gfxutil_scale16(&card->is[mod].scale);
  
  card_debug(VERBOSE, "Update [ mod = %s, bscale = %u, iscale = %i ] (%s)",
             card_mods[mod], bscale16, iscale16, card->name);
  
  /* ...und damit die Translation, Transformation und das Blending updaten */
  gfxutil_translate(&move, 
                    card->is[mod].move.start, 
                    card->is[mod].move.end, iscale16);
  
  gfxutil_transform(&rotozoom,
                    card->is[mod].rotozoom.start, 
                    card->is[mod].rotozoom.end, iscale16);
  
  gfxutil_blend(&blend,
                card->blend.start, 
                card->blend.end, bscale16);
  
  /* Hat Blending geändert? */
  if(gfxutil_diffcolor(blend, card->blend.now))
  {
    card_debug(DETAILS, "Blend -> %02x%02x%02x%02x",
               blend.r, blend.g, blend.b, blend.a);
    
    card->status |= CARD_RENDER;
  }
  /* Hat Transformation geändert? */
  else if(gfxutil_difftransform(rotozoom, card->is[mod].rotozoom.now))
  {
    card_debug(DETAILS, "Transform -> r = %.3f, z = %.3f, a = %.3f",
               rotozoom.r, rotozoom.z, rotozoom.a);
    
    card->is[mod].status |= CARD_TRANSFORM;
  }
  /* Hat Position geändert? */
  else if(gfxutil_diffposition(move, card->is[mod].move.now))
  {
    card_debug(DETAILS, "Move -> %i|%i",
               move.x - card->is[mod].move.now.x, 
               move.y - card->is[mod].move.now.y);
    
    card->is[mod].status |= CARD_MOVE;
  }

  /* Wenn die Bewegung aufgehört hat, dann Karte wieder einfrieren */
  if(done)
    card->is[mod].status |= CARD_FROZEN;

  /* Die Werte aktualisieren */
  card->blend.now = blend;
  card->is[mod].move.now = move;
  card->is[mod].rotozoom.now = rotozoom;
  
  return card->status|card->is[mod].status;
}

/* -------------------------------------------------------------------------- *
 * Färbt eine Karte und errechnet den Alpha-Layer, das Ergebnis wird in ein   *
 * quadratisches Surface, welches sich besser zur späteren Rotation eignet,   *
 * geschrieben                                                                *
 * -------------------------------------------------------------------------- */
void card_render(struct card *card)
{
  SDL_Rect drect;
  struct color tint;
  int i;
  
  /* Wir brauchen ein neues Bild */
  if(card->surface == NULL)
    card->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, IMAGE_WIDTH, IMAGE_HEIGHT, 
                                         32, RMASK, GMASK, BMASK, AMASK);

  /* Kartenrechteck auf Ziel zentrieren */
  drect.w = card->rect.w;
  drect.h = card->rect.h;
  gfxutil_centerrect(&drect, &card->surface->clip_rect);

  SDL_FillRect(card->surface, NULL, 0);
  
  /* Kopieren, evtl. mit Farbtönung und Alpha-Transformation */
  tint = card_selected(card) ? card->blend.now : gfxutil_white;

  gfxutil_tint(card_image, &card->rect, card->surface, &drect, tint);

  card_debug(VERBOSE, "Render [ tint = %s, border = %i ] (%s)",
             gfxutil_strcolor(tint), card_hilited(card), card->name);
  
  /* Wenn Karte ausgewählt und inaktiv ist, dann übermalen wird den Rand.
     Ist sie nicht ausgewählt aber aktiv, dann malen wir einen dickeren 
     schwarzen Rand, andernfalls tun wir nichts */
  tint = card_hilited(card) ? gfxutil_black : card->blend.now;
  
  if(card_hilited(card) || card_selected(card))
    gfxutil_blur(card_image, &card_mrect, card->surface, &drect, tint, 
                 card_hilited(card) ? card_config.border : 2);
  
  /* Wenn wir frisch gerendert haben müssen wir danach transformieren */
  card->status &= ~CARD_RENDER;
  
  for(i = 0; i < CARD_MODS; i++)
    card->is[i].status |= CARD_TRANSFORM;
}

/* -------------------------------------------------------------------------- *
 * Transformiert eine Karte                                                   *
 * -------------------------------------------------------------------------- */
void card_transform(struct card *card, int mod)
{
  /* Da ist bereits ein Surface? */
  if(card->is[mod].surface)
    SDL_FreeSurface(card->is[mod].surface);

  card_debug(VERBOSE, "Transform [ mod = %s, angle = %3i, zoom = %.3f ] (%s)",
             card_mods[mod], (int)card->is[mod].rotozoom.now.r,
             card->is[mod].rotozoom.now.z, card->name);
  
  /* Transformation ausführen */
  card->is[mod].surface =
    gfxutil_rotozoom(card->surface, card->is[mod].rotozoom.now.r, 
                     card->is[mod].rotozoom.now.z, 255);
  
  card->is[mod].status &= ~CARD_TRANSFORM;
}
  
/* -------------------------------------------------------------------------- *
 * Zentriert das Zielrechteck einer Karte auf die aktuelle Kartenposition     *
 * -------------------------------------------------------------------------- */
void card_center(struct card *card, int mod)
{
  /* Zielrechteck neu kalkulieren, da die Dimensionen geändert haben könnten */
  card->is[mod].rect.w = card->is[mod].surface->w;
  card->is[mod].rect.h = card->is[mod].surface->h;
  
  gfxutil_centerxy(&card->is[mod].rect, 
                   card->is[mod].move.now.x, 
                   card->is[mod].move.now.y);
}

/* -------------------------------------------------------------------------- *
 * Zeichnet eine Karte im angegebenen Modus auf die angegebene Surface        *
 * -------------------------------------------------------------------------- */
void card_blit(struct card *card, int mod, SDL_Surface *sf)
{
  /* Karte erst rendern und transformieren, falls nötig */
  if((card->status & CARD_RENDER) || card->surface == NULL)
    card_render(card);
  
  if((card->is[mod].status & CARD_TRANSFORM) || card->is[mod].surface == NULL) 
    card_transform(card, mod);
  
  /* Zielrechteck auf Position zentrieren */
  card_center(card, mod);
    
  /* finally Karte zeichnen :) */
  SDL_BlitSurface(card->is[mod].surface, NULL, sf, &card->is[mod].rect);
  
  card_debug(DETAILS, "Blit [ mod = %s, x = %3u, y = %3u, w = %3u, h = %3u ] (%s)",
             card_mods[mod], 
             card->is[mod].rect.x, card->is[mod].rect.y,
             card->is[mod].rect.w, card->is[mod].rect.h,
             card->name);
}

/* -------------------------------------------------------------------------- *
 * Kartenstruktur ausgeben                                                    *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void card_dump(struct card *card, int mod)
{
  card_log(INFO, "-- CARD [%s] ---------------------------------------", 
           card->name);
  card_log(INFO, "status  =%s%s%s",
           (card->status & CARD_HILITE) ? " HILITE" : "",
           (card->status & CARD_SEL) ? " SEL" : "",
           (card->status & CARD_RENDER) ? " RENDER" : "");
  
  card_log(INFO, "value   = %i",
           card->value);
  
  card_log(INFO, "tint    = #%02X%02X%02X%02X",
           card->blend.now.r, card->blend.now.g, card->blend.now.b, card->blend.now.a);
  
  card_log(INFO, "rect    = %i,%i %ux%u",
           card->rect.x, card->rect.y,
           card->rect.w, card->rect.h);

  if(card->surface)
    card_log(INFO, "surface = %ux%u",
             card->surface->w, card->surface->h);
  else
    card_log(INFO, "surface = NULL");
  
  card_log(INFO, "-- CARD [%s] instance %s ---------------------------",
           card->name, card_mods[mod]);
  
  if(card->is[mod].surface)
    card_log(INFO, "surface  = %ux%u",
             card->is[mod].surface->w, card->is[mod].surface->h);
  else
    card_log(INFO, "surface  = NULL");
  
  card_log(INFO, "status   =%s%s%s",
           (card->is[mod].status & CARD_TRANSFORM) ? " TRANSFORM" : "",
           (card->is[mod].status & CARD_MOVE) ? " MOVE" : "",
           (card->is[mod].status & CARD_FROZEN) ? " FROZEN" : "");
  
  card_log(INFO, "rect     = %i,%i %ux%u",
           card->is[mod].rect.x, card->is[mod].rect.y,
           card->is[mod].rect.w, card->is[mod].rect.h);

  card_log(INFO, "scale    = %5u/%5u/%5u",
           card->is[mod].scale.start,
           card->is[mod].scale.now,
           card->is[mod].scale.end);

  card_log(INFO, "move     = %i|%i / %i|%i / %i|%i",
           card->is[mod].move.start.x, card->is[mod].move.start.y,
           card->is[mod].move.now.x, card->is[mod].move.now.y,
           card->is[mod].move.end.x, card->is[mod].move.end.y);

  card_log(INFO, "rotozoom = %3i:%.3f:%3u / %3i:%.3f:%3i / %3i:%.3f:%3u",
           (int)card->is[mod].rotozoom.start.r,
                card->is[mod].rotozoom.start.z,
          (int)(card->is[mod].rotozoom.start.a * 255) & 0xff,
           (int)card->is[mod].rotozoom.now.r,
                card->is[mod].rotozoom.now.z,
          (int)(card->is[mod].rotozoom.now.a * 255) & 0xff,
           (int)card->is[mod].rotozoom.end.r,
                card->is[mod].rotozoom.end.z,
          (int)(card->is[mod].rotozoom.end.a * 255) & 0xff);
  
  card_log(INFO, "-- End of CARD [%s] --------------------------------",
           card->name);  
}
#endif /* DEBUG */
