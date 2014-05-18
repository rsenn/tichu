/* $Id: reply.h,v 1.10 2005/05/21 08:27:20 smoli Exp $
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

#ifndef REPLY_H
#define REPLY_H

/* -------------------------------------------------------------------------- *
 * Das REPLY Modul verarbeitet die Daten die vom Server kommen und            *
 * aktualisiert damit die Engine                                              *
 * -------------------------------------------------------------------------- */
#include "action.h"
#include "net.h"

/* -------------------------------------------------------------------------- *
 * Makros fürs Loggen                                                         *
 * -------------------------------------------------------------------------- */
#define reply_log(l, s...) writelog(MOD_REPLY, (l), s)

#if DEBUG_RPL
#define reply_debug(l, s...) reply_log((l), s)
#else
#define reply_debug(l, s...)
#endif

/* -------------------------------------------------------------------------- *
 * Datentypen                                                                 *
 * -------------------------------------------------------------------------- */
typedef void (*reply_callback)(char *prefix, int argc, char *argv[]);

struct reply 
{
  const char    *name;
  int            args;
  int            maxargs;
  reply_callback callback;
}; 

extern struct reply reply_table[];

/* -------------------------------------------------------------------------- *
 * Verarbeitet eine eingehende Nachricht vom Server                           *
 * -------------------------------------------------------------------------- */
extern int          reply_parse(char *msg);
  
#endif /* REPLY_H */
