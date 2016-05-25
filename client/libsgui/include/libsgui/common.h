/* $Id: common.h,v 1.23 2005/05/19 23:08:32 smoli Exp $
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

#ifndef SGUI_COMMON_H
#define SGUI_COMMON_H

/** @defgroup sgCommon  sgCommon: Commonly used data, functions and macros 
 *
 *  @{
 */

#include <SDL.h>
#include <string.h>

#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C" {  
#endif /* __cplusplus */

/** Constants for commonly used keys */
enum sgKey
{
  SG_KEY_UP = 0,
  SG_KEY_DOWN,
  SG_KEY_LEFT,
  SG_KEY_RIGHT,
  SG_KEY_ENTER
};
  
/** Type for commonly used keys */
typedef enum sgKey sgKey;

/*#define SG_NOFOCUS 0
#define SG_FOCUS   1*/


/** Flags for low-level drawing functions */
enum sgDraw
{   
  SG_DRAW_NORMAL   = 0x00, /**< normal gradients (will draw raised frames and fillings) */
  SG_DRAW_INVERSE  = 0x01, /**< inverse gradients (will draw sunken frames and fillings) */
  SG_DRAW_HIGH     = 0x02, /**< increased contrast (for outer frames) */
//  SG_DRAW_LOW      = 0x04, /**< less contrast (for inner frames) */
  SG_DRAW_INNER    = 0x08,
  SG_DRAW_SOFT     = 0x10,
  SG_DRAW_CLEAR    = 0x20, /**< clear instead of fill (fill with zero) */
  SG_DRAW_FILL     = 0x40, /**< draw filled frames */
  SG_DRAW_INVFILL  = 0x80, /**< inverse fill */
};

/* 32-bit software surfaces are BGRA by default on all architectures 
 * i have encountered so far, so we'll use those masks for pixel magic
 */
#ifndef AMASK
#define AMASK  0xff000000     /**< alpha channel mask */
#define ASHIFT 24             /**< alpha channel shifting */
#endif /* AMASK */
#ifndef RMASK
#define RMASK  0x00ff0000     /**< red channel mask */
#define RSHIFT 16             /**< red channel shifting */
#endif /* RMASK */
#ifndef GMASK
#define GMASK  0x0000ff00     /**< green channel mask */
#define GSHIFT 8              /**< green channel shifting */
#endif /* GMASK */
#ifndef BMASK
#define BMASK  0x000000ff     /**< blue channel mask */
#define BSHIFT 0              /**< blue channel shifting */
#endif /* BMASK */

/** values of the «edge» argument of several padding & splitting functions */
enum sgEdge 
{
  SG_EDGE_BOTTOM = 0x01, /**< splits/pads rectangle at bottom */
  SG_EDGE_TOP    = 0x02, /**< splits/pads rectangle at top */
  SG_EDGE_RIGHT  = 0x04, /**< splits/pads rectangle at right */
  SG_EDGE_LEFT   = 0x08, /**< splits/pads rectangle at left */
    
  SG_EDGE_ALL    = 0x0f  /**< all edges */
};

/** Type for edge enumeration */
typedef enum sgEdge sgEdge;
  
/** values of the «align» argument of several rectangle and font functions */
enum sgAlign
{
  SG_ALIGN_CENTER     = 0x00, /**< centered */
  SG_ALIGN_LEFT       = 0x01, /**< left aligned */
  SG_ALIGN_RIGHT      = 0x02, /**< right aligned */
  SG_ALIGN_HORIZONTAL = 0x03, /**< mask for horizontal alignment */

  SG_ALIGN_MIDDLE     = 0x00, /**< vertically centered */
  SG_ALIGN_TOP        = 0x10, /**< top aligned */
  SG_ALIGN_BOTTOM     = 0x20, /**< bottom aligned */
  SG_ALIGN_VERTICAL   = 0x30, /**< mask for vertical alignment */
};

/** Type for alignment enumeration */
typedef enum sgAlign sgAlign;
  
