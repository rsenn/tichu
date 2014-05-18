/* $Id: colorsel.h,v 1.4 2005/05/19 23:08:32 smoli Exp $
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

#ifndef SGUI_COLORSEL_H
#define SGUI_COLORSEL_H

/** @defgroup sgColorSel sgColorSel: ColorSel bar widget
 *
 *                     The adjust bar widget consists of a title, a caption, a 
 *                     ruler and a knob.
 *                     The knob can be moved in the range of the ruler, while
 *                     the position (which is represented by the caption of
 *                     the widget) is scaled from <i>ruler width</i> to
 *                     <i>range</i>, which means it is always a value from 0 to
 *                     <i>range</i>.
 *  @{
 */

#include <libsgui/stub.h>
#include <libsgui/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Color select mode */
enum sgColorSelMode
{
  SG_COLORSEL_HUE,        /**< Adjust the hue channel */
  SG_COLORSEL_SATURATION, /**< Adjust the saturation channel */
  SG_COLORSEL_VALUE,      /**< Adjust the value channel */
};  
  
/** Color select mode type */
typedef enum sgColorSelMode sgColorSelMode;
  
/** specialised structure for an color select widget */
struct sgColorSel 
{
  sgDraw         pushed;  /**< State of the knob */
  sgDraw         rdraw;   /**< Draw flags for the ruler */
  int            offset;  /**< Clicked offset within know rectangle */
  int            pos;     /**< Current position of the ruler */
  int            max;     /**< Maximal value */
  int            len;     /**< Length of the ruler in pixels */
  int            cursor;  /**< cursor status */
  sgHSV          hsv;     /**< HSV color value */
  sgColorSelMode mode;    /**< Color selection mode */
  SDL_Rect       caption; /**< Rectangle for the caption */
  SDL_Rect       preview; /**< Rectangle for the preview */
  SDL_Rect       range;   /**< Rectangle for the ruler-range */
  SDL_Rect       outer;   /**< Rectangle for the outer ruler frame */
  SDL_Rect       inner;   /**< Rectangle for the ruler frame */
  SDL_Rect       knob;    /**< Rectangle for the knob */
  SDL_Rect       window;  /**< The Window in the know */
};

/** data type for widget-specific storage */
typedef struct sgColorSel sgColorSel;

/** macro to access the specialised color select structure */
#define sgColorSel(w) (w)->data.colorsel

/** ColorSel widget type */
extern sgWidgetType sgColorSelType;

/** Creates a new color selection widget by splitting another
 * 
 *  @param based   widget which we split
 *  @param edge    at which edge «based» will be splitted
 *  @param pixels  how many pixels to splitt off
 *  @param mode    color selection mode
 *  @param caption caption of the widget
 *  
 *  @return        pointer to the newly created widget or NULL on error.
 */
sgWidget *sgNewColorSelSplitted(sgWidget      *based,
                                sgEdge         edge,
                                Uint16         pixels,
                                sgColorSelMode mode, 
                                const char    *caption);

/** Creates a new color selection widget and subtracts it from a group
 * 
 *  @param group   group the widget will be added to
 *  @param edge    at which edge «group» will be splitted
 *  @param align   alignment of the widget inside the group
 *  @param w       width of the widget
 *  @param h       height of the widget
 *  @param mode    color selection mode
 *  @param caption caption of the widget
 *  
 *  @return        pointer to the newly created widget or NULL on error.
 */
sgWidget *sgNewColorSelGrouped (sgWidget      *group,
                                sgEdge         edge,
                                sgAlign        align,
                                Uint16         w, 
                                Uint16         h,
                                sgColorSelMode mode,
                                const char    *caption);

/** Creates a new color selection widget and adds it to a group
 * 
 *  @param group   group the widget will be added to
 *  @param edge    at which edge «group» will be enhanced
 *  @param align   alignment of the widget inside the group
 *  @param w       width of the widget
 *  @param h       height of the widget
 *  @param mode    color selection mode
 *  @param caption caption of the widget
 *  
 *  @return        pointer to the newly created widget or NULL on error.
 */
sgWidget *sgNewColorSelAligned (sgWidget      *group,
                                sgEdge         edge,
                                sgAlign        align,
                                Uint16         w, 
                                Uint16         h,
                                sgColorSelMode mode,
                                const char    *caption);

