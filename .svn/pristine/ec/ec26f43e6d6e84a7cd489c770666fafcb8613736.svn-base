/* $Id: client.c,v 1.167 2005/05/26 10:48:35 smoli Exp $
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

/* .-Ä signoff: araujo/##c ("Programs must be written for people to read, and 
 *                           only incidentally for machines to execute")
 * -------------------------------------------------------------------------- */
#include <string.h>
#include <errno.h>
#include <SDL.h>

#include <libsgui/png.h>
#include <libsgui/file.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_TICHU_CLIENT_H
#include "tichu_client.h"
#define PACKAGE_NAME PRODUCT_NAME
#define PACKAGE_VERSION PRODUCT_VERSION
#endif /* HAVE_TICHU_CLIENT_H */

/* -------------------------------------------------------------------------- *
 * Das CLIENT Modul ist das Hauptmodul des Tichu-Clients                      *
 * -------------------------------------------------------------------------- */
#include "net.h"
#include "ini.h"
#include "fan.h"
#include "card.h"
#include "game.h"
#include "reply.h"
#include "sound.h"
#include "queue.h"
#include "client.h"

#include "ui_chat.h"
#include "ui_config.h"
#include "ui_main.h"
#include "ui_game.h"
#include "ui_settings.h"

/* Screen-Dimensionen
 * -------------------------------------------------------------------------- */
SDL_Rect       client_rect;

/* Client-Status
 * -------------------------------------------------------------------------- */
int            client_status;

/* Konfigurationsdatei
 * -------------------------------------------------------------------------- */
struct ini    *client_ini;

/* Bitmap-Fonts (libsgui)
 * -------------------------------------------------------------------------- */
sgFont        *client_font[3];

/* Scrollpuffer für die Spielkonsole und initialer Inhalt (ASCII-Logo)
 * -------------------------------------------------------------------------- */
char          *client_buffer;
const char    *client_ascii  = "   o        o                o \n"
                               "  <|>     _<|>_             <|>\n"
                               "  < >                       / >\n"
                               "   |        o        __o__  \\o__ __o     o       o \n"
                               "   o__/_   <|>      />  \\    |     v\\   <|>     <|>\n"
                               "   |       / \\    o/        / \\     <\\  < >     < >\n"
                               "   |       \\o/   <|         \\o/     o/   |       | \n"
                               "   o        |     \\\\         |     <|    o       o \n"
                               "   <\\__    / \\     _\\o__</  / \\    / \\   <\\__ __/> \n";
  

/* Bildschirm-Surface und Hintergrund-Muster
 * -------------------------------------------------------------------------- */
SDL_Surface   *client_screen;
//SDL_Surface   *client_background;

/* Default-Hintergrundfarbe
 * -------------------------------------------------------------------------- */
sgHSV          client_default_bgcolor = { 0x00, 0x00, 0x80, 0xff };

/* Momentantes Ziel von Eingaben ohne Kommando-Präfix (/)
 * -------------------------------------------------------------------------- */
char           client_target[CLIENT_USERLEN + 2];

/* Musik-Playliste
 * -------------------------------------------------------------------------- */
struct list    client_playlist; 

/* Suchpfade
 * -------------------------------------------------------------------------- */
char          *client_path;              /* Pfad der letzten gefundenen Datei */
const char    *client_paths[8] =
{
  "",
  "..",
  "client",
  "sounds",
  "tichu/client",
#ifdef SHAREDIR
  SHAREDIR,
#ifdef PACKAGE_NAME
  SHAREDIR PACKAGE_NAME,
#endif  
#endif
  NULL
};

/* Namen für die einzelnen Module 
 * -------------------------------------------------------------------------- */
static const char *const client_modules[] =
{
  "FAN", "PLR", "STK", "DND", "ACT", "CRD", "CLI", "LNK", 
  "GAM", "GFX", "INI", "NET", "QUE", "RPL", "SND",
#ifdef DEBUG
  "MDG"
#endif /* DEBUG */
};

/* Symbole für Logfile-Einträge 
 * -------------------------------------------------------------------------- */
