/* $Id: image.h,v 1.10 2005/05/18 22:45:56 smoli Exp $
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

#ifndef SGUI_IMAGE_H
#define SGUI_IMAGE_H

/** @defgroup sgImage   sgImage: Image widget
 *  @{
 */

#include <stdarg.h>
#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C" {  
#endif /* __cplusplus */

/** a specialised structure for a image widget */
struct sgImage 
{
  SDL_Rect     outer;   /**< outer frame rectangle */
  SDL_Rect     inner;   /**< inner frame rectangle */
  SDL_Rect     caption; /**< rectangle which will contain the text */
  SDL_Rect     body;    /**< rectangle which will contain the image */
  SDL_Rect     srect;   /**< source rectangle */
  SDL_Rect     drect;   /**< destination rectangle */
  SDL_Surface *image;   /**< image surface (unconverted) */
  sgAlign      align;   /**< alignment of the image */
  int          max;     /**< maximal character count */
};

/** Data type for image specific storage */
typedef struct sgImage sgImage;

/** A macro to access the specialised image structure */
#define sgImage(w) (w)->data.image
   
/** Configuration and initial methods of the image widget */
extern sgWidgetType sgImageType;

/** Creates a new image widget by splitting another
 * 
 *  @param based    widget which we'll split
 *  @param edge     at which edge «based» will be splitted
 *  @param pixels   how many pixels to split off
 *  @param caption  initial image text
 *
 *  @return         pointer to the newly created image or NULL on error.
 */
sgWidget *sgNewImageSplitted (sgWidget    *based,
                              sgEdge       edge,
                              int          pixels,
                              const char  *caption);
  
/** Creates a new image widget and adds it to a group
 * 
 *  @param group    group the editbox will be added to
 *  @param edge     at which edge «group» will be splitted
 *  @param align    alignment of the image inside the group
 *  @param w        width of the image
 *  @param h        height of the image
 *  @param caption  initial image text
 *  
 *  @return         pointer to the newly created image or NULL on error.
 */
sgWidget *sgNewImageGrouped  (sgWidget    *group,
                              sgEdge       edge,
                              sgAlign      align,
                              Uint16       w, 
                              Uint16       h,
                              const char  *caption);

/** Creates a new image widget from rectangle dimensions
 * 
 *  @param parent   widget which will contain the image
 *  @param rect     image position and dimensions
 *  @param caption  initial image text
 *  
 *  @return         pointer to the newly created image or NULL on error.
 */
sgWidget *sgNewImageRect     (sgWidget    *parent,
                              SDL_Rect     rect, 
                              const char  *caption);


/** Creates a new image widget 
 * 
 *  @param parent   widget which will contain the image
 *  @param x        x-position of the image relative to the parent
 *  @param y        y-position of the image relative to the parent
 *  @param w        width of the image
 *  @param h        height of the image
 *  @param caption  initial image text
 *
 *  @return         pointer to the newly created image or NULL on error
 */
sgWidget *sgNewImage         (sgWidget    *parent,
                              Sint16       x,
                              Sint16       y, 
                              Uint16       w,
                              Uint16       h,
                              const char  *caption);

/** Recalcs image dimensions */
void      sgRecalcImage      (sgWidget    *image);

/** Redraws image look */
void      sgRedrawImage      (sgWidget    *image);

/** Sets the image to display inside the widget */
int       sgSetImageSurface  (sgWidget    *image, 
                              SDL_Surface *surface,
                              sgAlign      align);

/** Sets alignment for the image inside the body rectangle */
int       sgSetImageAlign    (sgWidget    *image, 
                              sgAlign      align);
    
/** Loads an image from a file */
int       sgLoadImageFile    (sgWidget    *image, 
                              const char  *file, 
                              sgAlign      align);

/** Loads an image from am SDL_RWops */
int       sgLoadImageRWops   (sgWidget    *image, 
                              SDL_RWops   *rwops, 
                              sgAlign      align);

/** Loads an image from am SDL_RWops */
int       sgLoadImageFp      (sgWidget    *image, 
                              FILE        *fp, 
                              sgAlign      align);
/** Blits an image widget */ 
int       sgBlitImage        (sgWidget    *widget,
                              SDL_Surface *surface, 
                              Sint16       x, 
                              Sint16       y);
  
/** Handle events concerning the image */
int       sgHandleImageEvent (sgWidget    *image, 
                              SDL_Event   *event);    
#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */
  
#endif /* SGUI_IMAGE_H */
