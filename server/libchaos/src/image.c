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
 * $Id: image.c,v 1.25 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

#include <math.h>

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>
#include <libchaos/mem.h>
#include <libchaos/image.h>
#include <libchaos/log.h>
#include <libchaos/str.h>
#include <libchaos/gif.h>

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int                image_log; 
struct sheap       image_heap;       /* heap containing image blocks */
struct dheap       image_palette_heap; 
struct dheap       image_data_heap;  /* heap containing image data */
struct list        image_list;       /* list linking image blocks */
uint32_t           image_id;
int                image_dirty;
#include <libchaos/image_defpal.h>

/* bitmap fonts (not ttf!) */
/*#include "font_6x10.h"
#include "font_8x13.h"
#include "font_8x13b.h"*/

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static inline int image_bit_size(int n)
{
  int i;

  for(i = 1; i <= 8; i++)
  {
    if((1 << i) >= n)
      break;
  }

  return i;
}
 
/* -------------------------------------------------------------------------- *
 * Initialize image heap and add garbage collect timer.                       *
 * -------------------------------------------------------------------------- */
void image_init(void)
{
  image_log = log_source_register("image");
  
  dlink_list_zero(&image_list);
  
  image_id = 0;
  image_dirty = 0;
  
  mem_static_create(&image_heap, sizeof(struct image), IMAGE_BLOCK_SIZE);
  mem_static_note(&image_heap, "image block heap");
  
  mem_dynamic_create(&image_data_heap, 1024 * 1024);
  mem_dynamic_note(&image_data_heap, "image data heap");
  
  mem_dynamic_create(&image_palette_heap, (sizeof(struct palette) +
                                           sizeof(struct color) * 256) *
                                          IMAGE_BLOCK_SIZE * 2);
  mem_dynamic_note(&image_palette_heap, "image palette heap");

  log(image_log, L_status, "Initialized [image] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy image heap                                                         *
 * -------------------------------------------------------------------------- */
void image_shutdown(void)
{
  struct image *iptr;
  struct image *next;
  
  /* Report status */
  log(image_log, L_status, "Shutting down [image] module...");
  
  /* Remove all image blocks */
  dlink_foreach_safe(&image_list, iptr, next)
  {
    if(iptr->refcount)
      iptr->refcount--;

    image_delete(iptr);
  }

  mem_static_destroy(&image_heap);
  mem_dynamic_destroy(&image_data_heap);
  mem_dynamic_destroy(&image_palette_heap);
    
  /* Unregister log source */
  log_source_unregister(image_log);
}

/* -------------------------------------------------------------------------- *
 * Collect image block garbage.                                              *
 * -------------------------------------------------------------------------- */
int image_collect(void)
{
  struct image *cnptr;
  struct image *next;
  size_t         n = 0;
  
  if(image_dirty)
  {
    /* Report verbose */
    log(image_log, L_verbose, "Doing garbage collect for [image] module.");
    
    /* Free all image blocks with a zero refcount */
    dlink_foreach_safe(&image_list, cnptr, next)
    {
      if(!cnptr->refcount)
      {
        image_delete(cnptr);
        
        n++;
      }
    }
  
    /* Collect garbage on image_heap */
    mem_static_collect(&image_heap);
    
    image_dirty = 0;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_default(struct image *image)
{
  dlink_node_zero(&image->node);
  
  strcpy(image->name, "default");
  image->id = 0;
  image->refcount = 0;
  image->hash = 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_rect_clip(struct image *image, struct rect *rect)
{
  if(rect->x < 0)
  {
    rect->w -= -rect->x;
    rect->x = 0;
  }
  if(rect->x >= image->w)
  {
    rect->w = 0;
    rect->x = image->w;
  }
  
  if(rect->y < 0)
  {
    rect->h -= -rect->y;
    rect->y = 0;
  }
  
  if(rect->y >= image->h)
  {
    rect->h = 0;
    rect->y = image->h;
  }
  
  if(rect->x + rect->w >= image->w)
    rect->w = image->w - rect->x;
  
  if(rect->y + rect->h >= image->h)
    rect->h = image->h - rect->y;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_rect_unify(struct rect *a, struct rect *b)
{
  if(a->w > b->w)
    a->w = b->w;
  else
    b->w = a->w;
  
  if(a->h > b->h)
    a->h = b->h;
  else
    b->h = a->h;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_blit_32to32(struct image *src, struct rect *srect, 
                       struct image *dst, struct rect *drect)
{
  struct color *srow;
  struct color *drow;
  uint16_t      y;
  uint16_t      x;
  struct rect   sr;
  struct rect   dr;
  
  sr = (srect == NULL ? src->rect : *srect);
  dr = (drect == NULL ? dst->rect : *drect);
  
  image_rect_clip(src, &sr);
  image_rect_clip(dst, &dr);
  image_rect_unify(&sr, &dr);
  
/*  image_rect_dump(&sr);
  image_rect_dump(&dr);*/
  
  srow = (struct color *)&src->pixel.data32[((sr.y * src->pitch) >> 2) + sr.x];
  drow = (struct color *)&dst->pixel.data32[((dr.y * dst->pitch) >> 2) + dr.x];
  
  for(y = 0; y < sr.h; y++)
  {
    for(x = 0; x < sr.w; x++)
    {
      drow[x].r = (srow[x].r * srow[x].a / 255) + (drow[x].r * (255 - srow[x].a) / 255);
      drow[x].g = (srow[x].g * srow[x].a / 255) + (drow[x].g * (255 - srow[x].a) / 255);
      drow[x].b = (srow[x].b * srow[x].a / 255) + (drow[x].b * (255 - srow[x].a) / 255);
    }
    
    srow += src->pitch >> 2;
    drow += dst->pitch >> 2;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *image_new(int type, uint16_t width, uint16_t height)
{
  struct image *image;
//  uint32_t      i;
  
  image = mem_static_alloc(&image_heap);
  
  image->id = image_id++;
  image->refcount = 1;
  image->w = width;
  image->pitch = image->w;
  image->h = height;
  
  image->rect.x = 0;
  image->rect.y = 0;
  image->rect.w = image->w;
  image->rect.h = image->h;
  image->colorkey = -1;
  
  image->type = (type == IMAGE_TYPE_8 ? IMAGE_TYPE_8 : IMAGE_TYPE_32);
  
  switch(image->type)
  {
    case IMAGE_TYPE_8:
    {
      image->bpp = 8;
      image->opp = 1;
      break;
    }
    case IMAGE_TYPE_32:
    {
      image->bpp = 32;
      image->opp = 4;
      image->pitch *= 4;
      break;
    }
  }
    
  image->pixel.data = mem_dynamic_alloc(&image_data_heap,
                                        image->opp * image->w * image->h);
  
  dlink_add_tail(&image_list, &image->node, image);

  memcpy(image->palette, image_defpal, sizeof(image->palette));
  
  log(image_log, L_status, "Added image block %ux%ux%u", 
      image->w, image->h, image->bpp);
  
  return image;
}
     
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_delete(struct image *image)
{
  log(image_log, L_status, "Deleting image block: %s", image->name);
 
/*  for(i = 0; i < image->h; i++)
    mem_dynamic_free(&image_data_heap, image->pixel.data[i]);*/
  
  mem_dynamic_free(&image_data_heap, image->pixel.data);
  
  dlink_delete(&image_list, &image->node);
  
  mem_static_free(&image_heap, image);
}

/* -------------------------------------------------------------------------- *
 * Loose all references                                                       *
 * -------------------------------------------------------------------------- */
void image_release(struct image *iptr)
{
  image_dirty = 1;
}


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_putpixel(struct image *iptr, int16_t  x,
                    int16_t       y,    uint32_t c)
{
  if(x < 0 || x >= iptr->w ||
     y < 0 || y >= iptr->h)
    return;
  
  if(iptr->type == IMAGE_TYPE_8)
    iptr->pixel.data8[y * iptr->pitch + x] = c;
  else
    iptr->pixel.data32[y * (iptr->pitch >> 2) + x] = c;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static uint32_t image_pitch(struct image *iptr)
{
  int      pixelsize = (iptr->type == IMAGE_TYPE_32) ? 4 : 1;
  uint32_t width;
  uint32_t mask = 1;
  
  width = pixelsize * iptr->w;
  
  while((width & mask) != width)
  {
    mask <<= 1;
    mask |= 1;
  }
  
  mask += 1;
  
  return mask;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void image_alloc(struct image *iptr)
{
  iptr->pitch = image_pitch(iptr);
  
  iptr->pixel.data = mem_dynamic_alloc(&image_data_heap, iptr->pitch * iptr->h);
}

/* -------------------------------------------------------------------------- *
 * octtree based quantization                                                 *
 * -------------------------------------------------------------------------- */
#define COLOR_TO_KEY(c, shift) \
(((((c)->r >> (shift)) & 1) << 0) | \
 ((((c)->g >> (shift)) & 1) << 1) | \
 ((((c)->b >> (shift)) & 1) << 2)) 
    
struct palette *image_quantize(struct image *iptr, int maxcolors, int *ckey)
{
  struct ctree    octtree;
  struct palette *palette;
  int16_t         x;
  int16_t         y;
  uint32_t       *row;
  int             bitcount;
  struct ctree   *current;
  uint32_t        counts[8];
  int             depth = 0;
  struct list     levels[8];
  int             colorkey = -1;
  struct node    *node;
  int             i;
  
  row = iptr->pixel.data32;
  
  memset(&octtree, 0, sizeof(struct ctree));
  memset(&counts, 0, sizeof(counts));
  memset(&levels, 0, sizeof(levels));
  
  if(maxcolors > 256)
    maxcolors = 256;
  
  for(y = 0; y < iptr->h; y++)
  {
    for(x = 0; x < iptr->w; x++)
    {
      struct color *c = (struct color *)&row[x];
      
      current = &octtree;
      
      if(c->a < 128)
      {
        if(maxcolors == 256)
          maxcolors--;
        
        colorkey = 255;
        
        continue;
      }
      
      for(bitcount = 7; bitcount >= 0; bitcount--)
      {
        int key = COLOR_TO_KEY(c, bitcount);
        
        if(current->children[key] == NULL)
        {
          struct node *node;
          
          if(++counts[bitcount] > maxcolors)
          {
            dlink_destroy(&levels[depth]);
            depth++;
            
            log(image_log, L_warning, "Color count exceeded, cropping to %u bits per channel.",
                8 - depth);            
            break;
          }
            
          current->children[key] = mem_dynamic_alloc(&image_data_heap, sizeof(struct ctree));
          memset(current->children[key], 0, sizeof(struct ctree));
          current->children[key]->parent = current;
          
          node = dlink_node_new();
          dlink_add_tail(&levels[bitcount], node, current->children[key]);
          
/*          if(bitcount == 0)
            image_color_dump(c);*/
        }
        
        current->children[key]->key = key;
        current->children[key]->color.r = c->r;
        current->children[key]->color.g = c->g;
        current->children[key]->color.b = c->b;
        current->children[key]->color.a = c->a;
        current->children[key]->count++;
        
                             
        current = current->children[key];
        
        if(bitcount <= depth)
          break;
      }
    }
    
    row += iptr->pitch >> 2;
  }
  
  
  log(image_log, L_status, "Counted %u colors total while quantizing image %s (level %u).", counts[depth], iptr->name, depth);

  palette = image_palette_new(counts[depth]);
  
  i = 0;
  
  if(colorkey >= 0)
  {
    palette->colors[colorkey].r = 0xff;
    palette->colors[colorkey].g = 0x7f;
    palette->colors[colorkey].b = 0x7f;
    palette->colors[colorkey].a = 0x00;
  }
  
  dlink_foreach_data(&levels[depth], node, current)
  {
    if(i == colorkey)
      i++;
    
    palette->colors[i].r = current->color.r;
    palette->colors[i].g = current->color.g;
    palette->colors[i].b = current->color.b;
    palette->colors[i++].a = current->color.a;
//    image_color_dump(&palette->colors[i]);
  }
  
  for(; i < palette->count; i++)
  {
    palette->colors[i].r = 0;
    palette->colors[i].g = 0;
    palette->colors[i].b = 0;
    
    if(i == colorkey)
      palette->colors[i].a = 0x00;
    else
      palette->colors[i].a = 0xff;
  }
  
  log(image_log, L_status, "Created a %u-color palette for image %s.", palette->count, iptr->name);

  if(ckey)
    *ckey = colorkey;
  
  /* free the whole tree */
  for(i = 0; i < 8; i++)
  {
    dlink_foreach_safe(&levels[i], node, row)
      mem_dynamic_free(&image_data_heap, node->data);
    
    dlink_destroy(&levels[i]);
  }
    
  return palette;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
/*static*/ void image_8to32(struct image *iptr)
{
  uint8_t  *olddata;
  uint32_t  oldpitch;
  uint16_t  x;
  uint16_t  y;
  uint8_t  *srow;
  uint32_t *drow;
  uint8_t   alpha;
  struct color c;

  if(iptr->type == IMAGE_TYPE_32)
    return;
  
  olddata = iptr->pixel.data8;
  oldpitch = iptr->pitch;
  
  iptr->type = IMAGE_TYPE_32;
  
  image_alloc(iptr);
  
  srow = olddata;
  drow = iptr->pixel.data32;
  
  for(y = 0; y < iptr->h; y++)
  {
    for(x = 0; x < iptr->w; x++)
    {
      if((int)(unsigned int)srow[x] == iptr->colorkey)
        alpha = 0;
      else
        alpha = 255;
      
      c = iptr->palette[(uint32_t)srow[x]];
      c.a = alpha;
      
      drow[x] = *(uint32_t *)&c;
    }
    
    srow += oldpitch;
    drow += iptr->pitch >> 2;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
/*static*/ void image_32to8(struct image *iptr, struct palette *palette, int colorkey)
{
  struct color *c;
  uint32_t     *olddata;
  uint32_t      oldpitch;
  uint16_t      x;
  uint16_t      y;
  uint32_t     *srow;
  uint8_t      *drow;

  if(iptr->type == IMAGE_TYPE_8)
    return;
  
  olddata = iptr->pixel.data32;
  oldpitch = iptr->pitch;
  
  iptr->type = IMAGE_TYPE_8;
  
  image_alloc(iptr);
  
  srow = olddata;
  drow = iptr->pixel.data8;
  
  if(colorkey > 0xff)
    colorkey &= 0xff;
  
  for(y = 0; y < iptr->h; y++)
  {
    for(x = 0; x < iptr->w; x++)
    {
      c = (struct color *)&srow[x];

      drow[x] = image_palette_match(palette, c, colorkey);
    }
    
    srow += oldpitch >> 2;
    drow += iptr->pitch;
  }
  
  image_palette_set(iptr, palette);
  iptr->colorkey = colorkey;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_convert(struct image *iptr, int type)
{
  if(type == IMAGE_TYPE_8 && iptr->type == IMAGE_TYPE_32)
  {
    int colorkey = -1;
    struct palette *palette;
    
    palette = image_quantize(iptr, 256, &colorkey);
    
    image_32to8(iptr, palette, colorkey);
  }

  
  if(type == IMAGE_TYPE_32 && iptr->type == IMAGE_TYPE_8)
  {
    image_8to32(iptr);
  }
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_clear(struct image *iptr, uint32_t c)
{
  uint32_t y;
  uint32_t x;
  
  if(iptr->type == IMAGE_TYPE_8)
  {
    memset(iptr->pixel.data8, c, iptr->pitch * iptr->h);
  }
  else
  {
    for(y = 0; y < iptr->h; y++)
      for(x = 0; x < iptr->w; x++)
        iptr->pixel.data32[y * (iptr->pitch >> 2) + x] = c;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_putindex(struct image *iptr, int16_t x,
                    int16_t       y,    uint8_t i)
{
  if(x < 0 || x >= iptr->w ||
     y < 0 || y >= iptr->h)
    return;
  
  if(iptr->type == IMAGE_TYPE_8)
    iptr->pixel.data8[y * iptr->pitch + x] = i;
  else
    iptr->pixel.data32[y * (iptr->pitch >> 2) + x] = *(uint32_t *)&iptr->palette[i];
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_putcolor(struct image *iptr, int16_t       x,
                    int16_t       y,    struct color *color)
{
  if(x < 0 || x >= iptr->w ||
     y < 0 || y >= iptr->h)
    return;
  
  if(iptr->type == IMAGE_TYPE_8)
    return;

  iptr->pixel.data32[y * (iptr->pitch >> 2) + x] = *(uint32_t *)color;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_puthline(struct image *iptr, int16_t x1, int16_t  x2,
                    int16_t       y,    uint32_t c)
{
  int16_t x;
  
  if(x1 > x2)
  {
    x1 ^= x2;
    x2 ^= x1;
    x1 ^= x2;
  }
  
  for(x = x1; x <= x2; x++)
    image_putpixel(iptr, x, y, c);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_putvline(struct image *iptr, int16_t  x,  int16_t  y1,
                    int16_t       y2,   uint32_t c)
{
  int16_t y;
  
  if(y1 > y2)
  {
    y1 ^= y2;
    y2 ^= y1;
    y1 ^= y2;
  }
  
  for(y = y1; y <= y2; y++)
    image_putpixel(iptr, x, y, c);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_putline(struct image *iptr, int16_t x1, int16_t  y1,
                   int16_t       x2,   int16_t y2, uint32_t c)
{
  int      i;
  int16_t  dx;
  int16_t  dy;
  uint16_t steps;
  double   x;
  double   y;
  double   sx;
  double   sy;
  
  if(x1 == x2)
  {
    image_putvline(iptr, x1, y1, y2, c);
    return;
  }
  
  if(y1 == y2)
  {
    image_puthline(iptr, x1, x2, y1, c);
    return;
  }
  
  steps = 0;
  
  dx = x2 - x1;
  dy = y2 - y1;
  x = x2;
  y = y2;
  
  if(abs(dx) > abs(dy))
  {
    if(dx > 0)
      sx = -1;
    else
      sx = 1;
    
    if(dx == 0)
      sy = 0;
    else
      sy = (double)dy / ((double)(dx * sx));
    
    steps = abs(dx);
  } 
  else 
  {
    if(dy > 0)
      sy = -1;
    else
      sy = 1;
    
    if(dy == 0)
      sx = 0;
    else
      sx = (double)dx / ((double)(dy * sy));
    
    steps = abs(dy);
  }
  
  for(i = 0; i <= steps; i++)
  {
    image_putpixel(iptr, (int)x, (int)y, c);
    x += sx;
    y += sy;
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_putrect(struct image *iptr, struct rect *rect, uint32_t c)
{
  image_puthline(iptr, rect->x, rect->x + rect->w, rect->y, c);
  image_puthline(iptr, rect->x, rect->x + rect->w, rect->y + rect->h, c);
  image_putvline(iptr, rect->x, rect->y, rect->y + rect->h, c);
  image_putvline(iptr, rect->x + rect->w, rect->y, rect->y + rect->h, c);
}

/* -------------------------------------------------------------------------- *
 * Draw a filled rect                                                         *
 * -------------------------------------------------------------------------- */
void image_putfrect(struct image *iptr, struct rect *rect, uint32_t c)
{
  int16_t x, y;
  uint16_t x2, y2;
  
  x2 = rect->x + rect->w;
  y2 = rect->y + rect->h;
  
  for(y = rect->y; y <= y2; y++)
  {
    for(x = rect->x; x <= x2; x++)
    {
      if(iptr->type == IMAGE_TYPE_8)
        iptr->pixel.data8[y * iptr->pitch + x] = c;
      else
        iptr->pixel.data32[y * (iptr->pitch >> 2) + x] = c;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_putcircle(struct image *iptr, int16_t x,     int16_t  y,
                     int           rad,  int     steps, uint32_t c)
{
  int      lastx;
  int      lasty;
  double   sx;
  double   sy;
  double   inc;
  double   angle;
  int      i;
  
  angle = 0;
  inc = M_PI * 2 / steps;
  
  lastx = x;
  lasty = y + rad;
  
  for(i = 0; i < steps; i++)
  {
    angle += inc;
    
    sx = (double)x + sin(angle) * (double)rad;
    sy = (double)y + cos(angle) * (double)rad;
    
    image_putline(iptr, lastx, lasty, (int)sx, (int)sy, c);
    
    lastx = sx;
    lasty = sy;
  }
  
  image_putline(iptr, lastx, lasty, x, y + rad, c);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_putellipse(struct image *iptr, int16_t x,    int16_t  y,
                      int           xrad, int     yrad, int      steps, 
                      uint32_t      c)
{
  int      lastx;
  int      lasty;
  double   sx;
  double   sy;
  double   inc;
  double   angle;
  int      i;
  
  angle = 0;
  inc = M_PI * 2 / steps;
  
  lastx = x;
  lasty = y + yrad;
  
  for(i = 0; i < steps; i++)
  {
    angle += inc;
    
    sx = (double)x + sin(angle) * (double)xrad;
    sy = (double)y + cos(angle) * (double)yrad;
    
    image_putline(iptr, lastx, lasty, (int)sx, (int)sy, c);
    
    lastx = sx;
    lasty = sy;
  }
  
  image_putline(iptr, lastx, lasty, x, y + yrad, c);
}


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_putchar(struct image *iptr, struct font *ifptr, uint16_t x, 
                   uint16_t      y,    uint32_t     c,     char     a)
{
  uint16_t dx;
  uint16_t dy;
  uint32_t row;
  uint32_t col;
  uint8_t *rowptr;
  int      bit;
  
  row = ((uint32_t)((uint8_t)a & 0xf0) >> 4) * ifptr->h;
  col = (uint32_t)((uint8_t)a & 0x0f) * ifptr->w;
  
  for(dy = 0; dy < ifptr->h; dy++)
  {
    for(dx = 0; dx < ifptr->w; dx++)
    {
      rowptr = &ifptr->data[((row + dy) * ifptr->w * 16) >> 3];
      
      bit = rowptr[(col + dx) >> 3];
      
      bit >>= (col + dx) & 0x07;
      bit &= 0x01;
      
      if(bit)
        image_putpixel(iptr, x + dx, y + dy, c);
    }
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_putstr(struct image *iptr, struct font *ifptr, uint16_t x, 
                  uint16_t      y,    uint32_t     c,     int align,
                  char         *s)
{
  size_t len = strlen(s);
  size_t width = len * ifptr->w;
  
  switch(align)
  {
    case IMAGE_ALIGN_CENTER:
    {
      x -= width / 2;
      break;
    }
    case IMAGE_ALIGN_RIGHT:
    {
      x -= width;
      break;
    }
  }
  
  do
  {
    image_putchar(iptr, ifptr, x, y, c, *s);
    x += ifptr->w;
  }
  while(*s++);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_putnum(struct image *iptr, struct font *ifptr, uint16_t x, 
                  uint16_t      y,    uint32_t     c,     int align,
                  int           num)
{
  char numbuf[32];
  
  str_snprintf(numbuf, sizeof(numbuf), "%i", num);
  
  image_putstr(iptr, ifptr, x, y, c, align, numbuf);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int image_save_gif(struct image *iptr, const char *name)
{
  struct gif     *gif;
  struct palette *pal;
  int             fd;
  uint32_t        i;
  
  if(iptr->type != IMAGE_TYPE_8)
  {
    log(image_log, L_warning, "Cannot save 32bit image as GIF.");
    return -1;
  }
  
  fd = io_open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  
  if(fd == -1)
    return -1;
  
  io_queue_control(fd, OFF, OFF, OFF);
  
  if((gif = gif_open_fd(fd, GIF_WRITE)) == NULL)
  {
    io_close(fd);
    return -1;
  }
  
  if((pal = gif_palette_make(256, iptr->palette)) == NULL)
    return -1;
  
  gif_screen_put(gif, iptr->w, iptr->h, iptr->bpp, 0, pal);
  
  gif_put_gfx_control(gif, 0, 0, iptr->colorkey, 0);
  
  gif_image_put(gif, 0, 0, iptr->w, iptr->h, 0, pal);
  
  for(i = 0; i < iptr->h; i++)
    gif_data_put(gif, &iptr->pixel.data8[i * iptr->pitch], iptr->w);
  
  gif_close(gif);
  gif_delete(gif);

  log(image_log, L_verbose, "Saved image %s as %s.", iptr->name, name);
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *image_load_gif(const char *name)
{
  struct gif     *gif;
  struct image   *image;
  int             fd;
  uint32_t        i;
  struct palette *palette;
  
  fd = io_open(name, O_RDONLY);
  
  if(fd == -1)
    return NULL;
  
  io_queue_control(fd, OFF, OFF, OFF);
  
  if((gif = gif_open_fd(fd, GIF_READ)) == NULL)
  {
    io_close(fd);
    return NULL;
  }
  
  if(gif_slurp(gif))
    return NULL;
  
  if((image = image_new(IMAGE_TYPE_8, gif->width, gif->height)))
  {
    image_set_name(image, name);
    palette = gif->palette;
    
    if(gif->images.head)
    {
      int colorkey = -1;      
      struct gif_image *img = gif->images.head->data;
      
      for(i = 0; i < image->h; i++)
        memcpy(&image->pixel.data8[i * image->pitch], &img->bits[i * img->desc.width], img->desc.width);
      
      if(img->desc.palette)
        palette = img->desc.palette;
      
      image_palette_set(image, palette);

      log(image_log, L_status, "Loaded %u-color image.", palette->count);
      
      gif_get_gfx_control(img, NULL, NULL, &colorkey, NULL);
      
      image->colorkey = colorkey;      
    }
  }
  
  gif_close(gif);
  gif_delete(gif);
  
  return image;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *image_pop(struct image *iptr)
{
  if(iptr)
  {
    if(!iptr->refcount)
      log(image_log, L_warning, "Poping deprecated image: %s",
          iptr->name);
    
    iptr->refcount++;
  }

  return iptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *image_push(struct image **iptrptr)
{
  if(*iptrptr)
  {
    if(!(*iptrptr)->refcount)
    {
      log(image_log, L_warning, "Trying to push deprecated image %s",
          (*iptrptr)->name);
    }
    else
    {
      if(--(*iptrptr)->refcount == 0)
        image_release(*iptrptr);
    }
        
    (*iptrptr) = NULL;
  }
    
  return *iptrptr;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_set_name(struct image *image, const char *name)
{
  strlcpy(image->name, name, sizeof(image->name));
  
  image->hash = strihash(image->name);
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
const char *image_get_name(struct image *image)
{
  return image->name;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *image_find_name(const char *name)
{
  struct node   *node;
  struct image *image;
  uint32_t       hash;
  
  hash = strihash(name);
  
  dlink_foreach(&image_list, node)
  {
    image = node->data;
    
    if(image->hash == hash)
    {
      if(!stricmp(image->name, name))
        return image;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *image_find_id(uint32_t id)
{
  struct image *iptr;
  
  dlink_foreach(&image_list, iptr)
  {
    if(iptr->id == id)
      return iptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_color_hsv2rgb(uint8_t *hue, uint8_t *sat, uint8_t *val)
{
  double h, s, v;
  double f, p, q, t;

  if(*sat == 0)
  {
    *hue = *val;
    *sat = *val;
  } 
  else 
  {
    h = *hue * 6.0  / 255.0;
    s = *sat / 255.0;
    v = *val / 255.0;

    f = h - (uint8_t)h;
    p = v * (1.0 - s);
    q = v * (1.0 - (s * f));
    t = v * (1.0 - (s * (1.0 - f)));

    switch((uint8_t)h) 
    {
      case 0:
        *hue = v * 255;
        *sat = t * 255;
        *val = p * 255;
        break;

      case 1:
        *hue = q * 255;
        *sat = v * 255;
        *val = p * 255;
        break;

      case 2:
        *hue = p * 255;
        *sat = v * 255;
        *val = t * 255;
        break;

      case 3:
        *hue = p * 255;
        *sat = q * 255;
        *val = v * 255;
        break;

      case 4:
        *hue = t * 255;
        *sat = p * 255;
        *val = v * 255;
        break;

      case 5:
        *hue = v * 255;
        *sat = p * 255;
        *val = q * 255;
        break;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_color_parse(struct color *color, const char *str)
{
  char digits[3];
  
  while(isspace(*str) || *str == '#')
    str++;
  
  color->r = 0;
  color->g = 0;
  color->b = 0;
  
  strlcpy(digits, str, sizeof(digits));
  color->r = strtoul(digits, NULL, 16);
  if(!*str++) return; if(!*str++) return;
  
  strlcpy(digits, str, sizeof(digits));
  color->g = strtoul(digits, NULL, 16);
  if(!*str++) return; if(!*str++) return; 

  strlcpy(digits, str, sizeof(digits));
  color->b = strtoul(digits, NULL, 16);
  if(!*str++) return; if(!*str++) return; 
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_color_set(struct image *iptr, uint8_t index, struct color *color)
{
  iptr->palette[index].r = color->r;
  iptr->palette[index].g = color->g;
  iptr->palette[index].b = color->b;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_color_setrgb(struct image *iptr, uint8_t index, uint8_t red,
                        uint8_t green, uint8_t blue)
{
  iptr->palette[index].r = red;
  iptr->palette[index].g = green;
  iptr->palette[index].b = blue;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
char *image_color_str(struct color c)
{
  static char color[8];
  static const char hexalphabet[] = "0123456789abcdef";
  
  color[0] = '#';
  color[1] = hexalphabet[c.r >> 4];
  color[2] = hexalphabet[c.r & 15];
  color[3] = hexalphabet[c.g >> 4];
  color[4] = hexalphabet[c.g & 15];
  color[5] = hexalphabet[c.b >> 4];
  color[6] = hexalphabet[c.b & 15];
  color[7] = '\0';
  
  return color;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_color_dump(struct color *color)
{
  log(image_log, L_verbose, "r: %u g: %u b: %u a: %u", 
      color->r, color->g, color->b, color->a);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_rect_dump(struct rect *rect)
{
  log(image_log, L_verbose, "x: %i y: %i w: %u h: %u",
      rect->x, rect->y, rect->w, rect->h);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_color_sethtml(struct image *iptr, uint8_t index, const char *html)
{
  image_color_parse(&iptr->palette[index], html);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void image_palette_set(struct image *iptr, struct palette *pal)
{
//  int i;
  
  memcpy(iptr->palette, pal->colors, pal->count * sizeof(struct color));
  
  if(256 - pal->count)
    memset(&iptr->palette[pal->count], 0, 
           (256 - pal->count) * sizeof(struct color));
  
/*  for(i = 0; i < pal->count; i++)
    image_color_dump(&iptr->palette[i]);*/
  
  log(image_log, L_status, "Set a %u-color palette for image %s.", pal->count, iptr->name);
}

/* -------------------------------------------------------------------------- *
 * Create a new palette                                                       *
 * -------------------------------------------------------------------------- */
struct palette *image_palette_new(uint32_t ncolors)
{
  struct palette *pal;
  size_t          n;
  int             bpp;
    
  /* Round to either 2, 4, 8, 16, 32, 64, 128, 256 colors */
  bpp = image_bit_size(ncolors);
  ncolors = 1 << bpp;
                                
  /* Allocate memory for the palette */
  n = sizeof(struct palette) + (ncolors * sizeof(struct color));

  pal = mem_dynamic_alloc(&image_palette_heap, n);

  /* Initialize palette header */
  pal->count = ncolors;
  pal->bpp = bpp;
  pal->colors = (struct color *)&pal[1];
  
  /* Initialize colors */
  for(n = 0; n < pal->count; n++)
  {
    pal->colors[n].r = 0x00;
    pal->colors[n].g = 0x00;
    pal->colors[n].b = 0x00;
    pal->colors[n].a = 0xff;
  }
  
  return pal;
}

/* -------------------------------------------------------------------------- *
 * Create a greyscale palette                                                 *
 * -------------------------------------------------------------------------- */
struct palette *image_palette_greyscale(uint32_t ncolors, int colorkey)
{
  struct palette *pal = image_palette_new(ncolors);
  
  if(pal)
  {
    int i;
    size_t n = 0;
    int count = pal->count;
    
    if(colorkey >= 0 && colorkey <= 255)
      count--;
    
    for(i = 0; i < pal->count; i++)
    {
      if(i == colorkey)
      {
        pal->colors[i].r = 0;
        pal->colors[i].g = 0;
        pal->colors[i].b = 0;
        pal->colors[i].a = 0;
        
        i++;
      }
      
      
      pal->colors[i].r = n * 256 / count;
      pal->colors[i].g = n * 256 / count;
      pal->colors[i].b = n * 256 / count;
      pal->colors[i].a = 0xff;
      n++;
    }
  }
  
  return pal;
}
  
/* -------------------------------------------------------------------------- *
 * Create a new palette and initialise it                                     *
 * -------------------------------------------------------------------------- */
struct palette *image_palette_make(uint32_t      ncolors,
                                 struct color *colors)
{
  struct palette *pal;

  /* Create new palette */
  pal = image_palette_new(ncolors);
 
  /* Copy the colors */
  memcpy(pal->colors, colors, ncolors * sizeof(struct color));

  return pal;
}
 
/* -------------------------------------------------------------------------- *
 * Copy a palette                                                             *
 * -------------------------------------------------------------------------- */
struct palette *image_palette_copy(struct palette *pal)
{
  return image_palette_make(pal->count, pal->colors);
}

/* -------------------------------------------------------------------------- *
 * Free a palette                                                             *
 * -------------------------------------------------------------------------- */
void image_palette_free(struct palette *pal)
{
  mem_dynamic_free(&image_palette_heap, pal);
}

/* -------------------------------------------------------------------------- *
 * Returns index of the closest match                                         *
 * -------------------------------------------------------------------------- */
uint8_t image_palette_match(struct palette *palette, struct color *c, int colorkey)
{
  int i;
  int rdiff, gdiff, bdiff;
  int distance;
  unsigned int ldist = ~0;
  uint8_t index = 0;
  
  if(colorkey > 0xff)
    colorkey &= 0xff;
  
  for(i = 0; i < palette->count; i++)
  {
    if(c->a < 128 && colorkey >= 0)
      return colorkey;
    
    if(i == colorkey)
      continue;
    
    rdiff = palette->colors[i].r - c->r;
    gdiff = palette->colors[i].g - c->g;
    bdiff = palette->colors[i].b - c->b;
    
    distance = (rdiff * rdiff) +
               (gdiff * gdiff) +
               (bdiff * bdiff);
    
    if(distance < ldist)
    {
      index = i;
      ldist = distance;
      
      if(distance == 0)
        break;
    }
  }

  return index;
} 
  
/* -------------------------------------------------------------------------- *
 * Dump images and image heap.                                                *
 * -------------------------------------------------------------------------- */
void image_dump(struct image *iptr)
{
  if(iptr == NULL)
  {
    dump(image_log, "[============== image summary ===============]");
    
    dlink_foreach(&image_list, iptr)
      dump(image_log, " #%03u: [%u] %-20s",
           iptr->id, 
           iptr->refcount,
           iptr->name);
    
    dump(image_log, "[========== end of image summary ============]");
  }
  else
  {
    dump(image_log, "[============== image dump ===============]");
    dump(image_log, "         id: #%u", iptr->id);
    dump(image_log, "   refcount: %u", iptr->refcount);
    dump(image_log, "       hash: %p", iptr->hash);
    dump(image_log, "       name: %s", iptr->name);

    dump(image_log, "[========== end of image dump ============]");    
  }
}

