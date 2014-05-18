/* chaosircd - pi-networks irc server
 *              
 * Copyright (C) 2004-2005  Roman Senn <smoli@paranoya.ch>
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
 * $Id: graph.h,v 1.31 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_GRAPH_H
#define LIB_GRAPH_H

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/dlink.h>

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */
#define GRAPH_TYPE_LINE   0        
#define GRAPH_TYPE_COLUMN 1        

#define GRAPH_COLOR_DARK   0            /* dark palette style */
#define GRAPH_COLOR_BRIGHT 1            /* bright palette style */

#define GRAPH_SOURCE_INT   0
#define GRAPH_SOURCE_UINT  1
#define GRAPH_SOURCE_LONG  2
#define GRAPH_SOURCE_ULONG 3

#define GRAPH_COLOR_BACKGROUND 0
#define GRAPH_COLOR_BORDER 1
#define GRAPH_COLOR_GRID 2
#define GRAPH_COLOR_DATA 3
#define GRAPH_COLOR_TEXT 4
#define GRAPH_COLOR_UTEXT 5

#define GRAPH_COLUMN_WIDTH   14
#define GRAPH_COLUMN_SPACING 8

#define GRAPH_GRID_MEASURE 6
#define GRAPH_GRID_GRID 3
#define GRAPH_GRID_MINSPACE 20

#define GRAPH_PADDING_LEFT   50
#define GRAPH_PADDING_RIGHT  20
#define GRAPH_PADDING_TOP    10
#define GRAPH_PADDING_BOTTOM 40

/* -------------------------------------------------------------------------- *
 * template block structure.                                                  *
 * -------------------------------------------------------------------------- */
struct graph {
  struct node            node;        /* linking node for graph_list */
  uint32_t               id;
  uint32_t               refcount;    /* times this block is referenced */
  uint32_t               hash;
  uint16_t               width;
  uint16_t               height;
  uint16_t               grid_width;
  uint16_t               grid_height;
  time_t                 current;
  time_t                 end;
  uint32_t               resolution;
  uint32_t               samples;
  uint32_t               usamples;
  uint32_t               mul;
  uint32_t               mdiv;
  uint32_t               unit;
  int                    moveu;
  uint32_t               column_spacing;
  uint32_t               column_width;
  double                 maxval;
  double                *data;
  char                 **measure;
  char                 **umeasure;
  uint32_t              *mval;
  struct image          *image;
  char                   name[64];    /* user-definable name */
  char                   format[64];
  char                   uformat[64];
};

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
extern int                      graph_log;
extern struct sheap             graph_heap;      /* heap containing graph blocks */
extern struct dheap             graph_data_heap; /* heap containing graph data */
extern struct list              graph_list;      /* list linking graph blocks */
extern uint32_t                 graph_id;
extern int                      graph_dirty;

/* -------------------------------------------------------------------------- *
 * Initialize graph heap and add garbage collect timer.                       *
 * -------------------------------------------------------------------------- */
extern void              graph_init            (void);

/* -------------------------------------------------------------------------- *
 * Destroy graph heap and cancel timer.                                       *
 * -------------------------------------------------------------------------- */
extern void              graph_shutdown        (void);

/* -------------------------------------------------------------------------- *
 * Garbage collect                                                            *
 * -------------------------------------------------------------------------- */
extern int               graph_collect         (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              graph_default         (struct graph  *iptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct graph     *graph_new             (const char    *name,
                                                uint16_t       width,
                                                uint16_t       height,
                                                int            type);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              graph_source_add      (struct graph  *graph,
                                                int            measure,
                                                int            type,
                                                void          *data,
                                                const char    *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              graph_drain_add       (struct graph  *graph,
                                                int            type);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              graph_colorize        (struct graph  *graph, 
                                                int            how);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              graph_drain_render    (struct graph  *graph, 
                                                int            i);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              graph_drain_save      (struct graph  *graph, 
                                                int            i);    

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              graph_delete          (struct graph  *iptr);

/* -------------------------------------------------------------------------- *
 * Loose all references                                                       *
 * -------------------------------------------------------------------------- */
extern void              graph_release         (struct graph  *iptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              graph_set             (struct graph  *graph,
                                                uint32_t       resolution,
                                                uint32_t       samples,
                                                uint32_t       mdiv,
                                                uint32_t       unit);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              graph_feed            (struct graph  *graph,
                                                time_t         t, 
                                                double         value);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              graph_calc            (struct graph  *graph);  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              graph_draw            (struct graph  *graph);  

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              graph_set_name        (struct graph  *iptr,
                                                const char    *name);
 
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern const char       *graph_get_name        (struct graph  *iptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct graph     *graph_find_name       (const char    *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct graph     *graph_find_id         (uint32_t       id);

/* -------------------------------------------------------------------------- *
 * Dump graphers and graph heap.                                              *
 * -------------------------------------------------------------------------- */
extern void              graph_dump            (struct graph  *iptr);

#endif /* LIB_GRAPH_H */
