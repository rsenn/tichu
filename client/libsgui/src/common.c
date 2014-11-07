/* $Id: common.c,v 1.18 2005/05/19 23:08:32 smoli Exp $
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

/** @weakgroup sgCommon
 *  @{
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <SDL.h>

#include <libsgui/common.h>

/* pixel format for 32-bit BGRA surfaces */
SDL_PixelFormat sgPixelFormat32 =
{
  .palette = NULL,
  .BitsPerPixel = 32,
  .BytesPerPixel = 4,
  .Rloss = 0, .Gloss = 0, .Bloss = 0, .Aloss = 0,
  .Rshift = RSHIFT, .Gshift = GSHIFT, .Bshift = BSHIFT, .Ashift = ASHIFT,
  .Rmask  = RMASK,  .Gmask  = GMASK,  .Bmask  = BMASK,  .Amask  = AMASK,
  .colorkey = 0,
  .alpha = 0xff,
};

/* Writes a log entry
 * -------------------------------------------------------------------------- */
void sgLog(const char *msg, ...)
{
  va_list args;
  
  va_start(args, msg);
  
  fprintf(stderr, ">>> GUI: ");
  vfprintf(stderr, msg, args);
  fputc('\n', stderr);
  fflush(stderr);
  
  va_end(args);
}

/* Dumps rectangle dimensions to the log
 * -------------------------------------------------------------------------- */
void sgDumpRect(SDL_Rect *rect)
{
  sgLog("Rectangle: %i %i %u %u", rect->x, rect->y, rect->w, rect->h);
}

/* Dumps color values to the log
 * -------------------------------------------------------------------------- */
void sgDumpColor(SDL_Color *c)
{
  sgLog("Color: %u %u %u %u", c->r, c->g, c->b, c->unused);
}

/* Fades from one surface to another in a specified time
 * -------------------------------------------------------------------------- */
void sgFade(SDL_Surface *screen, SDL_Surface *from, 
            SDL_Surface *to, Uint32 duration)
{
  SDL_Surface *f, *t;
  Uint32 start;
  int alpha;
  
  f = SDL_DisplayFormat(from);
  t = SDL_DisplayFormat(to);
  
  start = SDL_GetTicks();
  
  do
  {
    alpha = (SDL_GetTicks() - start) * 255 / duration;
    SDL_SetAlpha(t, SDL_SRCALPHA, alpha < 255 ? alpha : 255);
    
    SDL_BlitSurface(f, NULL, screen, NULL);
    SDL_BlitSurface(t, NULL, screen, NULL);
    
    SDL_Flip(screen);
    SDL_Delay(10);
    
  } while(alpha < 255);
  
  SDL_FreeSurface(f);
  SDL_FreeSurface(t);
}

/* Copies from one RGBA surface to another
 * -------------------------------------------------------------------------- */
void sgCopy(SDL_Surface *src, SDL_Rect *sr,
            SDL_Surface *dst, SDL_Rect *dr)
{
  SDL_Rect srect;
  SDL_Rect drect;
  Uint32 *srow;
  Uint32 *drow;
  Uint16 y;
    
  srect = (sr ? *sr : src->clip_rect);
  drect = (dr ? *dr : dst->clip_rect);
    
  sgClip(src, &srect);
  sgClip(dst, &drect);
  sgIntersect(&srect, &drect);
 
  SDL_LockSurface(src);
  SDL_LockSurface(dst);

  srow = (Uint32 *)src->pixels + ((srect.y * src->pitch) >> 2) + srect.x;
  drow = (Uint32 *)dst->pixels + ((drect.y * dst->pitch) >> 2) + drect.x;
    
  for(y = 0; y < srect.h; y++)
  {
    memcpy(drow, srow, srect.w * src->format->BytesPerPixel);

    srow += src->pitch >> 2;
    drow += dst->pitch >> 2;
  }
  
  SDL_UnlockSurface(src);
  SDL_UnlockSurface(dst);
}

/* Copies an 8-bit paletted surface to a 32-bit RGBA one by using the 
 * per-surface-alpha of the source surface
 * -------------------------------------------------------------------------- */
