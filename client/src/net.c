/* $Id: net.c,v 1.23 2005/05/04 07:27:09 smoli Exp $
 * -------------------------------------------------------------------------- *
 *  .___.    .                                                                *
 *    |  * _.|_ . .        Portabler, SDL-basierender Client für das          *
 *    |  |(_.[ )(_|             Multiplayer-Kartenspiel Tichu.                *
 *  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .   *
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
#include <SDL.h>
#include <SDL_net.h>

#include "queue.h"
#include "client.h"
#include "net.h"
#include "ini.h"

/* -------------------------------------------------------------------------- *
 * Globale Variabeln                                                          *
 * -------------------------------------------------------------------------- */
int              net_status;
char             net_host[512];
char             net_ip[32];
unsigned short   net_port;
IPaddress        net_address;
char             net_error[512];
TCPsocket        net_socket;
SDLNet_SocketSet net_sockset;
struct fqueue    net_recvq;
struct fqueue    net_sendq;
net_callback     net_callbacks[3] = { NULL, NULL, NULL };

/* -------------------------------------------------------------------------- *
 * Net Initialisation                                                         *
 * -------------------------------------------------------------------------- */
void net_init(void)
{
  SDLNet_Init();  
  
  queue_zero(&net_recvq);
  queue_zero(&net_sendq);
 
  net_unset(NET_CONNECTED);
}

/* -------------------------------------------------------------------------- *
 * Net Shutdown                                                               *
 * -------------------------------------------------------------------------- */
void net_shutdown(void)
{
  if(net_sockset)
  {
    SDLNet_FreeSocketSet(net_sockset);
    net_sockset = NULL;
  }
  
  SDLNet_Quit();  
}

/* -------------------------------------------------------------------------- *
 * Netzwerk-Konfiguration lesen                                               *
 * -------------------------------------------------------------------------- */
void net_configure(struct ini *ini)
{
  ini_section(ini, "net");
  
  client_strlcpy(net_host, ini_gets_default(ini, "host", NET_DEFAULT_HOST), 
                 sizeof(net_host));
  
  net_port = ini_getulong_default(ini, "port", NET_DEFAULT_PORT);
}

/* -------------------------------------------------------------------------- *
 * Flags setzen                                                               *
 * -------------------------------------------------------------------------- */
int net_set(int flags)
{
  /* Müssen wir noch Flags setzen? */
  if((net_status & flags) != flags)
  {
    net_debug(VERBOSE, "Set [%s%s ]",
              ((~net_status) & flags & NET_CONNECTED) ? " CONNECTED" : "",
              ((~net_status) & flags & NET_SHUTUP) ? " SHUTUP" : "");
              
    /* Flags setzen */
    net_status |= flags;
    return 1;
  }
  
  return 0;
}
  
/* -------------------------------------------------------------------------- *
 * Flags löschen                                                              *
 * -------------------------------------------------------------------------- */