static const char *const client_symbols[] =
{
  /* MOD_FAN */    "\\|/", /* MOD_PLAYER */  "C}<",
  /* MOD_STACK */  "|||",  /* MOD_DND */     ">>>",  
  /* MOD_ACTION */ "<--",  /* MOD_CARD */    "@@@",
  /* MOD_CLIENT */ "<->",  /* MOD_DLINK */   "o=o",
  /* MOD_GAME */   "+++",  /* MOD_GFXUTIL */ "%%%",
  /* MOD_INI */    "[i]",  /* MOD_NET */     "===",
  /* MOD_QUEUE */  "...",  /* MOD_REPLY */   "-->", 
  /* MOD_SOUND */  "«!»",
#ifdef DEBUG
  "___",    /* MOD_MDEBUG */
#endif
};

/* SDL-Status
 * -------------------------------------------------------------------------- */
/*static SDL_Rect client_rect_640x480 =   { 0, 0, 640,   480 };*/
static SDL_Rect client_rect_800x600 =   { 0, 0, 800,   600 };
static SDL_Rect client_rect_1024x768 =  { 0, 0, 1024,  768 };
static SDL_Rect client_rect_1152x864 =  { 0, 0, 1152,  864 };
static SDL_Rect client_rect_1280x1024 = { 0, 0, 1280,  1024 };
static SDL_Rect client_rect_1600x1200 = { 0, 0, 1600,  1200 };

static Uint32     client_subsystems;
static SDL_Rect **client_videomodes;
static SDL_Rect  *client_resolutions[] =
{
/*  &client_rect_640x480,*/
  &client_rect_800x600,
  &client_rect_1024x768,
  &client_rect_1152x864, 
  &client_rect_1280x1024,
  &client_rect_1600x1200,
  NULL
};

sgCursorTheme *client_cursor;
sgPattern     *client_bgnd;
sgColor        client_bgcolor;

/* -------------------------------------------------------------------------- *
 * Konfiguration für den Client                                               *
 * -------------------------------------------------------------------------- */
#define CLIENT_DEFAULT_WIDTH      800
#define CLIENT_DEFAULT_HEIGHT     600
#define CLIENT_DEFAULT_DEPTH      0
#define CLIENT_DEFAULT_FULLSCREEN 1

#define CLIENT_DEFAULT_BGCOLOR    client_default_bgcolor
#define CLIENT_DEFAULT_PATTERN    "Turn"
#define CLIENT_DEFAULT_CONTRAST   255

#define CLIENT_DEFAULT_FONT0      "font-normal.png"
#define CLIENT_DEFAULT_FONT1      "font-bold.png"
#define CLIENT_DEFAULT_FONT2      "font-fixed.png"

#define CLIENT_DEFAULT_CURSOR     "ghost.cur"

struct client_config client_config;

/* -------------------------------------------------------------------------- *
 * Client Initialisation                                                      *
 * -------------------------------------------------------------------------- */
void client_init(void)
{
  int i;
  SDL_RWops *rwops;
  const sgVersion *sgver;
  const SDL_version *sdlver;
  
  client_log(STATUS, "Starte "PACKAGE" v"VERSION"...");
  
  /* GUI Library initialisieren */
  sgver = sgLinkedVersion();
  
  client_log(STATUS, "Initialisiere sgUI v%u.%u.%u",
             sgver->major, sgver->minor, sgver->patch);
  
  sgInit();
  
  /* Such-Pfade adden... Alle Dateien des Tichu-Clients 
     werden über die libsgui geöffnet 
   
     Auf windows starten wir immer aus dem Programm-Verzeichnis, wir müssen
     also nur unsere Unterverzeichnisse kennen:
   */
  sgAddFilePath("data");
  sgAddFilePath("sounds");
  sgAddFilePath("images");
  
  /* Auf *NIXes wird immer an einen festen Pfad installiert: */
#ifdef DATADIR
    sgAddFilePath(DATADIR "/data");
    sgAddFilePath(DATADIR "/sounds");
    sgAddFilePath(DATADIR "/images");
#endif
  
  /* Vielleicht sollten wir die folgenden Pfade 
     nur für debug-builds  absuchen (#ifdef DEBUG) */
  sgAddFilePath("client/data");
  sgAddFilePath("client/sounds");
  sgAddFilePath("client/images");
  
  sgAddFilePath("tichu/client/data");
  sgAddFilePath("tichu/client/sounds");
  sgAddFilePath("tichu/client/images");
  
  sgAddFilePath("../data");
  sgAddFilePath("../sounds");
  sgAddFilePath("../images");

  client_path = sgGetFilePath();
  
  /* cursors themes laden */
  sgOpenCursorThemes();

  /* Konfigurationsdatei öffnen */
  client_ini = ini_open(CLIENT_INI, INI_READ);
  
#ifdef DEBUG
  ini_dump(client_ini);
#endif /* DEBUG */
  
  if(!client_ini)
    client_log(ERROR, "Fehler beim Öffnen der Konfigurationsdatei %s!", 
               CLIENT_INI);

  /* SDL Initialisation */
  sdlver = SDL_Linked_Version();
  
  client_log(STATUS, "Initialisiere SDL v%u.%u.%u",
             sdlver->major, sdlver->minor, sdlver->patch);
             
  if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_NOPARACHUTE))
    client_log(ERROR, "Kann SDL nicht initialisieren!");
  
  client_subsystems = SDL_WasInit(SDL_INIT_EVERYTHING);
  
  client_getmodes();
  
  client_configure();
  