void sgCopyPalette(SDL_Surface *src, SDL_Rect *sr,
                   SDL_Surface *dst, SDL_Rect *dr)
{
  SDL_Rect srect;
  SDL_Rect drect;
  Uint8 *srow;
  Uint32 *drow;
  Uint16 y;
  Uint16 x;
  SDL_Color *palette;
    
  srect = (sr ? *sr : src->clip_rect);
  drect = (dr ? *dr : dst->clip_rect);
    
  sgClip(src, &srect);
  sgClip(dst, &drect);
  sgIntersect(&srect, &drect);
 
  SDL_LockSurface(src);
  SDL_LockSurface(dst);

  palette = src->format->palette->colors;
  
  srow = (Uint8 *)src->pixels + (srect.y * src->pitch) + srect.x;
  drow = (Uint32 *)dst->pixels + ((drect.y * dst->pitch) >> 2) + drect.x;
    
  for(y = 0; y < srect.h; y++)
  {
    for(x = 0; x < srect.w; x++)
    {
      if(srow[x] != src->format->colorkey)
        drow[x] = (palette[srow[x]].r << RSHIFT) |
                  (palette[srow[x]].g << GSHIFT) |
                  (palette[srow[x]].b << BSHIFT) |
                  (src->format->alpha << ASHIFT);
    }
    
    srow += src->pitch;
    drow += dst->pitch >> 2;
  }
  
  SDL_UnlockSurface(src);
  SDL_UnlockSurface(dst);
}

/** Blurs a specific color channel */
#define sgBlurChannel(c, v, d, m, s) \
    sgSetPixelChannel(sgClamp(sgGetPixelChannel(c, m, s) + (((d - sgGetPixelChannel(c, m, s)) * v) / 255)), m, s)

/** Blurs a whole pixel */
#define sgBlurPixel(c, v) \
  cp = &(c); \
  if(cp >= start && cp < end) \
    { *cp = sgBlurChannel(*cp, v, tint.r, RMASK, RSHIFT) | \
            sgBlurChannel(*cp, v, tint.g, GMASK, GSHIFT) | \
            sgBlurChannel(*cp, v, tint.b, BMASK, BSHIFT) | \
            sgBlurChannel(*cp, v, tint.a, AMASK, ASHIFT); }

/* Blits from one surface to another by using a feathered brush and applying
 *  color tinting
 */
void sgBlur(SDL_Surface *src, SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *dr, sgColor tint)
{
  SDL_Rect srect;
  SDL_Rect drect;
  Uint32 *srow;
  Uint32 *start;
  Uint32 *end;
  Uint32 *drow;
  Uint16 x;
  Uint16 y;
  Uint32 *cp;
  
  srect = (sr ? *sr : src->clip_rect);
  drect = (dr ? *dr : dst->clip_rect);
  
  sgClip(src, &srect);
  sgClip(dst, &drect);
  sgIntersect(&srect, &drect);

//  SDL_FillRect(dst, NULL, 0);
  
  SDL_LockSurface(src);
  SDL_LockSurface(dst);

  
  srow = (Uint32 *)src->pixels + ((srect.y * src->pitch) >> 2) + srect.x;
  drow = (Uint32 *)dst->pixels + ((drect.y * dst->pitch) >> 2) + drect.x;
  start = dst->pixels;
  end = (Uint32 *)dst->pixels + ((dst->h * dst->pitch) >> 2);
  
  for(y = 0; y < srect.h && y < drect.h; y++)
  {
    for(x = 0; x < srect.w && x < drect.w; x++)
    {
      int sa = (srow[x] & AMASK) >> ASHIFT;

      sgBlurPixel(drow[x - (dst->pitch >> 2) - 1], sa >> 2);
      sgBlurPixel(drow[x - (dst->pitch >> 2) + 1], sa >> 2);
      sgBlurPixel(drow[x + (dst->pitch >> 2) - 1], sa >> 2);
      sgBlurPixel(drow[x + (dst->pitch >> 2) + 1], sa >> 2);

      sgBlurPixel(drow[x - 1], sa >> 1);
      sgBlurPixel(drow[x + 1], sa >> 1);
      sgBlurPixel(drow[x - (dst->pitch >> 2)], sa >> 1);
      sgBlurPixel(drow[x + (dst->pitch >> 2)], sa >> 1);

      sgBlurPixel(drow[x], sa);
    }
    
    srow += src->pitch >> 2;
    drow += dst->pitch >> 2;
  }
  
  SDL_UnlockSurface(src);
  SDL_UnlockSurface(dst);  
}