/** Commonly used luminance values (palette indexes) */
enum sgLuminance
{
  SG_COLOR_BLACK      = 0,   /**< darkest color */
  SG_COLOR_DARKER     = 56,  /**< darker color */
  SG_COLOR_DARK       = 85,  /**< dark color */
  SG_COLOR_DARKTONE   = 113, /**< somewhat darker */
  SG_COLOR_NORMAL     = 141, /**< the color tone with full saturation */
  SG_COLOR_BRIGHTTONE = 170, /**< somewhat brighter */
  SG_COLOR_BRIGHT     = 198, /**< bright color */
  SG_COLOR_BRIGHTER   = 226, /**< brighter color */
  SG_COLOR_WHITE      = 255  /**< brightest color */
};
  
/** Type for luminance */
typedef enum sgLuminance sgLuminance;
  
/* Luminance constants as used by sgDrawFrame() */
#define SG_COLOR_OUTER_DARK   SG_COLOR_DARKER     /**< shadowed part of outer frames */
#define SG_COLOR_INNER_DARK   SG_COLOR_DARK       /**< shadowed part of inner frames */
#define SG_COLOR_FILL_DARK    SG_COLOR_DARKTONE   /**< shadowed part of frame filling */
#define SG_COLOR_FILL         SG_COLOR_NORMAL     /**< frame fill color */
#define SG_COLOR_FILL_BRIGHT  SG_COLOR_BRIGHTTONE /**< lightened part of frame filling */
#define SG_COLOR_INNER_BRIGHT SG_COLOR_BRIGHT     /**< lightened part of inner frames */
#define SG_COLOR_OUTER_BRIGHT SG_COLOR_BRIGHTER   /**< lightened part of outer frames */
/*
#ifndef MOUSE_WHEELUP
#define MOUSE_WHEELUP   (1<<4)
#endif

#ifndef MOUSE_WHEELDOWN
#define MOUSE_WHEELDOWN (1<<5)
#endif
*/
/** RGBA 32-bit color structure */
struct sgColor
{
  Uint8 r;    /**< red channel */
  Uint8 g;    /**< green channel */
  Uint8 b;    /**< blue channel */
  Uint8 a;    /**< alpha channel */
};

/** RGBA 32-bit color type */
typedef struct sgColor sgColor;

/** Hue/Saturation/Value structure */
struct sgHSV
{
  Uint8 h;    /**< hue channel */
  Uint8 s;    /**< saturation channel */
  Uint8 v;    /**< value channel */
  Uint8 a;    /**< alpha channel */
};

/** Hue/Saturation/Value type */
typedef struct sgHSV sgHSV;
  

/** macro to access a pixel on a 32-bit Surface */
#define sgGetPixel32(sf, x, y)     (((Uint32 *)(sf)->pixels) \
                                         [(y) * ((sf)->pitch >> 2) + (x)])

/** macro to access a pixel on a 8-bit Surface */
#define sgGetPixel8(sf, x, y)      (((Uint8 *)(sf)->pixels) \
                                         [(y) * (sf)->pitch + (x)])

/** macro to write a pixel to a 32-bit Surface */
#define sgPutPixel32(sf, x, y, c)  (((Uint32 *)(sf)->pixels) \
                                         [(y) * ((sf)->pitch >> 2) + (x)] = c)

/** macro to write a pixel to a 8-bit Surface */
#define sgPutPixel8(sf, x, y, c)   (((Uint8 *)(sf)->pixels) \
                                         [(y) * (sf)->pitch + (x)] = c)

/** gets value of a specific color channel from pixel data */
#define sgGetPixelChannel(x, m, s) (Uint8)(((x) & (m)) >> (s))
  
/** sets value of a specific color channel */
#define sgSetPixelChannel(x, m, s) (Uint32)(((x) << (s)) & (m))
  
/** A wrapper around strncpy() so the string will always be null-terminated
 *  and the destination buffers size is "auto-detected" respectively preprocessor-
 *  evaluated.
 */