#if MDEBUG_ENABLE
  mdebug_init();
#endif /* MDEBUG_ENABLE */
  
  /* Netzwerk initialisieren */
  net_init();
  
  /* Module welche auf SDL basieren initialisieren */
  sound_init();
//  card_init("data/cards.ini", "data/cards-alpha.png");

  /* net read/error funktionen */
  net_register(NET_READ, client_event);
  net_register(NET_ERROR, client_event);

#ifdef WIN32
  /* Im Windows-Taskbar sieht ein 32x32 Icon am besten aus */  
  client_load_icon("dragon-32.png");
#else
  /* Unter Fluxbox und XFCE macht sich ein 16x16 Icon am Besten :P
   *
   * Oooops, unter der neuen Fluxbox mit all dem Antialias Gewixe
   * aber nicht, dort wäre auch ein 32x32 angebracht.
   * 
   * Aber wie finden wir raus welchen WM wir runnen auf X11?
   * Naja, vorerst einmal scheissegal :P
   */
  client_load_icon("dragon-16.png");
#endif /* WIN32 */

  /* Window-Manager Titel setzen */
  SDL_WM_SetCaption("Tichu SDL Client v"VERSION, NULL);

  /* Fonts laden und der GUI übergeben */
  for(i = 0; i < 3; i++)
  {
    rwops = client_open_rwops(client_config.font[i], CLIENT_READ);
    client_font[i] = sgLoadFontRWops(rwops);
  }
  
  /* Einen Puffer für die Kommandozeile und Chatkonsole reservieren */
  client_buffer = strdup(client_ascii);
  
  if(sound_status & SOUND_AVAIL)
  {
    sound_configure(client_ini);
    
    /* Soundeffekte laden */
    ui_loadsounds();
    
    /* Soundtracks laden */
    sound_mus_scan();
  
    if(sound_playlist.head)
      sound_mus_play((struct sound_mus *)sound_playlist.head);
  }
  
  /* Kartendaten laden */
  card_configure(client_ini);
  card_init();
  
  /* Fächerkonfiguration laden */
  fan_configure(client_ini);
}

/* -------------------------------------------------------------------------- *
 * Client Shutdown                                                            *
 * -------------------------------------------------------------------------- */
void client_shutdown(void)
{
  if(client_ini)
    ini_close(client_ini);
  
  /* Bilder freigeben */
  client_free(client_buffer);  
/*  client_free_image(client_background);*/
  
  /* Client-Module herunterfahren */
  sound_shutdown();
  card_shutdown();
  net_shutdown();
  
  /* Bibliotheken herunterfahren */
  SDL_Quit();
  sgQuit();
  
#if MDEBUG_ENABLE
  mdebug_shutdown();
#endif /* MDEBUG_ENABLE */
  
  client_log(STATUS, "Baba! *Winke-Wink*");
  exit(0);
}

/* -------------------------------------------------------------------------- *
 * Liest Benutzeridentifikation aus der client.ini                            *
 * Gibt 1 zurück falls der Videomodus geändert wurde                          *
 * -------------------------------------------------------------------------- */
