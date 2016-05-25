/* $Id: widget.h,v 1.37 2005/05/18 10:05:54 smoli Exp $
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

#ifndef SGUI_WIDGET_H
#define SGUI_WIDGET_H

/** @defgroup sgWidget  sgWidget: Generic GUI element (abstract)
 * 
 *  @{
 */

#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** A handler which processes SDL events */
typedef int  sgWidgetHandler  (sgWidget  *widget,
                               SDL_Event *event);
  
/** A handler which catches events from parent widgets */
/*typedef int  sgWidgetProc     (sgWidget  *widget, 
                               sgEvent    event);*/

/** Blitting callback */  
typedef int  sgWidgetBlit     (sgWidget    *widget, 
                               SDL_Surface *surface, 
                               Sint16       x, 
                               Sint16       y);
  
/** The methods of a widget 
 *
 *  The methods of a widget are called whenever a redraw/recalc/deletion of the
 *  widget is requested, or when there are incoming SDL events 
 */
struct sgWidgetMethods
{
  sgWidgetCallback *delete;   /**< cleans the widget */
  sgWidgetCallback *recalc;   /**< calcs widget dimension */
  sgWidgetCallback *redraw;   /**< draws widget look */
  sgWidgetHandler  *handler;  /**< processes SDL events */
  sgWidgetBlit     *blit;     /**< blits widget to a surface */
};  
  
/** Widget methods type */
//typedef struct sgWidgetMethods sgWidgetMethods;
  
/** Defines widget-specific callbacks etc. 
 *
 *  The sgWidgetType structure is used for initializing a widget
 */  
struct sgWidgetType
{
  const char       *name;     /**< name of the widget */
  size_t            size;     /**< size of widget specific structure */
  sgWidgetMethods   methods;  /**< initial methods for widgets of this type */
};
  
/** Defines for widget status 
 *
 *  Indicates status of the widget
 */
enum sgWidgetStatus
{
  SG_RUNNING        = 0x00000001, /**< the dialog is running (sgRunDialog()) */
  SG_DISABLED       = 0x00000002, /**< is greyed out and won't receive events */
  SG_HIDDEN         = 0x00000004, /**< is not displayed and won't receive events */
  SG_FOCUS          = 0x00000008, /**< has focus */
  SG_NOFOCUS        = 0x00000010, /**< can't get focus */
  SG_GRAB           = 0x00000020, /**< widget grabs the focus */
  SG_REDRAW_FRAME   = 0x00010000, /**< frame surface of the widget must be redrawn */
  SG_REDRAW_BORDER  = 0x00020000, /**< border surface of the widget must be redrawn */
  SG_REDRAW_CONTENT = 0x00040000, /**< content surface of the widget must be redrawn */
  SG_REDRAW_SCREEN  = 0x00080000, /**< widget must be redrawn to screen */
  SG_REDRAW_PALETTE = 0x00100000, /**< widget must change palette */
  SG_REDRAW_MOUSE   = 0x00200000, /**< mouse pointer must be redrawn */
  SG_REDRAW_NEEDED  = 0x003f0000, /**< is a redraw needed? */
};
   
/** Checks if a redraw of the widget border surface is needed */
#define sgRedrawWidgetBorder(w)  (((w)->status & SG_REDRAW_BORDER) && (w)->face.border)

/** Checks if a redraw of the widget frame surface is needed */
#define sgRedrawWidgetFrame(w)   (((w)->status & SG_REDRAW_FRAME) && (w)->face.frame)

/** Checks if a redraw of the widget content surface is needed */
#define sgRedrawWidgetContent(w) (((w)->status & SG_REDRAW_CONTENT) && (w)->face.content)
  

/*#define SG_GROUP_BOTTOM 0
#define SG_GROUP_TOP    1
#define SG_GROUP_RIGHT  2
#define SG_GROUP_LEFT   3*/

/*#define sgSetWidgetStatus(w, flags)   (w)->status |=  (flags)
#define sgClearWidgetStatus(w, flags) (w)->status &= ~(flags) */


/** Widget surface data */
struct sgWidgetFace
{
  SDL_Surface   *border;  /**< transparent border stuff */
  SDL_Surface   *frame;   /**< paletted frames */ 
  SDL_Surface   *content; /**< 32-bit surface for fonts and images */
};
  
