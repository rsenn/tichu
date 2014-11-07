/* $Id: ini.h,v 1.15 2005/05/21 22:36:09 smoli Exp $
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

#ifndef INI_H
#define INI_H

#include <stdio.h>
#include <stdint.h>
#include <libsgui/sgui.h>

/* -------------------------------------------------------------------------- *
 * Das INI Modul liest und schreibt Dateien im .ini Format, welches für das   *
 * CFG Modul benötigt wird                                                    *
 * -------------------------------------------------------------------------- */
#include "dlink.h"
#include "gfxutil.h"

/* -------------------------------------------------------------------------- *
 * Makros fürs Loggen                                                         *
 * -------------------------------------------------------------------------- */
#define ini_log(l, s...) writelog(MOD_INI, l, s)

#ifdef DEBUG_INI
#define ini_debug(l, s...) ini_log(l, s)
#else
#define ini_debug(l, s...)
#endif

/* -------------------------------------------------------------------------- *
 * Konstanten                                                                 *
 * -------------------------------------------------------------------------- */

/* Lese-/Schreibmodus */
#define INI_READ  (CLIENT_READ)
#define INI_WRITE (CLIENT_WRITE)
#define INI_RDWR  (CLIENT_WRITE|CLIENT_READ)

/* .ini wurde modifiziert und muss beim Schliessen gespeichert werden */
#define INI_MODIFIED 0x80000000

/* Auf Windows werden die Zeilen mit CR/LF terminiert */
#ifdef WIN32
#define INI_NEWLINE "\r\n"
#else
#define INI_NEWLINE "\n"
#endif

/* -------------------------------------------------------------------------- *
 * Datenstrukturen                                                            *
 * -------------------------------------------------------------------------- */

/* Ein INI-Schlüssel (Name=Wert-Paar) 
 * -------------------------------------------------------------------------- */
struct key 
{
  struct node     node;
  uint32_t        hash;
  char           *name;
  char           *value;
};

/* Eine INI-Sektion (enthält Schlüssel) 
 * -------------------------------------------------------------------------- */
struct section 
{
  struct node     node;
  struct list     keys;  
  unsigned int    nkeys; /* Effektive Keys (ohne Whitespace und Comments) */
  uint32_t        hash;  /* 32-bit Hash vom Namen */
  char           *name;
};

/* Eine INI-Datei (enthält Sektionen und Sektionslose Schlüssel) 
 * -------------------------------------------------------------------------- */
struct ini 
{
  FILE           *file;
  int             mode;
  char           *path;
  struct list     sections;
  unsigned int    nsections;   /* Effektive Sektionen */
  struct section *section;     /* Aktuelle Sektion */
};

/* -------------------------------------------------------------------------- *
 * .ini Datei öffnen und falls erfolgreich, eine neue Instanz erstellen       *
 * -------------------------------------------------------------------------- */
struct ini   *ini_open             (const char       *filename, 
                                    int               mode);

/* -------------------------------------------------------------------------- *
 * INI-Datei schliessen und alles freigeben                                   *
 * -------------------------------------------------------------------------- */
void          ini_close            (struct ini   *ini);

/* -------------------------------------------------------------------------- *
 * Alle Sektionen einer INI-Datei laden                                       *
 * -------------------------------------------------------------------------- */
int           ini_load             (struct ini       *ini);
  
/* -------------------------------------------------------------------------- *
 * Alle Sektionen einer INI-Datei speichern                                   *
 * -------------------------------------------------------------------------- */
int           ini_save             (struct ini       *ini);

/* -------------------------------------------------------------------------- *
 * INI Daten löschen                                                          *
 * -------------------------------------------------------------------------- */
void          ini_clear            (struct ini   *ini);

/* -------------------------------------------------------------------------- *
 * Sektion suchen, erstellen falls nicht gefunden und im Schreib-Modus, und   *
 * dann als aktuelle Sektion setzen                                           *
 * Gibt 1 zurück, falls eine Sektion erstellt werden musste                   *
 * -------------------------------------------------------------------------- */
int           ini_section          (struct ini   *ini, 
                                    const char   *name,
                                    ...);

/* -------------------------------------------------------------------------- *
 * Einen String als Schlüssel in die aktuelle INI-Sektion schreiben           *
 * -------------------------------------------------------------------------- */
