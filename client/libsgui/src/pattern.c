/* $Id: pattern.c,v 1.10 2005/05/21 10:09:34 smoli Exp $
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

/** @weakgroup sgPattern
 *  @{
 */

#include <SDL.h>
#include <math.h>
#include <string.h>

#include <libsgui/pattern.h>


Uint8 sgSin[256]; /**< Sine table with 8-bit resolution */
Uint8 sgCos[256]; /**< Cosine table with 8-bit resolution */

/* -------------------------------------------------------------------------- */
static Uint8 sgSinePattern(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
  return sgSin[(Uint8)((x + y) ^ (y - x))] +
         sgCos[(Uint8)((x - y) ^ (y + x))];         
}

/* -------------------------------------------------------------------------- */
static Uint8 sgSickPattern(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
  return (x ^ (y << (x & 0x04))) + (y ^ (x << (y & 0x04)));
}

/* -------------------------------------------------------------------------- */
static Uint8 sgNicePattern(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
  return (x - y) ^ (y + x);
}

/* -------------------------------------------------------------------------- */
static Uint8 sgWeirdPattern(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
  return (x - (x ^ y)) + (y - (x ^ y));
}

/* -------------------------------------------------------------------------- */
static Uint8 sgFunkyTrianglePattern(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
  return ((x ^ 145) + (y ^ 543)) ^ ((x ^ 342) + (y ^ 761));
}

/* -------------------------------------------------------------------------- */
static Uint8 sgNestedRectanglePattern(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
  return ((x ^ 0xe4a8) + (x ^ 0x1e8b)) ^ ((y ^ 0x193f) - (y ^ 0x94e2));
}

/* -------------------------------------------------------------------------- */
static Uint8 sgNestedCubePattern(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
  return ((x - 0x834e) ^ (y + 0x2314)) - ((y - 0x23ab) ^ (x + 0x11e9));
}

/* -------------------------------------------------------------------------- */
static Uint8 sgScotlandPattern(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
  return ((x | 0x834e) + (x & 0x2314)) ^ ((y & 0x23ab) - (y | 0x11e9));
}

/* Fills an 8-bit surface with a pattern described by fn(x, y) 
 * -------------------------------------------------------------------------- */
static Uint8 sgDefaultPattern(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
  return ((x + y) & 3) ? 255 : 0;
}

static sgPattern sgPatternTable[] =
{
  { "Turn",               sgDefaultPattern },
  { "Fractal",            sgWeirdPattern },
  { "Exclusive OR",       sgNicePattern },
  { "Sine XOR",           sgSinePattern },
  { "Funky Triangles",    sgFunkyTrianglePattern },
  { "Nested Rectangles",  sgNestedRectanglePattern },
  { "Nested Cubes",       sgNestedCubePattern },
  { "Scotland Web",       sgScotlandPattern },
  { "Chaotic Web",        sgSickPattern },
  { "",              NULL },
};

/* -------------------------------------------------------------------------- */
sgPattern *sgGetPatternTable(void)
{
  return sgPatternTable;
}

/* -------------------------------------------------------------------------- */
sgPattern *sgFindPattern(const char *name)
{
  int i;
  
  for(i = 0; sgPatternTable[i].fn; i++)
  {
    if(!strcmp(name, sgPatternTable[i].name))
      return &sgPatternTable[i];
  }
  
  return NULL;
}
  
/* Initialize sine/cosine tables */
void sgInitPatterns(void)
{
  int i;
  
  for(i = 0; i < 256; i++)
  {
    sgSin[i] = (sin(i * M_PI * 2 / 256) + 1.0) * 255 / 2;
    sgCos[i] = (cos(i * M_PI * 2 / 256) + 1.0) * 255 / 2;
  }
}  
  
/* Fill a surface using a pattern function */
void sgFillPattern(SDL_Surface *surface, sgPattern *pattern, Uint8 contrast)
{
  Uint8 *pixel;
  Sint16 x;
  Sint16 y;
  
  SDL_LockSurface(surface);
  
  pixel = surface->pixels;
      
  for(y = 0; y < surface->h; y++)
  {
    for(x = 0; x < surface->w; x++)
    {
      int value;
      
      value = pattern->fn(x, y, surface->w, surface->h) - 128;
      
      pixel[x] = ((value * contrast / 255) + 128);
    }
    
    pixel += surface->pitch;
  }     
  
  SDL_UnlockSurface(surface);
}

/** @} */

