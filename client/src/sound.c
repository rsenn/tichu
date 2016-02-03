/* $Id: sound.c,v 1.15 2005/05/22 02:44:34 smoli Exp $
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

#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <libsgui/file.h>

/* -------------------------------------------------------------------------- *
 * Das SOUND Modul lädt und mischt Soundeffekte und die Hintergrundmusik      *
 * -------------------------------------------------------------------------- */
#include "mikmod.h"
#include "client.h"
#include "sound.h"

/* -------------------------------------------------------------------------- *
 * Globale Variabeln                                                          *
 * -------------------------------------------------------------------------- */
struct list         sound_playlist;        /* Liste aller Soundtracks */
struct list         sound_effects;         /* Liste aller F/X */
int                 sound_status;          /* Sound status */
struct sound_mus   *sound_music;
struct sound_config sound_config;
/* -------------------------------------------------------------------------- *
 * Sound-Modul initialisieren                                                 *
 * -------------------------------------------------------------------------- */
void sound_init(void)
{
  const SDL_version *version;
  const SDL_AudioSpec *fmt;
  
  /* Version des Mixers anzeigen */
  version = Mix_Linked_Version();
  
  sound_log(STATUS, "Initialisiere SDL_mixer v%u.%u.%u",
            version->major, version->minor, version->patch);
  
  /* Das Device öffnen */
  if((fmt = Mix_OpenAudio(44100/*MIX_DEFAULT_FREQUENCY*/, MIX_DEFAULT_FORMAT,
                          1, 4096)))
  {
    sound_log(STATUS, "Audio-Gerät geöffnet, %d bit %s audio (%s) @ %uHz",
              (fmt->format & 0xff),
              (fmt->format & 0x8000) ? "signed" : "unsigned",
              (fmt->channels > 2) ? "surround" :
              (fmt->channels > 1) ? "stereo" : "mono", fmt->freq);
    
    sound_status |= SOUND_AVAIL;
  }
  else
    sound_log(STATUS, "Kein Sound: %s", Mix_GetError());

  /* Ein paar Channels brauchen wir schon (klick, klick, klick :D) */
  Mix_AllocateChannels(SOUND_CHANNELS);
  
//  Mix_SetPostMix(sound_postmix, NULL);
  Mix_HookMusicFinished(sound_mus_finish);
  
  dlink_list_zero(&sound_playlist);
  dlink_list_zero(&sound_effects);
}

/* -------------------------------------------------------------------------- *
 * Sound-Modul herunterfahren                                                 *
 * -------------------------------------------------------------------------- */
void sound_shutdown(void)
{
  struct node *node;
  struct node *next;
  
  /* Playliste speichern */
  sound_mus_save();
  
  /* Alle Effekte cleanen */
  dlink_foreach_safe(&sound_effects, node, next)
    sound_fx_free(node->data);
  
  /* Alle Musiktracks cleanen */
  dlink_foreach_safe(&sound_playlist, node, next)
    sound_mus_free(node->data);
  
  /* Audio-Gerät wieder freigeben */
  Mix_CloseAudio();
}

/* -------------------------------------------------------------------------- *
 * Konfiguriert das Sound-Modul                                               *
 * -------------------------------------------------------------------------- */
void sound_configure(struct ini *ini)
{
  /* Sound-Sektion in der .ini lesen */
  if(ini_section(ini, "sound") == 0)
  {
    sound_fx_setvol(ini_getulong_default(ini, "fx", SOUND_DEFAULT_FX_VOL));
    sound_mus_setvol(ini_getulong_default(ini, "mus", SOUND_DEFAULT_MUS_VOL));
    
    sound_config.track = ini_gets(ini, "track");
  }
  else
  {
    sound_fx_setvol(SOUND_DEFAULT_FX_VOL);
    sound_mus_setvol(SOUND_DEFAULT_MUS_VOL);
  }
}

/* -------------------------------------------------------------------------- *
 * Speichert Sound-Konfiguration                                              *
 * -------------------------------------------------------------------------- */
