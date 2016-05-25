/* $Id: player.h,v 1.29 2005/05/21 08:27:20 smoli Exp $
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

#ifndef PLAYER_H
#define PLAYER_H

#include "gfxutil.h"
#include "card.h"

/* -------------------------------------------------------------------------- *
 * Makros fürs Loggen                                                         *
 * -------------------------------------------------------------------------- */
#define player_log(l, s...) writelog(MOD_PLAYER, (l), s)

#if DEBUG_PLR
#define player_debug(l, s...) player_log((l), s)
#else
#define player_debug(l, s...)
#endif

/* -------------------------------------------------------------------------- *
 * Konstanten für den Modul-Status                                            *
 * -------------------------------------------------------------------------- */
#define PLAYER_REDRAW   0x01     /* neu zeichnen */
#define PLAYER_LOCKED   0x02     /* gesperrt */
#define PLAYER_HIDDEN   0x04     /* versteckt */
#define PLAYER_FROZEN   0x08
#define PLAYER_RETURN   0x10     /* schupfungen entgegennehmen */

/* -------------------------------------------------------------------------- *
 * Konstanten für den Spieler-Status                                          *
 * -------------------------------------------------------------------------- */
#define PLAYER_HILITE    0x01
#define PLAYER_MOVE      0x02
#define PLAYER_RENDER    0x04
#define PLAYER_TRANSFORM 0x08
#define PLAYER_SCHUPF    0x10  /* Diesem Spieler muss noch geschupft werden */
#define PLAYER_SENT      0x20  /* Schupfung wurde gesendet */

/* -------------------------------------------------------------------------- *
 * Konstanten für den Spieler-Bewegungen                                      *
 * -------------------------------------------------------------------------- */
#if SLOW_PLR
#define PLAYER_MDELAY    10000   /* Zeitdauer einer Bewegung */
#define PLAYER_FDELAY    1000   /* Zeitdauer des Alpha-Fadings */
#else
#define PLAYER_MDELAY    1000   /* Zeitdauer einer Bewegung */
#define PLAYER_FDELAY    500    /* Zeitdauer des Alpha-Fadings */
#endif

/* Konstanten für player_move() */
#define PLAYER_IN       0      /* Innerhalb des Bildschirms */
#define PLAYER_OUT      1      /* Ausserhalb des Bildschirms */

/* -------------------------------------------------------------------------- *
 * Spieler Datenstruktur                                                      *
 * -------------------------------------------------------------------------- */
struct player 
{
  struct node         node;
  SDL_Surface        *sf;        /* Gerenderter Kreis */
  SDL_Rect            sr;
  SDL_Surface        *tf;        /* Transformierter Kreis */
  SDL_Rect            tr;
  char                name[64];
  int                 index;     /* Index, Lokaler Spieler = 0 */
  int                 status;    /* Statusflags */
  struct list         cards;     /* Geschupfte Karten */

  struct position     pos[2];    /* Innere und äussere Position */
  float               angle[2];  /* Innerer und äusserer Winkel */
  
  struct range        scale;     /* Bereich in dem die Bewegungen stattfinden */
  
  /* Translationsdaten */
  struct move         move;
  
  /* Transformationsdaten */
  struct rotozoom     rotozoom;
  
  /* Color-Blending */
  struct blend        blend;
};

/* -------------------------------------------------------------------------- *
 * Globale Variabeln                                                          *
 * -------------------------------------------------------------------------- */
extern struct list    player_list;    /* Liste der Spieler */
extern int            player_status;  /* Status des Spieler-Moduls */
extern struct player *player_hilite;  /* Ausgewählter Spieler */
extern struct player *player_local;   /* Lokaler Spieler */
extern SDL_Surface   *player_image;   /* Bild mit Kreis */

/* -------------------------------------------------------------------------- *
 * Initialisiert die Spielerverwaltung                                        *
 * -------------------------------------------------------------------------- */
void           player_init       (void);

/* -------------------------------------------------------------------------- *
 * Fährt die Spielerverwaltung runter                                         *
 * -------------------------------------------------------------------------- */
void           player_shutdown   (void);

/* -------------------------------------------------------------------------- *
 * Erstellt einen neuen Spieler.                                              *
 *                                                                            *
 * 'index' ist die Position des Spielers. Der lokale Spieler hat immer einen  *
 * index von 0, gegen den Uhrzeigersinn werden dann die Spieler bis 3 durch-  *
 * nummeriert.                                                                *
 * -------------------------------------------------------------------------- */
struct player *player_new        (const char    *name,
                                  int            index, 
                                  struct color   tint);

/* -------------------------------------------------------------------------- *
 * Löscht den Spieler                                                         *
 * -------------------------------------------------------------------------- */
void           player_delete     (struct player *player);

/* -------------------------------------------------------------------------- *
 * Findet einen Spieler nach Namen                                            *
 * -------------------------------------------------------------------------- */
struct player *player_find       (const char    *name);

/* -------------------------------------------------------------------------- *
 * Setzt Spielerfarbe                                                         *
 * -------------------------------------------------------------------------- */