#define sgStringCopy(d, s) do \
{ \
  if(s) { \
    strncpy((d), (s), sizeof(d)); \
    (d)[sizeof(d) - 1] = '\0'; \
  } else { \
    (d)[0] = '\0'; \
  } \
} while(0)  

/** A wrapper around strncat() so the string will always be null-terminated
 *  and the destination buffers size is "auto-detected" respectively preprocessor-
 *  evaluated.
 */
#define sgStringCat(d, s) do \
{ \
  if(s) { \
    int p = strlen(d); \
    int n = sizeof(d) - p - 1; \
    strncat((d), (s), n); \
    (d)[sizeof(d) - 1] = '\0'; \
  } \
} while(0)  

/** clamps a value to 0..255 */
#define sgClamp(v)                 (Uint8)((v) > 255 ? 255 : (v))
  
/** Limits «value» to 0 - «range» */
#define sgLimit(value, range) ((value) < 0 ? 0 : \
                              ((value) < (range) ? \
                               (value) : (range)))
  
/* Yeah I know the following rectangle macro stuff is 
 * really ugly, suggestions and patches are welcome! :) */


/** Subtracts a border of thickness «border» from the rectangle
 *  (Makes rectangle smaller)
 */
#define sgSubBorder(rect, border) do { \
  (rect)->x += (border); \
  (rect)->y += (border); \
  (rect)->w -= (border) * 2; \
  (rect)->h -= (border) * 2; \
} while(0);

/** Adds a border of thickness «border» to the rectangle
 *  (Makes rectangle bigger)
 */
#define sgAddBorder(rect, border) do { \
  (rect)->x -= (border); \
  (rect)->y -= (border); \
  (rect)->w += (border) * 2; \
  (rect)->h += (border) * 2; \
} while(0);

/** Splits a rectangle into 2 rectangles, splits «rect» vertically and creates
 *  «newrect» on the left side which is having a width of «pixels»             
 */
#define sgSplitLeft(rect, newrect, pixels) do { \
  if((SDL_Rect *)newrect) \
  { \
    *((SDL_Rect *)newrect) = *(rect); \
     ((SDL_Rect *)newrect)->w = pixels; \
  } \
  (rect)->w -= pixels; \
  (rect)->x += pixels; \
} while(0);

/** Splits a rectangle into 2 rectangles, splits «rect» vertically and creates
 *  «newrect» on the right side which is having a width of «pixels»
 */
#define sgSplitRight(rect, newrect, pixels) do { \
  if((SDL_Rect *)newrect) \
  { \
    *((SDL_Rect *)newrect) = *(rect); \
     ((SDL_Rect *)newrect)->w = (pixels); \
     ((SDL_Rect *)newrect)->x += (rect)->w - (pixels); \
  } \
  (rect)->w -= pixels; \
} while(0);

/** Splits a rectangle into 2 rectangles, splits «rect» horizontally and
 *  creates «newrect» on the top which is having a height of «pixels»
 */
#define sgSplitTop(rect, newrect, pixels) do { \
  if((SDL_Rect *)newrect) \
  { \
    *((SDL_Rect *)newrect) = *(rect); \
     ((SDL_Rect *)newrect)->h = (pixels); \
  } \
  (rect)->h -= (pixels); \
  (rect)->y += (pixels); \
} while(0);

/** Splits a rectangle into 2 rectangles, splits «rect» horizontally and
 *  creates «newrect» on the bottom side which is having a height of «pixels»
 */
#define sgSplitBottom(rect, newrect, pixels) do { \
  if((SDL_Rect *)newrect) \
  { \
    *((SDL_Rect *)newrect) = *(rect); \
     ((SDL_Rect *)newrect)->h = (pixels); \
     ((SDL_Rect *)newrect)->y += (rect)->h - (pixels); \
  } \
  (rect)->h -= (pixels); \
} while(0);