/* Additive alpha blit
 * -------------------------------------------------------------------------- */
void sgAlphaBlit(SDL_Surface *src, SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *dr)
{
  SDL_Rect srect;
  SDL_Rect drect;
  Uint32 *srow;
  Uint32 *drow;
  Uint16 x;
  Uint16 y;
  
  srect = (sr ? *sr : src->clip_rect);
  drect = (dr ? *dr : dst->clip_rect);
  
  sgClip(src, &srect);
  sgClip(dst, &drect);
  sgIntersect(&srect, &drect);
  
  SDL_LockSurface(src);
  SDL_LockSurface(dst);
  
  srow = (Uint32 *)src->pixels + ((srect.y * src->pitch) >> 2) + srect.x;
  drow = (Uint32 *)dst->pixels + ((drect.y * dst->pitch) >> 2) + drect.x;
  
  for(y = 0; y < srect.h; y++)
  {
    for(x = 0; x < srect.w; x++)
    {
      Uint32 salpha = ((srow[x] & AMASK) >> ASHIFT);
      Uint32 dalpha = ((drow[x] & AMASK) >> ASHIFT);
      int r;
      int g;
      int b;
      int a;
      
      if(!salpha && !dalpha)
        continue;
      
      r = ((srow[x] & RMASK) >> RSHIFT);
      g = ((srow[x] & GMASK) >> GSHIFT);
      b = ((srow[x] & BMASK) >> BSHIFT);

      if(dalpha)
      {
        r *= salpha;
        g *= salpha;
        b *= salpha;
        
        r /= 255;
        g /= 255;
        b /= 255;
        
        r += (((drow[x] & RMASK) >> RSHIFT) * (255 - salpha) / 255);
        g += (((drow[x] & GMASK) >> GSHIFT) * (255 - salpha) / 255);
        b += (((drow[x] & BMASK) >> BSHIFT) * (255 - salpha) / 255);
      }

      a = (int)salpha + (int)dalpha;
      
      r = r > 255 ? 255 : r;
      g = g > 255 ? 255 : g;
      b = b > 255 ? 255 : b;
      a = a > 255 ? 255 : a;
      
      drow[x] = ((a << ASHIFT)) |
                ((r << RSHIFT)) |
                ((g << GSHIFT)) |
                ((b << BSHIFT));
    }
    
    srow += src->pitch >> 2;
    drow = (Uint32 *)((Uint8 *)drow + dst->pitch);
  }
  
  SDL_UnlockSurface(src);
  SDL_UnlockSurface(dst);
}

/* Draws a gradient to an 8-bit Surface
 * -------------------------------------------------------------------------- */
void sgDrawRect(SDL_Surface *dest, SDL_Rect *rect, 
                Uint8 ia, Uint8 ib, sgDraw draw)
{
  Uint8 *row;
  int x, y;
  int xblend, yblend;
  SDL_Rect dr = (rect ? *rect : dest->clip_rect);
  
  sgClip(dest, &dr);
  
  SDL_LockSurface(dest);

  row = (Uint8 *)dest->pixels + dr.y * dest->pitch + dr.x;

  for(y = 0; y < dr.h; y++)
  {
    yblend = y * 127 / dr.h;

    for(x = 0; x < dr.w; x++)
    {
      xblend = yblend + x * 127 / dr.w;

      if((draw & SG_DRAW_INVERSE))
        xblend = 255 - xblend;

      if((draw & SG_DRAW_FILL) == 0)
      {
        if(x != 0 && x != dr.w - 1 &&
           y != 0 && y != dr.h - 1)
          continue;
      }
      
      row[x] = ((ia * xblend + ib * (255 - xblend)) >> 8) + 1;
    }
    
    row += dest->pitch;
  }
  
  SDL_UnlockSurface(dest);
}

