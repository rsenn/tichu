/* $Id: card.h,v 1.31 2005/05/21 08:27:20 smoli Exp $
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

#ifndef CARD_H
#define CARD_H

#include <stdint.h>
#include <SDL.h>

/* -------------------------------------------------------------------------- *
 * Das CARD Modul kümmert sich um Laden, Rendern, Bewegen und Anzeigen von    *
 * Karten                                                                     *
 * -------------------------------------------------------------------------- */
#include "dlink.h"
#include "client.h"
#include "gfxutil.h"

/* -------------------------------------------------------------------------- *
 * Konstanten                                                                 *
 * -------------------------------------------------------------------------- */
#define CARD_WIDTH     129
#define CARD_HEIGHT    200

#define CARD_PADX      16
#define CARD_PADY      16

#define IMAGE_WIDTH   (((CARD_WIDTH + CARD_PADX - 1) / CARD_PADX) * CARD_PADX)
#define IMAGE_HEIGHT  (((CARD_HEIGHT + CARD_PADY - 1) / CARD_PADY) * CARD_PADY)

/* Position und Surface-Indexes */
#define CARD_FAN       0            /* fächer-position */
#define CARD_STACK     1            /* stack-position */
#define CARD_PLAYER    2            /* schupf-position */
#define CARD_DND       3            /* drag-position */

#define CARD_MODS     (CARD_DND + 1)

#define CARD_DRAGSPEED 250          /* pixel/sekunde */
#define CARD_MOVESPEED 200          /* pixel/sekunde */

/* Karten-Status */
#define CARD_HILITE    0x01
#define CARD_SEL       0x02
#define CARD_RENDER    0x10          /* neue kartenfarbe/alpha rendern */
#define CARD_FROZEN    0x20

/* Instanz-Status */
#define CARD_TRANSFORM 0x40          /* neue rotation/zoom */
#define CARD_MOVE      0x80


#define CARD_REDRAW    (CARD_MOVE|CARD_RENDER|CARD_TRANSFORM)

/* Default Spielerfarbe */
#define CARD_COLOR     gfxutil_red128 /* rot und halbtransparent */

/* -------------------------------------------------------------------------- *
 * Makros                                                                     *
 * -------------------------------------------------------------------------- */
#define card_hilited(c)        (!!(((struct card *)(c))->status & CARD_HILITE))
#define card_selected(c)       (!!(((struct card *)(c))->status & CARD_SEL))

#define card_log(l, s...) writelog(MOD_CARD, (l), s)

#if DEBUG_CRD
#define card_debug(l, s...) card_log((l), s)
#else
#define card_debug(l, s...)
#endif

/* -------------------------------------------------------------------------- *
 * Datenstrukturen                                                            *
 * -------------------------------------------------------------------------- */

/* Eine Instanz der Karte. Eine Karte kann an mehreren Orten instanziert sein
   (Fächer, Stapel, Spieler, DnD) 
 
   Instanzen einer Karte können verschiedene Positionen, Winkel und 
   Zoomfaktoren haben, haben jedoch immer den gleichen Farbton, Alphakanal,
   Rahmen, und Zoomfaktor.
 */
struct instance
{
  SDL_Surface      *surface;          /* Transformiertes Bild */
  int               status;           /* Statusflags */
  SDL_Rect          rect;             /* Zielrechteck */
  struct node       node;             /* Knoten für die Spezial-Listen */

  /* Translations- und Transformationsdaten */
  struct range      scale;
  struct move       move;
  struct rotozoom   rotozoom;
};

/* Eine Kartenstruktur ist einerseits ein Abbild der entsprechenden Sektion 
   in cards.ini und enthält andererseits Laufzeit-Infos wie z.B. Farbton,
   Alphawert des untransformierten Bildes oder die Position der Karte inner-
   halb des Fächers 
 */
struct card 
{
  struct node       node;             /* Knoten für die globale Kartenliste */
  struct player    *player;           
  char              name[4];
  int               status;
  int               value;            /* Wertigkeit der Karte */
  struct range      scale;
  struct blend      blend;
  SDL_Rect          rect;             /* Ausschnitt innerhalb card_image */
  SDL_Surface      *surface;          /* Gerendertes Bild in Ausgangslage */
  struct instance   is[CARD_MODS];
};

struct card_config
{
  uint8_t      border;
  uint8_t      alpha;
  sgHSV        color;
  int          antialias:1;
  float        zoom;
  const char  *image;
  const char  *ini;
};