int sound_save(struct ini *ini)
{
  ini_section(ini, "sound");
  
  ini_putulong(ini, "fx", sound_config.fx);
  ini_putulong(ini, "mus", sound_config.mus);
  
  ini_puts(ini, "track", (sound_music ? sound_music->fname : ""));
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Verarbeitet die gemixten Sounddaten                                        *
 * -------------------------------------------------------------------------- */
void sound_postmix(void *udata, Uint8 *stream, int len)
{
  sound_log(INFO, "mixer data: %u bytes", len);
}

/* -------------------------------------------------------------------------- *
 * Checkt den aktuellen Status                                                *
 * -------------------------------------------------------------------------- */
void sound_update(void)
{
  if(sound_music && Mix_PlayingMusic() && Mix_FadingMusic() == MIX_NO_FADING)
  {
    if((int)Mix_GetMusicPosition() == sound_music->pos)
      return;
    {
      sound_music->pos = Mix_GetMusicPosition();
      sound_music->patterns = Mix_GetMusicLength();
  
      /* Track nähert sich dem Ende -> Fadeout */
      if(sound_music->pos && sound_music->pos + 1 >= sound_music->patterns)
      {
        Uint32 elapsed = SDL_GetTicks() - sound_music->start;
        Uint32 per_pattern = elapsed / sound_music->pos;
        
        sound_log(INFO, "Fadeout in %ums", per_pattern);

        Mix_FadeOutMusic(per_pattern);
      }
    }
    
    sound_log(STATUS, "Position %i/%i", sound_music->pos, sound_music->patterns);    
  }
}

/* -------------------------------------------------------------------------- *
 * Lädt einen Soundtrack                                                      *
 * -------------------------------------------------------------------------- */
Mix_Music *sound_mus_open(struct sound_mus *mus)
{
  if(mus->music)
    return mus->music;
  
  if((mus->music = Mix_LoadMUS(mus->fname)) == NULL)
  {
    sound_log(STATUS, "Failed loading music file %s: %s", 
              mus->fname, Mix_GetError());
    return NULL;
  }

  if(mus->title[0] == '\0')
    sound_mus_readinfo(mus);

  return mus->music;
}

/* -------------------------------------------------------------------------- *
 * Gibt einen Soundtrack wieder frei                                          *
 * -------------------------------------------------------------------------- */
void sound_mus_close(struct sound_mus *mus)
{
  if(mus->music)
  {
    Mix_FreeMusic(mus->music);
    mus->music = NULL;
  }
}
  
/* -------------------------------------------------------------------------- *
 * Fügt Daten an den Infostring eines Musiktracks                             *
 * -------------------------------------------------------------------------- */
void sound_mus_addinfo(struct sound_mus *mus, char *info)
{
  size_t n;
  int i;
  int space = !!mus->infolen;

  if((n = strlen(info)))
  {
    if((mus->infostr = realloc(mus->infostr, mus->infolen + n + 2)))
    {
        
      for(i = 0; info[i]; i++)
      {
        if(isspace(info[i]))
        {
          space = 1;
          continue;
        }
        
        if(space)
        {
          mus->infostr[mus->infolen++] = ' ';

          space = 0;
        }
        
        mus->infostr[mus->infolen++] = info[i];
      }

      mus->infostr[mus->infolen] = '\0';
    }
    else
    {
      mus->infolen = 0;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * Alles Kleinbuchstaben?                                                     *
 * -------------------------------------------------------------------------- */
static int sound_mus_all_lower(const char *s)
{
  while(*s)
  {
    if(isalpha(*s) && isupper(*s))
      return 0;

    s++;
  }
  
  return 1;
}

/* -------------------------------------------------------------------------- *
 * Alles Grossbuchstaben?                                                     *
 * -------------------------------------------------------------------------- */
static int sound_mus_all_upper(const char *s)
{
  while(*s)
  {
    if(isalpha(*s) && islower(*s))
      return 0;
    
    s++;
  }
  
  return 1;
}

/* -------------------------------------------------------------------------- *
 * Titel säubern                                                              *
 * -------------------------------------------------------------------------- */
static void sound_mus_clean_title(char *title)
{
  int i;
  int capital = 1;
  
  for(i = 0; title[i]; i++)
  {
    if(isalpha(title[i]) && capital)
    {
      title[i] = toupper(title[i]);
      capital = 0;
      continue;
    }
    
    if(!isspace(title[i]) && !isalnum(title[i]))
    {
      capital = 1;
      continue;
    }
    
    if(isalpha(title[i]))
      title[i] = tolower(title[i]);
  }
}

/* -------------------------------------------------------------------------- *
 * Artist säubern                                                             *
 * -------------------------------------------------------------------------- */
static void sound_mus_clean_artist(char *artist)
{
  int i;
  int capital = 1;
  
  for(i = 0; artist[i]; i++)
  {
    if(isalpha(artist[i]) && capital)
    {
      artist[i] = toupper(artist[i]);
      capital = 0;
      continue;
    }
    
    if(!isalnum(artist[i]))
    {
      capital = 1;
      continue;
    }
    
    if(isalpha(artist[i]))
      artist[i] = tolower(artist[i]);
  }
}

/* -------------------------------------------------------------------------- *
 * Titel setzen                                                               *
 * -------------------------------------------------------------------------- */
int sound_mus_set_title(struct sound_mus *mus, char *title)
{
  int i;
  int space = 0;
  
  for(i = 0; *title && i < sizeof(mus->title) - 1;)
  {
    if(isspace(*title))
    {
      space = 1;
      title++;
      continue;
    }
    
    if(space)
    {
      if(i) mus->title[i++] = ' ';
      space = 0;
    }
    
    mus->title[i++] = *title++;
  }
  
  mus->title[i] = '\0';
  
  if(strlen(mus->title) < 2 || strstr(mus->title, "by"))
    return -1;
  
  if(sound_mus_all_lower(mus->title) || sound_mus_all_upper(mus->title))
    sound_mus_clean_title(mus->title);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Artist setzen                                                              *
 * -------------------------------------------------------------------------- */
int sound_mus_set_artist(struct sound_mus *mus, char *artist)
{
  char *p;
  int i;
  int space = 0;  
  
  mus->artist[0] = '\0';
  
  if((p = strstr(artist, "by ")) || (p = strstr(artist, "By ")))
  {
    artist = p + 3;
  }
  else if((p = strchr(artist, '@')))
  {
    if(!(p > artist && isalnum(p[-1]) && isalnum(p[1])))
      return -1;
    
    while(p > artist && !isspace(*p))
      p--;
    
    artist = p;
  }
  else
    return -1;

  for(i = 0; *artist && i < sizeof(mus->artist) - 1;)
  {
    if(isspace(*artist))
    {
      space = 1;
      artist++;
      continue;
    }
    
    if(space)
    {
      if(i) mus->artist[i++] = ' ';
      space = 0;
    }
    
    mus->artist[i++] = *artist++;
  }
  
  mus->artist[i] = '\0';
  
  if((sound_mus_all_lower(mus->artist) && !strchr(mus->artist, '@')) || 
     sound_mus_all_upper(mus->artist))
    sound_mus_clean_artist(mus->artist);
  
  return 0;
}
  
/* -------------------------------------------------------------------------- *
 * Liest Infos über einen Musiktrack aus der Playlist                         *
 * -------------------------------------------------------------------------- */
int sound_mus_getinfo(struct sound_mus *mus, struct ini *ini)
{
  char *s;
  
  /* Sind die Infos in der Playlist vorhanden? */
  if(ini_section(ini, "%s", client_basename(mus->fname)))
    return -1;
  
  if((s = ini_gets(ini, "title")))
  {
    strncpy(mus->title, s, sizeof(mus->title));
    mus->title[sizeof(mus->title) - 1] = '\0';
  }
  
  if((s = ini_gets(ini, "artist")))
  {
    strncpy(mus->artist, s, sizeof(mus->artist));
    mus->artist[sizeof(mus->artist) - 1] = '\0';
  }
  
  if((s = ini_gets(ini, "info")))
  {
    if(mus->infostr)
      free(mus->infostr);
    
    if((mus->infostr = strdup(s)))
      mus->infolen = strlen(mus->infostr);
  }  

  mus->length = ini_getulong_default(ini, "length", 0L);
  mus->patterns = ini_getulong_default(ini, "patterns", 0L);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Liest Infos aus einem Musiktrack                                           *
 * -------------------------------------------------------------------------- */
int sound_mus_readinfo(struct sound_mus *mus)
{
  Mix_Music *music;
  MODULE *module;
  char *title;
  char *artist;
  int unload = 0;
  int ii = 0;
  
  mus->title[0] = 1;
  
  /* Soundtrack laden */
  if((music = mus->music) == NULL)
  {
    if((music = sound_mus_open(mus)) == NULL)
      return -1;
    
    unload = 1;
  }
  
  /* Module-Struktur */
  if((module = music->data.module) == NULL)
    return -1;

  /* Track name lesen */
  for(title = module->songname; ii < module->numins;)
  {
    if(sound_mus_set_title(mus, title))
      title = module->instruments[ii++].insname;
    else
      break;
  }

  /* Artist lesen */
  for(ii = 0; ii < module->numins;)
  {
    artist = module->instruments[ii++].insname;
    
    if(!sound_mus_set_artist(mus, artist))
      break;
  }

  /* Ganze module-info laden */
  for(ii = 0; ii < module->numins; ii++)
  {
    if(module->instruments[ii].insname)
      sound_mus_addinfo(mus, module->instruments[ii].insname);
  }
  
  if(unload)
    sound_mus_close(mus);
  
  return 0;
}
  
/* -------------------------------------------------------------------------- *
 * Lädt einen Musiktrack in die Playlist                                      *
 * -------------------------------------------------------------------------- */
struct sound_mus *sound_mus_add(const char *file, struct ini *ini)
{
  struct sound_mus *mus;

  if((mus = malloc(sizeof(struct sound_mus))) == NULL)
    return NULL;

  mus->music = NULL;
  mus->title[0] = '\0';
  mus->infostr = NULL;
  mus->infolen = 0;

  mus->start = 0;
  mus->stop = 0;
  mus->length = 0;
  
  mus->pos = 0;
  mus->patterns = 0;
  
  client_strlcpy(mus->fname, file, sizeof(mus->fname));
  dlink_add_tail(&sound_playlist, &mus->node, mus);
  
  if(sound_mus_getinfo(mus, ini))
    sound_mus_readinfo(mus);
  
  sound_log(STATUS, "Soundtrack %s: %s", mus->fname, mus->title);
  
  return mus;
}

/* -------------------------------------------------------------------------- *
 * Findet einen Soundtrack nach Namen                                         *
 * -------------------------------------------------------------------------- */
struct sound_mus *sound_mus_find(const char *name)
{
  struct sound_mus *mus;
  
  dlink_foreach(&sound_playlist, mus)
  {
    if(!strcmp(mus->title, name) || !strcmp(mus->fname, name))
      return mus;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Spielt einen Soundtrack ab                                                 *
 * -------------------------------------------------------------------------- */
void sound_mus_play(struct sound_mus *mus)
{
  if(Mix_PlayingMusic())
  {
    sound_log(INFO, "Da spielt bereits ein Track!");
    return;
  }
  
  if(mus->music == NULL)
    sound_mus_open(mus);
  
  if(mus->music)
  {
    sound_music = mus;
    sound_status |= SOUND_PLAYING;
    
    mus->start = SDL_GetTicks();
    mus->stop = 0;
    
    mus->pos = -1;
    
    Mix_FadeInMusic(mus->music, 0, SOUND_DELAY);

    sound_log(INFO, "Spiele Soundtrack: %s", mus->title);
  }
  else
  {
    sound_log(INFO, "Fehler beim Laden von '%s': %s", 
              mus->title, Mix_GetError());
  }
}

/* -------------------------------------------------------------------------- *
 * Wenn ein Stück fertig ist ruft der Mixer diese Funktion auf                *
 * -------------------------------------------------------------------------- */
void sound_mus_finish(void)
{
  if(sound_music)
  {
    Uint32 seconds;
    
    /* Länge des Tracks berechnen */
    if(sound_music->length == 0)
      sound_music->length = SDL_GetTicks() - sound_music->start;
    
    seconds = sound_music->length / 1000;
    
    sound_log(INFO, "Track '%s' fertig gespielt (%02u:%02u Total)",
              sound_music->title, (seconds % 3600) / 60, seconds % 60);              
  }
}
  
/* -------------------------------------------------------------------------- *
 * Stoppt das abspielen des aktuellen Soundtracks                             *
 * -------------------------------------------------------------------------- */
void sound_mus_stop(void)
{
  /* Kein soundtrack am spielen? */
  if(sound_music == NULL)
    return;
  
  
}
  
/* -------------------------------------------------------------------------- *
 * Pausiert das Abspielen der Musik                                           *
 * -------------------------------------------------------------------------- */
void sound_mus_pause(void)
{
  if(!(sound_status & SOUND_PAUSED))
  {
    sound_status |= SOUND_PAUSED;
    Mix_FadeOutMusic(500);
  }
}

/* -------------------------------------------------------------------------- *
 * Löscht einen Soundtrack aus der Playlist                                   *
 * -------------------------------------------------------------------------- */
void sound_mus_free(struct sound_mus *mus)
{
  Mix_FreeMusic(mus->music);
  
  dlink_delete(&sound_playlist, &mus->node);

  if(mus->infostr)
    free(mus->infostr);

  free(mus);
}
  
/* -------------------------------------------------------------------------- *
 * Liest das Musikvolumen aus (0-100)                                         *
 * -------------------------------------------------------------------------- */
int sound_mus_getvol(void)
{
  return sound_config.mus;
}

/* -------------------------------------------------------------------------- *
 * Setzt das Musikvolumen (0-255)                                             *
 * -------------------------------------------------------------------------- */
void sound_mus_setvol(int vol)
{
  sound_config.mus = vol;
  Mix_VolumeMusic(vol * 128 / 255);
}

/* -------------------------------------------------------------------------- *
 * Scannt alle Verzeichnisse nach Soundtracks ab                              *
 * -------------------------------------------------------------------------- */
void sound_mus_scan(void)
{
  sgList *list;
  sgPath *path;
  struct ini *ini;
  
  list = sgGetFilePaths();
  
  ini = ini_open(SOUND_PLAYLIST, INI_READ);
  
  sgForeach(list, path)
    sound_mus_scandir(path->dir, ini);
  
  ini_close(ini);
}

/* -------------------------------------------------------------------------- *
 * Scannt ein Verzeichnis nach Soundtracks ab                                 *
 * -------------------------------------------------------------------------- */
void sound_mus_scandir(const char *path, struct ini *ini)
{
  DIR           *d;
  struct dirent *de;
  size_t         len;
  char           filename[PATH_MAX];
  
  if((d = opendir(path)) == NULL)
    return;
  
  while((de = readdir(d)))
  {
    if((len = strlen(de->d_name)) > 3 && !strcmp(&de->d_name[len - 3], ".xm"))
    {
      strcpy(filename, path);
      strcat(filename, "/");
      strcat(filename, de->d_name);

      sound_mus_add(filename, ini);
    }
  }
}

/* -------------------------------------------------------------------------- *
 * Speichert die Playlist ab                                                  *
 * -------------------------------------------------------------------------- */
int sound_mus_save(void)
{
  struct ini *ini;
  struct sound_mus *mus;
  
  if((ini = ini_open(SOUND_PLAYLIST, INI_WRITE)) == NULL)
    return -1;
  
  dlink_foreach(&sound_playlist, mus)
  {
    ini_section(ini, "%s", client_basename(mus->fname));
    
    ini_puts(ini, "title", mus->title);
    ini_puts(ini, "artist", mus->artist);
    ini_puts(ini, "info", mus->infostr);
    
    if(mus->length)
      ini_putulong(ini, "length", mus->length);
    if(mus->patterns)
      ini_putulong(ini, "patterns", mus->patterns);
  }
  
  ini_save(ini);
  ini_close(ini);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Lädt einen Soundeffekt                                                     *
 * -------------------------------------------------------------------------- */
struct sound_fx *sound_fx_open(const char *file)
{
  struct sound_fx *fx;
  SDL_RWops       *rwops;
  Mix_Chunk       *effect;
  
  if((rwops = client_open_rwops(file, CLIENT_READ)))
  {
    if((effect = Mix_LoadWAV_RW(rwops, 1)))
    {
      sound_log(STATUS, "Loading effect file: %s", file);
      
      fx = malloc(sizeof(struct sound_fx));
      
      client_strlcpy(fx->path, file, sizeof(fx->path));
      client_strlcpy(fx->name, client_basename(file), sizeof(fx->name));
      
      fx->effect = effect;
      
      dlink_add_tail(&sound_effects, &fx->node, fx);
      
      return fx;
    }
  }
  
  sound_log(STATUS, "Failed loading effect file %s", file);
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Findet einen Soundeffekt nach Namen                                        *
 * -------------------------------------------------------------------------- */
struct sound_fx *sound_fx_find(const char *name)
{
  struct sound_fx *fx;
  
  dlink_foreach(&sound_effects, fx)
  {
    if(!strcmp(fx->name, name))
      return fx;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Spielt einen Soundeffekt ab                                                *
 * -------------------------------------------------------------------------- */
void sound_fx_play(struct sound_fx *fx)
{
  int i = 0;
  
  if(fx)
  {
    sound_debug(INFO, "SOUND: %s", fx->name);
    /*
     for(i = 0; i < SOUND_CHANNELS; i++)
     {
     if(!Mix_Playing(i))
     {
     Mix_PlayChannel(i, fx->effect, 0);
     break;
     }
     }*/
  
    if(Mix_Playing(i))
      Mix_HaltChannel(i);
  
    Mix_PlayChannel(i, fx->effect, 0);
  }
}

/* -------------------------------------------------------------------------- *
 * Gibt einen Soundeffekt wieder frei                                         *
 * -------------------------------------------------------------------------- */
void sound_fx_free(struct sound_fx *fx)
{
  Mix_FreeChunk(fx->effect);
  
  dlink_delete(&sound_effects, &fx->node);
  
  free(fx);
}
  
/* -------------------------------------------------------------------------- *
 * Liest das F/X Volumen aus (0-255)                                          *
 * -------------------------------------------------------------------------- */
int sound_fx_getvol(void)
{
  return sound_config.fx;
}

/* -------------------------------------------------------------------------- *
 * Setzt das F/X Volumen (0-255)                                              *
 * -------------------------------------------------------------------------- */
void sound_fx_setvol(int vol)
{
  sound_config.fx = vol;
  Mix_Volume(0, vol * 128 / 255);
}

