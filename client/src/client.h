/* $Id: client.h,v 1.75 2005/05/26 10:48:35 smoli Exp $
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

#ifndef CLIENT_H
#define CLIENT_H

#include <limits.h>
#include <SDL.h>
#include <libsgui/sgui.h>

/* -------------------------------------------------------------------------- *
 * Das CLIENT Modul ist das Hauptmodul des Tichu-Clients                      *
 * -------------------------------------------------------------------------- */
#include "gfxutil.h"

/* Name und Version des Softwarepackets 
 * -------------------------------------------------------------------------- */
#ifndef PACKAGE
#define PACKAGE PACKAGE_NAME
#endif

#ifndef VERSION
#define VERSION PACKAGE_VERSION
#endif

/* Initiale Screendimensionen 
 * -------------------------------------------------------------------------- */
#define CLIENT_WIDTH  800
#define CLIENT_HEIGHT 600

/* Default Host/Port für den Tichu-Server
 * -------------------------------------------------------------------------- */
#define CLIENT_DEFAULT_USER "Benutzer"
#define CLIENT_DEFAULT_INFO PACKAGE" v"VERSION
#define CLIENT_DEFAULT_PASS ""

/* Flags für client_open() 
 * -------------------------------------------------------------------------- */
#define CLIENT_READ  0x01
#define CLIENT_WRITE 0x02

/* -------------------------------------------------------------------------- *
 * strncpy() ist per Standard eine Scheiss-Funktion, weil wenn der String die *
 * Länge überschreitet, dann wird der Zielbuffer nicht 0-terminiert. Und auf  *
 * Windows haben wir halt kein strlcpy(), deshalb helfen wir uns mit einem    *
 * Makro.                                                                     *
 * -------------------------------------------------------------------------- */
#define client_strlcpy(dest, src, n) \
  strncpy((dest), (src), (n)), (dest)[n-1] = '\0'

/* -------------------------------------------------------------------------- *
 * Durchnummerierung der Module                                               *
 * -------------------------------------------------------------------------- */
enum
{
  MOD_FAN = 0,
  MOD_PLAYER,
  MOD_STACK,
  MOD_DND,
  MOD_ACTION,
  MOD_CARD,
  MOD_CLIENT,
  MOD_DLINK,
  MOD_GAME,
  MOD_GFXUTIL,
  MOD_INI,
  MOD_NET,
  MOD_QUEUE,
  MOD_REPLY,
  MOD_SOUND,
  MOD_UI,
#ifdef DEBUG
  MOD_MDEBUG,
#endif /* DEBUG */  
};

/* -------------------------------------------------------------------------- *
 * Makros fürs loggen                                                         *
 * -------------------------------------------------------------------------- */
#define ERROR   0      /* Gibt Message überall aus, und bricht Programm ab */
#define STATUS  1      /* Gibt Message überall aus */
#define CONSOLE 2      /* Gibt Message nur auf der graphischen Konsole aus */

#define INFO    3      /* Info für ins Logfile/Textkonsole */
#define VERBOSE 4      /* Zusätzliche Info für Log/Text */
#define DETAILS 5      /* Debugging Info */

#ifndef LOGLEVEL
#define LOGLEVEL CONSOLE
#endif

#if (LOGLEVEL >= INFO)
#define writedbg(module, level, string...) client_write(module, level, string)
#else
#define writedbg(module, level, string...) 
#endif

#define writelog(module, level, string...) client_write(module, level, string)


#define client_log(l, s...) writelog(MOD_CLIENT, (l), s)

#ifdef DEBUG_CLIENT
#define client_debug(l, s...) client_log((l), s)
#else
#define client_debug(l, s...) 
#endif

/* Client-Identifikation
 * -------------------------------------------------------------------------- */
#define CLIENT_USERLEN 32
#define CLIENT_PASSLEN 32
#define CLIENT_INFOLEN 256

/* -------------------------------------------------------------------------- *
 * Konfiguration für den Client                                               *
 * -------------------------------------------------------------------------- */
struct client_config
{
  uint16_t     height;
  uint16_t     width;
  uint8_t      depth;
  int          fullscreen:1;
  const char  *cursor;
  
