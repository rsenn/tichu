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
 * $Id: ttf.h,v 1.9 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef LIB_TTF_H
#define LIB_TTF_H

#ifdef HAVE_FT2
#include <ft2build.h>

#ifndef FT_FREETYPE_H
#warning "FT_FREETYPE_H not defined"
#define FT_FREETYPE_H <freetype/freetype.h>
#endif

#include FT_FREETYPE_H 

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */

#include <libchaos/defs.h>
#include <libchaos/dlink.h>
#include <libchaos/image.h>

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */
#define TTF_STYLE_NORMAL        0x00
#define TTF_STYLE_BOLD          0x01
#define TTF_STYLE_ITALIC        0x02
#define TTF_STYLE_UNDERLINE     0x04

#define TTF_NUM_GRAYS           256

/* ZERO WIDTH NO-BREAKSPACE (Unicode byte order mark) */
#define UNICODE_BOM_NATIVE      0xFEFF
#define UNICODE_BOM_SWAPPED     0xFFFE

/* -------------------------------------------------------------------------- *
 * ttf block structure.                                                     *
 * -------------------------------------------------------------------------- */
struct ttf_cache {
  int       stored;
  FT_UInt   index;
  FT_Bitmap bitmap;
  FT_Bitmap pixmap;
  int       minx;
  int       maxx;
  int       miny;
  int       maxy;
  int       yoffset;
  int       advance;
  uint16_t  cached;
};  

struct ttf {
  struct node            node;        /* linking node for ttf_list */
  uint32_t               id;
  uint32_t               refcount;    /* times this block is referenced */
  uint32_t               hash;
  
  int                    height;
  int                    ascent;
  int                    descent;
  int                    lineskip;
  int                    style;
  int                    glyph_overhang;
  int                    glyph_italics;
  int                    underline_offset;
  int                    underline_height;
  int                    font_size;
  
  FT_Face                face;
  FT_Open_Args           args;
  char                   name[64];    /* user-definable name */
  
  struct ttf_cache      *current;
  struct ttf_cache       cache[256];
  struct ttf_cache       scratch;
};

/* -------------------------------------------------------------------------- *
 * Global variables                                                           *
 * -------------------------------------------------------------------------- */
extern int                      ttf_log;
extern struct sheap             ttf_heap;     /* heap containing ttf blocks */
extern struct dheap             ttf_data_heap;/* heap containing the actual ttfs */
extern struct list              ttf_list;     /* list linking ttf blocks */
extern struct timer            *ttf_timer;
extern uint32_t                 ttf_id;
extern int                      ttf_dirty;

/* -------------------------------------------------------------------------- *
 * Initialize ttf heap and add garbage collect timer.                        *
 * -------------------------------------------------------------------------- */
extern void              ttf_init            (void);

/* -------------------------------------------------------------------------- *
 * Destroy ttf heap and cancel timer.                                        *
 * -------------------------------------------------------------------------- */
extern void              ttf_shutdown        (void);

/* -------------------------------------------------------------------------- *
 * Garbage collect                                                            *
 * -------------------------------------------------------------------------- */
extern int               ttf_collect         (void);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              ttf_default         (struct ttf      *iptr);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct ttf       *ttf_new             (const char      *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int               ttf_open            (struct ttf      *ttf,
                                              const char      *path);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int               ttf_load            (struct ttf      *ttf,
                                              void            *data,
                                              size_t           len);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct ttf       *ttf_new             (const char      *name);


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int               ttf_calc            (struct ttf      *ttf,
                                              int              ptsize,
                                              int              style);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct image     *ttf_glyph_solid     (struct ttf      *ttf,
                                              uint16_t         ch, 
                                              struct color    *c);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern uint32_t          ttf_glyph_width     (struct ttf      *ttf, 
                                              uint16_t         ch);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct image     *ttf_unicode_solid   (struct ttf      *ttf,
                                              const uint16_t  *text,
                                              struct color    *c);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct image     *ttf_text_solid      (struct ttf      *ttf,
                                              const char      *text, 
                                              struct color    *c);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct image     *ttf_utf8_solid      (struct ttf      *ttf,
                                              const char      *utf8,
                                              struct color    *c);      

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct image     *ttf_glyph_shaded    (struct ttf      *ttf,
                                              uint16_t         ch,
                                              struct color    *bg,
                                              struct color    *fg);
    
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct image     *ttf_unicode_shaded  (struct ttf      *ttf,
                                              const uint16_t  *text,
                                              struct color    *bg,
                                              struct color    *fg);      
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct image     *ttf_text_shaded     (struct ttf      *ttf,
                                              const char      *text, 
                                              struct color    *bg,
                                              struct color    *fg);      

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct image     *ttf_utf8_shaded     (struct ttf      *ttf,
                                              const char      *utf8,
                                              struct color    *bg,
                                              struct color    *fg);      

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct image     *ttf_glyph_blended    (struct ttf     *ttf,
                                               uint16_t        ch,
                                               struct color   *c);
    
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct image     *ttf_unicode_blended  (struct ttf     *ttf,
                                               const uint16_t *ch,
                                               struct color   *c);
    
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct image     *ttf_text_blended     (struct ttf     *ttf,
                                               const char     *text,
                                               struct color   *c);
    
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct image     *ttf_utf8_blended     (struct ttf     *ttf,
                                               const char     *utf8,
                                               struct color   *c);
    
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              ttf_delete          (struct ttf      *iptr);

/* -------------------------------------------------------------------------- *
 * Loose all references                                                       *
 * -------------------------------------------------------------------------- */
extern void              ttf_release         (struct ttf      *iptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              ttf_set_name        (struct ttf      *iptr,
                                              const char      *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern const char       *ttf_get_name        (struct ttf      *iptr);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct ttf       *ttf_find_name       (const char      *name);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct ttf       *ttf_find_id         (uint32_t         id);

/* -------------------------------------------------------------------------- *
 * Dump ttfers and ttf heap.                                            *
 * -------------------------------------------------------------------------- */
extern void              ttf_dump            (struct ttf      *iptr);

#endif /* HAVE_FT2 */

#endif /* LIB_TTF_H */
