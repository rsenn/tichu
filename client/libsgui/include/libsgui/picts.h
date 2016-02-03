/* $Id: picts.h,v 1.8 2005/05/22 02:44:34 smoli Exp $
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

#ifndef SG_PICTS_H
#define SG_PICTS_H

/** @defgroup sgPicts sgPicts: Pictograms
 * 
 *  @{
 */  

#ifdef __cplusplus
extern "C" {  
#endif /* __cplusplus */

/** Pictogram info and data */
struct sgPict 
{
  Uint16 w;      /**< pictogram width */
  Uint16 h;      /**< pictogram height */
  Uint8  data[]; /**< bitmap data */
};

/** A pictogram */
//typedef struct sgPict sgPict;

/* -------------------------------------------------------------------------- */
extern sgPict sgArrowUp;      /**< arrow pointing upwards */
extern sgPict sgArrowDown;    /**< arrow pointing downwards */
extern sgPict sgArrowDrop;    /**< arrow for undeployed dropdown */
extern sgPict sgArrowUndrop;  /**< arrow for deployed dropdown */
extern sgPict sgCircle;       /**< a 16x16 circle */

/** Applies pictogram data to a surface (some kind of bump-mapping) */
extern void sgPutPict(SDL_Surface *surface, SDL_Rect *drect, sgPict *pict,
                      sgDraw draw);

/** Copies pictogram data to a surface */
extern void sgCopyPict(SDL_Surface *surface, SDL_Rect *drect, sgPict *pict,
                       sgDraw draw);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* SG_PICTS_H */
