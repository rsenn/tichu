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
 * $Id: graph.c,v 1.61 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/mem.h>
#include <libchaos/graph.h>
#include <libchaos/log.h>
#include <libchaos/str.h>
#include <libchaos/image.h>
#include <libchaos/timer.h>

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int                graph_log; 
struct sheap       graph_heap;        /* heap containing graph blocks */
struct dheap       graph_data_heap;  
struct list        graph_list;        /* list linking graph blocks */
uint32_t           graph_id;
int                graph_dirty;
uint32_t           graph_split[] = { 40, 20, 10, 5, 4, 2, 1 };
    
/* -------------------------------------------------------------------------- *
 * Initialize graph heap and add garbage collect timer.                       *
 * -------------------------------------------------------------------------- */
void graph_init(void)
{
  graph_log = log_source_register("graph");
  
  dlink_list_zero(&graph_list);
  
  graph_id = 0;
  graph_dirty = 0;
  
  mem_static_create(&graph_heap, sizeof(struct graph), GRAPH_BLOCK_SIZE);
  mem_static_note(&graph_heap, "graph block heap");
  mem_dynamic_create(&graph_data_heap, sizeof(double) * 32768);
  mem_dynamic_note(&graph_data_heap, "graph data heap");

  log(graph_log, L_status, "Initialized [graph] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy graph heap                                                         *
 * -------------------------------------------------------------------------- */
void graph_shutdown(void)
{
  struct graph *iptr;
  struct graph *next;
  
  /* Report status */
  log(graph_log, L_status, "Shutting down [graph] module...");
  
  /* Remove all graph blocks */
  dlink_foreach_safe(&graph_list, iptr, next)
  {
    if(iptr->refcount)
      iptr->refcount--;
    
    graph_delete(iptr);
  }
  
  mem_dynamic_destroy(&graph_data_heap);
  mem_static_destroy(&graph_heap);
  
  /* Unregister log source */
  log_source_unregister(graph_log);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void graph_default(struct graph *graph)
{
  dlink_node_zero(&graph->node);
  
  strcpy(graph->name, "default");
  graph->id = 0;
  graph->refcount = 0;
  graph->hash = 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct graph *graph_new(const char *name, uint16_t width, uint16_t height, int type)
{
  struct graph *graph;
  
  graph = mem_static_alloc(&graph_heap);
  
  graph->id = graph_id++;
  graph->refcount = 1;
  graph->width = width;
  graph->height = height;
  graph->image = image_new(IMAGE_TYPE_8, graph->width, graph->height);  
  
  graph->column_spacing = GRAPH_COLUMN_SPACING;
  graph->column_width = GRAPH_COLUMN_WIDTH;
  
  strlcpy(graph->name, name, sizeof(graph->name));
  
  graph->hash = strhash(graph->name);
  
  dlink_add_tail(&graph_list, &graph->node, graph);

  log(graph_log, L_status, "Added graph block: %s", graph->name);
  
  return graph;
}
     
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void graph_delete(struct graph *graph)
{
  image_delete(graph->image);
  graph->image = NULL;
  
  log(graph_log, L_status, "Deleting graph block: %s", graph->name);
 
  dlink_delete(&graph_list, &graph->node);
  
  mem_static_free(&graph_heap, graph);
}

/* -------------------------------------------------------------------------- *
 * Loose all references                                                       *
 * -------------------------------------------------------------------------- */
void graph_release(struct graph *iptr)
{
  graph_dirty = 1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void graph_set(struct graph *graph, uint32_t resolution,
               uint32_t samples, uint32_t mdiv, uint32_t unit)
{
  uint32_t width, i;
  
  graph->resolution = resolution;
  graph->samples = samples;
  graph->unit = unit;
  
  graph->current = time(NULL);
  graph->current -= graph->current % resolution;
  graph->mdiv = mdiv;
  
  
  graph->end = graph->current - (long)(resolution * (samples - 1));

/*  log(graph_log, L_verbose, "#%u: %u - %u (%u)",
      graph->id, graph->end, graph->current, time(NULL));*/
  
  graph->maxval = 0.0;
  
  /* calculate sizes */
  width = graph->width - (GRAPH_PADDING_LEFT + GRAPH_PADDING_RIGHT);
  
/*  width += graph->samples - 1;
  width /= graph->samples;*/
  
  if(width < graph->column_width + graph->column_spacing)
    width = graph->column_width + graph->column_spacing;
  
  samples = width / (graph->column_width + graph->column_spacing);
  
  if(samples < graph->samples)
    graph->samples = samples;
  
//  width *= graph->samples;

  graph->width = width + GRAPH_PADDING_LEFT + GRAPH_PADDING_RIGHT;
  
  graph->grid_width = width;
  
  graph->usamples = (graph->samples + 1) * (graph->resolution + 1) / graph->unit;
  graph->usamples++;
  
  graph->data = mem_dynamic_alloc(&graph_data_heap, sizeof(double) * graph->samples);
  
  for(i = 0; i < graph->samples; i++)
    graph->data[i] = 0.0;
  
  graph->measure = mem_dynamic_alloc(&graph_data_heap, sizeof(char *) * graph->samples);
  graph->umeasure = mem_dynamic_alloc(&graph_data_heap, sizeof(char *) * graph->usamples);
  graph->mval = mem_dynamic_alloc(&graph_data_heap, sizeof(uint32_t) * graph->samples);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void graph_feed(struct graph *graph, time_t t, double value)
{
  uint32_t index;
  
//  log(graph_log, L_verbose, "value: %u time: %u", (uint32_t)value, (uint32_t)time(NULL));
  
  if(t > graph->current || t < graph->end)
  {
    log(graph_log, L_verbose, "Time mismatch: %u (Should be %u - %u) (Time: %u)",
        t, graph->end, graph->current, time(NULL));
        
    
    return;
  }
  
  index = graph->current - (t - (t % graph->resolution));
  index = (index / graph->resolution) % graph->samples;
  
  graph->data[index] = value;
  
  if(value > graph->maxval)
  {
    
    graph->maxval = value;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void graph_calc(struct graph *graph)
{
  long value, tmul, tval;
  uint32_t i;
  uint32_t t;
  uint32_t diff;
  uint32_t height;
  struct tm tm;
  struct tm g_tm;
  char m[64];
  
  /* calculate range */
  value = (long)graph->maxval;
  
  for(tval = 10, tmul = 2; tval < value; tval *= tmul)
  {
    if(tmul == 2)
      tmul = 5;
    else
      tmul = 2;
  }

  log(graph_log, L_verbose, "Max. value is: %u, range is: %u", 
      (uint32_t)value, (uint32_t)tval);
  
  if(tval > 100)
  {
    while(tval / 10 > (long)graph->maxval)
      tval /= 10;
    while(tval / 5 > (long)graph->maxval)
      tval /= 5;
    while(tval / 2 > (long)graph->maxval)
      tval /= 2;
  }
  
  graph->maxval = (double)tval;

  tmul = tval;
  
  while(tmul && !(tmul % 10) && tmul > 100)
  {
    tmul /= 10;
  }

  tval = tmul;
  
  for(tval = tmul, tmul = 5; tval <= 10; tval *= tmul)
  {
    if(tmul == 5)
      tmul = 2;
    else
      tmul = 5;
  }
  
  graph->mul = tval;
  
  height = graph->height - (GRAPH_PADDING_TOP + GRAPH_PADDING_BOTTOM);
  
  height += tmul - 1;
  height /= tmul;
  height *= tmul;
  
  graph->height = height + GRAPH_PADDING_TOP + GRAPH_PADDING_BOTTOM;
  graph->grid_height = height;
    
  t = graph->current;// % (graph->unit);

/*  log(graph_log, L_verbose, "#%u: %u - %u (%u)",
      graph->id, graph->end, graph->current, time(NULL));*/
  
/*  t = time(NULL);
  tm = gmtime((const time_t *)(void *)&t);
  strftime(m, sizeof(m), "%H:%M:%S", tm);
   */
  /* ugly hack for month */
  if(graph->resolution == 2629800)
   t += graph->resolution / 2;
  
  for(i = 0; i < graph->samples; i++)
  {
#ifdef FIXME
    localtime_r((time_t *)(void *)&t, &tm);
    gmtime_r((time_t *)(void *)&t, &g_tm);
#endif /* WIN32 */
    diff = (tm.tm_hour - g_tm.tm_hour) * 3600 + (tm.tm_min - g_tm.tm_min) * 60;
    
    timer_strftime(m, sizeof(m), graph->format, &tm);
//    str_snprintf(m, sizeof(m), "%u", tm.tm_sec);
//    
    graph->measure[i] = mem_dynamic_alloc(&graph_data_heap, strlen(m) + 1);
    
    if(isdigit(m[0]))
      graph->mval[i] = (uint32_t)strtoul(m, NULL, 10);
    else
      graph->mval[i] = ((t + diff) % graph->unit) / graph->mdiv;
    
    strcpy(graph->measure[i], m);
    
    t -= graph->resolution;
  }
  
  t = graph->current;
  
  for(i = 0; i < graph->usamples; i++)
  {
#ifdef FIXME
    localtime_r((time_t *)(void *)&t, &tm);
    gmtime_r((time_t *)(void *)&t, &g_tm);
#endif /* WIN32 */
    diff = (tm.tm_hour - g_tm.tm_hour) * 3600 + (tm.tm_min - g_tm.tm_min) * 60;
    
    timer_strftime(m, sizeof(m), graph->uformat, &tm);
//    str_snprintf(m, sizeof(m), "%u", tm.tm_sec);
    
    log(graph_log, L_status, "umeasure: %s", m);
    
    graph->umeasure[i] = mem_dynamic_alloc(&graph_data_heap, strlen(m) + 1);
    
    strcpy(graph->umeasure[i], m);
    
    t -= graph->unit;    
  }  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void graph_draw_grid(struct graph *graph)
{
  int16_t x1, y1, x2, y2;
  uint32_t i;
  uint32_t val = 0;
  uint32_t split = 1;
  uint32_t space;
  uint32_t lastgrid = 0;
  uint32_t column_width;
  uint32_t column_spacing;
  
  x1 = GRAPH_PADDING_LEFT;
  y1 = GRAPH_PADDING_TOP;
  x2 = graph->width - GRAPH_PADDING_RIGHT;
  y2 = graph->height - GRAPH_PADDING_BOTTOM;
  
  for(i = 0; i < sizeof(graph_split) / sizeof(graph_split[0]); i++)
  {
    val = (uint32_t)graph->maxval;
    
    if((val % graph_split[i]) == 0 && (graph->grid_height / graph_split[i]) >= GRAPH_GRID_MINSPACE)
    {
      split = graph_split[i];
      break;
    }
  }
  
  space = val / split;
  
  log(graph_log, L_status, "splitting in %u parts, spacing %u pixels", split, space);
  
  /* zeichnet die vertikal unterteilig */
  for(i = 0; i <= graph->grid_height; i++)
  {
    val = i * graph->maxval / graph->grid_height;
    
    if(((val % ((uint32_t)graph->maxval / split)) == 0 &&
       i - lastgrid >= GRAPH_GRID_MINSPACE) ||
       val == (uint32_t)graph->maxval || (val == 0 && i == 0))
    {
      if(val != (uint32_t)graph->maxval && val != 0)
        image_puthline(graph->image, x1 - GRAPH_GRID_GRID, x2, y2 - i, GRAPH_COLOR_GRID);
      
/*      image_putnum(graph->image, &image_font_6x10, 
                   x1 - GRAPH_GRID_GRID - 5, y2 - i - 5, 
                   GRAPH_COLOR_TEXT, IMAGE_ALIGN_RIGHT, val);*/
      
      lastgrid = i;
    }
  }
  
  column_width = graph->column_width;
  column_spacing = graph->column_spacing;

  /* zeichnet die horizontal unterteilig */
  for(i = 0; i < graph->samples; i++)
  {
    x1 += column_spacing / 2 + column_width / 2;
    
    image_putvline(graph->image, 
                   x1, y2, y2 + GRAPH_GRID_GRID, 
                   GRAPH_COLOR_GRID);

/*    image_putstr(graph->image, &image_font_6x10, 
                 x1, y2 + GRAPH_GRID_MEASURE, 
                 GRAPH_COLOR_TEXT, IMAGE_ALIGN_CENTER, graph->measure[i]);*/
    
    x1 += column_spacing / 2 + column_width / 2;
    
    if(i < graph->samples - 1 && graph->mval[i + 1] > graph->mval[i])
    {
      int16_t xpos1, xpos2;
      uint16_t w1, w2;
      uint32_t index;
      
      image_putvline(graph->image, x1, y1, y2, GRAPH_COLOR_GRID);
     
      w1 = (graph->column_width + graph->column_spacing) * ((graph->unit / graph->resolution) + 1) / 2;
      w2 = (graph->column_width + graph->column_spacing) * ((graph->unit / graph->resolution) - 1) / 2;
      
      xpos1 = x1 - w1;
      xpos2 = x1 + w2;
      
      index = i * graph->resolution / graph->unit;
      
      if(x1 - (w1 * 2) <= GRAPH_PADDING_LEFT)
        xpos1 = ((x1 - GRAPH_PADDING_LEFT) / 2) + GRAPH_PADDING_LEFT;
/*      image_putstr(graph->image, &image_font_8x13b, 
                   xpos1, y2 + GRAPH_GRID_MEASURE * 4,
                   GRAPH_COLOR_TEXT, IMAGE_ALIGN_CENTER, graph->umeasure[index]);
      image_putstr(graph->image, &image_font_8x13b, 
                   xpos1 + 1, y2 + GRAPH_GRID_MEASURE * 4 + 1,
                   GRAPH_COLOR_TEXT, IMAGE_ALIGN_CENTER, graph->umeasure[index]);
      image_putstr(graph->image, &image_font_8x13b, 
                   xpos1, y2 + GRAPH_GRID_MEASURE * 4,
                   GRAPH_COLOR_UTEXT, IMAGE_ALIGN_CENTER, graph->umeasure[index]);*/
/*        image_putstr(graph->image, &image_font_8x13, 
                     x1 - xpos1 - 1, y2 + GRAPH_GRID_MEASURE * 4 - 1,
                     GRAPH_COLOR_DATA, IMAGE_ALIGN_CENTER, graph->umeasure[index]);*/
      
      if(i + 1 < graph->samples)
      {
        if(x1 + (w2 * 2) >= graph->width - GRAPH_PADDING_RIGHT)
          xpos2 = x1 + ((graph->width - GRAPH_PADDING_RIGHT) - x1) / 2;
        
/*        image_putstr(graph->image, &image_font_8x13b, 
                     xpos2, y2 + GRAPH_GRID_MEASURE * 4,
                     GRAPH_COLOR_TEXT, IMAGE_ALIGN_CENTER, graph->umeasure[index + 1]);
        image_putstr(graph->image, &image_font_8x13b, 
                     xpos2 + 1, y2 + GRAPH_GRID_MEASURE * 4 + 1,
                     GRAPH_COLOR_TEXT, IMAGE_ALIGN_CENTER, graph->umeasure[index + 1]);
        image_putstr(graph->image, &image_font_8x13b, 
                     xpos2, y2 + GRAPH_GRID_MEASURE * 4,
                     GRAPH_COLOR_UTEXT, IMAGE_ALIGN_CENTER, graph->umeasure[index + 1]);
        image_putstr(graph->image, &image_font_8x13, 
                     x1 + xpos2 - 1, y2 + GRAPH_GRID_MEASURE * 4 - 1,
                     GRAPH_COLOR_DATA, IMAGE_ALIGN_CENTER, graph->umeasure[index + 1]);*/
      }
      
      log(graph_log, L_status, "draw umeasure: %u %s", xpos1, graph->umeasure[index]);
    }
  }
  
  x1 = GRAPH_PADDING_LEFT;
  
  /* zeichnet d x achsä */
  image_puthline(graph->image, x1 - GRAPH_GRID_MEASURE, x2,
                 y2, GRAPH_COLOR_BORDER);
  
  /* zeichnet d y achsä */
  image_putvline(graph->image, x1, y1,
                 y2 + GRAPH_GRID_MEASURE, GRAPH_COLOR_BORDER);

  /* zeichnet ä chlinä querbaukä am rächtä änd vor x achsä */
  image_putvline(graph->image, x2, 
                 y2 - GRAPH_GRID_MEASURE,
                 y2 + GRAPH_GRID_MEASURE,
                 GRAPH_COLOR_BORDER);
  
  /* zeichnet ä chlinä querbaukä am oberä änd vor y achsä */
  image_puthline(graph->image, 
                 x1 - GRAPH_GRID_MEASURE,
                 x1 + GRAPH_GRID_MEASURE, 
                 y1, GRAPH_COLOR_BORDER);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void graph_draw_columns(struct graph *graph)
{
  int16_t x1, y1, x2, y2;
  int i;
  struct rect rect;
  
  x1 = GRAPH_PADDING_LEFT;
  y1 = GRAPH_PADDING_TOP;
  x2 = graph->width - GRAPH_PADDING_RIGHT;
  y2 = graph->height - GRAPH_PADDING_BOTTOM;
  
//  column_width = (graph->grid_width / graph->samples) - graph->column_spacing;
  for(i = 0; i < graph->samples; i++)
  {
    uint16_t height;
    
    height = graph->data[i] * graph->grid_height / graph->maxval;
    
    x1 += graph->column_spacing / 2;
    
    if(height > 1)
    {
      rect.x = x1 + 1;
      rect.y = y2 - height + 1;
      rect.w = graph->column_width - 2;
      rect.h = height - 2;
      
      image_putfrect(graph->image, &rect, GRAPH_COLOR_DATA);
    }
    
    rect.x = x1;
    rect.y = y2 - height;
    rect.w = graph->column_width;
    rect.h = height;
    
    image_putrect(graph->image, &rect, GRAPH_COLOR_BORDER);
    
    x1 += graph->column_width + graph->column_spacing / 2;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void graph_draw(struct graph *graph)
{
  char filename[64];
  
  graph_draw_grid(graph);
  graph_draw_columns(graph);
  
  strlcpy(filename, graph->name, sizeof(filename));
  strlcat(filename, ".gif", sizeof(filename));
  
  image_save_gif(graph->image, filename);  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void graph_set_name(struct graph *graph, const char *name)
{
  strlcpy(graph->name, name, sizeof(graph->name));
  
  graph->hash = strihash(graph->name);
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
const char *graph_get_name(struct graph *graph)
{
  return graph->name;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct graph *graph_find_name(const char *name)
{
  struct node   *node;
  struct graph *graph;
  uint32_t       hash;
  
  hash = strihash(name);
  
  dlink_foreach(&graph_list, node)
  {
    graph = node->data;
    
    if(graph->hash == hash)
    {
      if(!stricmp(graph->name, name))
        return graph;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct graph *graph_find_id(uint32_t id)
{
  struct graph *iptr;
  
  dlink_foreach(&graph_list, iptr)
  {
    if(iptr->id == id)
      return iptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Dump graphs and graph heap.                                                *
 * -------------------------------------------------------------------------- */
void graph_dump(struct graph *gptr)
{
  if(gptr == NULL)
  {
    dump(graph_log, "[============== graph summary ===============]");
    
    dlink_foreach(&graph_list, gptr)
      dump(graph_log, " #%03u: [%u] (%u/%u) %-20s",
           gptr->id, 
           gptr->refcount,
           gptr->resolution,
           gptr->samples,
           gptr->name);
    
    dump(graph_log, "[========== end of graph summary ============]");
  }
  else
  {
    dump(graph_log, "[============== graph dump ===============]");
    dump(graph_log, "         id: #%u", gptr->id);
    dump(graph_log, "   refcount: %u", gptr->refcount);
    dump(graph_log, "       hash: %p", gptr->hash);
    dump(graph_log, "       name: %s", gptr->name);
    dump(graph_log, " resolution: %s", gptr->resolution);
    dump(graph_log, "    samples: %s", gptr->samples);

    dump(graph_log, "[========== end of graph dump ============]");    
  }
}