/* -------------------------------------------------------------------------- */
void sgDrawGradient(SDL_Surface *dest, SDL_Rect *rect, sgDraw draw)
{
  Uint8 ia;
  Uint8 ib;
  
/*  if(draw & SG_DRAW_FILL)
  {
    ia = SG_COLOR_DARK;
    ib = SG_COLOR_BRIGHT;
  }
  else*/  if((draw & SG_DRAW_HIGH))
  {
    if(draw & SG_DRAW_SOFT)
    {
      ia = SG_COLOR_DARKTONE;
      ib = SG_COLOR_BRIGHTTONE;
    }
    else
    {
      ia = SG_COLOR_DARK;
      ib = SG_COLOR_BRIGHT;
    }
  }
  else 
  {
    if(draw & SG_DRAW_SOFT)
    {
      ia = SG_COLOR_DARKTONE;
      ib = SG_COLOR_BRIGHT;
    }
    else
    {
      ia = SG_COLOR_DARKTONE;
      ib = SG_COLOR_BRIGHTER;
    }
  }

  sgDrawRect(dest, rect, ia, ib, draw);
}

/* -------------------------------------------------------------------------- */
void sgDrawSingleFrame(SDL_Surface *dest, SDL_Rect *rect, sgDraw draw)
{
  SDL_Rect dr = (rect ? *rect : dest->clip_rect);
  Sint16 x1, x2;
  Sint16 y1, y2;
  Uint8 i1, i2;
  
  x1 = dr.x;
  x2 = dr.x + dr.w - 1;
  y1 = dr.y;
  y2 = dr.y + dr.h - 1;

  if(draw & SG_DRAW_INVERSE)
  {
    x1 ^= x2; x2 ^= x1; x1 ^= x2;
    y1 ^= y2; y2 ^= y1; y1 ^= y2;
  }
  
//  draw = draw & ~SG_DRAW_INNER;
  
  i1 = (draw & SG_DRAW_HIGH) ? SG_COLOR_BRIGHTER :  SG_COLOR_WHITE;
  i2 = (draw & SG_DRAW_HIGH) ? SG_COLOR_BRIGHTTONE : SG_COLOR_DARKTONE;

  sgDrawHLine(dest, x1, x2, y1, i1, i2);
  sgDrawVLine(dest, x1, y1, y2, i1, i2);
  
  i1 = (draw & SG_DRAW_HIGH) ? SG_COLOR_DARKER : SG_COLOR_DARK ;
  i2 = (draw & SG_DRAW_HIGH) ? SG_COLOR_DARKTONE : SG_COLOR_BRIGHTTONE;
  
  sgDrawHLine(dest, x1, x2, y2, i2, i1);
  sgDrawVLine(dest, x2, y1, y2, i2, i1);
  
  if(draw & SG_DRAW_FILL)
  {
    dr.x += 1; dr.w -= 2;
    dr.y += 1; dr.h -= 2;
    
    if(draw & SG_DRAW_INVERSE)
    {
      i1 ^= i2; i2 ^= i1; i1 ^= i2;
    }
    
    sgDrawRect(dest, &dr, i1, i2, draw);
  }
}

/* Draws a paletted frame to an 8-bit Surface
 * -------------------------------------------------------------------------- */
