/* $Id: png.c,v 1.6 2005/05/13 00:36:58 smoli Exp $
 * ------------------------------------------------------------------------- *
 *                     /                                                     *
 *      ___  ___                                                             *
 *     |___ |   )|   )|        Simple and smooth GUI library :)              *
 *      __/ |__/ |__/ |        Copyright (C) 2003-2005  Roman Senn           *
 *          __/                                                              *
 *                                                                           *
 *  This library is free software; you can redistribute it and/or            *
 *  modify it under the terms of the GNU Library General Public              *
 *  License as published by the Free Software Foundation; either             *
 *  version 2 of the License, or (at your option) any later version.         *
 *                                                                           *
 *  This library is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  Library General Public License for more details.                         *
 *                                                                           *
 *  You should have received a copy of the GNU Library General Public        *
 *  License along with this library; if not, write to the Free               *
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA *
 * ------------------------------------------------------------------------- */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <SDL.h>

/** @weakgroup sgPng
 *  @{
 */

#include <libsgui/png.h>
#include <libsgui/file.h>
#include <libsgui/common.h>

/* Load a PNG type image from memory */
SDL_Surface *sgLoadPngMemory(const void *mem, Uint32 size)
{
  SDL_Surface *surface;
  SDL_RWops *rwops = SDL_RWFromConstMem(mem, size);
  
  if(rwops == NULL)
    return NULL;
  
  surface = sgLoadPngRWops(rwops);
  SDL_FreeRW(rwops);
  
  return surface;
}

/* Load a PNG type image from a file */
SDL_Surface *sgLoadPngFile(const char *file)
{
  SDL_Surface *surface;
  SDL_RWops *rwops = sgOpenFileRWops(file, "rb");
  
  if(rwops == NULL)
    return NULL;
  
  surface = sgLoadPngRWops(rwops);
  SDL_FreeRW(rwops);
  
  return surface;
}

/* Load a PNG type image from a stdio file pointer */
SDL_Surface *sgLoadPngFp(FILE *fp, int autoclose)
{
  SDL_Surface *surface;
  SDL_RWops *rwops = SDL_RWFromFP(fp, autoclose);
  
  if(rwops == NULL)
    return NULL;
  
  surface = sgLoadPngRWops(rwops);
  SDL_FreeRW(rwops);
  
  return surface;
}

/* sgReadPngData and sgLoadPngRWops are stolen from SDL_image
 * which is Copyright (C) by Sam Lantinga, Philippe Lavoie 
 * ------------------------------------------------------------------------- */

/* Load a PNG type image from an SDL datasource */
static void sgReadPngData(png_structp ctx, png_bytep area, png_size_t size)
{
  SDL_RWops *src;
  
  src = (SDL_RWops *)png_get_io_ptr(ctx);
  SDL_RWread(src, area, size, 1);
}

