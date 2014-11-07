/* $Id: net.h,v 1.14 2005/05/21 08:27:20 smoli Exp $
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

#ifndef NET_H
#define NET_H

#include <stdarg.h>
#include <SDL_net.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

#include "ini.h"

/* -------------------------------------------------------------------------- *
 * Makros fürs Loggen                                                         *
 * -------------------------------------------------------------------------- */
#define net_log(l, s...) writelog(MOD_NET, (l), s)

#if DEBUG_NET
#define net_debug(l, s...) net_log((l), s)
#else
#define net_debug(l, s...)
#endif

/* Konstanten für Callbacks
 * -------------------------------------------------------------------------- */
#define NET_READ  0
#define NET_WRITE 1
#define NET_ERROR 2

/* Statusflags
 * -------------------------------------------------------------------------- */
#define NET_CONNECTED 0x01
#define NET_SHUTUP    0x02

/* Millisekunden-Interval in dem nach neuen Daten auf dem Socket gecheckt wird
 * -------------------------------------------------------------------------- */
#define NET_INTERVAL 5

/* Netzwerk-Defaults
 * -------------------------------------------------------------------------- */
#define NET_DEFAULT_PORT 2222
#define NET_DEFAULT_HOST "localhost"

/* -------------------------------------------------------------------------- *
 * Callback Funktionstyp                                                      *
 * -------------------------------------------------------------------------- */
typedef void        (*net_callback)(int type, char *msg);

/* -------------------------------------------------------------------------- *
 * Globale Variabeln                                                          *
 * -------------------------------------------------------------------------- */
extern int              net_status;
extern char             net_host[512];
extern char             net_ip[32];
extern unsigned short   net_port;
extern IPaddress        net_address;
extern char             net_error[512];
extern TCPsocket        net_socket;
extern SDLNet_SocketSet net_sockset;
extern struct fqueue    net_recvq;
extern struct fqueue    net_sendq;
extern net_callback     net_callbacks[3];

/* -------------------------------------------------------------------------- *
 * Net Initialisation                                                         *
 * -------------------------------------------------------------------------- */
void           net_init     (void);

/* -------------------------------------------------------------------------- *
 * Net Shutdown                                                               *
 * -------------------------------------------------------------------------- */
void           net_shutdown (void);

/* -------------------------------------------------------------------------- *
 * Netzwerk-Konfiguration lesen                                               *
 * -------------------------------------------------------------------------- */
void           net_configure(struct ini  *ini);

/* -------------------------------------------------------------------------- *
 * Flags setzen                                                               *
 * -------------------------------------------------------------------------- */
int            net_set      (int          flags);

/* -------------------------------------------------------------------------- *
 * Flags löschen                                                              *
 * -------------------------------------------------------------------------- */
int            net_unset    (int          flags);  

/* -------------------------------------------------------------------------- *
 * Löst den angegebenen Hostnamen auf und speichert ihn, falls erfolgreich,   *
 * zusammen mit dem Port.                                                     *
 * -------------------------------------------------------------------------- */
int            net_resolve  (void);
  
/* -------------------------------------------------------------------------- *
 * Startet eine Verbindung                                                    *
 * -------------------------------------------------------------------------- */
int            net_connect  (void);

/* -------------------------------------------------------------------------- *
 * Setzt einen Callback für Read/Write/Error Events                           *
 * -------------------------------------------------------------------------- */
int            net_register (int          type, 
                             net_callback callback);

/* -------------------------------------------------------------------------- *
 * Prüft den Status der Netzwerkverbindung.                                   *
 * Gibt verbleibende Zeit zurück falls ein Event reingekommen ist, sonst      *
 * wird 'timeout' Millisekunden lang gewartet und dann mit einem Rückgabewert *
 * von 0 abgebrochen.                                                         * 
 * -------------------------------------------------------------------------- */
int            net_poll     (int          dummy,
                             int          timeout);

/* -------------------------------------------------------------------------- *
 * Schliesst die Netzwerkverbindung und räumt die Socket-Sets und Queues auf. *
 * -------------------------------------------------------------------------- */
void           net_close    (void);
  

/* -------------------------------------------------------------------------- *
 * Schreibt Daten in die Sende-Warteschlange                                  *
 * -------------------------------------------------------------------------- */
void           net_vsend    (const char  *format, va_list args);
void           net_send     (const char  *format, ...);

#endif /* NET_H */
