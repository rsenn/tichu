/* $Id: sound.h,v 1.11 2005/05/22 02:44:34 smoli Exp $
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

#ifndef SOUND_H
#define SOUND_H

#include <limits.h>

/* -------------------------------------------------------------------------- *
 * Das SOUND Modul lädt und mischt Soundeffekte und die Hintergrundmusik      *
 * -------------------------------------------------------------------------- */
#include "dlink.h"
#include "mixer.h"
#include "ini.h"

/* -------------------------------------------------------------------------- *
 * Makros fürs Loggen                                                         *
 * -------------------------------------------------------------------------- */
#define sound_log(l, s...) writelog(MOD_SOUND, (l), s)

#ifdef DEBUG_SOUND
#define sound_debug(l, s...) sound_log((l), s)
#else
#define sound_debug(l, s...)
#endif

/* -------------------------------------------------------------------------- *
 * Konstanten                                                                 *
 * -------------------------------------------------------------------------- */
#define SOUND_CHANNELS 8                  /* Anzahl Effektkanäle */
#define SOUND_DELAY    1000               /* Verzögerung fürs faden */
#define SOUND_PLAYLIST "playlist.ini"

#define SOUND_DEFAULT_MUS_VOL 192         /* Default Musikvolumen */
#define SOUND_DEFAULT_FX_VOL   64         /* Default Effektvolumen */

/* Status-Flags */
#define SOUND_AVAIL   0x01
#define SOUND_PLAYING 0x02
#define SOUND_PAUSED  0x04

/* -------------------------------------------------------------------------- *
 * Datentypen                                                                 *
 * -------------------------------------------------------------------------- */
struct sound_mus
{
  struct node node;
  Mix_Music  *music;

  char        fname[PATH_MAX];
  char        title[256];
  char        artist[64];

  char       *infostr;      /* Informationen aus der Instrumenten-Liste */
  size_t      infolen;      /* Länge des Info-Strings */

  Uint32      start;        /* Zeitpunkt als das Abspielen begann */
  Uint32      stop;         /* Zeitpunkt zu dem pausiert wurde */
  Uint32      length;       /* Länge des ganzen Tracks */
  
  int         patterns;
  int         pos;
};

struct sound_fx
{
  struct node node;
  Mix_Chunk  *effect;
  char        name[PATH_MAX];
  char        path[256];
};

struct sound_config
{
  uint8_t     mus;
  uint8_t     fx;
  const char *track;
};

/* -------------------------------------------------------------------------- *
  * Globale Variabeln                                                          *
  * -------------------------------------------------------------------------- */
extern struct list         sound_playlist;        /* Liste aller Soundtracks */
extern struct list         sound_effects;         /* Liste aller F/X */
extern int                 sound_status;   
extern struct sound_mus   *sound_music;           /* Aktuelles Musikstück */
extern struct sound_config sound_config;

/* -------------------------------------------------------------------------- *
 * Sound-Modul initialisieren                                                 *
 * -------------------------------------------------------------------------- */
void                       sound_init           (void);

/* -------------------------------------------------------------------------- *
 * Sound-Modul herunterfahren                                                 *
 * -------------------------------------------------------------------------- */
void                       sound_shutdown       (void);

/* -------------------------------------------------------------------------- *
 * Konfiguriert das Sound-Modul                                               *
 * -------------------------------------------------------------------------- */
void                       sound_configure      (struct ini       *ini);

/* -------------------------------------------------------------------------- *
 * Speichert Sound-Konfiguration                                              *
 * -------------------------------------------------------------------------- */
int                        sound_save           (struct ini       *ini);
  
/* -------------------------------------------------------------------------- *
 * Verarbeitet die gemixten Sounddaten                                        *
 * -------------------------------------------------------------------------- */
void                       sound_postmix        (void             *udata,
                                                 Uint8            *stream, 
                                                 int               len);

/* -------------------------------------------------------------------------- *
 * Checkt den aktuellen Status                                                *
 * -------------------------------------------------------------------------- */
void                       sound_update         (void);

/* -------------------------------------------------------------------------- *
 * Lädt einen Soundtrack                                                      *
 * -------------------------------------------------------------------------- */
Mix_Music                 *sound_mus_open       (struct sound_mus *mus);
  
/* -------------------------------------------------------------------------- *
 * Gibt einen Soundtrack wieder frei                                          *
 * -------------------------------------------------------------------------- */
void                       sound_mus_close      (struct sound_mus *mus);

/* -------------------------------------------------------------------------- *
 * Fügt Daten an den Infostring eines Musiktracks                             *
 * -------------------------------------------------------------------------- */