/* Loads png from RWops into SDL_Surface */
SDL_Surface *sgLoadPngRWops(SDL_RWops *src)
{  
  SDL_Surface *volatile surface;
  png_structp png_ptr;
  png_infop info_ptr;
  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;
  Uint32 Rmask;
  Uint32 Gmask;
  Uint32 Bmask;
  Uint32 Amask;
  SDL_Palette *palette;
  png_bytep *volatile row_pointers;
  int row, i;
  volatile int ckey = -1;
  png_color_16 *transv;
  
  if(!src)
  {
    /* The error message has been set in SDL_RWFromFile */
    return NULL;
  }

  /* Initialize the data we will clean up when we're done */
  png_ptr = NULL; info_ptr = NULL; row_pointers = NULL; surface = NULL;
  
  /* Create the PNG loading context structure */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                   NULL, NULL, NULL);
  if(png_ptr == NULL)
  {
    sgLog("Couldn't allocate memory for PNG file or incompatible PNG dll");
    goto done;
  }

  /* Allocate/initialize the memory for image information.  REQUIRED. */
  info_ptr = png_create_info_struct(png_ptr);
  if(info_ptr == NULL)
  {
    sgLog("Couldn't create image information for PNG file");
    goto done;
  }

  /* Set error handling if you are using setjmp/longjmp method (this is
   * the normal method of doing things with libpng).  REQUIRED unless you
   * set up your own error handlers in png_create_read_struct() earlier.
   */
  if(setjmp(png_ptr->jmpbuf))
  {
    sgLog("Error reading the PNG file.");
    goto done;
  }
  
  /* Set up the input control */
  png_set_read_fn(png_ptr, src, sgReadPngData);
  
  /* Read PNG header info */
  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
               &color_type, &interlace_type, NULL, NULL);
  
  /* tell libpng to strip 16 bit/color files down to 8 bits/color */
  png_set_strip_16(png_ptr) ;
  
  /* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
   * byte into separate bytes (useful for paletted and grayscale images).
   */
  png_set_packing(png_ptr);
  
  /* scale greyscale values to the range 0..255 */
  if(color_type == PNG_COLOR_TYPE_GRAY)
    png_set_expand(png_ptr);
  
  png_set_bgr(png_ptr);
  
  /* For images with a single "transparent colour", set colour key;
   * if more than one index has transparency, or if partially transparent
   * entries exist, use full alpha channel */
  if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
  {
    int num_trans;
    Uint8 *trans;
    png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans,
                 &transv);
    if(color_type == PNG_COLOR_TYPE_PALETTE)
    {
      /* Check if all tRNS entries are opaque except one */
      int i, t = -1;

      for(i = 0; i < num_trans; i++)
      {
        if(trans[i] == 0)
        {
          if(t >= 0)
            break;
          t = i;
        }
        else if(trans[i] != 255)
          break;
      }
      
      if(i == num_trans)
      {
        /* exactly one transparent index */
        ckey = t;
      }
       else
      {
        /* more than one transparent index, or translucency */
        png_set_expand(png_ptr);
      }
    }
    else
    {
      ckey = 0; /* actual value will be set later */
    }
  }
  
  if(color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_ptr);
  
  png_read_update_info(png_ptr, info_ptr);
  
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
               &color_type, &interlace_type, NULL, NULL);
  
  /* Allocate the SDL surface to hold the image */
  Rmask = Gmask = Bmask = Amask = 0;

  if(color_type != PNG_COLOR_TYPE_PALETTE)
  {
/*    if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
    {
      Rmask = 0x000000FF;
      Gmask = 0x0000FF00;
      Bmask = 0x00FF0000;
      Amask = (info_ptr->channels == 4) ? 0xFF000000 : 0;
    }
    else
    {
      int s = (info_ptr->channels == 4) ? 0 : 8;
      Rmask = 0xFF000000 >> s;
      Gmask = 0x00FF0000 >> s;
      Bmask = 0x0000FF00 >> s;
      Amask = 0x000000FF >> s;
    }*/
    Rmask = RMASK;
    Gmask = GMASK;
    Bmask = BMASK;
    Amask = AMASK;
  }
  
  surface = SDL_AllocSurface(SDL_SWSURFACE, width, height,
                             bit_depth * info_ptr->channels, Rmask, Gmask, Bmask, Amask);
  if(surface == NULL)
  {
    sgLog("Out of memory");
    goto done;
  }
  
  if(ckey != -1)
  {
    if(color_type != PNG_COLOR_TYPE_PALETTE)
      /* FIXME: Should these be truncated or shifted down? */
      ckey = SDL_MapRGB(surface->format,
                        (Uint8)transv->red,
                        (Uint8)transv->green,
                        (Uint8)transv->blue);
    SDL_SetColorKey(surface, SDL_SRCCOLORKEY, ckey);
  }
  
  /* Create the array of pointers to image data */
  row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
  
  if((row_pointers == NULL))
  {
    sgLog("Out of memory");
    SDL_FreeSurface(surface);
    surface = NULL;
    goto done;
  }
  
  for(row = 0; row < (int)height; row++)
  {
    row_pointers[row] = (png_bytep)
      (Uint8 *)surface->pixels + row*surface->pitch;
  }
  
    
  /* Read the entire image in one go */
  png_read_image(png_ptr, row_pointers);
  
  /* and we're done!  (png_read_end() can be omitted if no processing of
   * post-IDAT text/time/etc. is desired)
   * In some cases it can't read PNG's created by some popular programs (ACDSEE),
   * we do not want to process comments, so we omit png_read_end
   * 
   *   png_read_end(png_ptr, info_ptr);
   */
    
  /* Load the palette, if any */
  palette = surface->format->palette;
  
  if(palette)
  {
    if(color_type == PNG_COLOR_TYPE_GRAY)
    {
      palette->ncolors = 256;
      
      for(i = 0; i < 256; i++)
      {
        palette->colors[i].r = i;
        palette->colors[i].g = i;
        palette->colors[i].b = i;
      }
    }
    else if(info_ptr->num_palette > 0)
    {
      palette->ncolors = info_ptr->num_palette;
      
      for(i = 0; i < info_ptr->num_palette; i++)
      {
        palette->colors[i].b = info_ptr->palette[i].blue;
        palette->colors[i].g = info_ptr->palette[i].green;
        palette->colors[i].r = info_ptr->palette[i].red;
      }
    }
  }
  
