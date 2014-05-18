/* $Id: stack.h,v 1.32 2005/05/21 08:27:20 smoli Exp $
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

#ifndef STACK_H
#define STACK_H

/* -------------------------------------------------------------------------- *
 * Das STACK Modul zeigt den Kartenstapel an und nimmt die gespielten Karten  *
 * entgegen                                                                   *
 * -------------------------------------------------------------------------- */
#include "gfxutil.h"
#include "card.h"
#include "player.h"

/* -------------------------------------------------------------------------- *
 * Makros fürs Loggen                                                         *
 * -------------------------------------------------------------------------- */
#define stack_log(l, s...) writelog(MOD_STACK, (l), s)

#if DEBUG_STK
#define stack_debug(l, s...) stack_log((l), s)
#else
#define stack_debug(l, s...)
#endif

/* Status Flags für den Stapel
 * -------------------------------------------------------------------------- */
#define STACK_HIDDEN    0x01   /* Kein Redraw */
#define STACK_LOCKED    0x02   /* Keine Events */
#define STACK_FROZEN    0x04   /* Kein Update */
#define STACK_DRAG      0x08

#define STACK_DISABLED  (STACK_LOCKED|STACK_HIDDEN|STACK_FROZEN)

/* Redraw Flags für den Stapel
 * -------------------------------------------------------------------------- */
#define STACK_MOVE      0x10   /* Position hat geändert */
#define STACK_RENDER    0x20   /* Look (Farbe, Alpha) hat geändert */
#define STACK_TRANSFORM 0x40   /* Winkel oder Zoom hat geändert */

#define STACK_REDRAW    (STACK_MOVE|STACK_RENDER|STACK_TRANSFORM)

/* Geometrie & Physik für Stapel 
 * -------------------------------------------------------------------------- */
#define STACK_ANGLE     16     /* Winkel zwischen 2 Karten in ° */
#define STACK_SPEED     90.0   /* Rotationsgeschwindigkeit in °/s */

#define STACK_IN        0      /* Innere Position */ 
#define STACK_OUT       1      /* Äussere Position */

/* -------------------------------------------------------------------------- *
 * Globale Variabeln                                                          *
 * -------------------------------------------------------------------------- */
extern struct position stack_pos;
extern SDL_Rect        stack_rect;
extern int             stack_status;

/* -------------------------------------------------------------------------- *
 * Initialisiert den Kartenstapel                                             *
 * -------------------------------------------------------------------------- */
void         stack_init      (void);

/* -------------------------------------------------------------------------- *
 * Gibt die Stapel-Ressourcen wieder frei                                     *
 * -------------------------------------------------------------------------- */
void         stack_shutdown  (void);

/* -------------------------------------------------------------------------- *
 * Setzt Stapel-Statusflags                                                   *
 * -------------------------------------------------------------------------- */
int          stack_set       (int            flags);
  
/* -------------------------------------------------------------------------- *
 * Löscht Fächer-Statusflags                                                  *
 * -------------------------------------------------------------------------- */
int          stack_unset     (int            flags);    

/* -------------------------------------------------------------------------- *
 * Setzt den Spieler der an der Reihe ist                                     *
 * -------------------------------------------------------------------------- */
void         stack_actor     (struct player *player);

/* -------------------------------------------------------------------------- *
 * Eine Karte auf den Stapel legen (vielleicht nur temporär)                  *
 * -------------------------------------------------------------------------- */
void         stack_add       (struct card   *card,
                              struct player *player);
  
/* -------------------------------------------------------------------------- *
 * Karte wieder vom Stapel nehmen                                             *
 * -------------------------------------------------------------------------- */
void         stack_remove    (struct card   *card);

/* -------------------------------------------------------------------------- *
 * Setzt die Stapelfarbe                                                      *
 * -------------------------------------------------------------------------- */
void         stack_setcolor  (struct color   color);

/* -------------------------------------------------------------------------- *
 * Überblendet die Stapelfarbe                                                *
 * -------------------------------------------------------------------------- */
void         stack_blend     (struct color   color);

/* -------------------------------------------------------------------------- *
 * Setzt den Winkel des Stapels                                               *
 * -------------------------------------------------------------------------- */
void         stack_setangle  (int            a);