  sgHSV        bgcolor;
  const char  *pattern;
  uint8_t      contrast;

  const char  *font[3];
  
  /* Client-Identifikation */
  char         user[CLIENT_USERLEN + 1];
  char         pass[CLIENT_PASSLEN + 1];
  char         info[CLIENT_INFOLEN + 1];
  
  /* Version des Clients der die Konfiguration geschrieben hat */
  const char  *version;
};

extern struct client_config client_config;

/* Screen-Dimensionen
 * -------------------------------------------------------------------------- */
extern SDL_Rect     client_rect;

/* Client-Status
 * -------------------------------------------------------------------------- */
#define CLIENT_EXIT      0x01
#define CLIENT_MAIN      0x02
#define CLIENT_CONFIG    0x04
#define CLIENT_CHAT      0x05
#define CLIENT_SETTINGS  0x06
#define CLIENT_GAME      0x07
#define CLIENT_GAME_END  0x08
#define CLIENT_TEST      0x09

extern int          client_status;

extern sgCursorTheme *client_cursor;
extern sgPattern     *client_bgnd;
extern sgColor        client_bgcolor;

/* Konfigurationsdatei
 * -------------------------------------------------------------------------- */
#define CLIENT_INI "client.ini"

extern struct ini  *client_ini;

/* Bitmap-Fonts (libsgui)
 * -------------------------------------------------------------------------- */
extern sgFont      *client_font[3];

/* Scrollpuffer für die Spielkonsole und initialer Inhalt (ASCII-Logo)
 * -------------------------------------------------------------------------- */
extern char        *client_buffer;
extern const char  *client_ascii;

/* Suchpfade
 * -------------------------------------------------------------------------- */
extern const char  *client_paths[8];
extern char        *client_path; /* Pfad der letzten gefundenen Datei */

/* Bildschirm-Surface und Hintergrund-Muster
 * -------------------------------------------------------------------------- */
extern SDL_Surface *client_screen;
extern SDL_Surface *client_background;

/* Momentantes Ziel von Eingaben ohne Kommando-Präfix (/)
 * -------------------------------------------------------------------------- */
extern char         client_target[CLIENT_USERLEN + 2];

/* Musik-Playliste
 * -------------------------------------------------------------------------- */
extern struct list  client_playlist;

/* Makros für sicheres free()
 * -------------------------------------------------------------------------- */
#define client_free(x)       if(!x){}else{free(x);x=0;};
#define client_free_image(x) if(!x){}else{SDL_FreeSurface(x);x=0;};

/* -------------------------------------------------------------------------- *
 * Client Initialisation                                                      *
 * -------------------------------------------------------------------------- */
void         client_init         (void);

/* -------------------------------------------------------------------------- *
 * Client Shutdown                                                            *
 * -------------------------------------------------------------------------- */
void         client_shutdown     (void);
  
/* -------------------------------------------------------------------------- *
 * Liest Benutzeridentifikation aus der client.ini                            *
 * -------------------------------------------------------------------------- */
int          client_configure    (void);


void         client_setmode      (void);
  
/* -------------------------------------------------------------------------- *
 * Setzt Cursortheme und Hintergrundmuster                                    *
 * -------------------------------------------------------------------------- */
void         client_setlook      (void);
  

/* -------------------------------------------------------------------------- *
 * Liest die möglichen Video-Modes aus                                        *
 * -------------------------------------------------------------------------- */
void         client_getmodes     (void);

/* -------------------------------------------------------------------------- *
 * Checkt ob eine gewisse Auflösung unter den Hardwarebedingungen möglich ist *
 * -------------------------------------------------------------------------- */
int          client_supportedmode(SDL_Rect     *rect);

/* -------------------------------------------------------------------------- *
 * Sucht nach der bestmöglichen Videomode für die gewünschte Auflösung        *
 * -------------------------------------------------------------------------- */
SDL_Rect    *client_bestmode     (Uint16        w,
                                  Uint16        h);

/* -------------------------------------------------------------------------- *
 * Liest die möglichen Video-Modes aus und addet diese in ein Drop-Down       *
 * widget                                                                     *
 * -------------------------------------------------------------------------- */
