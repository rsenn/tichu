/* $Id: dnd.h,v 1.12 2005/05/21 08:27:20 smoli Exp $
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

#ifndef DND_H
#define DND_H

/* -------------------------------------------------------------------------- *
 * Das DND Modul erledigt das Bewegen von Karten zwischen verschiedenen       *
 * Modulen (FAN, PLAYER, STACK)                                               *
 * -------------------------------------------------------------------------- */
#include "card.h"

/* -------------------------------------------------------------------------- *
 * Makros fürs Loggen                                                         *
 * -------------------------------------------------------------------------- */
#define dnd_log(l, s...) writelog(MOD_DND, (l), s)

#if DEBUG_DND
#define dnd_debug(l, s...) dnd_log((l), s)
#else
#define dnd_debug(l, s...)
#endif

/* -------------------------------------------------------------------------- *
 * Konstanten                                                                 *
 * -------------------------------------------------------------------------- */
#define DND_DISTANCE 5       /* Pixeldistanz nach der das Modul anspricht
                                und in den DND Modus wechselt */

#define DND_SPEED    500     /* Geschwindigkeit in der die Karten an den 
                                Ausgangspunkt zurückgeschoben werden */

/* -------------------------------------------------------------------------- *
 * Statusflags für das Modul                                                  *
 * -------------------------------------------------------------------------- */
#define DND_REDRAW 0x01      /* Neu zeichnen */
#define DND_LOCKED 0x02      /* Nicht auf Events reagieren */
#define DND_HIDDEN 0x04      /* Nicht zeichnen */
#define DND_FROZEN 0x08      /* Nicht updaten */
#define DND_RETURN 0x10      /* An die Ausgangsposition zurück */

#define DND_DISABLED (DND_LOCKED|DND_HIDDEN|DND_FROZEN)

/* -------------------------------------------------------------------------- *
 * Globale Variabeln                                                          *
 * -------------------------------------------------------------------------- */
extern struct list dnd_list;          /* Liste aller gedraggten Karten */
extern int         dnd_status;        /* Status dieses Moduls */
extern int         dnd_startx;        /* DnD Startposition */
extern int         dnd_starty;
extern int         dnd_mode;          /* Modus aus dem das Draggen
                                         gestartet hat */
extern int         dnd_target;        /* Modul in dem das Draggen enden soll */

/* -------------------------------------------------------------------------- *
 * Initialisiert Drag & Drop                                                  *
 * -------------------------------------------------------------------------- */
void dnd_init     (void);

/* -------------------------------------------------------------------------- *
 * Gibt die Drag & Drop-Ressourcen wieder frei                                *
 * -------------------------------------------------------------------------- */
void dnd_shutdown (void);  

/* -------------------------------------------------------------------------- *
 * Setzt Drag & Drop Statusflags                                              *
 * -------------------------------------------------------------------------- */
int  dnd_set      (int          flags);
                                             
/* -------------------------------------------------------------------------- *
 * Löscht Drag & Drop Statusflags                                             *
 * -------------------------------------------------------------------------- */
int  dnd_unset    (int          flags);

/* -------------------------------------------------------------------------- *
 * Starte Drag & Drop, ausgehend vom angegebenen Modus                        *
 * (vorläufig eigentlich immer CARD_FAN)                                      *
 * -------------------------------------------------------------------------- */
int  dnd_start    (int          mode,
                   int          tgt,
                   int          x,
                   int          y);

/* -------------------------------------------------------------------------- *
 * Stoppe Drag & Drop                                                         * 
 * -------------------------------------------------------------------------- */
int  dnd_stop     (void);

/* -------------------------------------------------------------------------- *
 * Fügt eine Karte hinzu                                                      *
 * Mit 'mode' bestimmt man aus welchem Modus die Positionen und der Winkel    *
 * übernommen wird.                                                           *
 * -------------------------------------------------------------------------- */
void dnd_add      (struct card *card);

/* -------------------------------------------------------------------------- *
 * Nimmt eine Karte von der DnD Liste                                         *
 * -------------------------------------------------------------------------- */
void dnd_remove   (struct card *card);

/* -------------------------------------------------------------------------- *
 * Drag & Drop Karten zeichnen                                                *
 * -------------------------------------------------------------------------- */
void dnd_redraw   (SDL_Surface *surface);
  

/* -------------------------------------------------------------------------- *
 * Behandelt Mausbewegungen                                                   *
 * -------------------------------------------------------------------------- */
int  dnd_motion   (uint8_t      type,
                   uint8_t      state,
                   int          x, 
                   int          y,
                   int16_t      xrel,
                   int16_t      yrel);

/* -------------------------------------------------------------------------- *
 * Behandelt Mausgeklicke                                                     *
 * -------------------------------------------------------------------------- */
int  dnd_button   (uint8_t      type,
                   uint8_t      button,
                   uint8_t      state, 
                   int16_t      x,
                   int16_t      y);

/* -------------------------------------------------------------------------- *
 * Behandelt Events die das Drag & Drop betreffen                             *
 * -------------------------------------------------------------------------- */
int  dnd_event    (SDL_Event   *event);

/* -------------------------------------------------------------------------- *
 * Updatet Kartenpositionen und so                                            *
 * -------------------------------------------------------------------------- */
int  dnd_update   (uint32_t     t);

#endif /* DND_H */
