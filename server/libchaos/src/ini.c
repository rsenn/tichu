/* chaosircd - pi-networks irc server
 *
 * Copyright (C) 2003-2005  Roman Senn <smoli@paranoya.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 *
 * $Id: ini.c,v 1.44 2005/01/17 19:09:50 smoli Exp $
 */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/io.h>
#include <libchaos/dlink.h>
#include <libchaos/timer.h>
#include <libchaos/log.h>
#include <libchaos/mem.h>
#include <libchaos/image.h>
#include <libchaos/ini.h>
#include <libchaos/str.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int           ini_log;
struct sheap  ini_heap;
struct sheap  ini_key_heap;
struct sheap  ini_sec_heap;
struct list   ini_list;
uint32_t      ini_id;
struct timer *ini_timer;

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int ini_collect(void)
{
  mem_static_collect(&ini_heap);
  mem_static_collect(&ini_key_heap);
  mem_static_collect(&ini_sec_heap);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Strip whitespace                                                           *
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
 * Convert a hex digit to its value                                           *
 * -------------------------------------------------------------------------- */
static inline uint8_t ini_get_hex(char c) 
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
 * Search a key in current section.                                           *
 * -------------------------------------------------------------------------- */
static inline struct ini_key *ini_key_get(struct ini_section *section, const char *name)
{  
  struct ini_key     *key;
  struct node    *node;
  uint32_t       hash;
  
  if(section == NULL)
    return NULL;
  
  hash = strhash(name);
  
  dlink_foreach(&section->keys, node)
  {
    key = (struct ini_key *)node;
    
    if(key->name && key->hash == hash)
    {
      if(!strcmp(key->name, name))
        return key;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Create new key in current section.                                         *
 * -------------------------------------------------------------------------- */
static inline struct ini_key *ini_key_new(struct ini_section *section, const char *name)
{  
  struct ini_key *key;
  
  if(section == NULL)
    return NULL;
  
  key = mem_static_alloc(&ini_key_heap);
  
  key->value = NULL;
  key->hash = 0;
  key->name = NULL;
  
  if(name)
  {
    key->name = strdup(name);
    key->hash = strhash(name);
  }
  
  dlink_add_tail(&section->keys, &key->node, key);
  
  return key;
}

/* -------------------------------------------------------------------------- *
 * Write key to file.                                                         *
 * -------------------------------------------------------------------------- */
static inline int ini_key_write(struct ini *ini, struct ini_key *key) 
{  
  if(key->name == NULL) 
  {
    if(key->value == NULL)
      io_puts(ini->fd, "");
    else 
      io_puts(ini->fd, ";%s", key->value);
  } 
  else 
  {
    if(key->value == NULL) 
      io_puts(ini->fd, "%s=", key->name);
    else 
      io_puts(ini->fd, "%s=%s", key->name, key->value);
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Free key.                                                                  *
 * -------------------------------------------------------------------------- */
static inline void ini_key_free(struct ini_key *key)
{  
  if(key->name)
    free(key->name);
  
  if(key->value)
    free(key->value);
  
  mem_static_free(&ini_key_heap, key);
}

/* -------------------------------------------------------------------------- *
 * Free section.                                                              *
 * -------------------------------------------------------------------------- */
static inline void ini_section_free(struct ini_section *section)
{
  struct node *node;
  struct node *next;
  
  dlink_foreach_safe(&section->keys, node, next)
  {
    dlink_delete(&section->keys, node);
    
    ini_key_free((struct ini_key *)node);
  }
  
  mem_static_free(&ini_sec_heap, section);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void ini_read_cb(int fd, void *ptr)
{
  struct ini *ini = ptr;
  
  io_unregister(fd, IO_CB_READ);
  
  ini_load(ini);
  io_close(fd);
  
//  if(io_list[fd].status.closed || io_list[fd].status.err)
  {
    if(io_list[fd].error)
      log(ini_log, L_warning, "could not read %s: %s", 
          ini->name, syscall_strerror(io_list[fd].error));
  /*  else 
      ini_load(ini);*/
    
//    io_close(fd);
  }
}

/* -------------------------------------------------------------------------- *
 * Initialize INI heaps and add garbage collect timers.                       *
 * -------------------------------------------------------------------------- */
void ini_init(void)
{
  ini_log = log_source_register("ini");
  
  mem_static_create(&ini_heap, sizeof(struct ini), INI_BLOCK_SIZE);
  mem_static_note(&ini_heap, "ini block heap");
  mem_static_create(&ini_key_heap, sizeof(struct ini_key), KEY_BLOCK_SIZE);
  mem_static_note(&ini_key_heap, "ini key heap");
  mem_static_create(&ini_sec_heap, sizeof(struct ini_section), SEC_BLOCK_SIZE);
  mem_static_note(&ini_sec_heap, "ini section heap");

  ini_id = 0;
  
  log(ini_log, L_status, "Initialized [connect] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy INI heap and cancel timers.                                        *
 * -------------------------------------------------------------------------- */
void ini_shutdown(void)
{
  struct node *next;
  struct ini  *ini;
  
  log(ini_log, L_status, "Shutting down [ini] module...");
  
  dlink_foreach_safe(&ini_list, ini, next)
    ini_remove(ini);
  
  mem_static_destroy(&ini_sec_heap);
  mem_static_destroy(&ini_key_heap);
  mem_static_destroy(&ini_heap);
  
  log_source_unregister(ini_log);
}  

/* -------------------------------------------------------------------------- *
 * New INI context.                                                           *
 * -------------------------------------------------------------------------- */
struct ini *ini_add(const char *path)
{
  struct ini *ini;
  char       *x;
  
  ini = mem_static_alloc(&ini_heap);
  
  strlcpy(ini->path, path, sizeof(ini->path));
  
  x = strrchr(ini->path, '/');
  
  if(x)
    strlcpy(ini->name, &x[1], sizeof(ini->name));
  else
    strlcpy(ini->name, path, sizeof(ini->name));
  
  ini->nhash = strihash(ini->name);
  ini->phash = strhash(ini->path);
  ini->id = ini_id++;
  ini->refcount = 1;
  ini->changed = 1;
  
  log(ini_log, L_status, "Added INI file: %s", ini->name);

  ini_open(ini, INI_READ);
 
  dlink_add_tail(&ini_list, &ini->node, ini);
  
  return ini;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int ini_update(struct ini *ini)
{
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Remove INI context.                                                        *
 * -------------------------------------------------------------------------- */
void ini_remove(struct ini *ini)
{
  ini_save(ini);
  
  ini_close(ini);
  
  dlink_delete(&ini_list, &ini->node);
  
  mem_static_free(&ini_heap, ini);
}

/* -------------------------------------------------------------------------- *
 * Find INI context.                                                          *
 * -------------------------------------------------------------------------- */
struct ini *ini_find_name(const char *name)
{
  struct ini  *ini;
  uint32_t     hash;
  
  hash = strihash(name);
    
  dlink_foreach(&ini_list, ini)
  {
    if(ini->nhash == hash)
    {
      if(!strcmp(ini->name, name))
        return ini;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Find INI context.                                                          *
 * -------------------------------------------------------------------------- */
struct ini *ini_find_path(const char *path)
{
  struct ini  *ini;
  uint32_t     hash;
  
  hash = strhash(path);
    
  dlink_foreach(&ini_list, ini)
  {
    if(ini->phash == hash)
    {
      if(!strcmp(ini->path, path))
        return ini;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Find INI context.                                                          *
 * -------------------------------------------------------------------------- */
struct ini *ini_find_id(uint32_t id)
{
  struct ini  *ini;
    
  dlink_foreach(&ini_list, ini)
  {
    if(ini->id == id)
      return ini;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Open INI file.                                                             *
 * -------------------------------------------------------------------------- */
int ini_open(struct ini *ini, int mode) 
{  
  ini->fd = io_open(ini->path, 
                    mode == INI_READ ? O_RDONLY : 
                    O_WRONLY | O_CREAT | O_TRUNC, 0644);
  
  io_queue_control(ini->fd, ON, OFF, ON);
  
  io_register(ini->fd, IO_CB_READ, ini_read_cb, ini);
  
  return ini->fd;
}

/* -------------------------------------------------------------------------- *
 * Open INI file.                                                             *
 * -------------------------------------------------------------------------- */
int ini_open_fd(struct ini *ini, int fd)
{  
  ini->fd = fd;
  
  io_queue_control(ini->fd, ON, OFF, ON);
  
  io_register(ini->fd, IO_CB_READ, ini_read_cb, ini);
  
  return ini->fd;
}

/* -------------------------------------------------------------------------- *
 * Load all sections of an INI file.                                          *
 * -------------------------------------------------------------------------- */
int ini_load(struct ini *ini) 
{
  
  char           buf[4096];
  char          *p;
  uint32_t       len;
  struct ini_section *section = NULL;
  struct ini_key     *key;
  uint32_t       line = 0;
  
  if(ini->mode != INI_READ || ini->sections.size || ini->fd == -1)
    return -1;
  
  /* Read line by line */
  while(io_gets(ini->fd, buf, 4096))
  {
    line++;
    
    /* Remove leading & trailing whitespace */
    ini_strip(buf); 

    len = strlen(buf);
        
    if(buf[0] == ';' || len == 0) 
    {
      if(section == NULL) 
      {
        key = mem_static_alloc(&ini_key_heap);

        key->name = NULL;
        key->value = NULL;
        key->hash = 0;
        
        dlink_add_tail(&ini->keys, &key->node, key);
      } 
      else 
      {
        key = ini_key_new(section, NULL);
      }

      if(key)
      {
        if(len)
          key->value = strdup(&buf[1]);
      }
      
      continue;
    }
    
    /* New section? */
    if(buf[0] == '[' && buf[len - 1] == ']') 
    {
      /* Extract section name */
      buf[0] = ' ';
      buf[len - 1] = ' ';
      ini_strip(buf);
      
      section = ini_section_new(ini, buf);
      
      continue;
    }    
    
    /* No section? */
    if(section == NULL) { 
      log(ini_log, L_warning, "%s:%i: data outside of section", ini->name, line);
      return -1;
    }
    
    /* Split up into key & value */
    p = strchr(buf, '=');
      
    if(p == NULL) { 
      log(ini_log, L_warning, "%s:%i: missing '='", ini->name, line);
      return -1;
    }
    
    *p++ = '\0';
    
    ini_strip(p);
    ini_strip(buf);
    
    ini_write_str(section, buf, p);
  }
  
  ini_section_rewind(ini);
  
/*  ini->changed = 0;*/
  
  if(ini->cb)
    ini->cb(ini);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Save all sections to an INI file.                                          *
 * -------------------------------------------------------------------------- */
int ini_save(struct ini *ini) 
{  
  struct ini_section *section;
  struct ini_key     *key;
  
  if(ini->fd == -1)
    return -1;
  
  if(!ini->changed)
    return 0;
  
  io_queue_control(ini->fd, OFF, ON, ON);
  
  dlink_foreach(&ini->keys, key)
    ini_key_write(ini, key);
  
  dlink_foreach(&ini->sections, section)
  {
    io_puts(ini->fd, "[%s]", section->name);
    
    dlink_foreach(&section->keys, key)
      ini_key_write(ini, key);
  }
  
  io_flush(ini->fd);

  ini->changed = 0;
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Set up a callback.                                                         *
 * -------------------------------------------------------------------------- */
void ini_callback(struct ini *ini, ini_callback_t *cb)
{
  ini->cb = cb;
}

/* -------------------------------------------------------------------------- *
 * Find a INI section by name.                                                *
 * -------------------------------------------------------------------------- */
struct ini_section *ini_section_find(struct ini *ini, const char *name)
{
  struct ini_section *section;
  uint32_t            hash;
  
  if(name == NULL)
    return NULL;
  
  hash = strhash(name);
  
  ini->current = NULL;
  
  dlink_foreach(&ini->sections, section)
  {
    if(section->name && section->hash == hash)
    {
      if(!strcmp(section->name, name))
      {
        ini->current = section;
        
        return section;
      }
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Find a INI section by name.                                                *
 * -------------------------------------------------------------------------- */
struct ini_section *ini_section_find_next(struct ini *ini, const char *name)
{
  struct ini_section *section;
  struct node        *node;
  uint32_t            hash;
  
  if(ini->current == NULL)
    return NULL;
  
  if(name == NULL)
    return NULL;
  
  hash = ini->current->hash;
  
  for(node = ini->current->node.next; node; node = node->next)
  {
    section = node->data;
    
    if(section->name && section->hash == hash)
    {
      if(!strcmp(section->name, name))
      {
        ini->current = section;
        
        return section;
      }
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Create new section.                                                        *
 * -------------------------------------------------------------------------- */
struct ini_section *ini_section_new(struct ini *ini, const char *name) 
{
  struct ini_section *section;

  section = mem_static_alloc(&ini_sec_heap);
    
  section->name = strdup(name);
  section->hash = strhash(name);
  section->ini = ini;
  
  ini->current = section;

  dlink_add_tail(&ini->sections, &section->node, section);
  
  log(ini_log, L_debug, "New INI section %s.", name);
  
  ini->changed = 1;
  
  return section;
}

/* -------------------------------------------------------------------------- *
 * Delete a section.                                                          *
 * -------------------------------------------------------------------------- */
void ini_section_remove(struct ini *ini, struct ini_section *section)
{
  dlink_delete(&ini->sections, &section->node);
  
  ini_section_free(section);

  ini->changed = 1;
}

/* -------------------------------------------------------------------------- *
 * Clear content.                                                             *
 * -------------------------------------------------------------------------- */
void ini_clear(struct ini *ini)
{
  struct ini_section *isptr;
  struct node        *next;
  
  dlink_foreach_safe(&ini->sections, isptr, next)
    ini_section_remove(ini, isptr);
  
  ini->changed = 1;
}

/* -------------------------------------------------------------------------- *
 * Get current section name.                                                  *
 * -------------------------------------------------------------------------- */
char *ini_section_name(struct ini *ini) 
{
  return ini->current ? ini->current->name : NULL;
}

/* -------------------------------------------------------------------------- *
 * Set section by index.                                                      *
 * -------------------------------------------------------------------------- */
struct ini_section *ini_section_index(struct ini *ini, uint32_t i)
{
  struct node *node;

  node = dlink_index(&ini->sections, i);
  
  if(node)
  {
    ini->current = node->data;
    return ini->current;
  }
  else
  {
    ini->current = NULL;
    return NULL;
  }
}

/* -------------------------------------------------------------------------- *
 * Get pointer to first section.                                              *
 * -------------------------------------------------------------------------- */
struct ini_section *ini_section_first(struct ini *ini)
{ 
  return ini->sections.head ? ini->sections.head->data : NULL;
}

/* -------------------------------------------------------------------------- *
 * Skip to next section.                                                      *
 * -------------------------------------------------------------------------- */
struct ini_section *ini_section_next(struct ini *ini)
{ 
  struct ini_section *section;
  
  if(ini->current == NULL)
    return NULL;
  
  section = ini->current;
  
  ini->current = ini->current->node.next ? ini->current->node.next->data : NULL;
  
  return section;
}

/* -------------------------------------------------------------------------- *
 * Return number of sections.                                                 *
 * -------------------------------------------------------------------------- */
uint32_t ini_section_count(struct ini *ini) 
{
  return ini->sections.size;
}

/* -------------------------------------------------------------------------- *
 * Return position of current section.                                        *
 * -------------------------------------------------------------------------- */
uint32_t ini_section_pos(struct ini *ini) 
{  
  struct ini_section *section;
  uint32_t            n = 0;
 
  dlink_foreach(&ini->sections, section)
  {
    if(section == ini->current)
      return n;
    
    n++;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Rewind to first section.                                                   *
 * -------------------------------------------------------------------------- */
void ini_section_rewind(struct ini *ini)
{
  ini->current = (struct ini_section *)ini->sections.head;
}

/* -------------------------------------------------------------------------- *
 * Write data to .ini, creating new key when necessary.                       *
 * -------------------------------------------------------------------------- */
int ini_write_str(struct ini_section *section, const char *key, const char *str) 
{  
  struct ini_key *k;
  
  k = ini_key_get(section, key);
  
  if(k) 
  {
    if(k->value)
      free(k->value);
    
    k->value = strdup(str);
    
    return 0;
  }

  k = ini_key_new(section, key);
  
  if(k) 
  {
    k->value = strdup(str); 

    section->ini->changed = 1;
    
    return 0;
  }
  
  return -1;
}

int ini_write_int(struct ini_section *section, const char *key, int i) 
{
  char buf[16];
  
  snprintf(buf, 16, "%i", i);
  
  return ini_write_str(section, key, buf);
}

int ini_write_ulong_long(struct ini_section *section, const char *key, uint64_t u)
{  
  char buf[64];
  
  snprintf(buf, 64, "%llu", u);
  
  return ini_write_str(section, key, buf);  
}

int ini_write_double(struct ini_section *section, const char *key, double d) 
{  
  char buf[64];
  
  snprintf(buf, 64, "%f", d);
  
  return ini_write_str(section, key, buf);  
}

/* -------------------------------------------------------------------------- *
 * Read data from .ini, returning -1 when key not found.                      *
 * -------------------------------------------------------------------------- */
int ini_read_str(struct ini_section *section, const char *key, char **str) 
{
  struct ini_key *k = ini_key_get(section, key);
  
  if(k) 
  {
    if(str) 
      *str = k->value;
    
    return 0;
  }

  return -1;
}

int ini_get_str(struct ini_section *section, const char *key, char *str, size_t len) 
{  
  struct ini_key *k = ini_key_get(section, key);
  
  if(k) 
  {
    if(str && len) 
      strlcpy(str, k->value, len);
    
    return 0;
  }

  return -1;
}

int ini_read_int(struct ini_section *section, const char *key, int *i) 
{  
  char *s;
  
  if(!ini_read_str(section, key, &s)) 
  {
    if(!s) 
      return -1;
    
    if(i == NULL) 
      return 0;
    
    *i = atoi(s);
    
    return 0;
  }
  
  return -1;
}

int ini_read_ulong_long(struct ini_section *section, const char *key, uint64_t *u)
{  
  char *s;
  
  if(!ini_read_str(section, key, &s)) 
  {
    if(!s) 
      return -1;
    
    if(u == NULL) 
      return 0;
    
    *u = strtoull(s, NULL, 10);

    return 0;
  }
  
  return -1;
}

int ini_read_str_def(struct ini_section *section, const char *key, char **str, char *def) 
{
  if(ini_read_str(section, key, str) == -1) 
  {
    ini_write_str(section, key, def);
    
    if(str) 
      *str = def;
    
    return 1;
  }
  
  return 0;
}

int ini_get_str_def(struct ini_section *section, const char *key, char *str,
                    size_t len, char *def) 
{
  if(ini_get_str(section, key, str, len) == -1) 
  {
    ini_write_str(section, key, def);
    
    if(str) 
    {
      if(len) 
        strlcpy(str, def, len);
    }
    
    return 1;
  }
  
  return 0;
}

int ini_read_int_def(struct ini_section *section, const char *key, int *i, int def) 
{
  if(ini_read_int(section, key, i) == -1) 
  {
    ini_write_int(section, key, def);
    
    if(i) 
      *i = def;
    
    return 1;
  }
  
  return 0;
}

int ini_read_color(struct ini_section *section, const char *key, struct color *color)
{
  char *str = "";
  
  ini_read_str(section, key, &str);
  
  image_color_parse(color, str);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Close INI file.                                                            *
 * -------------------------------------------------------------------------- */
void ini_close(struct ini *ini)
{
  if(ini->fd >= 0) 
    io_close(ini->fd);
  
  ini->fd = -1;
}

/* -------------------------------------------------------------------------- *
 * Free INI context.                                                          *
 * -------------------------------------------------------------------------- */
void ini_free(struct ini *ini) 
{
  struct ini_section *section;
  struct ini_key     *key;
  struct node        *next;
  
//  if(io_valid(ini->fd))
    ini_close(ini);
  
  dlink_foreach_safe(&ini->keys, key, next)
  {
    dlink_delete(&ini->keys, &key->node);
    ini_key_free(key);
  }
  
  dlink_foreach_safe(&ini->sections, section, next)
  {
    dlink_delete(&ini->sections, &section->node);
    
    ini_section_free(section);
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct ini *ini_pop(struct ini *ini)
{
  if(ini)
  {
    if(!ini->refcount)
      log(ini_log, L_warning, "Poping deprecated ini: %s",
          ini->name);
    
    ini->refcount++;
  }
  
  return ini;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct ini *ini_push(struct ini **iniptr)
{
  if(*iniptr)
  {
    if((*iniptr)->refcount == 0)
    {
      log(ini_log, L_warning, "Trying to push deprecated user: %s",
          (*iniptr)->name);
    }
    else
    {
      if(--(*iniptr)->refcount == 0)
        ini_remove(*iniptr);
      
      (*iniptr) = NULL;
    }
  }

  return *iniptr;
}

/* -------------------------------------------------------------------------- *
 * Free INI context.                                                          *
 * -------------------------------------------------------------------------- */
void ini_dump(struct ini *ini) 
{
  struct ini_section *section;
  struct ini_key     *key;
  
  if(ini == NULL)
  {
    dump(ini_log, "[================ ini summary ================]");
    
    dlink_foreach(&ini_list, ini)
    {
      dump(ini_log, " #%u: [%u] %-20s (%i)",
           ini->id, ini->refcount, ini->name, ini->fd);
    }
    
    dump(ini_log, "[============= end of ini summary ============]");
  }  
  else
  {
    dump(ini_log, "[================= ini dump =================]");
    
    dump(ini_log, "         id: #%u", ini->id);
    dump(ini_log, "   refcount: %u", ini->refcount);
    dump(ini_log, "      nhash: %p", ini->nhash);
    dump(ini_log, "      phash: %p", ini->phash);
    dump(ini_log, "   sections: %i items", ini->sections.size);
    dump(ini_log, "       keys: %i items", ini->keys.size);
    dump(ini_log, "         fd: %i", ini->fd);
    dump(ini_log, "       path: %s", ini->path);
    dump(ini_log, "       name: %s", ini->name);
    dump(ini_log, "       mode: %p", ini->mode);
    dump(ini_log, "    current: %p", ini->current);
    dump(ini_log, "         cb: %p", ini->cb);    
    dump(ini_log, "      error: %s", ini->error);

    dump(ini_log, "------------------ ini data -----------------");
    
    dlink_foreach(&ini->keys, key)
    {
      if(key->name == NULL)
      {
        if(key->value)
        {
          dump(ini_log, ";%s", key->value);
        }
        else
        {
          dump(ini_log, "");
        }
      } 
      else
      {
        dump(ini_log, "%s=%s", key->name, key->value);
      }
    }
    
    dlink_foreach(&ini->sections, section)
    {
      dump(ini_log, "[%s]", section->name);
      
      dlink_foreach(&section->keys, key)
      {
        if(key->name == NULL)
        {
          if(key->value)
          {
            dump(ini_log, ";%s", key->value);
          }
          else
          {
            dump(ini_log, "");
          }
        } 
        else
        {
          dump(ini_log, "%s=%s", key->name, key->value);
        }
      }
    }
    
    dump(ini_log, "[============== end of ini dump =============]");
  }    
}