void         client_addmodes     (sgWidget     *dropdown);  

/* -------------------------------------------------------------------------- *
 * Öffnet eine Datei und sucht dabei alle Pfade in denen der Tichu-Client     *
 * seine Daten finden könnte ab.                                              *
 * -------------------------------------------------------------------------- */
FILE        *client_open_fp      (const char   *file, 
                                  int           mode);

/* -------------------------------------------------------------------------- *
 * Öffnet eine Datei als SDL_RWops                                            *
 * -------------------------------------------------------------------------- */
SDL_RWops   *client_open_rwops   (const char   *file,
                                  int           mode);
  
/* -------------------------------------------------------------------------- *
 * Gibt den Dateinamen-Teil eines Pfades zurück                               *
 * -------------------------------------------------------------------------- */
extern char *client_basename     (const char   *path);

/* -------------------------------------------------------------------------- *
 * Lädt ein 32-bit GIF Bild in eine SDL Surface                               *
 * -------------------------------------------------------------------------- */
void         client_load_image32 (SDL_Surface **surface,
                                  const char   *filename);

/* -------------------------------------------------------------------------- *
 * Lädt ein 32-bit PNG Bild in eine SDL Surface                               *
 * -------------------------------------------------------------------------- */
void         client_load_png     (SDL_Surface **surface, 
                                  const char   *filename);

/* -------------------------------------------------------------------------- *
 * Lädt ein 32x32 Icon und setzt dieses als Windowmanager Icon                *
 * -------------------------------------------------------------------------- */
void         client_load_icon    (const char   *filename);
  
/* -------------------------------------------------------------------------- *
 * Lädt ein PNG Bild und teilt diese in <count> Bilder auf, welche in einem   *
 * Array mit Surface Pointern abgelegt werden                                 *
 * -------------------------------------------------------------------------- */
void         client_load_icons   (const char   *filename, 
                                  SDL_Surface **sf, 
                                  int           count);

/* -------------------------------------------------------------------------- *
 * Erstellt einen Hintergrund, bestehend aus dem Hintergrundmuster und einem  *
 * optinalen Bild                                                             *
 * -------------------------------------------------------------------------- */
void         client_load_bg      (SDL_Surface **dest,
                                  const char   *bg, 
                                  int           align);  

/* -------------------------------------------------------------------------- *
 * Eine Zeile in die Konsole schreiben                                        *
 * -------------------------------------------------------------------------- */
void         client_puts         (const char   *format, 
                                  ...);

/* -------------------------------------------------------------------------- *
 * Schreibt einen Log-Eintrag auf die Konsole und/oder auf stderr             *
 * -------------------------------------------------------------------------- */
void         client_write        (int           mod,
                                  int           lev,
                                  const char   *s, 
                                  ...);

/* -------------------------------------------------------------------------- *
 * Nimmt Eingaben eines Input Widgets entgegen und sendet sie an den Server.  *
 * Es kann sich dabei um eine Nachricht an das aktuelle Spiel, an einen Mit-  *
 * spieler oder um einen Befehl handeln. Letzterer beginnt mit /              *
 * -------------------------------------------------------------------------- */
void         client_message      (char         *msg);
  
/* -------------------------------------------------------------------------- *
 * Behandelt Netzwerk-Ereignisse                                              *
 * -------------------------------------------------------------------------- */
void         client_event        (int           type,
                                  char         *msg);
  
/* -------------------------------------------------------------------------- *
 * Rendert eine Surface mit dem Hintergrund-Muster                            *
 * -------------------------------------------------------------------------- */
SDL_Surface *client_render       (void);

/* -------------------------------------------------------------------------- *
 * Bildschirm updaten                                                         *
 * -------------------------------------------------------------------------- */
void         client_flip         (void);

/* -------------------------------------------------------------------------- *
 * Hauptschleife des Clients                                                  *
 * -------------------------------------------------------------------------- */
void         client_run          (void);

/* -------------------------------------------------------------------------- *
 * Hauptfunktion des Clients                                                  *
 * -------------------------------------------------------------------------- */
#define client_main main

int          client_main         (int           argc, 
                                  char         *argv[]);
#endif /* CLIENT_H */