/* -------------------------------------------------------------------------- *
 * Globale Variabeln                                                          *
 * -------------------------------------------------------------------------- */
extern struct list        card_list;
extern const char        *card_mods[CARD_MODS];
extern struct color       card_tint;
extern struct card_config card_config;
extern struct color       card_tint;

/* -------------------------------------------------------------------------- *
 * Initialisiert die Kartenverwaltung                                         *
 * -------------------------------------------------------------------------- */
int          card_init        (void);

/* -------------------------------------------------------------------------- *
 * Gibt alle Daten im Zusammenhang mit der Kartenverwaltung wieder frei       *
 * -------------------------------------------------------------------------- */
void         card_shutdown    (void);

/* -------------------------------------------------------------------------- *
 * Liest die Kartenkonfiguration [card] Sektion der angegebenen ini-Datei     *
 * -------------------------------------------------------------------------- */
void         card_configure   (struct ini      *ini);  

/* -------------------------------------------------------------------------- *
 * Erstellt eine Vorschau der Karte                                           *
 * -------------------------------------------------------------------------- */
SDL_Surface *card_preview     (SDL_Rect rect, 
                               struct card *card, 
                               float angle);
  

/* -------------------------------------------------------------------------- *
 * Eine Kartenstruktur anhand einer INI Sektion erstellen                     *
 * -------------------------------------------------------------------------- */
struct card *card_new         (const char      *name);

/* -------------------------------------------------------------------------- *
 * Gibt alle mit einer Karte assozierten Daten frei                           *
 * -------------------------------------------------------------------------- */
void         card_clean       (struct card     *card);

/* -------------------------------------------------------------------------- *
 * Gibt eine Karte frei                                                       *
 * -------------------------------------------------------------------------- */
void         card_delete      (struct card     *card);

/* -------------------------------------------------------------------------- *
 * Die Instanz-Surface freigeben, aber nur wenn Sie in keiner anderen Instanz *
 * Benutzt wird                                                               *
 * -------------------------------------------------------------------------- */
void         card_freeinst    (struct card     *card, 
                               int              mod);

/* -------------------------------------------------------------------------- *
 * Aktuelle Position und Transformation einer Instanz übernehmen              *
 * -------------------------------------------------------------------------- */
void         card_copyinst    (struct card     *card,
                               int              from,
                               int              to);
  
/* -------------------------------------------------------------------------- *
 * Findet eine Karte nach dem Namen                                           *
 * -------------------------------------------------------------------------- */
struct card *card_find        (const char      *name);

/* -------------------------------------------------------------------------- *
 * Karte suchen und laden falls nicht gefunden                                *
 * -------------------------------------------------------------------------- */
struct card *card_get         (const char      *name);

/* -------------------------------------------------------------------------- *
 * Findet eine zufällige Karte welche, falls angegeben, noch nicht auf der    *
 * angegebenen Liste ist                                                      *
 * -------------------------------------------------------------------------- */
struct card *card_random      (struct list     *list);  

/* -------------------------------------------------------------------------- *
 * Setzt Karten-Statusflags                                                   *
 * -------------------------------------------------------------------------- */
int          card_set         (struct card     *card, 
                               int              flags);

/* -------------------------------------------------------------------------- *
 * Löscht Karten-Statusflags                                                  *
 * -------------------------------------------------------------------------- */
int          card_unset       (struct card     *card, 
                               int              flags);
    
/* -------------------------------------------------------------------------- *
 * Setzt die aktuelle Kartenposition                                          *
 * -------------------------------------------------------------------------- */
int          card_setpos      (struct card     *card, 
                               int              mod, 
                               int16_t          x, 
                               int16_t          y);

/* -------------------------------------------------------------------------- *
 * Setzt den aktuellen Kartenwinkel                                           *
 * -------------------------------------------------------------------------- */
int          card_setangle    (struct card     *card,
                               int              mod,
                               float            angle);

/* -------------------------------------------------------------------------- *
 * Setzt den aktuellen Zoomfaktor                                             *
 * -------------------------------------------------------------------------- */
int          card_setzoom     (struct card     *card,
                               int              mod,
                               float            zoom);

/* -------------------------------------------------------------------------- *
 * Setzt den aktuellen Alphawert                                              *
 * -------------------------------------------------------------------------- */
int          card_setalpha    (struct card     *card,
                               int              mod, 
                               float            alpha);
  
/* -------------------------------------------------------------------------- *
 * Setzt Kartenfarbe                                                          *
 * -------------------------------------------------------------------------- */
int          card_setcolor    (struct card     *card,
                               struct color     color);
  