void                       sound_mus_addinfo    (struct sound_mus *mus, 
                                                 char             *info);

/* -------------------------------------------------------------------------- *
 * Liest Infos über einen Musiktrack aus der Playlist                         *
 * -------------------------------------------------------------------------- */
int                        sound_mus_getinfo    (struct sound_mus *mus, 
                                                 struct ini       *ini);
  
/* -------------------------------------------------------------------------- *
 * Liest Infos aus einem Musiktrack                                           *
 * -------------------------------------------------------------------------- */
int                        sound_mus_readinfo   (struct sound_mus *mus);
  
/* -------------------------------------------------------------------------- *
 * Titel setzen                                                               *
 * -------------------------------------------------------------------------- */
int                        sound_mus_set_title  (struct sound_mus *mus, 
                                                 char             *title);

/* -------------------------------------------------------------------------- *
 * Artist setzen                                                              *
 * -------------------------------------------------------------------------- */
int                        sound_mus_set_artist (struct sound_mus *mus, 
                                                 char             *artist);
    
/* -------------------------------------------------------------------------- *
 * Findet einen Soundtrack nach Namen                                         *
 * -------------------------------------------------------------------------- */
struct sound_mus          *sound_mus_find       (const char       *name);

/* -------------------------------------------------------------------------- *
 * Wenn ein Stück fertig ist ruft der Mixer diese Funktion auf                *
 * -------------------------------------------------------------------------- */
void                       sound_mus_finish     (void);
  
/* -------------------------------------------------------------------------- *
 * Stoppt das Abspielen des aktuellen Soundtracks                             *
 * -------------------------------------------------------------------------- */
void                       sound_mus_stop       (void);

/* -------------------------------------------------------------------------- *
 * Spielt einen Soundtrack ab                                                 *
 * -------------------------------------------------------------------------- */
void                       sound_mus_play       (struct sound_mus *mus);

/* -------------------------------------------------------------------------- *
 * Pausiert das Abspielen der Musik                                           *
 * -------------------------------------------------------------------------- */
void                       sound_mus_pause      (void);

/* -------------------------------------------------------------------------- *
 * Gibt einen Soundtrack wieder frei                                          *
 * -------------------------------------------------------------------------- */
void                       sound_mus_free       (struct sound_mus *mus);

/* -------------------------------------------------------------------------- *
 * Liest das Musikvolumen aus (0-100)                                         *
 * -------------------------------------------------------------------------- */
int                        sound_mus_getvol     (void);
  
/* -------------------------------------------------------------------------- *
 * Setzt das Musikvolumen (0-100)                                             *
 * -------------------------------------------------------------------------- */
void                       sound_mus_setvol     (int               vol);  

/* -------------------------------------------------------------------------- *
 * Scannt alle Verzeichnisse nach Soundtracks ab                              *
 * -------------------------------------------------------------------------- */
void                       sound_mus_scan       (void);

/* -------------------------------------------------------------------------- *
 * Scannt ein Verzeichnis nach Soundtracks ab                                 *
 * -------------------------------------------------------------------------- */
void                       sound_mus_scandir    (const char       *path,
                                                 struct ini       *ini);

/* -------------------------------------------------------------------------- *
 * Speichert die Playlist ab                                                  *
 * -------------------------------------------------------------------------- */
int                        sound_mus_save       (void);

/* -------------------------------------------------------------------------- *
 * Lädt einen Soundeffekt                                                     *
 * -------------------------------------------------------------------------- */
struct sound_fx           *sound_fx_open        (const char       *file);
  
/* -------------------------------------------------------------------------- *
 * Findet einen Soundeffekt nach Namen                                        *
 * -------------------------------------------------------------------------- */
struct sound_fx           *sound_fx_find        (const char       *name);
  
/* -------------------------------------------------------------------------- *
 * Spielt einen Soundeffekt ab                                                *
 * -------------------------------------------------------------------------- */
void                       sound_fx_play        (struct sound_fx  *fx);

/* -------------------------------------------------------------------------- *
 * Gibt einen Soundeffekt wieder frei                                         *
 * -------------------------------------------------------------------------- */
void                       sound_fx_free        (struct sound_fx  *fx);

/* -------------------------------------------------------------------------- *
 * Liest das F/X Volumen aus (0-255)                                          *
 * -------------------------------------------------------------------------- */
int                        sound_fx_getvol      (void);

/* -------------------------------------------------------------------------- *
 * Setzt das F/X Volumen (0-255)                                              *
 * -------------------------------------------------------------------------- */
void                       sound_fx_setvol      (int               vol);  

#endif /* SOUND_H */


