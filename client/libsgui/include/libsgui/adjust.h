/* $Id: adjust.h,v 1.16 2005/05/18 22:45:56 smoli Exp $
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

#ifndef SGUI_ADJUST_H
#define SGUI_ADJUST_H

/** @defgroup sgAdjust sgAdjust: Adjust bar widget
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

#ifdef __cplusplus
extern "C" {
#endif

/** specialised structure for an adjust bar widget 
 */
struct sgAdjust 
{
  sgDraw      pushed; /**< State of the knob */
  sgDraw      rdraw;  /**< draw flags for the ruler */
  int         offset; /**< Clicked offset within know rectangle */
  double      pos;    /**< Current position of the ruler */
  double      max;    /**< Maximal value */
  double      delta;  /**< delta when start != 0.0 */
  int         len;    /**< Length of the ruler in pixels */
  const char *fmt;    /**< format string for the value */
  int         cursor; /**< cursor status */
  
  SDL_Rect    caption; /**< Rectangle for the caption */
  SDL_Rect    value;   /**< Rectangle for the value */
  SDL_Rect    range;   /**< Rectangle for the ruler-range */
  SDL_Rect    ruler;   /**< Rectangle for the ruler frame */
  SDL_Rect    knob;    /**< Rectangle for the knob */
};

/** data type for widget-specific storage */
typedef struct sgAdjust sgAdjust;

/** macro to access the specialised adjust bar structure */
#define sgAdjust(w) (w)->data.adjust

/** Adjust widget type */
extern sgWidgetType sgAdjustType;

/** Creates a new adjust widget by splitting another
 * 
 *  @param based   widget which we split
 *  @param edge    at which edge «based» will be splitted
 *  @param pixels  how many pixels to splitt off
 *  @param start   start of the adjust range
 *  @param end     end of the adjust range
 *  @param caption caption of the widget
 *  
 *  @return        pointer to the newly created widget or NULL on error.
 */
sgWidget *sgNewAdjustSplitted(sgWidget   *based,
                              sgEdge      edge,
                              Uint16      pixels,
                              double      start, 
                              double      end, 
                              const char *caption);


  
/** Creates a new adjust widget and subtracts it to a group
 * 
 *  @param group   group the widget will be added to
 *  @param edge    at which edge «group» will be splitted
 *  @param align   alignment of the widget inside the group
 *  @param w       width of the widget
 *  @param h       height of the widget
 *  @param start   start of the adjust range
 *  @param end     end of the adjust range
 *  @param caption caption of the widget
 *  
 *  @return        pointer to the newly created widget or NULL on error.
 */
sgWidget *sgNewAdjustGrouped (sgWidget   *group,
                              sgEdge      edge,
                              sgAlign     align,
                              Uint16      w, 
                              Uint16      h,
                              double      start, 
                              double      end, 
                              const char *caption);

/** Creates a new adjust widget and adds it to a group
 * 
 *  @param group   group the widget will be added to
 *  @param edge    at which edge «group» will be enhanced
 *  @param align   alignment of the widget inside the group
 *  @param w       width of the widget
 *  @param h       height of the widget
 *  @param start   start of the adjust range
 *  @param end     end of the adjust range
 *  @param caption caption of the widget
 *  
 *  @return        pointer to the newly created widget or NULL on error.
 */
sgWidget *sgNewAdjustAligned (sgWidget   *group,
                              sgEdge      edge,
                              sgAlign     align,
                              Uint16      w, 
                              Uint16      h,
                              double      start, 
                              double      end, 
                              const char *caption);

/** Creates a new adjustment widget from a rectangle
 *
 *  @param parent  parent the widget will be added to
 *  @param rect    position and dimensions of the widget relative to the parent
 *  @param start   start of the adjust range
 *  @param end     end of the adjust range
 *  @param caption caption of the adjustment widget
 *
 *  @return        A pointer to the newly created widget or NULL on error.
 */
sgWidget *sgNewAdjustRect    (sgWidget   *parent, 
                              SDL_Rect    rect,
                              double      start, 
                              double      end, 
                              const char *caption);
  
/** Creates a new adjustment widget
 *
 *  @param parent  parent the widget will be added to
 *  @param x       x-position of the widget relative to the parent
 *  @param y       y-Position of the widget relative to the parent
 *  @param w       width of the widget
 *  @param h       height of the widget
 *  @param start   start of the adjust range
 *  @param end     end of the adjust range
 *  @param caption caption of the adjustment widget
 *
 *  @return        A pointer to the newly created widget or NULL on error.
 */
sgWidget *sgNewAdjust        (sgWidget   *parent,
                              Sint16      x,
                              Sint16      y,
                              Uint16      w, 
                              Uint16      h, 
                              double      start, 
                              double      end, 
                              const char *caption);

/** Calculates internal widget dimensions
 *
 *  @param adjust  The adjust bar widget to recalculate
 */
void      sgRecalcAdjust     (sgWidget   *adjust);
  
/** Redraws adjust widget faces
 * 
 *  @param adjust  The adjust bar widget to draw
 */
void      sgRedrawAdjust     (sgWidget   *adjust);
  
/** Handles an incoming event for the adjust bar widget
 * 
 *  @param adjust  The adjust bar widget which receives the event
 *  @param event   An SDL_Event structure from SDL_PollEvent()
 * 
 *  @return        1 if some action has taken place, 0 otherwise
 */
int       sgHandleAdjustEvent(sgWidget   *adjust, 
                              SDL_Event  *event);

/** Sets the format string which is used for the formatting of the value
 * 
 *  @param adjust  Adjust bar widget
 *  @param fmt     Format string - See the manual page of printf(3),
 *                 the value of the adjust bar is passed to snprintf(3) as
 *                 integer argument.
 */
int       sgSetAdjustFormat  (sgWidget   *adjust, 
                              const char *fmt);  

/** Sets the position of the adjust bar knob
 * 
 *  @param adjust  The adjust bar widget which will have its position changed
 *  @param pos     A value from 0 to «range»
 * 
 *  @return        Returns 1 when the value has changed, 0 otherwise
 */
int       sgSetAdjustValue   (sgWidget   *adjust,
                              double      pos);

/** Gets the position of the adjust bar knob
 * 
 *  @param adjust  The adjust bar widget of which the position is requested
 *  @param pos     A pointer to integer value which will receive the current 
 *                 position or NULL
 * 
 *  @return        The current knob position
 */
double    sgGetAdjustValue   (sgWidget   *adjust, 
                              double     *pos);
#ifdef __cplusplus
}
#endif

/** @} */

#endif /* SGUI_ADJUST_H */