/** Widget color palette */
struct sgWidgetPalette
{
  struct sgColor *border;  /**< palette for the border surface */
  struct sgColor *frame;   /**< palette for the frame surface */
  struct sgColor *content; /**< palette for the content surface (e.g. fonts) */                                
};  
  
/** Widget surface data */
//typedef struct sgWidgetFace sgWidgetFace;

/** An abstract structure */
struct sgWidget 
{
  sgNode          node;         /**< node used to link to the tree */
  sgList          list;         /**< list of childrens */
  sgWidget       *parent;       /**< parent of this widget */
  sgWidget       *dialog;       /**< dialog of this widget */
  sgWidgetType   *type;         /**< type structure */
  sgWidgetFace    face;         /**< widget surfaces */
  sgWidgetPalette palette;      /**< palettes (color gradients) */
  sgWidgetProc   *proc;         /**< widget event procedure */
  sgWidgetMethods methods;      /**< widget methods */
  sgWidgetStatus  status;       /**< widget status */
  
  sgFont         *font[3];       /**< fonts used to draw the widget */
  
  char            caption[256];  /**< text/value */
  int             border;        /**< widget border width */
  Uint8           alpha;         /**< alpha value for border face */
  SDL_Rect        rect;          /**< dimension of the widget and its position relative to the parent */
  SDL_Rect        area;          /**< focus area */
  
  union
  {
    sgDialog     *dialog;
    sgButton     *button;
    sgGroup      *group;
    sgListbox    *listbox;
    sgEdit       *edit;
    sgAdjust     *adjust;
    sgLabel      *label;
    sgConsole    *console;
    sgInput      *input;
    sgImage      *image;
    sgTab        *tab;
    sgToggle     *toggle;
    sgDropdown   *dropdown;
    sgColorSel   *colorsel;
/*    sgLED       *led;*/
    void         *ptr;
  } data;                       /**< pointer to widget-type specific data */

//#warning "deprecated"
/*  char                **history;
  int                   history_pos;
  sgList                completions;*/
};


/** Frees the border face of a widget */
#define sgFreeWidgetBorder(w)   if(!(w)->face.border){}else{SDL_FreeSurface((w)->face.border);(w)->face.border=NULL;(w)->status|=SG_REDRAW_BORDER;}

/** Frees the frame face of a widget */
#define sgFreeWidgetFrame(w)    if(!(w)->face.frame){}else{SDL_FreeSurface((w)->face.frame);(w)->face.frame=NULL;(w)->status|=SG_REDRAW_FRAME;}

/** Frees the content face of a widget */
#define sgFreeWidgetContent(w)  if(!(w)->face.content){}else{SDL_FreeSurface((w)->face.content);(w)->face.content=NULL;(w)->status|=SG_REDRAW_CONTENT;}

/** Clears the border face of a widget */
#define sgClearWidgetBorder(w)  SDL_FillRect((w)->face.border, NULL, 0)
  
/** Clears the frame face of a widget */
#define sgClearWidgetFrame(w)   SDL_FillRect((w)->face.frame, NULL, 0)
  
/** Clears the content face of a widget */
#define sgClearWidgetContent(w) SDL_FillRect((w)->face.content, NULL, 0)

/** Global list of (root)-widgets */
extern sgList sgWidgetList;
  
/** Creates a new widget based on a rectangle
 * 
 *  @param type    widget type structure
 *  @param parent  parent widget 
 *  @param rect    rectangle of the widget relative to the parent
 *  @param caption text
 */
sgWidget *sgNewWidgetRect        (sgWidgetType *type,
                                  sgWidget     *parent,
                                  SDL_Rect      rect,
                                  const char   *caption);
/** Creates a new widget
 * 
 *  @param type    widget type structure
 *  @param parent  parent widget 
 *  @param x       x-position relative to the parent
 *  @param y       y-position of the button relative to the parent
 *  @param w       width
 *  @param h       height
 *  @param caption text
 */
sgWidget *sgNewWidget            (sgWidgetType *type,
                                  sgWidget     *parent,
                                  Sint16        x,
                                  Sint16        y,
                                  Uint16        w,
                                  Uint16        h,
                                  const char   *caption);

/** Free widget ressources */
void      sgFreeWidget           (sgWidget     *widget);

