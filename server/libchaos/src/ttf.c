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
 * $Id: ttf.c,v 1.13 2005/01/17 19:09:50 smoli Exp $
 */

#define _GNU_SOURCE

#include <math.h>
#include <libchaos/defs.h>
#ifdef HAVE_FT2
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/mem.h>
#include <libchaos/ttf.h>
#include <libchaos/log.h>
#include <libchaos/str.h>
#include <libchaos/image.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define CACHED_BITMAP  0x01
#define CACHED_PIXMAP  0x02
#define CACHED_METRICS 0x04

/* handy routines for converting from fixed point */
#define TTF_FLOOR(X)     ((X & -64) / 64)
#define TTF_CEIL(X)      (((X + 63) & -64) / 64)

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
int                ttf_log; 
struct sheap       ttf_heap;       /* heap containing ttf blocks */
struct list        ttf_list;       /* list linking ttf blocks */
uint32_t           ttf_id;
int                ttf_dirty;
FT_Library         ttf_library;
int                ttf_byteswapped;

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static uint16_t *ttf_latin1_to_unicode(uint16_t *unicode, const char *text, size_t len)
{
  size_t i;
  
  for(i = 0; i < len; i++)
    unicode[i] = ((const uint8_t *)text)[i];
  
  unicode[i] = 0;
  
  return unicode;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static uint16_t *ttf_utf8_to_unicode(uint16_t *unicode, const char *utf8, size_t len)
{
  int      i, j;
  uint16_t ch;
  
  for(i = 0, j = 0; i < len; j++)
  {
    ch = ((const uint8_t *)utf8)[i];
    
    if(ch >= 0xf0)
    {
      ch  = (uint16_t)(utf8[i++] & 0x07) << 18;
      ch |= (uint16_t)(utf8[i++] & 0x3f) << 12;
      ch |= (uint16_t)(utf8[i++] & 0x3f) << 6;
      ch |= (uint16_t)(utf8[i++] & 0x3f);
    }
    else if(ch >= 0xe0)
    {
      ch  = (uint16_t)(utf8[i++] & 0x3f) << 12;
      ch |= (uint16_t)(utf8[i++] & 0x3f) << 6;
      ch |= (uint16_t)(utf8[i++] & 0x3f);
    }
    else if(ch >= 0xc0)
    {
      ch  = (uint16_t)(utf8[i++] & 0x3f) << 6;
      ch |= (uint16_t)(utf8[i++] & 0x3f);
    }
    unicode[j] = ch;
    
  }
  
  unicode[j] = 0;
  
  return unicode;
}

/* -------------------------------------------------------------------------- *
 * Initialize ttf heap and add garbage collect timer.                       *
 * -------------------------------------------------------------------------- */
void ttf_init(void)
{
  int error;
  
  ttf_log = log_source_register("ttf");
  
  dlink_list_zero(&ttf_list);
  
  ttf_id = 0;
  ttf_dirty = 0;
  
  mem_static_create(&ttf_heap, sizeof(struct ttf), TTF_BLOCK_SIZE);
  mem_static_note(&ttf_heap, "ttf block heap");

  error = FT_Init_FreeType(&ttf_library);
  
  if(error)
  {
   /*  ... an error occurred during library initialization ... */
  }
  
  log(ttf_log, L_status, "Initialized [ttf] module.");
}

/* -------------------------------------------------------------------------- *
 * Destroy ttf heap                                                         *
 * -------------------------------------------------------------------------- */
void ttf_shutdown(void)
{
  struct ttf *iptr;
  struct ttf *next;
  
  /* Report status */
  log(ttf_log, L_status, "Shutting down [ttf] module...");
  
  /* Remove all ttf blocks */
  dlink_foreach_safe(&ttf_list, iptr, next)
  {
    if(iptr->refcount)
      iptr->refcount--;

    ttf_delete(iptr);
  }

  mem_static_destroy(&ttf_heap);
    
  FT_Done_FreeType(ttf_library);
  
  /* Unregister log source */
  log_source_unregister(ttf_log);
}

/* -------------------------------------------------------------------------- *
 * Collect ttf block garbage.                                              *
 * -------------------------------------------------------------------------- */
int ttf_collect(void)
{
  struct ttf *cnptr;
  struct ttf *next;
  size_t         n = 0;
  
  if(ttf_dirty)
  {
    /* Report verbose */
    log(ttf_log, L_verbose, 
        "Doing garbage collect for [ttf] module.");
    
    /* Free all ttf blocks with a zero refcount */
    dlink_foreach_safe(&ttf_list, cnptr, next)
    {
      if(!cnptr->refcount)
      {
        ttf_delete(cnptr);
        
        n++;
      }
    }
  
    /* Collect garbage on ttf_heap */
    mem_static_collect(&ttf_heap);
    
    ttf_dirty = 0;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void ttf_default(struct ttf *ttf)
{
  dlink_node_zero(&ttf->node);
  
  strcpy(ttf->name, "default");
  ttf->id = 0;
  ttf->refcount = 0;
  ttf->hash = 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void ttf_set_error(struct ttf *ttf, const char *error)
{
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct ttf *ttf_new(const char *name)
{
  struct ttf *ttf;
  
  ttf = mem_static_alloc(&ttf_heap);
  
  ttf->id = ttf_id++;
  ttf->refcount = 1;
  
  strlcpy(ttf->name, name, sizeof(ttf->name));
  
  ttf->hash = strhash(ttf->name);
  
  dlink_add_tail(&ttf_list, &ttf->node, ttf);

  log(ttf_log, L_status, "Added ttf block: %s", ttf->name);
  
  return ttf;
}
     
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int ttf_open(struct ttf *ttf, const char *path)
{
  int error;
  
  ttf->args.flags = FT_OPEN_PATHNAME;
  ttf->args.pathname = (void *)path;
  ttf->args.driver = 0;
  
  error = FT_Open_Face(ttf_library, &ttf->args, ttf->id, &ttf->face);
  
  if(error)
  {
    log(ttf_log, L_warning, "An error occured while loading font: %s",
        path);
    
    return -1;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void ttf_close(struct ttf *ttf)
{
  
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int ttf_calc(struct ttf *ttf, int ptsize, int style)
{
  FT_Face face;
  FT_Error error;
  FT_Fixed scale;
  
  face = ttf->face;
  
  /* Make sure that our font face is scalable (global metrics) */
  if(FT_IS_SCALABLE(face))
  {
    /* Set the character size and use default DPI (72) */
    error = FT_Set_Char_Size(ttf->face, 0, ptsize * 64, 0, 0);
    
    if(error)
    {
      ttf_set_error(ttf, "Couldn't set font size");
      ttf_close(ttf);
      return -1;
    }

    /* Get the scalable font metrics for this font */
    scale = face->size->metrics.y_scale;
    ttf->ascent  = TTF_CEIL(FT_MulFix(face->bbox.yMax, scale));
    ttf->descent = TTF_CEIL(FT_MulFix(face->bbox.yMin, scale));
    ttf->height  = ttf->ascent - ttf->descent + /* baseline */ 1;
    ttf->lineskip = TTF_CEIL(FT_MulFix(face->height, scale));
    ttf->underline_offset = TTF_FLOOR(FT_MulFix(face->underline_position, scale));
    ttf->underline_height = TTF_FLOOR(FT_MulFix(face->underline_thickness, scale));
    
  }
   else
  {    
    /* Non-scalable font case.  ptsize determines which family
     * or series of fonts to grab from the non-scalable format.
     * It is not the point size of the font.
     */
    if(ptsize >= ttf->face->num_fixed_sizes)
      ptsize = ttf->face->num_fixed_sizes - 1;
    
    ttf->font_size = ptsize;
    error = FT_Set_Pixel_Sizes(face,
                               face->available_sizes[ptsize].height,
                               face->available_sizes[ptsize].width);
    /* With non-scalale fonts, Freetype2 likes to fill many of the
     * font metrics with the value of 0.  The size of the
     * non-scalable fonts must be determined differently
     * or sometimes cannot be determined.
     */
    ttf->ascent = face->available_sizes[ptsize].height;
    ttf->descent = 0;
    ttf->height = face->available_sizes[ptsize].height;
    ttf->lineskip = TTF_CEIL(ttf->ascent);
    ttf->underline_offset = TTF_FLOOR(face->underline_position);
    ttf->underline_height = TTF_FLOOR(face->underline_thickness);
  }
          
  if(ttf->underline_height < 1)
  {
    ttf->underline_height = 1;
  }
  
  /* Set the default font style */
  ttf->style = style;
  ttf->glyph_overhang = face->size->metrics.y_ppem / 10;
  /* x offset = cos(((90.0-12)/360)*2*M_PI), or 12 degree angle */
  ttf->glyph_italics = 0.207f;
  ttf->glyph_italics *= ttf->height;
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int ttf_load(struct ttf *ttf, void *data, size_t len)
{
  int error;
  
  ttf->args.flags = FT_OPEN_MEMORY;
  ttf->args.memory_base = data;
  ttf->args.memory_size = len;
  ttf->args.driver = 0;
  
  error = FT_Open_Face(ttf_library, &ttf->args, ttf->id, &ttf->face);
  
  if(error)
  {
    log(ttf_log, L_warning, "An error occured while loading font from memory");
    
    return -1;
  }
  
  if(ttf->face->family_name)
  {
    log(ttf_log, L_status, "Loaded font family: %s", ttf->face->family_name);
  }
  if(ttf->face->style_name)
  {
    log(ttf_log, L_status, "Loaded font style: %s", ttf->face->style_name);
  }
  
  if(FT_Get_Postscript_Name(ttf->face))
  {
    log(ttf_log, L_status, "Loaded Postscript font: %s", FT_Get_Postscript_Name(ttf->face));
  }
  
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static int ttf_glyph_load(struct ttf *ttf, uint16_t ch, struct ttf_cache *cached, int want)
{
  FT_Glyph_Metrics *metrics;
  FT_GlyphSlot      glyph;
  FT_Outline       *outline;
  FT_Error          error;
  FT_Face           face;
  
  face = ttf->face;
  
  /* get glyph index and load it */
  if(!cached->index)
    cached->index = FT_Get_Char_Index(face, ch);
  
  if((error = FT_Load_Glyph(ttf->face, cached->index, FT_LOAD_DEFAULT)))
  {
    log(ttf_log, L_warning, "Error while loading glyph for font: %s", ttf->name);
    return error;
  }
  
  /* setup some shortcuts */
  glyph = face->glyph;
  metrics = &glyph->metrics;
  outline = &glyph->outline;
  
  /* get the glyph metrics if desired */
  if((want & CACHED_METRICS) && !(cached->stored & CACHED_METRICS))
  {
    if(FT_IS_SCALABLE(face))
    {
      /* get the bounding box */
      cached->minx = TTF_FLOOR(metrics->horiBearingX);
      cached->maxx = cached->minx + TTF_CEIL(metrics->width);
      cached->maxy = TTF_FLOOR(metrics->horiBearingY);
      cached->miny = cached->maxy - TTF_CEIL(metrics->height);
      cached->yoffset = ttf->ascent - cached->maxy;
      cached->advance = TTF_CEIL(metrics->horiAdvance);
    }
    else
    {
      /* Get the bounding box for non-scalable format.
         Again, freetype2 fills in many of the font metrics
         with the value of 0, so some of the values we
         need must be calculated differently with certain
         assumptions about non-scalable formats. */
      cached->minx = TTF_FLOOR(metrics->horiBearingX);
      cached->maxx = cached->minx + TTF_CEIL(metrics->horiAdvance);
      cached->maxy = TTF_FLOOR(metrics->horiBearingY);
      cached->miny = cached->maxy - TTF_CEIL(face->available_sizes[ttf->font_size].height);
      cached->yoffset = 0;
      cached->advance = TTF_CEIL(metrics->horiAdvance);
    }
    
    if(ttf->style & TTF_STYLE_BOLD)
      cached->maxx += ttf->glyph_overhang;
    
    if(ttf->style & TTF_STYLE_ITALIC)
      cached->maxx += (int)ceil(ttf->glyph_italics);
    
    cached->stored |= CACHED_METRICS;
  }
  
  if(((want & CACHED_BITMAP) && !(cached->stored & CACHED_BITMAP)) ||
     ((want & CACHED_PIXMAP) && !(cached->stored & CACHED_PIXMAP)))
  {
    int mono = (want & CACHED_BITMAP);
    int i;
    FT_Bitmap *src;
    FT_Bitmap *dst;
    
    if(ttf->style & TTF_STYLE_ITALIC)
    {
      FT_Matrix shear;
      
      shear.xx = 1 << 16;
      shear.xy = (int)(ttf->glyph_italics * (1 << 16)) / ttf->height;
      shear.yx = 0;
      shear.yy = 1 << 16;
      
      FT_Outline_Transform(outline, &shear);
    }
    
    if(mono)
      error = FT_Render_Glyph(glyph, ft_render_mode_mono);
    else
      error = FT_Render_Glyph(glyph, ft_render_mode_normal);
    
    if(error)
      return error;
    
    src = &glyph->bitmap;
    if(mono)
      dst = &cached->bitmap;
    else
      dst = &cached->pixmap;
    
    memcpy(dst, src, sizeof(FT_Bitmap));
    
    if(mono || !FT_IS_SCALABLE(face))
      dst->pitch *= 8;
    
    if(ttf->style & TTF_STYLE_BOLD)
    {
      int bump = ttf->glyph_overhang;
      
      dst->pitch += bump;
      dst->width += bump;
    }
    
    if(ttf->style & TTF_STYLE_ITALIC)
    {
      int bump = (int)ceil(ttf->glyph_italics);
      
      dst->pitch += bump;
      dst->width += bump;
    }
    
    if(dst->rows != 0)
    {
      dst->buffer = malloc(dst->pitch * dst->rows);
      
      if(!dst->buffer)
        return -1;
      
      memset(dst->buffer, 0, dst->pitch * dst->rows);
      
      for(i = 0; i < src->rows; i++)
      {
        int soffset = i * src->pitch;
        int doffset = i * dst->pitch;
        
        if(mono)
        {
          uint8_t *srcp = src->buffer + soffset;
          uint8_t *dstp = dst->buffer + doffset;
          int      j;
          
          for(j = 0; j < src->width; j += 8)
          {
            uint8_t ch = *srcp++;

            *dstp++ = (ch & 0x80) >> 7;
            ch <<= 1;
            *dstp++ = (ch & 0x80) >> 7;
            ch <<= 1;
            *dstp++ = (ch & 0x80) >> 7;
            ch <<= 1;
            *dstp++ = (ch & 0x80) >> 7;
            ch <<= 1;
            *dstp++ = (ch & 0x80) >> 7;
            ch <<= 1;
            *dstp++ = (ch & 0x80) >> 7;
            ch <<= 1;
            *dstp++ = (ch & 0x80) >> 7;
            ch <<= 1;
            *dstp++ = (ch & 0x80) >> 7;
            ch <<= 1;
          }
        }
        else if(!FT_IS_SCALABLE(face))
        {
          uint8_t *srcp = src->buffer + soffset;
          uint8_t *dstp = dst->buffer + doffset;
          uint8_t  ch;
          int      j, k;
          
          for(j = 0; j < src->width; j += 8)
          {
            ch = *srcp++;
            
            for(k = 0; k < 8; k++)
            {
              if((ch & 0x80) >> 7)
                *dstp++ = TTF_NUM_GRAYS - 1;
              else
                *dstp++ = 0x00;
            
              ch <<= 1;
            }
          }
        }
        else
        {
          memcpy(dst->buffer + doffset, src->buffer + soffset, src->pitch);
        }
      }
    }
    
    if(ttf->style & TTF_STYLE_BOLD)
    {
      int      row;
      int      col;
      int      offset;
      int      pixel;
      uint8_t *pixmap;
      
      for(row = dst->rows - 1; row >= 0; --row)
      {
        pixmap = (uint8_t *)dst->buffer + row * dst->pitch;
        
        for(offset = 1; offset <= ttf->glyph_overhang; offset++)
        {
          for(col = dst->width - 1; col > 0; col--)
          {
            pixel = (pixmap[col] + pixmap[col - 1]);
            
            if(pixel > TTF_NUM_GRAYS - 1)
              pixel = TTF_NUM_GRAYS - 1;
            
            pixmap[col] = (uint8_t)pixel;
          }
        }
      }
    }
    
    if(mono)
      cached->stored |= CACHED_BITMAP;
    else
      cached->stored |= CACHED_PIXMAP;
  }
  
  cached->cached = ch;
  
  return 0;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static void ttf_glyph_flush(struct ttf_cache *glyph)
{
  glyph->stored = 0;
  glyph->index = 0;
  
  if(glyph->bitmap.buffer)
  {
    free(glyph->bitmap.buffer);
    glyph->bitmap.buffer = NULL;
  }
  
  if(glyph->pixmap.buffer)
  {
    free(glyph->pixmap.buffer);
    glyph->pixmap.buffer = 0;
  }
  
  glyph->cached = 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
static int ttf_glyph_find(struct ttf *ttf, uint16_t ch, int want)
{
  int ret = 0;

  if(ch < 0x100)
  {
    ttf->current = &ttf->cache[ch];
  }
  else
  {
    if(ttf->scratch.cached != ch)
      ttf_glyph_flush(&ttf->scratch);

    ttf->current = &ttf->scratch;
  }
  
  if((ttf->current->stored & want) != want)
    ret = ttf_glyph_load(ttf, ch, ttf->current, want);

  return ret;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
/*static*/ void ttf_flush(struct ttf *ttf)
{
  int i;
  int size = sizeof(ttf->cache) / sizeof(ttf->cache[0]);
  
  for(i = 0; i < size; i++)
  {
    if(ttf->cache[i].cached)
      ttf_glyph_flush(&ttf->cache[i]);
  }
  
  if(ttf->scratch.cached)
    ttf_glyph_flush(&ttf->scratch);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *ttf_glyph_solid(struct ttf *ttf, uint16_t ch, struct color *c)
{
  struct image     *image;
  struct palette   *palette;
  uint8_t          *src, *dst;
  int               row;
  FT_Error          error;
  struct ttf_cache *glyph;
  
  if((error = ttf_glyph_find(ttf, ch, CACHED_METRICS|CACHED_BITMAP)))
  {
    log(ttf_log, L_warning, "Could not find glyph '%c' in font: %s", ch, ttf->name);
    
    return NULL;
  }
  
  glyph = ttf->current;
  
  image = image_new(IMAGE_TYPE_8, glyph->bitmap.pitch, glyph->bitmap.rows);
  
  if(!image)
  {
    log(ttf_log, L_warning, "Could not create image (%ux%u)", glyph->bitmap.pitch, glyph->bitmap.rows);
    
    return NULL;
  }
  
  palette = image_palette_new(2);
  
  palette->colors[0].r = 255 - c->r;
  palette->colors[0].g = 255 - c->g;
  palette->colors[0].b = 255 - c->b;
  palette->colors[1].r = c->r;
  palette->colors[1].g = c->g;
  palette->colors[1].b = c->b;
  
  image_palette_set(image, palette);
  image_palette_free(palette);
  // set colorkey later 
  
  src = glyph->bitmap.buffer;
  dst = (uint8_t *)image->pixel.data8;
  
  for(row = 0; row < image->h; row++)
  {
    memcpy(dst, src, glyph->bitmap.pitch);
    src += glyph->bitmap.pitch;
    dst += image->pitch;
  }
  
  if(ttf->style & TTF_STYLE_UNDERLINE)
  {
    row = ttf->ascent - ttf->underline_offset - 1;
    
    if(row >= image->h)
      row = (image->h - 1) - ttf->underline_height;
    
    dst = (uint8_t *)&image->pixel.data8[row * image->pitch];
    
    for(row = ttf->underline_height; row > 0; row--)
    {
      memset(dst, 1, image->w);
      dst += image->pitch;
    }
  }
  
  return image;
}  
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
uint32_t ttf_glyph_width(struct ttf *ttf, uint16_t ch)
{
  FT_Error          error;
  struct ttf_cache *glyph;
  
  if((error = ttf_glyph_find(ttf, ch, CACHED_METRICS|CACHED_BITMAP)))
  {
    log(ttf_log, L_warning, "Could not find glyph '%c' in font: %s", ch, ttf->name);
    
    return 0;
  }
  
  glyph = ttf->current;
  
  return glyph->advance + (ttf->style & TTF_STYLE_BOLD ? ttf->glyph_overhang : 0);
}  
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int ttf_unicode_size(struct ttf *ttf, const uint16_t *text, 
                     uint16_t   *w,   uint16_t *h)
{
  struct ttf_cache *glyph;
  const uint16_t   *ch;
  FT_Error          error;
  int               status;
  int               swapped;
  int               x, z;
  int               minx, maxx;
  int               miny, maxy;
  
  status = 0;
  minx = maxx = 0;
  miny = maxy = 0;
  swapped = ttf_byteswapped;
  
  x = 0;
  
  for(ch = text; *ch; ch++)
  {
    uint16_t c = *ch;
    
    if(c == UNICODE_BOM_NATIVE)
    {
      swapped = 0;
      
      if(text == ch)
        text++;
      
      continue;
    }
    
    if(c == UNICODE_BOM_SWAPPED)
    {
      swapped = 1;
      
      if(text == ch)
        text++;
      
      continue;
    }
    
    if(swapped)
      c = ((c >> 8) & 0xff) | ((c & 0xff) << 8);
    
    if((error = ttf_glyph_find(ttf, c, CACHED_METRICS|CACHED_BITMAP)))
      return -1;
    
    glyph = ttf->current;
    
    if((ch == text) && (glyph->minx < 0))
      z -= glyph->minx;
    
    z = x + glyph->minx;
    
    if(minx > z)
      minx = z;
    
    if(ttf->style & TTF_STYLE_BOLD)
      x += ttf->glyph_overhang;
    
    if(glyph->advance > glyph->maxx)
      z = x + glyph->advance;
    else
      z = x + glyph->maxx;
    
    if(maxx < z)
      maxx = z;
    
    x += glyph->advance;
    
    if(glyph->miny < miny)
      miny = glyph->miny;
    
    if(glyph->maxy > maxy)
      maxy = glyph->maxy;
  }
  
  if(w)
    *w = (maxx - minx);
  
  if(h)
    *h = ttf->height;
  
  return status;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *ttf_unicode_solid(struct ttf *ttf, const uint16_t *text, struct color *c)
{
  struct ttf_cache *glyph;
  const uint16_t   *ch;
  struct palette   *palette;
  struct image     *image;
  FT_Bitmap        *current;
  FT_Error          error;
  uint16_t          width;
  uint16_t          height;
  uint8_t          *src;
  uint8_t          *dst;
  int               swapped;
  int               xstart;
  int               row;
  int               col;
  
  if((ttf_unicode_size(ttf, text, &width, NULL) < 0) || !width)
  {
    ttf_set_error(ttf, "text has zero width");
    return NULL;
  }

  height = ttf->height;
  
  if((image = image_new(IMAGE_TYPE_8, width, height)) == NULL)
    return NULL;
  
  palette = image_palette_new(2);
  palette->colors[0].r = 255 - c->r;
  palette->colors[0].g = 255 - c->g;
  palette->colors[0].b = 255 - c->b;
  palette->colors[0].r = c->r;
  palette->colors[0].g = c->g;
  palette->colors[0].b = c->b;
  image_palette_set(image, palette);
  image_palette_free(palette);
 
  xstart = 0;
  swapped = ttf_byteswapped;
  
  for(ch = text; *ch; ch++)
  {
    uint16_t c = *ch;
    
    if(c == UNICODE_BOM_NATIVE)
    {
      swapped = 0;
      
      if(text == ch)
        text++;
      
      continue;
    }
    
    if(c == UNICODE_BOM_SWAPPED)
    {
      swapped = 1;
      
      if(text == ch)
        text++;
      
      continue;
    }
    
    if(swapped)
      c = ((c >> 8) & 0xff) | ((c & 0xff) << 8);
    
    if((error = ttf_glyph_find(ttf, c, CACHED_METRICS|CACHED_BITMAP)))
    {
      image_delete(image);
      return NULL;
    }
    
    glyph = ttf->current;
    current = &glyph->bitmap;
    
    if((ch == text) && (glyph->minx < 0))
      xstart -= glyph->minx;
    
    for(row = 0; row < current->rows; row++)
    {
      if(row + glyph->yoffset >= image->h)
        continue;
      
      dst = image->pixel.data8 + (row + glyph->yoffset) * image->pitch + xstart + glyph->minx;
      src = current->buffer + row * current->pitch;
      
      for(col = current->width; col > 0; col--)
        *dst++ |= *src++;
    }
    
    xstart += glyph->advance;
    
    if(ttf->style & TTF_STYLE_BOLD)
      xstart += ttf->glyph_overhang;
  }
  
  if(ttf->style & TTF_STYLE_UNDERLINE)
  {
    row = ttf->ascent - ttf->underline_offset - 1;
    
    if(row >= image->h)
      row = (image->h - 1) - ttf->underline_height;

    dst = image->pixel.data8 + row * image->pitch;
    
    for(row = ttf->underline_height; row > 0; row--)
    {
      memset(dst, 1, image->w);
      dst += image->pitch;
    }
  }
  
  return image;
}  
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *ttf_text_solid(struct ttf *ttf, const char *text, struct color *c)
{
  struct image *image;
  uint16_t     *unicode_text;
  int           unicode_len;
  
  unicode_len = strlen(text);
  unicode_text = (uint16_t *)alloca((1 + unicode_len + 1) * sizeof(uint16_t));
  if(unicode_text == NULL)
  {
    ttf_set_error(ttf, "out of memory");
    return NULL;
  }
  
  *unicode_text = UNICODE_BOM_NATIVE;
  
  ttf_latin1_to_unicode(unicode_text + 1, text, unicode_len);
  
  image = ttf_unicode_solid(ttf, unicode_text, c);
  
  return image;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *ttf_utf8_solid(struct ttf *ttf, const char *utf8, struct color *c)
{
  struct image *image;
  uint16_t     *unicode_text;
  int           unicode_len;
  
  unicode_len = strlen(utf8);
  unicode_text = (uint16_t *)alloca((1 + unicode_len + 1) * sizeof(uint16_t));
  
  if(unicode_text == NULL)
  {
    ttf_set_error(ttf, "out of memory");
    return NULL;
  }
  
  *unicode_text = UNICODE_BOM_NATIVE;
  
  ttf_utf8_to_unicode(unicode_text, utf8, unicode_len);
  
  image = ttf_unicode_solid(ttf, unicode_text, c);
  
  return image;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *ttf_glyph_shaded(struct ttf *ttf, uint16_t ch, struct color *bg, struct color *fg)
{
  struct image     *image;
  struct palette   *palette;
  uint8_t          *src, *dst;
  uint32_t          index;
  int               row;
  FT_Error          error;
  struct ttf_cache *glyph;
  int               rdiff, gdiff, bdiff;

  if((error = ttf_glyph_find(ttf, ch, CACHED_METRICS|CACHED_PIXMAP)))
  {
    log(ttf_log, L_warning, "Could not find glyph '%c' in font: %s", ch, ttf->name);
    
    return NULL;
  }
  
  glyph = ttf->current;
  
  image = image_new(IMAGE_TYPE_8, glyph->pixmap.pitch, glyph->pixmap.rows);
  
  if(!image)
  {
    log(ttf_log, L_warning, "Could not create image (%ux%u)", glyph->pixmap.pitch, glyph->pixmap.rows);
    
    return NULL;
  }
  
  palette = image_palette_new(TTF_NUM_GRAYS);
  rdiff = fg->r - bg->r;
  gdiff = fg->g - bg->g;
  bdiff = fg->b - bg->b;
  
  for(index = 0; index < TTF_NUM_GRAYS; index++)
  {
    palette->colors[index].r = bg->r + (index * rdiff) / (TTF_NUM_GRAYS - 1);
    palette->colors[index].g = bg->g + (index * gdiff) / (TTF_NUM_GRAYS - 1);
    palette->colors[index].b = bg->b + (index * bdiff) / (TTF_NUM_GRAYS - 1);
  }
  
  image_palette_set(image, palette);
  image_palette_free(palette);
  
  src = glyph->pixmap.buffer;
  dst = image->pixel.data8;
  
  for(row = 0; row < image->h; row++)
  {
    memcpy(dst, src, glyph->pixmap.pitch);
    src += glyph->pixmap.pitch;
    dst += image->pitch;
  }
  
  if(ttf->style & TTF_STYLE_UNDERLINE)
  {
    row = ttf->ascent - ttf->underline_offset - 1;
    
    if(row >= image->h)
    {
      row = (image->h - 1) - ttf->underline_height;
    }
    
    dst = image->pixel.data8 + row * image->pitch;
    
    for(row = ttf->underline_height; row > 0; row--)
    {
      memset(dst, TTF_NUM_GRAYS - 1, image->w);
      dst += image->pitch;
    }
  }
    
  return image;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *ttf_unicode_shaded(struct ttf *ttf, const uint16_t *text, struct color *bg, struct color *fg)
{
  struct ttf_cache *glyph;
  struct palette   *palette;
  const uint16_t   *ch;
  struct image     *image;
  FT_Bitmap        *current;
  FT_Error          error;
  uint16_t          height;
  uint16_t          width;
  uint8_t          *src;
  uint8_t          *dst;
  int               row, col;
  int               swapped;
  int               xstart;
  int               index;
  int               rdiff;
  int               gdiff;
  int               bdiff;
  
  if((ttf_unicode_size(ttf, text, &width, NULL) < 0) || !width)
  {
    ttf_set_error(ttf, "text has zero width");
    return NULL;
  }

  height = ttf->height;
  
  if((image = image_new(IMAGE_TYPE_8, width, height)) == NULL)
    return NULL;
  
  palette = image_palette_new(TTF_NUM_GRAYS);
  rdiff = fg->r - bg->r;
  gdiff = fg->g - bg->g;
  bdiff = fg->b - bg->b;
  
  for(index = 0; index < TTF_NUM_GRAYS; index++)
  {
    palette->colors[index].r = bg->r + (index * rdiff) / (TTF_NUM_GRAYS - 1);
    palette->colors[index].g = bg->g + (index * gdiff) / (TTF_NUM_GRAYS - 1);
    palette->colors[index].b = bg->b + (index * bdiff) / (TTF_NUM_GRAYS - 1);
  }
  
  image_palette_set(image, palette);
  image_palette_free(palette);

  xstart = 0;
  swapped = ttf_byteswapped;
  
  for(ch = text; *ch; ch++)
  {
    uint16_t c = *ch;
    
    if(c == UNICODE_BOM_NATIVE)
    {
      swapped = 0;
      
      if(text == ch)
        text++;
      
      continue;
    }
    
    if(c == UNICODE_BOM_SWAPPED)
    {
      swapped = 1;
      
      if(text == ch)
        text++;
      
      continue;
    }
    
    if(swapped)
      c = ((c >> 8) & 0xff) | ((c & 0xff) << 8);
    
    if((error = ttf_glyph_find(ttf, c, CACHED_METRICS|CACHED_PIXMAP)))
    {
      image_delete(image);
      return NULL;
    }
    
    glyph = ttf->current;
    
    if((ch == text) && (glyph->minx < 0))
      xstart -= glyph->minx;
    
    current = &glyph->pixmap;
    
    for(row = 0; row < current->rows; row++)
    {
      if(row + glyph->yoffset >= image->h)
        continue;
      
      dst = image->pixel.data8 + (row + glyph->yoffset) * image->pitch + xstart + glyph->minx;
      src = current->buffer + row * current->pitch;
      
      for(col = current->width; col > 0; col--)
        *dst++ |= *src++;      
    }
    
    xstart += glyph->advance;
    
    if(ttf->style & TTF_STYLE_BOLD)
    {
      xstart += ttf->glyph_overhang;
    }
  }

  if(ttf->style & TTF_STYLE_UNDERLINE)
  {
    row = ttf->ascent - ttf->underline_offset - 1;
    
    if(row >= image->h)
      row = (image->h - 1) - ttf->underline_height;
    
    dst = image->pixel.data8 + row * image->pitch;
    
    for(row = ttf->underline_height; row > 0; row--)
    {
      memset(dst, TTF_NUM_GRAYS - 1, image->w);
      dst += image->pitch;
    }
  }

  return image;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *ttf_text_shaded(struct ttf *ttf, const char *text, struct color *bg, struct color *fg)
{
  struct image *image;
  uint16_t     *unicode_text;
  int           unicode_len;
  
  unicode_len = strlen(text);
  unicode_text = (uint16_t *)alloca((1 + unicode_len + 1) * sizeof(uint16_t));
  if(unicode_text == NULL)
  {
    ttf_set_error(ttf, "out of memory");
    return NULL;
  }
  
  *unicode_text = UNICODE_BOM_NATIVE;
  
  ttf_latin1_to_unicode(unicode_text + 1, text, unicode_len);
  
  image = ttf_unicode_shaded(ttf, unicode_text, bg, fg);
  
  return image;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *ttf_utf8_shaded(struct ttf *ttf, const char *utf8, struct color *bg, struct color *fg)
{
  struct image *image;
  uint16_t     *unicode_text;
  int           unicode_len;
  
  unicode_len = strlen(utf8);
  unicode_text = (uint16_t *)alloca((1 + unicode_len + 1) * sizeof(uint16_t));
  
  if(unicode_text == NULL)
  {
    ttf_set_error(ttf, "out of memory");
    return NULL;
  }
  
  *unicode_text = UNICODE_BOM_NATIVE;
  
  ttf_utf8_to_unicode(unicode_text, utf8, unicode_len);
  
  image = ttf_unicode_shaded(ttf, unicode_text, bg, fg);
  
  return image;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *ttf_glyph_blended(struct ttf *ttf, uint16_t ch, struct color *c)
{
  struct ttf_cache *glyph;
  struct image     *image;
  FT_Error          error;
  uint32_t          alpha;
  uint32_t          pixel;
  uint32_t         *dst;
  uint8_t          *src;
  int               row, col;
  
  error = ttf_glyph_find(ttf, ch, CACHED_METRICS|CACHED_PIXMAP);
  
  if(error)
    return NULL;
  
  glyph = ttf->current;
  
  image = image_new(IMAGE_TYPE_32, glyph->pixmap.width, glyph->pixmap.rows);
  
  if(image == NULL)
    return NULL;
  
  pixel = (c->r << RSHIFT) | (c->g << GSHIFT) | (c->b << BSHIFT);
  
  for(row = 0; row < image->h; row++)
  {
    src = glyph->pixmap.buffer + row * glyph->pixmap.pitch; 
    dst = image->pixel.data32 + ((row * image->pitch) >> 2);
    
    for(col = 0; col < glyph->pixmap.width; col++)
    {
      alpha = *src++;
      *dst++ = pixel | (alpha << ASHIFT);
    }    
  }
  
  if(ttf->style & TTF_STYLE_UNDERLINE)
  {
    row = ttf->ascent - ttf->underline_offset - 1;
    
    if(row >= image->h)
      row = image->h - 1 - ttf->underline_height;
    
    dst = image->pixel.data32 + ((row * image->pitch) >> 2);
    pixel |= AMASK;
    
    for(row = ttf->underline_height; row > 0; row--)
    {
      for(col = 0; col < image->w; col++)
        dst[col] = pixel;
      
      dst += image->pitch >> 2;
    }
  }
  
  return image;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *ttf_unicode_blended(struct ttf *ttf, const uint16_t *text, struct color *c)
{
  struct ttf_cache *glyph;
  struct image     *image;
  FT_Error          error;
  uint32_t          alpha;
  uint32_t          pixel;
  uint32_t         *dst;
  uint16_t          width;
  uint16_t          height;
  const uint16_t   *ch;
  uint8_t          *src;
  int               row, col;
  int               xstart;
  int               swapped;
  
  if((ttf_unicode_size(ttf, text, &width, NULL) < 0) || !width)
  {
    ttf_set_error(ttf, "text has zero width");
    return NULL;
  }
  
  height = ttf->height;
  
  image = image_new(IMAGE_TYPE_32, width, height);
  
  image_set_name(image, "blended text");
  
  if(image == NULL)
    return NULL;

  xstart = 0;
  swapped = ttf_byteswapped;
  
  pixel = (c->r << RSHIFT) | (c->g << GSHIFT) | (c->b << BSHIFT);

  for(ch = text; *ch; ch++)
  {
    uint16_t c = *ch;
    
    if(c == UNICODE_BOM_NATIVE)
    {
      swapped = 0;
      
      if(text == ch)
        text++;
      
      continue;
    }
    
    if(c == UNICODE_BOM_SWAPPED)
    {
      swapped = 1;
      
      if(text == ch)
        text++;
      
      continue;
    }
    
    if(swapped)
      c = ((c >> 8) & 0xff) | ((c & 0xff) << 8);
      
    if((error = ttf_glyph_find(ttf, c, CACHED_METRICS|CACHED_PIXMAP)))
    {
      image_delete(image);
      return NULL;
    }
    
    glyph = ttf->current;
    width = glyph->pixmap.width;
    
    if((ch == text) && (glyph->minx < 0))
      xstart -= glyph->minx;
    
    for(row = 0; row < glyph->pixmap.rows; row++)
    {
      if(row + glyph->yoffset >= image->h)
        continue;
      
      dst = image->pixel.data32 + (((row + glyph->yoffset) * image->pitch) >> 2) + xstart + glyph->minx;
      
      src = (uint8_t *)(glyph->pixmap.buffer + glyph->pixmap.pitch * row);
      
      for(col = width; col > 0; col--)
      {
        alpha = *src++;
        *dst++ |= pixel | (alpha << ASHIFT);
      }
    }
    
    xstart += glyph->advance;
    
    if(ttf->style & TTF_STYLE_BOLD)
      xstart += ttf->glyph_overhang;
  }
  
  if(ttf->style & TTF_STYLE_UNDERLINE)
  {
    row = ttf->ascent - ttf->underline_offset - 1;
    
    if(row >= image->h)
      row = (image->h - 1) - ttf->underline_height;
    
    dst = image->pixel.data32 + ((row * image->pitch) >> 2);
    
    pixel |= AMASK;
    
    for(row = ttf->underline_height; row > 0; row--)
    {
      for(col = 0; col < image->w; col++)
        dst[col] = pixel;
      
      dst += image->pitch >> 2;
    }
  }
  
  return image;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *ttf_text_blended(struct ttf *ttf, const char *text, struct color *c)
{
  struct image *image;
  uint16_t     *unicode_text;
  int           unicode_len;
  
  unicode_len = strlen(text);
  unicode_text = (uint16_t *)alloca((1 + unicode_len + 1) * sizeof(uint16_t));
  if(unicode_text == NULL)
  {
    ttf_set_error(ttf, "out of memory");
    return NULL;
  }
  
  *unicode_text = UNICODE_BOM_NATIVE;
  
  ttf_latin1_to_unicode(unicode_text + 1, text, unicode_len);
  
  image = ttf_unicode_blended(ttf, unicode_text, c);
  
  return image;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct image *ttf_utf8_blended(struct ttf *ttf, const char *utf8, struct color *c)
{
  struct image *image;
  uint16_t     *unicode_text;
  int           unicode_len;
  
  unicode_len = strlen(utf8);
  unicode_text = (uint16_t *)alloca((1 + unicode_len + 1) * sizeof(uint16_t));
  
  if(unicode_text == NULL)
  {
    ttf_set_error(ttf, "out of memory");
    return NULL;
  }
  
  *unicode_text = UNICODE_BOM_NATIVE;
  
  ttf_utf8_to_unicode(unicode_text, utf8, unicode_len);
  
  image = ttf_unicode_blended(ttf, unicode_text, c);
  
  return image;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void ttf_delete(struct ttf *ttf)
{
  log(ttf_log, L_status, "Deleting ttf block: %s", ttf->name);
 
  dlink_delete(&ttf_list, &ttf->node);
  
  mem_static_free(&ttf_heap, ttf);
}

/* -------------------------------------------------------------------------- *
 * Loose all references                                                       *
 * -------------------------------------------------------------------------- */
void ttf_release(struct ttf *iptr)
{
  ttf_dirty = 1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void ttf_set_name(struct ttf *ttf, const char *name)
{
  strlcpy(ttf->name, name, sizeof(ttf->name));
  
  ttf->hash = strihash(ttf->name);
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
const char *ttf_get_name(struct ttf *ttf)
{
  return ttf->name;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct ttf *ttf_find_name(const char *name)
{
  struct node   *node;
  struct ttf *ttf;
  uint32_t       hash;
  
  hash = strihash(name);
  
  dlink_foreach(&ttf_list, node)
  {
    ttf = node->data;
    
    if(ttf->hash == hash)
    {
      if(!stricmp(ttf->name, name))
        return ttf;
    }
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
struct ttf *ttf_find_id(uint32_t id)
{
  struct ttf *iptr;
  
  dlink_foreach(&ttf_list, iptr)
  {
    if(iptr->id == id)
      return iptr;
  }
  
  return NULL;
}

/* -------------------------------------------------------------------------- *
 * Dump ttfs and ttf heap.                                                *
 * -------------------------------------------------------------------------- */
void ttf_dump(struct ttf *iptr)
{
  if(iptr == NULL)
  {
    dump(ttf_log, "[============== ttf summary ===============]");
    
    dlink_foreach(&ttf_list, iptr)
      dump(ttf_log, " #%03u: [%u] %-20s",
           iptr->id, 
           iptr->refcount,
           iptr->name);
    
    dump(ttf_log, "[========== end of ttf summary ============]");
  }
  else
  {
    dump(ttf_log, "[============== ttf dump ===============]");
    dump(ttf_log, "         id: #%u", iptr->id);
    dump(ttf_log, "   refcount: %u", iptr->refcount);
    dump(ttf_log, "       hash: %p", iptr->hash);
    dump(ttf_log, "       name: %s", iptr->name);

    dump(ttf_log, "[========== end of ttf dump ============]");    
  }
}

#endif /* HAVE_FT2 */