/** Creates a new color selection widget from a rectangle
 *
 *  @param parent  parent the widget will be added to
 *  @param rect    position and dimensions of the widget relative to the parent
 *  @param mode    color selection mode
 *  @param caption caption of the color select widget
 *
 *  @return        A pointer to the newly created widget or NULL on error.
 */
sgWidget *sgNewColorSelRect    (sgWidget      *parent, 
                                SDL_Rect       rect,
                                sgColorSelMode mode,
                                const char    *caption);
  
/** Creates a new color selection widget
 *
 *  @param parent  parent the widget will be added to
 *  @param x       x-position of the widget relative to the parent
 *  @param y       y-Position of the widget relative to the parent
 *  @param w       width of the widget
 *  @param h       height of the widget
 *  @param mode    color selection mode
 *  @param caption caption of the color select widget
 *
 *  @return        A pointer to the newly created widget or NULL on error.
 */
  sgWidget *sgNewColorSel        (sgWidget      *parent,
                                  Sint16         x,
                                  Sint16         y,
                                  Uint16         w, 
                                  Uint16         h, 
                                  sgColorSelMode mode,
                                  const char    *caption);
  
/** Calculates internal widget dimensions
 *
 *  @param colorsel  The colorsel bar widget to recalculate
 */
void      sgRecalcColorSel       (sgWidget      *colorsel);
  
/** Redraws colorsel widget faces
 * 
 *  @param colorsel  The colorsel bar widget to draw
 */
void      sgRedrawColorSel       (sgWidget      *colorsel);
  
/** Handles an incoming event for the colorsel bar widget
 * 
 *  @param colorsel  The colorsel bar widget which receives the event
 *  @param event     An SDL_Event structure from SDL_PollEvent()
 * 
 *  @return          1 if some action has taken place, 0 otherwise
 */
int       sgHandleColorSelEvent  (sgWidget      *colorsel, 
                                  SDL_Event     *event);

/** Sets the position of the colorsel bar knob
 * 
 *  @param colorsel  The colorsel bar widget which will have its position changed
 *  @param pos       A value from 0 to «range»
 * 
 *  @return          Returns 1 when the value has changed, 0 otherwise
 */
int       sgSetColorSelPos       (sgWidget      *colorsel, 
                                  int            pos);

/** Gets the position of the colorsel bar knob
 * 
 *  @param colorsel  The colorsel bar widget of which the position is requested
 *  @param pos       A pointer to integer value which will receive the current 
 *                   position or NULL
 * 
 *  @return          The current knob position
 */
int       sgGetColorSelPos       (sgWidget      *colorsel, 
                                  int           *pos);

/** Blit a color selection widget */
int       sgBlitColorSel         (sgWidget      *colorsel,
                                  SDL_Surface   *surface, 
                                  Sint16         x,
                                  Sint16         y);
  
/** Get the color of the color select widget (RGB) */
sgColor   sgGetColorSelRGB        (sgWidget      *colorsel);

/** Set the color of the color select widget (RGB) */
void      sgSetColorSelRGB        (sgWidget     *colorsel,
                                   sgColor       rgb);
    
/** Get the color of the color select widget (HSV) */
sgHSV     sgGetColorSelHSV        (sgWidget      *colorsel);

/** Set the color of the color select widget (HSV) */
void      sgSetColorSelHSV        (sgWidget     *colorsel,
                                   sgHSV         hsv);
    
/** Set the hue value of the color select widget */
void      sgSetColorSelHue        (sgWidget     *colorsel,
                                   Uint8         hue);
  
/** Set the saturation value of the color select widget */
void      sgSetColorSelSaturation (sgWidget     *colorsel, 
                                   Uint8         saturation);

/** Set the value of the color select widget */
void      sgSetColorSelValue      (sgWidget     *colorsel, 
                                   Uint8         value);

/** Get the hue value of the color select widget */
Uint8     sgGetColorSelHue        (sgWidget     *colorsel);

/** Get the saturation value of the color select widget */
Uint8     sgGetColorSelSaturation (sgWidget     *colorsel);

/** Get the value of the color select widget */
Uint8     sgGetColorSelValue      (sgWidget     *colorsel);
              
  
#ifdef __cplusplus
}
#endif

/** @} */

#endif /* SGUI_COLORSEL_H */