/** An universal split wrapper
 *
 *  «where» can be SG_EDGE_LEFT, SG_EDGE_RIGHT, SG_EDGE_TOP or SG_EDGE_BOTTOM
 */
#define sgSplitRect(rect, newrect, where, pixels) do { \
  Uint16 p = (pixels); \
  switch(where) \
  { \
    case SG_EDGE_LEFT: \
      sgSplitLeft(rect, newrect, p); \
      break; \
    case SG_EDGE_RIGHT: \
      sgSplitRight(rect, newrect, p); \
      break; \
    case SG_EDGE_TOP: \
      sgSplitTop(rect, newrect, p); \
      break; \
    case SG_EDGE_BOTTOM: \
      sgSplitBottom(rect, newrect, p); \
      break; \
    default: \
      break; \
  } \
} while(0);

/** Adds padding to a rectangle
 *  «where» can be SG_EDGE_LEFT | SG_EDGE_RIGHT | SG_EDGE_TOP | SG_EDGE_BOTTOM
 */
#define sgPadRect(rect, where, pixels) do { \
  if((where) & SG_EDGE_LEFT) \
  { \
    ((SDL_Rect *)rect)->x += (pixels); \
    ((SDL_Rect *)rect)->w -= (pixels); \
  } \
  if((where) & SG_EDGE_TOP) \
  { \
    ((SDL_Rect *)rect)->y += (pixels); \
    ((SDL_Rect *)rect)->h -= (pixels); \
  } \
  if((where) & SG_EDGE_RIGHT) \
  { \
    ((SDL_Rect *)rect)->w -= (pixels); \
  } \
  if((where) & SG_EDGE_BOTTOM) \
  { \
    ((SDL_Rect *)rect)->h -= (pixels); \
  } \
} while(0);

/** Aligns the rectangle to the x and y position according to flags set in «align»
 */
#define sgAlignPos(rect, x, y, align) do { \
  switch((align) & SG_ALIGN_HORIZONTAL) \
  { \
    case SG_ALIGN_RIGHT: \
      (rect)->x = (x) - (rect)->w; \
      break; \
    case SG_ALIGN_CENTER: \
      (rect)->x = (x) - (rect)->w / 2; \
      break; \
    case SG_ALIGN_LEFT: \
    default: \
      (rect)->x = (x); \
      break; \
  } \
  switch((align) & SG_ALIGN_VERTICAL) \
  { \
    case SG_ALIGN_TOP: \
      (rect)->y = (y); \
      break; \
    case SG_ALIGN_BOTTOM: \
      (rect)->y = (y) - (rect)->h; \
      break; \
    case SG_ALIGN_MIDDLE: \
    default: \
      (rect)->y = (y) - (rect)->h / 2; \
      break; \
  } \
} while(0);

/** Aligns the «inside» rectangle within the «outside» rectangle according
 *  to flags set in «align»
 */
#define sgAlignRect(outside, inside, align) do { \
  switch((align) & SG_ALIGN_HORIZONTAL) \
  { \
    case SG_ALIGN_RIGHT: \
      (inside)->x = (outside)->x + (outside)->w - (inside)->w; \
      break; \
    case SG_ALIGN_CENTER: \
      (inside)->x = (outside)->x + ((outside)->w - (inside)->w) / 2; \
      break; \
    case SG_ALIGN_LEFT: \
    default: \
      (inside)->x = (outside)->x; \
      break; \
  } \
  switch((align) & SG_ALIGN_VERTICAL) \
  { \
    case SG_ALIGN_TOP: \
      (inside)->y = (outside)->y; \
      break; \
    case SG_ALIGN_BOTTOM: \
      (inside)->y = (outside)->y + (outside)->h - (inside)->h; \
      break; \
    case SG_ALIGN_MIDDLE: \
    default: \
      (inside)->y = (outside)->y + ((outside)->h - (inside)->h) / 2; \
      break; \
  } \
} while(0);