void sgDrawFrame(SDL_Surface *dest, SDL_Rect *rect, sgDraw draw)
{
  SDL_Rect dr = (rect ? *rect : dest->clip_rect);
  Uint8 *row;
  int x, y;
  int xblend, yblend;
  Uint8 ib, ia;
  
  sgClip(dest, &dr);
  
/*  sgLog("Drawing frame: %i %i %u %u", 
        (int)dr.x, (int)dr.y, 
        (unsigned int)dr.w, (unsigned int)dr.h);*/
  
  SDL_LockSurface(dest);
  
  row = (Uint8 *)dest->pixels + dr.y * dest->pitch + dr.x;
    
  for(y = 0; y < dr.h; y++)
  {
    yblend = y * 127 / dr.h;
    
    for(x = 0; x < dr.w; x++)
    {
      xblend = yblend + x * 127 / dr.w;

      if((draw & SG_DRAW_INVERSE))
        xblend = 255 - xblend;

      if(x == 0 || y == 0 || x == (dr.w - 1) || y == (dr.h - 1))
      {
        if((draw & SG_DRAW_HIGH))
        {
          ib = SG_COLOR_DARKTONE;
          ia = SG_COLOR_BLACK;
        }
        else
        {
          ib = SG_COLOR_DARKTONE;
          ia = SG_COLOR_DARK;
        }
      }
      else if(x == 1 || y == 1 || x == (dr.w - 2) || y == (dr.h - 2))
      {
        if((draw & SG_DRAW_HIGH))
        {
          ib = SG_COLOR_BRIGHTER;
          ia = SG_COLOR_DARKTONE;
        }
        else
        {
          ib = SG_COLOR_BRIGHT;
          ia = SG_COLOR_NORMAL;
        }
        
      }
      else if((draw & SG_DRAW_INVFILL))
      {
          ib = SG_COLOR_DARKTONE;
          ia = SG_COLOR_BRIGHTTONE;
      }
      else if((draw & SG_DRAW_FILL))
      {
          ib = SG_COLOR_BRIGHTTONE;
          ia = SG_COLOR_DARKTONE;
      }
      else if((draw & SG_DRAW_CLEAR))
      {
        row[x] = 0;
        continue;
      }
      else
        continue;
      
      row[x] = ((ia * xblend + ib * (255 - xblend)) >> 8) + 1;
    }
    
    row += dest->pitch;
  }
  
  SDL_UnlockSurface(dest);
}

/* Draws a horizontal line to an 8-bit Surface
 * -------------------------------------------------------------------------- */
void sgDrawHLine(SDL_Surface *dest, Sint16 x1, Sint16 x2, Sint16 y, 
                 Uint8 i1, Uint8 i2)
{
  Uint8 *row;
  int blend;
  int range;
  int pos;

  SDL_LockSurface(dest);

  if(x1 > x2)
  {
    x1 ^= x2; x2 ^= x1; x1 ^= x2;
    i1 ^= i2; i2 ^= i1; i1 ^= i2;
  }
  
  range = x2 - x1;
  
  row = (Uint8 *)dest->pixels + y * dest->pitch + x1;

  for(pos = 0; pos < range; pos++)
  {
    blend = pos * 255 / range;
    
    *row = (((i1 * (255 - blend)) + (i2 * blend)) >> 8) + 1;
    row++;
  }
  
  SDL_UnlockSurface(dest);
}

/* Draws a horizontal line to an 8-bit Surface
 * Start and end colors are read from the start and end points
 * -------------------------------------------------------------------------- */
void sgBlendHLine(SDL_Surface *dest, Sint16 x1, Sint16 x2, Sint16 y)
{
  Uint8 i1, i2;
  
  i1 = sgGetPixel8(dest, x1, y);
  i2 = sgGetPixel8(dest, x2, y);
  
  sgDrawHLine(dest, x1, x2, y, i1, i2);
}
/* Draws a vertical line to an 8-bit Surface
 * Start and end colors are read from the start and end points
 * -------------------------------------------------------------------------- */
void sgBlendVLine(SDL_Surface *dest, Sint16 x, Sint16 y1, Sint16 y2)
{
  Uint8 i1, i2;
  
  i1 = sgGetPixel8(dest, x, y1);
  i2 = sgGetPixel8(dest, x, y2);
  
  sgDrawVLine(dest, x, y1, y2, i1, i2);
}

/* Draws a vertical line to an 8-bit Surface
 * -------------------------------------------------------------------------- */
