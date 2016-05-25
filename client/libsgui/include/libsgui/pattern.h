/* $Id: pattern.h,v 1.6 2005/05/18 10:05:54 smoli Exp $
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

#ifndef SGUI_PATTERN_H
#define SGUI_PATTERN_H

/** @defgroup sgPattern  sgPattern: Procedural pattern generators
 *  @{
 */

#include <SDL.h>
#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C" { 
#endif /* __cplusplus */

/** The color index is a function of the x/y-position
 *  within the surface 
 */
typedef Uint8            sgPatternFunc    (Sint16       x,
                                           Sint16       y,
                                           Uint16       w,
                                           Uint16       h);
/** A structure describing a pattern */ 
struct sgPattern
{
  char           name[32]; /**< Short description of the pattern */
  sgPatternFunc *fn;       /**< The function which generates it */
};  

/** Pattern datatype */
typedef struct sgPattern sgPattern;

/** Returns a pointer to the pattern table */
sgPattern               *sgGetPatternTable(void);
  
  
/** Finds a pattern by its name */
sgPattern               *sgFindPattern    (const char  *name);
  
/** Initialize sine/cosine tables */
void                     sgInitPatterns   (void);
  
/** Fill a surface using a pattern function  */
void                     sgFillPattern    (SDL_Surface *surface,
                                           sgPattern   *fn,
                                           Uint8        contrast);

#ifdef __cplusplus
}
#endif /* __cplusplus */
 
/** @} */

#endif /* SGUI_PATTERN_H */