int           ini_puts             (struct ini   *ini, 
                                    const char   *key,
                                    const char   *s);
  
/* -------------------------------------------------------------------------- *
 * Einen Long-Integer als Schlüssel in die aktuelle INI-Sektion schreiben     *
 * -------------------------------------------------------------------------- */
int           ini_putlong          (struct ini   *ini,
                                    const char   *key,
                                    long          l);
  
/* -------------------------------------------------------------------------- *
 * Einen Unsigned Long-Integer als Schlüssel in die aktuelle INI-Sektion      *
 * schreiben                                                                  *
 * -------------------------------------------------------------------------- */
int           ini_putulong         (struct ini   *ini,
                                    const char   *key,
                                    unsigned long u);

/* -------------------------------------------------------------------------- *
 * Einen double als Schlüssel in die aktuelle INI-Sektion schreiben           *
 * -------------------------------------------------------------------------- */
int           ini_putdouble        (struct ini   *ini,
                                    const char   *key,
                                    double        d);
  
/* -------------------------------------------------------------------------- *
 * Eine Farbe im HTML-Format als Schlüssel in die aktuelle INI-Sektion        *
 * schreiben                                                                  *
 * -------------------------------------------------------------------------- */
int           ini_putcolor         (struct ini   *ini,
                                    const char   *key,
                                    struct color  color);

/* -------------------------------------------------------------------------- *
 * Eine Farbe im HSV-Format als Schlüssel in die aktuelle INI-Sektion         *
 * schreiben                                                                  *
 * -------------------------------------------------------------------------- */
int           ini_puthsv           (struct ini   *ini,
                                    const char   *key,
                                    sgHSV         hsv);

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und String zurückgeben                            *
 * -------------------------------------------------------------------------- */
char         *ini_gets             (struct ini   *ini,
                                    const char   *key);

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Long Integer zurückgeben                      *
 * -------------------------------------------------------------------------- */
long          ini_getlong          (struct ini   *ini, 
                                    const char   *key);

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Unsigned Long Integer zurückgeben             *
 * -------------------------------------------------------------------------- */
unsigned long ini_getulong         (struct ini   *ini, 
                                    const char   *key);

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und double zurückgeben                            *
 * -------------------------------------------------------------------------- */
double        ini_getdouble        (struct ini   *ini, 
                                    const char   *key);

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Farbe zurückgeben                             *
 * -------------------------------------------------------------------------- */
struct color  ini_getcolor         (struct ini   *ini,
                                    const char   *key);

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und HSV-Farbe zurückgeben                         *
 * -------------------------------------------------------------------------- */
sgHSV        *ini_gethsv           (struct ini   *ini,
                                    const char   *key);

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und String zurückgeben falls gefunden, sonst def  *
 * -------------------------------------------------------------------------- */
char         *ini_gets_default     (struct ini   *ini,
                                    const char   *key, 
                                    char         *def);
/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Long Integer zurückgeben falls gefunden,      *
 * sonst default                                                              *
 * -------------------------------------------------------------------------- */
long          ini_getlong_default  (struct ini   *ini,
                                    const char   *key,
                                    long          def);

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Unsigned Long Integer zurückgeben falls       *
 * gefunden, sonst default                                                    *
 * -------------------------------------------------------------------------- */
unsigned long ini_getulong_default (struct ini   *ini,
                                    const char   *key,
                                    unsigned long def);

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und double zurückgeben falls gefunden, sonst def  *
 * -------------------------------------------------------------------------- */
double        ini_getdouble_default(struct ini   *ini,
                                    const char   *key,
                                    double        def);
  
/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Farbe zurückgeben falls gefunden, sonst def   *
 * -------------------------------------------------------------------------- */
struct color  ini_getcolor_default (struct ini   *ini,
                                    const char   *key, 
                                    struct color  def);  

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Farbe zurückgeben falls gefunden, sonst def   *
 * -------------------------------------------------------------------------- */
sgHSV         ini_gethsv_default   (struct ini   *ini,
                                    const char   *key, 
                                    sgHSV         def);
  
/* -------------------------------------------------------------------------- *
 * Alle Daten einer INI Instanz ausgeben                                      *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void          ini_dump            (struct ini   *ini);
#endif /* DEBUG */

#endif /* INI_H */