void sgDrawVLine(SDL_Surface *dest, Sint16 x, Sint16 y1, Sint16 y2,
                 Uint8 i1, Uint8 i2)
{
  Uint8 *row;
  int blend;
  int range;
  int pos;

  SDL_LockSurface(dest);

  if(y1 > y2)
  {
    y1 ^= y2; y2 ^= y1; y1 ^= y2;
    i1 ^= i2; i2 ^= i1; i1 ^= i2;
  }
  
  range = y2 - y1;
  
  row = (Uint8 *)dest->pixels + y1 * dest->pitch + x;

  for(pos = 0; pos < range; pos++)
  {
    blend = pos * 255 / range;
    
    *row = (((i1 * (255 - blend)) + (i2 * blend)) >> 8) + 1;
    row += dest->pitch;
  }
  
  SDL_UnlockSurface(dest);
}

/* Converts a color value from HSV to RGB colorspace
 * -------------------------------------------------------------------------- */
sgColor sgHSVToRGB(sgHSV hsv)
{
  double h, s, v;
  double f, p, q, t;
  sgColor rgb;
  
  if(hsv.s == 0) {
    
    rgb.r = hsv.v;
    rgb.g = hsv.v;
    rgb.b = hsv.v;
    
  } else {
    
    h = (double)hsv.h * 6.0  / 255.0;
    s = (double)hsv.s / 255.0;
    v = (double)hsv.v / 255.0;
                     
    f = h - (Uint8)h;
    p = v * (1.0 - s);
    q = v * (1.0 - (s * f));
    t = v * (1.0 - (s * (1.0 - f)));
    
    switch((Uint8)h) {
      case 0:
        rgb.r = v * 255;
        rgb.g = t * 255;
        rgb.b = p * 255;
        break;

      case 1:
        rgb.r = q * 255;
        rgb.g = v * 255;
        rgb.b = p * 255;
        break;
      
      case 2:
        rgb.r = p * 255;
        rgb.g = v * 255;
        rgb.b = t * 255;
        break;

      case 3:
        rgb.r = p * 255;
        rgb.g = q * 255;
        rgb.b = v * 255;
        break;
      
      case 4:
        rgb.r = t * 255;
        rgb.g = p * 255;
        rgb.b = v * 255;
        break;

      case 5:
        rgb.r = v * 255;
        rgb.g = p * 255;
        rgb.b = q * 255;
        break;
    }
  }
  
  return rgb;
}

/* Converts a color value from RGB to HSV colorspace
 * -------------------------------------------------------------------------- */
sgHSV sgRGBToHSV(sgColor rgb)
{
  double r, g, b;
  double h, s, v;
  double max, min, delta;
  sgHSV hsv;
  
  r = ((double)rgb.r) / 255.0;
  g = ((double)rgb.g) / 255.0;
  b = ((double)rgb.b) / 255.0;
  
  max = (r > g ? (r > b ? r : b) : g);
  min = (r < g ? (r < b ? r : b) : g);

  v = max;
  delta = max - min;
  
  if(delta > 0.0001)
  {
    s = delta / max;
    
    if(r == max)
      h = (g - b) / delta;
    else if(g == max)
      h = 2.0 + (b - r) / delta;
    else// if(b == max)
      h = 4.0 + (r - g);
    
    h /= 6.0;
    
    if(h < 0.0)
      h += 1.0;
    if(h > 1.0)
      h -= 1.0;
  }
  else
  {
    s = 0.0;
    h = 0.0;
  }
  
  hsv.h = (int)(h * 255.0);
  hsv.s = (int)(s * 255.0);
  hsv.v = (int)(v * 255.0);
  
  return hsv;
}

/* Create a palette consisting of 256 colors matching the specified hue
 * -------------------------------------------------------------------------- */
sgColor *sgCreatePalette(sgColor base) 
{  
  sgColor *colors = malloc(256 * sizeof(sgColor));
  int i;
  
  for(i = 0; i < 128; i++)
  {
    colors[i].r = (base.r * i) >> 7;
    colors[i].g = (base.g * i) >> 7;
    colors[i].b = (base.b * i) >> 7;
    colors[i + 128].r = base.r + (((255 - base.r) * i) >> 7);
    colors[i + 128].g = base.g + (((255 - base.g) * i) >> 7);
    colors[i + 128].b = base.b + (((255 - base.b) * i) >> 7);
  }
  
  return colors;
}