done: /* Clean up and return */
  png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : (png_infopp)0,
                          (png_infopp)0);
  if(row_pointers)
    free(row_pointers);
  
  return surface;
}

/* Save a surface to memory as PNG image */
int sgSavePngMemory(SDL_Surface *surface, void *mem, Uint32 size)
{
  int ret;
  SDL_RWops *rwops = SDL_RWFromMem(mem, size);
  
  if(rwops == NULL)
    return -1;

  ret = sgSavePngRWops(surface, rwops);
  
  SDL_FreeRW(rwops);
  
  return ret;
}

/* Save a surface to a PNG file */
int sgSavePngFile(SDL_Surface *surface, const char *file)
{
  int ret;
  SDL_RWops *rwops = SDL_RWFromFile(file, "wb");
  
  if(rwops == NULL)
    return -1;
  
  ret = sgSavePngRWops(surface, rwops);
  
  SDL_FreeRW(rwops);

  return ret;
}

/* Save a surface to a stdio file pointer */
int sgSavePngFp(SDL_Surface *surface, FILE *fp, int autoclose)
{
  int ret;
  SDL_RWops *rwops = SDL_RWFromFP(fp, autoclose);
  
  if(rwops == NULL)
    return -1;
  
  ret = sgSavePngRWops(surface, rwops);
  SDL_FreeRW(rwops);
  
  return ret;
}

/* Write PNG image data to an SDL rwops */
static void sgWritePngData(png_structp ctx, png_bytep area, png_size_t size)
{
  SDL_RWops *rwops;
  
  rwops = (SDL_RWops *)png_get_io_ptr(ctx);
  SDL_RWwrite(rwops, area, size, 1);
}

static void sgTransformPngData(png_structp png_ptr, png_row_infop row_info, png_bytep data)
{
  SDL_Surface *surface;
  SDL_PixelFormat *format;
  
  if(png_ptr == NULL)return;
  
  /* contents of row_info:
   *  png_uint_32 width      width of row
   *  png_uint_32 rowbytes   number of bytes in row
   *  png_byte color_type    color type of pixels
   *  png_byte bit_depth     bit depth of samples
   *  png_byte channels      number of channels (1-4)
   *  png_byte pixel_depth   bits per pixel (depth*channels)
   */
  
  surface = (SDL_Surface *)png_get_user_transform_ptr(png_ptr);  
  format = surface->format;
  
  row_info->pixel_depth = format->BitsPerPixel;
  row_info->rowbytes = surface->w * format->BytesPerPixel;  
}