int net_unset(int flags)
{
  /* Müssen wir noch flags löschen? */
  if(net_status & flags)
  {
    net_debug(VERBOSE, "Unset [%s%s ]",
              (net_status & flags & NET_CONNECTED) ? " CONNECTED" : "",
              (net_status & flags & NET_SHUTUP) ? " SHUTUP" : "");
      
    /* Flags löschen */
    net_status &= ~flags;
    
    return 1;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Löst den angegebenen Hostnamen auf und speichert ihn, falls erfolgreich,   *
 * zusammen mit dem Port.                                                     *
 * -------------------------------------------------------------------------- */
int net_resolve(void)
{
  char *ip = (void *)&net_address;
  
  if(SDLNet_ResolveHost(&net_address, net_host, net_port))
    return -1;

  sprintf(net_ip, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  
  return 0;
}
  
/* -------------------------------------------------------------------------- *
 * Startet eine Verbindung                                                    *
 * -------------------------------------------------------------------------- */
int net_connect(void)
{
  net_socket = SDLNet_TCP_Open(&net_address);
  
  if(net_socket)
  {
    if(net_sockset)
      SDLNet_FreeSocketSet(net_sockset);
    
    net_sockset = SDLNet_AllocSocketSet(1);
    
    SDLNet_TCP_AddSocket(net_sockset, net_socket);
    
    net_set(NET_CONNECTED);
  }
  
  return (net_socket == NULL ? -1 : 0);
}

/* -------------------------------------------------------------------------- *
 * Setzt einen Callback für Read/Write/Error Events                           *
 * -------------------------------------------------------------------------- */
int net_register(int type, net_callback callback)
{
  net_callbacks[type] = callback;
  
  return type;
}

/* -------------------------------------------------------------------------- *
 * Prüft den Status der Netzwerkverbindung.                                   *
 * Gibt verbleibende Zeit zurück falls ein Event reingekommen ist, sonst      *
 * wird 'timeout' Millisekunden lang gewartet und dann mit einem Rückgabewert *
 * von 0 abgebrochen.                                                         *
 * -------------------------------------------------------------------------- */
int net_poll(int dummy, int timeout)
{
  char   buf[512];
  int    ret;
  size_t n;
  Uint32 before;
  int remain;

  if(!(net_status & NET_CONNECTED))
  {
    SDL_Delay(timeout);
    return 0;
  }
  
  SDLNet_TCP_AddSocket(net_sockset, net_socket);
  
  before = SDL_GetTicks();
  ret = SDLNet_CheckSockets(net_sockset, timeout);
  
  /* Daten verfügbar, probiere zu lesen */
  if(ret == 1)
  {
    ret = SDLNet_TCP_Recv(net_socket, buf, sizeof(buf));
    
    /* Wenn Daten empfangen wurden, dann 
       schreiben wir diese in die Warteschlange */
    if(ret > 0)
      queue_write(&net_recvq, buf, ret);
    else
    {
      net_close();
      
      if(net_callbacks[NET_ERROR])
        net_callbacks[NET_ERROR](NET_ERROR, SDLNet_GetError());
    }
  }
  
  /* Ist eine komplette Zeile in der Empfangswarteschlange, dann lesen 
     wir sie aus und übergeben sie dem NET_READ Callback */
  if(net_recvq.lines && !(net_status & NET_SHUTUP))
  {
    char line[4096];
    
    ret = queue_gets(&net_recvq, line, sizeof(line));
    
    if(ret > 0)
    {
      if(net_callbacks[NET_READ])
        net_callbacks[NET_READ](NET_READ, line);
    }
  }
  
  /* Ist eine komplette Zeile in der Sendewarteschlange, 
     dann versuchen wir sie zu senden */
  if(net_sendq.lines)
  {
    /* Bytes vom Anfang der Warteschlange lesen */
    n = queue_map(&net_sendq, buf, ((net_sendq.size > 512) ? 512 : net_sendq.size));
    
    ret = SDLNet_TCP_Send(net_socket, buf, n);
    
    /* Warteschlange kürzen wenn geschrieben wurde */
    if(ret > 0)
      queue_cut(&net_sendq, ret);
  }
  
  remain = SDL_GetTicks() - before - timeout;
  
  return remain > 0 ? remain : 0;
}

/* -------------------------------------------------------------------------- *
 * Schliesst die Netzwerkverbindung und räumt die Socket-Sets und Queues auf. *
 * -------------------------------------------------------------------------- */
void net_close(void)
{
  queue_destroy(&net_sendq);
  queue_destroy(&net_recvq);
  
  if(net_socket)
  {
    SDLNet_TCP_Close(net_socket);
    net_socket = NULL;
  }
  
  if(net_sockset)
  {
    SDLNet_FreeSocketSet(net_sockset);
    net_sockset = NULL;
  }

  net_unset(NET_CONNECTED);
}

/* -------------------------------------------------------------------------- *
 * Schreibt Daten in die Sende-Warteschlange                                  *
 * -------------------------------------------------------------------------- */
void net_vsend(const char *format, va_list args)
{
  char   buf[4096];
  size_t n;
  
  n = vsnprintf(buf, sizeof(buf) - 2, format, args);

  net_debug(INFO, "%s", buf);
  
  buf[n++] = '\n';
  buf[n++] = '\0';
  
  queue_puts(&net_sendq, buf);
}

/* Wrapper für varargs */
void net_send(const char *format, ...)
{
  va_list args;
  
  va_start(args, format);
  net_vsend(format, args);
  va_end(args);
}