/* Create a palette consisting of 256 colors matching the specified hue
 * but equalize the channels to get a greyscale palette
 * -------------------------------------------------------------------------- */
sgColor *sgCreateBWPalette(sgColor base)
{
  sgColor *colors = malloc(256 * sizeof(sgColor));
  int i;
  int c;
  
  c = base.r + base.g + base.g;
  c /= 3;
  
#if 1
  c = 128;
#endif
  
  for(i = 0; i < 128; i++)
  {
    colors[i].r = (c * i) >> 7;
    colors[i].g = (c * i) >> 7;
    colors[i].b = (c * i) >> 7;
    colors[i + 128].r = c + (((255 - c) * i) >> 7);
    colors[i + 128].g = c + (((255 - c) * i) >> 7);
    colors[i + 128].b = c + (((255 - c) * i) >> 7);
  }
  
  return colors;
}


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
/*void sgFillGradient(SDL_Surface *dest, SDL_Rect *cutout, Uint8 range, int sunken)
{
  Uint8 *row;
  int x, y;
  int xblend, yblend;
  
  SDL_FillRect(dest, NULL, 0);
  
  SDL_LockSurface(dest);
  
  row = (Uint8 *)dest->pixels;
  
  for(y = 0; y < dest->h; y++)
  {
    yblend = y * 127 / dest->h;

    for(x = 0; x < dest->w; x++)
    {
      if(cutout)
      {
        if(y >= cutout->y && y < cutout->y + cutout->h &&
           x >= cutout->x && x < cutout->x + cutout->w)
          continue;
      }
      
      xblend = yblend + x * 127 / dest->w + 1;

      if(sunken)
        xblend = 256 - xblend;
      
      row[x] = (xblend * range / 255) + ((255 - range) >> 1);
    }
    
    row += dest->pitch;
  }
  
  SDL_UnlockSurface(dest);
}*/

/* Returns 1 when x and y are inside rect                                     *
 * -------------------------------------------------------------------------- */
int sgMatchRect(SDL_Rect *rect, Sint16 x, Sint16 y) 
{
  int x1, x2;
  int y1, y2;
  
  x1 = rect->x;
  x2 = rect->x + rect->w - 1;
  y1 = rect->y;
  y2 = rect->y + rect->h - 1;
  
  if(x < x1) return 0;
  if(x > x2) return 0;
  if(y < y1) return 0;
  if(y > y2) return 0;
  
  return 1;
}

/* Merges two rectangles (A and B)
 * -------------------------------------------------------------------------- */
void sgMergeRect(SDL_Rect *rect, SDL_Rect a, SDL_Rect b)
{
  Sint16 x1, x2;
  Sint16 y1, y2;
  
  x1 = a.x; x2 = a.x + a.w;
  y1 = a.y; y2 = a.y + a.h;
  
  if(b.x + b.w > x2) x2 = b.x + b.w;
  if(b.y + b.h > y2) y2 = b.y + b.h;
  
  if(b.x < x1) x1 = b.x;
  if(b.y < y1) y1 = b.y;
  
  rect->x = x1; rect->w = x2 - x1;
  rect->y = y1; rect->h = y2 - y1;
}

/* Returns 1 when rectangle A and B intersect
 * -------------------------------------------------------------------------- */
void sgIntersect(SDL_Rect *a, SDL_Rect *b)
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

/* Clip rectangle so it will fit into the surface
 * -------------------------------------------------------------------------- */
void sgClip(SDL_Surface *surface, SDL_Rect *rect)
{
  if(rect->x < 0)
  {
    rect->w -= -rect->x;
    rect->x = 0;
  }
  
  if(rect->x >= surface->w)
  {
    rect->w = 0;
    rect->x = surface->w;
  }
  
  if(rect->y < 0)
  {
    rect->h -= -rect->y;
    rect->y = 0;
  }
  
  if(rect->y >= surface->h)
  {
    rect->h = 0;
    rect->y = surface->h;
  }
  
  if(rect->x + rect->w >= surface->w)
    rect->w = surface->w - rect->x;
  
  if(rect->y + rect->h >= surface->h)
    rect->h = surface->h - rect->y;
}

/** @} */