/** Free widget ressources and the widget struct itself (recurses) */
void      sgDeleteWidget         (sgWidget     *widget);
    
/** Set widget status flags */
void      sgSetWidgetStatus      (sgWidget     *widget, 
                                  int           flags);
  
/** Clear widget status flags */
void      sgClearWidgetStatus    (sgWidget     *widget, 
                                  int           flags);

/** Sets the widget procedure */
void      sgSetWidgetProc        (sgWidget     *widget, 
                                  sgWidgetProc *proc);
    
/** Set the widget position */
void      sgSetWidgetPos         (sgWidget     *widget, 
                                  Sint16        x, 
                                  Sint16        y,
                                  sgAlign       align);

/** Set the widget dimensions */
void      sgSetWidgetSize        (sgWidget     *widget, 
                                  Uint16        w, 
                                  Uint16        h);

/** Set both, widget position and dimension */
void      sgSetWidgetRect        (sgWidget     *widget, 
                                  SDL_Rect      rect);

/** Sets the widget border thickness */
void      sgSetWidgetBorder      (sgWidget     *widget, 
                                  int           border);
  
/** Blit widget to the screen */
int       sgBlitWidget           (sgWidget     *widget,
                                  SDL_Surface  *surface, 
                                  Sint16        x, 
                                  Sint16        y);
  
/** Report an event to the widget event procedure */
int       sgReportWidgetEvent    (sgWidget     *widget, 
                                  sgEvent       event);    
    
/** Recalc widget dimensions */
void      sgRecalcWidget         (sgWidget    *widget);
  
/** Split widget */
void      sgSplitWidget          (sgWidget    *widget,
                                  SDL_Rect    *newrect,
                                  sgEdge       edge,
                                  int          pixels);
  
/** Clears all widget surfaces */
void      sgClearWidget          (sgWidget    *widget);
  
/** Creates widget border surface (8-bit indexed, color-keyed, source alpha) */
void      sgCreateWidgetBorder   (sgWidget    *widget);
  
/** Converts widget border surface to display format */
void      sgConvertWidgetBorder  (sgWidget    *widget);
  
/** Creates widget frame surface (8-bit indexed, color-keyed, opaque) */
void      sgCreateWidgetFrame    (sgWidget    *widget);

/** Converts widget frame surface to display format */
void      sgConvertWidgetFrame   (sgWidget    *widget);
  
/** Creates widget content surface (32-bit, alpha layered) */
void      sgCreateWidgetContent  (sgWidget    *widget);
  
/** Converts widget content surface to display format */
void      sgConvertWidgetContent (sgWidget    *widget);  
  
/** Create widget surfaces */
void      sgCreateWidgetFace     (sgWidget    *widget);
  
/** Free widget surfaces */
void      sgFreeWidgetFace       (sgWidget    *widget);
  
/** Clears all widget surfaces */
void      sgClearWidgetFace      (sgWidget    *widget);
  
/** Creates border surface if it doesn't exist and draws a gradient to it */
void      sgDrawWidgetBorder     (sgWidget    *widget,
                                  SDL_Rect    *cutout);

/** Renders a widget to its surfaces */
void      sgRedrawWidget         (sgWidget    *widget);
  
/** Handles incoming events for the widget */
int       sgHandleWidgetEvent    (sgWidget    *widget, 
                                  SDL_Event   *event);
  
/** Draws a widget to the screen */
void      sgDrawWidget           (sgWidget    *widget, 
                                  SDL_Surface *surface);

/** Disable widget */
void      sgDisableWidget        (sgWidget    *widget);

/** Hide and disable widget */
void      sgHideWidget           (sgWidget    *widget);

/** Show and re-enable widget */
void      sgShowWidget           (sgWidget    *widget);

/** Re-enable widget */
void      sgEnableWidget         (sgWidget    *widget);

  
/** Set color for the widget frames and generate a new frame palette */
void      sgSetWidgetFrameColor  (sgWidget    *widget, 
                                  sgColor      color);
  
/** Set color for the widget borders and generate a new border palette */
void      sgSetWidgetBorderColor (sgWidget    *widget, 
                                  sgColor      color);        
  
/** Set color for the widget contents and generate a new content palette */
void      sgSetWidgetContentColor(sgWidget    *widget,
                                  sgColor      color);
  
