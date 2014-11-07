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
 * $Id: mfile.h,v 1.5 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_MFILE_H
#define LIB_MFILE_H

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define MFILE_LINELEN 1024

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct mfile {
  struct node node;
  uint32_t    id;
  uint32_t    refcount;
  struct list lines;
  uint32_t    nhash;
  uint32_t    phash;
  int         fd;
  char        path[256];
  char        name[32];
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           mfile_log;
extern struct sheap  mfile_heap;
extern struct dheap  mfile_dheap;
extern struct list   mfile_list;
extern uint32_t      mfile_id;

/* -------------------------------------------------------------------------- *
 * Initialize mfile heap.                                                    *
 * -------------------------------------------------------------------------- */
extern void          mfile_init      (void);

/* -------------------------------------------------------------------------- *
 * Destroy mfile heap.                                                       *
 * -------------------------------------------------------------------------- */
extern void          mfile_shutdown  (void);

/* -------------------------------------------------------------------------- *
 * Add a mfile.                                                              *
 * -------------------------------------------------------------------------- */
extern struct mfile *mfile_add       (const char    *path);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           mfile_update    (struct mfile  *mfile);

/* -------------------------------------------------------------------------- *
 * Remove a mfile.                                                           *
 * -------------------------------------------------------------------------- */
extern void          mfile_delete    (struct mfile *mfile);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct mfile *mfile_find_path (const char    *path);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct mfile *mfile_find_name (const char    *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct mfile *mfile_find_id   (uint32_t       id);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct mfile *mfile_pop       (struct mfile  *mfile);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct mfile *mfile_push      (struct mfile **mfileptr);    

/* -------------------------------------------------------------------------- *
 * Dump mfiles.                                                              *
 * -------------------------------------------------------------------------- */
extern void          mfile_dump     (struct mfile   *mfptr);

#endif /* LIB_MFILE_H */
