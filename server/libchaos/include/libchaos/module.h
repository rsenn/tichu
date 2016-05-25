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
 * $Id: module.h,v 1.14 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_MODULE_H
#define LIB_MODULE_H

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct module;

typedef int  (module_load_t)(struct module *mptr);
typedef void (module_unload_t)(struct module *mptr);

struct module {
  struct node      node;
  uint32_t         id;
  uint32_t         refcount;
  uint32_t         nhash;
  uint32_t         phash;
  int              fd;
  void            *map;
  void            *handle;
  module_load_t   *load;
  module_unload_t *unload;
  char             name[32];
  char             path[64];
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int            module_log;
extern struct sheap   module_heap;
extern struct list    module_list;
extern struct timer  *module_timer;
extern uint32_t       module_id;

/* -------------------------------------------------------------------------- *
 * Initialize module heap.                                                    *
 * -------------------------------------------------------------------------- */
extern void           module_init      (void);

/* -------------------------------------------------------------------------- *
 * Destroy module heap.                                                       *
 * -------------------------------------------------------------------------- */
extern void           module_shutdown  (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void           module_setpath   (const char    *path);

  /* -------------------------------------------------------------------------- *
 * Add a module.                                                              *
 * -------------------------------------------------------------------------- */
extern struct module *module_add       (const char    *path);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int            module_update    (struct module *mptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int            module_reload    (struct module *mptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern const char    *module_expand    (const char    *name);

/* -------------------------------------------------------------------------- *
 * Remove a module.                                                           *
 * -------------------------------------------------------------------------- */
extern void           module_delete    (struct module *module);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct module *module_find_path (const char    *path);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct module *module_find_name (const char    *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct module *module_find_id   (uint32_t       id);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct module *module_pop       (struct module *mptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct module *module_push      (struct module **mptrptr);

/* -------------------------------------------------------------------------- *
 * Dump modules.                                                              *
 * -------------------------------------------------------------------------- */
extern void           module_dump      (struct module *mptr);

#endif /* LIB_MODULE_H */
