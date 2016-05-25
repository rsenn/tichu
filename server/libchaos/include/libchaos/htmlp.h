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
 * $Id: htmlp.h,v 1.4 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_HTMLP_H
#define LIB_HTMLP_H

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define HTMLP_LINELEN  1024
#define HTMLP_MAX_BUF  (256 * 1024)

#define HTMLP_IDLE       0
#define HTMLP_DONE       1

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct htmlp;

typedef void (*htmlp_cb_t)(struct htmlp *, void *, void *, void *, void *);

struct htmlp_var {
  struct node node;
  uint32_t    hash;
  char        name[64];
  char        value[128];
};

struct htmlp_tag {
  struct node node;
  uint32_t    hash;
  int         closing;
  struct list vars;
  char       *text;
  char        name[64];
};

struct htmlp {
  struct node        node;
  uint32_t           id;
  uint32_t           refcount;
  int                fd;
  uint32_t           nhash;
  int                status;
  struct list        tags;
  char              *buf;
  struct htmlp_tag  *current;
  void              *args[4];
  char               name[32];
  char               path[64];
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           htmlp_log;
extern struct sheap  htmlp_heap;
extern struct sheap  htmlp_var_heap;
extern struct dheap  htmlp_dheap;
extern struct list   htmlp_list;
extern uint32_t      htmlp_id;

/* -------------------------------------------------------------------------- *
 * Initialize htmlp heap.                                                     *
 * -------------------------------------------------------------------------- */
extern void          htmlp_init      (void);

/* -------------------------------------------------------------------------- *
 * Destroy htmlp heap.                                                        *
 * -------------------------------------------------------------------------- */
extern void          htmlp_shutdown  (void);

/* -------------------------------------------------------------------------- *
 * Add a htmlp.                                                               *
 * -------------------------------------------------------------------------- */
extern struct htmlp *htmlp_new       (const char    *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           htmlp_update    (struct htmlp  *htmlp);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           htmlp_parse     (struct htmlp  *htmlp,
                                      const char    *data,
                                      size_t         n);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          htmlp_clear     (struct htmlp  *htmlp);

/* -------------------------------------------------------------------------- *
 * Remove a htmlp.                                                            *
 * -------------------------------------------------------------------------- */
extern void          htmlp_delete    (struct htmlp  *htmlp);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct htmlp *htmlp_find_name (const char    *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct htmlp *htmlp_find_id   (uint32_t       id);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          htmlp_vset_args (struct htmlp  *htmlp, 
                                      va_list        args);
extern void          htmlp_set_args  (struct htmlp  *htmlp, 
                                      ...);    

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct htmlp_tag *htmlp_tag_first (struct htmlp *htptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct htmlp_tag *htmlp_tag_next  (struct htmlp *htptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct htmlp_tag *htmlp_tag_find  (struct htmlp *htptr, 
                                          const char   *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern uint32_t          htmlp_tag_count (struct htmlp *htptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct htmlp_tag *htmlp_tag_index (struct htmlp *htptr, 
                                          uint32_t      i);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct htmlp_var *htmlp_var_find  (struct htmlp *htptr,
                                          const char   *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct htmlp_var *htmlp_var_set   (struct htmlp *htptr,
                                          const char   *name, 
                                          const char   *value);  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct htmlp *htmlp_pop       (struct htmlp  *htmlp);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct htmlp *htmlp_push      (struct htmlp **htmlpptr);    

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern char  *htmlp_decode           (const char    *s);
  

/* -------------------------------------------------------------------------- *
 * Dump htmlps.                                                               *
 * -------------------------------------------------------------------------- */
extern void          htmlp_dump      (struct htmlp   *hcptr);

#endif /* LIB_HTMLP_H */