int client_configure(void)
{
  int width, height, depth, fullscreen, contrast;
  const char *pattern, *cursor;
  sgHSV hsv;
  char *user, *pass, *info;
  
  ini_section(client_ini, "client");

  width = ini_getulong_default(client_ini, "width", CLIENT_DEFAULT_WIDTH);
  height = ini_getulong_default(client_ini, "height", CLIENT_DEFAULT_HEIGHT);
  depth = ini_getulong_default(client_ini, "depth", CLIENT_DEFAULT_DEPTH);
  fullscreen = ini_getulong_default(client_ini, "fullscreen", CLIENT_DEFAULT_FULLSCREEN);
  
  hsv = ini_gethsv_default(client_ini, "bgcolor", CLIENT_DEFAULT_BGCOLOR);
  
  hsv.a = 255;
  
  pattern = ini_gets_default(client_ini, "pattern", CLIENT_DEFAULT_PATTERN);
  contrast = ini_getlong_default(client_ini, "contrast", CLIENT_DEFAULT_CONTRAST);
  cursor = ini_gets_default(client_ini, "cursor", CLIENT_DEFAULT_CURSOR);

  client_config.font[0] = 
    ini_gets_default(client_ini, "font[0]", CLIENT_DEFAULT_FONT0);
  client_config.font[1] = 
    ini_gets_default(client_ini, "font[1]", CLIENT_DEFAULT_FONT1);
  client_config.font[2] =
    ini_gets_default(client_ini, "font[2]", CLIENT_DEFAULT_FONT2);

  client_config.version = 
    ini_gets_default(client_ini, "version", VERSION);
  
  user = ini_gets_default(client_ini, "user", CLIENT_DEFAULT_USER);
  pass = ini_gets_default(client_ini, "pass", CLIENT_DEFAULT_PASS);
  info = ini_gets_default(client_ini, "info", CLIENT_DEFAULT_INFO);
  
  client_strlcpy(client_config.user, user, sizeof(client_config.user));
  client_strlcpy(client_config.pass, pass, sizeof(client_config.pass));
  client_strlcpy(client_config.info, info, sizeof(client_config.info));
  
  client_strlcpy(client_config.info, info, sizeof(client_config.info));

  if(*(Uint32 *)&hsv != *(Uint32 *)&client_config.bgcolor ||
     client_config.contrast != contrast ||
     (client_config.cursor != cursor) ||
     (client_config.pattern != pattern))
  {
    client_config.cursor = cursor;
    client_config.bgcolor = hsv;
    client_config.pattern = pattern;
    client_config.contrast = contrast;
    
    client_setlook();
  }
  
  if(width != client_config.width || height != client_config.height ||
     depth != client_config.depth || fullscreen != client_config.fullscreen)
  {
    client_config.width = width;
    client_config.height = height;
    client_config.depth = depth;
    client_config.fullscreen = fullscreen;
    
    client_setmode();
    return 1;
  }  
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Setzt den Videomodus                                                       *
 * -------------------------------------------------------------------------- */
void client_setmode(void)
{
  SDL_Rect real_rect;
  SDL_Rect *best_rect;
  SDL_Surface *bg;
  
  if(client_config.fullscreen)    
  {
    /* Im Vollbildmodus schauen wir welcher Hardware-Modus am passendsten ist */
    if((best_rect = client_bestmode(client_config.width, client_config.height)))
      real_rect = *best_rect;
    else
      real_rect = *client_videomodes[0];
    
    client_rect.w = client_config.width;
    client_rect.h = client_config.height;
    
    sgAlignRect(&real_rect, &client_rect, SG_ALIGN_LEFT|SG_ALIGN_TOP);
  }
  else
  {
    real_rect.x = 0;
    real_rect.y = 0;
    real_rect.w = client_config.width;
    real_rect.h = client_config.height;
    
    client_rect = real_rect;
  }
  
  /* Videomodus setzen */
  client_screen = 
    SDL_SetVideoMode(real_rect.w, real_rect.h, client_config.depth,
                     SDL_HWSURFACE|(client_config.fullscreen ? SDL_FULLSCREEN : 0));

  SDL_SetClipRect(client_screen, &client_rect);
  
  if(client_screen == NULL)
  {
    client_log(ERROR, "Kann Videomodus nicht setzen!");
    client_shutdown();
  }
  
  /* Hintergrund gleich sofort anzeigen */
  if((bg = client_render()))
  {
    SDL_BlitSurface(bg, NULL, client_screen, NULL);
    SDL_FreeSurface(bg);
    client_flip();
  }
  
  client_log(STATUS, "Video-Modus %ux%u @ %ubpp%s",
             client_screen->w, client_screen->h,
             client_screen->format->BitsPerPixel,
             (client_screen->flags & (SDL_HWSURFACE|SDL_HWACCEL)) ?
             " (Hardwarebeschleunigt)" : "");

  client_debug(INFO, "Hardware R-Maske %08x", client_screen->format->Rmask);
  client_debug(INFO, "Hardware G-Maske %08x", client_screen->format->Gmask);
  client_debug(INFO, "Hardware B-Maske %08x", client_screen->format->Bmask);
  client_debug(INFO, "Client R-Maske %08x", RMASK);
  client_debug(INFO, "Client G-Maske %08x", GMASK);
  client_debug(INFO, "Client B-Maske %08x", BMASK);  
  
  /* Bildschirm-Rechteck des ganzen Clients */
/*  client_rect.x = client_rect.y = 0;

  client_rect.w = client_screen->w;
  client_rect.h = client_screen->h;*/
}

/* -------------------------------------------------------------------------- *
 * Setzt Cursortheme und Hintergrundmuster                                    *
 * -------------------------------------------------------------------------- */
void client_setlook(void)
{
  sgCursorTheme *theme;
  sgPattern *pattern;
  
  client_bgcolor = sgHSVToRGB(client_config.bgcolor);
  
  if((pattern = sgFindPattern(client_config.pattern)))
    client_bgnd = pattern;
  else
    client_bgnd = sgGetPatternTable();
  
  if((theme = sgFindCursorTheme(client_config.cursor)))
    client_cursor = theme;
  else
    client_cursor = (sgCursorTheme *)sgGetCursorThemes()->head;
  
  if(client_cursor == NULL)
    client_log(ERROR, "Cannot find cursor theme '%s'", client_config.cursor);  
}

/* -------------------------------------------------------------------------- *
 * Liest die möglichen Video-Modes aus                                        *
 * -------------------------------------------------------------------------- */
void client_getmodes(void)
{
  static SDL_Rect *unknown_modes[] = { &client_rect_800x600, NULL };
    
  client_videomodes = 
    SDL_ListModes(NULL, SDL_ANYFORMAT|SDL_HWSURFACE|SDL_FULLSCREEN);
  
  if(client_videomodes == (SDL_Rect **)-1)
    client_videomodes = unknown_modes;
}

/* -------------------------------------------------------------------------- *
 * Checkt ob eine gewisse Auflösung unter den Hardwarebedingungen möglich ist *
 * -------------------------------------------------------------------------- */
int client_supportedmode(SDL_Rect *rect)
{
  unsigned int i;
  
  for(i = 0; client_videomodes[i]; i++)
  {
    if(client_videomodes[i]->w >= rect->w && 
       client_videomodes[i]->h >= rect->h)
      return 1;
  }

  return 0;
}

/* -------------------------------------------------------------------------- *
 * Sucht nach der bestmöglichen Videomode für die gewünschte Auflösung        *
 * -------------------------------------------------------------------------- */
SDL_Rect *client_bestmode(Uint16 w, Uint16 h)
{
  unsigned long dist;
  unsigned long closest = -1L;
  unsigned int i;
  SDL_Rect *mode = NULL;
  
  for(i = 0; client_videomodes[i]; i++)
  {
    if(client_videomodes[i]->w >= w &&
       client_videomodes[i]->h >= h)
    {
      dist = (client_videomodes[i]->w - w) *
             (client_videomodes[i]->h - h);

      if(dist < closest)
      {
        closest = dist;
        mode = client_videomodes[i];
      }
    }
  }

  return mode;
}

/* -------------------------------------------------------------------------- *
 * Addet die unterstützten Video-Modes in eine Dropdown-Box                   *
 * -------------------------------------------------------------------------- */
void client_addmodes(sgWidget *dropdown)
{
  unsigned int i;
  char buf[32];

  for(i = 0; client_resolutions[i]; i++)
  {
    if(client_supportedmode(client_resolutions[i]))
    {
      snprintf(buf, sizeof(buf), "%ux%u",
               client_resolutions[i]->w, 
               client_resolutions[i]->h);

      sgAddDropdownItem(dropdown, buf, (void *)client_resolutions[i]);
      
      if(client_config.width == client_resolutions[i]->w &&
         client_config.height == client_resolutions[i]->h)
        sgSelectDropdownItem(dropdown, i);         
    }
  }  
}

/* -------------------------------------------------------------------------- *
 * Fehlermeldung falls das Öffnen einer Datei fehlschlägt                     *
 * -------------------------------------------------------------------------- */
static void client_open_error(const char *filename)
{
  client_log(ERROR, "Kann %s nicht öffnen: %s", filename, strerror(errno));
}

/* -------------------------------------------------------------------------- *
 * Öffnet eine Datei und sucht dabei alle Pfade in denen der Tichu-Client     *
 * seine Daten finden könnte ab.                                              *
 * -------------------------------------------------------------------------- */
FILE *client_open_fp(const char *filename, int mode)
{
  int i;
  char modestr[4];
  FILE *fp;

  /* mode-string zusammensetzen */
  i = 0;
  modestr[i++] = (mode & CLIENT_READ) ? ((mode & CLIENT_WRITE) ? 'a' : 'r') : 'w';
  modestr[i++] = 'b';
  modestr[i++] = '\0';
  
  /* dann per libsgui öffnen */
  fp = sgOpenFileFp(filename, modestr);
  
  if(fp == NULL)
    client_open_error(filename);
  
  return fp;
}

/* -------------------------------------------------------------------------- *
 * Öffnet eine Datei als SDL_RWops                                            *
 * -------------------------------------------------------------------------- */
SDL_RWops *client_open_rwops(const char *filename, int mode)
{
  int i;
  char modestr[4];
  SDL_RWops *rwops;

  /* mode-string zusammensetzen */
  i = 0;
  modestr[i++] = (mode & CLIENT_WRITE) ? 'w' : 'r';
  modestr[i++] = 'b';
  modestr[i++] = '\0';
  
  /* dann per libsgui öffnen */
  rwops = sgOpenFileRWops(filename, modestr);
  
  if(rwops == NULL)
    client_open_error(filename);
  
  return rwops;
}

/* -------------------------------------------------------------------------- *
 * Gibt den Dateinamen-Teil eines Pfades zurück                               *
 * -------------------------------------------------------------------------- */
char *client_basename(const char *path)
{
  char *s;
  
again:
  if((s = strrchr(path, '/')))
  {
    if(s[1] == '\0')
    {
      if(s == path)
        return s;
      else
      {
        s[0] = '\0';
        goto again;
      }
    }

    return &s[1];
  }
  
  return (char *)path;
}

/* -------------------------------------------------------------------------- *
 * Lädt ein 32-bit PNG Bild in eine SDL Surface                               *
 * -------------------------------------------------------------------------- */
void client_load_png(SDL_Surface **surface, const char *filename)
{  
  if((*surface = sgLoadPngFile(filename)) == NULL)
    client_log(ERROR, "Konnte %s nicht laden!", filename);
}

/* -------------------------------------------------------------------------- *
 * Lädt ein Icon und setzt dieses als Windowmanager Icon                      *
 * -------------------------------------------------------------------------- */
void client_load_icon(const char *filename)
{
  int x, y;
  Uint8 *row;
  Uint8 *mask_row;
  int bpr;
  SDL_Surface *icon;
  Uint8 *mask;
 
  icon = sgLoadPngFile(filename);
  
  /* Laden fehlgeschlagen? */
  if(icon == NULL)
  {
    client_log(ERROR, "Kann Icon %s nicht laden!", filename);
    return;
  }
  
  /* Dimensionen checken */
  if(icon->w != icon->h || icon->format->palette == NULL)
  {
    client_log(ERROR, "Icon muss quadratisch sein und eine Palette haben!");
    return;
  }

#ifdef WIN32
  if(icon->w != 32 || icon->h != 32)
  {
    client_log(ERROR, "Windows-Icons müssen 32x32 pixels gross sein!");
    return;
  }
#endif /* WIN32 */

  /* Maske erstellen */
  mask = malloc(icon->w * icon->h / 8);
  
  if(mask == NULL)
  {
    client_log(ERROR, "Nicht genug Memory für Icon Maske!");
    return;
  }
  
  row = icon->pixels;

  bpr = icon->w / 8;
  mask_row = mask;
  
  for(y = 0; y < icon->h; y++)
  {
    for(x = 0; x < bpr; x++)
    {
      mask_row[x] =
        (row[x * 8 + 0] == icon->format->colorkey ? 0 : 0x80) |
        (row[x * 8 + 1] == icon->format->colorkey ? 0 : 0x40) |
        (row[x * 8 + 2] == icon->format->colorkey ? 0 : 0x20) |
        (row[x * 8 + 3] == icon->format->colorkey ? 0 : 0x10) |
        (row[x * 8 + 4] == icon->format->colorkey ? 0 : 0x08) |
        (row[x * 8 + 5] == icon->format->colorkey ? 0 : 0x04) |
        (row[x * 8 + 6] == icon->format->colorkey ? 0 : 0x02) |
        (row[x * 8 + 7] == icon->format->colorkey ? 0 : 0x01);
    }
    
    mask_row += bpr;
    row += icon->pitch;
  }
  
  SDL_WM_SetIcon(icon, (Uint8 *)mask);

  free(mask);
  client_free_image(icon);
}
  
/* -------------------------------------------------------------------------- *
 * Lädt ein PNG Bild und teilt diese in <count> Bilder auf, welche in einem   *
 * Array mit Surface Pointern abgelegt werden                                 *
 * -------------------------------------------------------------------------- */
void client_load_icons(const char *filename, SDL_Surface **sf, int count)
{  
  SDL_Surface *surface, *temp;
  SDL_Rect srect;  
  SDL_Rect drect;  
  int i;
  
  surface = sgLoadPngFile(filename);
  
  if(surface)
  {
    SDL_SetAlpha(surface, 0, 255);
    
    srect.x = 0;
    srect.y = 0;
    srect.w = 16;
    srect.h = 16;
    
    drect = srect;
    for(i = 0; i < count; i++)
    {
      temp = SDL_CreateRGBSurface(SDL_SWSURFACE, 16, 16, 32, 
                                  RMASK, GMASK, BMASK, AMASK);

      SDL_BlitSurface(surface, &srect, temp, &drect);
      SDL_SetAlpha(temp, 0, 255);      
      sf[i] = temp;      
      SDL_Flip(sf[i]);
      srect.x += 16;

      client_debug(INFO, "Lade icon %i/%i aus %s...",
                   i, count, filename);
    }
    
    SDL_FreeSurface(surface);
    return;
  }
  
  client_log(ERROR, "Konnte Icon %s nicht laden!", filename);
}

/* -------------------------------------------------------------------------- *
 * Erstellt einen Hintergrund, bestehend aus dem Hintergrundmuster und einem  *
 * optinalen Bild                                                             *
 * -------------------------------------------------------------------------- */
/*void client_load_bg(SDL_Surface **dest, const char *bg, int align)
{
  SDL_Rect rect;
  
  *dest = SDL_DisplayFormat(client_screen);
    
  client_fill(*dest);
  
  if(bg)
  {
    SDL_Surface *temp;
    
    client_load_png(&temp, bg);
    
    if(temp == NULL)
      client_log(ERROR, "Konnte %s nicht laden!", bg);
    
    rect = temp->clip_rect;
    
    sgAlignRect(&client_rect, &rect, align);
    SDL_BlitSurface(temp, NULL, *dest, &rect);    
    client_free_image(temp);
  }
}*/

/* -------------------------------------------------------------------------- *
 * Eine Zeile in die Konsole schreiben                                        *
 * -------------------------------------------------------------------------- */
void client_puts(const char *format, ...)
{
  char    msg[4096];
  va_list args;
  struct  sgWidget *console = NULL;
  
  va_start(args, format);
  
  /* Hängt davon ab in welchem User-Interface wir uns gerade befinden */
  if(client_status == CLIENT_CHAT)
    console = ui_chat_console;
  else if(client_status == CLIENT_SETTINGS)
    console = ui_settings_console;
  else if(client_status == CLIENT_GAME)
    console = ui_game_chat_console;

  /* Die Linie dem entsprechenden Konsolen-Widget hinzufügen */
  if(console)
  {    
    vsnprintf(msg, sizeof(msg), format, args);
    sgAddConsoleLineWrapped(console, msg);
  }
    
  va_end(args);
}

/* -------------------------------------------------------------------------- *
 * Schreibt einen Log-Eintrag auf die Konsole oder auf stderr                 *
 * -------------------------------------------------------------------------- */
void client_write(int mod, int lev, const char *s, ...)
{
  char    msg[4096];
  va_list args;
  size_t  n = 0;
  
  if(lev > LOGLEVEL)
    return;
  
  va_start(args, s);
  
  strncpy(&msg[n], client_symbols[mod], sizeof(msg) - n);
  n += 3;
  msg[n++] = ' ';
  
#if defined(DEBUG) && defined(DEBUG_TICKS)
  {
    uint32_t ticks = (client_subsystems ? SDL_GetTicks() : 0);
    sprintf(&msg[n], "(%04u.%03u)", (ticks / 1000) % 10000, ticks % 1000);
    n += 10;
    msg[n++] = ' ';
  }
#endif
  
  strncpy(&msg[n], client_modules[mod], sizeof(msg) - n);
  n += strlen(&msg[n]);
  msg[n++] = ':';
  msg[n++] = ' ';
  
  vsnprintf(&msg[n], sizeof(msg) - n, s, args);
  
  /* Auf die aktive Konsole damit */
  if(lev <= CONSOLE)
    client_puts("%s", &msg[n]);
  
  if(lev != CONSOLE)
    fprintf(stderr, "%s\n", msg);
  
  va_end(args);
  
  if(lev == ERROR)
    client_shutdown();
}

/* -------------------------------------------------------------------------- *
 * Nimmt Eingaben eines Input Widgets entgegen und sendet sie an den Server.  *
 * Es kann sich dabei um eine Nachricht an das aktuelle Spiel, an einen Mit-  *
 * spieler oder um einen Befehl handeln. Letzterer beginnt mit /              *
 * -------------------------------------------------------------------------- */
void client_message(char *msg)
{
  if(msg[0] == '/')
  {
    net_send("%s", &msg[1]);
  }
  else
  {
    net_send("msg %s :%s", client_target, msg);
  }
}

/* -------------------------------------------------------------------------- *
 * Behandelt Netzwerk-Ereignisse                                              *
 * -------------------------------------------------------------------------- */
void client_event(int type, char *msg)
{
  switch(type)
  {
    /* Es stehen Daten bereit */
    case NET_READ:
    {
      reply_parse(msg);
      break;
    }
    
    /* Es trat ein Fehler auf */
    case NET_ERROR:
    {
      client_log(STATUS, "Verbindung getrennt: %s", msg);
    
      if(client_status == CLIENT_CHAT)
      {
        sgSetWidgetCaption(ui_chat_connect, "Verbinden");
        sgDisableWidget(ui_chat_create);
      }

      break;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * Rendert eine Surface mit dem Hintergrund-Muster                            *
 * -------------------------------------------------------------------------- */
SDL_Surface *client_render(void)
{
  SDL_Surface *pattern;
  sgColor *palette;
  
  pattern = SDL_CreateRGBSurface(SDL_SWSURFACE, client_rect.w, client_rect.h, 
                                 8, 0, 0, 0, 0);

  palette = sgCreatePalette(client_bgcolor);
  
  SDL_SetPalette(pattern, SDL_LOGPAL, (SDL_Color *)palette, 0, 256);
 
  sgFillPattern(pattern, client_bgnd, client_config.contrast);
  
  free(palette);
  
  return pattern;
}

/* -------------------------------------------------------------------------- *
 * Bildschirm updaten                                                         *
 * -------------------------------------------------------------------------- */
void client_flip(void)
{
  if(client_screen->flags & SDL_DOUBLEBUF)
    SDL_Flip(client_screen);
  else
    SDL_UpdateRect(client_screen, client_rect.x, client_rect.y,
                                  client_rect.w, client_rect.h);
}

/* -------------------------------------------------------------------------- *
 * Hauptschleife des Clients                                                  *
 * -------------------------------------------------------------------------- */
void client_run(void)
{
  /* Mit dem Menu beginnen */
  client_status = CLIENT_MAIN;
  
  while(client_status != CLIENT_EXIT)
  {
    switch(client_status)
    {
      /* Das Hauptmenu */
      case CLIENT_MAIN:
      {
        ui_main(client_screen);
        break;
      }
      
      /* Konfiguration des Clients */
      case CLIENT_CONFIG:
      {
        ui_config(client_screen);
        break;
      }
      
      /* Öffentlicher Chat und Spielübersicht */
      case CLIENT_CHAT:
      {
        ui_chat(client_screen);
        break;
      }
      
      /* "Chat Channel" und Spieleinstellungen */
      case CLIENT_SETTINGS:
      {
        ui_settings(client_screen);
        break;
      }
      case CLIENT_TEST:
      {
        ui_test(client_screen);
        break;
      }
      case CLIENT_GAME:
      {
        ui_game(client_screen);
        break;
      }
      case CLIENT_GAME_END:
      {
        ui_game_end(client_screen);
        client_status = CLIENT_CHAT;
        break;
      }
    }
  }  
}

/* -------------------------------------------------------------------------- *
 * Hauptfunktion des Clients                                                  *
 * -------------------------------------------------------------------------- */
int client_main(int argc, char *argv[])
{
  client_init();
  client_run();
  client_shutdown();
  
  return 0;
}