/* Saves png from SDL_Surface into RWops */
int sgSavePngRWops(SDL_Surface *surface, SDL_RWops *rwops)
{  
  png_structp png_ptr;
  png_infop info_ptr;
  png_color_8 sig_bit;
  png_uint_32 width, height;
  int bit_depth, color_type;
  SDL_Palette *palette;
  SDL_Surface *freesf = NULL;
  SDL_PixelFormat fmt;
  png_bytep *volatile row_pointers;
  int row, i;
  int ret = 0;
  /* FIXME: color keying and transparency not implemented */
/*  volatile int ckey = -1;
  png_color_16 *transv;*/
  union {
    Uint32 masks;
    Uint8  maskbuf[4];
  } m;
  
  SDL_LockSurface(surface);
  
  memcpy(&fmt, surface->format, sizeof(SDL_PixelFormat));
  palette = fmt.palette;

  /* Initialize the data we will clean up when we're done */
  png_ptr = NULL; info_ptr = NULL; row_pointers = NULL;
  
  /* Create the PNG loading context structure */
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                    NULL, NULL, NULL);
  if(png_ptr == NULL)
  {
    sgLog("Couldn't allocate memory for PNG file or incompatible PNG dll");
    goto done;
  }

  /* Allocate/initialize the memory for image information.  REQUIRED. */
  info_ptr = png_create_info_struct(png_ptr);
  if(info_ptr == NULL)
  {
    sgLog("Couldn't create image information for PNG file");
    goto done;
  }

  /* Set error handling if you are using setjmp/longjmp method (this is
   * the normal method of doing things with libpng).  REQUIRED unless you
   * set up your own error handlers in png_create_read_struct() earlier.
   */
  if((ret = setjmp(png_ptr->jmpbuf)))
  {
    sgLog("Error reading the PNG file.");
    goto done;
  }
  
  /* set the zlib compression level */
  png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
  
  /* set other zlib parameters */
  png_set_compression_mem_level(png_ptr, 8);
  png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
  png_set_compression_window_bits(png_ptr, 15);
  png_set_compression_method(png_ptr, 8);
  png_set_compression_buffer_size(png_ptr, 8192);
  
  /* Setting the contents of info for output */
  width = surface->w;
  height = surface->h;
  
  if(palette)
  {
    /* for paletted images, get a bit depth that fits all colors */
    bit_depth = 1;
    
    while((1 << bit_depth) < palette->ncolors)
      bit_depth <<= 1;
    
    color_type = PNG_COLOR_TYPE_PALETTE;
  }
  else
  {
    /* otherwise its always 8-bit per channel RGB(A) */
    bit_depth = 8;
    
    color_type = (fmt.Amask ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB);
  }
  
  png_set_IHDR(png_ptr, info_ptr, width, height,
               bit_depth, color_type, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  /* parepare the palette */
  if(palette)
  {
    png_colorp colors = malloc(sizeof(png_color) * palette->ncolors);
    
    for(i = 0; i < palette->ncolors; i++)
    {
      colors[i].red = palette->colors[i].r;
      colors[i].green = palette->colors[i].g;
      colors[i].blue = palette->colors[i].b;
    }
    
    png_set_PLTE(png_ptr, info_ptr, colors, palette->ncolors);
    
    if(bit_depth < 8)
      png_set_packing(png_ptr);
  }
    
  /* Set up the output control */
  png_set_write_fn(png_ptr, rwops, sgWritePngData, NULL);
  
  /* Write the image */
  png_write_info(png_ptr, info_ptr);
  
  if(palette == NULL)
  {
    /* Convert RGB formats with a depth smaller than 24-bit */
    if(fmt.BytesPerPixel < 3)
    {
      fmt.BytesPerPixel = 4;
      fmt.BitsPerPixel = 32;
      fmt.Rmask = RMASK;
      fmt.Gmask = GMASK;
      fmt.Bmask = BMASK;
      if(fmt.Amask)
        fmt.Amask = AMASK;
      fmt.Rloss = 0; fmt.Gloss = 0; fmt.Bloss = 0; fmt.Aloss = 0;
      fmt.Rshift = RSHIFT; fmt.Gshift = GSHIFT; fmt.Bshift = BSHIFT; fmt.Ashift = ASHIFT;
      
      if((freesf = SDL_ConvertSurface(surface, &fmt, SDL_SWSURFACE)))
      {
        SDL_UnlockSurface(surface);
        surface = freesf;
      }
    }

    m.masks = fmt.Rmask|fmt.Gmask|fmt.Bmask|fmt.Amask;
  
    /* When we're in RGB (not paletted) mode, and there is no alpha that
       means we must reduce 32-bit pixels to 24-bit
       (SDL always stores RGB > 16-bit as 32-bit integer) */
      /* check where the filler (unused byte) is */
    if(m.maskbuf[0] == 0)
      png_set_filler(png_ptr, 0, PNG_FILLER_BEFORE);
    
    if(m.maskbuf[3] == 0)
      png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
  
    /* detect BGR colors and swap channels accordingly */
    m.masks = fmt.Rmask;
    
    if(m.maskbuf[2] || m.maskbuf[3])
      png_set_bgr(png_ptr);    
    
    /* now, if we have loss, set the shift count */
    if(fmt.Rloss|fmt.Gloss|fmt.Bloss|fmt.Aloss)
    {
      sig_bit.red = 8 - fmt.Rloss;
      sig_bit.green = 8 - fmt.Gloss;
      sig_bit.blue = 8 - fmt.Bloss;
      sig_bit.alpha = 8 - fmt.Aloss;
      png_set_shift(png_ptr, &sig_bit);
    }
    
    png_set_write_user_transform_fn(png_ptr, sgTransformPngData);
    png_set_user_transform_info(png_ptr, (void *)surface, 0, 0);
  }
  
  /* Create the array of pointers to image data */
  row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
  
  if((row_pointers == NULL))
  {
    sgLog("Out of memory");
    goto done;
  }
  
  for(row = 0; row < (int)height; row++)
  {
    row_pointers[row] = (png_bytep)
      (Uint8 *)surface->pixels + row * surface->pitch;
  }
    
  png_set_rows(png_ptr, info_ptr, row_pointers);

  png_write_image(png_ptr, row_pointers);
  
  png_write_end(png_ptr, info_ptr);
  
    
done: /* Clean up and return */
  png_destroy_write_struct(&png_ptr, info_ptr ? &info_ptr : (png_infopp)0);
    
  if(freesf)
    SDL_FreeSurface(freesf);
  else
    SDL_UnlockSurface(surface);
  
  if(row_pointers)
    free(row_pointers);
  
  return ret;
}


/** @} */
