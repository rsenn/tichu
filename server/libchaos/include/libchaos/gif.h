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
 * $Id: gif.h,v 1.10 2005/01/17 19:09:50 smoli Exp $
 */

#ifndef GIF_H

/* -------------------------------------------------------------------------- *
 * Library headers                                                            *
 * -------------------------------------------------------------------------- */
#include <libchaos/defs.h>

#ifdef HAVE_SDL
#include <SDL.h>
#endif

/* -------------------------------------------------------------------------- *
 * Constants                                                                  *
 * -------------------------------------------------------------------------- */
/* File identification */
#define GIF_MAGIC           "GIF89a"        
#define GIF_MAGIC_LEN       6

/* Hashtable constants and macros */
#define GIF_HT_SIZE         8192
#define GIF_HT_KEY_MASK     0x1FFF
#define GIF_HT_KEY_NUM_BITS 13 
#define GIF_HT_MAX_KEY      8191
#define GIF_HT_MAX_CODE     4095

#define GIF_HT_GET_KEY(x)   ((x) >> 12)
#define GIF_HT_GET_CODE(x)  ((x) & 0x0FFF)
#define GIF_HT_PUT_KEY(x)   ((x) << 12)
#define GIF_HT_PUT_CODE(x)  ((x) & 0x0FFF)

/* LZW specific values */
#define GIF_LZ_MAX_CODE     4095
#define GIF_LZ_BITS         12

/* LZW special codes */
#define GIF_FLUSH_OUTPUT    4096
#define GIF_FIRST_CODE      4097
#define GIF_NO_SUCH_CODE    4098

/* GIF status */
#define GIF_STATE_WRITE     0x01
#define GIF_STATE_SCREEN    0x02
#define GIF_STATE_IMAGE     0x04
#define GIF_STATE_READ      0x08

/* Extension types */
#define GIF_EXT_COMMENT     0xfe
#define GIF_EXT_GRAPHICS    0xf9
#define GIF_EXT_PLAINTEXT   0x01
#define GIF_EXT_APPLICATION 0xff

#define GIF_READ            GIF_STATE_READ
#define GIF_WRITE           GIF_STATE_WRITE

/* -------------------------------------------------------------------------- *
 * Types                                                                      *
 * -------------------------------------------------------------------------- */
#ifndef COLOR_TYPE
struct color {
  uint8_t               r;
  uint8_t               g;
  uint8_t               b;
  uint8_t               a;
};
#define COLOR_TYPE
#endif /* COLOR_TYPE */

#ifndef PALETTE_TYPE
struct palette {
  int                   count;
  int                   bpp;
  struct color         *colors;
};
#define PALETTE_TYPE
#endif /* PALETTE_TYPE */

/* Describes a single image */
struct gif_descriptor {
  int16_t               left;
  int16_t               top;
  uint16_t              width;
  uint16_t              height;
  int                   interlace;
  struct palette       *palette;
};

/* Hastable for LZW codes */
struct gif_hashtable {
  unsigned long         htable[GIF_HT_SIZE];
};

/* Internal GIF stuff, mainly for LZW compression */
struct gif_lzw { 
  int                   handle;
  int                   clear_code;
  int                   eof_code;
  int                   running_code;
  int                   running_bits;
  int                   max_code1;
  int                   last_code;
  int                   crnt_code;
  int                   stack_ptr;
  int                   shift_state;
  uint32_t              shift_dword;
  uint32_t              code_size;
  uint32_t              pixel_count;
  uint8_t               buf[256];
  uint8_t               stack[GIF_LZ_MAX_CODE];
  uint8_t               suffix[GIF_LZ_MAX_CODE + 1];
  uint32_t              prefix[GIF_LZ_MAX_CODE + 1];
  struct gif_hashtable *table;
};

/* GIF extension header */
struct gif_ext { 
  struct node           node;
  uint32_t              size;
  uint8_t              *buf;
  uint32_t              type;
};

/* An actual image */
struct gif_image {
  struct node           node;
  struct gif_descriptor desc;
  uint8_t              *bits;
  struct list           exts;
  uint32_t              type;
};

/* A GIF instance */
struct gif {
  struct node           node;
  uint32_t              id;
  uint32_t              refcount;
  uint32_t              nhash;
  uint32_t              status;
  int                   fd;
  uint8_t              *mem;
  size_t                size;
  size_t                offset;
  uint16_t              width;
  uint16_t              height;
  int                   bpp;
  int                   resolution;
  int                   background;
  struct palette       *palette;        /* The global palette */
  struct gif_descriptor desc;           /* Global image descriptor */
  struct list           images;
  struct gif_lzw        lzw;            /* Private structural data */
  char                  name[PATH_MAX];
};

enum gif_record {
  GIF_RECORD_UNDEFINED   = 0,
  GIF_RECORD_SCREEN_DESC = 1,
  GIF_RECORD_IMAGE_DESC  = 2,
  GIF_RECORD_EXTENSION   = 3,
  GIF_RECORD_TERMINATE   = 4
};

/* -------------------------------------------------------------------------- *
 * Initialize GIF code                                                        *
 * -------------------------------------------------------------------------- */
extern void              gif_init             (void);

/* -------------------------------------------------------------------------- *
 * Shut down GIF code                                                         *
 * -------------------------------------------------------------------------- */
extern void              gif_shutdown         (void);

/* -------------------------------------------------------------------------- *
 * Create GIF instance                                                        *
 * -------------------------------------------------------------------------- */
extern struct gif       *gif_new              (const char      *name,
                                               int              state);