/** Pixel format for 32-bit BGRA surfaces */
extern SDL_PixelFormat sgPixelFormat32;
  
/** Writes a log entry 
 *
 *  @param msg        Message to write to the log
 */
void         sgLog             (const char  *msg, 
                                ...);

/** Dumps rectangle dimensions to the log
 *  
 *  @param rect       The rectangle to dump out
 */
void         sgDumpRect        (SDL_Rect    *rect);
  
/** Dumps color values to the log
 *  
 *  @param color      The color to dump out
 */
void         sgDumpColor       (SDL_Color   *color);
  

/** Fades from one surface to another in a specified time
 * 
 *  @param screen     The screen surface 
 *  @param from       Original surface
 *  @param to         Resulting surface
 *  @param duration   Time in milliseconds
 */
void         sgFade            (SDL_Surface *screen,
                                SDL_Surface *from,
                                SDL_Surface *to, 
                                Uint32       duration);  

/** Copies from one RGBA surface to another
 * 
 *  @param src        Source surface
 *  @param sr         Source clipping rectangle
 *  @param dst        Destination surface
 *  @param dr         Destination clipping rectangle
 */
void         sgCopy            (SDL_Surface *src,
                                SDL_Rect    *sr,
                                SDL_Surface *dst, 
                                SDL_Rect    *dr);

/** Copies an 8-bit paletted surface to a 32-bit RGBA one by using the 
 *  per-surface-alpha of the source surface.
 *  (Currently not used by any widget)
 * 
 *  @param src        Source surface
 *  @param sr         Source clipping rectangle
 *  @param dst        Destination surface
 *  @param dr         Destination clipping rectangle
 */
void         sgCopyPalette     (SDL_Surface *src,
                                SDL_Rect    *sr,
                                SDL_Surface *dst, 
                                SDL_Rect    *dr);

/** Blits from one surface to another by using a feathered brush and applying
 *  color tinting
 * 
 *  @param src        Source surface
 *  @param sr         Source clipping rectangle
 *  @param dst        Destination surface
 *  @param dr         Destination rectangle
 *  @param tint       Color tinting value (also influences the alpha channel)
 */
void         sgBlur            (SDL_Surface *src,
                                SDL_Rect    *sr,
                                SDL_Surface *dst,
                                SDL_Rect    *dr,
                                sgColor      tint);

/** Additive alpha blit
 *  (Currently not used by any widget)
 * 
 *  @param src        Source surface
 *  @param sr         Source clipping rectangle
 *  @param dst        Destination surface
 *  @param dr         Destination rectangle
 */
void         sgAlphaBlit       (SDL_Surface *src,
                                SDL_Rect    *sr,
                                SDL_Surface *dst,
                                SDL_Rect    *dr);

/** Draws a gradient rectangle with fixed starting and 
 *  ending colors to an 8-bit Surface
 */
void         sgDrawRect        (SDL_Surface *dest, 
                                SDL_Rect    *rect,
                                Uint8        ia,
                                Uint8        ib, 
                                sgDraw       draw);

/** Draws a gradient rectangle to an 8-bit Surface according to the 
 *  drawing flags 
 */
void         sgDrawGradient    (SDL_Surface *dest,
                                SDL_Rect    *rect, 
                                sgDraw       draw);    
  
/** Draws a paletted frame to an 8-bit Surface
 * 
 *  @param dest       Destination surface (8-bit logically paletted)
 *  @param rect       Frame dimensions
 *  @param draw       Flags which determine frame appearance, see \ref sgDraw
 */
void         sgDrawSingleFrame (SDL_Surface *dest,
                                SDL_Rect    *rect, 
                                sgDraw       draw);
  
/** Draws a paletted 2-pixel-wide frame to an 8-bit Surface
 * 
 *  @param dest       Destination surface (8-bit logically paletted)
 *  @param rect       Frame dimensions
 *  @param draw       Flags which determine frame appearance, see \ref sgDraw
 */
