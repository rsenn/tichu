/* $Id: ini.c,v 1.29 2005/05/22 01:25:47 smoli Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <libsgui/file.h>

/* -------------------------------------------------------------------------- *
 * Das INI Modul liest und schreibt Dateien im .ini Format, welches für das   *
 * CFG Modul benötigt wird                                                    *
 * -------------------------------------------------------------------------- */
#include "ini.h"
#include "client.h"

/* Bit-Rotationsmakros 
 * -------------------------------------------------------------------------- */
#define ini_ror(v, n) ((v >> (n & 0x1f)) | (v << (32 - (n & 0x1f))))
#define ini_rol(v, n) ((v >> (n & 0x1f)) | (v << (32 - (n & 0x1f))))

/* -------------------------------------------------------------------------- *
 * Entfernt Lücken (space, tab, cr, lf) am Schluss und am Ende des Strings    *
 * -------------------------------------------------------------------------- */
static inline void ini_strip(char *s) 
{
  uint32_t i;
  uint32_t len;
  
  len = strlen(s);
  
  for(i = 0; isspace(s[i]); i++);
  
  len -= i;
  
  if(i > 0) 
    memmove(s, &s[i], len + 1);
  
  if(len <= 0) return;
  
  for(i = len; i > 0; i--) 
  {
    if(!isspace(s[i - 1])) 
    {
      s[i] = '\0';
      break;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * Kalkuliert einen 32-bit Hash aus einem String                              *
 * -------------------------------------------------------------------------- */
uint32_t ini_hash(const char *s)
{
  uint32_t ret = 0xcafebabe;
  uint32_t temp;
  uint32_t i;
  
  for(i = 0; s[i]; i++)
  {
    temp = ret;
    ret = ini_ror(ret, s[i]);
    temp ^= s[i];
    temp = ini_rol(temp, ret);
    ret -= temp;
  }
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * Konvertiert eine Hexadezimale Ziffer in ihren Wert                         *
 * -------------------------------------------------------------------------- */
static inline uint8_t ini_hexval(char c) 
{  
  uint8_t ret;
  
  c = tolower(c);
  
  if(c >= 'a' && c <= 'f')
    ret = c - 'a' + 10;
  else if(c >= '0' && c <= '9')
    ret = c - '0';
  else
    ret = 0;

  return ret;
}

/* -------------------------------------------------------------------------- *
 * Erstellt einen neuen Schlüssel                                             *
 * -------------------------------------------------------------------------- */
static struct key *key_new(struct section *section, const char *name)
{  
  struct key *key;
  
  key = malloc(sizeof(struct key));
  
  if(key)
  {
    /* Mit Namen */
    if(name)
    {
      key->name = strdup(name);
      key->hash = ini_hash(name);
      
      /* Wow, zur Abwechslung wiedermal ein effektiver Key :P */
      section->nkeys++;
    }
    /* Ohne Namen */
    else
    {
      key->name = NULL;
      key->hash = 0;
    }
    
    key->value = NULL;

    /* In die Liste linken */
    dlink_add_tail(&section->keys, &key->node, key);
  }
  
  return key;
}

/* -------------------------------------------------------------------------- *
 * Schlüssel löschen                                                          *
 * -------------------------------------------------------------------------- */
static void key_delete(struct section *section, struct key *key)
{  
  /* Aus der Liste entfernen */
  dlink_delete(&section->keys, &key->node);
  
  /* Strings freigeben */
  if(key->name) free(key->name);
  if(key->value) free(key->value);
  
  /* Struktur freigeben */
  free(key);
}

/* -------------------------------------------------------------------------- *
 * Sucht einen Schlüssel in der INI-Sektion                                   *
 * -------------------------------------------------------------------------- */
static struct key *key_search(struct section *section, const char *name)
{  
  struct key *key;
  uint32_t hash;
  
  /* Hash-Wert ermitteln */
  hash = ini_hash(name);
  
  dlink_foreach(&section->keys, key)
  {
    /* Falls ein Name da ist, vergleichen wir ihn mit dem Hashwert */
    if(key->name && key->hash == hash)
    {
      if(!strcmp(key->name, name))
        return key;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Schreibt einen INI-Key in die Datei                                        *
 * -------------------------------------------------------------------------- */
static int key_write(struct ini *ini, struct key *key) 
{
  /* Der Schlüssel hat einen Namen */
  if(key->name)
  {
    /* Name=Wert-Paar ausgeben */
    return fprintf(ini->file, (key->value ? "%s=%s"INI_NEWLINE : 
                                            "%s="INI_NEWLINE), 
                   key->name, key->value);
  } 
  
  
  /* Mit Wert ists ein String, ohne eine leere Zeile */
  return fprintf(ini->file, (key->value ? "%s"INI_NEWLINE : INI_NEWLINE), 
                 key->value);
}


/* -------------------------------------------------------------------------- *
 * Erstellt eine neue Sektion                                                 *
 * -------------------------------------------------------------------------- */
static struct section *section_new(struct ini *ini, const char *name)
{  
  struct section *section;
  
  section = malloc(sizeof(struct section));
  
  if(section)
  {
    /* Mit Namen */
    if(name)
    {
      section->name = strdup(name);
      section->hash = ini_hash(name);
      
      /* Wow, zur Abwechslung eine neue effektive Sektion :) */
      ini->nsections++;
    }
    /* Ohne Namen */
    else
    {
      section->name = NULL;
      section->hash = 0;
    }
   
    /* Schlüsselliste initialisieren */
    dlink_list_zero(&section->keys);
    section->nkeys = 0;
    
    /* In die Liste linken */
    dlink_add_tail(&ini->sections, &section->node, section);
  }
  
  return section;
}

/* -------------------------------------------------------------------------- *
 * Sektion löschen                                                            *
 * -------------------------------------------------------------------------- */
static void section_delete(struct ini *ini, struct section *section)
{  
  struct key *key;
  struct node *next;
  
  /* Aus der Liste entfernen */
  dlink_delete(&ini->sections, &section->node);
  
  /* Alle Schlüssel löschen */
  dlink_foreach_safe(&section->keys, key, next)
    key_delete(section, key);
  
  /* String freigeben */
  if(section->name)
  {
    free(section->name);
    ini->nsections--;
  }
  
  /* Struktur freigeben */
  free(section);
}

/* -------------------------------------------------------------------------- *
 * Sucht eine INI-Sektion                                                     *
 * -------------------------------------------------------------------------- */
struct section *section_search(struct ini *ini, const char *name)
{
  struct section *section;
  uint32_t hash;
  
  /* Hash-Wert ermitteln */
  hash = ini_hash(name);
  
  dlink_foreach(&ini->sections, section)
  {
    /* Falls ein Name da ist, vergleichen wir ihn mit dem Hashwert */
    if(section->name && section->hash == hash)
    {
      if(!strcmp(section->name, name))
        return section;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Neue INI Instanz                                                           *
 * -------------------------------------------------------------------------- */
static struct ini *ini_new(void)
{
  struct ini *ini;
  
  /* Speicher reservieren */
  ini = malloc(sizeof(struct ini));
  
  /* Struktur initialisieren */
  if(ini)
  {
    ini->file = NULL;
    ini->mode = 0;
    ini->section = NULL;
    ini->nsections = 0;
    
    dlink_list_zero(&ini->sections);
  }
  
  return ini;
}

/* -------------------------------------------------------------------------- *
 * INI Instanz freigeben                                                      *
 * -------------------------------------------------------------------------- */
static void ini_free(struct ini *ini)
{
  /* Alle Sektionen freigeben */
  ini_clear(ini);

  if(ini->path)
    free(ini->path);
  
  /* Struktur freigeben */
  free(ini);
}

/* -------------------------------------------------------------------------- *
 * .ini Datei öffnen und falls erfolgreich, eine neue Instanz erstellen       *
 * -------------------------------------------------------------------------- */
struct ini *ini_open(const char *filename, int mode) 
{ 
  struct ini *ini;
  FILE *file;
  
  /* Versuche Datei zu öffnen */
  file = client_open_fp(filename, mode);

  if(file == NULL)
    return NULL;

  /* Neue INI-Instanz */
  ini = ini_new();
  
  ini->file = file;
  ini->mode = mode;
  ini->path = strdup(sgGetFilePath());

  /* Log führen über das Öffnen von .inis */
  ini_debug(INFO, "%s wurde geöffnet (%s%s)", ini->path,
            (mode & INI_READ ? "r" : ""),
            (mode & INI_WRITE ? "w" : ""));
  
  /* Lesen?? :) */
  if(mode & INI_READ)
    ini_load(ini);
  
  return ini;
}

/* -------------------------------------------------------------------------- *
 * .ini Datei neu öffnen                                                      *
 * -------------------------------------------------------------------------- */
int ini_reopen(struct ini *ini, int mode)
{
  fclose(ini->file);
  
  /* Versuche Datei zu öffnen */
  ini->file = fopen(ini->path, (mode == INI_WRITE ? "wb" : "rb"));
  
  if(ini->file == NULL)
    return -1;

  /* Effektiven Modus setzen */
  ini->mode = mode;
  
  /* Log führen über das Öffnen von .inis */
  ini_debug(INFO, "%s wurde geöffnet (%s%s)",
            client_basename(ini->path),
            (mode & INI_READ ? "r" : ""),
            (mode & INI_WRITE ? "w" : ""));
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * INI-Datei schliessen und alles freigeben                                   *
 * -------------------------------------------------------------------------- */
void ini_close(struct ini *ini) 
{
  /* Wenn die Daten modifiert wurden, dann erst speichern */
  ini_save(ini);
  
  if(ini->file) 
    fclose(ini->file);
  
  ini->file = NULL;
  ini->mode = 0;
  
  ini_free(ini);
}

/* -------------------------------------------------------------------------- *
 * Alle Sektionen einer INI-Datei laden                                       *
 * -------------------------------------------------------------------------- */
int ini_load(struct ini *ini) 
{  
  char *p;
  uint32_t len;
  struct section *section;
  struct key *key;
  uint32_t line = 0;
  uint32_t nkeys = 0;
  char buffer[4096];

  /* Neu öffnen wenn nicht im Lese-Modus */
  if((ini->mode & INI_READ) == 0)
    ini_reopen(ini, INI_READ);
  
  /* Initiale Sektion */
  ini_clear(ini);
  
  ini->section = section = section_new(ini, NULL);
  
  /* Lese Zeile für Zeile */
  rewind(ini->file);
  
  while(fgets(buffer, sizeof(buffer), ini->file))
  {
    /* Zeilenzähler :) */
    line++;
    
    /* Zeile säubern */
    ini_strip(buffer); 

    len = strlen(buffer);

    /* Leere Zeilen und Kommentare behandeln */
    if(buffer[0] == ';' || len == 0) 
    {
      /* Key ohne Namen */
      key = key_new(section, NULL);
      
      key->value = strdup(buffer);      
      continue;
    }
    
    /* Neue Sektion? */
    if(buffer[0] == '[' && buffer[len - 1] == ']') 
    {
      /* Sektionsnamen ermitteln */
      buffer[0] = ' ';
      buffer[len - 1] = ' ';
      ini_strip(buffer);
      
      /* Sektion erstellen */
      ini->section = section = section_new(ini, buffer);
      continue;
    }    
    
    /* In Name/Wert aufsplitten */
    p = strchr(buffer, '=');
      
    /* Muss ein gültiger Schlüssel sein! */
    if(p == NULL) 
    { 
      ini_log(ERROR, "%s:%u: '=' erwartet!", ini->path, line);
      return -1;
    }
    
    /* Schlüssel hinzufügen... */
    *p++ = '\0';    
    ini_strip(p);
    ini_strip(buffer);
    ini_puts(ini, buffer, p);
    nkeys++;
  }

  /* Log führen über das Laden von .inis */
  ini_log(STATUS, "%s wurde geladen (%u Keys in %u Sektionen)",
          client_basename(ini->path), nkeys, ini->nsections);

  ini->mode &= ~INI_MODIFIED;
  
  /* Auf erste Sektion zurücksetzen */
  ini->section = ini->sections.head->data;
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Alle Sektionen einer INI-Datei speichern                                   *
 * -------------------------------------------------------------------------- */
int ini_save(struct ini *ini) 
{  
  struct section *section;
  struct key *key;
  unsigned int nkeys = 0;
  
  /* Nicht modifiziert -> nicht speichern */
  if((ini->mode & INI_MODIFIED) == 0)
    return 0;

  /* Im Schreibmodus öffnen */
  ini_reopen(ini, INI_WRITE);  
  
  rewind(ini->file);
  
  dlink_foreach(&ini->sections, section)
  {
    if(section->name)
      fprintf(ini->file, "[%s]"INI_NEWLINE, section->name);
    
    dlink_foreach(&section->keys, key)
      key_write(ini, key);
    
    fflush(ini->file);
    
    nkeys += section->nkeys;
  }
  
  /* Log führen über das Speichern von .inis */
  ini_log(STATUS, "%s wurde gespeichert (%u Keys in %u Sektionen)",
          client_basename(ini->path), nkeys, ini->nsections);
  
  ini->mode &= ~INI_MODIFIED;
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * INI Daten löschen                                                          *
 * -------------------------------------------------------------------------- */
void ini_clear(struct ini *ini)
{
  struct section *section;
  struct node *next;
  
  dlink_foreach_safe(&ini->sections, section, next)
    section_delete(ini, section);
}

/* -------------------------------------------------------------------------- *
 * Sektion suchen, erstellen falls nicht gefunden und im Schreib-Modus, und   *
 * dann als aktuelle Sektion setzen                                           *
 * Gibt 1 zurück, falls eine Sektion erstellt werden musste                   *
 * -------------------------------------------------------------------------- */
int ini_section(struct ini *ini, const char *name, ...)
{
  char buf[1024];
  va_list args;
  
  va_start(args, name);
  vsnprintf(buf, sizeof(buf), name, args);
  va_end(args);
  
  ini->section = section_search(ini, buf);
  
  if(ini->section)
    return 0;
  
  /* Sektion nicht gefunden und im Readonly-Modus */
/*  if(!(ini->mode & INI_WRITE))
    return -1;*/
  
  ini->section = section_new(ini, buf);
  
  ini_log(STATUS, "%s: Sektion %s erstellt", client_basename(ini->path), buf);
  
  return 1;
}
  
/* -------------------------------------------------------------------------- *
 * Einen String als Schlüssel in die aktuelle INI-Sektion schreiben           *
 * -------------------------------------------------------------------------- */
int ini_puts(struct ini *ini, const char *key, const char *s) 
{  
  struct key *k;

  /* Als modifiziert markieren */
  ini->mode |= INI_MODIFIED;
  
  /* Schlüssel finden oder erstellen */
  if(!(k = key_search(ini->section, key)))
    k = key_new(ini->section, key);
  
  if(k) 
  {
    if(k->value)
      free(k->value);
    
    if(s)
      k->value = strdup(s);
    else
      k->value = NULL;
    
    return 0;
  }

  return -1;
}

/* -------------------------------------------------------------------------- *
 * Einen Long-Integer als Schlüssel in die aktuelle INI-Sektion schreiben     *
 * -------------------------------------------------------------------------- */
int ini_putlong(struct ini *ini, const char *key, long l)
{
  char buffer[16];
  
  snprintf(buffer, 16, "%li", l);
  

  return ini_puts(ini, key, buffer);
}

/* -------------------------------------------------------------------------- *
 * Einen Unsigned Long-Integer als Schlüssel in die aktuelle INI-Sektion      *
 * schreiben                                                                  *
 * -------------------------------------------------------------------------- */
int ini_putulong(struct ini *ini, const char *key, unsigned long u)
{  
  char buffer[64];
  
  sprintf(buffer, "%lu", u);
  
  return ini_puts(ini, key, buffer);  
}

/* -------------------------------------------------------------------------- *
 * Einen double als Schlüssel in die aktuelle INI-Sektion schreiben           *
 * -------------------------------------------------------------------------- */
int ini_putdouble(struct ini *ini, const char *key, double f)
{  
  char buffer[64];
  
  sprintf(buffer, "%.5f", f);
  
  return ini_puts(ini, key, buffer);  
}

/* -------------------------------------------------------------------------- *
 * Eine Farbe im HTML-Format als Schlüssel in die aktuelle INI-Sektion        *
 * schreiben                                                                  *
 * -------------------------------------------------------------------------- */
int ini_putcolor(struct ini *ini, const char *key, struct color color)
{
  return ini_puts(ini, key, gfxutil_strcolor(color));
}

/* -------------------------------------------------------------------------- *
 * Eine Farbe im HSV-Format als Schlüssel in die aktuelle INI-Sektion         *
 * schreiben                                                                  *
 * -------------------------------------------------------------------------- */
int ini_puthsv(struct ini *ini, const char *key, sgHSV hsv)
{
  return ini_puts(ini, key, gfxutil_strhsv(hsv));
}

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und String zurückgeben                            *
 * -------------------------------------------------------------------------- */
char *ini_gets(struct ini *ini, const char *key)
{
  struct key *k = key_search(ini->section, key);
  
  if(k) 
    return k->value;

  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Long Integer zurückgeben                      *
 * -------------------------------------------------------------------------- */
long ini_getlong(struct ini *ini, const char *key)
{  
  char *s = ini_gets(ini, key);
  
  if(s)
    return strtol(s, NULL, 10);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Unsigned Long Integer zurückgeben             *
 * -------------------------------------------------------------------------- */
unsigned long ini_getulong(struct ini *ini, const char *key)
{  
  char *s = ini_gets(ini, key);
  
  if(s)
    return strtoul(s, NULL, 10);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und double zurückgeben                            *
 * -------------------------------------------------------------------------- */
double ini_getdouble(struct ini *ini, const char *key)
{  
  double v = 0.0;
  char *s = ini_gets(ini, key);
  
  if(s)
    sscanf(s, "%lf", &v);
  
  return v;
}

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Farbe zurückgeben                             *
 * -------------------------------------------------------------------------- */
struct color ini_getcolor(struct ini *ini, const char *key)
{  
  struct color c = gfxutil_black;
  char *s;
  
  s = ini_gets(ini, key);
  
  if(s)
    gfxutil_parsecolor(&c, s);
  
  return c;
}

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und String zurückgeben falls gefunden, sonst def  *
 * -------------------------------------------------------------------------- */
char *ini_gets_default(struct ini *ini, const char *key, char *def)
{
  char *s = ini_gets(ini, key);
  
  if(s)
    return s;
  
  /* Im Schreib-Modus einen neuen Key mit dem Default-Wert erstellen */
//  if(ini->mode & INI_WRITE)
    ini_puts(ini, key, def);
  
  return def;
}

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Long Integer zurückgeben falls gefunden,      *
 * sonst default                                                              *
 * -------------------------------------------------------------------------- */
long ini_getlong_default(struct ini *ini, const char *key, long def)
{  
  char *s = ini_gets(ini, key);
  
  if(s)
    return strtol(s, NULL, 10);
  
  /* Im Schreib-Modus einen neuen Key mit dem Default-Wert erstellen */
//  if(ini->mode & INI_WRITE)
    ini_putlong(ini, key, def);
  
  return def;
}

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Unsigned Long Integer zurückgeben falls       *
 * gefunden, sonst default                                                    *
 * -------------------------------------------------------------------------- */
unsigned long ini_getulong_default(struct ini *ini, const char *key, unsigned long def)
{  
  char *s = ini_gets(ini, key);
  
  if(s)
    return strtoul(s, NULL, 10);
  
  /* Im Schreib-Modus einen neuen Key mit dem Default-Wert erstellen */
//  if(ini->mode & INI_WRITE)
    ini_putulong(ini, key, def);
  
  return def;
}

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und double zurückgeben falls gefunden, sonst def  *
 * -------------------------------------------------------------------------- */
double ini_getdouble_default(struct ini *ini, const char *key, double def)
{
  double v = def;
  char *s = ini_gets(ini, key);
  
  if(s)
    sscanf(s, "%lf", &v);
  else// if(ini->mode & INI_WRITE)
    ini_putdouble(ini, key, def);
  
  return v;
}

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Farbe zurückgeben falls gefunden, sonst def   *
 * -------------------------------------------------------------------------- */
struct color ini_getcolor_default(struct ini *ini, const char *key, struct color def)
{  
  char *s;
  
  s = ini_gets(ini, key);
  
  if(s)
    gfxutil_parsecolor(&def, s);
  /* Im Schreib-Modus einen neuen Key mit dem Default-Wert erstellen */
  else //if(ini->mode & INI_WRITE)
    ini_putcolor(ini, key, def);
  
  return def;
}

/* -------------------------------------------------------------------------- *
 * Einen Schlüssel auslesen und Farbe zurückgeben falls gefunden, sonst def   *
 * -------------------------------------------------------------------------- */
sgHSV ini_gethsv_default(struct ini *ini, const char *key, sgHSV def)
{  
  char *s;
  
  s = ini_gets(ini, key);
  
  if(s)
    gfxutil_parsehsv(&def, s);
  /* Im Schreib-Modus einen neuen Key mit dem Default-Wert erstellen */
  else //if(ini->mode & INI_WRITE)
    ini_puthsv(ini, key, def);
  
  return def;
}

/* -------------------------------------------------------------------------- *
 * Alle Daten einer INI Instanz ausgeben                                      *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
void ini_dump(struct ini *ini) 
{
  struct section *section;
  struct key     *key;
  
  ini_debug(INFO, "- ini ----------------------------------------------");
  ini_debug(INFO, "file    = %p", ini->file);
//  ini_debug(INFO, "mode    =%s%s", ((ini->mode & INI_READ) ? " READ"), ((ini->mode & INI_READ) ? " WRITE"));
  ini_debug(INFO, "path    = %s", ini->path);
  ini_debug(INFO, "section = %s", ini->section ? ini->section->name : "NULL");
  
  ini_debug(INFO, "- data ---------------------------------------------");
    
  dlink_foreach(&ini->sections, section)
  {
    if(section->name)
      ini_debug(INFO, "[%s]", section->name);
      
    dlink_foreach(&section->keys, key)
    {
      if(key->name == NULL)
      {
        if(key->value)
          ini_debug(INFO, "%s", key->value);
        else
          ini_debug(INFO, "");
      } 
      else
        ini_debug(INFO, "%s=%s", key->name, key->value);
    }
  }
  
  ini_debug(INFO, "----------------------------------------------------");
}
#endif /* DEBUG */
