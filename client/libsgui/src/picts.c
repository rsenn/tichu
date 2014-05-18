/* $Id: picts.c,v 1.9 2005/05/16 05:19:55 smoli Exp $
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

#include <SDL.h>

#include <libsgui/common.h>
#include <libsgui/picts.h>

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
#define I 0x1   /* invert */
#define D 0x80  /* darken */
#define L 0x7f  /* lighten */

sgPict sgArrowUp =
{
  16, 16,
  {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,D,D,0,0,0,0,0,0,0,
    0,0,0,0,0,0,D,I,I,D,0,0,0,0,0,0,
    0,0,0,0,0,D,I,I,I,I,D,0,0,0,0,0,    
    0,0,0,0,D,I,I,I,I,I,I,D,0,0,0,0,
    0,0,0,D,I,I,I,I,I,I,I,I,D,0,0,0,
    0,0,D,I,I,I,I,I,I,I,I,I,I,D,0,0,
    0,0,D,I,I,I,I,I,I,I,I,I,I,D,0,0,
    0,0,0,L,L,I,I,I,I,I,I,L,L,0,0,0,
    0,0,0,0,0,D,I,I,I,I,I,L,0,0,0,0,
    0,0,0,0,0,D,I,I,I,I,I,L,0,0,0,0,
    0,0,0,0,0,D,I,I,I,I,I,L,0,0,0,0,
    0,0,0,0,0,0,L,L,L,L,L,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  }
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
sgPict sgArrowDown =
{
  16, 16,
  {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,D,D,D,D,D,0,0,0,0,0,
    0,0,0,0,0,D,I,I,I,I,I,L,0,0,0,0,
    0,0,0,0,0,D,I,I,I,I,I,L,0,0,0,0,
    0,0,0,0,0,D,I,I,I,I,I,L,0,0,0,0,
    0,0,0,D,D,I,I,I,I,I,I,D,D,0,0,0,
    0,0,D,I,I,I,I,I,I,I,I,I,I,L,0,0,
    0,0,L,I,I,I,I,I,I,I,I,I,I,L,0,0,
    0,0,0,L,I,I,I,I,I,I,I,I,L,0,0,0,
    0,0,0,0,L,I,I,I,I,I,I,L,0,0,0,0,
    0,0,0,0,0,L,I,I,I,I,L,0,0,0,0,0,    
    0,0,0,0,0,0,L,I,I,L,0,0,0,0,0,0,
    0,0,0,0,0,0,0,L,L,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  }
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
sgPict sgArrowUndrop =
{
  16, 16,
  {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,D,D,0,0,0,0,0,0,0,
    0,0,0,0,0,0,D,I,I,D,0,0,0,0,0,0,
    0,0,0,0,0,D,I,I,I,I,D,0,0,0,0,0,    
    0,0,0,0,D,I,I,I,I,I,I,D,0,0,0,0,
    0,0,0,D,I,I,I,I,I,I,I,I,D,0,0,0,
    0,0,D,I,I,I,I,I,I,I,I,I,I,D,0,0,
    0,0,D,I,I,I,I,I,I,I,I,I,I,D,0,0,
    0,0,0,D,L,L,L,L,L,L,L,L,L,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  }
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
sgPict sgArrowDrop =
{
  16, 16,
  {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,D,D,D,D,D,D,D,D,D,D,0,0,0,
    0,0,D,I,I,I,I,I,I,I,I,I,I,L,0,0,
    0,0,L,I,I,I,I,I,I,I,I,I,I,L,0,0,
    0,0,0,L,I,I,I,I,I,I,I,I,L,0,0,0,
    0,0,0,0,L,I,I,I,I,I,I,L,0,0,0,0,
    0,0,0,0,0,L,I,I,I,I,L,0,0,0,0,0,    
    0,0,0,0,0,0,L,I,I,L,0,0,0,0,0,0,
    0,0,0,0,0,0,0,L,L,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  }
};

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
sgPict sgCircle =
{
  16, 16,
  {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,I,I,I,I,0,0,0,0,0,0,
    0,0,0,0,I,I,I,I,I,I,I,I,0,0,0,0,
    0,0,0,I,I,I,I,I,I,I,I,I,I,0,0,0,
    0,0,0,I,I,I,I,I,I,I,I,I,I,0,0,0,
    0,0,I,I,I,I,I,I,I,I,I,I,I,I,0,0,
    0,0,I,I,I,I,I,I,I,I,I,I,I,I,0,0,
    0,0,I,I,I,I,I,I,I,I,I,I,I,I,0,0,
    0,0,I,I,I,I,I,I,I,I,I,I,I,I,0,0,
    0,0,0,I,I,I,I,I,I,I,I,I,I,0,0,0,
    0,0,0,I,I,I,I,I,I,I,I,I,I,0,0,0,
    0,0,0,0,I,I,I,I,I,I,I,I,0,0,0,0,
    0,0,0,0,0,0,I,I,I,I,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  }
};

/** Applies pictogram data to a surface (some kind of bump-mapping) */
void sgPutPict(SDL_Surface *surface, SDL_Rect *drect, sgPict *pict, sgDraw draw)
{
  Uint8 *row;
  SDL_Rect dr;
  int x;
  int y;

  dr = (drect ? *drect : surface->clip_rect);
  dr.x += (dr.w - pict->w) / 2;
  dr.w -= (dr.w - pict->w);
  dr.y += (dr.h - pict->h) / 2;
  dr.h -= (dr.h - pict->h);

  SDL_LockSurface(surface);
  
  row = (Uint8 *)surface->pixels + dr.y * surface->pitch + dr.x;
  
  for(y = 0; y < dr.h; y++)
  {
    for(x = 0; x < dr.w; x++)
    {
      Uint8 p = pict->data[y * pict->w + x];
      
      if(draw & SG_DRAW_INVERSE)
        p = -p;
      
      if(p == L)
      {
        if(draw & SG_DRAW_HIGH)
          row[x] += (255 - row[x]) / 2;
        else
          row[x] += (255 - row[x]) * 3 / 7;
      }
      else if(p == D)
      {
        if(draw & SG_DRAW_HIGH)
          row[x] -= row[x] / 2;
        else
          row[x] -= row[x] * 3 / 7;
      }
      else if(p)
        row[x] ^= 0xff;
    }
    
    row += surface->pitch;
  }
  
  SDL_UnlockSurface(surface);
}

/** Copies pictogram data to a surface */
void sgCopyPict(SDL_Surface *surface, SDL_Rect *drect, sgPict *pict, sgDraw draw)
{
  Uint8 *row;
  SDL_Rect dr;
  int x;
  int y;

  dr = (drect ? *drect : surface->clip_rect);
  dr.x += (dr.w - pict->w) / 2;
  dr.w -= (dr.w - pict->w);
  dr.y += (dr.h - pict->h) / 2;
  dr.h -= (dr.h - pict->h);

  SDL_LockSurface(surface);
  
  row = (Uint8 *)surface->pixels + dr.y * surface->pitch + dr.x;
  
  for(y = 0; y < dr.h; y++)
  {
    for(x = 0; x < dr.w; x++)
    {
      Uint8 p = pict->data[y * pict->w + x];
      
      if(p)
        row[x] = 0x80;
      else
        row[x] = 0;
    }
    
    row += surface->pitch;
  }
  
  SDL_UnlockSurface(surface);
  
  sgPutPict(surface, drect, pict, draw);
}

