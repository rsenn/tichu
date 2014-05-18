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
 * $Id: template.h,v 1.5 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_TEMPLATE_H
#define LIB_TEMPLATE_H

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- *
 * template block structure.                                                     *
 * -------------------------------------------------------------------------- */
struct template {
  struct node            node;        /* linking node for template_list */
  uint32_t               id;
  uint32_t               refcount;    /* times this block is referenced */
  uint32_t               hash;
  char                   name[64];    /* user-definable name */
};

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
extern int                      template_log;
extern struct sheap             template_heap;     /* heap containing template blocks */
extern struct dheap             template_data_heap;/* heap containing the actual templates */
extern struct list              template_list;     /* list linking template blocks */
extern struct timer            *template_timer;
extern uint32_t                 template_id;
extern int                      template_dirty;

/* -------------------------------------------------------------------------- *
 * Initialize template heap and add garbage collect timer.                        *
 * -------------------------------------------------------------------------- */
extern void              template_init            (void);

/* -------------------------------------------------------------------------- *
 * Destroy template heap and cancel timer.                                        *
 * -------------------------------------------------------------------------- */
extern void              template_shutdown        (void);

/* -------------------------------------------------------------------------- *
 * Garbage collect                                                            *
 * -------------------------------------------------------------------------- */
extern int               template_collect         (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              template_default         (struct template  *iptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct template  *template_new             (const char       *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              template_delete          (struct template  *iptr);

/* -------------------------------------------------------------------------- *
 * Loose all references                                                       *
 * -------------------------------------------------------------------------- */
extern void              template_release         (struct template  *iptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              template_set_name        (struct template  *iptr,
                                                   const char    *name);
 
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern const char       *template_get_name        (struct template  *iptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct template  *template_find_name       (const char    *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct template  *template_find_id         (uint32_t       id);

/* -------------------------------------------------------------------------- *
 * Dump templateers and template heap.                                            *
 * -------------------------------------------------------------------------- */
extern void              template_dump            (struct template  *iptr);

#endif /* LIB_TEMPLATE_H */
