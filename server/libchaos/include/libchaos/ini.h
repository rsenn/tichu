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
 * $Id: ini.h,v 1.20 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_INI_H
#define LIB_INI_H

#include <libchaos/image.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define INI_READ  0
#define INI_WRITE 1

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct ini;

typedef void (ini_callback_t)(struct ini *ini);

struct ini_key 
{
  struct node              node;
  uint32_t                 hash;
  char                    *name;
  char                    *value;
};

struct ini_section 
{
  struct node              node;
  struct list              keys;
  struct ini              *ini;
  uint32_t                 hash;
  char                    *name;
};

struct ini 
{
  struct node              node;
  uint32_t                 id;
  uint32_t                 refcount;
  uint32_t                 nhash;
  uint32_t                 phash;  
  struct list              keys;        /* keys & comments before first section */
  struct list              sections;
  int                      fd;
  char                     path[PATH_MAX];
  char                     name[64];
  int                      mode;
  int                      changed;
  struct ini_section      *current;
  ini_callback_t          *cb;
  char                     error[256];
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int                 ini_log;
extern struct sheap        ini_heap;
extern struct sheap        ini_key_heap;
extern struct sheap        ini_sec_heap;
extern struct list         ini_list;
extern uint32_t            ini_serial;
extern struct timer       *ini_timer;

/* -------------------------------------------------------------------------- *
 * Initialize INI heaps and add garbage collect timers.                       *
 * -------------------------------------------------------------------------- */
extern void                ini_init             (void);
  
/* -------------------------------------------------------------------------- *
 * Destroy INI heap and cancel timers.                                        *
 * -------------------------------------------------------------------------- */
extern void                ini_shutdown         (void);
  
/* -------------------------------------------------------------------------- *
 * New INI context.                                                           *
 * -------------------------------------------------------------------------- */
extern struct ini         *ini_add              (const char         *path);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int                 ini_update           (struct ini         *ini);

/* -------------------------------------------------------------------------- *
 * Find INI context.                                                          *
 * -------------------------------------------------------------------------- */
extern struct ini         *ini_find_name        (const char         *name);

/* -------------------------------------------------------------------------- *
 * Find INI context.                                                          *
 * -------------------------------------------------------------------------- */
extern struct ini         *ini_find_path        (const char         *path);

/* -------------------------------------------------------------------------- *
 * Find INI context.                                                          *
 * -------------------------------------------------------------------------- */
extern struct ini         *ini_find_id          (uint32_t            id);

/* -------------------------------------------------------------------------- *
 * Destroy INI context.                                                       *
 * -------------------------------------------------------------------------- */
extern void                ini_remove           (struct ini         *ini);

/* -------------------------------------------------------------------------- *
 * Open INI file.                                                             *
 * -------------------------------------------------------------------------- */
extern int                 ini_open             (struct ini         *ini,
                                                 int                 mode);

/* -------------------------------------------------------------------------- *
 * Open INI file.                                                             *
 * -------------------------------------------------------------------------- */
extern int                 ini_open_fd          (struct ini         *ini,
                                                 int                 fd);

/* -------------------------------------------------------------------------- *
 * Load all sections of an INI file.                                          *
 * -------------------------------------------------------------------------- */
extern int                 ini_load             (struct ini         *ini);

/* -------------------------------------------------------------------------- *
 * Save all sections to an INI file.                                          *
 * -------------------------------------------------------------------------- */
extern int                 ini_save             (struct ini         *ini); 

/* -------------------------------------------------------------------------- *
 * Close INI file.                                                            *
 * -------------------------------------------------------------------------- */
extern void                ini_close            (struct ini         *ini);

/* -------------------------------------------------------------------------- *
 * Free INI context.                                                          *
 * -------------------------------------------------------------------------- */
extern void                ini_free             (struct ini         *ini);

/* -------------------------------------------------------------------------- *
 * Setup callback.                                                            *
 * -------------------------------------------------------------------------- */
extern void                ini_callback         (struct ini         *ini,
                                                 ini_callback_t     *cb);

/* -------------------------------------------------------------------------- *
 * Find a INI section by name.                                                *
 * -------------------------------------------------------------------------- */
extern struct ini_section *ini_section_find     (struct ini         *ini, 
                                                 const char         *name);

/* -------------------------------------------------------------------------- *
 * Find a INI section by name.                                                *
 * -------------------------------------------------------------------------- */
extern struct ini_section *ini_section_find_next(struct ini         *ini, 
                                                 const char         *name);

/* -------------------------------------------------------------------------- *
 * Create new section.                                                        *
 * -------------------------------------------------------------------------- */
extern struct ini_section *ini_section_new      (struct ini         *ini,
                                                 const char         *name);

/* -------------------------------------------------------------------------- *
 * Delete a section.                                                          *
 * -------------------------------------------------------------------------- */
extern void                ini_section_remove   (struct ini         *ini, 
                                                 struct ini_section *section);

/* -------------------------------------------------------------------------- *
 * Clear content.                                                             *
 * -------------------------------------------------------------------------- */
extern void                ini_clear            (struct ini         *ini);
  
/* -------------------------------------------------------------------------- *
 * Get current section name.                                                  *
 * -------------------------------------------------------------------------- */
extern char               *ini_section_name     (struct ini         *ini);

/* -------------------------------------------------------------------------- *
 * Set current section by index.                                              *
 * -------------------------------------------------------------------------- */
extern struct ini_section *ini_section_index    (struct ini         *ini,
                                                 uint32_t            index);

/* -------------------------------------------------------------------------- *
 * Get pointer to first section.                                              *
 * -------------------------------------------------------------------------- */
extern struct ini_section *ini_section_first    (struct ini         *ini);

/* -------------------------------------------------------------------------- *
 * Skip to next section and return current                                    *
 * -------------------------------------------------------------------------- */
extern struct ini_section *ini_section_next     (struct ini         *ini);

/* -------------------------------------------------------------------------- *
 * Return number of sections.                                                 *
 * -------------------------------------------------------------------------- */
extern uint32_t            ini_section_count    (struct ini         *ini);

/* -------------------------------------------------------------------------- *
 * Return position of current section.                                        *
 * -------------------------------------------------------------------------- */
extern uint32_t            ini_section_pos      (struct ini         *ini);

/* -------------------------------------------------------------------------- *
 * Rewind to first section.                                                   *
 * -------------------------------------------------------------------------- */
extern void                ini_section_rewind   (struct ini         *ini);

/* -------------------------------------------------------------------------- *
 * Write data to .ini, creating new key when necessary.                       *
 * -------------------------------------------------------------------------- */
extern int                 ini_write_str        (struct ini_section *section,
                                                 const char         *key,
                                                 const char         *str);

extern int                 ini_write_int        (struct ini_section *section,
                                                 const char         *key,
                                                 int                 i);

extern int                 ini_write_ulong_long (struct ini_section *section,
                                                 const char         *key,
                                                 uint64_t            u);

extern int                 ini_write_double     (struct ini_section *section,
                                                 const char         *key,
                                                 double              d);

/* -------------------------------------------------------------------------- *
 * Read data from .ini, returning -1 when key not found.                      *
 * -------------------------------------------------------------------------- */
extern int                 ini_read_str         (struct ini_section *section,
                                                 const char         *key,
                                                 char              **str);

extern int                 ini_get_str          (struct ini_section *section, 
                                                 const char         *key, 
                                                 char               *str, 
                                                 size_t              len);

extern int                 ini_read_int         (struct ini_section *section,
                                                 const char         *key,
                                                 int                *i);

extern int                 ini_read_ulong_long  (struct ini_section *section,
                                                 const char         *key,
                                                 uint64_t           *u);

extern int                 ini_read_double      (struct ini_section *section,
                                                 const char         *key,
                                                 double             *d);

/* -------------------------------------------------------------------------- *
 * Read data from .ini, defaulting to a given value.                          *
 * -------------------------------------------------------------------------- */
extern int                 ini_read_str_def     (struct ini_section *section,
                                                 const char         *key,
                                                 char              **str,
                                                 char               *def);

extern int                 ini_get_str_def      (struct ini_section *section,
                                                 const char         *key,
                                                 char               *str,
                                                 size_t              len,
                                                 char               *def);

extern int                 ini_read_int_def     (struct ini_section *section,
                                                 const char         *key,
                                                 int                *i,
                                                 int                 def);

extern int                 ini_read_double_def  (struct ini_section *section,
                                                 const char         *key,
                                                 double             *d,
                                                 double              def);

extern int                 ini_read_color       (struct ini_section *section,
                                                 const char         *key,
                                                 struct color       *color);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct ini         *ini_pop              (struct ini        *ini);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct ini         *ini_push             (struct ini       **iniptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void                ini_dump             (struct ini        *ini);

#endif /* LIB_INI_H */
