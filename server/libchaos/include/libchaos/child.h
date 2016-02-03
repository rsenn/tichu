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
 * $Id: child.h,v 1.16 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_CHILD_H
#define LIB_CHILD_H

/* -------------------------------------------------------------------------- *
 * System headers                                                             *
 * -------------------------------------------------------------------------- */
#include <sys/types.h>

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>
#include <libchaos/timer.h>

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */

#define CHILD_IDLE       0x00          /* child block just sitting there */
#define CHILD_RUNNING    0x01
#define CHILD_DEAD       0x02

#define CHILD_MAX_CHANNEL 4      /* max. I/O channels to the child */

#define PARENT 0
#define CHILD  1

#define CHILD_INTERVAL   20000L

/* -------------------------------------------------------------------------- *
 * Types                                                                      *
 * -------------------------------------------------------------------------- */
struct child;
typedef void (child_cb_t)(struct child *, void *, void *, void *, void *);

/* -------------------------------------------------------------------------- *
 * Child block structure.                                                     *
 * -------------------------------------------------------------------------- */
struct child {
  struct node    node;         /* linking node for child block list */
  
  /* externally initialised */
  pid_t          pid;
  uint32_t       id;
  uint32_t       refcount;
  uint32_t       chash;
  uint32_t       nhash;
  uint32_t       status;
  struct timer  *timer;
  uint32_t       chans;
  int            channels[CHILD_MAX_CHANNEL][2][2];
  int            autostart;
  int            exitcode;
  uint64_t       interval;
  uint64_t       start;        /* time at which the child was started */
  char           name[HOSTLEN + 1];
  char           path[PATH_MAX + 1];
  char           argv[64];
  char           arguments[CHILD_MAX_CHANNEL * 4][16];
  void          *args[4];
  child_cb_t    *callback;
};

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
extern int           child_log;          /* Log source */
extern struct sheap  child_heap;         /* Heap containing child blocks */
extern struct list   child_list;         /* List linking child blocks */
extern int           child_dirty;        /* we need a garbage collect */
extern uint32_t      child_id;           /* Next serial number */

/* -------------------------------------------------------------------------- *
 * Initialize child heap and add garbage collect timer.                       *
 * -------------------------------------------------------------------------- */
extern void          child_init         (void);

/* -------------------------------------------------------------------------- *
 * Destroy child heap and cancel timer.                                       *
 * -------------------------------------------------------------------------- */
extern void          child_shutdown     (void);

/* -------------------------------------------------------------------------- *
 * Garbage collect child blocks                                               *
 * -------------------------------------------------------------------------- */
extern void          child_collect      (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          child_default      (struct child  *cdptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct child *child_new          (const char    *path,
                                         uint32_t       channels,
                                         const char    *argv,
                                         uint64_t       interval,
                                         int            autostart);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           child_update       (struct child  *cdptr,
                                         uint32_t       channels,
                                         const char    *argv,
                                         uint64_t       interval,
                                         int            autostart);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          child_delete       (struct child  *cdptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct child *child_find         (const char    *path);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct child *child_pop          (struct child  *cdptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct child *child_push         (struct child **cdptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int           child_launch       (struct child  *cdptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          child_cancel       (struct child  *cdptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          child_kill         (struct child  *cdptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          child_set_callback (struct child  *cdptr,
                                         void          *callback);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          child_vset_args    (struct child  *cdptr,
                                         va_list        args);

extern void          child_set_args     (struct child  *cdptr,
                                         ...);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          child_set_name     (struct child  *cdptr,
                                         const char    *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern const char   *child_get_name     (struct child  *cdptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct child *child_find_name    (const char    *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct child *child_find_id      (uint32_t       id);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void          child_dump         (struct child  *cdptr);
  
#endif /* LIB_CHILD_H */