void         sgDrawFrame       (SDL_Surface *dest,
                                SDL_Rect    *rect,
                                sgDraw       draw);

/** Draws a horizontal line to an 8-bit Surface
 *
 *  @param dest       destination surface (8-bit logically paletted)
 *  @param x1         starting position 
 *  @param x2         ending position
 *  @param y          position on y-axis
 *  @param i1         starting color
 *  @param i2         ending color
 */
void         sgDrawHLine       (SDL_Surface *dest,
                                Sint16       x1,
                                Sint16       x2,
                                Sint16       y,
                                Uint8        i1,
                                Uint8        i2);
  
/** Draws a horizontal line to an 8-bit Surface
 *  start and end colors are read from the start and end points
 * 
 *  @param dest       destination surface (8-bit logically paletted)
 *  @param x1         starting position 
 *  @param x2         ending position
 *  @param y          position on y-axis
 */
void         sgBlendHLine      (SDL_Surface *dest,
                                Sint16       x1,
                                Sint16       x2, 
                                Sint16       y);    

/** Draws a vertical line to an 8-bit Surface
 * 
 *  @param dest       destination surface (8-bit logically paletted)
 *  @param x          position on the x-axis
 *  @param y1         starting position
 *  @param y2         ending position
 *  @param i1         starting color
 *  @param i2         ending color
 */
void         sgDrawVLine       (SDL_Surface *dest, 
                                Sint16       x, 
                                Sint16       y1,
                                Sint16       y2,
                                Uint8        i1, 
                                Uint8        i2);

/** Draws a vertical line to an 8-bit Surface.
 *  Start and end colors are read from the start and end points.
 * 
 *  @param dest       destination surface (8-bit logically paletted)
 *  @param x          position on the x-axis
 *  @param y1         starting position
 *  @param y2         ending position
 */ 
void         sgBlendVLine      (SDL_Surface *dest, 
                                Sint16       x, 
                                Sint16       y1, 
                                Sint16       y2);
   
/** Converts a color value from HSV to RGB colorspace */
sgColor      sgHSVToRGB        (sgHSV        hsv);

/** Converts a color value from RGB to HSV colorspace */
sgHSV        sgRGBToHSV        (sgColor      rgb);

/** Create a palette consisting of 256 colors matching the specified hue
 * 
 *  @param base       hue value in the HSV model on which the palette will base.
 */
sgColor     *sgCreatePalette   (sgColor      base);

/** Create a palette consisting of 256 colors matching the specified hue
 *  but equalize the channels to get a greyscale palette
 * 
 *  @param base       hue value in the HSV model on which the palette will base.
 */
sgColor     *sgCreateBWPalette (sgColor      base);

/** Checks whether a specific x/y position lies within a rectangle
 * 
 *  @param rect       The rectangle to match
 *  @param x          The tested value on the X-axis
 *  @param y          The tested value on the Y-axis
 * 
 *  @return           1 when x and y are inside rect, 0 otherwise
 */
int          sgMatchRect       (SDL_Rect    *rect,
                                Sint16       x, 
                                Sint16       y);

/** Merges two rectangles (A and B)
 * 
 *  @param rect       Destination rectangle
 *  @param a          Rectangle A
 *  @param b          Rectangle B
 */
void         sgMergeRect       (SDL_Rect    *rect,
                                SDL_Rect     a,
                                SDL_Rect     b);

/** Checks whether rectangle A and B intersect
 * 
 *  @param a          Some rectangle
 *  @param b          Another rectangle
 * 
 *  @return           1 when rectangle A and B intersect, 0 otherwise
 */
void         sgIntersect       (SDL_Rect    *a, 
                                SDL_Rect    *b);

/** Clip rectangle so it will fit into the surface
 * 
 *  @param surface    The rectangle will be clipped to the dimensions of this surface
 *  @param rect       Rectangle to be clipped
 */
void         sgClip            (SDL_Surface *surface, 
                                SDL_Rect    *rect);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */
  
#endif /* SGUI_COMMON_H */
