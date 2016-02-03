/* $Id: game.h,v 1.21 2005/05/23 02:59:57 smoli Exp $
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

#ifndef GAME_H
#define GAME_H

/* -------------------------------------------------------------------------- *
 * Makros fürs Loggen                                                         *
 * -------------------------------------------------------------------------- */
#define game_log(l, s...) writelog(MOD_GAME, (l), s)

#if DEBUG_GAM
#define game_debug(l, s...) game_log((l), s)
#else
#define game_debug(l, s...)
#endif

/* -------------------------------------------------------------------------- *
 * Konstanten für Modul-Status                                                *
 * -------------------------------------------------------------------------- */
#define GAME_RUN    0x01

/* Flags für aktive Elemente */
#define GAME_REDRAW_DIALOG 0x08
#define GAME_REDRAW_FAN    0x10
#define GAME_STACK  0x20
#define GAME_PLAYER 0x40
#define GAME_DND    0x80

/* Allgemeines Redraw-Flag */
#define GAME_REDRAW 0xf8

/* Frame-Rate die die Engine zu halten versuchen soll
 * -------------------------------------------------------------------------- */
#define GAME_FPS    50

extern Uint32       game_ticks;       /* Zeitpunkt der aktuellen Iteration */


/* -------------------------------------------------------------------------- *
 * Initialisiert das Spiel                                                    *
 * -------------------------------------------------------------------------- */
void                game_init      (void);

/* -------------------------------------------------------------------------- *
 * Beendet den Spielmodus                                                     *
 * -------------------------------------------------------------------------- */
void                game_shutdown  (void);

/* -------------------------------------------------------------------------- *
 * Setzt Game-Statusflags                                                     *
 * -------------------------------------------------------------------------- */
int                 game_set       (int          flags);
  
/* -------------------------------------------------------------------------- *
 * Löscht Game-Statusflags                                                    *
 * -------------------------------------------------------------------------- */
int                 game_unset     (int          flags);

/* -------------------------------------------------------------------------- *
 * Wird per Iteration der Game-Schleife aufgerufen und updated Zeitabhängige  *
 * Dinge (wie z.B. FPS-Counter)                                               *
 * -------------------------------------------------------------------------- */
int                 game_update    (Uint32       diff);
  
/* -------------------------------------------------------------------------- *
 * Blitted alle Surfaces auf den Screen                                       *
 * -------------------------------------------------------------------------- */
void                game_redraw    (SDL_Surface *screen);  

/* -------------------------------------------------------------------------- *
 * Hauptloop im Game-Modus                                                    *
 * -------------------------------------------------------------------------- */
int                 game_loop      (SDL_Surface *screen);

#endif /* GAME_H */