/* -------------------------------------------------------------------------- *
 * Setzt den Zoomfaktor des Stapels                                           *
 * -------------------------------------------------------------------------- */
void         stack_setzoom   (float          z);

/* -------------------------------------------------------------------------- *
 * Rotiert den Stapel an die angegebene Position mit einer Geschwindigkeit    *
 * von v° pro Sekunde                                                         *
 * -------------------------------------------------------------------------- */
void         stack_rotate    (int            a,
                              uint32_t       v);

/* -------------------------------------------------------------------------- *
 * Zoomt den Stapel während einer Rotation                                    *
 * -------------------------------------------------------------------------- */
void         stack_zoom      (float          z);

/* -------------------------------------------------------------------------- *
 * Kalkuliert die Positionen und Winkel aller Karten und rotiert die Bilder   *
 * dementsprechend                                                            *
 *                                                                            *
 * 'center' gibt den Startwinkel an, 0° für den lokalen Spieler, 90° für den  *
 * nächsten Spieler                                                           *
 * -------------------------------------------------------------------------- */
void         stack_calc      (struct player *player,
                              uint32_t       speed);

/* -------------------------------------------------------------------------- *
 * Findet per Rechteck-Überlappung und Pixel-Test heraus ob sich an der       *
 * angegebenen Position tatsächlich der Stapel befindet                       *
 * -------------------------------------------------------------------------- */
int          stack_isat      (int            x, 
                              int            y);

/* -------------------------------------------------------------------------- *
 * Blittet das rotierte und gezoomte Stack-Surface nach stack_sf und färbt    *
 * es dabei ein                                                               *
 * -------------------------------------------------------------------------- */
void         stack_render    (void);

/* -------------------------------------------------------------------------- *
 * Rotiert und zoomt den Stack nach den aktuellen Werten                      *
 * -------------------------------------------------------------------------- */
void         stack_transform (void);

/* -------------------------------------------------------------------------- *
 * updatet die werte für zoom, rotation und farbe des kartenstapels für die   *
 * vergangene zeit <diff> (millisekunden)                                     *
 * -------------------------------------------------------------------------- */
int          stack_update    (uint32_t t);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void         stack_cardpos   (struct card   *card, 
                              double         angle);

/* -------------------------------------------------------------------------- *
 * Karten provisorisch auf dem Stapel anzeigen                                *
 * (vor dem Ablegen oder Zurückweisen)                                        *
 * -------------------------------------------------------------------------- */
void         stack_float     (void);

/* -------------------------------------------------------------------------- *
 * Legt Karten auf dem Stapel ab                                              *
 * -------------------------------------------------------------------------- */
void         stack_drop      (void);
  
/* -------------------------------------------------------------------------- *
 * Weist Karten zurück                                                        *
 * -------------------------------------------------------------------------- */
void         stack_refuse    (void);

/* -------------------------------------------------------------------------- *
 * Nimmt die Karten definitiv entgegen und zeichnet sie auf das Surface für   *
 * die abgelegten Karten                                                      *
 * -------------------------------------------------------------------------- */
void         stack_accept    (void);
  
/* -------------------------------------------------------------------------- *
 * Schiebt Karten aus Position von anderem Spieler richtung Stapel            *
 *                                                                            *
 * Winkel wird aus player_index und player_count berechnet,                   *
 * player_index 0 ist der lokale Spieler                                      *
 * -------------------------------------------------------------------------- */
void         stack_appear    (struct list   *cards,
                              struct player *player,
                              int            radius);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void         stack_set_actor (struct player *player);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int          stack_motion    (uint8_t        type,
                              uint8_t        state,
                              uint16_t       x, 
                              uint16_t       y,
                              int16_t        xrel,
                              int16_t        yrel);

/* -------------------------------------------------------------------------- *
 * Behandelt Mausgeklicke                                                     *
 * -------------------------------------------------------------------------- */
int          stack_button    (uint8_t        type,
                              uint8_t        button,
                              uint8_t        state, 
                              int16_t        x, 
                              int16_t        y);  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int          stack_event     (SDL_Event     *event);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void         stack_redraw    (SDL_Surface   *surface);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void         stack_clear     (void);

/* -------------------------------------------------------------------------- *
 * Stapeldaten ausgeben                                                       *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void         stack_dump      (void);
#endif /* DEBUG */

#endif /* STACK_H */