/* -------------------------------------------------------------------------- *
 * Setzt Kartenposition und Winkel                                            *
 * -------------------------------------------------------------------------- */
int          card_setvector   (struct card     *card,
                               int              mod, 
                               struct position  pos,
                               struct transform tf);
  
/* -------------------------------------------------------------------------- *
  * Setzt die Zielposition einer Karte.                                        *
  * Gibt die noch zurückzulegende Distanz in Pixel zurück.                     *
  * -------------------------------------------------------------------------- */
uint32_t     card_translate   (struct card   *card,
                               int            mod,
                               int16_t        x, 
                               int16_t        y);
  
/* -------------------------------------------------------------------------- *
 * Setzt die Zielposition welche die Karte innerhalb von 't' Millisekunden    *
 * erreichen soll.                                                            *
 * -------------------------------------------------------------------------- */
uint32_t     card_move        (struct card   *card, 
                               int            mod,
                               int16_t        x,
                               int16_t        y, 
                               uint32_t       t);

/* -------------------------------------------------------------------------- *
 * Setzt den Winkel, welchen die Karte innerhalb von 't' Millisekunden        *
 * erreichen soll.                                                            *
 * -------------------------------------------------------------------------- */
int          card_rotate      (struct card   *card,
                               int            mod,
                               float          angle);

/* -------------------------------------------------------------------------- *
 * Setzt den Zoomfaktor, welchen die Karte innerhalb von 't' Millisekunden    *
 * erreichen soll.                                                            *
 * -------------------------------------------------------------------------- */
int          card_zoom        (struct card   *card,
                               int            mod,
                               float          zoom,
                               uint32_t       t);

/* -------------------------------------------------------------------------- *
 * Setzt den Farbwert, welchen die Karte innerhalb von 't' Millisekunden      *
 * erreichen soll.                                                            *
 * -------------------------------------------------------------------------- */
int          card_blend       (struct card   *card,
                               struct color   color, 
                               uint32_t       t);  

/* -------------------------------------------------------------------------- *
 * Setzt die Zeitspanne oder Mausdistanz innerhalb der die Bewegung statt-    *
 * findet                                                                     *
 * -------------------------------------------------------------------------- */
void         card_setrange    (struct card   *card,
                               int            mod,
                               uint32_t       start,
                               uint32_t       end);
/* -------------------------------------------------------------------------- *
 * Ist die Karte an der angegebenen Position?                                 *
 * -------------------------------------------------------------------------- */
int          card_isat        (struct card   *card,
                               int            mod,
                               int16_t        x, 
                               int16_t        y);
  
/* -------------------------------------------------------------------------- *
 * Updated Kartenpositionen                                                   *
 * -------------------------------------------------------------------------- */
int          card_update      (struct card   *card,
                               int            mod, 
                               uint32_t       diff);
  
/* -------------------------------------------------------------------------- *
 * Färbt eine Karte und errechnet den Alpha-Layer, das Ergebnis wird in ein   *
 * quadratisches Surface, welches sich besser zur späteren Rotation eignet,   *
 * geschrieben                                                                *
 * -------------------------------------------------------------------------- */
void         card_render      (struct card   *card);

/* -------------------------------------------------------------------------- *
 * Transformiert eine Karte                                                   *
 * -------------------------------------------------------------------------- */
void         card_transform   (struct card   *card,
                               int            mod);  

/* -------------------------------------------------------------------------- *
 * Zentriert das Zielrechteck einer Karte auf die aktuelle Kartenposition     *
 * -------------------------------------------------------------------------- */
void         card_center      (struct card   *card, 
                               int            mod);

/* -------------------------------------------------------------------------- *
 * Zeichnet eine Karte im angegebenen Modus auf das angegebene Surface        *
 *                                                                            *
 * Wenn 'mask' angebenen wurde, dann wird die Kartenmaske auf dieses Surface  *
 * gezeichnet.                                                                *
 * Der Parameter 'tint' bestimmt die Farbtönung, 'alpha' die Transparenz.     *
 * Wen 'margin' nicht 0 ist dann wird ein dickerer Kartenrahmen gezeichnet.   *
 * -------------------------------------------------------------------------- */
void         card_blit        (struct card   *card,
                               int            mod,
                               SDL_Surface   *dest);

/* -------------------------------------------------------------------------- *
 * Kartenstruktur ausgeben                                                    *
 * -------------------------------------------------------------------------- */
void         card_dump        (struct card   *card, 
                               int            mod);
#endif /* CARD_H */
