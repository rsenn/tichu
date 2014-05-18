/* $Id: font.c,v 1.13 2005/05/19 23:08:32 smoli Exp $
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

#include <stdlib.h>
#include <string.h>
#ifndef PATH_MAX
#include <limits.h>
#endif

#include <libsgui/png.h>
#include <libsgui/font.h>
#include <libsgui/list.h>
#include <libsgui/common.h>

static sgList fonts;

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgInitFonts() 
{  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
sgFont *sgLoadFontFile(const char *file)
{
  SDL_Surface *surface;
  sgFont *ret;
  
  if(!(surface = sgLoadPngFile(file)))
    return NULL;

  ret = sgAddFont(surface);
  SDL_FreeSurface(surface);
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
sgFont *sgLoadFontRWops(SDL_RWops *rwops)
{
  SDL_Surface *surface;
  sgFont *ret;
  
  if(!(surface = sgLoadPngRWops(rwops)))
    return NULL;

  ret = sgAddFont(surface);
  SDL_FreeSurface(surface);
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
sgFont *sgLoadFontFp(FILE *fp)
{
  SDL_Surface *surface;
  sgFont *ret;
  
  if(!(surface = sgLoadPngFp(fp, 0)))
    return NULL;

  ret = sgAddFont(surface);
  SDL_FreeSurface(surface);
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
sgFont *sgAddFont(SDL_Surface *font)
{
  sgFont  *ret;
  int      x;
  int      i, idx;
  SDL_Rect rc;
 
  if(font->format->palette == NULL ||
     font->format->BitsPerPixel != 8)
  {
    sgLog("Fonts must be 8-bit grayscale or paletted");
    return NULL;
  }
  
  ret = malloc(sizeof(sgFont));
  
  sgAddList(&fonts, &ret->node, ret);
  
  ret->font = font;
  
  SDL_LockSurface(font);
  
  memset(ret->rects, 0, sizeof(ret->rects));
  
  rc.y = 0;
  rc.h = font->h;
  
  for(idx = 32, i = 0, x = 0; x < font->w && idx < 256; i++, idx++)
  {
    int value;
    
    if(idx == 128)
      idx = 160;
    
    value = 255 - sgGetPixel8(font, x, 0);
    sgPutPixel8(font, x, 0, 0);
    
    rc.x = x;
    rc.w = value;
    
    ret->rects[idx] = rc;
    
    x += value;
  }
  
  SDL_UnlockSurface(font);

  ret->color.r = 0xff;
  ret->color.g = 0xff;
  ret->color.b = 0xff;
  ret->color.unused = 0xff;
  
  sgConvertFont(ret);
  
  return ret;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgConvertFont(sgFont *font)
{
  Uint32 *pixels;
  int x, y;
  SDL_Surface *temp;
  
  temp = SDL_DisplayFormatAlpha(font->font);
  
  SDL_LockSurface(temp);
  
  pixels = (Uint32 *)temp->pixels;

  for(y = 0; y < temp->h; y++)
  {
    for(x = 0; x < temp->w; x++)
    {
      pixels[x] = (font->color.r << RSHIFT) | \
                  (font->color.g << GSHIFT) | \
                  (font->color.b << BSHIFT) | ((pixels[x] & RMASK) >> RSHIFT) << ASHIFT;
    }
    
    pixels += temp->pitch >> 2;
  }

  SDL_UnlockSurface(temp);

/*  SDL_FreeSurface(font->font);*/
  font->font = temp;  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgFreeFont(sgFont *font) 
{  
  SDL_FreeSurface(font->font);
  
  sgDeleteList(&fonts, &font->node);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgFreeFonts() 
{  
  sgFont *font;
  sgFont *next;

  sgForeachSafe(&fonts, font, next)
    sgFreeFont(font);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int sgTextWidth(sgFont *font, const char *text)
{
  int width = 0;
  
  for(; *text; text++)
  {
    width += font->rects[(unsigned int)(unsigned char)*text].w;
  }
  
  return width;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int sgTextHeight(sgFont *font, const char *text)
{
  return font->font->h;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int sgFontHeight(sgFont *font)
{
  return font->font->h;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgDrawChar(sgFont *font, SDL_Color *color, SDL_Surface *dest, Uint16 x, 
                Uint16 y, char c) 
{
  
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgDrawText(sgFont *font, SDL_Color *color, SDL_Surface *dest, 
                SDL_Rect *rect, int align, const char *text) 
{ 
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgDrawTextOutline(sgFont *font, SDL_Surface *dest, SDL_Rect *rect, 
                       int align, const char *text)
{   
  
  SDL_Rect textrect;
  
  if(!rect)
    rect = &dest->clip_rect;
  
  textrect.w = sgTextWidth(font, text);
  textrect.h = sgTextHeight(font, text);
  
  sgAlignRect(rect, &textrect, align);
  
  for(; *text; text++)
  {
    SDL_Rect *r = &font->rects[(unsigned int)(unsigned char)*text];
    
    if(r->w)
    {
      sgColor c = { 0,0,0,255 };
        
      textrect.w = r->w;
      
      sgBlur(font->font, r, dest, &textrect, c);
      SDL_BlitSurface(font->font, r, dest, &textrect);
      textrect.x += r->w;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
sgFont *sgGetFont(const char *name) 
{
  sgFont *font;

  sgForeach(&fonts, font)
  {
    if(!strcmp(font->name, name))
      return font;
  }

  return NULL;
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
sgFont *sgGetFirstFont() 
{
  return (sgFont *)fonts.head;
}
  