/** Set all widget colors and generate new palettes
 * 
 *  @param widget  widget which will have its color changed
 *  @param color   new widget color
 */
void      sgSetWidgetColor       (sgWidget    *widget,
                                  sgColor      color);

/** Set RGB value for the widget frames and generate a new frame palette */
void      sgSetWidgetFrameRGB    (sgWidget    *widget,
                                  Uint8        r,
                                  Uint8        g,
                                  Uint8        b);
  
/** Set RGB value for the widget borders and generate a new border palette */
void      sgSetWidgetBorderRGB   (sgWidget    *widget, 
                                  Uint8        r,
                                  Uint8        g,
                                  Uint8        b);
  
/** Set RGB value for the widget contents and generate a new content palette */
void      sgSetWidgetContentRGB  (sgWidget    *widget,
                                  Uint8        r,
                                  Uint8        g,
                                  Uint8        b);
  
/** Generate widget palette from RGB value */
void      sgSetWidgetRGB         (sgWidget    *widget,
                                  Uint8        r,
                                  Uint8        g, 
                                  Uint8        b);    
  
/** Set a widget font
 * 
 *  @param widget  the widget which will have fonts loaded
 *  @param type    font face type (SG_FONT_NORMAL, SG_FONT_BOLD, SG_FONT_FIXED)
 *  @param font    the specified font face
 * 
 *  @return        returns 0 on success, -1 on error
 */
void      sgSetWidgetFont        (sgWidget     *widget,
                                  int           type,
                                  sgFont       *font);

/** Load a bitmap font for the widget
 * 
 *  @param widget  the widget which will have fonts loaded
 *  @param type    font face type (SG_FONT_NORMAL, SG_FONT_BOLD, SG_FONT_FIXED)
 *  @param file    GIF file with the specified font face
 * 
 *  @return        returns 0 on success, -1 on error
 */
int       sgLoadWidgetFont       (sgWidget     *widget,
                                  int           type,
                                  const char   *file);

/** Set all widget fonts
 * 
 *  @param widget  the widget which will have fonts loaded
 *  @param normal  «normal» font face
 *  @param bold    «bold» font face
 *  @param fixed   «fixed» font face
 */
void      sgSetWidgetFonts       (sgWidget     *widget,
                                  sgFont       *normal,
                                  sgFont       *bold,
                                  sgFont       *fixed);

/** Load bitmap fonts for the widget
 * 
 *  @param widget  the widget which will have fonts loaded
 *  @param normal  GIF file with the «normal» font face
 *  @param bold    GIF file with the «bold» font face
 *  @param fixed   GIF file with the «fixed» font face
 * 
 *  @return        returns 0 on success, -1 on error
 */
int       sgLoadWidgetFonts      (sgWidget     *widget,
                                  const char   *normal,
                                  const char   *bold,
                                  const char   *fixed);
  
/** Sets the caption of a widget (va_list)
 * 
 *  @param widget  widget which will have the caption changed
 *  @param caption format string 
 *  @param args    va_list for vsnprintf
 */
void      sgvSetWidgetCaption    (sgWidget     *widget,
                                  const char   *caption, 
                                  va_list       args);

/** Sets the caption of a widget (vararg)
 * 
 *  @param widget  widget which will have the caption changed
 *  @param caption format string 
 *  @param ...     arguments
 */
void      sgSetWidgetCaption     (sgWidget     *widget, 
                                  const char   *caption, 
                                  ...);

/** Gets the caption of a widget */
char     *sgGetWidgetCaption     (sgWidget     *widget);

/** A wrapper to SDL_GetMouseState which returns a position relative to the
 *  widget
 */
Uint8     sgGetWidgetMouse       (sgWidget     *widget, 
                                  Sint16       *x, 
                                  Sint16       *y);
/** Indicates whether the widget and all its parents have focus
 */
int       sgHasWidgetFocus       (sgWidget     *widget);    


/** Handles motion highlite for a rectangle inside the widget */
void      sgHandleWidgetHilite   (sgWidget     *widget,
                                  SDL_Rect      rect,
                                  sgDraw       *draw,
                                  Sint16        x,
                                  Sint16        y);
  
  
#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* SGUI_WIDGET_H */