void           player_setcolor   (struct player *player, 
                                  struct color   tint);

/* -------------------------------------------------------------------------- *
 * Fadet den Alpha-Layer des Spielerkreises                                   *
 * -------------------------------------------------------------------------- */
void           player_fade       (struct player *player,
                                  uint8_t        alpha, 
                                  uint32_t       t);

/* -------------------------------------------------------------------------- *
 * Berechnet inneren und äusseren Winkel eines Gegen- oder Mitspielers        *
 * anhand seines Indexes.                                                     *
 *                                                                            *
 * Der äussere Winkel legt fest aus welcher Richtung die Karten des Spielers  *
 * erscheinen.                                                                *
 * Der 'innere' Winkel hingegen die Position des Schupf-Kreises auf dem Kreis *
 * bogen.                                                                     *
 * -------------------------------------------------------------------------- */
void           player_calc       (struct player *player);

/* -------------------------------------------------------------------------- *
 * Stellt fest ob sich die angegebende Position innerhalb des Schupf-Kreises  *
 * des angegebenen Spielers befindet                                          *
 * -------------------------------------------------------------------------- */
int            player_isat       (struct player *player, 
                                  int            x, 
                                  int            y);

/* -------------------------------------------------------------------------- *
 * Gibt den Spieler an der angegebenen Position zurück, oder NULL falls dort  *
 * keiner ist.                                                                *
 * -------------------------------------------------------------------------- */
struct player *player_at         (int            x, 
                                  int            y);

/* -------------------------------------------------------------------------- *
 * Checkt ob jedem Spieler geschupft wurde.                                   *
 * -------------------------------------------------------------------------- */
int            player_check      (void);

/* -------------------------------------------------------------------------- *
 * Alle Spieler abwählen                                                      *
 * -------------------------------------------------------------------------- */
void           player_deselect   (void);

  /* -------------------------------------------------------------------------- *
 * Legt Karten temporär auf dem Schupf-Kreis ab und schickt Schupf-Kommando   *
 * an den Server                                                              *
 * -------------------------------------------------------------------------- */
void           player_drop       (struct player *player);

/* -------------------------------------------------------------------------- *
 * Generiert einen eingefärbten Kreis in der Spielerfarbe                     *
 * -------------------------------------------------------------------------- */
void           player_render     (struct player *player);
  
/* -------------------------------------------------------------------------- *
 * Zeichnet einen Schupfkreis und die dazugehörigen Karten auf den Screen     *
 * -------------------------------------------------------------------------- */
void           player_blit       (struct player *player, 
                                  SDL_Surface   *surface);

/* -------------------------------------------------------------------------- *
 * Alle Player (Schupfkreise und Karten) neu zeichnen                         *
 * -------------------------------------------------------------------------- */
void           player_redraw     (SDL_Surface   *surface);

/* -------------------------------------------------------------------------- *
 * Beginnt mit dem Schupfen. Alle Winkel und Positionen werden erst berechnet *
 * -------------------------------------------------------------------------- */
void           player_start      (void);

/* -------------------------------------------------------------------------- *
 * Zurückschupfen.                                                            *
 * -------------------------------------------------------------------------- */
void           player_return     (struct player *player, 
                                  struct card   *card);
  
/* -------------------------------------------------------------------------- *
 * Berechnet Winkel der gedraggten Karte anhand den Distanzverhältnissen der  *
 * verschiedenen Schupfkreisen                                                *
 * -------------------------------------------------------------------------- */
float          player_dragangle  (int            x,
                                  int            y,
                                  int            sdist, 
                                  float sangle);

/* -------------------------------------------------------------------------- *
 * Behandelt Mausgeklicke                                                     *
 * -------------------------------------------------------------------------- */
int            player_button     (uint8_t        type,
                                  uint8_t        button,
                                  uint8_t        state, 
                                  int16_t        x, 
                                  int16_t        y);

/* -------------------------------------------------------------------------- *
 * Behandelt Mausbewegungen                                                   *
 * -------------------------------------------------------------------------- */
int            player_motion     (uint8_t        type,
                                  uint8_t        state, 
                                  int16_t        x,
                                  int16_t        y,
                                  int16_t        xrel, 
                                  int16_t        yrel);

/* -------------------------------------------------------------------------- *
 * Behandelt allgemeine Events und leitet sie weiter and Subroutinen          *
 * -------------------------------------------------------------------------- */
int            player_event      (SDL_Event     *event);

/* -------------------------------------------------------------------------- *
 * Aktualisiert x-y Position eines Schupfkreises                              *
 * -------------------------------------------------------------------------- */
int            player_update_xy  (struct player *player, 
                                  int            diff);

/* -------------------------------------------------------------------------- *
 * Spielerposition aktualisieren                                              *
 * -------------------------------------------------------------------------- */
int            player_update     (uint32_t       t);

/* -------------------------------------------------------------------------- *
 * Playerstruktur ausgeben                                                    *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void           player_dump       (struct player *player);
#endif /* DEBUG */

#endif /* PLAYER_H */
