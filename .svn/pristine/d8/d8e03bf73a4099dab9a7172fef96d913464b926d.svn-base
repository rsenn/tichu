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
 * $Id: queue.h,v 1.16 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_QUEUE_H
#define LIB_QUEUE_H

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/dlink.h>
#include <libchaos/defs.h>

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */
#define QUEUE_CHUNK_SIZE 128

/* -------------------------------------------------------------------------- *
 * Types                                                                      *
 * -------------------------------------------------------------------------- */
struct finfo {
  uint32_t head;                   /* Read ptr inside block */
  uint32_t tail;                   /* Write ptr inside block */
  uint32_t size;                   /* Byte count of the block */
  uint32_t lines;                  /* Line count of the block */
};

struct fblock {
  uint32_t     refcount;
  struct finfo info;
  uint8_t      data[QUEUE_CHUNK_SIZE]; /* Actual data */
};

struct fqueue {
  uint32_t     size;
  uint32_t     lines;
  struct list  blocks;
  struct finfo head;
  struct finfo tail;
};

#define queue_zero_info(x) do { \
  (x)->head = 0; \
  (x)->tail = 0; \
  (x)->size = 0; \
  (x)->lines = 0; \
} while(0);

/* -------------------------------------------------------------------------- *
 * Initialize the queue code.                                                 *
 * -------------------------------------------------------------------------- */
extern void     queue_init        (void);

/* -------------------------------------------------------------------------- *
 * Initialize the queue code.                                                 *
 * -------------------------------------------------------------------------- */
extern void     queue_shutdown    (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void     queue_zero        (struct fqueue *fifoptr);

/* -------------------------------------------------------------------------- *
 * Garbage collect queue data                                                 *
 * -------------------------------------------------------------------------- */
extern void     queue_collect     (void);

/* -------------------------------------------------------------------------- *
 * Write data to the queue tail.                                              *
 * -------------------------------------------------------------------------- */
extern uint32_t queue_write       (struct fqueue *fifoptr,
                                   const void    *buf,
                                   uint32_t       n);

/* -------------------------------------------------------------------------- *
 * Write a string to queue tail.                                              *
 * -------------------------------------------------------------------------- */
extern uint32_t queue_puts        (struct fqueue *fifoptr,
                                   const char    *s);

/* -------------------------------------------------------------------------- *
 * Read data from the queue head.                                             *
 * -------------------------------------------------------------------------- */
extern uint32_t queue_read        (struct fqueue *fifoptr,
                                   void          *buf,
                                   uint32_t       n);

/* -------------------------------------------------------------------------- *
 * Get a line from queue tail.                                                *
 * -------------------------------------------------------------------------- */
extern uint32_t  queue_gets       (struct fqueue *fifoptr,
                                   void          *buf,
                                   uint32_t       n);

/* -------------------------------------------------------------------------- *
 * Read stuff from the queue tail but do NOT dequeue it.                      *
 * -------------------------------------------------------------------------- */
extern uint32_t  queue_map         (struct fqueue *fifoptr,
                                    void          *buf, 
                                    uint32_t       n);  

/* -------------------------------------------------------------------------- *
 * Remove n bytes from queue tail.                                            *
 * -------------------------------------------------------------------------- */
extern uint32_t  queue_cut         (struct fqueue *fifoptr,
                                    uint32_t       n);

/* -------------------------------------------------------------------------- *
 * Frees a queue.                                                             *
 * -------------------------------------------------------------------------- */
extern void      queue_free        (struct fqueue *fifoptr);

/* -------------------------------------------------------------------------- *
 * Link a queue to the tail of another.                                       *
 * -------------------------------------------------------------------------- */
extern void      queue_link        (struct fqueue *from, 
                                    struct fqueue *to);

/* -------------------------------------------------------------------------- *
 * Release a multicast list.                                                  *
 * -------------------------------------------------------------------------- */
extern void      queue_destroy     (struct fqueue *fifoptr);
    
/* -------------------------------------------------------------------------- *
 * Dump a queue.                                                              *
 * -------------------------------------------------------------------------- */
#ifdef DEBUG
extern void      queue_dump        (struct fqueue *fifoptr);
#endif /* DEBUG */  

#endif /* LIB_QUEUE_H */