/* -------------------------------------------------------------------------- *
 * Open GIF file                                                              *
 * -------------------------------------------------------------------------- */
extern struct gif       *gif_open             (const char      *filename, 
                                               int              state);

/* -------------------------------------------------------------------------- *
 * Open GIF filedescriptor                                                    *
 * -------------------------------------------------------------------------- */
extern struct gif       *gif_open_fd          (int              fd,
                                               int              state);

/* -------------------------------------------------------------------------- *
 * Open GIF from/to mem                                                       *
 * -------------------------------------------------------------------------- */
extern struct gif       *gif_open_mem         (void            *mem,
                                               size_t           n,
                                               int              state);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int               gif_slurp            (struct gif      *gif);
  
/* --------------------------------------------------------------------------- *
 * Clear structural data                                                       *
 * --------------------------------------------------------------------------- */
extern void              gif_clear_struct     (struct gif      *gif);
/* --------------------------------------------------------------------------- *
 * Clear raw data                                                              *
 * --------------------------------------------------------------------------- */
extern void              gif_clear_raw        (struct gif       *gif);
/* --------------------------------------------------------------------------- *
 * Clear all                                                                   *
 * --------------------------------------------------------------------------- */
extern void              gif_clear            (struct gif       *gif);
/* --------------------------------------------------------------------------- *
 * Terminate and close file                                                    *
 * --------------------------------------------------------------------------- */
extern void              gif_close            (struct gif       *gif);

/* --------------------------------------------------------------------------- *
 * --------------------------------------------------------------------------- */
extern void              gif_delete           (struct gif       *gif);

/* --------------------------------------------------------------------------- *
 * Get screen descriptor                                                       *
 * --------------------------------------------------------------------------- */
extern int               gif_screen_get       (struct gif       *gif);

/* --------------------------------------------------------------------------- *
 * Put screen descriptor                                                       *
 * --------------------------------------------------------------------------- */
extern int               gif_screen_put       (struct gif       *gif,       
                                               uint16_t          width,
                                               uint16_t          height,    
                                               int               resolution,
                                               uint8_t           background, 
                                               struct palette   *pal);
/* --------------------------------------------------------------------------- *
 * Get an image                                                                *
 * --------------------------------------------------------------------------- */
extern int               gif_image_get        (struct gif       *gif);

/* --------------------------------------------------------------------------- *
 * Put an image                                                                *
 * --------------------------------------------------------------------------- */
extern int               gif_image_put        (struct gif       *gif,
                                               int16_t           left, 
                                               int16_t           top,
                                               uint16_t          width,
                                               uint16_t          height,
                                               int               interlace,
                                               struct palette   *pal);

/* --------------------------------------------------------------------------- *
 * Get data from current image                                                 *
 * --------------------------------------------------------------------------- */
extern int               gif_data_get         (struct gif       *gif,
                                               uint8_t          *data, 
                                               uint32_t          len);

/* --------------------------------------------------------------------------- *
 * Put data into current image                                                 *
 * --------------------------------------------------------------------------- */
extern int               gif_data_put         (struct gif       *gif,
                                               uint8_t          *data,
                                               uint32_t          len);  

/* -------------------------------------------------------------------------- *
 * Create an empty palette                                                    *
 * -------------------------------------------------------------------------- */
extern struct palette   *gif_palette_new      (uint32_t          ncolors);

/* -------------------------------------------------------------------------- *
 * Create an initialised palette                                              *
 * -------------------------------------------------------------------------- */
extern struct palette   *gif_palette_make     (uint32_t          ncolors,
                                               struct color     *colors);

/* -------------------------------------------------------------------------- *
 * Copy a palette                                                             *
 * -------------------------------------------------------------------------- */
extern struct palette   *gif_palette_copy     (struct palette   *pal);

/* -------------------------------------------------------------------------- *
 * Free a palette                                                             *
 * -------------------------------------------------------------------------- */
extern void              gif_palette_free     (struct palette   *palette);


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern struct gif_image *gif_image_add        (struct gif       *gif,    
                                               int16_t           left,
                                               int16_t           top, 
                                               uint16_t          width,
                                               uint16_t          height, 
                                               int               interlace,
                                               struct palette   *pal);


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              gif_image_delete     (struct gif       *gif, 
                                               struct gif_image *image);
 
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern void              gif_image_clear      (struct gif       *gif);
 
/* -------------------------------------------------------------------------- *
 * Add an extension to an image header                                        *
 * -------------------------------------------------------------------------- */
extern struct gif_ext   *gif_extension_add    (struct gif_image *image, 
                                               uint32_t          len,
                                               uint8_t          *ext);

/* -------------------------------------------------------------------------- *
 * Remove an extension from an image header                                   *
 * -------------------------------------------------------------------------- */
extern void              gif_extension_delete (struct gif_image *image, 
                                               struct gif_ext   *ext);

/* -------------------------------------------------------------------------- *
 * Remove all extensions from an image header                                 *
 * -------------------------------------------------------------------------- */
extern void              gif_extension_clear  (struct gif_image *image);

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int               gif_get_gfx_control  (struct gif_image *image,
                                               int              *disposal,
                                               int              *user_input,
                                               int              *trans,
                                               uint16_t         *delay);
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int               gif_put_gfx_control  (struct gif       *gif,
                                               int               disposal,
                                               int               user_input, 
                                               int               trans,
                                               uint16_t          delay);
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
extern int               gif_save             (struct gif       *gif);
      
#endif /* GIF_H */
