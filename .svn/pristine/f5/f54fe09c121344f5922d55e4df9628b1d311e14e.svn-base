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
 * $Id: cfg.h,v 1.4 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_cfg_H
#define LIB_cfg_H

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- *
 * cfg block structure.                                                     *
 * -------------------------------------------------------------------------- */
struct cfg {
  struct node            node;        /* linking node for cfg_list */
  uint32_t               id;
  uint32_t               refcount;    /* times this block is referenced */
  uint32_t               hash;
  struct list           *chain;       /* chain of ini files */
  char                   name[64];    /* user-definable name */
};

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
extern int                      cfg_log;
extern struct sheap             cfg_heap;     /* heap containing cfg blocks */
extern struct dheap             cfg_data_heap;/* heap containing the actual cfgs */
extern struct list              cfg_list;     /* list linking cfg blocks */
extern struct timer            *cfg_timer;
extern uint32_t                 cfg_id;
extern int                      cfg_dirty;

/* -------------------------------------------------------------------------- *
 * Initialize cfg heap and add garbage collect timer.                        *
 * -------------------------------------------------------------------------- */
extern void              cfg_init            (void);

/* -------------------------------------------------------------------------- *
 * Destroy cfg heap and cancel timer.                                        *
 * -------------------------------------------------------------------------- */
extern void              cfg_shutdown        (void);

/* -------------------------------------------------------------------------- *
 * Garbage collect                                                            *
 * -------------------------------------------------------------------------- */
extern int               cfg_collect         (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              cfg_default         (struct cfg  *iptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct cfg       *cfg_new             (const char  *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int               cfg_load            (struct cfg  *cfptr,
                                              const char  *path);

/* -------------------------------------------------------------------------- *
 * Loose all references                                                       *
 * -------------------------------------------------------------------------- */
extern void              cfg_release         (struct cfg  *cfptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              cfg_delete         (struct cfg  *cfptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              cfg_set_name        (struct cfg  *cfptr,
                                              const char    *name);
 
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern const char       *cfg_get_name        (struct cfg  *cfptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct cfg  *cfg_find_name       (const char    *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct cfg  *cfg_find_id         (uint32_t       id);

/* -------------------------------------------------------------------------- *
 * Dump cfgers and cfg heap.                                            *
 * -------------------------------------------------------------------------- */
extern void              cfg_dump            (struct cfg  *cfptr);

#endif /* LIB_cfg_H */
