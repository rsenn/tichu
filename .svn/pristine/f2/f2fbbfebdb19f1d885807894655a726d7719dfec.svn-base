/* chaosircd - pi-networks irc server
 *              
 * Copyright (C) 2003,2004  Roman Senn <smoli@paranoya.ch>
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
 * $Id: wav.h,v 1.1 2005/01/24 13:44:51 smoli Exp $
 */

#ifndef LIB_WAV_H
#define LIB_WAV_H

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */
#define WAV_MONO   1
#define WAV_STEREO 2

#define WAV_8BIT   8
#define WAV_16BIT 16

#define WAV_CHUNK_SIZE 65536

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct wav_chunk
{
  struct node            node;
  size_t                 size;
  char                   data[0];
};

/* -------------------------------------------------------------------------- *
 * wav block structure.                                                     *
 * -------------------------------------------------------------------------- */
struct wav {
  struct node            node;        /* linking node for wav_list */
  uint32_t               id;
  uint32_t               refcount;    /* times this block is referenced */
  uint32_t               hash;
  
  int                    fd;
  
  int                    bits;
  int                    channels;
  int                    rate;
  
  size_t                 size;
  size_t                 chunksize;
  int                    rpos;         /* position inside rchunk */
  int                    wpos;         /* position inside wchunk */
  struct wav_chunk      *rchunk;       /* current chunk */
  struct wav_chunk      *wchunk;       /* current chunk */
  
  struct list            chunks;
  
  char                   name[64];    /* user-definable name */
  char                   path[PATH_MAX];
};

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
extern int               wav_log;
extern struct sheap      wav_heap;     /* heap containing wav blocks */
extern struct dheap      wav_data_heap;/* heap containing the actual wavs */
extern struct list       wav_list;     /* list linking wav blocks */
extern struct timer     *wav_timer;
extern uint32_t          wav_id;
extern int               wav_dirty;

/* -------------------------------------------------------------------------- *
 * Initialize wav heap and add garbage collect timer.                        *
 * -------------------------------------------------------------------------- */
extern void              wav_init            (void);

/* -------------------------------------------------------------------------- *
 * Destroy wav heap and cancel timer.                                        *
 * -------------------------------------------------------------------------- */
extern void              wav_shutdown        (void);

/* -------------------------------------------------------------------------- *
 * Garbage collect                                                            *
 * -------------------------------------------------------------------------- */
extern int               wav_collect         (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              wav_default         (struct wav  *wav);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct wav       *wav_new             (const char  *name,
                                              int          bits,
                                              int          channels,
                                              int          rate);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              wav_sample_putmono  (struct wav  *wav,
                                              short        data);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int               wav_save            (struct wav  *wav);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              wav_delete          (struct wav  *wav);

/* -------------------------------------------------------------------------- *
 * Loose all references                                                       *
 * -------------------------------------------------------------------------- */
extern void              wav_release         (struct wav  *wav);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              wav_set_name        (struct wav  *wav,
                                              const char  *name);
 
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern const char       *wav_get_name        (struct wav  *wav);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct wav       *wav_find_name       (const char  *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct wav       *wav_find_id         (uint32_t     id);

/* -------------------------------------------------------------------------- *
 * Dump wavers and wav heap.                                            *
 * -------------------------------------------------------------------------- */
extern void              wav_dump            (struct wav  *wav);

#endif /* LIB_WAV_H */
