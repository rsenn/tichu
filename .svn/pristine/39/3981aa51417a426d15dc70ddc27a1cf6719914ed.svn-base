/* $Id: font.h,v 1.11 2005/05/14 08:53:08 smoli Exp $
 * ------------------------------------------------------------------------- *
 *                 /                                                         *
 *  ___  ___                                                                 *
 * |___ |   )|   )|        Simple and smooth GUI library :)                  *
 *  __/ |__/ |__/ |        Copyright (C) 2003-2005  Roman Senn               *
 *      __/                                                                  *
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

#ifndef SGUI_FONT_H
#define SGUI_FONT_H

/** @defgroup sgFont sgFont: Font loading and blitting routines
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
  
#include <libsgui/stub.h>
#include <libsgui/list.h>

/** Font type enumeration */
enum sgFontType 
{
  SG_FONT_NORMAL = 0,  /**< Used mainly for labels */
  SG_FONT_BOLD,        /**< Used for titles and button captions */
  SG_FONT_FIXED        /**< Used for editboxes, listboxes, etc. */  
};

/** Structure for a loaded font */
struct sgFont 
{
  sgNode       node;       /**< Node which links to the font list */
  SDL_Surface *font;       /**< Pixel data */
  SDL_Color    color;      /**< Current color? */
  char         name[128];  /**< Name of the font */
  SDL_Rect     rects[256]; /**< Source rectangle for each ASCII character */
};

/** Initializes the font module */
void    sgInitFonts       (void);

/** Loads a font from a .png file */
sgFont *sgLoadFontFile    (const char  *file);

/** Loads a font from an RWops containing a .png file */
sgFont *sgLoadFontRWops   (SDL_RWops   *rwops);

/** Loads a font from a previously opened .png file */
sgFont *sgLoadFontFp      (FILE        *fp);
  

/** Adds a new font, getting it from an already loaded surface */
sgFont *sgAddFont         (SDL_Surface *font);

/** Converts a font into display format for accelerated blitting */
void    sgConvertFont     (sgFont      *font);
 
/** Frees a previously loaded font */
void    sgFreeFont        (sgFont      *font);

/** Frees all loaded fonts */
void    sgFreeFonts       (void);

/** Draws a single character to a surface */
void    sgDrawChar        (sgFont      *font,
                           SDL_Color   *color, 
                           SDL_Surface *dest, 
                           Uint16       x, 
                           Uint16       y, 
                           char         c);

/** Draws a text using the specified font
 *
 *  @param font  The font in which the text should appear
 *  @param color Color of the rendered glyphs
 *  @param dest  Where we will blit the text to
 *  @param rect  Destination rectnagle
 *  @param align Alignment inside the rectangle
 *  @param text  The text we'll render
 */
void    sgDrawText        (sgFont      *font, 
                           SDL_Color   *color, 
                           SDL_Surface *dest,
                           SDL_Rect    *rect,
                           int          align,
                           const char  *text);

/** Draws a text using the specified font and renders outlines using the
 *  blur function
 *  
 *  @param font  The font in which the text should appear
 *  @param dest  Where we will blit the text to
 *  @param rect  Destination rectnagle
 *  @param align Alignment inside the rectangle
 *  @param text  The text we'll render
 */
void    sgDrawTextOutline (sgFont      *font,
                           SDL_Surface *dest, 
                           SDL_Rect    *rect,
                           int          align,
                           const char  *text);


/** Returns the first font in the list */
sgFont *sgGetFirstFont    (void);
  
/** Gets the width of the specified text using the specified font in pixels */
int     sgTextWidth       (sgFont      *font,
                           const char  *text);

/** Gets the height of the specified font in pixels */
int     sgFontHeight      (sgFont      *font);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */
  
#endif /* SGUI_FONT_H */
